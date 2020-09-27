#pragma once


#include <windows.h>

#include <map>

#include <GR/GRTypes.h>


class GRUIStatusBar;

class GRUICmdManager
{

  public:

    struct tCmdInfo
    {
      enum eCmdInfoFlags
      {
        CMD_FLAG_CONTROL  = 0x00000001,
        CMD_FLAG_SHIFT    = 0x00000002,
        CMD_FLAG_ALT      = 0x00000004,
      };

      HICON               m_hIcon;
      GR::String         m_strName;
      GR::String         m_strToolTip;
      GR::up              m_dwID;
      GR::up              m_dwCommandGroup;
      GR::up              m_wCmdKey;
      GR::up              m_dwCmdKeyFlags;

      tCmdInfo() :
        m_hIcon( NULL ),
        m_strName( "" ),
        m_strToolTip( "" ),
        m_dwID( 0 ),
        m_dwCommandGroup( 0 ),
        m_wCmdKey( 0 ),
        m_dwCmdKeyFlags( 0 )
      {
      }

      ~tCmdInfo()
      {
        if ( m_hIcon )
        {
          DestroyIcon( m_hIcon );
          m_hIcon = NULL;
        }
      }
    };

    GRUIStatusBar*            m_pStatusBar;


    GRUICmdManager();
    ~GRUICmdManager();

    bool                      IsKnownCommand( GR::up dwID );
    bool                      HasShortCut( GR::up dwID );
    GR::String               GetShortCutDesc( GR::up dwID );
    HICON                     GetIcon( GR::up dwID );

    void                      SetCommand( tCmdInfo& CmdInfo );

    void                      HoverOverItem( GR::up dwID );


  protected:

    typedef std::map<GR::u32,tCmdInfo>      tMapCmdInfos;


    tMapCmdInfos              m_mapCmdInfos;


};
