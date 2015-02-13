/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Core infrastructure
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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#ifdef __WXMSW__
//#include "c:\\Program Files\\visual leak detector\\include\\vld.h"
#endif

#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

class OCPN_Base
{
public:
    OCPN_Base( wxStandardPaths &standard_paths );
    ~OCPN_Base();
    wxStandardPaths& GetStandardPaths() { return std_path; }
    wxString* GetHomeLocation() { return pHome_Locn; }
    wxString *newPrivateFileName( const char *name, const char *windowsName ) { return newPrivateFileName( std_path, pHome_Locn, name, windowsName ); }
    static void appendOSDirSlash( wxString* pString );

private:
    wxString *newPrivateFileName( wxStandardPaths &std_path, wxString *home_locn, const char *name, const char *windowsName );
    
    wxStandardPaths std_path;
    wxString *pHome_Locn;
};
