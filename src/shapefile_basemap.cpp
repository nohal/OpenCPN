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

#include "shapefile_basemap.h"
#include "shaders.h"
#include "chartbase.h"

#ifdef __WXMSW__
#define __CALL_CONVENTION  //__stdcall
#else
#define __CALL_CONVENTION
#endif

typedef union {
  GLdouble data[6];
  struct sGLvertex {
    GLdouble x;
    GLdouble y;
    GLdouble z;
    GLdouble r;
    GLdouble g;
    GLdouble b;
  } info;
} GLvertexshp;
#include <list>

static std::list<float_2Dpt> g_pvshp;
static std::list<GLvertexshp *> g_vertexesshp;
static int g_typeshp, g_posshp;
static float_2Dpt g_p1shp, g_p2shp;

void __CALL_CONVENTION shpscombineCallback(GLdouble coords[3],
                                           GLdouble *vertex_data[4],
                                           GLfloat weight[4],
                                           GLdouble **dataOut) {
  GLvertexshp *vertex;

  vertex = new GLvertexshp();
  g_vertexesshp.push_back(vertex);

  vertex->info.x = coords[0];
  vertex->info.y = coords[1];

  *dataOut = vertex->data;
}

void __CALL_CONVENTION shpserrorCallback(GLenum errorCode) {
  const GLubyte *estring;
  estring = gluErrorString(errorCode);
  // wxLogMessage( _T("OpenGL Tessellation Error: %s"), estring );
}

void __CALL_CONVENTION shpsbeginCallback(GLenum type) {
  switch (type) {
    case GL_TRIANGLES:
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
      g_typeshp = type;
      break;
    default:
      printf("tess unhandled begin type: %d\n", type);
  }

  g_posshp = 0;
}

void __CALL_CONVENTION shpsendCallback() {}

void __CALL_CONVENTION shpsvertexCallback(GLvoid *arg) {
  GLvertexshp *vertex;
  vertex = (GLvertexshp *)arg;
  float_2Dpt p;
  p.y = vertex->info.x;
  p.x = vertex->info.y;

  // convert strips and fans into triangles
  if (g_typeshp != GL_TRIANGLES) {
    if (g_posshp > 2) {
      g_pvshp.push_back(g_p1shp);
      g_pvshp.push_back(g_p2shp);
    }

    if (g_typeshp == GL_TRIANGLE_STRIP)
      g_p1shp = g_p2shp;
    else if (g_posshp == 0)
      g_p1shp = p;
    g_p2shp = p;
  }

  g_pvshp.push_back(p);
  g_posshp++;
}

WorldShapeBaseChart::WorldShapeBaseChart() {
  LoadBasemaps("/home/nohal/source/shapefiles/data");
}

wxPoint2DDouble WorldShapeBaseChart::GetDoublePixFromLL(ViewPort &vp,
                                                        double lat,
                                                        double lon) {
  wxPoint2DDouble p = vp.GetDoublePixFromLL(lat, lon);
  p.m_x -= vp.rv_rect.x, p.m_y -= vp.rv_rect.y;
  return p;
}

void BaseMapQuality::DoDrawPolygonFilled(ocpnDC &pnt, ViewPort &vp,
                                         const shp::Feature &feature) {
  double old_x = -9999999.0, old_y = -9999999.0;
  auto polygon = static_cast<shp::Polygon *>(feature.getGeometry());
  for (auto &ring : polygon->getRings()) {
    wxPoint *poly_pt = new wxPoint[ring.getPoints().size()];
    size_t cnt{0};
    for (auto &point : ring.getPoints()) {
      // if (bbox.ContainsMarge(point.getY(), point.getX(), 0.5)) {
      wxPoint2DDouble q = WorldShapeBaseChart::GetDoublePixFromLL(
          vp, point.getY(), point.getX());
      if (round(q.m_x) != round(old_x) || round(q.m_y) != round(old_y)) {
        poly_pt[cnt].x = round(q.m_x);
        poly_pt[cnt].y = round(q.m_y);
        cnt++;
      }
      old_x = q.m_x;
      old_y = q.m_y;
      //}
    }
    if (cnt > 1) {
      pnt.DrawPolygonTessellated(cnt, poly_pt, 0, 0);
    }
    delete[] poly_pt;
  }
}

void BaseMapQuality::DrawPolygonFilled(ocpnDC &pnt, ViewPort &vp,
                                       wxColor const &color) {
  if (!_is_usable) {
    return;
  }
  if (!_reader) {
    if (!LoadSHP()) {
      return;
    }
  }
  pnt.SetBrush(color);

  LLBBox bbox = vp.GetBBox();
  if (_is_tiled) {
    for (int i = floor(bbox.GetMinLat()); i < ceil(bbox.GetMaxLat()); i++) {
      for (int j = floor(bbox.GetMinLon()); j < ceil(bbox.GetMaxLon()); j++) {
        for (auto fid : _tiles[LatLonKey(i, j)]) {
          auto const &feature = _reader->getFeature(fid);
          DoDrawPolygonFilled(pnt, vp,
                              feature);  // Parallelize using std::async?
        }
      }
    }
  } else {
    for (auto const &feature : *_reader) {
      DoDrawPolygonFilled(pnt, vp, feature);  // Parallelize using std::async?
    }
  }
}

