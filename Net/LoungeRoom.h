#pragma once

#include <set>

#include <GR/GRTypes.h>



class CLoungeRoom
{

  public:

    enum eFlags
    {
      LR_DEFAULT        = 0x00000000,
      LR_LOBBY          = 0x00000001,     // Lobby-Typ
      LR_LOCKED         = 0x00000002,     // gesperrt, kein Beitreten
      LR_NO_PRIVATE_MSG = 0x00000004,     // keine priv. Nachrichten in diesem Raum
      LR_PASSWORD       = 0x00000008,     // mit Passwort gesperrt
    };


    typedef std::set<GR::u32>     tSetClients;


  protected:

    tSetClients                   m_setClients;

    GR::String                   m_strName,
                                  m_strPassword;

    GR::u32                       m_dwID,
                                  m_dwFlags,
                                  m_dwMaxClients;   // 0 = unbegrenzt


  public:


    CLoungeRoom( const GR::String& strName = "",
                 const GR::u32 dwID = 0,
                 const GR::u32 dwFlags = LR_LOBBY,
                 const GR::u32 dwMaxCliens = 0,
                 const GR::String& strPassword = "" );

    void                          AddClient( GR::u32 );
    void                          RemoveClient( GR::u32 );
    bool                          IsClientInside( GR::u32 );

    GR::String                   Name() const;
    GR::u32                       ID() const;
    GR::u32                       ClientCount() const;
    GR::u32                       ClientLimit() const;


    friend class CLoungeServer;
    friend class CLoungeClient;

};
