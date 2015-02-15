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
#ifdef USE_S57
#include "s52plib.h"
#include "s52s57.h"
#include "s57RegistrarMgr.h"
#endif // USE_S57
#include "OCPN_Base.h"
#include "navutil.h"

extern bool g_bresponsive;
extern ChartCanvas *cc1;

extern s52plib *ps52plib;
extern wxColorHashMap *pcurrent_user_color_hash;

extern wxArrayPtrVoid *UserColorTableArray;
extern wxArrayPtrVoid *UserColourHashTableArray;

extern wxLocale *plocale_def_lang;
extern wxString g_locale;
extern wxArrayString    g_locale_catalog_array;
extern wxString         g_csv_locn;
extern wxString         g_SData_Locn;
extern bool g_bportable;
extern OCPN_Base *g_pBASE;
extern wxString g_SENCPrefix;
extern wxString g_PrivateDataDir;
extern wxString g_UserPresLibData;
extern s57RegistrarMgr *g_pRegistrarMan; //FIXME: seems totally unused
extern FILE *flog; //FIXME: dtto
extern bool b_novicemode;
extern MyConfig *pConfig;

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

//---------------------------------------------------------------------------------------
//
//        GPS Positioning Device Detection
//
//---------------------------------------------------------------------------------------

/*
 *     Enumerate all the serial ports on the system
 *
 *     wxArrayString *EnumerateSerialPorts(void)

 *     Very system specific, unavoidably.
 */

#ifdef __WXGTK__
extern "C" int wait(int *);                     // POSIX wait() for process

#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#endif

// ****************************************
// Fulup devices selection with scandir
// ****************************************

// reserve 4 pattern for plugins
char* devPatern[] = {
  NULL,NULL,NULL,NULL,
  NULL,NULL,NULL,NULL, (char*)-1};


// This function allow external plugin to search for a special device name
// ------------------------------------------------------------------------
int paternAdd (const char* patern) {
  int ind;

  // snan table for a free slot inside devpatern table
  for (ind=0; devPatern[ind] != (char*)-1; ind++)
       if (devPatern[ind] == NULL) break;

  // table if full
  if  (devPatern [ind] == (char*) -1) return -1;

  // store a copy of the patern in case calling routine had it on its stack
  devPatern [ind]  = strdup (patern);
  return 0;
}

#ifdef __WXGTK__
// This filter verify is device is withing searched patern and verify it is openable
// -----------------------------------------------------------------------------------
int paternFilter (const struct dirent * dir) {
 char* res = NULL;
 char  devname [255];
 int   fd, ind;

  // search if devname fits with searched paterns
  for (ind=0; devPatern [ind] != (char*)-1; ind++) {
     if (devPatern [ind] != NULL) res=(char*)strcasestr(dir->d_name,devPatern [ind]);
     if (res != NULL) break;
  }

  // File does not fit researched patern
  if (res == NULL) return 0;

  // Check if we may open this file
  snprintf (devname, sizeof (devname), "/dev/%s", dir->d_name);
  fd = open(devname, O_RDWR|O_NDELAY|O_NOCTTY);

  // device name is pointing to a real device
  if(fd > 0) {
    close (fd);
    return 1;
  }

  // file is not valid
  perror (devname);
  return 0;
}

int isTTYreal(const char *dev)
{
    struct serial_struct serinfo;
    int ret = 0;

    int fd = open(dev, O_RDWR | O_NONBLOCK | O_NOCTTY);

    // device name is pointing to a real device
    if(fd >= 0) {
        if (ioctl(fd, TIOCGSERIAL, &serinfo)==0) {
            // If device type is no PORT_UNKNOWN we accept the port
            if (serinfo.type != PORT_UNKNOWN)
                ret = 1;
        }
        close (fd);
    }

    return ret;
}


#endif

