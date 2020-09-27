// GameData.h: interface for the CGameData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEEVENT_H__E4BABE46_43A9_446F_A166_3C865F8237FE__INCLUDED_)
#define AFX_GAMEEVENT_H__E4BABE46_43A9_446F_A166_3C865F8237FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

struct tGameEvent
{

  enum eGameEvent
  {
    GE_INVALID,
    GE_CHANGE_MAP,
    GE_WARP,
  };

  GR::String       m_strEventParam;
  int               m_iEventParam[4];
  eGameEvent        m_Type;

  tGameEvent( eGameEvent eType = GE_INVALID,
              const GR::String& strEventParam = "",
              int iParam1 = 0,
              int iParam2 = 0,
              int iParam3 = 0,
              int iParam4 = 0 ) :
    m_strEventParam( strEventParam ),
    m_Type( eType )
  {
    m_iEventParam[0] = iParam1;
    m_iEventParam[1] = iParam2;
    m_iEventParam[2] = iParam3;
    m_iEventParam[3] = iParam4;
  }

};

#endif // !defined(AFX_GAMEEVENT_H__E4BABE46_43A9_446F_A166_3C865F8237FE__INCLUDED_)
