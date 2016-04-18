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


#include "DlgMDNSSources.h"
#include "wxServDisc.h"
#include <wx/msgdlg.h>
#include <wx/log.h>

BEGIN_EVENT_TABLE(DlgMDNSSources, wxDialog)
EVT_COMMAND  (wxID_ANY, wxServDiscNOTIFY, DlgMDNSSources::OnSDNotify)
END_EVENT_TABLE()

DlgMDNSSources::DlgMDNSSources( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer( wxVERTICAL );
    
    m_stLabel = new wxStaticText( this, wxID_ANY, _("Scanning for SignalK servers on local network, this may take a while..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_stLabel->Wrap( -1 );
    bSizerMain->Add( m_stLabel, 0, wxALL, 5 );
    
    m_lbSources = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    bSizerMain->Add( m_lbSources, 1, wxALL|wxEXPAND, 5 );
    
    wxBoxSizer* bSizerBtns;
    bSizerBtns = new wxBoxSizer( wxHORIZONTAL );
    
    m_btnScan = new wxButton( this, wxID_ANY, _("Scan for services"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizerBtns->Add( m_btnScan, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    
    bSizerBtns->Add( 0, 0, 1, wxEXPAND, 5 );
    
    m_sdbSizerBtns = new wxStdDialogButtonSizer();
    m_sdbSizerBtnsOK = new wxButton( this, wxID_OK );
    m_sdbSizerBtns->AddButton( m_sdbSizerBtnsOK );
    m_sdbSizerBtnsCancel = new wxButton( this, wxID_CANCEL );
    m_sdbSizerBtns->AddButton( m_sdbSizerBtnsCancel );
    m_sdbSizerBtns->Realize();
    
    bSizerBtns->Add( m_sdbSizerBtns, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    
    
    bSizerMain->Add( bSizerBtns, 0, wxALL|wxEXPAND, 5 );
    
    
    this->SetSizer( bSizerMain );
    this->Layout();
    
    this->Centre( wxBOTH );
    
    // Connect Events
    m_lbSources->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( DlgMDNSSources::OnSourceSelect ), NULL, this );
    m_lbSources->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( DlgMDNSSources::OnSourceDblClick ), NULL, this );
    m_btnScan->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgMDNSSources::OnScanClick ), NULL, this );
    m_sdbSizerBtnsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgMDNSSources::OnCancel ), NULL, this );
    m_sdbSizerBtnsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgMDNSSources::OnOK ), NULL, this );
    
    // mdns service scanner
    m_servscan = new wxServDisc(this, wxT("_signalk-ws._tcp.local."), QTYPE_PTR);
    
    m_selected.host = wxEmptyString;
    m_selected.self_context = wxEmptyString;
}

DlgMDNSSources::~DlgMDNSSources()
{
    // Disconnect Events
    m_lbSources->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( DlgMDNSSources::OnSourceSelect ), NULL, this );
    m_lbSources->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( DlgMDNSSources::OnSourceDblClick ), NULL, this );
    m_btnScan->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgMDNSSources::OnScanClick ), NULL, this );
    m_sdbSizerBtnsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgMDNSSources::OnCancel ), NULL, this );
    m_sdbSizerBtnsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DlgMDNSSources::OnOK ), NULL, this );
    
    delete m_servscan;
}

bool DlgMDNSSources::Scan()
{
    m_lbSources->Clear();
    m_stLabel->SetLabel( _("Scanning for SignalK servers on local network, this may take a while...") );
    delete m_servscan;
    m_servscan = new wxServDisc(this, wxT("_signalk-ws._tcp.local."), QTYPE_PTR);

    return true;
}

void DlgMDNSSources::OnScanClick( wxCommandEvent& event )
{
    Scan();
    event.Skip();
}

void DlgMDNSSources::OnSDNotify(wxCommandEvent& event)
{
    if(event.GetEventObject() == m_servscan)
    {
        wxArrayString items;
        
        // length of qeury plus leading dot
        size_t qlen =  m_servscan->getQuery().Len() + 1;
        
        std::vector<wxSDEntry> entries = m_servscan->getResults();
        std::vector<wxSDEntry>::const_iterator it;
        time_t timeout;
        for( it = entries.begin(); it != entries.end(); it++ )
        {
            wxServDisc namescan(0, it->name, QTYPE_SRV);
            timeout = 3000;
            while(!namescan.getResultCount() && timeout > 0)
            {
                wxMilliSleep(25);
                timeout-=25;
            }
            if(timeout <= 0)
            {
                wxLogError(wxString::Format( _("Timeout looking up SRV records for: %s"), it->name ) );
                return;
            }
            
            SignalKSource src;
            std::vector<wxSDEntry> entries_srv = namescan.getResults();
            std::vector<wxSDEntry>::const_iterator it_srv;
            for( it_srv = entries_srv.begin(); it_srv != entries_srv.end(); it_srv++ )
            {
                wxString hostname = it_srv->name;
                wxString port = wxString() << it_srv->port;
            
                items.Add( hostname + _T(":") + port );
                
                src.host = hostname;
                port.ToULong(&src.port);
            }
            
            wxServDisc txtscan(0, it->name, QTYPE_TXT);
            timeout = 3000;
            while(!txtscan.getResultCount() && timeout > 0)
            {
                wxMilliSleep(25);
                timeout-=25;
            }
            if(timeout <= 0)
            {
                wxLogError(wxString::Format( _("Timeout looking up TXT records for: %s"), it->name ) );
                return;
            }
            
            std::vector<wxSDEntry> entries_txt = txtscan.getResults();
            std::vector<wxSDEntry>::const_iterator it_txt;
            for( it_txt = entries_txt.begin(); it_txt != entries_txt.end(); it_txt++ )
            {
                wxString text = wxString() << it_txt->data;
                size_t l = text[0];
                size_t tl = 1;
                while( tl < text.Len() )
                {
                    wxArrayString txt_key_val = wxSplit(text.Mid(tl, l), '=');
                    tl += l;
                    l = text[tl];
                    tl += 1;
                    if( txt_key_val[0] == _T("TLS") )
                    {
                        if( txt_key_val[1] == _T("0") )
                        {
                            src.tls = false;
                            items.Last().Prepend( _T("ws://") );
                        } else {
                            src.tls = true;
                            items.Last().Prepend( _T("wss://") );
                        }
                    }
                    if( txt_key_val[0] == _T("self") )
                    {
                        src.self_context.Append(txt_key_val[1]);
                        items.Last().Append( wxString::Format(_T(" (%s)"), txt_key_val[1].c_str()) );
                    }
                }
                m_sources.push_back(src);
            }
        }
        m_stLabel->SetLabel( _("SignalK data sources detected on local network") );
        m_lbSources->Set(items);
    }
}

void DlgMDNSSources::OnSourceSelect( wxCommandEvent& event )
{
    m_selected = m_sources[ m_lbSources->GetSelection() ];
    event.Skip();
}

void DlgMDNSSources::OnSourceDblClick( wxCommandEvent& event )
{
    m_selected = m_sources[ m_lbSources->GetSelection() ];
    event.Skip();
    this->EndModal(wxID_OK);
    this->Close();
}

void DlgMDNSSources::OnOK( wxCommandEvent& event )
{
    event.Skip();
}

const SignalKSource DlgMDNSSources::GetSelectedServer()
{
    return m_selected;
}
