#include <vector>
#include <iterator>

#include <Interface/IIOStream.h>

#include <Misc/Format.h>


#include "SimpleTable.h"


namespace GR
{
  namespace Database
  {

SimpleTable::SimpleTable( size_t iSize )
{

}



SimpleTable::~SimpleTable()
{

}



void SimpleTable::SetEntry( size_t iIndex, const GR::String& strText, GR::u32 LangID )
{
  if ( LangID == 0 )
  {
    LangID = m_LanguageID;
  }
  std::map<GR::u32,std::vector<GR::String> >::iterator    it( m_Entries.find( LangID ) );
  if ( it == m_Entries.end() )
  {
    return;
  }
  std::vector<GR::String>&   vectTable = it->second;


  if ( iIndex > vectTable.size() )
  {
    return;
  }
  if ( iIndex == vectTable.size() )
  {
    std::map<GR::u32,std::vector<GR::String> >::iterator    it( m_Entries.begin() );
    while ( it != m_Entries.end() )
    {
      std::vector<GR::String>&   vectTable = it->second;

      vectTable.push_back( strText );

      ++it;
    }
    return;
  }
  vectTable[iIndex] = strText;
}



void SimpleTable::InsertEntry( size_t iIndex, const GR::String& strText )
{
  std::map<GR::u32,std::vector<GR::String> >::iterator    it( m_Entries.find( m_LanguageID ) );
  if ( it == m_Entries.end() )
  {
    return;
  }
  std::vector<GR::String>&   vectTable = it->second;

  if ( iIndex > vectTable.size() )
  {
    return;
  }
  it = m_Entries.begin();
  while ( it != m_Entries.end() )
  {
    std::vector<GR::String>&   vectTable = it->second;

    if ( it->first == m_LanguageID )
    {
      vectTable.insert( vectTable.begin() + iIndex, strText );
    }
    else
    {
      vectTable.insert( vectTable.begin() + iIndex, "" );
    }

    ++it;
  }
}



GR::String SimpleTable::Entry( size_t iIndex, GR::u32 LangID ) const
{
  if ( LangID == 0 )
  {
    LangID = m_LanguageID;
  }
  std::map<GR::u32,std::vector<GR::String> >::const_iterator    it( m_Entries.find( LangID ) );
  if ( it == m_Entries.end() )
  {
    return ( Misc::Format( "Invalid Index (%1%)" ) << (int)iIndex );
  }
  const std::vector<GR::String>&   vectTable = it->second;

  if ( iIndex >= vectTable.size() )
  {
    return ( Misc::Format( "Invalid Index (%1%)" ) << (int)iIndex );
  }
  return vectTable[iIndex];
}



bool SimpleTable::IsEntryValid( size_t iIndex ) const
{
  std::map<GR::u32,std::vector<GR::String> >::const_iterator    it( m_Entries.find( m_LanguageID ) );
  if ( it == m_Entries.end() )
  {
    return false;
  }
  const std::vector<GR::String>&   vectTable = it->second;

  return ( iIndex < vectTable.size() );
}



void SimpleTable::DeleteEntry( size_t iIndex )
{
  std::map<GR::u32,std::vector<GR::String> >::iterator    it( m_Entries.begin() );
  while ( it != m_Entries.end() )
  {
    std::vector<GR::String>&   vectTable = it->second;

    if ( iIndex < vectTable.size() )
    {
      std::vector<GR::String>::iterator    it( vectTable.begin() );
      std::advance( it, iIndex );

      vectTable.erase( it );
    }

    ++it;
  }
}



void SimpleTable::Save( IIOStream& Stream )
{
  // Version
  Stream.WriteU32( 0x0001 );

  // Anzahl Sprachen
  Stream.WriteU32( (unsigned long)m_Entries.size() );

  // Anzahl Einträge
  if ( m_Entries.empty() )
  {
    Stream.WriteU32( 0 );
  }
  else
  {
    Stream.WriteU32( (GR::u32)m_Entries.begin()->second.size() );
  }

  std::map<GR::u32,std::vector<GR::String> >::iterator    it( m_Entries.begin() );
  while ( it != m_Entries.end() )
  {
    std::vector<GR::String>&   vectTable = it->second;

    // Sprache
    Stream.WriteU32( it->first );

    // Einträge
    for ( size_t i = 0; i < vectTable.size(); ++i )
    {
      Stream.WriteString( vectTable[i] );
    }

    ++it;
  }
}



bool SimpleTable::Load( IIOStream& Stream )
{
  m_Entries.clear();

  // Version
  GR::u32     Version = Stream.ReadU32();

  if ( Version == 0x0001 )
  {
    // Anzahl Sprachen
    GR::u32     Languages = Stream.ReadU32();

    // Anzahl Einträge
    GR::u32     Entries = Stream.ReadU32();

    for ( GR::u32 Lang = 0; Lang < Languages; ++Lang )
    {
      GR::u32     Language = Stream.ReadU32();

      m_Entries[Language].reserve( Entries );

      std::vector<GR::String>&   vectTable = m_Entries[Language];

      GR::String                 strResult;

      for ( GR::u32 Entry = 0; Entry < Entries; ++Entry )
      {
        Stream.ReadString( strResult );
        vectTable.push_back( strResult );
      }
    }
    if ( Languages )
    {
      SetLangID( m_Entries.begin()->first );
    }
  }
  return true;
}



bool SimpleTable::LoadOld( IIOStream& Stream )
{
  m_Entries.clear();

  std::vector<GR::String>&   vectTable = m_Entries[m_LanguageID];

  size_t   iEntries = Stream.ReadU32();

  vectTable.reserve( iEntries );

  for ( size_t i = 0; i < iEntries; ++i )
  {
    GR::String   strTemp;

    Stream.ReadString( strTemp );
    vectTable.push_back( strTemp );
    // Comments überlesen
    Stream.ReadString( strTemp );
  }
  return true;
}



size_t SimpleTable::GetEntryCount() const
{
  std::map<GR::u32,std::vector<GR::String> >::const_iterator    it( m_Entries.find( m_LanguageID ) );
  if ( it == m_Entries.end() )
  {
    return 0;
  }
  const std::vector<GR::String>&   vectTable = it->second;

  return vectTable.size();
}



void SimpleTable::SetLangID( GR::u32 Language )
{
  m_LanguageID = Language;
}



GR::u32 SimpleTable::LangID()
{
  return m_LanguageID;
}



bool SimpleTable::ChangeLangID( GR::u32 OldLangID, GR::u32 NewLangID )
{
  std::map<GR::u32,std::vector<GR::String> >::iterator   it( m_Entries.find( OldLangID ) );
  if ( it == m_Entries.end() )
  {
    // die Sprache gibt es nicht!
    return false;
  }

  std::map<GR::u32,std::vector<GR::String> >::iterator   itNew( m_Entries.find( NewLangID ) );
  if ( itNew != m_Entries.end() )
  {
    // die Sprache gibt es schon!
    return false;
  }

  std::vector<GR::String>    vectTemp = it->second;

  m_Entries.erase( it );
  m_Entries[NewLangID] = vectTemp;

  return true;
}



bool SimpleTable::AddLangID( GR::u32 NewLangID, GR::u32 CopyFromLangID )
{
  std::map<GR::u32,std::vector<GR::String> >::iterator   it( m_Entries.find( NewLangID ) );
  if ( it != m_Entries.end() )
  {
    // die Sprache gibt es schon!
    return false;
  }
  std::map<GR::u32,std::vector<GR::String> >::iterator   itCopy( m_Entries.find( CopyFromLangID ) );
  if ( itCopy != m_Entries.end() )
  {
    m_Entries[NewLangID] = m_Entries[CopyFromLangID];
    return true;
  }

  if ( m_Entries.empty() )
  {
    m_Entries[NewLangID].clear();
    return true;
  }
  m_Entries[NewLangID] = m_Entries.begin()->second;
  return true;
}



void SimpleTable::RemoveLangID( GR::u32 NewLangID )
{
  std::map<GR::u32,std::vector<GR::String> >::iterator   it( m_Entries.find( NewLangID ) );
  if ( it == m_Entries.end() )
  {
    return;
  }
  m_Entries.erase( it );
}



bool SimpleTable::HasLanguage( GR::u32 LanguageID ) const
{
  return m_Entries.find( LanguageID ) != m_Entries.end();
}



GR::u32 SimpleTable::GetLangCount() const
{
  return (GR::u32)m_Entries.size();
}



GR::u32 SimpleTable::GetLangID( GR::u32 Index ) const
{
  if ( Index >= m_Entries.size() )
  {
    return 0;
  }

  std::map<GR::u32,std::vector<GR::String> >::const_iterator   it( m_Entries.begin() );
  std::advance( it, Index );

  return it->first;
}



GR::Database::Table::eTableType SimpleTable::TableType() const
{
  return SIMPLE_TABLE;
}

};
};