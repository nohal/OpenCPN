/***************************************************************************
 *
 * Project:  OpenCPN
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
 ***************************************************************************
 */

#ifndef __AIS_DECODER_H__
#define __AIS_DECODER_H__

#include "ais.h"
#include <map>
#include "OCPN_Sound.h"
#include "OCPN_DataStreamEvent.h"

#define TRACKTYPE_DEFAULT       0
#define TRACKTYPE_ALWAYS        1
#define TRACKTYPE_NEVER         2

class Track;

class MMSIProperties
{
public:
    MMSIProperties(){};
    MMSIProperties( int mmsi ){ Init(); MMSI = mmsi; }
    MMSIProperties( wxString &spec );
    
    ~MMSIProperties();
    
    wxString Serialize();
    
    void Init( void );
    int         MMSI;
    int         TrackType;
    bool        m_bignore;
    bool        m_bMOB;
    bool        m_bVDM;
    bool        m_bPersistentTrack;
};

WX_DECLARE_OBJARRAY(MMSIProperties *,      ArrayOfMMSIProperties);

class AIS_Decoder : public wxEvtHandler
{

public:
    AIS_Decoder(wxFrame *parent);

    ~AIS_Decoder(void);

    void OnEvtAIS(OCPN_DataStreamEvent& event);
    AIS_Error Decode(const wxString& str);
    AIS_Target_Hash *GetTargetList(void) {return AISTargetList;}
    AIS_Target_Hash *GetAreaNoticeSourcesList(void) {return AIS_AreaNotice_Sources;}
    AIS_Target_Data *Get_Target_Data_From_MMSI(int mmsi);
    int GetNumTargets(void){ return m_n_targets;}
    bool IsAISSuppressed(void){ return m_bSuppressed; }
    bool IsAISAlertGeneral(void) { return m_bGeneralAlert; }
    AIS_Error DecodeSingleVDO( const wxString& str, GenericPosDatEx *pos, wxString *acc );
    void DeletePersistentTrack( Track *track );
    std::map<int, Track*> m_persistent_tracks;
    
private:
    void OnActivate(wxActivateEvent& event);
    void OnTimerAIS(wxTimerEvent& event);
    void OnTimerAISAudio(wxTimerEvent& event);
    void OnTimerDSC( wxTimerEvent& event );
    
    bool NMEACheckSumOK(const wxString& str);
    bool Parse_VDXBitstring(AIS_Bitstring *bstr, AIS_Target_Data *ptd);
    void UpdateAllCPA(void);
    void UpdateOneCPA(AIS_Target_Data *ptarget);
    void UpdateAllAlarms(void);
    void UpdateAllTracks(void);
    void UpdateOneTrack(AIS_Target_Data *ptarget);
    void BuildERIShipTypeHash(void);
    AIS_Target_Data *ProcessDSx( const wxString& str, bool b_take_dsc = false );
    
    AIS_Target_Hash *AISTargetList;
    AIS_Target_Hash *AIS_AreaNotice_Sources;
    AIS_Target_Name_Hash *AISTargetNames;

    bool              m_busy;
    wxTimer           TimerAIS;
    wxFrame           *m_parent_frame;

    int               nsentences;
    int               isentence;
    wxString          sentence_accumulator;
    bool              m_OK;

    AIS_Target_Data   *m_pLatestTargetData;

