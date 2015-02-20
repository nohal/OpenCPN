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

#include <wx/wxprec.h>

#ifndef  WX_PRECOMP
#include <wx/wx.h>
#endif //precompiled headers

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>
#endif

#ifdef LINUX_CRASHRPT
#include "crashprint.h"
#endif

#include "OCPN_App.h"
#include "OCPN_Base.h"
#include "toolbar.h"
#include "ocpn_types.h"
#include "navutil.h"
#include "routeman.h"
#include "Select.h"
#include "AIS_Decoder.h"
#include "OCPN_Dialogs.h"
#include "OCPN_Functions.h"
#include "pluginmanager.h"
#include "chart1.h"
#include "glChartCanvas.h"
#include "chartdb.h"
#include "statwin.h"
#include "thumbwin.h"
#include "FontMgr.h"
#include "compasswin.h"
#include "tcmgr.h"
#include "Layer.h"
#include "options.h" //Just because of lang_list
#ifdef USE_S57
#include "s52plib.h"
#include "s57RegistrarMgr.h"
#include "cpl_csv.h"
#endif // USE_S57

bool g_unit_test_1;
bool g_bportable;
bool g_bdisable_opengl;
bool g_start_fullscreen;
bool g_rebuild_gl_cache;
wxDateTime g_start_time;
wxDateTime g_loglast_time;
double AnchorPointMinDist;
static unsigned int malloc_max;
int g_mem_total, g_mem_used, g_mem_initial;
wxString gExe_path;
FILE *flog;
wxLog *logger;
wxLog *Oldlogger;
wxString g_PrivateDataDir;
wxString g_SData_Locn;
wxString *pInit_Chart_Dir;
wxString glog_file;
ChartGroupArray *g_pGroupArray;
int g_GroupIndex;
wxString g_Plugin_Dir;
AIS_Decoder *g_pAIS;
OCPN_Base *g_pBASE;
wxArrayString *pMessageOnceArray;
Routeman *g_pRouteMan;
WayPointman *pWayPointMan;
MyConfig *pConfig;
Select *pSelect;
Select *pSelectTC;
Select *pSelectAIS;
AISTargetQueryDialog *g_pais_query_dialog_active;
wxString *phost_name;
wxString gConfig_File;
double g_display_size_mm;
bool b_novicemode = false;
bool g_btouch;
bool g_bresponsive;
int n_NavMessageShown;
bool g_bFirstRun;
wxLocale *plocale_def_lang;
wxString g_locale;
bool g_b_assume_azerty;
wxString g_config_version_string;
ColorScheme               global_color_scheme;
int                       g_memCacheLimit;
wxString                  *pChartListFileName;
wxString                  *pWorldMapLocation;
bool                      g_bShowOutlines;
bool                      g_bTrackDaily;
double                    g_PlanSpeed;
bool                      g_bFullScreenQuilt;
bool                      g_bQuiltEnable;
bool                      g_bskew_comp;
wxArrayString             TideCurrentDataSet;
wxDateTime                g_StartTime;
int                       g_StartTimeTZ;
IDX_entry                 *gpIDX;
int                       gpIDXn;
int                       g_nframewin_x;
int                       g_nframewin_y;
int                       g_nframewin_posx;
int                       g_nframewin_posy;
bool                      g_bframemax;
int                       g_lastClientRectx;
int                       g_lastClientRecty;
int                       g_lastClientRectw;
int                       g_lastClientRecth;
wxAuiManager              *g_pauimgr;
PlugInManager             *g_pi_manager;
double                    vLat, vLon;
double                    initial_scale_ppm;
int                       g_toolbar_x;
int                       g_toolbar_y;
double                    gLat, gLon, gCog, gSog, gHdt, gHdm, gVar;
bool                      bDBUpdateInProgress;
int                       g_restore_dbindex;
bool                      g_bopengl;
int                       gps_watchdog_timeout_ticks;
int                       sat_watchdog_timeout_ticks;
ChartDB                   *ChartData;
ChartStack                *pCurrentStack;
RoutePoint                *pAnchorWatchPoint1;
RoutePoint                *pAnchorWatchPoint2;
wxString                  g_AW1GUID;
wxString                  g_AW2GUID;
int                       gGPS_Watchdog;
bool                      bGPSValid;
bool                      g_bTrackCarryOver;
bool                      g_bDeferredStartTrack;
bool                      g_bCourseUp;
bool                      g_bShowChartBar;
int                       gHDx_Watchdog;
int                       gHDT_Watchdog;
int                       gVAR_Watchdog;
bool                      g_bHDT_Rx;
bool                      g_bVAR_Rx;

int                       gSAT_Watchdog;

int quitflag;
ocpnStyle::StyleManager* g_StyleManager;
MyFrame *gFrame;
ChartCanvas *cc1;
StatWin *stats;
ThumbWin *pthumbwin;
extern ocpnFloatingToolbarDialog *g_FloatingToolbarDialog; //created in MyFrame
ocpnFloatingCompassWindow *g_FloatingCompassDialog;
extern wxString OpenCPNVersion; //FIXME: from about.cpp, move here
extern wxString str_version_major; //about
extern wxString str_version_minor; //about
extern wxString str_version_patch; //about
extern ocpnGLOptions g_GLOptions; //glChartCanvas
extern TCMgr *ptcmgr; //created in MyFrame
extern ChartDummy *pDummyChart; //Created in MyFrame
extern LayerList *pLayerList; //Created in MyConfig
extern bool portaudio_initialized; //Belongs to OCPN_Sound
#ifdef USE_S57
s52plib *ps52plib; //Created in LoadS57 function
s57RegistrarMgr *g_pRegistrarMan;
#endif // USE_S57

#ifdef LINUX_CRASHRPT
wxCrashPrint g_crashprint;
#endif

#ifndef __WXMSW__
sigjmp_buf env;                    // the context saved by sigsetjmp();
#endif

#ifndef __WXMSW__
struct sigaction          sa_all;
struct sigaction          sa_all_old;
#endif

//-----------------------------------------------------------------------
//      Signal Handlers
//-----------------------------------------------------------------------
#ifndef __WXMSW__

//These are the signals possibly expected
//      SIGUSR1
//      Raised externally to cause orderly termination of application
//      Intended to act just like pushing the "EXIT" button

//      SIGSEGV
//      Some undefined segfault......

void catch_signals(int signo)
{
    switch(signo)
    {
        case SIGUSR1:
        quitflag++;                             // signal to the timer loop
        break;

        case SIGSEGV:
        siglongjmp(env, 1);// jump back to the setjmp() point
        break;

        case SIGTERM:
        LogMessageOnce(_T("Sigterm received"));
        gFrame->Close();
        break;

        default:
        break;
    }

}
#endif

