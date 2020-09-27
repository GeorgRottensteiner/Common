#ifndef MISC_H
#define MISC_H


#include <GR/GRTypes.h>

#include <string>


#ifdef _WIN32
#include <tchar.h>
#else
#ifdef UNICODE
#define _T(x)      L ## x
#else
#define _T(x)      x
#endif
#endif

template < typename P > //- P wie Pointer
void SafeDelete( P& p )
{
  if ( p )
  {
    delete p;
    p = 0; //- oder NULL
  }
}

//- das selbe für arrays
template < typename P > //- P wie Pointer
void SafeDeleteArray( P& p )
{
  if ( p )
  {
    delete[] p;
    p = 0;
  }
}

template < typename P > //- P wie Pointer
void SafeRelease( P& p )
{
  if ( p )
  {
    p->Release();
    p = 0; //- oder NULL
  }
}



class CMisc
{
  public:

    static GR::String         AppPath( const GR::WChar* formatstr, ... );
    static GR::String         AppPath( const GR::Char* formatstr, ... );

    static const GR::Char*    printf( const GR::Char *lpszFormat, ... );

    static const GR::Char*    TimeStamp();

    static void               FillValue( unsigned char *pDestination, unsigned long ulWidth, unsigned char ucValue );

    static void               FillValue16( unsigned short *pDestination, unsigned long ulWidth, unsigned short wValue );

    static void               FillValue24( GR::u32* pDestination, unsigned long ulWidth, GR::u32 Value );

    static void               FillValue32( GR::u32* pDestination, unsigned long ulWidth, GR::u32 Value );

};


namespace Misc
{
  inline GR::String ToString( GR::i64 iValue, GR::u64 dwStellen = 0, GR::Char* cFillByte = 0 )
  {
    GR::String     strResult;

    bool            bNegative = ( iValue < 0 );

    if ( iValue == 0 )
    {
      strResult = "0";
    }

    while ( iValue )
    {
      GR::i64 iNibble = iValue % 10;
      if ( iNibble < 0 )
      {
        iNibble = -iNibble;
      }
      strResult = (GR::Char)( '0' + iNibble ) + strResult;
      iValue /= 10;
    }
    if ( ( bNegative )
    &&   ( dwStellen ) )
    {
      dwStellen--;
    }
    if ( cFillByte != 0 )
    {
      while ( strResult.length() < dwStellen )
      {
        strResult = *cFillByte + strResult;
      }
    }
    if ( bNegative )
    {
      strResult = "-" + strResult;
    }
    return strResult;
  }

  inline GR::String ToString( GR::u64 dwValue, GR::u64 dwStellen = 0, GR::Char* cFillByte = 0 )
  {
    GR::String     strResult;

    if ( dwValue == 0 )
    {
      strResult = "0";
    }
    while ( dwValue )
    {
      strResult = (GR::Char)( '0' + ( dwValue % 10 ) ) + strResult;
      dwValue /= 10;
    }
    if ( cFillByte != 0 )
    {
      while ( strResult.length() < dwStellen )
      {
        strResult = cFillByte + strResult;
      }
    }
    return strResult;
  }

}




#endif // __MISC_H__



