#include <Game/Score.h>
#include <IO/FileStream.h>
#include <Misc/Misc.h>

#include <debug/debugclient.h>



HighScore::HighScore()
{
}



HighScore::HighScore( GR::u32 Entries, size_t ItemDataLength, bool HigherScoreUp ) :
  m_MaxEntries( Entries ),
  m_HigherUp( HigherScoreUp ),
  m_ItemDataLength( ItemDataLength )
{

  Initialise( Entries, ItemDataLength, HigherScoreUp );

}



HighScore::~HighScore()
{


}



void HighScore::Clear()
{
  m_Entries.clear();
}



bool HighScore::Initialise( GR::u32 Entries, size_t ItemDataLength, bool HigherScoreUp )
{

  Clear();
  m_MaxEntries      = Entries;
  m_HigherUp        = HigherScoreUp;
  m_ItemDataLength  = ItemDataLength;
  m_Entries.resize( m_MaxEntries );

  for ( GR::u32 i = 0; i < m_MaxEntries; ++i )
  {
    m_Entries[i].m_ItemLength = m_ItemDataLength;
  }
  if ( m_ItemDataLength )
  {
    for ( GR::u32 i = 0; i < m_MaxEntries; ++i )
    {
      m_Entries[i].m_pExtraData = new GR::u8[m_ItemDataLength];
      memset( m_Entries[i].m_pExtraData, 0, m_ItemDataLength );
    }
  }
  return true;

}



bool HighScore::SetEntry( GR::u32 Nr, int Score, const GR::String& Name, void* pData )
{

  if ( ( Nr - 1 >= m_Entries.size() )
  ||   ( Nr < 1 ) )
  {
    dh::Log( "HighScore::SetEntry %d out of bounds", Nr );
    return false;
  }

  tScoreEntry&    Entry = m_Entries[Nr - 1];
  Entry.m_Score = Score;
  Entry.m_Name  = Name;
  if ( ( m_ItemDataLength )
  &&   ( pData )
  &&   ( Entry.m_pExtraData ) )
  {
    memcpy( Entry.m_pExtraData, pData, m_ItemDataLength );
  }
  return true;

}



int HighScore::Score( GR::u32 Nr )
{

  if ( ( Nr < 1 )
  ||   ( Nr > (GR::u32)m_Entries.size() ) )
  {
    return 0;
  }
  
  return m_Entries[Nr - 1].m_Score;

}



GR::String HighScore::Name( GR::u32 Nr )
{

  if ( ( Nr > (GR::u32)m_Entries.size() )
  ||   ( Nr < 1 ) )
  {
    return GR::String();
  }
  return m_Entries[Nr - 1].m_Name;

}



bool HighScore::ItemData( GR::u32 dwNr, void* pBuffer )
{

  if ( ( dwNr > m_Entries.size() )
  ||   ( dwNr < 1 ) )
  {
    return false;
  }
  if ( ( m_ItemDataLength )
  &&   ( pBuffer != NULL ) )
  {
    memcpy( pBuffer, m_Entries[dwNr - 1].m_pExtraData, m_ItemDataLength );
  }
  return true;

}



bool HighScore::SetItemData( GR::u32 dwNr, void* pBuffer )
{

  if ( ( dwNr > m_Entries.size() )
  ||   ( dwNr < 1 ) )
  {
    return false;
  }
  if ( ( m_ItemDataLength )
  &&   ( pBuffer != NULL ) )
  {
    memcpy( m_Entries[dwNr - 1].m_pExtraData, pBuffer, m_ItemDataLength );
  }
  return true;

}



GR::u32 HighScore::GetPosition( int Score )
{

  GR::u32         i;


  if ( m_HigherUp )
  {
    // der höhere Score gewinnt
    for ( i = 0; i < m_Entries.size(); i++ )
    {
      if ( m_Entries[i].m_Score <= Score )
      {
        return i + 1;
      }
    }
  }
  else
  {
    // der niedrigere gewinnt
    for ( i = 0; i < m_Entries.size(); i++ )
    {
      if ( m_Entries[i].m_Score >= Score )
      {
        return i + 1;
      }
    }
  }
  if ( m_MaxEntries == 0 )
  {
    return (GR::u32)m_Entries.size() + 1;
  }
  return 0;

}



bool HighScore::Remove( GR::u32 Pos )
{

  if ( ( Pos == 0 )
  ||   ( Pos > m_Entries.size() ) )
  {
    return false;
  }

  tEntries::iterator    it( m_Entries.begin() );
  std::advance( it, Pos - 1 );

  m_Entries.erase( it );

  return true;

}