// Memory monitor support
bool GetMemoryStatus( int *mem_total, int *mem_used )
{

#ifdef __LINUX__

//      Use filesystem /proc/pid/status to determine memory status

    unsigned long processID = wxGetProcessId();
    wxTextFile file;
    wxString file_name;

    if(mem_used)
    {
        *mem_used = 0;
        file_name.Printf(_T("/proc/%d/status"), (int)processID);
        if(file.Open(file_name))
        {
            bool b_found = false;
            wxString str;
            for ( str = file.GetFirstLine(); !file.Eof(); str = file.GetNextLine() )
            {
                wxStringTokenizer tk(str, _T(" :"));
                while ( tk.HasMoreTokens() )
                {
                    wxString token = tk.GetNextToken();
                    if(token == _T("VmRSS"))
                    {
                        wxStringTokenizer tkm(str, _T(" "));
                        wxString mem = tkm.GetNextToken();
                        long mem_extract = 0;
                        while(mem.Len())
                        {
                            mem.ToLong(&mem_extract);
                            if(mem_extract)
                            break;
                            mem = tkm.GetNextToken();
                        }

                        *mem_used = mem_extract;
                        b_found = true;
                        break;
                    }
                    else
                    break;
                }
                if(b_found)
                break;
            }
        }
    }

    if(mem_total)
    {
        *mem_total = 0;
        wxTextFile file_info;
        file_name = _T("/proc/meminfo");
        if(file_info.Open(file_name))
        {
            bool b_found = false;
            wxString str;
            for ( str = file_info.GetFirstLine(); !file_info.Eof(); str = file_info.GetNextLine() )
            {
                wxStringTokenizer tk(str, _T(" :"));
                while ( tk.HasMoreTokens() )
                {
                    wxString token = tk.GetNextToken();
                    if(token == _T("MemTotal"))
                    {
                        wxStringTokenizer tkm(str, _T(" "));
                        wxString mem = tkm.GetNextToken();
                        long mem_extract = 0;
                        while(mem.Len())
                        {
                            mem.ToLong(&mem_extract);
                            if(mem_extract)
                            break;
                            mem = tkm.GetNextToken();
                        }

                        *mem_total = mem_extract;
                        b_found = true;
                        break;
                    }
                    else
                    break;
                }
                if(b_found)
                break;
            }
        }
    }

#endif

#ifdef __WXMSW__
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    unsigned long processID = wxGetProcessId();

    if( mem_used ) {
        hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );

        if( hProcess && GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
            /*
             printf( "\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount );
             printf( "\tPeakWorkingSetSize: 0x%08X\n",
             pmc.PeakWorkingSetSize );
             printf( "\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize );
             printf( "\tQuotaPeakPagedPoolUsage: 0x%08X\n",
             pmc.QuotaPeakPagedPoolUsage );
             printf( "\tQuotaPagedPoolUsage: 0x%08X\n",
             pmc.QuotaPagedPoolUsage );
             printf( "\tQuotaPeakNonPagedPoolUsage: 0x%08X\n",
             pmc.QuotaPeakNonPagedPoolUsage );
             printf( "\tQuotaNonPagedPoolUsage: 0x%08X\n",
             pmc.QuotaNonPagedPoolUsage );
             printf( "\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage );
             printf( "\tPeakPagefileUsage: 0x%08X\n",
             pmc.PeakPagefileUsage );
             */
            *mem_used = pmc.WorkingSetSize / 1024;
        }

        CloseHandle( hProcess );
    }

    if( mem_total ) {
        MEMORYSTATUSEX statex;

        statex.dwLength = sizeof( statex );

        GlobalMemoryStatusEx( &statex );
        /*
         _tprintf (TEXT("There is  %*ld percent of memory in use.\n"),
         WIDTH, statex.dwMemoryLoad);
         _tprintf (TEXT("There are %*I64d total Kbytes of physical memory.\n"),
         WIDTH, statex.ullTotalPhys/DIV);
         _tprintf (TEXT("There are %*I64d free Kbytes of physical memory.\n"),
         WIDTH, statex.ullAvailPhys/DIV);
         _tprintf (TEXT("There are %*I64d total Kbytes of paging file.\n"),
         WIDTH, statex.ullTotalPageFile/DIV);
         _tprintf (TEXT("There are %*I64d free Kbytes of paging file.\n"),
         WIDTH, statex.ullAvailPageFile/DIV);
         _tprintf (TEXT("There are %*I64d total Kbytes of virtual memory.\n"),
         WIDTH, statex.ullTotalVirtual/DIV);
         _tprintf (TEXT("There are %*I64d free Kbytes of virtual memory.\n"),
         WIDTH, statex.ullAvailVirtual/DIV);
         */

        *mem_total = statex.ullTotalPhys / 1024;
    }
#endif

    return true;
}

#ifdef OCPN_USE_CRASHRPT

// Define the crash callback
int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{
  //  Flush log file
    if( logger)
        logger->Flush();

    return CR_CB_DODEFAULT;
}

#endif

#ifdef __WXMSW__
bool GetWindowsMonitorSize( int *w, int *h );
#endif

    
double  GetDisplaySizeMM()
{
    double ret = wxGetDisplaySizeMM().GetWidth();
    
#ifdef __WXMSW__    
    int w,h;
    if( GetWindowsMonitorSize( &w, &h) ){
        if(w > 100)             // sanity check
            ret = w;
    }
#endif
#ifdef __WXOSX__
    ret = GetMacMonitorSize();
#endif

    wxString msg;
    msg.Printf(_T("Detected display size: %d mm"), (int) ret);
    wxLogMessage(msg);
    
    return ret;
}

int ShowNavWarning()
{
    wxString msg0(
            _("\n\
OpenCPN is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied\n\
warranty of MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.\n\
See the GNU General Public License for more details.\n\n\
OpenCPN must only be used in conjunction with approved\n\
paper charts and traditional methods of navigation.\n\n\
DO NOT rely upon OpenCPN for safety of life or property.\n\n\
Please click \"OK\" to agree and proceed, \"Cancel\" to quit.\n") );

    wxString vs = wxT(" .. Version ") + str_version_major + wxT(".") + str_version_minor + wxT(".")
            + str_version_patch;

    wxMessageDialog odlg( gFrame, msg0, _("Welcome to OpenCPN") + vs, wxCANCEL | wxOK );

    return ( odlg.ShowModal() );
}


// `Main program' equivalent, creating windows and returning main app frame
//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

#ifndef OCPN_USE_WRAPPER
IMPLEMENT_APP( MyApp )
#endif

BEGIN_EVENT_TABLE(MyApp, wxApp)
EVT_ACTIVATE_APP(MyApp::OnActivateApp)
END_EVENT_TABLE()

#include "wx/dynlib.h"

#if wxUSE_CMDLINE_PARSER
void MyApp::OnInitCmdLine( wxCmdLineParser& parser )
{
    //    Add some OpenCPN specific command line options
    parser.AddSwitch( _T("unit_test_1") );
    parser.AddSwitch( _T("p") );
    parser.AddSwitch( _T("no_opengl") );
    parser.AddSwitch( _T("fullscreen") );
    parser.AddSwitch( _T("rebuild_gl_raster_cache") );
}

bool MyApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
    g_unit_test_1 = parser.Found( _T("unit_test_1") );
    g_bportable = parser.Found( _T("p") );
    g_bdisable_opengl = parser.Found( _T("no_opengl") );
    g_start_fullscreen = parser.Found( _T("fullscreen") );
    g_rebuild_gl_cache = parser.Found( _T("rebuild_gl_raster_cache") );

    return true;
}
#endif

#ifdef __WXMSW__
    //  Handle any exception not handled by CrashRpt
    //  Most probable:  Malloc/new failure
    
bool MyApp::OnExceptionInMainLoop()
{
    wxLogWarning(_T("Caught MainLoopException, continuing..."));
    return true;
}
#endif

void MyApp::OnActivateApp( wxActivateEvent& event )
{
//    Code carefully in this method.
//    It is called in some unexpected places,
//    such as on closure of dialogs, etc.

//      Activating?

#ifdef __WXOSX__

//      On the Mac, this method gets hit when...
//      a) switching between apps by clicking title bars, coming and going
//      b) un-iconizing, activeate only/
//      It does NOT get hit on iconizing the app
    if(!event.GetActive())
    {
//        printf("App de-activate\n");
        if(g_FloatingToolbarDialog) {
            if(g_FloatingToolbarDialog->IsShown())
                g_FloatingToolbarDialog->Submerge();
        }


        AppActivateList.Clear();
        if(cc1){
            for ( wxWindowList::iterator it = cc1->GetChildren().begin(); it != cc1->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    (*it)->Hide();
                    AppActivateList.Append(*it);
                }
            }
        }

        if(gFrame){
            for ( wxWindowList::iterator it = gFrame->GetChildren().begin(); it != gFrame->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    if( !(*it)->IsKindOf( CLASSINFO(ChartCanvas) ) ) {
                        (*it)->Hide();
                        AppActivateList.Append(*it);
                    }
                }
            }
        }

#if 0
        if(console && console->IsShown()) {
            console->Hide();
        }

        if(g_FloatingCompassDialog && g_FloatingCompassDialog->IsShown()) {
            g_FloatingCompassDialog->Hide();
        }

        if(stats && stats->IsShown()) {
            stats->Hide();
        }
#endif
    }
    else
    {
//        printf("App Activate\n");
        gFrame->SubmergeToolbar();              // This is needed to reset internal wxWidgets logic
                                                // Also required for other TopLevelWindows here
                                                // reportedly not required for wx 2.9
        gFrame->SurfaceToolbar();

        wxWindow *pOptions = NULL;

        wxWindowListNode *node = AppActivateList.GetFirst();
        while (node) {
            wxWindow *win = node->GetData();
            win->Show();
            if( win->IsKindOf( CLASSINFO(options) ) )
                pOptions = win;

            node = node->GetNext();
        }

#if 0
        if(g_FloatingCompassDialog){
            g_FloatingCompassDialog->Hide();
            g_FloatingCompassDialog->Show();
        }

        if(stats){
            stats->Hide();
            stats->Show();
        }

        if(console) {
            if( g_pRouteMan->IsAnyRouteActive() ){
                console->Hide();
                console->Show();
            }
        }
#endif
        if( pOptions )
            pOptions->Raise();
        else
            gFrame->Raise();

    }
#endif

    if( !event.GetActive() ) {

        //  Remove a temporary Menubar when the application goes inactive
        //  This is one way to handle properly ALT-TAB navigation on the Windows desktop
        //  without accidentally leaving an unwanted Menubar shown.
#ifdef __WXMSW__        
        if( g_bTempShowMenuBar ) {
            g_bTempShowMenuBar = false;
            if(gFrame)
                gFrame->ApplyGlobalSettings(false, false);
        }
#endif        
        
        if( g_FloatingToolbarDialog )
            g_FloatingToolbarDialog->HideTooltip(); // Hide any existing tip
    }

    event.Skip();
}