wxArrayString *EnumerateSerialPorts( void )
{
    wxArrayString *preturn = new wxArrayString;

#ifdef __WXGTK__

    //Initialize the pattern table
    if( devPatern[0] == NULL ) {
        paternAdd ( "ttyUSB" );
        paternAdd ( "ttyACM" );
        paternAdd ( "ttyGPS" );
        paternAdd ( "refcom" );
    }

 //  Looking for user privilege openable devices in /dev
 //  Fulup use scandir to improve user experience and support new generation of AIS devices.

      wxString sdev;
      int ind, fcount;
      struct dirent **filelist = {0};

      // scan directory filter is applied automatically by this call
      fcount = scandir("/dev", &filelist, paternFilter, alphasort);

      for(ind = 0; ind < fcount; ind++)  {
       wxString sdev (filelist[ind]->d_name, wxConvUTF8);
       sdev.Prepend (_T("/dev/"));

       preturn->Add (sdev);
       free(filelist[ind]);
      }


//        We try to add a few more, arbitrarily, for those systems that have fixed, traditional COM ports

    if( isTTYreal("/dev/ttyS0") )
        preturn->Add( _T("/dev/ttyS0") );

    if( isTTYreal("/dev/ttyS1") )
        preturn->Add( _T("/dev/ttyS1") );


#endif

#ifdef PROBE_PORTS__WITH_HELPER

    /*
     *     For modern Linux/(Posix??) systems, we may use
     *     the system files /proc/tty/driver/serial
     *     and /proc/tty/driver/usbserial to identify
     *     available serial ports.
     *     A complicating factor is that most (all??) linux
     *     systems require root privileges to access these files.
     *     We will use a helper program method here, despite implied vulnerability.
     */

    char buf[256]; // enough to hold one line from serial devices list
    char left_digit;
    char right_digit;
    int port_num;
    FILE *f;

    pid_t pID = vfork();

    if (pID == 0)// child
    {
//    Temporarily gain root privileges
        seteuid(file_user_id);

//  Execute the helper program
        execlp("ocpnhelper", "ocpnhelper", "-SB", NULL);

//  Return to user privileges
        seteuid(user_user_id);

        wxLogMessage(_T("Warning: ocpnhelper failed...."));
        _exit(0);// If exec fails then exit forked process.
    }

    wait(NULL);                  // for the child to quit

//    Read and parse the files

    /*
     * see if we have any traditional ttySx ports available
     */
    f = fopen("/var/tmp/serial", "r");

    if (f != NULL)
    {
        wxLogMessage(_T("Parsing copy of /proc/tty/driver/serial..."));

        /* read in each line of the file */
        while(fgets(buf, sizeof(buf), f) != NULL)
        {
            wxString sm(buf, wxConvUTF8);
            sm.Prepend(_T("   "));
            sm.Replace(_T("\n"), _T(" "));
            wxLogMessage(sm);

            /* if the line doesn't start with a number get the next line */
            if (buf[0] < '0' || buf[0] > '9')
            continue;

            /*
             * convert digits to an int
             */
            left_digit = buf[0];
            right_digit = buf[1];
            if (right_digit < '0' || right_digit > '9')
            port_num = left_digit - '0';
            else
            port_num = (left_digit - '0') * 10 + right_digit - '0';

            /* skip if "unknown" in the string */
            if (strstr(buf, "unknown") != NULL)
            continue;

            /* upper limit of 15 */
            if (port_num > 15)
            continue;

            /* create string from port_num  */

            wxString s;
            s.Printf(_T("/dev/ttyS%d"), port_num);

            /*  add to the output array  */
            preturn->Add(wxString(s));

        }

        fclose(f);
    }

    /*
     * Same for USB ports
     */
    f = fopen("/var/tmp/usbserial", "r");

    if (f != NULL)
    {
        wxLogMessage(_T("Parsing copy of /proc/tty/driver/usbserial..."));

        /* read in each line of the file */
        while(fgets(buf, sizeof(buf), f) != NULL)
        {

            wxString sm(buf, wxConvUTF8);
            sm.Prepend(_T("   "));
            sm.Replace(_T("\n"), _T(" "));
            wxLogMessage(sm);

            /* if the line doesn't start with a number get the next line */
            if (buf[0] < '0' || buf[0] > '9')
            continue;

            /*
             * convert digits to an int
             */
            left_digit = buf[0];
            right_digit = buf[1];
            if (right_digit < '0' || right_digit > '9')
            port_num = left_digit - '0';
            else
            port_num = (left_digit - '0') * 10 + right_digit - '0';

            /* skip if "unknown" in the string */
            if (strstr(buf, "unknown") != NULL)
            continue;

            /* upper limit of 15 */
            if (port_num > 15)
            continue;

            /* create string from port_num  */

            wxString s;
            s.Printf(_T("/dev/ttyUSB%d"), port_num);

            /*  add to the output array  */
            preturn->Add(wxString(s));

        }

        fclose(f);
    }

    //    As a fallback, in case seteuid doesn't work....
    //    provide some defaults
    //    This is currently the case for GTK+, which
    //    refuses to run suid.  sigh...

    if(preturn->IsEmpty())
    {
        preturn->Add( _T("/dev/ttyS0"));
        preturn->Add( _T("/dev/ttyS1"));
        preturn->Add( _T("/dev/ttyUSB0"));
        preturn->Add( _T("/dev/ttyUSB1"));
        preturn->Add( _T("/dev/ttyACM0"));
        preturn->Add( _T("/dev/ttyACM1"));
    }

//    Clean up the temporary files created by helper.
    pid_t cpID = vfork();

    if (cpID == 0)// child
    {
//    Temporarily gain root privileges
        seteuid(file_user_id);

//  Execute the helper program
        execlp("ocpnhelper", "ocpnhelper", "-U", NULL);

//  Return to user privileges
        seteuid(user_user_id);
        _exit(0);// If exec fails then exit forked process.
    }

#endif      // __WXGTK__
#ifdef __WXOSX__
#include "macutils.h"
    char* paPortNames[MAX_SERIAL_PORTS];
    int iPortNameCount;

    memset(paPortNames,0x00,sizeof(paPortNames));
    iPortNameCount = FindSerialPortNames(&paPortNames[0],MAX_SERIAL_PORTS);
    for (int iPortIndex=0; iPortIndex<iPortNameCount; iPortIndex++)
    {
        wxString sm(paPortNames[iPortIndex], wxConvUTF8);
        preturn->Add(sm);
        free(paPortNames[iPortIndex]);
    }
#endif      //__WXOSX__
#ifdef __WXMSW__
    /*************************************************************************
     * Windows provides no system level enumeration of available serial ports
     * There are several ways of doing this.
     *
     *************************************************************************/

#include <windows.h>

    //    Method 1:  Use GetDefaultCommConfig()
    // Try first {g_nCOMPortCheck} possible COM ports, check for a default configuration
    //  This method will not find some Bluetooth SPP ports
    for( int i = 1; i < g_nCOMPortCheck; i++ ) {
        wxString s;
        s.Printf( _T("COM%d"), i );

        COMMCONFIG cc;
        DWORD dwSize = sizeof(COMMCONFIG);
        if( GetDefaultCommConfig( s.fn_str(), &cc, &dwSize ) )
            preturn->Add( wxString( s ) );
    }

#if 0
    // Method 2:  Use FileOpen()
    // Try all 255 possible COM ports, check to see if it can be opened, or if
    // not, that an expected error is returned.

    BOOL bFound;
    for (int j=1; j<256; j++)
    {
        char s[20];
        sprintf(s, "\\\\.\\COM%d", j);

        // Open the port tentatively
        BOOL bSuccess = FALSE;
        HANDLE hComm = ::CreateFile(s, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

        //  Check for the error returns that indicate a port is there, but not currently useable
        if (hComm == INVALID_HANDLE_VALUE)
        {
            DWORD dwError = GetLastError();

            if (dwError == ERROR_ACCESS_DENIED ||
                    dwError == ERROR_GEN_FAILURE ||
                    dwError == ERROR_SHARING_VIOLATION ||
                    dwError == ERROR_SEM_TIMEOUT)
            bFound = TRUE;
        }
        else
        {
            bFound = TRUE;
            CloseHandle(hComm);
        }

        if (bFound)
        preturn->Add(wxString(s));
    }
#endif

    // Method 3:  WDM-Setupapi
    //  This method may not find XPort virtual ports,
    //  but does find Bluetooth SPP ports

    GUID *guidDev = (GUID*) &GUID_CLASS_COMPORT;

    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

    hDevInfo = SetupDiGetClassDevs( guidDev,
                                     NULL,
                                     NULL,
                                     DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if(hDevInfo != INVALID_HANDLE_VALUE) {

        BOOL bOk = TRUE;
        SP_DEVICE_INTERFACE_DATA ifcData;

        ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        for (DWORD ii=0; bOk; ii++) {
            bOk = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guidDev, ii, &ifcData);
            if (bOk) {
            // Got a device. Get the details.

                SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
                bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                                      &ifcData, NULL, 0, NULL, &devdata);

                //      We really only need devdata
                if( !bOk ) {
                    if( GetLastError() == 122)  //ERROR_INSUFFICIENT_BUFFER, OK in this case
                        bOk = true;
                }
//#if 0
                //      We could get friendly name and/or description here
                TCHAR fname[256] = {0};
                TCHAR desc[256] ={0};
                if (bOk) {
                    BOOL bSuccess = SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
                        (PBYTE)fname, sizeof(fname), NULL);

                    bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
                        (PBYTE)desc, sizeof(desc), NULL);
                }
//#endif
                //  Get the "COMn string from the registry key
                if(bOk) {
                    bool bFoundCom = false;
                    TCHAR dname[256];
                    HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(hDevInfo, &devdata,
                                                                   DICS_FLAG_GLOBAL, 0,
                                                                   DIREG_DEV, KEY_QUERY_VALUE);
                    if(INVALID_HANDLE_VALUE != hDeviceRegistryKey) {
                            DWORD RegKeyType;
                            wchar_t    wport[80];
                            LPCWSTR cstr = wport;
                            MultiByteToWideChar( 0, 0, "PortName", -1, wport, 80);
                            DWORD len = sizeof(dname);

                            int result = RegQueryValueEx(hDeviceRegistryKey, cstr,
                                                        0, &RegKeyType, (PBYTE)dname, &len );
                            if( result == 0 )
                                bFoundCom = true;
                    }

                    if( bFoundCom ) {
                        wxString port( dname, wxConvUTF8 );

                        //      If the port has already been found, remove the prior entry
                        //      in favor of this entry, which will have descriptive information appended
                        for( unsigned int n=0 ; n < preturn->GetCount() ; n++ ) {
                            if((preturn->Item(n)).IsSameAs(port)){
                                preturn->RemoveAt( n );
                                break;
                            }
                        }
                        wxString desc_name( desc, wxConvUTF8 );         // append "description"
                        port += _T(" ");
                        port += desc_name;

                        preturn->Add( port );
                    }
                }
            }
        }//for
    }// if


//    Search for Garmin device driver on Windows platforms

    HDEVINFO hdeviceinfo = INVALID_HANDLE_VALUE;

    hdeviceinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_DETECT_GUID, NULL, NULL,
            DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

    if( hdeviceinfo != INVALID_HANDLE_VALUE ) {
        
        if(GarminProtocolHandler::IsGarminPlugged()){
            wxLogMessage( _T("EnumerateSerialPorts() Found Garmin USB Device.") );
            preturn->Add( _T("Garmin-USB") );         // Add generic Garmin selectable device
        }
    }

#if 0
    SP_DEVICE_INTERFACE_DATA deviceinterface;
    deviceinterface.cbSize = sizeof(deviceinterface);

    if (SetupDiEnumDeviceInterfaces(hdeviceinfo,
                    NULL,
                    (GUID *) &GARMIN_DETECT_GUID,
                    0,
                    &deviceinterface))
    {
        wxLogMessage(_T("Found Garmin Device."));

        preturn->Add(_T("GARMIN"));         // Add generic Garmin selectable device
    }
#endif

#endif      //__WXMSW__
    return preturn;
}


