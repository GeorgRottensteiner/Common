#include ".\loungeroom.h"



CLoungeRoom::CLoungeRoom( const GR::String& strName,
                          const GR::u32 dwID,
                          const GR::u32 dwFlags,
                          const GR::u32 dwMaxClients,
                          const GR::String& strPassword ) :
  m_strName( strName ),
  m_strPassword( strPassword ),
  m_dwID( dwID ),
  m_dwFlags( dwFlags ),
  m_dwMaxClients( dwMaxClients )
{
}



void CLoungeRoom::AddClient( GR::u32 dwClientID )
{

  m_setClients.insert( dwClientID );

}



void CLoungeRoom::RemoveClient( GR::u32 dwClientID )
{

  m_setClients.erase( dwClientID );

}



bool CLoungeRoom::IsClientInside( GR::u32 dwClientID )
{

  return ( m_setClients.find( dwClientID ) != m_setClients.end() );

}



GR::String CLoungeRoom::Name() const
{

  return m_strName;

}



GR::u32 CLoungeRoom::ID() const
{

  return m_dwID;

}



GR::u32 CLoungeRoom::ClientCount() const
{

  return (GR::u32)m_setClients.size();

}



GR::u32 CLoungeRoom::ClientLimit() const
{

  return m_dwMaxClients;

}




