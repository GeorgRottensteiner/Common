#include <windows.h>

#include <debug/debugclient.h>

#include <HTTP/HTTPBody.h>



CHTTPBody::CHTTPBody() :
  m_strCommand( "" ),
  m_strHTTPVersion( "HTTP/1.0" ),
  m_strAddress( "" ),
  m_Body(),
  m_dwAnswerCode( 0 )
{
}



void CHTTPBody::Clear()
{

  m_listLines.clear();
  m_Body.Set( NULL, 0 );
  m_dwAnswerCode = 0;
  m_strCommand.clear();
  m_strAddress.clear();

}



void CHTTPBody::AddLine( const GR::String& strName, const GR::String& strValue )
{

  GR::String     strTrimmedValue = strValue;

  while ( ( strTrimmedValue.length() )
  &&      ( strTrimmedValue[0] == ' ' ) )
  {
    strTrimmedValue = strTrimmedValue.substr( 1 );
  }
  tListLines::iterator    it( m_listLines.begin() );
  while ( it != m_listLines.end() )
  {
    tHeaderLine&    Line = *it;

    if ( Line.m_strName == strName )
    {
      Line.m_strValue = strTrimmedValue;
      return;
    }

    ++it;
  }
  m_listLines.push_back( tHeaderLine( strName, strTrimmedValue ) );

}



void CHTTPBody::AddLineOnTop( const GR::String& strName, const GR::String& strValue )
{

  GR::String     strTrimmedValue = strValue;

  while ( ( strTrimmedValue.length() )
  &&      ( strTrimmedValue[0] == ' ' ) )
  {
    strTrimmedValue = strTrimmedValue.substr( 1 );
  }
  tListLines::iterator    it( m_listLines.begin() );
  while ( it != m_listLines.end() )
  {
    tHeaderLine&    Line = *it;

    if ( Line.m_strName == strName )
    {
      Line.m_strValue = strTrimmedValue;
      return;
    }

    ++it;
  }
  m_listLines.push_front( tHeaderLine( strName, strTrimmedValue ) );

}



GR::String CHTTPBody::Line( const GR::String& strName ) const
{

  tListLines::const_iterator    it( m_listLines.begin() );
  while ( it != m_listLines.end() )
  {
    const tHeaderLine&    Line = *it;

    if ( _stricmp( Line.m_strName.c_str(), strName.c_str() ) == 0 )
    {
      return Line.m_strValue;
    }

    ++it;
  }
  return "";

}



CHTTPBody::~CHTTPBody()
{
}



ByteBuffer* CHTTPBody::CompleteRequest()
{

  GR::String     strCmdLine = m_strCommand + ' ' + m_strAddress + ' ' + m_strHTTPVersion;

  size_t    dwLength = strCmdLine.length() + 2;

  // Header-Zeilen
  tListLines::iterator   it( m_listLines.begin() );
  while ( it != m_listLines.end() )
  {
    tHeaderLine&    Line = *it;

    dwLength += Line.m_strName.length() + 2 + Line.m_strValue.length() + 2;

    ++it;
  }

  dwLength += 2;

  if ( m_Body.Size() )
  {
    dwLength += m_Body.Size();
  }

  ByteBuffer*      pBlock = new ByteBuffer();

  pBlock->Reserve( dwLength );

  BYTE*             pData = (BYTE*)pBlock->Data();

  pBlock->AppendData( strCmdLine.c_str(), strCmdLine.length() );
  pBlock->AppendU8( 13 );
  pBlock->AppendU8( 10 );

  it = m_listLines.begin();
  while ( it != m_listLines.end() )
  {
    tHeaderLine&    Line = *it;

    pBlock->AppendData( Line.m_strName.c_str(), Line.m_strName.length() );

    pBlock->AppendU8( ':' );
    pBlock->AppendU8( ' ' );

    pBlock->AppendData( Line.m_strValue.c_str(), Line.m_strValue.length() );

    pBlock->AppendU8( 13 );
    pBlock->AppendU8( 10 );

    ++it;
  }

  // abschließender Zeilenumbruch
  pBlock->AppendU8( 13 );
  pBlock->AppendU8( 10 );

  // Content
  if ( m_Body.Size() )
  {
    pBlock->AppendData( m_Body.Data(), m_Body.Size() );
  }

  return pBlock;

}



void CHTTPBody::Address( const GR::String& strAddress )
{

  m_strAddress = strAddress;

}



void CHTTPBody::Command( const GR::String& strCommand )
{

  m_strCommand = strCommand;

}



GR::String CHTTPBody::Command() const
{

  return m_strCommand;

}



void CHTTPBody::SetBody( const void* pData, size_t iBytes )
{

  m_Body.Set( pData, iBytes );

}



void CHTTPBody::AnswerCode( const DWORD dwCode )
{

  m_dwAnswerCode = dwCode;

}



DWORD CHTTPBody::AnswerCode() const
{

  return m_dwAnswerCode;

}



size_t CHTTPBody::BodySize() const
{

  return m_Body.Size();

}



const void* CHTTPBody::Body() const
{

  return m_Body.Data();

}