bool CheckSerialAccess( void )
{
    bool bret = true;
#ifdef __WXGTK__

#if 0
    termios ttyset_old;
    termios ttyset;
    termios ttyset_check;

    // Get a list of the ports
    wxArrayString *ports = EnumerateSerialPorts();
    if( ports->GetCount() == 0 )
        bret = false;

    for(unsigned int i=0 ; i < ports->GetCount() ; i++){
        wxCharBuffer buf = ports->Item(i).ToUTF8();

        //      For the first real port found, try to open it, write some config, and
        //      be sure it reads back correctly.
        if( isTTYreal( buf.data() ) ){
            int fd = open(buf.data(), O_RDWR | O_NONBLOCK | O_NOCTTY);

            // device name is pointing to a real device
            if(fd > 0) {

                if (isatty(fd) != 0)
                {
                    /* Save original terminal parameters */
                    tcgetattr(fd,&ttyset_old);
                    // Write some data
                    memcpy(&ttyset, &ttyset_old, sizeof(termios));

                    ttyset.c_cflag &=~ CSIZE;
                    ttyset.c_cflag |= CSIZE & CS7;

                    tcsetattr(fd, TCSANOW, &ttyset);

                    // Read it back
                    tcgetattr(fd, &ttyset_check);
                    if(( ttyset_check.c_cflag & CSIZE) != CS7 ){
                        bret = false;
                    }
                    else {
                            // and again
                        ttyset.c_cflag &=~ CSIZE;
                        ttyset.c_cflag |= CSIZE & CS8;

                        tcsetattr(fd, TCSANOW, &ttyset);

                            // Read it back
                        tcgetattr(fd, &ttyset_check);
                        if(( ttyset_check.c_cflag & CSIZE) != CS8 ){
                            bret = false;
                        }
                    }

                    tcsetattr(fd, TCSANOW, &ttyset_old);
                }

                close (fd);
            }   // if open
        }
    }

#endif

    //  Who owns /dev/ttyS0?
    bret = false;

    wxArrayString result1;
    wxExecute(_T("stat -c %G /dev/ttyS0"), result1);
    if(!result1.size())
        wxExecute(_T("stat -c %G /dev/ttyUSB0"), result1);

    if(!result1.size())
        wxExecute(_T("stat -c %G /dev/ttyACM0"), result1);

    wxString msg1 = _("OpenCPN requires access to serial ports to use serial NMEA data.\n");
    if(!result1.size()) {
        wxString msg = msg1 + _("No Serial Ports can be found on this system.\n\
You must install a serial port (modprobe correct kernel module) or plug in a usb serial device.\n");

        OCPNMessageBox ( NULL, msg, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 30 );
        return false;
    }

    //  Is the current user in this group?
    wxString user = wxGetUserId(), group = result1[0];

    wxArrayString result2;
    wxExecute(_T("groups ") + user, result2);

    if(result2.size()) {
        wxString user_groups = result2[0];

        if(user_groups.Find(group) != wxNOT_FOUND)
            bret = true;
    }

    if(!bret){

        wxString msg = msg1 + _("\
You do currently not have permission to access the serial ports on this system.\n\n\
It is suggested that you exit OpenCPN now,\n\
and add yourself to the correct group to enable serial port access.\n\n\
You may do so by executing the following command from the linux command line:\n\n\
                sudo usermod -a -G ");

        msg += group;
        msg += _T(" ");
        msg += user;
        msg += _T("\n");

        OCPNMessageBox ( NULL, msg, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 30 );
    }



#endif

    return bret;
}



/*************************************************************************
 * Global color management routines
 *
 *************************************************************************/
wxColour GetGlobalColor(wxString colorName)
{
    wxColour ret_color;

#ifdef USE_S57
    //    Use the S52 Presentation library if present
    if( ps52plib )
        ret_color = ps52plib->getwxColour( colorName );
#endif
    if( !ret_color.Ok() && pcurrent_user_color_hash )
        ret_color = ( *pcurrent_user_color_hash )[colorName];

    //    Default
    if( !ret_color.Ok() ) {
        ret_color.Set( 128, 128, 128 );  // Simple Grey
        wxLogMessage(_T("Warning: Color not found ") + colorName);
        // Avoid duplicate warnings:
        ( *pcurrent_user_color_hash )[colorName] = ret_color;
    }

    return ret_color;
}

static const char *usercolors[] = { "Table:DAY", "GREEN1;120;255;120;", "GREEN2; 45;150; 45;",
        "GREEN3;200;220;200;", "GREEN4;  0;255;  0;", "BLUE1; 170;170;255;", "BLUE2;  45; 45;170;",
        "BLUE3;   0;  0;255;", "GREY1; 200;200;200;", "GREY2; 230;230;230;", "RED1;  220;200;200;",
        "UBLCK;   0;  0;  0;", "UWHIT; 255;255;255;", "URED;  255;  0;  0;", "UGREN;   0;255;  0;",
        "YELO1; 243;229; 47;", "YELO2; 128; 80;  0;", "TEAL1;   0;128;128;", "GREEN5;170;254;  0;",
#ifdef __WXOSX__
        "DILG0; 255;255;255;",              // Dialog Background white
#else
        "DILG0; 238;239;242;",              // Dialog Background white
#endif
        "DILG1; 212;208;200;",              // Dialog Background
        "DILG2; 255;255;255;",              // Control Background
        "DILG3;   0;  0;  0;",              // Text
        "UITX1;   0;  0;  0;",              // Menu Text, derived from UINFF
        "UDKRD; 124; 16;  0;",
        "UARTE; 200;  0;  0;",              // Active Route, Grey on Dusk/Night

        "NODTA; 163; 180; 183;",
        "CHBLK;   7;   7;   7;",
        "SNDG1; 125; 137; 140;",
        "SNDG2;   7;   7;   7;",
        "UIBDR; 125; 137; 140;",
        "UINFB;  58; 120; 240;",
        "UINFD;   7;   7;   7;",
        "UINFO; 235; 125;  54;",
        "PLRTE; 220;  64;  37;",
        "CHMGD; 197; 69; 195;",
        "UIBCK; 212; 234; 238;",

        "DASHB; 255;255;255;",              // Dashboard Instr background
        "DASHL; 190;190;190;",              // Dashboard Instr Label
        "DASHF;  50; 50; 50;",              // Dashboard Foreground
        "DASHR; 200;  0;  0;",              // Dashboard Red
        "DASHG;   0;200;  0;",              // Dashboard Green
        "DASHN; 200;120;  0;",              // Dashboard Needle
        "DASH1; 204;204;255;",              // Dashboard Illustrations
        "DASH2; 122;131;172;",              // Dashboard Illustrations

        "Table:DUSK", "GREEN1; 60;128; 60;", "GREEN2; 22; 75; 22;", "GREEN3; 80;100; 80;",
        "GREEN4;  0;128;  0;", "BLUE1;  80; 80;160;", "BLUE2;  30; 30;120;", "BLUE3;   0;  0;128;",
        "GREY1; 100;100;100;", "GREY2; 128;128;128;", "RED1;  150;100;100;", "UBLCK;   0;  0;  0;",
        "UWHIT; 255;255;255;", "URED;  120; 54; 11;", "UGREN;  35;110; 20;", "YELO1; 120;115; 24;",
        "YELO2;  64; 40;  0;", "TEAL1;   0; 64; 64;", "GREEN5; 85;128; 0;",
        "DILG0; 110;110;110;",              // Dialog Background
        "DILG1; 110;110;110;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3; 130;130;130;",              // Text
        "UITX1;  41; 46; 46;",              // Menu Text, derived from UINFF
        "UDKRD;  80;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night

        "NODTA;  41;  46;  46;"
        "CHBLK;  54;  60;  61;",
        "SNDG1;  41;  46;  46;",
        "SNDG2;  71;  78;  79;",
        "UIBDR;  54;  60;  61;",
        "UINFB;  19;  40;  80;",
        "UINFD;  71;  78;  79;",
        "UINFO;  75;  38;  19;",
        "PLRTE;  73;  21;  12;",
        "CHMGD; 74; 58; 81;",
        "UIBCK; 7; 7; 7;",

        "DASHB;  77; 77; 77;",              // Dashboard Instr background
        "DASHL;  54; 54; 54;",              // Dashboard Instr Label
        "DASHF;   0;  0;  0;",              // Dashboard Foreground
        "DASHR;  58; 21; 21;",              // Dashboard Red
        "DASHG;  21; 58; 21;",              // Dashboard Green
        "DASHN; 100; 50;  0;",              // Dashboard Needle
        "DASH1;  76; 76;113;",              // Dashboard Illustrations
        "DASH2;  48; 52; 72;",              // Dashboard Illustrations

        "Table:NIGHT", "GREEN1; 30; 80; 30;", "GREEN2; 15; 60; 15;", "GREEN3; 12; 23;  9;",
        "GREEN4;  0; 64;  0;", "BLUE1;  60; 60;100;", "BLUE2;  22; 22; 85;", "BLUE3;   0;  0; 40;",
        "GREY1;  48; 48; 48;", "GREY2;  64; 64; 64;", "RED1;  100; 50; 50;", "UWHIT; 255;255;255;",
        "UBLCK;   0;  0;  0;", "URED;   60; 27;  5;", "UGREN;  17; 55; 10;", "YELO1;  60; 65; 12;",
        "YELO2;  32; 20;  0;", "TEAL1;   0; 32; 32;", "GREEN5; 44; 64; 0;",
        "DILG0;  80; 80; 80;",              // Dialog Background
        "DILG1;  80; 80; 80;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3;  65; 65; 65;",              // Text
        "UITX1;  31; 34; 35;",              // Menu Text, derived from UINFF
        "UDKRD;  50;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night

        "NODTA;   7;   7;   7;"
        "CHBLK; 163; 180; 183;",
        "SNDG1; 125; 137; 140;",
        "SNDG2; 212; 234; 238;",
        "UIBDR; 163; 180; 183;",
        "UINFB;  21;  29;  69;",
        "UINFD; 212; 234; 238;",
        "UINFO; 221; 118;  51;",
        "PLRTE; 220;  64;  37;",
        "CHMGD; 52; 18; 52;",
        "UIBCK; 7; 7; 7;",

        "DASHB;   0;  0;  0;",              // Dashboard Instr background
        "DASHL;  20; 20; 20;",              // Dashboard Instr Label
        "DASHF;  64; 64; 64;",              // Dashboard Foreground
        "DASHR;  70; 15; 15;",              // Dashboard Red
        "DASHG;  15; 70; 15;",              // Dashboard Green
        "DASHN;  17; 80; 56;",              // Dashboard Needle
        "DASH1;  48; 52; 72;",              // Dashboard Illustrations
        "DASH2;  36; 36; 53;",              // Dashboard Illustrations

        "*****" };

