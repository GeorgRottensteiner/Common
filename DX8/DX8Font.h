#ifndef _DX8_FONT_H_
#define _DX8_FONT_H_
/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Interface/IFont.h>

#include <D3DX8.h>

#include <map>
#include <string>

#include "DX8Types.h"


class CD3DViewer;

struct tTextureSection;

class CDX8Letter
{
  public:

    tTextureSection*    m_pTextureSection;

    int                 m_iWidth,
                        m_iXOffset;


    CDX8Letter() :
      m_pTextureSection( NULL )
    {
    }


    virtual ~CDX8Letter();

};

class CDX8Font : public Interface::IFont
{
  public:

    CD3DViewer*                       m_pViewer;

    std::map<char,CDX8Letter*>        m_mapLetters;

    GR::String                       m_strFileName;

    int                               m_iWidth,
                                      m_iHeight;

    DWORD                             m_dwCreationFlags,
                                      m_dwTransparentColor;


    CDX8Font( CD3DViewer* pViewer );

    virtual ~CDX8Font();

    virtual int                       TextLength( const TCHAR* szText );
    virtual int                       TextHeight( const TCHAR* szText = NULL );
    virtual int                       FontSpacing();

    CDX8Letter*                       Letter( const char cLetter );

    void                              Recreate();
    void                              Release();

    bool                              LoadFont( const char *szFileName, DWORD dwFlags, DWORD dwTransparentColor );
    bool                              LoadFontSquare( const char *szFileName, DWORD dwFlags = DX8::TF_SQUARED, DWORD dwTransparentColor = 0xff000000 );

    void                              Dump();

};




#endif // _DX8_FONT_H_