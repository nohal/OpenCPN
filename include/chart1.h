/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
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

#ifndef __CHART1_H__
#define __CHART1_H__

#include "wx/datetime.h"
#include <wx/cmdline.h>
#include <wx/snglinst.h>
#include <wx/power.h>
#include <wx/clrpicker.h>

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#include "ocpn_types.h"
#include "viewport.h"
#include "nmea0183.h"

//----------------------------------------------------------------------------
// fwd class declarations
//----------------------------------------------------------------------------
class ChartBase;
class ocpnToolBarSimple;
class OCPN_DataStreamEvent;
class DataStream;
class AIS_Target_Data;
class ChartCanvas;
class OCPN_MsgEvent;
class options;
class Track;
class OCPN_ThreadMessageEvent;

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define TIMER_GFRAME_1 999

#define ID_QUIT         101
#define ID_CM93ZOOMG    102

//    ToolBar Constants
const int ID_TOOLBAR = 500;

static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT ;

#define N_STATUS_BAR_FIELDS_MAX     20

#define STAT_FIELD_TICK             0
#define STAT_FIELD_SOGCOG           1
#define STAT_FIELD_CURSOR_LL        2
#define STAT_FIELD_CURSOR_BRGRNG    3
#define STAT_FIELD_SCALE            4

//      Define a constant GPS signal watchdog timeout value
#define GPS_TIMEOUT_SECONDS  6

//    Define a timer value for Tide/Current updates
//    Note that the underlying data algorithms produce fresh data only every 15 minutes
//    So maybe 5 minute updates should provide sufficient oversampling
#define TIMER_TC_VALUE_SECONDS      300

#define MAX_COG_AVERAGE_SECONDS        60
#define MAX_COGSOG_FILTER_SECONDS      60

class MyApp: public wxApp
{
  public:
    bool OnInit();
    int OnExit();
    void OnInitCmdLine(wxCmdLineParser& parser);
    bool OnCmdLineParsed(wxCmdLineParser& parser);
    void OnActivateApp(wxActivateEvent& event);

#ifdef LINUX_CRASHRPT
    //! fatal exeption handling
    void OnFatalException();
#endif

#ifdef __WXMSW__
    //  Catch malloc/new fail exceptions
    //  All the rest will be caught be CrashRpt
    bool OnExceptionInMainLoop();
#endif    
    
    void TrackOff(void);
    
    wxSingleInstanceChecker *m_checker;
    
    DECLARE_EVENT_TABLE()

};

