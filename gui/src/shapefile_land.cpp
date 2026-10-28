/**************************************************************************
 *   Copyright (C) 2012-2026 by David S. Register                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement shapefile_land.h -- Thread safe land crossing queries
 */

#include <algorithm>
#include <atomic>
#include <cmath>
#include <utility>

#include "shapefile_land.h"

using LatLon = std::pair<double, double>;

/** True if segments AB and CD intersect, points as (lat, lon). */
static bool LineLineIntersect(const LatLon &A, const LatLon &B, const LatLon &C,
                              const LatLon &D) {
  constexpr double EPSILON = 1e-9;  // Tolerance for floating-point comparisons

  // Fast bounding box check - early rejection for improved performance
  double minABx = std::min(A.first, B.first);
  double maxABx = std::max(A.first, B.first);
  double minCDx = std::min(C.first, D.first);
  double maxCDx = std::max(C.first, D.first);

  if (maxABx < minCDx || maxCDx < minABx) return false;

  double minABy = std::min(A.second, B.second);
  double maxABy = std::max(A.second, B.second);
  double minCDy = std::min(C.second, D.second);
  double maxCDy = std::max(C.second, D.second);

  if (maxABy < minCDy || maxCDy < minABy) return false;

  // Line AB represented as a1x + b1y = c1
  double a1 = B.second - A.second;
  double b1 = A.first - B.first;
  double c1 = a1 * A.first + b1 * A.second;

  // Line CD represented as a2x + b2y = c2
  double a2 = D.second - C.second;
  double b2 = C.first - D.first;
  double c2 = a2 * C.first + b2 * C.second;

  double determinant = a1 * b2 - a2 * b1;

  if (std::abs(determinant) < EPSILON) {
    // Lines are parallel or collinear.
    // Check if collinear by testing if points from one segment lie on the other
    // line.
    if (std::abs(a1 * C.first + b1 * C.second - c1) < EPSILON &&
        std::abs(a1 * D.first + b1 * D.second - c1) < EPSILON) {
      // Segments are collinear, check for overlap
      return !(maxABx < minCDx || maxCDx < minABx || maxABy < minCDy ||
               maxCDy < minABy);
    }
    return false;  // Parallel but not collinear
  }

  // Calculate intersection point
  double x = (b2 * c1 - b1 * c2) / determinant;
  double y = (a1 * c2 - a2 * c1) / determinant;

  // Check if intersection point lies on both segments
  return (minABx <= x && x <= maxABx && minABy <= y && y <= maxABy &&
          minCDx <= x && x <= maxCDx && minCDy <= y && y <= maxCDy);
}

ShapeLandQuery::ShapeLandQuery(const std::string &filename,
                               const TileIndex &tiles, int dmod,
                               bool key_is_south_edge)
    : _filename(filename),
      _tiles(tiles),
      _dmod(dmod),
      _key_is_south_edge(key_is_south_edge) {
  static std::atomic<uint64_t> next_id{1};
  _id = next_id++;
}

bool ShapeLandQuery::CrossesLand(double lat1, double lon1, double lat2,
                                 double lon2) {
  while (lon1 < -180) lon1 += 360;
  while (lon1 > 180) lon1 -= 360;
  while (lon2 < -180) lon2 += 360;
  while (lon2 > 180) lon2 -= 360;

  // Take the short way around and split the segment at the date line
  if (lon2 - lon1 > 180) {
    lon2 -= 360;
  } else if (lon1 - lon2 > 180) {
    lon2 += 360;
  }
  if (lon2 > 180 || lon2 < -180) {
    double edge = lon2 > 180 ? 180 : -180;
    double lat = lat1 + (edge - lon1) / (lon2 - lon1) * (lat2 - lat1);
    return SegmentCrossesLand(lat1, lon1, lat, edge) ||
           SegmentCrossesLand(lat, -edge, lat2, lon2 - 2 * edge);
  }
  return SegmentCrossesLand(lat1, lon1, lat2, lon2);
}