bool MyApp::OnInit()
{
    wxStopWatch sw;

    if( !wxApp::OnInit() ) return false;
    
    g_pBASE = new OCPN_Base( *dynamic_cast<wxStandardPaths*>(&wxApp::GetTraits()->GetStandardPaths()) );

    //  On Windows
    //  We allow only one instance unless the portable option is used
#ifdef __WXMSW__
    m_checker = new wxSingleInstanceChecker(_T("OpenCPN"));
    if(!g_bportable) {
        if ( m_checker->IsAnotherRunning() )
            return false;               // exit quietly
    }
#endif

#if wxCHECK_VERSION(3,0,0)
    // Set the name of the app as displayed to the user.
    // This is necessary at least on OS X, for the capitalisation to be correct in the system menus.
    MyApp::SetAppDisplayName("OpenCPN");
#endif

#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
    // Install Windows crash reporting

    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);
    info.pszAppName = _T("OpenCPN");

    wxString version_crash = str_version_major + _T(".") + str_version_minor + _T(".") + str_version_patch;
    info.pszAppVersion = version_crash.c_str();

    int type = MiniDumpWithDataSegs;  // Include the data sections from all loaded modules.
                                                // This results in the inclusion of global variables

    type |=  MiniDumpNormal;// | MiniDumpWithPrivateReadWriteMemory | MiniDumpWithIndirectlyReferencedMemory;
    info.uMiniDumpType = (MINIDUMP_TYPE)type;

    // Install all available exception handlers....
    info.dwFlags = CR_INST_ALL_POSSIBLE_HANDLERS;
    
    //  Except memory allocation failures
    info.dwFlags &= ~CR_INST_NEW_OPERATOR_ERROR_HANDLER;
    
    //  Allow user to attach files
    info.dwFlags |= CR_INST_ALLOW_ATTACH_MORE_FILES;
    
    //  Allow user to add more info
    info.dwFlags |= CR_INST_SHOW_ADDITIONAL_INFO_FIELDS;
    
    
    // URL for sending error reports over HTTP.
    if(g_bEmailCrashReport){
        info.pszEmailTo = _T("opencpn@bigdumboat.com");
        info.pszSmtpProxy = _T("mail.bigdumboat.com:587");
        info.pszUrl = _T("http://bigdumboat.com/crashrpt/ocpn_crashrpt.php");
        info.uPriorities[CR_HTTP] = 1;  // First try send report over HTTP
    }
    else{
        info.dwFlags |= CR_INST_DONT_SEND_REPORT;
        info.uPriorities[CR_HTTP] = CR_NEGATIVE_PRIORITY;       // don't send at all
    }
        
    info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;  // Second try send report over SMTP
    info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY; //1; // Third try send report over Simple MAPI

    wxStandardPaths& crash_std_path = *dynamic_cast<wxStandardPaths*>(&wxApp::GetTraits()->GetStandardPaths());
    wxString crash_rpt_save_locn = crash_std_path.GetConfigDir();
    if( g_bportable ) {
        wxFileName exec_path_crash( crash_std_path.GetExecutablePath() );
        crash_rpt_save_locn = exec_path_crash.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
    }
    
    wxString locn = crash_rpt_save_locn + _T("\\CrashReports");
    
    if(!wxDirExists( locn ) )
        wxMkdir( locn );
        
    if(wxDirExists( locn ) ){
        wxCharBuffer buf = locn.ToUTF8();
        wchar_t wlocn[256];
        if(buf && (locn.Length() < sizeof(wlocn)) ){
            MultiByteToWideChar( 0, 0, buf.data(), -1, wlocn, sizeof(wlocn)-1);
            info.pszErrorReportSaveDir = (LPCWSTR)wlocn;
        }
    }

    // Provide privacy policy URL
    wxStandardPathsBase& std_path_crash = wxApp::GetTraits()->GetStandardPaths();
    std_path_crash.Get();
    wxFileName exec_path_crash( std_path_crash.GetExecutablePath() );
    wxString policy_file =  exec_path_crash.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
    policy_file += _T("PrivacyPolicy.txt");
    policy_file.Prepend(_T("file:"));

    info.pszPrivacyPolicyURL = policy_file.c_str();;

    int nResult = crInstall(&info);
    if(nResult!=0) {
         TCHAR buff[256];
         crGetLastErrorMsg(buff, 256);
         MessageBox(NULL, buff, _T("crInstall error, Crash Reporting disabled."), MB_OK);
     }

    // Establish the crash callback function
    crSetCrashCallback( CrashCallback, NULL );

    // Take screenshot of the app window at the moment of crash
    crAddScreenshot2(CR_AS_PROCESS_WINDOWS|CR_AS_USE_JPEG_FORMAT, 95);

    //  Mark some files to add to the crash report
    wxString home_data_crash = std_path_crash.GetConfigDir();
    if( g_bportable ) {
        wxFileName f( std_path_crash.GetExecutablePath() );
        home_data_crash = f.GetPath();
    }
    g_pBASE->appendOSDirSlash( &home_data_crash );

    wxString config_crash = _T("opencpn.ini");
    config_crash.Prepend( home_data_crash );
    crAddFile2( config_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE );

    wxString log_crash = _T("opencpn.log");
    log_crash.Prepend( home_data_crash );
    crAddFile2( log_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE );

#endif
#endif

#ifdef LINUX_CRASHRPT
#if wxUSE_ON_FATAL_EXCEPTION
    // fatal exceptions handling
    wxHandleFatalExceptions (true);
#endif
#endif

#ifdef __WXMSW__
    //  Invoke my own handler for failers of malloc/new
    _set_new_handler( MyNewHandler );
    //  configure malloc to call the New failure handler on failure
    _set_new_mode(1);
#endif    
    
    //  Seed the random number generator
    wxDateTime x = wxDateTime::UNow();
    long seed = x.GetMillisecond();
    seed *= x.GetTicks();
    srand(seed);

    //    On MSW, force the entire process to run on one CPU core only
    //    This resolves some difficulty with wxThread syncronization
#if 0
#ifdef __WXMSW__
    //Gets the current process handle
    HANDLE hProc = GetCurrentProcess();
    DWORD procMask;
    DWORD sysMask;
    HANDLE hDup;
    DuplicateHandle( hProc, hProc, hProc, &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS );

//Gets the current process affinity mask
    GetProcessAffinityMask( hDup, &procMask, &sysMask );

// Take a simple approach, and assume up to 4 processors
    DWORD newMask;
    if( ( procMask & 1 ) == 1 ) newMask = 1;
    else
        if( ( procMask & 2 ) == 2 ) newMask = 2;
        else
            if( ( procMask & 4 ) == 4 ) newMask = 4;
            else
                if( ( procMask & 8 ) == 8 ) newMask = 8;

//Set te affinity mask for the process
    BOOL res = SetProcessAffinityMask( hDup, (DWORD_PTR) newMask );

    if( res == 0 ) {
        //Error setting affinity mask!!
    }
#endif
#endif

//Fulup: force floating point to use dot as separation.
// This needs to be set early to catch numerics in config file.
//#ifdef __POSIX__
    setlocale( LC_NUMERIC, "C" );
//#endif

//      CALLGRIND_STOP_INSTRUMENTATION



    g_start_time = wxDateTime::Now();

    g_loglast_time = g_start_time;   // pjotrc 2010.02.09
    g_loglast_time.MakeGMT();        // pjotrc 2010.02.09
    g_loglast_time.Subtract( wxTimeSpan( 0, 29, 0, 0 ) ); // give 1 minute for GPS to get a fix   // pjotrc 2010.02.09

    AnchorPointMinDist = 5.0;

#ifdef __WXMSW__

    //    Handle any Floating Point Exceptions which may leak thru from other
    //    processes.  The exception filter is in cutil.c
    //    Seems to only happen for W98

    wxPlatformInfo Platform;
    if( Platform.GetOperatingSystemId() == wxOS_WINDOWS_9X ) SetUnhandledExceptionFilter (&MyUnhandledExceptionFilter);
#endif

#ifdef __WXMSW__
//     _CrtSetBreakAlloc(25503);
#endif

#ifndef __WXMSW__
//      Setup Linux SIGNAL handling, for external program control

//      Build the sigaction structure
    sa_all.sa_handler = catch_signals;// point to my handler
    sigemptyset(&sa_all.sa_mask);// make the blocking set
                                 // empty, so that all
                                 // other signals will be
                                 // unblocked during my handler
    sa_all.sa_flags = 0;

    sigaction(SIGUSR1, NULL, &sa_all_old);// save existing action for this signal

//      Register my request for some signals
    sigaction(SIGUSR1, &sa_all, NULL);

    sigaction(SIGUSR1, NULL, &sa_all_old);// inspect existing action for this signal

    sigaction(SIGTERM, &sa_all, NULL);
    sigaction(SIGTERM, NULL, &sa_all_old);
#endif

//      Initialize memory tracer
#ifndef __WXMSW__
//        mtrace();
#endif

//      Here is some experimental code for wxTheme support
//      Not also these lines included above....
//      They are necessary to ensure that the themes are statically loaded

//      #ifdef __WXUNIVERSAL__
//      WX_USE_THEME(gtk);
//      WX_USE_THEME(Metal);
//      #endif

#ifdef __WXUNIVERSAL__
//        wxTheme* theme = wxTheme::Create("gtk");
//        wxTheme::Set(theme);
#endif

