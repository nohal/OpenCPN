/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Timed pop-up window
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

#include "TimedPopupWindow.h"

//      Auto timed popup Window implementation

BEGIN_EVENT_TABLE(TimedPopupWin, wxWindow) EVT_PAINT(TimedPopupWin::OnPaint)
EVT_TIMER(POPUP_TIMER, TimedPopupWin::OnTimer)

END_EVENT_TABLE()

// Define a constructor
TimedPopupWin::TimedPopupWin( wxWindow *parent, int timeout ) :
wxWindow( parent, wxID_ANY, wxPoint( 0, 0 ), wxSize( 1, 1 ), wxNO_BORDER )
{
    m_pbm = NULL;

    m_timer_timeout.SetOwner( this, POPUP_TIMER );
    m_timeout_sec = timeout;
    isActive = false;
    Hide();
}

TimedPopupWin::~TimedPopupWin()
{
    delete m_pbm;
}
void TimedPopupWin::OnTimer( wxTimerEvent& event )
{
    if( IsShown() )
        Hide();
}


void TimedPopupWin::SetBitmap( wxBitmap &bmp )
{
    delete m_pbm;
    m_pbm = new wxBitmap( bmp );

    // Retrigger the auto timeout
    if( m_timeout_sec > 0 )
        m_timer_timeout.Start( m_timeout_sec * 1000, wxTIMER_ONE_SHOT );
}

void TimedPopupWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    wxMemoryDC mdc;
    mdc.SelectObject( *m_pbm );
    dc.Blit( 0, 0, width, height, &mdc, 0, 0 );

}