class MyFrame: public wxFrame
{
  public:
    MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size, long style);

    ~MyFrame();

    int GetApplicationMemoryUse(void);

    void OnEraseBackground(wxEraseEvent& event);
    void OnActivate(wxActivateEvent& event);
    void OnMaximize(wxMaximizeEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMove(wxMoveEvent& event);
    void OnInitTimer(wxTimerEvent& event);
    void OnFrameTimer1(wxTimerEvent& event);
    bool DoChartUpdate(void);
    void OnEvtTHREADMSG(OCPN_ThreadMessageEvent& event);
    void OnEvtOCPN_NMEA(OCPN_DataStreamEvent & event);
    void OnEvtPlugInMessage( OCPN_MsgEvent & event );
    void OnMemFootTimer(wxTimerEvent& event);
    void OnBellsTimer(wxTimerEvent& event);
#ifdef wxHAS_POWER_EVENTS
    void OnSuspending(wxPowerEvent &event);
    void OnSuspended(wxPowerEvent &event);
    void OnSuspendCancel(wxPowerEvent &event);
    void OnResume(wxPowerEvent &event);
#endif // wxHAS_POWER_EVENTS
    
    void UpdateAllFonts(void);
    void PositionConsole(void);
    void OnToolLeftClick(wxCommandEvent& event);
    void ClearRouteTool();
    void DoStackUp(void);
    void DoStackDown(void);
    void DoStackDelta( int direction );
    void DoSettings( void );
    
    void MouseEvent(wxMouseEvent& event);
    void SelectChartFromStack(int index,  bool bDir = false,  ChartTypeEnum New_Type = CHART_TYPE_DONTCARE, ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
    void SelectdbChart(int dbindex);
    void SelectQuiltRefChart(int selected_index);
    void SelectQuiltRefdbChart(int db_index, bool b_autoscale = true);

    void JumpToPosition(double lat, double lon, double scale);

    void ProcessCanvasResize(void);

    void ApplyGlobalSettings(bool bFlyingUpdate, bool bnewtoolbar);
    void RegisterGlobalMenuItems();
    void UpdateGlobalMenuItems();
    void SetChartThumbnail(int index);
    int  DoOptionsDialog();
    int  ProcessOptionsDialog(int resultFlags , options* dialog );
    void DoPrint(void);
    void LaunchLocalHelp(void);
    void StopSockets(void);
    void ResumeSockets(void);
    void TogglebFollow(void);
    void ToggleFullScreen();
    void ToggleStats();
    void SetbFollow(void);
    void ClearbFollow(void);
    void ToggleChartOutlines(void);
    void ToggleENCText(void);
    void ToggleSoundings(void);
    void ToggleRocks(void);
    bool ToggleLights( bool doToggle = true, bool temporary = false );
    void ToggleAnchor(void);
    void TrackOn(void);
    Track *TrackOff(bool do_add_point = false);
    void TrackMidnightRestart(void);
    void ToggleColorScheme();
    int GetnChartStack(void);
    void SetMenubarItemState ( int item_id, bool state );
    void SetToolbarItemState ( int tool_id, bool state );
    void SetToolbarItemBitmaps ( int tool_id, wxBitmap *bitmap, wxBitmap *bmpDisabled );
    void ToggleQuiltMode(void);
    void ToggleCourseUp(void);
    void SetQuiltMode(bool bquilt);
    bool GetQuiltMode(void);
    void UpdateControlBar(void);
    void RemoveChartFromQuilt(int dbIndex);

    void SubmergeToolbar(void);
    void SubmergeToolbarIfOverlap(int x, int y, int margin = 0);
    void SurfaceToolbar(void);
    void SetToolbarScale(void);
    
    void HandlePianoClick(int selected_index, int selected_dbIndex);
    void HandlePianoRClick(int x, int y,int selected_index, int selected_dbIndex);
    void HandlePianoRollover(int selected_index, int selected_dbIndex);
    void HandlePianoRolloverIcon(int selected_index, int selected_dbIndex);

    void PianoPopupMenu ( int x, int y, int selected_index, int selected_dbIndex );
    void OnPianoMenuDisableChart(wxCommandEvent& event);
    void OnPianoMenuEnableChart(wxCommandEvent& event);
    bool IsPianoContextMenuActive(){ return piano_ctx_menu != 0; }
    
    void SetGroupIndex(int index);

    double GetBestVPScale(ChartBase *pchart);

    ChartCanvas *GetCanvasWindow(){ return m_pchart_canvas; }
    void SetCanvasWindow(ChartCanvas *pcanv){ m_pchart_canvas = pcanv; }

    ColorScheme GetColorScheme();
    void SetAndApplyColorScheme(ColorScheme cs);

    void OnFrameTCTimer(wxTimerEvent& event);
    void OnFrameCOGTimer(wxTimerEvent& event);
    void SetupQuiltMode(void);

    void ChartsRefresh(int dbi_hint, ViewPort &vp, bool b_purge = true);

    bool CheckGroup(int igroup);
    double GetTrueOrMag(double a);
    

    void TouchAISActive(void);
    void UpdateAISTool(void);

    void ActivateAISMOBRoute( AIS_Target_Data *ptarget );
    void UpdateAISMOBRoute( AIS_Target_Data *ptarget );
    
    wxStatusBar         *m_pStatusBar;
    wxMenuBar           *m_pMenuBar;
    int                 nRoute_State;
    int                 nBlinkerTick;
    bool                m_bTimeIsSet;

    wxTimer             InitTimer;
    int                 m_iInitCount;

    wxTimer             FrameTCTimer;
    wxTimer             FrameTimer1;
    wxTimer             FrameCOGTimer;
    wxTimer             MemFootTimer;

    int                 m_BellsToPlay;
    wxTimer             BellsTimer;

    //      PlugIn support
    int GetNextToolbarToolId(){return m_next_available_plugin_tool_id;}
    void RequestNewToolbarArgEvent( wxCommandEvent & event ){ return RequestNewToolbar(); }
    void RequestNewToolbar();

    void ActivateMOB(void);
    void UpdateGPSCompassStatusBox(bool b_force_new = false);
    void UpdateRotationState( double rotation );
    
    bool UpdateChartDatabaseInplace(ArrayOfCDI &DirArray,
                                    bool b_force, bool b_prog,
                                    const wxString &ChartListFileName);

    bool                m_bdefer_resize;
    wxSize              m_defer_size;

  private:
    void ODoSetSize(void);
    void DoCOGSet(void);

        //      Toolbar support
    ocpnToolBarSimple *CreateAToolbar();
    void DestroyMyToolbar();
    void UpdateToolbar(ColorScheme cs);

    void EnableToolbar(bool newstate);

    bool CheckAndAddPlugInTool(ocpnToolBarSimple *tb);
    bool AddDefaultPositionPlugInTools(ocpnToolBarSimple *tb);
    void FilterCogSog(void);
    void SetChartUpdatePeriod(ViewPort &vp);

    void ApplyGlobalColorSchemetoStatusBar(void);
    void PostProcessNNEA(bool pos_valid, const wxString &sfixtime);

    bool ScrubGroupArray();
    wxString GetGroupName(int igroup);
    void LoadHarmonics();

    bool EvalPriority(const wxString & message, DataStream *pDS );

    int                 m_StatusBarFieldCount;

    ChartCanvas         *m_pchart_canvas;

    NMEA0183        m_NMEA0183;                 // Used to parse messages from NMEA threads

    wxDateTime       m_MMEAeventTime;
    unsigned long    m_ulLastNEMATicktime;

    wxMutex          m_mutexNMEAEvent;         // Mutex to handle static data from NMEA threads

    wxString         m_last_reported_chart_name;
    wxString         m_last_reported_chart_pubdate;

    double           COGTable[MAX_COG_AVERAGE_SECONDS];

    wxString         m_lastAISiconName;

    bool             m_toolbar_scale_tools_shown;

    //      Plugin Support
    int                 m_next_available_plugin_tool_id;

    double              m_COGFilterLast;
    double              COGFilterTable[MAX_COGSOG_FILTER_SECONDS];
    double              SOGFilterTable[MAX_COGSOG_FILTER_SECONDS];

    bool                m_bpersistent_quilt;
    int                 m_ChartUpdatePeriod;
    bool                m_last_bGPSValid;

    wxString            prev_locale;
    bool                bPrevQuilt;
    bool                bPrevFullScreenQuilt;
    bool                bPrevOGL;

    MsgPriorityHash     NMEA_Msg_Hash;
    wxString            m_VDO_accumulator;
    
    time_t              m_fixtime;
    wxMenu              *piano_ctx_menu;
    
    DECLARE_EVENT_TABLE()
};

#endif