int get_static_line( char *d, const char **p, int index, int n )
{
    if( !strcmp( p[index], "*****" ) ) return 0;

    strncpy( d, p[index], n );
    return strlen( d );
}

void InitializeUserColors( void )
{
    const char **p = usercolors;
    char buf[80];
    int index = 0;
    char TableName[20];
    colTable *ctp;
    colTable *ct;
    int colIdx = 0;
    int R, G, B;

    UserColorTableArray = new wxArrayPtrVoid;
    UserColourHashTableArray = new wxArrayPtrVoid;

    //    Create 3 color table entries
    ct = new colTable;
    ct->tableName = new wxString( _T("DAY") );
    ct->color = new wxArrayPtrVoid;
    UserColorTableArray->Add( (void *) ct );

    ct = new colTable;
    ct->tableName = new wxString( _T("DUSK") );
    ct->color = new wxArrayPtrVoid;
    UserColorTableArray->Add( (void *) ct );

    ct = new colTable;
    ct->tableName = new wxString( _T("NIGHT") );
    ct->color = new wxArrayPtrVoid;
    UserColorTableArray->Add( (void *) ct );

    while( ( get_static_line( buf, p, index, 80 ) ) ) {
        if( !strncmp( buf, "Table", 5 ) ) {
            sscanf( buf, "Table:%s", TableName );

            for( unsigned int it = 0; it < UserColorTableArray->GetCount(); it++ ) {
                ctp = (colTable *) ( UserColorTableArray->Item( it ) );
                if( !strcmp( TableName, ctp->tableName->mb_str() ) ) {
                    ct = ctp;
                    colIdx = 0;
                    break;
                }
            }

        } else {
            char name[21];
            int j = 0;
            while( buf[j] != ';' && j < 20 ) {
                name[j] = buf[j];
                j++;
            }
            name[j] = 0;

            S52color *c = new S52color;
            strcpy( c->colName, name );

            sscanf( &buf[j], ";%i;%i;%i", &R, &G, &B );
            c->R = (char) R;
            c->G = (char) G;
            c->B = (char) B;

            ct->color->Add( c );

        }

        index++;
    }

    //    Now create the Hash tables

    for( unsigned int its = 0; its < UserColorTableArray->GetCount(); its++ ) {
        wxColorHashMap *phash = new wxColorHashMap;
        UserColourHashTableArray->Add( (void *) phash );

        colTable *ctp = (colTable *) ( UserColorTableArray->Item( its ) );

        for( unsigned int ic = 0; ic < ctp->color->GetCount(); ic++ ) {
            S52color *c2 = (S52color *) ( ctp->color->Item( ic ) );

            wxColour c( c2->R, c2->G, c2->B );
            wxString key( c2->colName, wxConvUTF8 );
            ( *phash )[key] = c;

        }
    }

    //    Establish a default hash table pointer
    //    in case a color is needed before ColorScheme is set
    pcurrent_user_color_hash = (wxColorHashMap *) UserColourHashTableArray->Item( 0 );
}

void DeInitializeUserColors( void )
{
    unsigned int i;
    for( i = 0; i < UserColorTableArray->GetCount(); i++ ) {
        colTable *ct = (colTable *) UserColorTableArray->Item( i );

        for( unsigned int j = 0; j < ct->color->GetCount(); j++ ) {
            S52color *c = (S52color *) ct->color->Item( j );
            delete c;                     //color
        }

        delete ct->tableName;               // wxString
        delete ct->color;                   // wxArrayPtrVoid

        delete ct;                          // colTable
    }

    delete UserColorTableArray;

    for( i = 0; i < UserColourHashTableArray->GetCount(); i++ ) {
        wxColorHashMap *phash = (wxColorHashMap *) UserColourHashTableArray->Item( i );
        delete phash;
    }

    delete UserColourHashTableArray;

}

#ifdef __WXMSW__

#define NCOLORS 40

typedef struct _MSW_COLOR_SPEC {
    int COLOR_NAME;
    wxString S52_RGB_COLOR;
    int SysRGB_COLOR;
} MSW_COLOR_SPEC;

MSW_COLOR_SPEC color_spec[] = { { COLOR_MENU, _T("UIBCK"), 0 }, { COLOR_MENUTEXT, _T("UITX1"), 0 },
        { COLOR_BTNSHADOW, _T("UIBCK"), 0 },                        // Menu Frame
        { -1, _T(""), 0 } };

void SaveSystemColors()
{
    /*
     color_3dface = pGetSysColor(COLOR_3DFACE);
     color_3dhilite = pGetSysColor(COLOR_3DHILIGHT);
     color_3dshadow = pGetSysColor(COLOR_3DSHADOW);
     color_3ddkshadow = pGetSysColor(COLOR_3DDKSHADOW);
     color_3dlight = pGetSysColor(COLOR_3DLIGHT);
     color_activecaption = pGetSysColor(COLOR_ACTIVECAPTION);
     color_gradientactivecaption = pGetSysColor(27); //COLOR_3DLIGHT);
     color_captiontext = pGetSysColor(COLOR_CAPTIONTEXT);
     color_windowframe = pGetSysColor(COLOR_WINDOWFRAME);
     color_inactiveborder = pGetSysColor(COLOR_INACTIVEBORDER);
     */
    //    Record the default system color in my substitution structure
    MSW_COLOR_SPEC *pcspec = &color_spec[0];
    while( pcspec->COLOR_NAME != -1 ) {
        pcspec->SysRGB_COLOR = pGetSysColor( pcspec->COLOR_NAME );
        pcspec++;
    }
}

void RestoreSystemColors()
{
    int element[NCOLORS];
    int rgbcolor[NCOLORS];
    int i = 0;

    MSW_COLOR_SPEC *pcspec = &color_spec[0];
    while( pcspec->COLOR_NAME != -1 ) {
        element[i] = pcspec->COLOR_NAME;
        rgbcolor[i] = pcspec->SysRGB_COLOR;

        pcspec++;
        i++;
    }

    pSetSysColors( i, (unsigned long *) &element[0], (unsigned long *) &rgbcolor[0] );

}

