#include "WADFile.h"

#include <debug/debugclient.h>

#include <String/StringUtil.h>



namespace GR
{
  
WADFile::WADFile()
{

  m_FileName     = "";

  CleanUp();

}



WADFile::~WADFile()
{

  CleanUp();

}



void WADFile::CleanUp()
{

  m_Entries.clear();

  m_FileName = "";
  m_File.Close();
  m_Memory.Close();
  m_TemporaryEntryData.Clear();

}



bool WADFile::Open( IIOStream& ioIn )
{

  if ( &ioIn != &m_Memory )
  {
    m_Entries.clear();

    m_FileName = "";
    m_Memory.Close();
    m_TemporaryEntryData.Clear();
  }

  if ( !ioIn.IsGood() )
  {
    dh::Log( "WADFile::Open stream is not good" );
    return false;
  }

  unsigned char       ucBuffer[30];


  if ( ioIn.ReadBlock( ucBuffer, 23 ) != 23 )
  {
    // keine 23 Bytes gelesen!
    return false;
  }

  /*
  if ( ( ucBuffer[0] != 'U' )
  ||   ( ucBuffer[1] != 'n' )
  ||   ( ucBuffer[2] != 'i' )
  ||   ( ucBuffer[3] != 'D' )
  ||   ( ucBuffer[4] != 'a' )
  ||   ( ucBuffer[5] != 't' )
  ||   ( ucBuffer[6] != 'a' ) )
  {
    // ungültige Kennung im File!
    CleanUp();
    return false;
  }
  */

  GR::u32   AnzahlEntries = ioIn.ReadU32();

  // vorsorgen
  m_Entries.resize( AnzahlEntries );

  for ( GR::u32 i = 0; i < AnzahlEntries; i++ )
  {
    tWADFileEntry&    wadEntry = m_Entries[i];

    // neues Format
    // 4 Bytes Länge des Strings
    // n Bytes String
    // 4 Bytes Offset
    // 4 Bytes Größe
    // 4 Bytes reserved

    ioIn.ReadString( wadEntry.m_FileName );
    wadEntry.m_Offset = ioIn.ReadU32();
    wadEntry.m_Size   = ioIn.ReadU32();
    ioIn.ReadU32();  // Reserved
  }
  
  return true;

}



bool WADFile::Open( const char* szFileName )
{

  if ( !m_File.Open( szFileName ) )
  {
    return false;
  }
  if ( !Open( m_File ) )
  {
    return false;
  }
  m_FileName = szFileName;
  return true;

}



bool WADFile::Open( const ByteBuffer& Source )
{

  m_FileName.erase();
  if ( !m_Memory.Open( Source ) )
  {
    dh::Log( "Memory open failed" );
    return false;
  }
  if ( !Open( m_Memory ) )
  {
    dh::Log( "open from stream" );
    return false;
  }
  return true;

}



void WADFile::Close()
{

  m_File.Close();
  m_Memory.Close();
  m_TemporaryEntryData.Clear();

}



GR::u32 WADFile::GetEntryCount()
{

  return (GR::u32)m_Entries.size();

}



int WADFile::GetEntryIndex( const char *szFileName ) const
{

  int   iIndex = 0;

  tVectFileEntries::const_iterator    it( m_Entries.begin() );
  while ( it != m_Entries.end() )
  {
    if ( GR::Strings::CompareUpperCase( it->m_FileName, szFileName ) == 0 )
    {
      return iIndex;
    }

    ++it;
    ++iIndex;
  }
  return -1;

}



GR::u32 WADFile::GetEntryOffset( const char *szFileName )
{
  tVectFileEntries::iterator    it( m_Entries.begin() );
  while ( it != m_Entries.end() )
  {
    if ( GR::Strings::CompareUpperCase( it->m_FileName, szFileName ) == 0 )
    {
      return it->m_Offset;
    }

    ++it;
  }
  return 0;
}



GR::u32 WADFile::GetEntryOffset( GR::u32 Nr )
{
  if ( Nr >= m_Entries.size() )
  {
    return 0;
  }
  return m_Entries[Nr].m_Offset;
}



GR::u32 WADFile::GetEntrySize( const char *szFileName )
{

  tVectFileEntries::iterator    it( m_Entries.begin() );
  while ( it != m_Entries.end() )
  {
    if ( GR::Strings::CompareUpperCase( it->m_FileName, szFileName ) == 0 )
    {
      return it->m_Size;
    }

    ++it;
  }
  return 0;

}



GR::u32 WADFile::GetEntrySize( GR::u32 Nr )
{

  if ( Nr >= m_Entries.size() )
  {
    return 0;
  }
  return m_Entries[Nr].m_Size;

}



MemoryStream WADFile::GetEntryStream( GR::u32 Nr )
{

  m_TemporaryEntryData.Clear();
  if ( Nr >= m_Entries.size() )
  {
    return MemoryStream();
  }

  if ( m_FileName.empty() )
  {
    // ein Memory-Stream
    return MemoryStream( (const void*)( ( (GR::u8*)m_Memory.Data() ) + m_Entries[Nr].m_Offset ), m_Entries[Nr].m_Size );
  }
  if ( !m_File.Open( m_FileName.c_str() ) )
  {
    return MemoryStream();
  }
  m_File.SetPosition( m_Entries[Nr].m_Offset );

  m_TemporaryEntryData.Resize( m_Entries[Nr].m_Size );

  m_File.ReadBlock( m_TemporaryEntryData.Data(), m_TemporaryEntryData.Size() );
  m_File.Close();

  return MemoryStream( m_TemporaryEntryData );

}



GR::String WADFile::GetEntryName( GR::u32 Nr )
{
  if ( Nr >= m_Entries.size() )
  {
    return GR::String();
  }
  return m_Entries[Nr].m_FileName;
}



GR::IO::FileStream& WADFile::GetFileHandle()
{
  return m_File;
}



GR::u32 WADFile::GetCompleteDataSize() const
{
  GR::u32   Size = 0;

  tVectFileEntries::const_iterator    it( m_Entries.begin() );
  while ( it != m_Entries.end() )
  {
    const tWADFileEntry&    fileEntry = *it;

    Size += fileEntry.m_Size;

    ++it;
  }

  return Size;

}


};

