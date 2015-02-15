/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Printing support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 **************************************************************************/

#ifndef __OCPN_PRINTOUT_H__
#define __OCPN_PRINTOUT_H__

#include <wx/print.h>
#include <wx/bitmap.h>

class ChartCanvas;

class OCPN_Printout: public wxPrintout
{
public:
    OCPN_Printout( ChartCanvas *cc = NULL, const wxChar *title = _T("OpenCPN printout") ) : wxPrintout( title ) { m_cc = cc; }
    virtual bool OnPrintPage( int page );
    virtual bool HasPage( int page );
    virtual bool OnBeginDocument( int startPage, int endPage );
    virtual void GetPageInfo( int *minPage, int *maxPage, int *selPageFrom, int *selPageTo );
    void DrawPageOne( wxDC *dc );
    void GenerateGLbmp( void );
  
private:
    wxBitmap m_GLbmp;
    ChartCanvas *m_cc;
};

#endif