    bool             m_bAIS_Audio_Alert_On;
    wxTimer          m_AIS_Audio_Alert_Timer;
    OCPN_Sound       m_AIS_Sound;
    int              m_n_targets;
    bool             m_bSuppressed;
    bool             m_bGeneralAlert;
    AIS_Target_Data  *m_ptentative_dsctarget;
    wxTimer          m_dsc_timer;
    wxString         m_dsc_last_string;
    
public:
    //AIS Global configuration accessors
    void        set_ShowAIS(bool value) { m_bShowAIS = value; }
    bool        ShowAIS() { return m_bShowAIS; }
    void        set_CPAMax(bool value) { m_bCPAMax = value; }
    bool        CPAMax() { return m_bCPAMax; }
    void        set_CPAMax_NM(double value) { m_CPAMax_NM = value; }
    double      CPAMax_NM() { return m_CPAMax_NM; }
    void        set_CPAWarn(bool value) { m_bCPAWarn = value; }
    bool        CPAWarn() { return m_bCPAWarn; }
    void        set_CPAWarn_NM(double value) { m_CPAWarn_NM = value; }
    double      CPAWarn_NM() { return m_CPAWarn_NM; }
    void        set_TCPAMax(bool value) { m_bTCPA_Max = value; }
    bool        TCPAMax() { return m_bTCPA_Max; }
    void        set_TCPAMax_Mins(double value) { m_TCPA_Max = value; }
    double      TCPAMax_Mins() { return m_TCPA_Max; }
    void        set_MarkLost(bool value) { m_bMarkLost = value; }
    bool        MarkLost() { return m_bMarkLost; }
    void        set_MarkLost_Mins(double value) { m_MarkLost_Mins = value; }
    double      MarkLost_Mins() { return m_MarkLost_Mins; }
    void        set_RemoveLost(bool value) { m_bRemoveLost = value; }
    bool        RemoveLost() { return m_bRemoveLost; }
    void        set_RemoveLost_Mins(double value) { m_RemoveLost_Mins = value; }
    double      RemoveLost_Mins() { return m_RemoveLost_Mins; }
    void        set_ShowCOG(bool value) { m_bShowCOG = value; }
    bool        ShowCOG() { return m_bShowCOG; }
    void        set_ShowCOG_Mins(double value) { m_ShowCOG_Mins = value; }
    double      ShowCOG_Mins() { return m_ShowCOG_Mins; }
    void        set_ShowTracks(bool value) { m_bAISShowTracks = value; }
    bool        ShowTracks() { return m_bAISShowTracks; }
    void        set_ShowTracks_Mins(double value) { m_AISShowTracks_Mins = value; }
    double      ShowTracks_Mins() { return m_AISShowTracks_Mins; }
    void        set_ShowMoored(bool value) { m_bShowMoored = value; }
    bool        ShowMoored() { return m_bShowMoored; }
    void        set_ShowMoored_Kts(double value) { m_ShowMoored_Kts = value; }
    double      ShowMoored_Kts() { return m_ShowMoored_Kts; }
    void        set_AlertSoundFile(wxString value) { m_sAIS_Alert_Sound_File = value; }
    wxString    AlertSoundFile() { return m_sAIS_Alert_Sound_File; }
    void        set_SuppressMooredCPAAlert(bool value) { m_bAIS_CPA_Alert_Suppress_Moored = value; }
    bool        SuppressMooredCPAAlert() { return m_bAIS_CPA_Alert_Suppress_Moored; }
    void        set_ACKTimeout(bool value) { m_bAIS_ACK_Timeout = value; }
    bool        ACKTimeout() { return m_bAIS_ACK_Timeout; }
    void        set_ACKTimeout_Mins(double value) { m_AckTimeout_Mins = value; }
    double      ACKTimeout_Mins() { return m_AckTimeout_Mins; }
    void        set_ShowAreaNotices(bool value) { m_bShowAreaNotices = value; }
    bool        ShowAreaNotices() { return m_bShowAreaNotices; }
    void        set_ShowRealSize(bool value) { m_bDrawAISSize = value; }
    bool        ShowRealSize() { return m_bDrawAISSize; }
    void        set_ShowTargetName(bool value) { m_bShowAISName = value; }
    bool        ShowTargetName() { return m_bShowAISName; }
    void        set_ShowNameScale(int value) { m_Show_Target_Name_Scale = value; }
    int         ShowNameScale() { return m_Show_Target_Name_Scale; }
    void        set_WplIsAPRSPosition(bool value) { m_bWplIsAprsPosition = value; }
    bool        WplIsAPRSPosition() { return m_bWplIsAprsPosition; }
    void        set_CPAAlert(bool value) { m_bAIS_CPA_Alert = value; }
    bool        CPAAlert() { return m_bAIS_CPA_Alert; }
    void        set_CPAAlertAudio(bool value) { m_bAIS_CPA_Alert_Audio = value; }
    bool        CPAAlertAudio() { return m_bAIS_CPA_Alert_Audio; }
    void        set_NameCacheEnabled(bool value) { m_benableAISNameCache = value; }
    bool        NameCacheEnabled() { return m_benableAISNameCache; }
    //Dialog properties
    void        set_AlertDlgSize(const wxSize& dlg_size) { m_ais_alert_dialog_size = dlg_size; }
    wxSize      AlertDlgSize() { return m_ais_alert_dialog_size; }
    void        set_AlertDlgPosition(const wxPoint& dlg_position) { m_ais_alert_dialog_position = dlg_position; } //Implement position check
    wxPoint     AlertDlgPosition() { return m_ais_alert_dialog_position; }
    void        set_QueryDlgPosition(const wxPoint& dlg_position) { m_ais_query_dialog_position = dlg_position; } //Implement position check
    wxPoint     QueryDlgPosition() { return m_ais_query_dialog_position; }
    void        set_TargetListAUIPerspective( wxString perspective ) { m_ais_target_list_perspective = perspective; }
    wxString    TargetListAUIPerspective() { return m_ais_target_list_perspective; }
    void        set_TargetListRange( int range ) { m_ais_target_list_range = range; }
    int         TargetListRange() { return m_ais_target_list_range; }
    void        set_TargetListSortColumn( int column ) { m_ais_target_list_sort_column = column; }
    int         TargetListSortColumn() { return m_ais_target_list_sort_column; }
    void        set_TargetListSortReverse( bool value ) { m_ais_target_list_sort_reverse = value; }
    bool        TargetListSortReverse() { return m_ais_target_list_sort_reverse; }
    void        set_TargetListColSpec( wxString col_spec ) { m_ais_target_list_column_spec = col_spec; }
    wxString    TargetListColSpec() { return m_ais_target_list_column_spec; }
    void        set_TargetListCount( int target_count ) { m_AisTargetList_count = target_count; }
    int         TargetListCount() { return m_AisTargetList_count; }
    void        set_TargetListAutosort( bool value ) { m_bAisTargetList_autosort = value; }
    bool        TargetListAutosort() { return m_bAisTargetList_autosort; }
    //Rollover properties
    void        set_RolloverShowClass( bool value ) { m_bAISRolloverShowClass = value; }
    bool        RolloverShowClass() { return m_bAISRolloverShowClass; }
    void        set_RolloverShowCOG( bool value ) { m_bAISRolloverShowCOG = value; }
    bool        RolloverShowCOG() { return m_bAISRolloverShowCOG; }
    void        set_RolloverShowCPA( bool value ) { m_bAISRolloverShowCPA = value; }
    bool        RolloverShowCPA() { return m_bAISRolloverShowCPA; }
        
private:
    // AIS Global configuration
    bool                    m_bShowAIS;
    bool                    m_bCPAMax;
    double                  m_CPAMax_NM;
    bool                    m_bCPAWarn;
    double                  m_CPAWarn_NM;
    bool                    m_bTCPA_Max;
    double                  m_TCPA_Max;
    bool                    m_bMarkLost;
    double                  m_MarkLost_Mins;
    bool                    m_bRemoveLost;
    double                  m_RemoveLost_Mins;
    bool                    m_bShowCOG;
    double                  m_ShowCOG_Mins;
    bool                    m_bAISShowTracks;
    double                  m_AISShowTracks_Mins;
    bool                    m_bShowMoored;
    double                  m_ShowMoored_Kts;
    wxString                m_sAIS_Alert_Sound_File;
    bool                    m_bAIS_CPA_Alert_Suppress_Moored;
    bool                    m_bAIS_ACK_Timeout;
    double                  m_AckTimeout_Mins;
    bool                    m_bShowAreaNotices;
    bool                    m_bDrawAISSize;
    bool                    m_bShowAISName;
    int                     m_Show_Target_Name_Scale;
    bool                    m_bWplIsAprsPosition;
    bool                    m_bAIS_CPA_Alert;
    bool                    m_bAIS_CPA_Alert_Audio;
    //Dialog properties
    wxPoint                 m_ais_alert_dialog_position;
    wxSize                  m_ais_alert_dialog_size;
    wxPoint                 m_ais_query_dialog_position;
    wxString                m_ais_target_list_perspective;
    int                     m_ais_target_list_range;
    int                     m_ais_target_list_sort_column;
    bool                    m_ais_target_list_sort_reverse;
    wxString                m_ais_target_list_column_spec;
    int                     m_AisTargetList_count;
    bool                    m_bAisTargetList_autosort;
    //Rollover properties
    bool                    m_bAISRolloverShowClass;
    bool                    m_bAISRolloverShowCOG;
    bool                    m_bAISRolloverShowCPA;
    
    wxString                *pAISTargetNameFileName;
    
    bool                    m_benableAISNameCache;


DECLARE_EVENT_TABLE()
};

#endif
