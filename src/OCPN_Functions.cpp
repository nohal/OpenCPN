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

#include <wx/font.h>
#include "OCPN_Functions.h"
#include "FontMgr.h"
#include "chcanv.h"

extern bool g_bresponsive;
extern ChartCanvas *cc1;

wxFont *GetOCPNScaledFont( wxString item, int default_size )
{
    wxFont *dFont = FontMgr::Get().GetFont( item, default_size );

    if( g_bresponsive ){
        //      Adjust font size to be no smaller than xx mm actual size
        double scaled_font_size = dFont->GetPointSize();

        if( cc1) {
            double min_scaled_font_size = 3 * cc1->GetPixPerMM();
            int nscaled_font_size = wxMax( wxRound(scaled_font_size), min_scaled_font_size );
            wxFont *qFont = wxTheFontList->FindOrCreateFont( nscaled_font_size,
                                                             dFont->GetFamily(),
                                                             dFont->GetStyle(),
                                                             dFont->GetWeight());
            return qFont;
        }
    }
    return dFont;
}