//      Init the private memory manager
    malloc_max = 0;

    //      Record initial memory status
    GetMemoryStatus( &g_mem_total, &g_mem_initial );

//      wxHandleFatalExceptions(true);

// Set up default FONT encoding, which should have been done by wxWidgets some time before this......
    wxFont temp_font( 10, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, wxString( _T("") ),
            wxFONTENCODING_SYSTEM );
    temp_font.SetDefaultEncoding( wxFONTENCODING_SYSTEM );

//      Establish a "home" location
    //wxStandardPaths& std_path = *dynamic_cast<wxStandardPaths*>(&wxApp::GetTraits()->GetStandardPaths());

    //TODO  Why is the following preferred?  Will not compile with gcc...
//    wxStandardPaths& std_path = wxApp::GetTraits()->GetStandardPaths();

#ifdef __unix__
    g_pBASE->GetStandardPaths().SetInstallPrefix(wxString(PREFIX, wxConvUTF8));
#endif

    gExe_path = g_pBASE->GetStandardPaths().GetExecutablePath();

#ifdef __WXMSW__
    g_pBASE->GetHomeLocation()->Append( g_pBASE->GetStandardPaths().GetConfigDir() );   // on w98, produces "/windows/Application Data"
#else
    g_pBASE->GetHomeLocation()->Append(g_pBASE->GetStandardPaths().GetUserConfigDir());
#endif

    //  On android, make the private data dir on the sdcard, if it exists.
    //  This make debugging easier, as it is not deleted whenever the APK is re-deployed.
    //  This behaviour should go away at Release.
#ifdef __OCPN__ANDROID__
    if( wxDirExists(_T("/mnt/sdcard")) ){
        g_pBASE->GetHomeLocation()->Clear();
        g_pBASE->GetHomeLocation()->Append( _T("/mnt/sdcard/.opencpn") );
    }
#endif

    if( g_bportable ) {
        g_pBASE->GetHomeLocation()->Clear();
        wxFileName f( g_pBASE->GetStandardPaths().GetExecutablePath() );
        g_pBASE->GetHomeLocation()->Append( f.GetPath() );
    }

    g_pBASE->appendOSDirSlash( g_pBASE->GetHomeLocation() );

    //      Establish Log File location
    glog_file = *g_pBASE->GetHomeLocation();

#ifdef  __WXOSX__
    g_pBASE->GetHomeLocation()->Append(_T("opencpn"));
    g_pBASE->appendOSDirSlash(g_pBASE->GetHomeLocation());

    wxFileName LibPref(glog_file);          // starts like "~/Library/Preferences"
    LibPref.RemoveLastDir();// takes off "Preferences"

    glog_file = LibPref.GetFullPath();
    g_pBASE->appendOSDirSlash(&glog_file);

    glog_file.Append(_T("Logs/"));// so, on OS X, opencpn.log ends up in ~/Library/Logs
                                  // which makes it accessible to Applications/Utilities/Console....
#endif

    // create the opencpn "home" directory if we need to
    wxFileName wxHomeFiledir( *g_pBASE->GetHomeLocation() );
    if( true != wxHomeFiledir.DirExists( wxHomeFiledir.GetPath() ) ) if( !wxHomeFiledir.Mkdir(
            wxHomeFiledir.GetPath() ) ) {
        wxASSERT_MSG(false,_T("Cannot create opencpn home directory"));
        return false;
    }

    // create the opencpn "log" directory if we need to
    wxFileName wxLogFiledir( glog_file );
    if( true != wxLogFiledir.DirExists( wxLogFiledir.GetPath() ) ) {
        if( !wxLogFiledir.Mkdir( wxLogFiledir.GetPath() ) ) {
            wxASSERT_MSG(false,_T("Cannot create opencpn log directory"));
            return false;
        }
    }
    glog_file.Append( _T("opencpn.log") );
    wxString logit = glog_file;
    
    //  Constrain the size of the log file
    wxString large_log_message;
    if( ::wxFileExists( glog_file ) ) {
        if( wxFileName::GetSize( glog_file ) > 1000000 ) {
            wxString oldlog = glog_file;
            oldlog.Append( _T(".log") );
            //  Defer the showing of this messagebox until the system locale is established.
            large_log_message = ( _("Old log will be moved to opencpn.log.log") );
            ::wxRenameFile( glog_file, oldlog );
        }
    }

#ifdef __OCPN__ANDROID__
    //  Force new logfile for each instance
    // TODO Remove this behaviour on Release
    if( ::wxFileExists( glog_file ) ){
        ::wxRemoveFile( glog_file );
    }
#endif
    
    flog = fopen( glog_file.mb_str(), "a" );
    logger = new wxLogStderr( flog );

#ifdef __OCPN__ANDROID__
    //  Trouble printing timestamp
    logger->SetTimestamp((const char *)NULL);
#endif
    
    Oldlogger = wxLog::SetActiveTarget( logger );

#ifdef __WXMSW__

//  Un-comment the following to establish a separate console window as a target for printf() in Windows
//     RedirectIOToConsole();

#endif

//        wxLog::AddTraceMask("timer");               // verbose message traces to log output

#if defined(__WXGTK__) || defined(__WXOSX__)
    logger->SetTimestamp(_T("%H:%M:%S %Z"));
#endif

//      Send init message
    wxLogMessage( _T("\n\n________\n") );

    wxDateTime date_now = wxDateTime::Now();

    wxString imsg = date_now.FormatISODate();
    wxLogMessage( imsg );

    imsg = _T(" ------- Starting OpenCPN -------");
    wxLogMessage( imsg );

    wxString version = OpenCPNVersion;
    wxString vs = version.Trim( true );
    vs = vs.Trim( false );
    wxLogMessage( vs );

    wxString wxver(wxVERSION_STRING);
    wxver.Prepend( _T("wxWidgets version: ") );
    
    wxPlatformInfo platforminfo = wxPlatformInfo::Get();
    
    wxString os_name;
#ifndef __WXQT__
    os_name = platforminfo.GetOperatingSystemIdName();
#else
    os_name = platforminfo.GetOperatingSystemFamilyName();
#endif
    
    wxString platform = os_name + _T(" ") +
    platforminfo.GetArchName()+ _T(" ") +
    platforminfo.GetPortIdName();
    
    wxLogMessage( wxver + _T(" ") + platform );

    wxLogMessage( _T("MemoryStatus:  mem_total: %d mb,  mem_initial: %d mb"), g_mem_total / 1024,
            g_mem_initial / 1024 );

    //    Initialize embedded PNG icon graphics
    ::wxInitAllImageHandlers();

//      Establish a "shared data" location
    /*  From the wxWidgets documentation...

     wxStandardPaths::GetDataDir
     wxString GetDataDir() const
     Return the location of the applications global, i.e. not user-specific, data files.
     * Unix: prefix/share/appname
     * Windows: the directory where the executable file is located
     * Mac: appname.app/Contents/SharedSupport bundle subdirectory
     */
    g_SData_Locn = g_pBASE->GetStandardPaths().GetDataDir();
    g_pBASE->appendOSDirSlash( &g_SData_Locn );

#ifdef __OCPN__ANDROID__
    wxFileName fdir = wxFileName::DirName(g_pBASE->GetStandardPaths().GetUserConfigDir());
    
    fdir.RemoveLastDir();
    g_SData_Locn = fdir.GetPath();
    g_SData_Locn += _T("/cache/");
#endif
    
    if( g_bportable )
        g_SData_Locn = *g_pBASE->GetHomeLocation();

    imsg = _T("SData_Locn is ");
    imsg += g_SData_Locn;
    wxLogMessage( imsg );

#ifdef __OCPN__ANDROID__
#if 0    
    //  Now we can load a Qt StyleSheet, if present
    wxString style_file = g_SData_Locn;
    g_pBASE->appendOSDirSlash( &style_file );
    style_file += _T("styles");
    g_pBASE->appendOSDirSlash( &style_file );
    style_file += _T("qtstylesheet.qss");
    
    if(LoadQtStyleSheet(style_file)){
        wxString smsg = _T("Loaded Qt Stylesheet: ") + style_file;
        wxLogMessage( smsg );
    }
    else
        wxLogMessage(_T("Qt Stylesheet not found"));
#endif    
#endif
        
    //      Create some static strings
    pInit_Chart_Dir = new wxString();

    //  Establish an empty ChartCroupArray
    g_pGroupArray = new ChartGroupArray;

    //      Establish the prefix of the location of user specific data files
#ifdef __WXMSW__
    g_PrivateDataDir = *g_pBASE->GetHomeLocation();                     // should be {Documents and Settings}\......
#elif defined __WXOSX__
            g_PrivateDataDir = g_pBASE->GetStandardPaths().GetUserConfigDir();     // should be ~/Library/Preferences
#else
            g_PrivateDataDir = g_pBASE->GetStandardPaths().GetUserDataDir();       // should be ~/.opencpn
#endif

    if( g_bportable )
        g_PrivateDataDir = *g_pBASE->GetHomeLocation();

#ifdef __OCPN__ANDROID__
    g_PrivateDataDir = *g_pBASE->GetHomeLocation();