#endif

void SetSystemColors( ColorScheme cs )
{
//---------------
#if 0
    //    This is the list of Color Types from winuser.h
    /*
     * Color Types
     */
#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            7

#define COLOR_SCROLLBAR         0         //??
#define COLOR_BACKGROUND        1         //??
#define COLOR_ACTIVECAPTION     2       //??
#define COLOR_INACTIVECAPTION   3         //??
#define COLOR_MENU              4         // Menu background
#define COLOR_WINDOW            5         // default window background
#define COLOR_WINDOWFRAME       6         // Sub-Window frames, like status bar, etc..
#define COLOR_MENUTEXT          7         // Menu text
#define COLOR_WINDOWTEXT        8         //??
#define COLOR_CAPTIONTEXT       9         //??
#define COLOR_ACTIVEBORDER      10        //??
#define COLOR_INACTIVEBORDER    11       //??
#define COLOR_APPWORKSPACE      12       //??
#define COLOR_HIGHLIGHT         13       //Highlited text background  in query box tree
#define COLOR_HIGHLIGHTTEXT     14        //??
#define COLOR_BTNFACE           15        //??
#define COLOR_BTNSHADOW         16        // Menu Frame
#define COLOR_GRAYTEXT          17        // Greyed out text in menu
#define COLOR_BTNTEXT           18        //??
#define COLOR_INACTIVECAPTIONTEXT 19      //??
#define COLOR_BTNHIGHLIGHT      20        //??
#if(WINVER >= 0x0400)
#define COLOR_3DDKSHADOW        21        //??
#define COLOR_3DLIGHT           22        // Grid rule lines in list control
#define COLOR_INFOTEXT          23        //??
#define COLOR_INFOBK            24
#endif /* WINVER >= 0x0400 */

#if(WINVER >= 0x0500)
#define COLOR_HOTLIGHT          26              //??
#define COLOR_GRADIENTACTIVECAPTION 27        //??
#define COLOR_GRADIENTINACTIVECAPTION 28        //??
#if(WINVER >= 0x0501)
#define COLOR_MENUHILIGHT       29              // Selected item in menu, maybe needs reset on popup menu?
#define COLOR_MENUBAR           30              //??
#endif /* WINVER >= 0x0501 */
#endif /* WINVER >= 0x0500 */

#if(WINVER >= 0x0400)
#define COLOR_DESKTOP           COLOR_BACKGROUND
#define COLOR_3DFACE            COLOR_BTNFACE
#define COLOR_3DSHADOW          COLOR_BTNSHADOW
#define COLOR_3DHIGHLIGHT       COLOR_BTNHIGHLIGHT
#define COLOR_3DHILIGHT         COLOR_BTNHIGHLIGHT
#define COLOR_BTNHILIGHT        COLOR_BTNHIGHLIGHT
#endif /* WINVER >= 0x0400 */
#endif

#ifdef __WXMSW__
    int element[NCOLORS];
    int rgbcolor[NCOLORS];
    int i = 0;
    if( ( GLOBAL_COLOR_SCHEME_DUSK == cs ) || ( GLOBAL_COLOR_SCHEME_NIGHT == cs ) ) {
        MSW_COLOR_SPEC *pcspec = &color_spec[0];
        while( pcspec->COLOR_NAME != -1 ) {
            wxColour color = GetGlobalColor( pcspec->S52_RGB_COLOR );
            rgbcolor[i] = ( color.Red() << 16 ) + ( color.Green() << 8 ) + color.Blue();
            element[i] = pcspec->COLOR_NAME;

            i++;
            pcspec++;
        }

        pSetSysColors( i, (unsigned long *) &element[0], (unsigned long *) &rgbcolor[0] );

    } else {         // for daylight colors, use default windows colors as saved....

        RestoreSystemColors();
    }
#endif
}

//               A helper function to check for proper parameters of anchor watch
//
double AnchorDistFix( double const d, double const AnchorPointMinDist,
        double const AnchorPointMaxDist )   //  pjotrc 2010.02.22
{
    if( d >= 0.0 ) if( d < AnchorPointMinDist ) return AnchorPointMinDist;
    else
        if( d > AnchorPointMaxDist ) return AnchorPointMaxDist;
        else
            return d;

    else
        //if ( d < 0.0 )
        if( d > -AnchorPointMinDist ) return -AnchorPointMinDist;
        else
            if( d < -AnchorPointMaxDist ) return -AnchorPointMaxDist;
            else
                return d;
}

//      Console supporting printf functionality for Windows GUI app

#ifdef __WXMSW__
static const WORD MAX_CONSOLE_LINES = 500;  // maximum mumber of lines the output console should have

//#ifdef _DEBUG

void RedirectIOToConsole()

{

    int hConHandle;

    long lStdHandle;

    CONSOLE_SCREEN_BUFFER_INFO coninfo;

    FILE *fp;

    // allocate a console for this app

    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

    // redirect unbuffered STDOUT to the console

    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 );


    // redirect unbuffered STDIN to the console

    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
    setvbuf( stdin, NULL, _IONBF, 0 );

    // redirect unbuffered STDERR to the console

    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, NULL, _IONBF, 0 );

    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well

    //ios::sync_with_stdio();

}

//#endif
#endif


#ifdef __WXMSW__
bool TestGLCanvas(wxString &prog_dir)
{
    wxString test_app = prog_dir;
    test_app += _T("ocpn_gltest1.exe");

    if(::wxFileExists(test_app)){
        long proc_return = ::wxExecute(test_app, wxEXEC_SYNC);
        printf("OpenGL Test Process returned %0X\n", proc_return);
        if(proc_return == 0)
            printf("GLCanvas OK\n");
        else
            printf("GLCanvas failed to start, disabling OpenGL.\n");

        return (proc_return == 0);
    }
    else
        return true;


}
#endif


#if 0
/*************************************************************************
 * Serial port enumeration routines
 *
 * The EnumSerialPort function will populate an array of SSerInfo structs,
 * each of which contains information about one serial port present in
 * the system. Note that this code must be linked with setupapi.lib,
 * which is included with the Win32 SDK.
 *
 * by Zach Gorman <gormanjz@hotmail.com>
 *
 * Copyright (c) 2002 Archetype Auction Software, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following condition is
 * met: Redistributions of source code must retain the above copyright
 * notice, this condition and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ARCHETYPE AUCTION SOFTWARE OR ITS
 * AFFILIATES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ************************************************************************/

// For MFC
#include <stdafx.h>

// The next 3 includes are needed for serial port enumeration
#include <objbase.h>
#include <initguid.h>
#include <Setupapi.h>

#include "EnumSerial.h"

// The following define is from ntddser.h in the DDK. It is also
// needed for serial port enumeration.
#ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, \
0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif


struct SSerInfo {
    SSerInfo() : bUsbDevice(FALSE) {}
    CString strDevPath;          // Device path for use with CreateFile()
    CString strPortName;         // Simple name (i.e. COM1)
    CString strFriendlyName;     // Full name to be displayed to a user
    BOOL bUsbDevice;             // Provided through a USB connection?
    CString strPortDesc;         // friendly name without the COMx
};

//---------------------------------------------------------------
// Helpers for enumerating the available serial ports.
// These throw a CString on failure, describing the nature of
// the error that occurred.

void EnumPortsWdm(CArray<SSerInfo,SSerInfo&> &asi);
void EnumPortsWNt4(CArray<SSerInfo,SSerInfo&> &asi);
void EnumPortsW9x(CArray<SSerInfo,SSerInfo&> &asi);
void SearchPnpKeyW9x(HKEY hkPnp, BOOL bUsbDevice,
                     CArray<SSerInfo,SSerInfo&> &asi);


//---------------------------------------------------------------
// Routine for enumerating the available serial ports.
// Throws a CString on failure, describing the error that
// occurred. If bIgnoreBusyPorts is TRUE, ports that can't
// be opened for read/write access are not included.

