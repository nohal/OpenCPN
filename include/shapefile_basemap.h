/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Shapefile basemap
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 */

#ifndef SHAPEFILE_BASEMAP_H
#define SHAPEFILE_BASEMAP_H

#include <functional>
#include <vector>
#include <map>
#include <thread>
#include <future>
#include <filesystem>
#include "ShapefileReader.hpp"
#include "poly_math.h"
#include "ocpndc.h"

class LatLonKey {
public:
  LatLonKey(int lat, int lon) {
    this->lat = lat;
    this->lon = lon;
  }
  int lat;
  int lon;

  bool operator<(const LatLonKey &k) const {
    if (this->lat < k.lat) {
      return this->lon < k.lon;
    }
    return this->lat < k.lat;
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

enum Quality { crude, low, medium, high, full };

class BaseMapQuality {
public:
  BaseMapQuality() = delete;
  BaseMapQuality(const std::string &filename, const size_t &min_scale, const wxColor & color = *wxBLACK)
      : _loading(false), _is_usable(false), _is_tiled(false),
        _min_scale(min_scale),
        _filename(filename),
        _reader(nullptr),_color(color)
         {
    _is_usable = std::filesystem::exists(filename);
    std::cout << filename << " " << _is_usable << std::endl;
  }

  BaseMapQuality(const BaseMapQuality &t)
  {
    this->_filename = t._filename;
    this->_is_usable = t._is_usable;
    this->_is_tiled = t._is_tiled;
    this->_min_scale = t._min_scale;
    this->_reader = nullptr;
    this->_color = t._color;
    std::cout << "Copy" << std::endl;
  }
  ~BaseMapQuality() { std::cout << "Destroy BaseMapQuality" << std::endl; delete _reader; }

  bool LoadSHP() {
    _reader = new shp::ShapefileReader(_filename);
    auto bounds = _reader->getBounds();
    _is_usable = _reader->getCount() > 1 && bounds.getMaxX() <= 180 &&
                 bounds.getMinX() >= -180 && bounds.getMinY() >= -90 &&
                 bounds.getMaxY() <=
                     90;  // TODO - Do we care whether the planet is covered?
    _is_usable &= _reader->getGeometryType() == shp::GeometryType::Polygon;
    bool has_x = false;
    bool has_y = false;
    for (auto field : _reader->getFields()) {
      if (field.getName() == "x") {
        has_x = true;
      } else if (field.getName() == "y") {
        has_y = true;
      }
    }
    _is_tiled = (has_x && has_y);
    if (_is_usable && _is_tiled) {
      size_t feat{0};
      for (auto const &feature : *_reader) {
        _tiles[LatLonKey(std::any_cast<int>(feature.getAttributes()["y"]),
                         std::any_cast<int>(feature.getAttributes()["x"]))]
            .push_back(feat);
        feat++;
      }
    }
    return _is_usable;
  }
  bool IsUsable() { return _is_usable && !_loading; }
  size_t MinScale() { return _min_scale; }
  void RenderViewOnDC(ocpnDC &dc, ViewPort &vp) {
    if (!dc.GetDC()) {
      // TODO: DrawPolygonFilledGL
    } else {
      DrawPolygonFilled(dc, vp, _color);
    }
  }
  static const std::string ConstructPath(const std::string &dir,
                                         const std::string &quality_suffix) {
    return dir + std::filesystem::path::preferred_separator + "basemap_" +
           quality_suffix + ".shp";
  }

private:
  std::future<bool> _loaded;
  bool _loading;
  bool _is_usable;
  bool _is_tiled;
  size_t _min_scale;
  void DoDrawPolygonFilled(ocpnDC &pnt, ViewPort &vp,
                           const shp::Feature &feature);
  void DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp, wxColor const &color);
  void DrawPolygonFilledGL(ocpnDC &pnt, int *pvc, ViewPort &vp,
                           wxColor const &color, bool idl);
  std::string _filename;
  shp::ShapefileReader *_reader;
  std::unordered_map<LatLonKey, std::vector<size_t>> _tiles;
  wxColor _color;
};

class WorldShapeBaseChart {
public:
  WorldShapeBaseChart();
  ~WorldShapeBaseChart() {}
  static wxPoint2DDouble GetDoublePixFromLL(ViewPort &vp, double lat,
                                            double lon);

  void DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp, wxColor const &color);