#endif

    imsg = _T("PrivateDataDir is ");
    imsg += g_PrivateDataDir;
    wxLogMessage( imsg );


    //  Get the PlugIns directory location
    g_Plugin_Dir = g_pBASE->GetStandardPaths().GetPluginsDir();   // linux:   {prefix}/lib/opencpn
                                               // Mac:     appname.app/Contents/PlugIns
#ifdef __WXMSW__
    g_Plugin_Dir += _T("\\plugins");             // Windows: {exe dir}/plugins
#endif

    if( g_bportable ) {
        g_Plugin_Dir = *g_pBASE->GetHomeLocation();
        g_Plugin_Dir += _T("plugins");
    }

//      Create an array string to hold repeating messages, so they don't
//      overwhelm the log
    pMessageOnceArray = new wxArrayString;

//      Init the Route Manager
    g_pRouteMan = new Routeman( this );

    //      Init the Selectable Route Items List
    pSelect = new Select();
    pSelect->SetSelectPixelRadius( 12 );

    //      Init the Selectable Tide/Current Items List
    pSelectTC = new Select();
    //  Increase the select radius for tide/current stations
    pSelectTC->SetSelectPixelRadius( 25 );

    //      Init the Selectable AIS Target List
    pSelectAIS = new Select();
    pSelectAIS->SetSelectPixelRadius( 12 );

    g_pAIS = new AIS_Decoder();
//      Initially AIS display is always on
    g_pAIS->set_ShowAIS( true );
    g_pais_query_dialog_active = NULL;

//      Who am I?
    phost_name = new wxString( ::wxGetHostName() );

//      (Optionally) Capture the user and file(effective) ids
//  Some build environments may need root privileges for hardware
//  port I/O, as in the NMEA data input class.  Set that up here.

#ifndef __WXMSW__
#ifdef PROBE_PORTS__WITH_HELPER
    user_user_id = getuid ();
    file_user_id = geteuid ();
#endif
#endif

//      Establish the location of the config file
#ifdef __WXMSW__
    gConfig_File = _T("opencpn.ini");
    gConfig_File.Prepend( *g_pBASE->GetHomeLocation() );

#elif defined __WXOSX__
    gConfig_File = g_pBASE->GetStandardPaths().GetUserConfigDir(); // should be ~/Library/Preferences
    g_pBASE->appendOSDirSlash(&gConfig_File);
    gConfig_File.Append(_T("opencpn.ini"));
#else
    gConfig_File = g_pBASE->GetStandardPaths().GetUserDataDir(); // should be ~/.opencpn
    g_pBASE->appendOSDirSlash(&gConfig_File);
    gConfig_File.Append(_T("opencpn.conf"));
#endif

    if( g_bportable ) {
        gConfig_File = *g_pBASE->GetHomeLocation();
#ifdef __WXMSW__
        gConfig_File += _T("opencpn.ini");
#elif defined __WXOSX__
        gConfig_File +=_T("opencpn.ini");
#else
        gConfig_File += _T("opencpn.conf");
#endif

    }

#ifdef __OCPN__ANDROID__
    gConfig_File = *g_pBASE->GetHomeLocation();
    gConfig_File += _T("opencpn.conf");
#endif

    b_novicemode = false;
    
    wxFileName config_test_file_name( gConfig_File );
    if( config_test_file_name.FileExists() ) wxLogMessage(
            _T("Using existing Config_File: ") + gConfig_File );
    else {
        {
            wxLogMessage( _T("Creating new Config_File: ") + gConfig_File );

            //    Flag to preset some options for initial config file creation
            b_novicemode = true;

            if( true != config_test_file_name.DirExists( config_test_file_name.GetPath() ) )
                if( !config_test_file_name.Mkdir(config_test_file_name.GetPath() ) )
                    wxLogMessage( _T("Cannot create config file directory for ") + gConfig_File );
        }
    }

    // Now initialize UI Style.
    g_StyleManager = new ocpnStyle::StyleManager();

    if( !g_StyleManager->IsOK() ) {
        wxString msg = _("Failed to initialize the user interface. ");
        msg << _("OpenCPN cannot start. ");
        msg << _("The necessary configuration files were not found. ");
        msg << _("See the log file at ") << glog_file << _(" for details.");
        wxMessageDialog w( NULL, msg, _("Failed to initialize the user interface. "),
                wxCANCEL | wxICON_ERROR );
        w.ShowModal();
        exit( EXIT_FAILURE );
    }

    g_display_size_mm = wxMax(100, GetDisplaySizeMM());

    //      Init the WayPoint Manager (Must be after UI Style init).
    pWayPointMan = NULL;

    //      Open/Create the Config Object (Must be after UI Style init).
    MyConfig *pCF = new MyConfig( wxString( _T("") ), wxString( _T("") ), gConfig_File );
    pConfig = (MyConfig *) pCF;
    pConfig->LoadMyConfig();

    if(g_btouch){
        int SelectPixelRadius = 50;

        pSelect->SetSelectPixelRadius(SelectPixelRadius);
        pSelectTC->SetSelectPixelRadius( wxMax(25, SelectPixelRadius) );
        pSelectAIS->SetSelectPixelRadius(SelectPixelRadius);
    }


    //        Is this the first run after a clean install?
    if( !n_NavMessageShown ) g_bFirstRun = true;

    //  Now we can set the locale

    //    Manage internationalization of embedded messages
    //    using wxWidgets/gettext methodology....

//    wxLog::SetVerbose(true);            // log all messages for debugging language stuff

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
    if( lang_list[0] ) {
    };                 // silly way to avoid compiler warnings

    // Add a new prefix for search order.
#ifdef __WXMSW__
    wxString locale_location = g_SData_Locn;
    locale_location += _T("share/locale");
    wxLocale::AddCatalogLookupPathPrefix( locale_location );
#endif

    //  Get the default language info
    wxString def_lang_canonical;
    const wxLanguageInfo* languageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
    if( languageInfo ) {
        def_lang_canonical = languageInfo->CanonicalName;
        imsg = _T("System default Language:  ");
        imsg += def_lang_canonical;
        wxLogMessage( imsg );
    }

    //  For windows, installer may have left information in the registry defining the
    //  user's selected install language.
    //  If so, override the config file value and use this selection for opencpn...
