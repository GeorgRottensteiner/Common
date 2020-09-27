#include ".\gruicmdmanager.h"
#include "GRUIStatusBar.h"



GRUICmdManager::GRUICmdManager() :
  m_pStatusBar( NULL )
{
}

GRUICmdManager::~GRUICmdManager(void)
{
}



bool GRUICmdManager::IsKnownCommand( GR::up dwID )
{

  return ( m_mapCmdInfos.find( dwID ) != m_mapCmdInfos.end() );

}



bool GRUICmdManager::HasShortCut( GR::up dwID )
{

  tMapCmdInfos::iterator    it( m_mapCmdInfos.find( dwID ) );
  if ( it == m_mapCmdInfos.end() )
  {
    return false;
  }

  tCmdInfo&   CmdInfo = it->second;

  return ( CmdInfo.m_wCmdKey != 0 );

}



HICON GRUICmdManager::GetIcon( GR::up dwID )
{

  tMapCmdInfos::iterator    it( m_mapCmdInfos.find( dwID ) );
  if ( it == m_mapCmdInfos.end() )
  {
    return NULL;
  }

  tCmdInfo&   CmdInfo = it->second;

  return CmdInfo.m_hIcon;

}


GR::String GRUICmdManager::GetShortCutDesc( GR::u32 dwID )
{

  tMapCmdInfos::iterator    it( m_mapCmdInfos.find( dwID ) );
  if ( it == m_mapCmdInfos.end() )
  {
    return GR::String( "" );
  }

  tCmdInfo&   CmdInfo = it->second;

  char        szTemp[260];

  GetKeyNameText( MapVirtualKey( (GR::u32)CmdInfo.m_wCmdKey, 0 ) << 16, szTemp, 260 );

  GR::String strTemp = szTemp;
  GR::String strResult;

  if ( CmdInfo.m_dwCmdKeyFlags & tCmdInfo::CMD_FLAG_CONTROL )
  {
    strResult = "Ctrl+";
  }
  if ( CmdInfo.m_dwCmdKeyFlags & tCmdInfo::CMD_FLAG_SHIFT )
  {
    strResult = "Shift+";
  }
  if ( CmdInfo.m_dwCmdKeyFlags & tCmdInfo::CMD_FLAG_ALT )
  {
    strResult = "Alt+";
  }
  strResult += strTemp;

  return strResult;

}



void GRUICmdManager::SetCommand( tCmdInfo& CmdInfo )
{

  m_mapCmdInfos[CmdInfo.m_dwID] = CmdInfo;

}



void GRUICmdManager::HoverOverItem( GR::up dwID )
{

  tMapCmdInfos::iterator    it( m_mapCmdInfos.find( dwID ) );
  if ( it == m_mapCmdInfos.end() )
  {
    if ( m_pStatusBar )
    {
      m_pStatusBar->SetWindowText( "" );
    }
    return;
  }

  tCmdInfo&   CmdInfo = it->second;

  if ( m_pStatusBar )
  {
    m_pStatusBar->SetWindowText( CmdInfo.m_strName + " - " + CmdInfo.m_strToolTip );
  }

}