void EnumSerialPorts(CArray<SSerInfo,SSerInfo&> &asi, BOOL bIgnoreBusyPorts)
{
    // Clear the output array
    asi.RemoveAll();

    // Use different techniques to enumerate the available serial
    // ports, depending on the OS we're using
    OSVERSIONINFO vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (!::GetVersionEx(&vi)) {
        CString str;
        str.Format("Could not get OS version. (err=%lx)",
                   GetLastError());
        throw str;
    }
    // Handle windows 9x and NT4 specially
    if (vi.dwMajorVersion < 5) {
        if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
            EnumPortsWNt4(asi);
        else
            EnumPortsW9x(asi);
    }
    else {
        // Win2k and later support a standard API for
        // enumerating hardware devices.
        EnumPortsWdm(asi);
    }

    for (int ii=0; ii<asi.GetSize(); ii++)
    {
        SSerInfo& rsi = asi[ii];
        if (bIgnoreBusyPorts) {
            // Only display ports that can be opened for read/write
            HANDLE hCom = CreateFile(rsi.strDevPath,
                                     GENERIC_READ | GENERIC_WRITE,
                                     0,    /* comm devices must be opened w/exclusive-access */
                                     NULL, /* no security attrs */
                                     OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
                                     0,    /* not overlapped I/O */
                                     NULL  /* hTemplate must be NULL for comm devices */
            );
            if (hCom == INVALID_HANDLE_VALUE) {
                // It can't be opened; remove it.
                asi.RemoveAt(ii);
                ii--;
                continue;
            }
            else {
                // It can be opened! Close it and add it to the list
                ::CloseHandle(hCom);
            }
        }

        // Come up with a name for the device.
        // If there is no friendly name, use the port name.
        if (rsi.strFriendlyName.IsEmpty())
            rsi.strFriendlyName = rsi.strPortName;

        // If there is no description, try to make one up from
            // the friendly name.
            if (rsi.strPortDesc.IsEmpty()) {
                // If the port name is of the form "ACME Port (COM3)"
                // then strip off the " (COM3)"
                rsi.strPortDesc = rsi.strFriendlyName;
                int startdex = rsi.strPortDesc.Find(" (");
                int enddex = rsi.strPortDesc.Find(")");
                if (startdex > 0 && enddex ==
                    (rsi.strPortDesc.GetLength()-1))
                    rsi.strPortDesc = rsi.strPortDesc.Left(startdex);
            }
    }
}

// Helpers for EnumSerialPorts

void EnumPortsWdm(CArray<SSerInfo,SSerInfo&> &asi)
{
    CString strErr;
    // Create a device information set that will be the container for
    // the device interfaces.
    GUID *guidDev = (GUID*) &GUID_CLASS_COMPORT;

    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

    try {
        hDevInfo = SetupDiGetClassDevs( guidDev,
                                        NULL,
                                        NULL,
                                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
        );

        if(hDevInfo == INVALID_HANDLE_VALUE)
        {
            strErr.Format("SetupDiGetClassDevs failed. (err=%lx)",
                          GetLastError());
            throw strErr;
        }

        // Enumerate the serial ports
        BOOL bOk = TRUE;
        SP_DEVICE_INTERFACE_DATA ifcData;
        DWORD dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
        pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[dwDetDataSize];
        // This is required, according to the documentation. Yes,
        // it's weird.
        ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        for (DWORD ii=0; bOk; ii++) {
            bOk = SetupDiEnumDeviceInterfaces(hDevInfo,
                                              NULL, guidDev, ii, &ifcData);
            if (bOk) {
                // Got a device. Get the details.
                SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
                bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                                      &ifcData, pDetData, dwDetDataSize, NULL, &devdata);
                if (bOk) {
                    CString strDevPath(pDetData->DevicePath);
                    // Got a path to the device. Try to get some more info.
                    TCHAR fname[256];
                    TCHAR desc[256];
                    BOOL bSuccess = SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
                        (PBYTE)fname, sizeof(fname), NULL);
                    bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
                        (PBYTE)desc, sizeof(desc), NULL);
                    BOOL bUsbDevice = FALSE;
                    TCHAR locinfo[256];
                    if (SetupDiGetDeviceRegistryProperty(
                        hDevInfo, &devdata, SPDRP_LOCATION_INFORMATION, NULL,
                        (PBYTE)locinfo, sizeof(locinfo), NULL))
                    {
                        // Just check the first three characters to determine
                        // if the port is connected to the USB bus. This isn't
                        // an infallible method; it would be better to use the
                        // BUS GUID. Currently, Windows doesn't let you query
                        // that though (SPDRP_BUSTYPEGUID seems to exist in
                        // documentation only).
                        bUsbDevice = (strncmp(locinfo, "USB", 3)==0);
                    }
                    if (bSuccess) {
                        // Add an entry to the array
                        SSerInfo si;
                        si.strDevPath = strDevPath;
                        si.strFriendlyName = fname;
                        si.strPortDesc = desc;
                        si.bUsbDevice = bUsbDevice;
                        asi.Add(si);
                    }

                }
                else {
                    strErr.Format("SetupDiGetDeviceInterfaceDetail failed. (err=%lx)",
                                  GetLastError());
                    throw strErr;
                }
            }
            else {
                DWORD err = GetLastError();
                if (err != ERROR_NO_MORE_ITEMS) {
                    strErr.Format("SetupDiEnumDeviceInterfaces failed. (err=%lx)", err);
                    throw strErr;
                }
            }
        }
    }
    catch (CString strCatchErr) {
        strErr = strCatchErr;
    }

    if (pDetData != NULL)
        delete [] (char*)pDetData;
    if (hDevInfo != INVALID_HANDLE_VALUE)
        SetupDiDestroyDeviceInfoList(hDevInfo);

    if (!strErr.IsEmpty())
        throw strErr;
}

void EnumPortsWNt4(CArray<SSerInfo,SSerInfo&> &asi)
{
    // NT4's driver model is totally different, and not that
    // many people use NT4 anymore. Just try all the COM ports
    // between 1 and 16
    SSerInfo si;
    for (int ii=1; ii<=16; ii++) {
        CString strPort;
        strPort.Format("COM%d",ii);
        si.strDevPath = CString("\\\\.\\") + strPort;
        si.strPortName = strPort;
        asi.Add(si);
    }
}

