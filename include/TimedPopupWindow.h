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


#include "timers.h"

//----------------------------------------------------------------------------
// Generic Auto Timed Window
// Belongs to the creator, not deleted automatically on application close
//----------------------------------------------------------------------------

class TimedPopupWin: public wxWindow
{
public:
    TimedPopupWin( wxWindow *parent, int timeout = -1 );
    ~TimedPopupWin();
    
    void OnPaint( wxPaintEvent& event );
    
    void SetBitmap( wxBitmap &bmp );
    wxBitmap* GetBitmap() { return m_pbm; }
    void OnTimer( wxTimerEvent& event );
    bool IsActive() { return isActive; }
    void IsActive( bool state ) { isActive = state; }
    
private:
    wxBitmap *m_pbm;
    wxTimer m_timer_timeout;
    int m_timeout_sec;
    bool isActive;
    
    DECLARE_EVENT_TABLE()
};