bool ShapeLandQuery::SegmentCrossesLand(double lat1, double lon1, double lat2,
                                        double lon2) {
  LatLon A(lat1, lon1), B(lat2, lon2);
  double minlat = std::min(lat1, lat2), maxlat = std::max(lat1, lat2);
  double minlon = std::min(lon1, lon2), maxlon = std::max(lon1, lon2);

  auto tile_crosses = [&](const LatLonKey &key,
                          const std::vector<size_t> &fids) {
    auto tile = Edges(key, fids);
    const double *pts = tile->pts.data();
    const uint32_t *edges = tile->edges.data();
    int row_first = tile->Row(minlat), row_last = tile->Row(maxlat);
    int col_first = tile->Col(minlon), col_last = tile->Col(maxlon);
    for (int row = row_first; row <= row_last; row++) {
      // Only the columns the segment passes through in this row's latitude
      // band, one more on each side for rounding. The outer rows also hold
      // what lies beyond the tile edge.
      int c0 = col_first, c1 = col_last;
      if (row_first != row_last) {
        double b0 =
            row == 0 ? minlat : std::max(minlat, tile->lat0 + row * tile->dlat);
        double b1 = row == tile->n - 1
                        ? maxlat
                        : std::min(maxlat, tile->lat0 + (row + 1) * tile->dlat);
        double l0 = lon1 + (b0 - lat1) / (lat2 - lat1) * (lon2 - lon1);
        double l1 = lon1 + (b1 - lat1) / (lat2 - lat1) * (lon2 - lon1);
        c0 = std::max(col_first, tile->Col(std::min(l0, l1)) - 1);
        c1 = std::min(col_last, tile->Col(std::max(l0, l1)) + 1);
      }
      for (int col = c0; col <= c1; col++) {
        int c = row * tile->n + col;
        for (uint32_t e = tile->start[c]; e < tile->start[c + 1]; e++) {
          uint32_t p = edges[2 * e], q = edges[2 * e + 1];
          if (LineLineIntersect(A, B, LatLon(pts[2 * p], pts[2 * p + 1]),
                                LatLon(pts[2 * q], pts[2 * q + 1]))) {
            return true;
          }
        }
      }
    }
    return false;
  };

  if (_dmod == 0) {  // untiled
    auto tile = _tiles.find(LatLonKey(0, 0));
    return tile != _tiles.end() && tile_crosses(tile->first, tile->second);
  }

  // Cells by north edge (lat) and west edge (lon). A point on a cell boundary
  // is in the cell north/east of it, so there always is at least one cell.
  int d = _dmod;
  int lat_first = floor(minlat / d) * d + d;
  int lat_last = floor(maxlat / d) * d + d;
  int lon_first = floor(minlon / d) * d;
  int lon_last = floor(maxlon / d) * d;
  for (int i = lat_first; i <= lat_last; i += d) {
    for (int j = lon_first; j <= lon_last; j += d) {
      int lon = j >= 180 ? j - 360 : j;
      auto tile = _tiles.find(LatLonKey(_key_is_south_edge ? i - d : i, lon));
      if (tile != _tiles.end() && tile_crosses(tile->first, tile->second)) {
        return true;
      }
    }
  }
  return false;
}

int ShapeLandQuery::TileEdges::Row(double lat) const {
  return std::clamp(int(floor((lat - lat0) / dlat)), 0, n - 1);
}

int ShapeLandQuery::TileEdges::Col(double lon) const {
  return std::clamp(int(floor((lon - lon0) / dlon)), 0, n - 1);
}

const ShapeLandQuery::TileEdges *ShapeLandQuery::Edges(
    const LatLonKey &key, const std::vector<size_t> &fids) {
  // Each thread remembers the last few tiles it used, so the hot path takes no
  // lock and touches no shared reference count, which would otherwise bounce
  // between the cores of all routing threads.
  // ponytail: a thread keeps up to kSlots tiles alive after the cache drops
  // them, fine for a handful of routing threads.
  constexpr int kSlots = 4;
  struct Slot {
    uint64_t owner{0};
    LatLonKey key{0, 0};
    std::shared_ptr<const TileEdges> tile;
  };
  thread_local Slot slots[kSlots];
  thread_local int next_slot = 0;
  for (auto &slot : slots) {
    if (slot.owner == _id && slot.key == key) {
      return slot.tile.get();
    }
  }
  Slot &slot = slots[next_slot];
  next_slot = (next_slot + 1) % kSlots;
  slot = {_id, key, SharedEdges(key, fids)};
  return slot.tile.get();
}