  void DrawPolygonFilledGL(ocpnDC &pnt, int *pvc, ViewPort &vp,
                           wxColor const &color, bool idl);
  void RenderViewOnDC(ocpnDC &dc, ViewPort &vp);
  BaseMapQuality& LowestQualityBaseMap() {
    if (_basemap_map.find(Quality::crude) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::crude)];
    }
    if (_basemap_map.find(Quality::low) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::low)];
    }
    if (_basemap_map.find(Quality::medium) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::medium)];
    }
    if (_basemap_map.find(Quality::high) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::high)];
    }
    return _basemaps[_basemap_map.at(Quality::full)];
  }

  BaseMapQuality& HighestQualityBaseMap() {
    if (_basemap_map.find(Quality::full) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::full)];
    }
    if (_basemap_map.find(Quality::high) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::high)];
    }
    if (_basemap_map.find(Quality::medium) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::medium)];
    }
    if (_basemap_map.find(Quality::low) != _basemap_map.end()) {
      return _basemaps[_basemap_map.at(Quality::low)];
    }
    return _basemaps[_basemap_map.at(Quality::crude)];
  }

  BaseMapQuality& SelectBaseMap(const size_t &scale) {
    if (_basemap_map.find(Quality::full) != _basemap_map.end() &&
        _basemaps[_basemap_map.at(Quality::full)].IsUsable() &&
        scale <= _basemaps[_basemap_map.at(Quality::full)].MinScale()) {
      return _basemaps[_basemap_map.at(Quality::full)];
    } else if (_basemap_map.find(Quality::high) != _basemap_map.end() &&
               _basemaps[_basemap_map.at(Quality::high)].IsUsable() &&
               scale <= _basemaps[_basemap_map.at(Quality::high)].MinScale()) {
      return _basemaps[_basemap_map.at(Quality::high)];
    } else if (_basemap_map.find(Quality::medium) != _basemap_map.end() &&
               _basemaps[_basemap_map.at(Quality::medium)].IsUsable() &&
               scale <= _basemaps[_basemap_map.at(Quality::medium)].MinScale()) {
      return _basemaps[_basemap_map.at(Quality::medium)];
    } else if (_basemap_map.find(Quality::low) != _basemap_map.end() &&
               _basemaps[_basemap_map.at(Quality::low)].IsUsable() &&
               scale <= _basemaps[_basemap_map.at(Quality::low)].MinScale()) {
      return _basemaps[_basemap_map.at(Quality::low)];
    }
    return LowestQualityBaseMap();
  }
  bool IsUsable() { return _basemaps.size() > 0; }

private:
  void LoadBasemaps(const std::string &dir) {
    if (std::filesystem::exists(BaseMapQuality::ConstructPath(dir, "crude"))) {
      _basemaps.push_back(BaseMapQuality(
          BaseMapQuality::ConstructPath(dir, "crude"), 300000000, *wxBLUE));
      _basemap_map.insert(std::make_pair(Quality::crude, _basemaps.size()-1));
    }
    if (std::filesystem::exists(BaseMapQuality::ConstructPath(dir, "low"))) {
      _basemaps.push_back(
          BaseMapQuality(BaseMapQuality::ConstructPath(dir, "low"), 20000000, *wxRED));
      _basemap_map.insert(std::make_pair(Quality::low, _basemaps.size()-1));
    }
    if (std::filesystem::exists(BaseMapQuality::ConstructPath(dir, "medium"))) {
      _basemaps.push_back(BaseMapQuality(
          BaseMapQuality::ConstructPath(dir, "medium"), 1000000, *wxGREEN));
      _basemap_map.insert(std::make_pair(Quality::medium, _basemaps.size()-1));
    }
    if (std::filesystem::exists(BaseMapQuality::ConstructPath(dir, "high"))) {
      _basemaps.push_back(
          BaseMapQuality(BaseMapQuality::ConstructPath(dir, "high"), 300000, *wxCYAN));
      _basemap_map.insert(std::make_pair(Quality::high, _basemaps.size()-1));
    }
    if (std::filesystem::exists(BaseMapQuality::ConstructPath(dir, "full"))) {
      _basemaps.push_back(
          BaseMapQuality(BaseMapQuality::ConstructPath(dir, "full"), 100000, *wxLIGHT_GREY));
      _basemap_map.insert(std::make_pair(Quality::full, _basemaps.size()-1));
    }
  }
  std::map<Quality, size_t> _basemap_map;
  std::vector<BaseMapQuality> _basemaps;
};

#endif
