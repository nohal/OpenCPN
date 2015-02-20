/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Custom dialogs
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

#ifndef __OCPN_DIALOGS_H__
#define __OCPN_DIALOGS_H__

#include <wx/dialog.h>
#include <wx/timer.h>

class  OCPNMessageDialog: public wxDialog
{

public:
    OCPNMessageDialog( wxWindow *parent, const wxString& message,
                        const wxString& caption = _T("OpenCPN"),
                        long style = wxOK|wxCENTRE, const wxPoint& pos = wxDefaultPosition );

    void OnYes( wxCommandEvent& event );
    void OnNo( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

private:
    int m_style;
    DECLARE_EVENT_TABLE()
};

class TimedMessageBox : wxEvtHandler
{
public:
    TimedMessageBox( wxWindow* parent, const wxString& message,
                    const wxString& caption = _T("OpenCPN"), long style = wxOK | wxCANCEL,
                    int timeout_sec = -1, const wxPoint& pos = wxDefaultPosition );
    ~TimedMessageBox();
    int GetRetVal( void ) { return ret_val; }
    void OnTimer( wxTimerEvent &evt );

private:
    wxTimer     m_timer;
    OCPNMessageDialog *dlg;
    int         ret_val;

    DECLARE_EVENT_TABLE()
};

int OCPNMessageBox( wxWindow *parent,
                    const wxString& message,
                    const wxString& caption = _T("OpenCPN"),
                    int style = wxOK,  int timout_sec = -1, int x = -1, int y = -1 );

#endif