void BaseMapQuality::DrawPolygonFilledGL(ocpnDC &pnt, int *pvc, ViewPort &vp,
                                         wxColor const &color, bool idl) {
#if 0
  // build the contour vertex array converted to normalized coordinates (if
  // needed)
  if (!*pv) {
    for (unsigned int c = 0; c < p->size(); c++) {
      if (!p->at(c).size()) continue;

      contour &cp = p->at(c);

      GLUtesselator *tobj = gluNewTess();

      gluTessCallback(tobj, GLU_TESS_VERTEX, (_GLUfuncptr)&shpsvertexCallback);
      gluTessCallback(tobj, GLU_TESS_BEGIN, (_GLUfuncptr)&shpsbeginCallback);
      gluTessCallback(tobj, GLU_TESS_END, (_GLUfuncptr)&shpsendCallback);
      gluTessCallback(tobj, GLU_TESS_COMBINE,
                      (_GLUfuncptr)&shpscombineCallback);
      gluTessCallback(tobj, GLU_TESS_ERROR, (_GLUfuncptr)&shpserrorCallback);

      gluTessNormal(tobj, 0, 0, 1);
      gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

      gluTessBeginPolygon(tobj, NULL);
      gluTessBeginContour(tobj);

      for (unsigned int v = 0; v < p->at(c).size(); v++) {
        wxRealPoint &ccp = cp.at(v);

        if (v == 0 || ccp != cp.at(v - 1)) {
          GLvertexshp *vertex = new GLvertexshp();
          g_vertexesshp.push_back(vertex);

          wxPoint2DDouble q;
          if (/*TODO glChartCanvas::HasNormalizedViewPort(vp)*/ true)
            q = GetDoublePixFromLL(vp, ccp.y, ccp.x);
          else  // tesselation directly from lat/lon
            q.m_x = ccp.y, q.m_y = ccp.x;

          if (vp.m_projection_type != PROJECTION_POLAR) {
            // need to correctly pick +180 or -180 longitude for projections
            // that have a discontiguous date line

            if (idl && ccp.x == 180) {
              if (vp.m_projection_type == PROJECTION_MERCATOR ||
                  vp.m_projection_type == PROJECTION_EQUIRECTANGULAR)
                q.m_x -=
                    40058986 * 4096.0;  // 360 degrees in normalized viewport
              else
                q.m_x -= 360;  // lat/lon coordinates
            }
          }

          vertex->info.x = q.m_x;
          vertex->info.y = q.m_y;

          gluTessVertex(tobj, (GLdouble *)vertex, (GLdouble *)vertex);
        }
      }

      gluTessEndContour(tobj);
      gluTessEndPolygon(tobj);
      gluDeleteTess(tobj);

      for (std::list<GLvertexshp *>::iterator it = g_vertexesshp.begin();
           it != g_vertexesshp.end(); it++)
        delete *it;
      g_vertexesshp.clear();
    }

    *pv = new float_2Dpt[g_pvshp.size()];
    int i = 0;
    for (std::list<float_2Dpt>::iterator it = g_pvshp.begin(); it != g_pvshp.end();
         it++)
      (*pv)[i++] = *it;

    *pvc = g_pvshp.size();
    g_pvshp.clear();

  }

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)


  GLuint vbo = 0;

  //  Build the shader viewport transform matrix
  mat4x4 m, mvp;
  mat4x4_identity(m);
  mat4x4_scale_aniso(mvp, m, 2.0 / (float)vp.pix_width,
                     2.0 / (float)vp.pix_height, 1.0);
  mat4x4_translate_in_place(mvp, -vp.pix_width / 2, vp.pix_height / 2, 0);

  if (/*TODO glChartCanvas::HasNormalizedViewPort(vp)*/ true) {

  } else {
    float *pvt = new float[2 * (*pvc)];
    for (int i = 0; i < *pvc; i++) {
      float_2Dpt *pc = *pv + i;
      wxPoint2DDouble q = vp.GetDoublePixFromLL(pc->y, pc->x);
      pvt[i * 2] = q.m_x;
      pvt[(i * 2) + 1] = q.m_y;
    }

    GLShaderProgram *shader = pcolor_tri_shader_program[pnt.m_canvasIndex];
    shader->Bind();

    float colorv[4];
    colorv[0] = color.Red() / float(256);
    colorv[1] = color.Green() / float(256);
    colorv[2] = color.Blue() / float(256);
    colorv[3] = 1.0;
    shader->SetUniform4fv("color", colorv);

    shader->SetAttributePointerf("position", pvt);

    glDrawArrays(GL_TRIANGLES, 0, *pvc);

    delete[] pvt;
    glDeleteBuffers(1, &vbo);
    shader->UnBind();
  }

#else
#endif
#endif  // 0
}

void WorldShapeBaseChart::RenderViewOnDC(ocpnDC &dc, ViewPort &vp) {
  SelectBaseMap(vp.chart_scale)->RenderViewOnDC(dc, vp);
}