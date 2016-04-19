/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  mDNS SignalK sources search dialog
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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

#ifndef DlgMDNSSources_h
#define DlgMDNSSources_h

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

#ifdef __OCPN_USE_MDNS__
#include "wxServDisc.h"
#endif

///////////////////////////////////////////////////////////////////////////

#include <vector>
using namespace std;

///////////////////////////////////////////////////////////////////////////////
/// SignalK Source definition
///////////////////////////////////////////////////////////////////////////////
struct SignalKSource
{
    wxString host;
    unsigned long port;
    bool tls;
    wxString self_context;
};

///////////////////////////////////////////////////////////////////////////////
/// Class DlgMDNSSources
///////////////////////////////////////////////////////////////////////////////
class DlgMDNSSources : public wxDialog
{
private:
    bool Scan();
    void OnSDNotify(wxCommandEvent& event);
#ifdef __OCPN_USE_MDNS__
    wxServDisc *m_servscan;
#endif
    SignalKSource m_selected;
    vector<SignalKSource> m_sources;

protected:
    wxStaticText* m_stLabel;
    wxListBox* m_lbSources;
    wxButton* m_btnScan;
    wxStdDialogButtonSizer* m_sdbSizerBtns;
    wxButton* m_sdbSizerBtnsOK;
    wxButton* m_sdbSizerBtnsCancel;
    
    void OnSourceSelect( wxCommandEvent& event );
    void OnSourceDblClick( wxCommandEvent& event );
    void OnScanClick( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event ) { event.Skip(); }
    void OnOK( wxCommandEvent& event );
    DECLARE_EVENT_TABLE();
    
public:
    
    DlgMDNSSources( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("SignalK sources - zeroconf"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 450,300 ), long style = wxDEFAULT_DIALOG_STYLE );
    ~DlgMDNSSources();
    
    const SignalKSource GetSelectedServer();
    
};
#endif /* DlgMDNSSources_h */
