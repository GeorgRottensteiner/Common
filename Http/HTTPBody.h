#ifndef _INCLUDE_HTTPBODY_H__
#define _INCLUDE_HTTPBODY_H__

// HTTPClient.h: interface for the HTTPClient class.
//
//////////////////////////////////////////////////////////////////////

#include <Memory/ByteBuffer.h>


#include <string>
#include <list>


class CHTTPBody
{

  protected:

    struct tHeaderLine
    {
      GR::String       m_strName;
      GR::String       m_strValue;

      tHeaderLine( const GR::String& strName = "", const GR::String& strValue = "" ) :
        m_strName( strName ),
        m_strValue( strValue )
      {
      }
    };

    GR::String         m_strCommand,
                        m_strAddress,
                        m_strHTTPVersion;

    typedef std::list<tHeaderLine>    tListLines;

    tListLines          m_listLines;

    unsigned long       m_dwAnswerCode;


    ByteBuffer         m_Body;


  public:


    CHTTPBody();
    ~CHTTPBody();


    void                AddLine( const GR::String& strName, const GR::String& strValue = "" );
    void                AddLineOnTop( const GR::String& strName, const GR::String& strValue = "" );
    GR::String         Line( const GR::String& strName ) const;

    void                Clear();


    ByteBuffer*        CompleteRequest();

    void                Address( const GR::String& strAddress );
    void                Command( const GR::String& strCommand );
    void                SetBody( const void* pData, size_t iBytes );

    GR::String         Command() const;

    void                AnswerCode( const unsigned long dwCode );
    unsigned long       AnswerCode() const;

    size_t              BodySize() const;
    const void*         Body() const;

};


#endif // _INCLUDE_HTTPBODY_H__