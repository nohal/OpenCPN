/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Global helper functions
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

#ifndef __OCPN_FUNCTIONS_H__
#define __OCPN_FUNCTIONS_H__

#include <wx/arrstr.h>
#include <wx/colour.h>
#include "OCPN_Dialogs.h"
#include "ocpn_types.h"

#ifdef USE_S57
#include "cpl_error.h"

//    Global Static error reporting function
extern "C" void MyCPLErrorHandler( CPLErr eErrClass, int nError,
                             const char * pszErrorMsg );

void LoadS57();
#endif

wxFont *GetOCPNScaledFont( wxString item, int default_size = 0 );

wxArrayString *EnumerateSerialPorts(void);

wxColour GetGlobalColor(wxString colorName);

int GetApplicationMemoryUse(void);

// Helper to create menu label + hotkey string when registering menus
wxString _menuText(wxString name, wxString shortcut);

// The point for anchor watch should really be a class...
double AnchorDistFix( double const d, double const AnchorPointMinDist, double const AnchorPointMaxDist);   //  pjotrc 2010.02.22

bool TestGLCanvas(wxString &prog_dir);

bool ReloadLocale();

//    Some static helpers
void appendOSDirSlash( wxString* pString );
void InitializeUserColors( void );
void DeInitializeUserColors( void );
void SetSystemColors( ColorScheme cs );
extern "C" bool CheckSerialAccess( void );

#endif