void EnumPortsW9x(CArray<SSerInfo,SSerInfo&> &asi)
{
    // Look at all keys in HKLM\Enum, searching for subkeys named
    // *PNP0500 and *PNP0501. Within these subkeys, search for
    // sub-subkeys containing value entries with the name "PORTNAME"
    // Search all subkeys of HKLM\Enum\USBPORTS for PORTNAME entries.

    // First, open HKLM\Enum
    HKEY hkEnum = NULL;
    HKEY hkSubEnum = NULL;
    HKEY hkSubSubEnum = NULL;

    try {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Enum", 0, KEY_READ,
            &hkEnum) != ERROR_SUCCESS)
            throw CString("Could not read from HKLM\\Enum");

        // Enumerate the subkeys of HKLM\Enum
            char acSubEnum[128];
            DWORD dwSubEnumIndex = 0;
            DWORD dwSize = sizeof(acSubEnum);
            while (RegEnumKeyEx(hkEnum, dwSubEnumIndex++, acSubEnum, &dwSize,
                NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
                HKEY hkSubEnum = NULL;
                if (RegOpenKeyEx(hkEnum, acSubEnum, 0, KEY_READ,
                    &hkSubEnum) != ERROR_SUCCESS)
                    throw CString("Could not read from HKLM\\Enum\\")+acSubEnum;

                // Enumerate the subkeys of HKLM\Enum\*\, looking for keys
                    // named *PNP0500 and *PNP0501 (or anything in USBPORTS)
                    BOOL bUsbDevice = (strcmp(acSubEnum,"USBPORTS")==0);
                    char acSubSubEnum[128];
                    dwSize = sizeof(acSubSubEnum);  // set the buffer size
                    DWORD dwSubSubEnumIndex = 0;
                    while (RegEnumKeyEx(hkSubEnum, dwSubSubEnumIndex++, acSubSubEnum,
                        &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                    {
                        BOOL bMatch = (strcmp(acSubSubEnum,"*PNP0500")==0 ||
                        strcmp(acSubSubEnum,"*PNP0501")==0 ||
                        bUsbDevice);
                        if (bMatch) {
                            HKEY hkSubSubEnum = NULL;
                            if (RegOpenKeyEx(hkSubEnum, acSubSubEnum, 0, KEY_READ,
                                &hkSubSubEnum) != ERROR_SUCCESS)
                                throw CString("Could not read from HKLM\\Enum\\") +
                                acSubEnum + "\\" + acSubSubEnum;
                            SearchPnpKeyW9x(hkSubSubEnum, bUsbDevice, asi);
                            RegCloseKey(hkSubSubEnum);
                            hkSubSubEnum = NULL;
                        }

                        dwSize = sizeof(acSubSubEnum);  // restore the buffer size
                    }

                    RegCloseKey(hkSubEnum);
                    hkSubEnum = NULL;
                    dwSize = sizeof(acSubEnum); // restore the buffer size
            }
    }
    catch (CString strError) {
        if (hkEnum != NULL)
            RegCloseKey(hkEnum);
        if (hkSubEnum != NULL)
            RegCloseKey(hkSubEnum);
        if (hkSubSubEnum != NULL)
            RegCloseKey(hkSubSubEnum);
        throw strError;
    }

    RegCloseKey(hkEnum);
}

void SearchPnpKeyW9x(HKEY hkPnp, BOOL bUsbDevice,
                     CArray<SSerInfo,SSerInfo&> &asi)
{
    // Enumerate the subkeys of the given PNP key, looking for values with
    // the name "PORTNAME"
    // First, open HKLM\Enum
    HKEY hkSubPnp = NULL;

    try {
        // Enumerate the subkeys of HKLM\Enum\*\PNP050[01]
        char acSubPnp[128];
        DWORD dwSubPnpIndex = 0;
        DWORD dwSize = sizeof(acSubPnp);
        while (RegEnumKeyEx(hkPnp, dwSubPnpIndex++, acSubPnp, &dwSize,
            NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            HKEY hkSubPnp = NULL;
            if (RegOpenKeyEx(hkPnp, acSubPnp, 0, KEY_READ,
                &hkSubPnp) != ERROR_SUCCESS)
                throw CString("Could not read from HKLM\\Enum\\...\\")
                + acSubPnp;

            // Look for the PORTNAME value
                char acValue[128];
                dwSize = sizeof(acValue);
                if (RegQueryValueEx(hkSubPnp, "PORTNAME", NULL, NULL, (BYTE*)acValue,
                    &dwSize) == ERROR_SUCCESS)
                {
                    CString strPortName(acValue);

                    // Got the portname value. Look for a friendly name.
                    CString strFriendlyName;
                    dwSize = sizeof(acValue);
                    if (RegQueryValueEx(hkSubPnp, "FRIENDLYNAME", NULL, NULL, (BYTE*)acValue,
                        &dwSize) == ERROR_SUCCESS)
                        strFriendlyName = acValue;

                    // Prepare an entry for the output array.
                        SSerInfo si;
                        si.strDevPath = CString("\\\\.\\") + strPortName;
                        si.strPortName = strPortName;
                        si.strFriendlyName = strFriendlyName;
                        si.bUsbDevice = bUsbDevice;

                        // Overwrite duplicates.
                        BOOL bDup = FALSE;
                        for (int ii=0; ii<asi.GetSize() && !bDup; ii++)
                        {
                            if (asi[ii].strPortName == strPortName) {
                                bDup = TRUE;
                                asi[ii] = si;
                            }
                        }
                        if (!bDup) {
                            // Add an entry to the array
                            asi.Add(si);
                        }
                }

                RegCloseKey(hkSubPnp);
                hkSubPnp = NULL;
                dwSize = sizeof(acSubPnp);  // restore the buffer size
        }
    }
    catch (CString strError) {
        if (hkSubPnp != NULL)
            RegCloseKey(hkSubPnp);
        throw strError;
    }
}

#endif

#ifdef __WXMSW__

#define NAME_SIZE 128

const GUID GUID_CLASS_MONITOR = {0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18};

// Assumes hDevRegKey is valid
bool GetMonitorSizeFromEDID(const HKEY hDevRegKey, int *WidthMm, int *HeightMm)
{
    DWORD dwType, AcutalValueNameLength = NAME_SIZE;
    TCHAR valueName[NAME_SIZE];
    
    BYTE EDIDdata[1024];
    DWORD edidsize=sizeof(EDIDdata);
    
    for (LONG i = 0, retValue = ERROR_SUCCESS; retValue != ERROR_NO_MORE_ITEMS; ++i)
    {
        retValue = RegEnumValue ( hDevRegKey, i, &valueName[0],
        &AcutalValueNameLength, NULL, &dwType,
        EDIDdata, // buffer
        &edidsize); // buffer size
        
        if (retValue != ERROR_SUCCESS || 0 != _tcscmp(valueName,_T("EDID")))
            continue;
        
        *WidthMm  = ((EDIDdata[68] & 0xF0) << 4) + EDIDdata[66];
        *HeightMm = ((EDIDdata[68] & 0x0F) << 8) + EDIDdata[67];
        
        return true; // valid EDID found
    }
        
    return false; // EDID not found
}
        
bool GetSizeForDevID(wxString &TargetDevID, int *WidthMm, int *HeightMm)
        {
            HDEVINFO devInfo = SetupDiGetClassDevsEx(
                &GUID_CLASS_MONITOR, //class GUID
                NULL, //enumerator
                NULL, //HWND
                DIGCF_PRESENT, // Flags //DIGCF_ALLCLASSES|
                NULL, // device info, create a new one.
                NULL, // machine name, local machine
                NULL);// reserved
            
            if (NULL == devInfo)
            return false;
            
            bool bRes = false;
            
            for (ULONG i=0; ERROR_NO_MORE_ITEMS != GetLastError(); ++i)
            {
                SP_DEVINFO_DATA devInfoData;
                memset(&devInfoData,0,sizeof(devInfoData));
                devInfoData.cbSize = sizeof(devInfoData);
                
                if (SetupDiEnumDeviceInfo(devInfo,i,&devInfoData))
                {
                    wchar_t    Instance[80];
                    SetupDiGetDeviceInstanceId(devInfo, &devInfoData, Instance, MAX_PATH, NULL);
                    wxString instance(Instance);
                    if(instance.Upper().Find( TargetDevID.Upper() ) == wxNOT_FOUND )
                        continue;
                    
                    HKEY hDevRegKey = SetupDiOpenDevRegKey(devInfo,&devInfoData,
                    DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                    
                    if(!hDevRegKey || (hDevRegKey == INVALID_HANDLE_VALUE))
                        continue;
                    
                    bRes = GetMonitorSizeFromEDID(hDevRegKey, WidthMm, HeightMm);
                    
                    RegCloseKey(hDevRegKey);
                }
            }
            SetupDiDestroyDeviceInfoList(devInfo);
            return bRes;
        }
        
bool GetWindowsMonitorSize( int *width, int *height)
{
            int WidthMm, HeightMm;
            
            DISPLAY_DEVICE dd;
            dd.cb = sizeof(dd);
            DWORD dev = 0; // device index
            int id = 1; // monitor number, as used by Display Properties > Settings
            
            wxString DeviceID;
            bool bFoundDevice = false;
            while (EnumDisplayDevices(0, dev, &dd, 0) && !bFoundDevice)
            {
                DISPLAY_DEVICE ddMon;
                ZeroMemory(&ddMon, sizeof(ddMon));
                ddMon.cb = sizeof(ddMon);
                DWORD devMon = 0;
                
                while (EnumDisplayDevices(dd.DeviceName, devMon, &ddMon, 0) && !bFoundDevice)
                {
                    if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE &&
                        !(ddMon.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
                        {
                            DeviceID = wxString(ddMon.DeviceID, wxConvUTF8);
                            DeviceID = DeviceID.Mid (8);
                            DeviceID = DeviceID.Mid (0, DeviceID.Find ( '\\' ));
                            
                            bFoundDevice = GetSizeForDevID(DeviceID, &WidthMm, &HeightMm);
                        }
                        devMon++;
                    
                    ZeroMemory(&ddMon, sizeof(ddMon));
                    ddMon.cb = sizeof(ddMon);
                }
                
                ZeroMemory(&dd, sizeof(dd));
                dd.cb = sizeof(dd);
                dev++;
            }
            
            if(width)
                *width = WidthMm;
            if(height)
                *height = HeightMm;
            
            return bFoundDevice;
}
        

#endif

bool ReloadLocale()
{
    bool ret = false;
    
    //  Old locale is done.
    delete plocale_def_lang;    
    
    plocale_def_lang = new wxLocale;
    wxString loc_lang_canonical;
    
    const wxLanguageInfo *pli = wxLocale::FindLanguageInfo( g_locale );
    bool b_initok = false;
    
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
        b_initok = plocale_def_lang->Init( wxLANGUAGE_ENGLISH_US, 0 );
        loc_lang_canonical = wxLocale::GetLanguageInfo( wxLANGUAGE_ENGLISH_US )->CanonicalName;
    }
    
    if(b_initok){
        wxString imsg = _T("Opencpn language reload for:  ");
        imsg += loc_lang_canonical;
        wxLogMessage( imsg );
    
        //  wxWidgets assigneds precedence to message catalogs in reverse order of loading.
        //  That is, the last catalog containing a certain translatable item takes precedence.
        
        //  So, Load the catalogs saved in a global string array which is populated as PlugIns request a catalog load.
        //  We want to load the PlugIn catalogs first, so that core opencpn translations loaded later will become precedent.
    
//        wxLog::SetVerbose(true);            // log all messages for debugging language stuff
        
        for(unsigned int i=0 ; i < g_locale_catalog_array.GetCount() ; i++){
            wxString imsg = _T("Loading catalog for:  ");
            imsg += g_locale_catalog_array.Item(i);
            wxLogMessage( imsg );
            plocale_def_lang->AddCatalog( g_locale_catalog_array.Item(i) );
        }
    
    
    // Get core opencpn catalog translation (.mo) file
        wxLogMessage( _T("Loading catalog for opencpn core.") );
        plocale_def_lang->AddCatalog( _T("opencpn") );
        
//        wxLog::SetVerbose(false);
       
        ret = true;
    }
    
    //    Always use dot as decimal
    setlocale( LC_NUMERIC, "C" );
    
    return ret;
    
}



//----------------------------------------------------------------------------------------------------------
//      Application-wide CPL Error handler
//----------------------------------------------------------------------------------------------------------
#ifdef USE_S57
void MyCPLErrorHandler( CPLErr eErrClass, int nError, const char * pszErrorMsg )

{
    char msg[256];

    if( eErrClass == CE_Debug )
    snprintf( msg, 255, "CPL: %s", pszErrorMsg );
    else
        if( eErrClass == CE_Warning )
        snprintf( msg, 255, "CPL Warning %d: %s", nError, pszErrorMsg );
        else
            snprintf( msg, 255, "CPL ERROR %d: %s", nError, pszErrorMsg );

    wxString str( msg, wxConvUTF8 );
    wxLogMessage( str );
}
#endif

#ifdef USE_S57
void LoadS57()
{
    if(ps52plib) // already loaded?
        return;

//      Set up a useable CPL library error handler for S57 stuff
    CPLSetErrorHandler( MyCPLErrorHandler );

//      Init the s57 chart object, specifying the location of the required csv files
    g_csv_locn = g_SData_Locn;
    g_csv_locn.Append( _T("s57data") );

    if( g_bportable ) {
        g_csv_locn = _T(".");
        g_pBASE->appendOSDirSlash( &g_csv_locn );
        g_csv_locn.Append( _T("s57data") );
    }

//      If the config file contains an entry for SENC file prefix, use it.
//      Otherwise, default to PrivateDataDir
    if( g_SENCPrefix.IsEmpty() ) {
        g_SENCPrefix = g_PrivateDataDir;
        g_pBASE->appendOSDirSlash( &g_SENCPrefix );
        g_SENCPrefix.Append( _T("SENC") );
    }

    if( g_bportable ) {
        wxFileName f( g_SENCPrefix );
        if( f.MakeRelativeTo( g_PrivateDataDir ) ) g_SENCPrefix = f.GetFullPath();
        else
            g_SENCPrefix = _T("SENC");
    }

//      If the config file contains an entry for PresentationLibraryData, use it.
//      Otherwise, default to conditionally set spot under g_pcsv_locn
    wxString plib_data;
    bool b_force_legacy = false;

    if( g_UserPresLibData.IsEmpty() ) {
        plib_data = g_csv_locn;
        g_pBASE->appendOSDirSlash( &plib_data );
        plib_data.Append( _T("S52RAZDS.RLE") );
    } else {
        plib_data = g_UserPresLibData;
        b_force_legacy = true;
    }

    ps52plib = new s52plib( plib_data, b_force_legacy );

    //  If the library load failed, try looking for the s57 data elsewhere

    //  First, look in UserDataDir
    /*    From wxWidgets documentation

     wxStandardPaths::GetUserDataDir
     wxString GetUserDataDir() const
     Return the directory for the user-dependent application data files:
     * Unix: ~/.appname
     * Windows: C:\Documents and Settings\username\Application Data\appname
     * Mac: ~/Library/Application Support/appname
     */

    if( !ps52plib->m_bOK ) {
        delete ps52plib;

        wxStandardPaths& std_path = *dynamic_cast<wxStandardPaths*>(&wxApp().GetTraits()->GetStandardPaths());

        wxString look_data_dir;
        look_data_dir.Append( std_path.GetUserDataDir() );
        g_pBASE->appendOSDirSlash( &look_data_dir );
        wxString tentative_SData_Locn = look_data_dir;
        look_data_dir.Append( _T("s57data") );

        plib_data = look_data_dir;
        g_pBASE->appendOSDirSlash( &plib_data );
        plib_data.Append( _T("S52RAZDS.RLE") );

        wxLogMessage( _T("Looking for s57data in ") + look_data_dir );
        ps52plib = new s52plib( plib_data );

        if( ps52plib->m_bOK ) {
            g_csv_locn = look_data_dir;
            g_SData_Locn = tentative_SData_Locn;
        }
    }

    //  And if that doesn't work, look again in the original SData Location
    //  This will cover the case in which the .ini file entry is corrupted or moved

    if( !ps52plib->m_bOK ) {
        delete ps52plib;

        wxString look_data_dir;
        look_data_dir = g_SData_Locn;
        look_data_dir.Append( _T("s57data") );

        plib_data = look_data_dir;
        g_pBASE->appendOSDirSlash( &plib_data );
        plib_data.Append( _T("S52RAZDS.RLE") );

        wxLogMessage( _T("Looking for s57data in ") + look_data_dir );
        ps52plib = new s52plib( plib_data );

        if( ps52plib->m_bOK ) g_csv_locn = look_data_dir;
    }

    if( ps52plib->m_bOK ) {
        wxLogMessage( _T("Using s57data in ") + g_csv_locn );
        g_pRegistrarMan = new s57RegistrarMgr( g_csv_locn, flog );

        if(b_novicemode) {
            ps52plib->m_bShowSoundg = true;
            ps52plib->SetDisplayCategory((enum _DisCat) STANDARD );
            ps52plib->m_nSymbolStyle = (LUPname) PAPER_CHART;
            ps52plib->m_nBoundaryStyle = (LUPname) PLAIN_BOUNDARIES;
            ps52plib->m_bUseSCAMIN = true;
            ps52plib->m_bShowAtonText = true;

            //    Preset some object class visibilites for "Mariner's Standard" disply category
            for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
                OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
                if( !strncmp( pOLE->OBJLName, "DEPARE", 6 ) ) pOLE->nViz = 1;
                if( !strncmp( pOLE->OBJLName, "LNDARE", 6 ) ) pOLE->nViz = 1;
                if( !strncmp( pOLE->OBJLName, "COALNE", 6 ) ) pOLE->nViz = 1;
            }
        }

        pConfig->LoadS57Config();
    } else {
        wxLogMessage( _T("   S52PLIB Initialization failed, disabling Vector charts.") );
        delete ps52plib;
        ps52plib = NULL;
    }
}
#endif
