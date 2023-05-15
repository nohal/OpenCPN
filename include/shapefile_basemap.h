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
#include <unordered_map>
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

class WorldShapeBaseChart {
public:
  WorldShapeBaseChart();
  wxPoint2DDouble GetDoublePixFromLL(ViewPort &vp, double lat, double lon);

  void DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp, wxColor const &color);

  void DrawPolygonFilledGL(ocpnDC &pnt, int *pvc, ViewPort &vp,
                           wxColor const &color, bool idl);
  void RenderViewOnDC(ocpnDC &dc, ViewPort &vp);

private:
  shp::ShapefileReader _reader;
  std::unordered_map<LatLonKey, std::vector<size_t>> _tiles;
};

#endif