#ifdef __WXMSW__
    if( g_bFirstRun ) {
        wxRegKey RegKey( wxString( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN") ) );
        if( RegKey.Exists() ) {
            wxLogMessage( _("Retrieving initial language selection from Windows Registry") );
            RegKey.QueryValue( wxString( _T("InstallerLanguage") ), g_locale );
        }
    }
#endif
    //  Find the language specified by the config file
    const wxLanguageInfo *pli = wxLocale::FindLanguageInfo( g_locale );
    wxString loc_lang_canonical;
    bool b_initok;
    plocale_def_lang = new wxLocale;

    if( pli ) {
        b_initok = plocale_def_lang->Init( pli->Language, 1 );
        // If the locale was not initialized OK, it may be that the wxstd.mo translations
        // of the wxWidgets strings is not present.
        // So try again, without attempting to load defaults wxstd.mo.
        if( !b_initok ){
            b_initok = plocale_def_lang->Init( pli->Language, 0 );
        }
        loc_lang_canonical = pli->CanonicalName;
    }

    if( !pli || !b_initok ) {
        delete plocale_def_lang;
        plocale_def_lang = new wxLocale;
        plocale_def_lang->Init( wxLANGUAGE_ENGLISH_US, 0 );
        loc_lang_canonical = wxLocale::GetLanguageInfo( wxLANGUAGE_ENGLISH_US )->CanonicalName;
    }

    imsg = _T("Opencpn language set to:  ");
    imsg += loc_lang_canonical;
    wxLogMessage( imsg );

    // Set filename without extension (example : opencpn_fr_FR)
    // i.e. : Set-up the filename needed for translation
//        wxString loc_lang_filename = _T("opencpn_") + loc_lang_canonical;
    wxString loc_lang_filename = _T("opencpn");

    // Get translation file (example : opencpn_fr_FR.mo)
    // No problem if the file doesn't exist
    // as this case is handled by wxWidgets
    if( plocale_def_lang ) plocale_def_lang->AddCatalog( loc_lang_filename );

    //    Always use dot as decimal
    setlocale( LC_NUMERIC, "C" );

//    wxLog::SetVerbose( false );           // log no more verbose messages

    //  French language locale is assumed to include the AZERTY keyboard
    //  This applies to either the system language, or to OpenCPN language selection
    if( loc_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;
    if( def_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;
#else
    wxLogMessage( _T("wxLocale support not available") );
#endif

//  Send the Welcome/warning message if it has never been sent before,
//  or if the version string has changed at all
//  We defer until here to allow for localization of the message
    if( !n_NavMessageShown || ( vs != g_config_version_string ) ) {
        if( wxID_CANCEL == ShowNavWarning() ) return false;
        n_NavMessageShown = 1;
    }

    g_config_version_string = vs;

    //  Show deferred log restart message, if it exists.
    if( !large_log_message.IsEmpty() )
        OCPNMessageBox ( NULL, large_log_message, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 5 );

    //  Validate OpenGL functionality, if selected
#ifdef ocpnUSE_GL

#ifdef __WXMSW__
    if( /*g_bopengl &&*/ !g_bdisable_opengl ) {
        wxFileName fn(std_path.GetExecutablePath());
        bool b_test_result = TestGLCanvas(fn.GetPathWithSep() );

        if( !b_test_result )
            wxLogMessage( _T("OpenGL disabled due to test app failure.") );

        g_bdisable_opengl = !b_test_result;
    }
#endif

#else
    g_bdisable_opengl = true;;
#endif

// Set default color scheme
    global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;

    // On Windows platforms, establish a default cache managment policy
    // as allowing OpenCPN a percentage of available physical memory,
    // not to exceed 1 GB
    // Note that this logic implies that Windows platforms always use
    // the memCacheLimit policy, and never use the fallback nCacheLimit policy
#ifdef __WXMSW__
    if( 0 == g_memCacheLimit )
        g_memCacheLimit = (int) ( g_mem_total * 0.5 );
    g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // math in kBytes, Max is 1 GB
#else
    if( 0 == g_memCacheLimit ){
        g_memCacheLimit = (int) ( (g_mem_total - g_mem_initial) * 0.5 );
        g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // Max is 1 GB if unspecified
    }
#endif

#ifdef __OCPN__ANDROID__
    g_memCacheLimit = 100 * 1024;
#endif

//      Establish location and name of chart database
    pChartListFileName = g_pBASE->newPrivateFileName( "chartlist.dat", "CHRTLIST.DAT" );
    
#ifdef __OCPN__ANDROID__
    pChartListFileName->Clear();
    pChartListFileName->Append(_T("chartlist.dat"));
    pChartListFileName->Prepend( *g_pBASE->GetHomeLocation() );
#endif

//      Establish guessed location of chart tree
    if( pInit_Chart_Dir->IsEmpty() ) {
        if( !g_bportable )
#ifndef __OCPN__ANDROID__
        pInit_Chart_Dir->Append( g_pBASE->GetStandardPaths().GetDocumentsDir() );
#else
        pInit_Chart_Dir->Append( _T("/mnt/sdcard") );
#endif
    }

//      Establish the GSHHS Dataset location
    pWorldMapLocation = new wxString( _T("gshhs") );
    pWorldMapLocation->Prepend( g_SData_Locn );
    pWorldMapLocation->Append( wxFileName::GetPathSeparator() );

    //  Override some config options for initial user startup with empty config file
    if( b_novicemode ) {
        g_bShowOutlines = true;

        g_pAIS->set_CPAMax_NM( 20. );
        g_pAIS->set_CPAWarn_NM( 2. );
        g_pAIS->set_TCPAMax_Mins( 30. );
        g_pAIS->set_MarkLost( true );
        g_pAIS->set_MarkLost_Mins( 8. );
        g_pAIS->set_RemoveLost( true );
        g_pAIS->set_RemoveLost_Mins( 10 );
        g_pAIS->set_ShowCOG( true);
        g_pAIS->set_ShowCOG_Mins( 6 );
        g_pAIS->set_ShowMoored( true );
        g_pAIS->set_ShowMoored_Kts( 0.2 );
        g_pAIS->set_ShowAreaNotices( false );
        g_pAIS->set_ShowRealSize( false );
        g_pAIS->set_ShowTargetName( false );
        g_pAIS->set_ShowNameScale( 100000 );
        g_bTrackDaily = false ;
        g_PlanSpeed = 6.;
        g_bFullScreenQuilt = true;
        g_bQuiltEnable = true;
        g_bskew_comp = false;
    }

    //  Check the global Tide/Current data source array
    //  If empty, preset one default (US) Ascii data source
    wxString default_tcdata =  ( g_SData_Locn + _T("tcdata") +
             wxFileName::GetPathSeparator() + _T("HARMONIC.IDX"));
    wxFileName fdefault( default_tcdata );

    if(!TideCurrentDataSet.GetCount()) {
        if( g_bportable ) {
            fdefault.MakeRelativeTo( g_PrivateDataDir );
            TideCurrentDataSet.Add( fdefault.GetFullPath() );
        }
        else
            TideCurrentDataSet.Add( default_tcdata );
    }
    else {
        wxString first_tide = TideCurrentDataSet.Item(0);
        wxFileName ft(first_tide);
        if(!ft.FileExists()){
            TideCurrentDataSet.RemoveAt(0);
            TideCurrentDataSet.Insert( default_tcdata, 0 );
        }
    }


    //  Check the global AIS alarm sound file
    //  If empty, preset default
    if(g_pAIS->AlertSoundFile().IsEmpty()) {
        wxString default_sound =  ( g_SData_Locn + _T("sounds") +
        wxFileName::GetPathSeparator() +
        _T("2bells.wav"));

        if( g_bportable ) {
            wxFileName f( default_sound );
            f.MakeRelativeTo( g_PrivateDataDir );
            g_pAIS->set_AlertSoundFile( f.GetFullPath() );
        }
        else
            g_pAIS->set_AlertSoundFile( default_sound );
    }


    g_StartTime = wxInvalidDateTime;
    g_StartTimeTZ = 1;                // start with local times
    gpIDX = NULL;
    gpIDXn = 0;

//  Set up the frame initial visual parameters
//      Default size, resized later
    wxSize new_frame_size( -1, -1 );
    int cx, cy, cw, ch;
    ::wxClientDisplayRect( &cx, &cy, &cw, &ch );

    InitializeUserColors();

    if( ( g_nframewin_x > 100 ) && ( g_nframewin_y > 100 ) && ( g_nframewin_x <= cw )
            && ( g_nframewin_y <= ch ) ) new_frame_size.Set( g_nframewin_x, g_nframewin_y );
    else
        new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );

    //  Try to detect any change in physical screen configuration
    //  This can happen when drivers are changed, for instance....
    //  and can confuse the WUI layout perspective stored in the config file.
    //  If detected, force a nominal window size and position....
    if( ( g_lastClientRectx != cx ) || ( g_lastClientRecty != cy ) || ( g_lastClientRectw != cw )
            || ( g_lastClientRecth != ch ) ) {
        new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );
        g_bframemax = false;
    }

    g_lastClientRectx = cx;
    g_lastClientRecty = cy;
    g_lastClientRectw = cw;
    g_lastClientRecth = ch;

    //  Validate config file position
    wxPoint position( 0, 0 );
    wxSize dsize = wxGetDisplaySize();

#ifdef __WXMAC__
    g_nframewin_posy = wxMax(g_nframewin_posy, 22);
#endif

    if( ( g_nframewin_posx < dsize.x ) && ( g_nframewin_posy < dsize.y ) ) position = wxPoint(
            g_nframewin_posx, g_nframewin_posy );

#ifdef __WXMSW__
    //  Support MultiMonitor setups which an allow negative window positions.
    RECT frame_rect;
    frame_rect.left = position.x;
    frame_rect.top = position.y;
    frame_rect.right = position.x + new_frame_size.x;
    frame_rect.bottom = position.y + new_frame_size.y;

    //  If the requested frame window does not intersect any installed monitor,
    //  then default to simple primary monitor positioning.
    if( NULL == MonitorFromRect( &frame_rect, MONITOR_DEFAULTTONULL ) ) position = wxPoint( 10,
            10 );
#endif

#ifdef __OCPN__ANDROID__
    ::wxDisplaySize( &cw, &ch);
    ch -= 24;                           // This accounts for an error in the wxQT-Android interface...
    
    if((cw > 200) && (ch > 200) )
        new_frame_size.Set( cw, ch );
    else
        new_frame_size.Set( 800, 500 );
#endif
        
    //  For Windows and GTK, provide the expected application Minimize/Close bar
    long app_style = wxDEFAULT_FRAME_STYLE;
    app_style |= wxWANTS_CHARS;

// Create the main frame window
    wxString myframe_window_title = wxT("OpenCPN ") + str_version_major + wxT(".")
            + str_version_minor + wxT(".") + str_version_patch; //Gunther

    if( g_bportable ) {
        myframe_window_title += _(" -- [Portable(-p) executing from ");
        myframe_window_title += *g_pBASE->GetHomeLocation();
        myframe_window_title += _T("]");
    }

    gFrame = new MyFrame( NULL, myframe_window_title, position, new_frame_size, app_style ); //Gunther

//  Initialize the Plugin Manager
    g_pi_manager = new PlugInManager( gFrame );

    g_pauimgr = new wxAuiManager;
//        g_pauidockart= new wxAuiDefaultDockArt;
//        g_pauimgr->SetArtProvider(g_pauidockart);

// tell wxAuiManager to manage the frame
    g_pauimgr->SetManagedWindow( gFrame );

//      Create Children of Frame
//              n.b.  if only one child exists, wxWindows expands the child
//                        to the parent client area automatically, (as a favor?)
//                        Here, we'll do explicit sizing on SIZE events

    cc1 = new ChartCanvas( gFrame );                         // the chart display canvas
    gFrame->SetCanvasWindow( cc1 );

    cc1->SetDisplaySizeMM(g_display_size_mm);
    
    cc1->SetQuiltMode( g_bQuiltEnable );                     // set initial quilt mode
    cc1->m_bFollow = pConfig->st_bFollow;               // set initial state
    cc1->SetViewPoint( vLat, vLon, initial_scale_ppm, 0., 0. );

    gFrame->Enable();

    cc1->SetFocus();

    pthumbwin = new ThumbWin( cc1 );

    gFrame->ApplyGlobalSettings( false, false );               // done once on init with resize

    g_toolbar_x = wxMax(g_toolbar_x, 0);
    g_toolbar_y = wxMax(g_toolbar_y, 0);

    g_toolbar_x = wxMin(g_toolbar_x, cw);
    g_toolbar_y = wxMin(g_toolbar_y, ch);

    gFrame->SetToolbarScale();

    //  The position and size of the static frame children (i.e. the canvas, and the status bar) are now set
    //  So now we can establish the AUI panes for them.
    //  It is important to have set the chartcanvas and status bar sizes before this point,
    //  so that the pane.BestSize values are correctly captured by the AuiManager.

    g_pauimgr->AddPane( cc1 );
    g_pauimgr->GetPane( cc1 ).Name( _T("ChartCanvas") );
    g_pauimgr->GetPane( cc1 ).Fixed();
    g_pauimgr->GetPane( cc1 ).CaptionVisible( false );
    g_pauimgr->GetPane( cc1 ).CenterPane();
    g_pauimgr->GetPane( cc1 ).BestSize( cc1->GetSize() );

// Show the frame

//    gFrame->ClearBackground();
    gFrame->Show( TRUE );

    gFrame->SetAndApplyColorScheme( global_color_scheme );

    if( g_bframemax ) gFrame->Maximize( true );

    if( g_bresponsive  && ( cc1->GetPixPerMM() > 4.0))
        gFrame->Maximize( true );

    stats = new StatWin( cc1 );
    stats->SetColorScheme( global_color_scheme );
    stats->Show();
    
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    if( cc1->GetQuiltMode() ) {
        stats->pPiano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        stats->pPiano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );

        stats->pPiano->SetRoundedRectangles( true );
    }
    stats->pPiano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
    stats->pPiano->SetPolyIcon( new wxBitmap( style->GetIcon( _T("polyprj") ) ) );
    stats->pPiano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

    //  Yield to pick up the OnSize() calls that result from Maximize()
    Yield();

    wxString perspective;
    pConfig->SetPath( _T ( "/AUI" ) );
    pConfig->Read( _T ( "AUIPerspective" ), &perspective );

    // Make sure the perspective saved in the config file is "reasonable"
    // In particular, the perspective should have an entry for every
    // windows added to the AUI manager so far.
    // If any are not found, then use the default layout

    bool bno_load = false;
    wxAuiPaneInfoArray pane_array_val = g_pauimgr->GetAllPanes();

    for( unsigned int i = 0; i < pane_array_val.GetCount(); i++ ) {
        wxAuiPaneInfo pane = pane_array_val.Item( i );
        if( perspective.Find( pane.name ) == wxNOT_FOUND ) {
            bno_load = true;
            break;
        }
    }

    if( !bno_load ) g_pauimgr->LoadPerspective( perspective, false );


    g_pauimgr->Update();
    
    bool b_SetInitialPoint = false;

    //   Build the initial chart dir array
    ArrayOfCDI ChartDirArray;
    pConfig->LoadChartDirArray( ChartDirArray );

    //  Windows installer may have left hints regarding the initial chart dir selection
#ifdef __WXMSW__
    if( g_bFirstRun ) {
        int ndirs = 0;

        wxRegKey RegKey( wxString( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN") ) );
        if( RegKey.Exists() ) {
            wxLogMessage( _("Retrieving initial Chart Directory set from Windows Registry") );
            wxString dirs;
            RegKey.QueryValue( wxString( _T("ChartDirs") ), dirs );

            wxStringTokenizer tkz( dirs, _T(";") );
            while( tkz.HasMoreTokens() ) {
                wxString token = tkz.GetNextToken();

                ChartDirInfo cdi;
                cdi.fullpath = token.Trim();
                cdi.magic_number = _T("");

                ChartDirArray.Add( cdi );
                ndirs++;
            }

        }

        if( ndirs ) pConfig->UpdateChartDirs( ChartDirArray );

        //    As a favor to new users, poll the database and
        //    move the initial viewport so that a chart will come up.
        if( ndirs ) b_SetInitialPoint = true;

    }
#endif

//    If the ChartDirArray is empty at this point, any existing chart database file must be declared invalid,
//    So it is best to simply delete it if present.
//    TODO  There is a possibility of recreating the dir list from the database itself......

    if( !ChartDirArray.GetCount() ) ::wxRemoveFile( *pChartListFileName );

//      Try to load the current chart list Data file
    ChartData = new ChartDB( gFrame );
    if (!ChartData->LoadBinary(*pChartListFileName, ChartDirArray)) {
        bDBUpdateInProgress = true;

        if( ChartDirArray.GetCount() ) {
//              Create and Save a new Chart Database based on the hints given in the config file

            /*
             wxString msg1(_("OpenCPN needs to update the chart database from config file entries...."));

             OCPNMessageDialog mdlg(gFrame, msg1, wxString(_("OpenCPN Info")),wxICON_INFORMATION | wxOK );
             int dlg_ret;
             dlg_ret = mdlg.ShowModal();
             */
            delete ChartData;
            ChartData = new ChartDB( gFrame );

            wxString line( _("Rebuilding chart database from configuration file entries...") );
            /* The following 3 strings are embeded in wxProgressDialog but must be included by xgettext
             * to be localized properly. See {wxWidgets}src/generic/progdlgg.cpp:190 */
            wxString dummy1 = _("Elapsed time : ");
            wxString dummy2 = _("Estimated time : ");
            wxString dummy3 = _("Remaining time : ");
            wxProgressDialog *pprog = new wxProgressDialog( _("OpenCPN Chart Update"), line, 100,
                    NULL,
                    wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );

            ChartData->Create( ChartDirArray, pprog );
            ChartData->SaveBinary(*pChartListFileName);

            delete pprog;
        }

        else            // No chart database, no config hints, so bail to Options....
        {
            wxLogMessage(
                    _T("Chartlist file not found, config chart dir array is empty.  Chartlist target file is:")
                            + *pChartListFileName );

            wxString msg1(
                    _("No Charts Installed.\nPlease select chart folders in Options > Charts.") );

            OCPNMessageBox(gFrame, msg1, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK );

            gFrame->DoOptionsDialog();

            b_SetInitialPoint = true;

        }

        bDBUpdateInProgress = false;

        //    As a favor to new users, poll the database and
        //    move the initial viewport so that a chart will come up.

        if( b_SetInitialPoint ) {
            double clat, clon;
            if( ChartData->GetCentroidOfLargestScaleChart( &clat, &clon, CHART_FAMILY_RASTER ) ) {
                gLat = clat;
                gLon = clon;
                gFrame->ClearbFollow();
            } else {
                if( ChartData->GetCentroidOfLargestScaleChart( &clat, &clon,
                        CHART_FAMILY_VECTOR ) ) {
                    gLat = clat;
                    gLon = clon;
                    gFrame->ClearbFollow();
                }
            }
        }

    }

    //  Verify any saved chart database startup index
    if(g_restore_dbindex >= 0){
        if(g_restore_dbindex > (ChartData->GetChartTableEntries()-1))
            g_restore_dbindex = 0;
    }

    //  Apply the inital Group Array structure to the chart data base
    ChartData->ApplyGroupArray( g_pGroupArray );

    //  Make sure that the Selected Group is sensible...
    if( g_GroupIndex > (int) g_pGroupArray->GetCount() ) g_GroupIndex = 0;
    if( !gFrame->CheckGroup( g_GroupIndex ) ) g_GroupIndex = 0;

    pCurrentStack = new ChartStack;

    //  A useability enhancement....
    //  if the chart database is truly empty on startup, switch to SCMode
    //  so that the GSHHS chart will at least be shown
    if( ChartData && ( 0 == ChartData->GetChartTableEntries() ) ) {
        cc1->SetQuiltMode( false );
        gFrame->SetupQuiltMode();
    }

//      All set to go.....

    // Process command line option to rebuild cache
#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;

    if(g_rebuild_gl_cache && g_bopengl &&
        g_GLOptions.m_bTextureCompression && g_GLOptions.m_bTextureCompressionCaching ) {

        cc1->ReloadVP();                  //  Get a nice chart background loaded

        //      Turn off the toolbar as a clear signal that the system is busy right now.
        // Note: I commented this out because the toolbar never comes back for me
        // and is unusable until I restart opencpn without generating the cache
//        if( g_FloatingToolbarDialog )
//            g_FloatingToolbarDialog->Hide();

        BuildCompressedCache();

    }
#endif


//      establish GPS timeout value as multiple of frame timer
//      This will override any nonsense or unset value from the config file
    if( ( gps_watchdog_timeout_ticks > 60 ) || ( gps_watchdog_timeout_ticks <= 0 ) ) gps_watchdog_timeout_ticks =
            ( GPS_TIMEOUT_SECONDS * 1000 ) / TIMER_GFRAME_1;

    wxString dogmsg;
    dogmsg.Printf( _T("GPS Watchdog Timeout is: %d sec."), gps_watchdog_timeout_ticks );
    wxLogMessage( dogmsg );

    sat_watchdog_timeout_ticks = 12;

    gGPS_Watchdog = 2;
    gHDx_Watchdog = 2;
    gHDT_Watchdog = 2;
    gSAT_Watchdog = 2;
    gVAR_Watchdog = 2;

    //  Most likely installations have no ownship heading information
    g_bHDT_Rx = false;
    g_bVAR_Rx = false;

//  Start up a new track if enabled in config file
    if( g_bTrackCarryOver )
        g_bDeferredStartTrack = true;

//    Re-enable anchor watches if set in config file
    if( !g_AW1GUID.IsEmpty() ) {
        pAnchorWatchPoint1 = pWayPointMan->FindRoutePointByGUID( g_AW1GUID );
    }
    if( !g_AW2GUID.IsEmpty() ) {
        pAnchorWatchPoint2 = pWayPointMan->FindRoutePointByGUID( g_AW2GUID );
    }

    Yield();

    gFrame->DoChartUpdate();

    FontMgr::Get().ScrubList(); // is this needed?

//    g_FloatingToolbarDialog->LockPosition(false);

//      Start up the ticker....
    gFrame->FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

//      Start up the ViewPort Rotation angle Averaging Timer....
    if(g_bCourseUp)
        gFrame->FrameCOGTimer.Start( 10, wxTIMER_CONTINUOUS );

//        gFrame->MemFootTimer.Start(wxMax(g_MemFootSec * 1000, 60 * 1000), wxTIMER_CONTINUOUS);
//        gFrame->MemFootTimer.Start(1000, wxTIMER_CONTINUOUS);

    // Import Layer-wise any .gpx files from /Layers directory
    wxString layerdir = g_PrivateDataDir;
    g_pBASE->appendOSDirSlash( &layerdir );
    layerdir.Append( _T("layers") );

#if 0
    wxArrayString file_array;
    g_LayerIdx = 0;

    if( wxDir::Exists( layerdir ) ) {
        wxString laymsg;
        laymsg.Printf( wxT("Getting .gpx layer files from: %s"), layerdir.c_str() );
        wxLogMessage( laymsg );

        wxDir dir;
        dir.Open( layerdir );
        if( dir.IsOpened() ) {
            wxString filename;
            layerdir.Append( wxFileName::GetPathSeparator() );
            bool cont = dir.GetFirst( &filename );
            while( cont ) {
                filename.Prepend( layerdir );
                wxFileName f( filename );
                if( f.GetExt().IsSameAs( wxT("gpx") ) ) pConfig->ImportGPX( gFrame, true, filename,
                        false ); // preload a single-gpx-file layer
                else
                    pConfig->ImportGPX( gFrame, true, filename, true ); // preload a layer from subdirectory
                cont = dir.GetNext( &filename );
            }
        }
    }
#endif

    if( wxDir::Exists( layerdir ) ) {
        wxString laymsg;
        laymsg.Printf( wxT("Getting .gpx layer files from: %s"), layerdir.c_str() );
        wxLogMessage( laymsg );

        pConfig->LoadLayers(layerdir);
    }

    cc1->ReloadVP();                  // once more, and good to go

    //  Some window managers get confused about z-order of Compass Window, and other windows not children of gFrame.
    //  We need to defer their creation until here.
    if( pConfig->m_bShowCompassWin ) {
        g_FloatingCompassDialog = new ocpnFloatingCompassWindow( cc1 );
        if( g_FloatingCompassDialog ) g_FloatingCompassDialog->UpdateStatus( true );
    }

    gFrame->Refresh( false );
    gFrame->Raise();

    gFrame->RequestNewToolbar();
#ifdef __WXQT__
    g_FloatingToolbarDialog->Raise();
#endif
//    g_FloatingToolbarDialog->Show();

    cc1->Enable();
    cc1->SetFocus();

    //  This little hack fixes a problem seen with some UniChrome OpenGL drivers
    //  We need a deferred resize to get glDrawPixels() to work right.
    //  So we set a trigger to generate a resize after 5 seconds....
    //  See the "UniChrome" hack elsewhere
#ifdef ocpnUSE_GL
    if ( !g_bdisable_opengl )
    {
        glChartCanvas *pgl = (glChartCanvas *) cc1->GetglCanvas();
        if( pgl && ( pgl->GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) )
        {
            gFrame->m_defer_size = gFrame->GetSize();
            gFrame->SetSize( gFrame->m_defer_size.x - 10, gFrame->m_defer_size.y );
            g_pauimgr->Update();
            gFrame->m_bdefer_resize = true;
        }
    }
#endif

    if ( g_start_fullscreen )
        gFrame->ToggleFullScreen();

    stats->Show( g_bShowChartBar );

    gFrame->Raise();
    cc1->Enable();
    cc1->SetFocus();

    // Perform delayed initialization after 50 milliseconds
    gFrame->InitTimer.Start( 50, wxTIMER_CONTINUOUS );

    wxLogMessage( wxString::Format(_("OpenCPN Initialized in %ld ms."), sw.Time() ) );
    
    return TRUE;
}

int MyApp::OnExit()
{
    wxLogMessage( _T("opencpn::MyApp starting exit.") );
    
    //  Send current nav status data to log file   // pjotrc 2010.02.09

    wxDateTime lognow = wxDateTime::Now();
    lognow.MakeGMT();
    wxString day = lognow.FormatISODate();
    wxString utc = lognow.FormatISOTime();
    wxString navmsg = _T("LOGBOOK:  ");
    navmsg += day;
    navmsg += _T(" ");
    navmsg += utc;
    navmsg += _T(" UTC ");

    if( bGPSValid ) {
        wxString data;
        data.Printf( _T("OFF: Lat %10.5f Lon %10.5f "), gLat, gLon );
        navmsg += data;

        wxString cog;
        if( wxIsNaN(gCog) ) cog.Printf( _T("COG ----- ") );
        else
            cog.Printf( _T("COG %10.5f "), gCog );

        wxString sog;
        if( wxIsNaN(gSog) ) sog.Printf( _T("SOG -----  ") );
        else
            sog.Printf( _T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed( gSog ) );

        navmsg += cog;
        navmsg += sog;

    } else {
        wxString data;
        data.Printf( _T("OFF: Lat %10.5f Lon %10.5f"), gLat, gLon );
        navmsg += data;
    }
    wxLogMessage( navmsg );
    g_loglast_time = lognow;

    if( ptcmgr ) delete ptcmgr;

    delete pConfig;
    delete pSelect;
    delete pSelectTC;
    delete pSelectAIS;

#ifdef USE_S57
    delete ps52plib;
#endif

    if(g_pGroupArray){
        for(unsigned int igroup = 0; igroup < g_pGroupArray->GetCount(); igroup++){
            delete g_pGroupArray->Item(igroup);
        }
            
        g_pGroupArray->Clear();
        delete g_pGroupArray;
    }
    
    delete pDummyChart;

    wxLogMessage( _T("opencpn::MyApp exiting cleanly...\n") );
    wxLog::FlushActive();
    
    if( logger ) {
        wxLog::SetActiveTarget( Oldlogger );
        delete logger;
    }

    delete pChartListFileName;
    delete phost_name;
    delete pInit_Chart_Dir;
    delete pWorldMapLocation;

    delete g_pRouteMan;
    delete pWayPointMan;

    delete pMessageOnceArray;

    DeInitializeUserColors();

    delete pLayerList;

#ifdef USE_S57
    delete g_pRegistrarMan;
    CSVDeaccess( NULL );
#endif

    delete g_StyleManager;

#ifdef USE_S57
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
    if(s_glu_dll_ready)
    FreeLibrary(s_hGLU_DLL);           // free the glu32.dll
#endif
#endif
#endif
#endif

#ifdef OCPN_USE_PORTAUDIO
    if(portaudio_initialized)
        Pa_Terminate();
#endif


    //      Restore any changed system colors
#ifdef __WXMSW__
    RestoreSystemColors();
#endif

#ifdef __MSVC__LEAK
    DeInitAllocCheck();
#endif

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
    delete plocale_def_lang;
#endif    

    FontMgr::Shutdown();

#ifdef __WXMSW__
    delete m_checker;
#endif

#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
    // Uninstall Windows crash reporting
    crUninstall();
#endif
#endif

    return TRUE;
}

#ifdef LINUX_CRASHRPT
void MyApp::OnFatalException () {
    g_crashprint.Report();
}
#endif

void MyApp::TrackOff( void )
{
    if( gFrame ) gFrame->TrackOff();
}
