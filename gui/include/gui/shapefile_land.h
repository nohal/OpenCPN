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
 * Thread safe land crossing queries on a tiled shapefile basemap
 */

#ifndef SHAPEFILE_LAND_H
#define SHAPEFILE_LAND_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ShapefileReader.hpp"

/**
 * A latitude/longitude key for 1x1 or 10x10 degree grid tiles.
 * Used for indexing geographical data in a grid-based system where each tile
 * represents a 1x1 degree area. The lon value is the west edge of the tile,
 * the lat value is its north edge or its south edge, depending on the dataset
 * (see ShapeBaseChart::_key_is_south_edge).
 *
 * The tiles are 1x1 degree, except the crude resolution which is 10x10 to
 * minimize the number of polygons needed to draw the planetary scale basemap.
 * This logic is inherited from the workflow used by the source dataset
 * https://osmdata.openstreetmap.de/data/land-polygons.html
 */
class LatLonKey {
public:
  /**
   * Constructor for creating a LatLonKey.
   * @param lat Integer latitude value of the north or south edge of the tile.
   * @param lon Integer longitude value indicating the west edge of a longitude
   * band.
   */
  LatLonKey(int lat, int lon) {
    this->lat = lat;
    this->lon = lon;
  }
  /** Integer latitude value of the north or south edge of the tile. */
  int lat;
  /**
   * Integer longitude value representing the western (left) boundary of a
   * longitude band. For example, lon=-120 represents the area between 120°W and
   * 119°W.
   */
  int lon;

  bool operator<(const LatLonKey &k) const {
    return lat < k.lat || (lat == k.lat && lon < k.lon);
  }

  bool operator==(const LatLonKey &other) const {
    return (lat == other.lat && lon == other.lon);
  }
};

template <>
struct std::hash<LatLonKey> {
  std::size_t operator()(const LatLonKey &k) const {
    return 360 * k.lat + k.lon;
  }
};

/** Feature indexes of a shapefile grouped by tile. */
using TileIndex = std::unordered_map<LatLonKey, std::vector<size_t>>;

/**
 * Answers "does this segment cross land" for one tiled shapefile, safe to call
 * from any number of threads. The polygon rings of a tile are read from the
 * shapefile the first time a segment touches the tile and kept in memory.
 */
class ShapeLandQuery {
public:
  /**
   * @param filename Shapefile to read the polygons from.
   * @param tiles Feature indexes by tile; must outlive this object and must
   * not change while it exists.
   * @param dmod Tile size in degrees.
   * @param key_is_south_edge True if the tile key lat is the south edge.
   */
  ShapeLandQuery(const std::string &filename, const TileIndex &tiles, int dmod,
                 bool key_is_south_edge);

  /**
   * Determines if the segment between two points intersects an edge of any
   * land polygon. The segment is a straight line in lat/lon.
   * @return true if the segment crosses a coastline, false if it is entirely
   * over water or entirely over land.
   */
  bool CrossesLand(double lat1, double lon1, double lat2, double lon2);

  /** Cache size limit in bytes. */
  static constexpr size_t kMaxBytes = 128 * 1024 * 1024;
  /** Each tile's grid is sized for about this many edges per sub-cell. */
  static constexpr size_t kEdgesPerCell = 16;

private:
  /**
   * Polygon edges of one tile, indexed by a grid of sub-cells so a segment
   * only tests the edges near it, even next to a huge ring like a mainland
   * coast. The points keep their exact coordinates: the tile clip edges are
   * axis aligned and the intersection test is exact, so rounding would flip
   * results on them.
   */
  struct TileEdges {
    /** South west corner of the tile and size of its sub-cells, degrees */
    double lat0, lon0, dlat, dlon;
    /** The grid has n x n sub-cells */
    int n;
    /** lat, lon of each point */
    std::vector<double> pts;
    /**
     * Edges as pairs of point indexes, grouped by sub-cell. The edges of
     * sub-cell (row, col) are edges[2 * start[c]] .. edges[2 * start[c + 1]],
     * c = row * n + col. An edge is listed in every sub-cell its bounding
     * box overlaps.
     */
    std::vector<uint32_t> start;
    std::vector<uint32_t> edges;
    /** Sub-cell row and column of a point, clamped to the grid. */
    int Row(double lat) const;
    int Col(double lon) const;
  };

  bool SegmentCrossesLand(double lat1, double lon1, double lat2, double lon2);
  /**
   * Edges of a tile, read and cached on first use.
   * @return Valid until the calling thread's next Edges() call.
   */
  const TileEdges *Edges(const LatLonKey &key, const std::vector<size_t> &fids);
  std::shared_ptr<const TileEdges> SharedEdges(const LatLonKey &key,
                                               const std::vector<size_t> &fids);
  /** Unique per object, identifies its tiles in the per-thread memo. */
  uint64_t _id;

  std::string _filename;
  const TileIndex &_tiles;
  int _dmod;
  bool _key_is_south_edge;

  std::shared_mutex _cache_mutex;
  std::unordered_map<LatLonKey, std::shared_ptr<const TileEdges>> _cache;
  size_t _cache_bytes{0};
  /** Serializes tile builds, guards _reader. */
  std::mutex _build_mutex;
  std::unique_ptr<shp::ShapefileReader> _reader;
};

#endif