std::shared_ptr<const ShapeLandQuery::TileEdges> ShapeLandQuery::SharedEdges(
    const LatLonKey &key, const std::vector<size_t> &fids) {
  {
    std::shared_lock lock(_cache_mutex);
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      return it->second;
    }
  }
  // ponytail: one tile build at a time, per-tile locks if threads starting in
  // different areas ever show up waiting here.
  std::lock_guard build_lock(_build_mutex);
  {
    std::shared_lock lock(_cache_mutex);
    auto it = _cache.find(key);
    if (it != _cache.end()) {
      return it->second;  // built by another thread while we waited
    }
  }
  if (!_reader) {
    _reader = std::make_unique<shp::ShapefileReader>(_filename);
  }

  auto tile = std::make_shared<TileEdges>();
  std::vector<uint32_t> all_edges;
  for (auto fid : fids) {
    auto const &feature = _reader->getFeature(fid);
    auto polygon = dynamic_cast<shp::Polygon *>(feature.getGeometry());
    if (!polygon) {
      continue;
    }
    for (auto &ring : polygon->getRings()) {
      auto points = ring.getPoints();
      if (points.size() < 3) {
        continue;
      }
      uint32_t first = tile->pts.size() / 2;
      for (auto &point : points) {
        tile->pts.push_back(point.getY());
        tile->pts.push_back(point.getX());
      }
      // The first edge closes the ring: from its last point to its first one
      uint32_t prev = first + points.size() - 1;
      for (uint32_t i = first; i < first + points.size(); i++) {
        all_edges.push_back(prev);
        all_edges.push_back(i);
        prev = i;
      }
    }
  }

  // Size the grid for the tile's detail, a crude ocean tile needs one cell
  // where a full resolution coast needs hundreds per side.
  int n = std::clamp(int(sqrt(all_edges.size() / 2 / kEdgesPerCell)), 1, 1024);
  if (_dmod == 0) {  // untiled, the grid covers the world
    *tile = {-90, -180, 180.0 / n, 360.0 / n, n, std::move(tile->pts)};
  } else {
    double south = _key_is_south_edge ? key.lat : key.lat - _dmod;
    *tile = {south, double(key.lon),     double(_dmod) / n, double(_dmod) / n,
             n,     std::move(tile->pts)};
  }

  // Bucket the edges by the sub-cells their bounding boxes overlap: count,
  // then fill.
  const double *pts = tile->pts.data();
  auto for_cells = [&](uint32_t p, uint32_t q, auto &&f) {
    int r0 = tile->Row(pts[2 * p]), r1 = tile->Row(pts[2 * q]);
    int c0 = tile->Col(pts[2 * p + 1]), c1 = tile->Col(pts[2 * q + 1]);
    for (int r = std::min(r0, r1); r <= std::max(r0, r1); r++) {
      for (int c = std::min(c0, c1); c <= std::max(c0, c1); c++) {
        f(r * n + c);
      }
    }
  };
  tile->start.assign(n * n + 1, 0);
  for (size_t e = 0; e < all_edges.size(); e += 2) {
    for_cells(all_edges[e], all_edges[e + 1],
              [&](int c) { tile->start[c + 1]++; });
  }
  for (int c = 0; c < n * n; c++) {
    tile->start[c + 1] += tile->start[c];
  }
  tile->edges.resize(2 * tile->start.back());
  std::vector<uint32_t> fill(tile->start.begin(), tile->start.end() - 1);
  for (size_t e = 0; e < all_edges.size(); e += 2) {
    for_cells(all_edges[e], all_edges[e + 1], [&](int c) {
      tile->edges[2 * fill[c]] = all_edges[e];
      tile->edges[2 * fill[c] + 1] = all_edges[e + 1];
      fill[c]++;
    });
  }
  tile->pts.shrink_to_fit();

  size_t bytes = tile->pts.size() * sizeof(double) +
                 (tile->start.size() + tile->edges.size()) * sizeof(uint32_t);
  std::unique_lock lock(_cache_mutex);
  // ponytail: whole cache dropped when full; tiles other threads are using
  // stay alive through their shared_ptr. Per-tile LRU if routing over a large
  // area ever thrashes here.
  if (_cache_bytes > kMaxBytes) {
    _cache.clear();
    _cache_bytes = 0;
  }
  _cache_bytes += bytes;
  _cache[key] = tile;
  return tile;
}
