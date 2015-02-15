/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN private types and ENUMs
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 ***************************************************************************
 *
 *
 *
 */




#ifndef __OCPNTYPES_H__
#define __OCPNTYPES_H__

//#include "bbox.h"
//#include "OCPNRegion.h"
//class OCPNRegion;

WX_DEFINE_ARRAY_INT(int, ArrayOfInts);

/** \brief ChartType constants
 */
typedef enum ChartTypeEnum
{
      CHART_TYPE_UNKNOWN = 0,
      CHART_TYPE_DUMMY,
      CHART_TYPE_DONTCARE,
      CHART_TYPE_KAP,
      CHART_TYPE_GEO,
      CHART_TYPE_S57,
      CHART_TYPE_CM93,
      CHART_TYPE_CM93COMP,
      CHART_TYPE_PLUGIN
}_ChartTypeEnum;

/** \brief ChartFamily constants
 */
typedef enum ChartFamilyEnum
{
      CHART_FAMILY_UNKNOWN = 0,
      CHART_FAMILY_RASTER,
      CHART_FAMILY_VECTOR,
      CHART_FAMILY_DONTCARE
}_ChartFamilyEnum;

/** \brief ColorScheme constants
 */
typedef enum ColorScheme
{
      GLOBAL_COLOR_SCHEME_RGB,
      GLOBAL_COLOR_SCHEME_DAY,
      GLOBAL_COLOR_SCHEME_DUSK,
      GLOBAL_COLOR_SCHEME_NIGHT,
      N_COLOR_SCHEMES
}_ColorScheme;



//----------------------------------------------------------------------------
// ocpn Toolbar stuff
//----------------------------------------------------------------------------
class ChartBase;
class wxSocketEvent;
class ocpnToolBarSimple;


//    A generic Position Data structure
typedef struct {
    double kLat;
    double kLon;
    double kCog;
    double kSog;
    double kVar;            // Variation, typically from RMC message
    double kHdm;            // Magnetic heading
    double kHdt;            // true heading
    time_t FixTime;
    int    nSats;
} GenericPosDatEx;


/** \brief IDs for the toolbar items
 * Some of them are also used in menus
 */
typedef enum ToolIdEnum
{
    // The following constants represent the toolbar items (some are also used in menus).
    // They MUST be in the SAME ORDER as on the toolbar and new items MUST NOT be added
    // amongst them, due to the way the toolbar button visibility is saved and calculated.
    ID_ZOOMIN = 1550,
    ID_ZOOMOUT,
    ID_STKUP,
    ID_STKDN,
    ID_ROUTE,
    ID_FOLLOW,
    ID_SETTINGS,
    ID_AIS,
    ID_ENC_TEXT,
    ID_CURRENT,
    ID_TIDE,
    ID_ABOUT,
    ID_TBEXIT,
    ID_TBSTAT,
    ID_PRINT,
    ID_COLSCHEME,
    ID_ROUTEMANAGER,
    ID_TRACK,
    ID_TBSTATBOX,
    ID_MOB,

    ID_PLUGIN_BASE // This MUST be the last item in the enum
}_ToolIdEnum;


/** \brief Menu item IDs for the main menu bar
 */
typedef enum MenuIdEnum
{
    ID_MENU_ZOOM_IN = 2000,
    ID_MENU_ZOOM_OUT,
    ID_MENU_SCALE_IN,
    ID_MENU_SCALE_OUT,

    ID_MENU_NAV_FOLLOW,
    ID_MENU_NAV_TRACK,

    ID_MENU_CHART_NORTHUP,
    ID_MENU_CHART_COGUP,
    ID_MENU_CHART_QUILTING,
    ID_MENU_CHART_OUTLINES,

    ID_MENU_UI_CHARTBAR,
    ID_MENU_UI_COLSCHEME,
    ID_MENU_UI_FULLSCREEN,

    ID_MENU_ENC_TEXT,
    ID_MENU_ENC_LIGHTS,
    ID_MENU_ENC_SOUNDINGS,
    ID_MENU_ENC_ANCHOR,

    ID_MENU_SHOW_TIDES,
    ID_MENU_SHOW_CURRENTS,

    ID_MENU_TOOL_MEASURE,
    ID_MENU_ROUTE_MANAGER,
    ID_MENU_ROUTE_NEW,
    ID_MENU_MARK_BOAT,
    ID_MENU_MARK_CURSOR,
    ID_MENU_MARK_MOB,

    ID_MENU_AIS_TARGETS,
    ID_MENU_AIS_TRACKS,
    ID_MENU_AIS_CPADIALOG,
    ID_MENU_AIS_CPASOUND,
    ID_MENU_AIS_TARGETLIST,
    ID_MENU_OQUIT
}_MenuIdEnum;


/** \brief A global definition for window, timer and other ID's as needed.
 */
typedef enum GlobalIdEnum
{
    ID_NMEA_WINDOW = wxID_HIGHEST,
    ID_AIS_WINDOW,
    INIT_TIMER,
    FRAME_TIMER_1,
    FRAME_TIMER_2,
    TIMER_AIS1,
    TIMER_DSC,
    TIMER_AISAUDIO,
    AIS_SOCKET_ID,
    FRAME_TIMER_DOG,
    FRAME_TC_TIMER,
    FRAME_COG_TIMER,
    MEMORY_FOOTPRINT_TIMER,
    BELLS_TIMER,
    ID_NMEA_THREADMSG
}_GlobalIdEnum;


/** \brief A small class used in an array to describe chart directories
 * On Windows, the installer may leave initial chart directories in the registry.
 * This class is used while retrieving them.
 */
class ChartDirInfo
{
public:
    wxString    fullpath;
    wxString    magic_number;
};


/** \brief Event for sending messages OpenCPN threads
 */
class OCPN_ThreadMessageEvent: public wxEvent
{
public:
    OCPN_ThreadMessageEvent( wxEventType commandType = wxEVT_NULL, int id = 0 ) {}
    ~OCPN_ThreadMessageEvent( ) {}
    
    // accessors
    void SetSString( std::string string ) { m_string = string; }
    std::string GetSString() { return m_string; }
    
    // required for sending with wxPostEvent()
    wxEvent *Clone() const
    {
        OCPN_ThreadMessageEvent *newevent = new OCPN_ThreadMessageEvent( *this );
        newevent->m_string = this->m_string;
        return newevent;
    }
    
private:
    std::string m_string;
};

#endif