bool HighScore::Insert( int Score, const GR::String& Name, void* pItemData )
{

  GR::u32         dwPos;
  

  dwPos = GetPosition( Score );
  if ( dwPos == 0 )
  {
    return false;
  }
  if ( m_MaxEntries == 0 )
  {
    tScoreEntry   Entry;

    Entry.m_ItemLength = m_ItemDataLength;
    Entry.m_Score      = Score;
    Entry.m_Name       = Name;
    if ( m_ItemDataLength )
    {
      Entry.m_pExtraData  = new GR::u8[m_ItemDataLength];
      memcpy( Entry.m_pExtraData, pItemData, m_ItemDataLength );
    }
    m_Entries.insert( m_Entries.begin() + dwPos - 1, Entry );
    return true;
  }
  if ( dwPos == m_MaxEntries )
  {
    SetEntry( dwPos, Score, Name, pItemData );
    return true;
  }

  // nachfolgende Plätze müssen abrutschen
  for ( size_t i = m_MaxEntries - 1; i >= dwPos; i-- )
  {
    m_Entries[i] = m_Entries[i - 1];
  }
  SetEntry( dwPos, Score, Name, pItemData );
  return true;

}



bool HighScore::Load( const char* szFileName )
{
  GR::IO::FileStream  fileScore;

  
  if ( !fileScore.Open( szFileName, IIOStream::OT_READ_ONLY ) )
  {
    return false;
  }

  if ( m_MaxEntries == 0 )
  {
    GR::u32     dwMagic = fileScore.ReadU32();
    if ( dwMagic != 0xcdcdcdcd )
    {
      fileScore.Close();
      return false;
    }
    // neu - Scores mit beliebiger Anzahl
    m_Entries.clear();

    GR::u32   dwEntries = fileScore.ReadU32();

    m_Entries.resize( dwEntries );
    for ( GR::u32 i = 0; i < dwEntries; ++i )
    {
      tScoreEntry&     Entry = m_Entries[i];

      Entry.m_Score = fileScore.ReadU32();
      Entry.m_ItemLength = m_ItemDataLength;
      fileScore.ReadString( Entry.m_Name );
      if ( m_ItemDataLength )
      {
        Entry.m_pExtraData = new GR::u8[m_ItemDataLength];
        fileScore.ReadBlock( Entry.m_pExtraData, (GR::u32)m_ItemDataLength );
      }
    }
  }
  else
  {
    fileScore.SetPosition( 0, IIOStream::PT_SET );
    for ( GR::u32 i = 0; i < m_MaxEntries; ++i )
    {
      m_Entries[i].m_Score = fileScore.ReadU32();
      if ( ( i == 0 )
      &&   ( m_Entries[i].m_Score == 0xcdcdcdcd ) )
      {
        // das war ein neuer Score
        fileScore.Close();
        return false;
      }
      fileScore.ReadString( m_Entries[i].m_Name );
      m_Entries[i].m_ItemLength = m_ItemDataLength;
      if ( m_ItemDataLength )
      {
        fileScore.ReadBlock( m_Entries[i].m_pExtraData, (GR::u32)m_ItemDataLength );
      }
    }
  }
  fileScore.Close();

  return true;

}



void HighScore::Save( const char* szFileName )
{

  GR::IO::FileStream     fileScore;


  if ( !fileScore.Open( szFileName, IIOStream::OT_WRITE_ONLY ) )
  {
    return;
  }

  if ( m_MaxEntries == 0 )
  {
    fileScore.WriteU32( 0xcdcdcdcd );
    fileScore.WriteU32( (GR::u32)m_Entries.size() );
    for ( size_t i = 0; i < m_Entries.size(); ++i )
    {
      fileScore.WriteU32( m_Entries[i].m_Score );
      fileScore.WriteString( m_Entries[i].m_Name );
      if ( m_ItemDataLength )
      {
        fileScore.WriteBlock( m_Entries[i].m_pExtraData, (GR::u32)m_ItemDataLength );
      }
    }
  }
  else
  {
    for ( GR::u32 i = 0; i < m_MaxEntries; ++i )
    {
      fileScore.WriteU32( m_Entries[i].m_Score );
      fileScore.WriteString( m_Entries[i].m_Name );
      if ( m_ItemDataLength )
      {
        fileScore.WriteBlock( m_Entries[i].m_pExtraData, (GR::u32)m_ItemDataLength );
      }
    }
  }
  fileScore.Close();

}



GR::u32 HighScore::Entries() const
{
  return (GR::u32)m_Entries.size();
}



bool HighScore::CouldEnter( int Score )
{
  if ( m_Entries.empty() )
  {
    return false;
  }
  // >= lowest score?
  return ( Score >= m_Entries.back().m_Score );
}