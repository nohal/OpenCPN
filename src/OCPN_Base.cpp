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

#include "OCPN_Base.h"

extern bool g_bportable;

OCPN_Base::OCPN_Base(wxStandardPaths &standard_paths)
{
    //      Establish a "home" location
    std_path = standard_paths;
    pHome_Locn = new wxString();
}

OCPN_Base::~OCPN_Base()
{
    delete pHome_Locn;
}

wxString *OCPN_Base::newPrivateFileName(wxStandardPaths &std_path, wxString *home_locn, const char *name, const char *windowsName)
{
    wxString fname = wxString::FromUTF8(name);
    wxString fwname = wxString::FromUTF8(windowsName);
    wxString *filePathAndName;

#ifdef __WXMSW__
    filePathAndName = new wxString( fwname );
    filePathAndName->Prepend( *pHome_Locn );

#else
    filePathAndName = new wxString(_T(""));
    filePathAndName->Append(std_path.GetUserDataDir());
    appendOSDirSlash(filePathAndName);
    filePathAndName->Append( fname );
#endif

    if( g_bportable ) {
        filePathAndName->Clear();
#ifdef __WXMSW__
        filePathAndName->Append( fwname );
#else
        filePathAndName->Append( fname );
#endif
        filePathAndName->Prepend( *home_locn );
    }
    return filePathAndName;
}

void OCPN_Base::appendOSDirSlash( wxString* pString )
{
    wxChar sep = wxFileName::GetPathSeparator();
    if( pString->Last() != sep ) pString->Append( sep );
}
