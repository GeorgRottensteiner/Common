#include <debug\debugclient.h>

#include <Grafik/Palette.h>
#include "ImageData.h"


namespace GR
{
  namespace Graphic
  {


    ImageData::ImageData() :
      m_pData( NULL ),
      m_Width( 0 ),
      m_Height( 0 ),
      m_LineOffsetInBytes( 0 ),
      m_BytesPerLine( 0 ),
      m_TransparentColor( 0 ),
      m_ImageFormat( GR::Graphic::IF_UNKNOWN ),
      m_Owner( false ),
      m_TransparentColorUsed( false ),
      m_Palette( 0 )
    {

    }



    ImageData::~ImageData()
    {
      if ( m_Owner )
      {
        if ( m_pData )
        {
          delete[] (unsigned char*)m_pData;
          m_pData = NULL;
        }
      }
      m_pData = NULL;
    }



    ImageData::ImageData( const ImageData& rhsID ) :
      m_pData( NULL ),
      m_Width( 0 ),
      m_Height( 0 ),
      m_LineOffsetInBytes( 0 ),
      m_BytesPerLine( 0 ),
      m_TransparentColor( 0 ),
      m_ImageFormat( GR::Graphic::IF_UNKNOWN ),
      m_Owner( false ),
      m_TransparentColorUsed( false )
    {
      CreateData( rhsID.Width(), rhsID.Height(), rhsID.ImageFormat() );

      m_Palette = rhsID.m_Palette;

      m_TransparentColorUsed  = rhsID.m_TransparentColorUsed;
      m_TransparentColor      = rhsID.m_TransparentColor;

      for ( int i = 0; i < Height(); ++i )
      {
        memcpy( GetRowData( i ), ( const_cast<ImageData*>(&rhsID)->GetRowData( i ) ), BytesPerLine() );
      }
    }



    ImageData& ImageData::operator= ( const ImageData& rhsID )
    {

      if ( &rhsID == this )
      {
        return *this;
      }
      CreateData( rhsID.Width(), rhsID.Height(), rhsID.ImageFormat() );

      m_Palette = rhsID.m_Palette;
      m_TransparentColorUsed  = rhsID.m_TransparentColorUsed;
      m_TransparentColor      = rhsID.m_TransparentColor;

      for ( int i = 0; i < Height(); ++i )
      {
        memcpy( GetRowData( i ), ( const_cast<ImageData*>(&rhsID)->GetRowData( i ) ), BytesPerLine() );
      }
      return *this;

    }



    int ImageData::LineOffsetInBytes() const
    {

      return m_LineOffsetInBytes;

    }



    int ImageData::BytesPerLine() const
    {
      switch ( m_ImageFormat )
      {
        case GR::Graphic::IF_INDEX1:
          {
            int   iBPL = m_Width / 8;
            if ( m_Width % 8 )
            {
              iBPL++;
            }
            return iBPL;
          }
        case GR::Graphic::IF_INDEX2:
          {
            int   iBPL = m_Width / 4;
            if ( m_Width % 4 )
            {
              iBPL++;
            }
            return iBPL;
          }
        case GR::Graphic::IF_INDEX4:
          {
            int   iBPL = m_Width / 2;
            if ( m_Width % 2 )
            {
              iBPL++;
            }
            return iBPL;
          }
        case GR::Graphic::IF_PALETTED:
        case GR::Graphic::IF_A8:
          return m_Width;
        case GR::Graphic::IF_A1R5G5B5:
        case GR::Graphic::IF_X1R5G5B5:
        case GR::Graphic::IF_R5G6B5:
        case GR::Graphic::IF_A4R4G4B4:
          return 2 * m_Width;
        case GR::Graphic::IF_R8G8B8:
          return 3 * m_Width;
        case GR::Graphic::IF_X8R8G8B8:
        case GR::Graphic::IF_A8R8G8B8:
        case GR::Graphic::IF_A2B10G10R10:
        case GR::Graphic::IF_A2R10G10B10:
          return 4 * m_Width;
        case GR::Graphic::IF_UNKNOWN:
        case GR::Graphic::IF_COMPLEX:
          return 0;
      }
      return 0;
    }



    int ImageData::BitsProPixel() const
    {
      switch ( m_ImageFormat )
      {
        case GR::Graphic::IF_INDEX1:
          return 1;
        case GR::Graphic::IF_INDEX2:
          return 2;
        case GR::Graphic::IF_INDEX4:
          return 4;
        case GR::Graphic::IF_PALETTED:
        case GR::Graphic::IF_A8:
          return 8;
        case GR::Graphic::IF_X1R5G5B5:
          return 15;
        case GR::Graphic::IF_A1R5G5B5:
        case GR::Graphic::IF_A4R4G4B4:
        case GR::Graphic::IF_R5G6B5:
          return 16;
        case GR::Graphic::IF_R8G8B8:
          return 24;
        case GR::Graphic::IF_X8R8G8B8:
        case GR::Graphic::IF_A8R8G8B8:
        case GR::Graphic::IF_A2B10G10R10:
        case GR::Graphic::IF_A2R10G10B10:
          return 32;
        case GR::Graphic::IF_UNKNOWN:
        case GR::Graphic::IF_COMPLEX:
          break;
      }

      return 0;
    }



    void* ImageData::Data() const
    {
      return m_pData;
    }



    int ImageData::Width() const
    {
      return m_Width;
    }



    int ImageData::Height() const
    {
      return m_Height;
    }



    int ImageData::BytesProPixel() const
    {
      return BytesProPixel( m_ImageFormat );
    }



    int ImageData::BytesProPixel( GR::Graphic::eImageFormat Format )
    {
      switch ( Format )
      {
        case GR::Graphic::IF_PALETTED:
        case GR::Graphic::IF_A8:
          return 1;
        case GR::Graphic::IF_X1R5G5B5:
        case GR::Graphic::IF_A1R5G5B5:
        case GR::Graphic::IF_A4R4G4B4:
        case GR::Graphic::IF_R5G6B5:
          return 2;
        case GR::Graphic::IF_R8G8B8:
          return 3;
          break;
        case GR::Graphic::IF_X8R8G8B8:
        case GR::Graphic::IF_A8R8G8B8:
        case GR::Graphic::IF_A2B10G10R10:
        case GR::Graphic::IF_A2R10G10B10:
          return 4;
          break;
        case GR::Graphic::IF_UNKNOWN:
        case GR::Graphic::IF_COMPLEX:
          return 0;
        case GR::Graphic::IF_INDEX1:
        case GR::Graphic::IF_INDEX2:
        case GR::Graphic::IF_INDEX4:
          return 1;
      }

      return 0;
    }



    int ImageData::VirtualDataSize() const
    {
      return BytesPerLine() * m_Height;
    }



    int ImageData::DataSize() const
    {
      return m_LineOffsetInBytes * m_Height;
    }



    bool ImageData::CreateData( int iWidth, int iHeight, GR::Graphic::eImageFormat imageFormat, int iLineOffsetInBytes )
    {
      if ( m_Owner )
      {
        if ( m_pData )
        {
          delete[] (unsigned char*)m_pData;
          m_pData = NULL;
        }
      }
      m_pData             = NULL;
  
      m_ImageFormat       = imageFormat;
      m_Width             = iWidth;
      m_Height            = iHeight;
      m_BytesPerLine      = BytesPerLine();
      if ( iLineOffsetInBytes == 0 )
      {
        m_LineOffsetInBytes = BytesPerLine();
      }
      else
      {
        m_LineOffsetInBytes = iLineOffsetInBytes;
      }

      m_Owner = true;
      m_pData = new unsigned char[m_LineOffsetInBytes * iHeight];

      if ( m_pData == NULL )
      {
        dh::Log( "ImageData::CreateData new failed to alloc %d bytes", m_LineOffsetInBytes * iHeight );
        return false;
      }

      memset( m_pData, 0, m_LineOffsetInBytes * iHeight );

      m_Palette = GR::Graphic::Palette( 0 );
      if ( m_ImageFormat == GR::Graphic::IF_INDEX4 )
      {
        m_Palette.Create( 16 );
      }
      else if ( m_ImageFormat == GR::Graphic::IF_INDEX2 )
      {
        m_Palette.Create( 4 );
      }
      else if ( m_ImageFormat == GR::Graphic::IF_INDEX1 )
      {
        m_Palette.Create( 2 );
      }
      else if ( m_ImageFormat == GR::Graphic::IF_PALETTED )
      {
        m_Palette.Create( 256 );
      }

      return true;

    }



    void ImageData::Attach( int iWidth, int iHeight, int iLineOffsetInBytes, GR::Graphic::eImageFormat imageFormat, void* pData )
    {

      if ( m_Owner )
      {
        if ( m_pData )
        {
          delete[] (unsigned char*)m_pData;
          m_pData = NULL;
        }
      }

      m_pData             = NULL;

      m_Owner             = false;
      m_ImageFormat       = imageFormat;
      m_Width             = iWidth;
      m_Height            = iHeight;
      m_BytesPerLine      = BytesPerLine();
      if ( iLineOffsetInBytes == 0 )
      {
        m_LineOffsetInBytes = m_BytesPerLine;
      }
      else
      {
        m_LineOffsetInBytes = iLineOffsetInBytes;
      }

      m_pData = pData;

    }



    bool ImageData::ConvertInto( ImageData* pTargetData, bool bColorKeying, GR::u32 TransparentColor,
                                  int iX1, int iY1, int iWidth, int iHeight, GR::u32 ColorKeyReplacementColor ) const
    {
      if ( pTargetData == NULL )
      {
        dh::Log( "ImageData::ConvertInto no targetdata" );
        return false;
      }

      GR::u32   replacementColor = ColorKeyReplacementColor & 0x00ffffff;

      if ( iWidth == 0 )
      {
        iWidth = m_Width;
      }
      if ( iHeight == 0 )
      {
        iHeight = m_Height;
      }

      // dasselbe Format, direkt durchreichen
      if ( ( pTargetData->m_ImageFormat == m_ImageFormat )
      &&   ( !bColorKeying ) )
      {
        GR::u8*   pTarget;
        GR::u8*   pSource;
        for ( int i = 0; i < iHeight; i++ )
        {
          pSource = ( GR::u8*)GetRowColumnData( iX1, iY1 + i );
          pTarget = ( GR::u8*)pTargetData->m_pData + i * pTargetData->m_LineOffsetInBytes;

          memcpy( pTarget, pSource, BytesPerLine() );
        }
        return true;
      }

      switch ( m_ImageFormat )
      {
        case GR::Graphic::IF_A8:
          {
            GR::u8*    pSource = ( GR::u8*)m_pData;

            // GR::Graphic::IF_A8 nach GR::Graphic::IF_A8R8G8B8 ---------------------------------------------
            if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
            {
              GR::u32     *pTarget;
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u32*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 4;
                pSource = ( GR::u8*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  *pTarget++ = ( (GR::u32)( *pSource++ ) << 24 ) | 0x00ffffff;
                }
              }
              return true;
            }
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8 )
            {
              // hier kommt eigentlich Colorkeying, aber bei reinem Alpha ist das witzlos
              GR::u8*   pTarget;
              GR::u8*   pSource;
              for ( int i = 0; i < iHeight; i++ )
              {
                pSource = ( GR::u8*)GetRowColumnData( iX1, iY1 + i );
                pTarget = ( GR::u8*)pTargetData->m_pData + i * pTargetData->m_LineOffsetInBytes;

                memcpy( pTarget, pSource, BytesPerLine() );
              }
              return true;
            }
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
            {
              GR::u16*      pTarget;
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( GR::u8*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  if ( *pSource++ )
                  {
                    *pTarget++ = 0x7fff;
                  }
                  else
                  {
                    *pTarget++ = 0;
                  }
                }
              }
              return true;
            }
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
            {
              GR::u16*      pTarget;
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( GR::u8*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  if ( *pSource++ )
                  {
                    *pTarget++ = 0xf81f;
                  }
                  else
                  {
                    *pTarget++ = 0;
                  }
                }
              }
              return true;
            }
          }
          break;
        case GR::Graphic::IF_A8R8G8B8:
          {
            GR::u32*    pSource = (GR::u32*)m_pData;

            // GR::Graphic::IF_A8R8G8B8 nach GR::Graphic::IF_A8R8G8B8 ---------------------------------------
            if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
            {
              GR::u32     *pTarget;
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u32*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 4;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                if ( bColorKeying )
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( *pSource & 0x00ffffff ) == TransparentColor )
                    {
                      // transparent, Alpha auf 0 setzen
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget++ = replacementColor;
                        ++pSource;
                      }
                      else
                      {
                        *pTarget++ = ( *pSource++ & 0x00ffffff );
                      }
                    }
                    else
                    {
                      *pTarget++ = *pSource++;
                    }
                  }
                }
                else
                {
                  memcpy( pTarget, pSource, iWidth * 4 );
                }
              }
              return true;
            }
            // GR::Graphic::IF_A8R8G8B8 nach GR::Graphic::IF_A1R5G5B5 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A1R5G5B5 )
            {
              GR::u16*     pTarget = NULL;
          
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u32   dwColor = *pSource++;
                  if ( ( bColorKeying )
                  &&   ( ( dwColor & 0x00ffffff ) != TransparentColor ) )
                  {
                    // nicht transparent
                    *pTarget++ = (GR::u16)( ( ( dwColor & 0xf80000 ) >> 9 )
                               | ( ( dwColor & 0x00f800 ) >> 6 )
                               | ( ( dwColor & 0x0000f8 ) >> 3 )
                               | 0x8000 );
                  }
                  else if ( ColorKeyReplacementColor != 0 )
                  {
                    *pTarget++ = (GR::u16)( ( ( replacementColor & 0xf80000 ) >> 9 )
                               | ( ( replacementColor & 0x00f800 ) >> 6 )
                               | ( ( replacementColor & 0x0000f8 ) >> 3 ) );
                  }
                  else
                  {
                    *pTarget++ = ( GR::u16 )( ( ( dwColor & 0xf80000 ) >> 9 )
                                            | ( ( dwColor & 0x00f800 ) >> 6 )
                                            | ( ( dwColor & 0x0000f8 ) >> 3 ) );
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_A8R8G8B8 nach GR::Graphic::IF_X1R5G5B5 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
            {
              GR::u16*     pTarget = NULL;
          
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u32   dwColor = *pSource++;
                  if ( ( bColorKeying )
                  &&   ( ( dwColor & 0x00ffffff ) != TransparentColor ) )
                  {
                    // nicht transparent
                    *pTarget++ = (GR::u16)( ( ( dwColor & 0xf80000 ) >> 9 )
                               | ( ( dwColor & 0x00f800 ) >> 6 )
                               | ( ( dwColor & 0x0000f8 ) >> 3 ) );
                  }
                  else if ( ColorKeyReplacementColor != 0 )
                  {
                    *pTarget++ = (GR::u16)( ( ( replacementColor & 0xf80000 ) >> 9 )
                               | ( ( replacementColor & 0x00f800 ) >> 6 )
                               | ( ( replacementColor & 0x0000f8 ) >> 3 ) );
                  }
                  else
                  {
                    *pTarget++ = 0;
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_A8R8G8B8 nach GR::Graphic::IF_R5G6B5 -----------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
            {
              GR::u16*     pTarget = NULL;
          
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u32   dwColor = *pSource++;
                  *pTarget++ = (GR::u16)( ( ( dwColor & 0xf80000 ) >> 8 )
                             | ( ( dwColor & 0x00fc00 ) >> 5 )
                             | ( ( dwColor & 0x0000f8 ) >> 3 ) );
                }
              }
              return true;
            }
            // GR::Graphic::IF_A8R8G8B8 nach GR::Graphic::IF_A8 ---------------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8 )
            {
              GR::u8     *pTarget;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                if ( bColorKeying )
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( *pSource & 0x00ffffff ) == TransparentColor )
                    {
                      // transparent, Alpha auf 0 setzen
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget++ = replacementColor;
                      }
                      else
                      {
                        *pTarget++ = 0;
                      }
                      ++pSource;
                    }
                    else
                    {
                      *pTarget++ = ( GR::u8 )( ( *pSource++ & 0xff000000 ) >> 24 );
                    }
                  }
                }
                else
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *pTarget++ = ( GR::u8 )( ( *pSource++ & 0xff000000 ) >> 24 );
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_A8R8G8B8 nach GR::Graphic::IF_A4R4G4B4 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A4R4G4B4 )
            {
              GR::u16     *pTarget;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u32   dwColor = *pSource++;
                  *pTarget++ = (GR::u16)( ( ( dwColor & 0xf0000000 ) >> 16 ) 
                             | ( ( dwColor & 0x00f00000 ) >> 12 )
                             | ( ( dwColor & 0x0000f000 ) >> 8 )
                             | ( ( dwColor & 0x000000f0 ) >> 4 ) );
                }
              }
              return true;
            }
            // GR::Graphic::IF_A8R8G8B8 nach GR::Graphic::IF_R8G8B8  --------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R8G8B8 )
            {
              GR::u8*           pTarget;
              GR::u8*           pSource;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                pSource = ( GR::u8*)m_pData + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  *pTarget++ = *pSource++;
                  *pTarget++ = *pSource++;
                  *pTarget++ = *pSource++;
                  pSource++;
                }
              }
              return true;
            }
          }
          break;
        case GR::Graphic::IF_X8R8G8B8:
          {
            GR::u32*    pSource = (GR::u32*)m_pData;

            // GR::Graphic::IF_X8R8G8B8 nach GR::Graphic::IF_A8R8G8B8 ---------------------------------------
            if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
            {
              GR::u32     *pTarget;
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u32*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 4;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                if ( bColorKeying )
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( *pSource & 0x00ffffff ) == TransparentColor )
                    {
                      // transparent, Alpha auf 0 setzen
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget++ = replacementColor;
                        ++pSource;
                      }
                      else
                      {
                        *pTarget++ = ( *pSource++ & 0x00ffffff );
                      }
                    }
                    else
                    {
                      *pTarget++ = *pSource++;
                    }
                  }
                }
                else
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *pTarget++ = *pSource++ | 0xff000000;
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_X8R8G8B8 nach GR::Graphic::IF_R8G8B8  --------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R8G8B8 )
            {
              GR::u8*           pTarget;
              GR::u8*           pSource;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                pSource = ( GR::u8*)m_pData + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  *pTarget++ = *pSource++;
                  *pTarget++ = *pSource++;
                  *pTarget++ = *pSource++;
                  pSource++;
                }
              }
              return true;
            }
            // GR::Graphic::IF_X8R8G8B8 nach GR::Graphic::IF_A1R5G5B5 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A1R5G5B5 )
            {
              GR::u16*     pTarget = NULL;
          
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u32   dwColor = *pSource++;
                  if ( ( bColorKeying )
                  &&   ( ( dwColor & 0x00ffffff ) != TransparentColor ) )
                  {
                    // nicht transparent
                    *pTarget++ = (GR::u16)( ( ( dwColor & 0xf80000 ) >> 9 )
                               | ( ( dwColor & 0x00f800 ) >> 6 )
                               | ( ( dwColor & 0x0000f8 ) >> 3 )
                               | 0x8000 );
                  }
                  else if ( ColorKeyReplacementColor != 0 )
                  {
                    *pTarget++ = (GR::u16)( ( ( replacementColor & 0xf80000 ) >> 9 )
                               | ( ( replacementColor & 0x00f800 ) >> 6 )
                               | ( ( replacementColor & 0x0000f8 ) >> 3 ) );
                  }
                  else
                  {
                    *pTarget++ = ( GR::u16 )( ( ( dwColor & 0xf80000 ) >> 9 )
                      | ( ( dwColor & 0x00f800 ) >> 6 )
                      | ( ( dwColor & 0x0000f8 ) >> 3 ) );
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_X8R8G8B8 nach GR::Graphic::IF_X1R5G5B5 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
            {
              GR::u16*     pTarget = NULL;
          
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u32   dwColor = *pSource++;
                  *pTarget++ = (GR::u16)( ( ( dwColor & 0xf80000 ) >> 9 )
                             | ( ( dwColor & 0x00f800 ) >> 6 )
                             | ( ( dwColor & 0x0000f8 ) >> 3 ) );
                }
              }
              return true;
            }
            // GR::Graphic::IF_X8R8G8B8 nach GR::Graphic::IF_R5G6B5 -----------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
            {
              GR::u16*     pTarget = NULL;
          
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u32   dwColor = *pSource++;
                  *pTarget++ = (GR::u16)( ( ( dwColor & 0xf80000 ) >> 8 )
                             | ( ( dwColor & 0x00fc00 ) >> 5 )
                             | ( ( dwColor & 0x0000f8 ) >> 3 ) );
                }
              }
              return true;
            }
            // GR::Graphic::IF_X8R8G8B8 nach GR::Graphic::IF_A8 ---------------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8 )
            {
              GR::u8     *pTarget;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                if ( bColorKeying )
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( *pSource & 0x00ffffff ) == TransparentColor )
                    {
                      // transparent, Alpha auf 0 setzen
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget++ = replacementColor;
                      }
                      else
                      {
                        *pTarget++ = 0;
                      }
                      ++pSource;
                    }
                    else
                    {
                      *pTarget++ = 0xff;
                    }
                  }
                }
                else
                {
                  memset( pTarget, 0xff, iWidth );
                }
              }
              return true;
            }
            // GR::Graphic::IF_X8R8G8B8 nach GR::Graphic::IF_A4R4G4B4 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A4R4G4B4 )
            {
              GR::u16     *pTarget;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = (GR::u32*)m_pData + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 4;

                if ( bColorKeying )
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u32   dwColor = *pSource++;
                    if ( ( dwColor & 0x00ffffff ) == TransparentColor )
                    {
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget++ = ( GR::u16 )( ( ( replacementColor & 0x00f00000 ) >> 12 )
                          | ( ( replacementColor & 0x0000f000 ) >> 8 )
                          | ( ( replacementColor & 0x000000f0 ) >> 4 ) );
                      }
                      else
                      {
                        *pTarget++ = ( GR::u16 )( ( ( dwColor & 0x00f00000 ) >> 12 )
                          | ( ( dwColor & 0x0000f000 ) >> 8 )
                          | ( ( dwColor & 0x000000f0 ) >> 4 ) );
                      }
                    }
                    else
                    {
                      *pTarget++ = (GR::u16)( 0xf000 
                               | ( ( dwColor & 0x00f00000 ) >> 12 )
                               | ( ( dwColor & 0x0000f000 ) >> 8 )
                               | ( ( dwColor & 0x000000f0 ) >> 4 ) );
                    }
                  }
                }
                else
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u32   dwColor = *pSource++;
                    *pTarget++ = (GR::u16)( 0xf000 
                               | ( ( dwColor & 0x00f00000 ) >> 12 )
                               | ( ( dwColor & 0x0000f000 ) >> 8 )
                               | ( ( dwColor & 0x000000f0 ) >> 4 ) );
                  }
                }
              }
              return true;
            }
          }
          break;
        case GR::Graphic::IF_R8G8B8:
          {
            GR::u8*    pSource = ( GR::u8*)m_pData;

            // GR::Graphic::IF_R8G8B8 nach GR::Graphic::IF_R5G6B5 -----------------------------------------
            if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
            {
              GR::u16*     pTarget;
              GR::u16      wResult;
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( ( GR::u8*)m_pData ) + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  wResult = ( ( GR::u16( *pSource++ & 0xf8 ) ) >> 3 );
                  wResult |= ( ( GR::u16( *pSource++ & 0xfc ) ) << 3 );
                  wResult |= ( ( GR::u16( *pSource++ & 0xf8 ) ) << 8 );

                  //wResult = ( ( GR::u16( *pSource++ & 0xf8 ) ) >> 3 );
                  //pSource += 2;
                  *pTarget++ = wResult;
                }
              }
              return true;
            }
            // GR::Graphic::IF_R8G8B8 nach GR::Graphic::IF_X1R5G5B5 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
            {
              GR::u16*     pTarget;
              GR::u16      wResult;
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( ( GR::u8*)m_pData ) + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  wResult = ( ( GR::u16( *pSource++ & 0xf8 ) ) >> 3 );
                  wResult |= ( ( GR::u16( *pSource++ & 0xf8 ) ) << 2 );
                  wResult |= ( ( GR::u16( *pSource++ & 0xf8 ) ) << 7 );

                  *pTarget++ = wResult;
                }
              }
              return true;
            }
            // GR::Graphic::IF_R8G8B8 nach GR::Graphic::IF_A1R5G5B5 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A1R5G5B5 )
            {
              GR::u16*     pTarget;
              GR::u16      wResult,
                        wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                          | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                          | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );
              GR::u16      replacement16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                                          | ( ( replacementColor & 0x00f800 ) >> 6 )
                                          | ( ( replacementColor & 0x0000f8 ) >> 3 ) );
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( GR::u8*)m_pData + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  wResult = ( ( GR::u16( *pSource++ & 0xf8 ) ) >> 3 );
                  wResult |= ( ( GR::u16( *pSource++ & 0xf8 ) ) << 2 );
                  wResult |= ( ( GR::u16( *pSource++ & 0xf8 ) ) << 7 );

                  if ( wResult != wTransparentColor )
                  {
                    wResult |= 0x8000;
                  }
                  else if ( ColorKeyReplacementColor != 0 )
                  {
                    wResult = replacement16;
                  }
                  *pTarget++ = wResult;
                }
              }
              return true;
            }
            // GR::Graphic::IF_R8G8B8 nach GR::Graphic::IF_X8R8G8B8  --------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X8R8G8B8 )
            {
              GR::u8*           pTarget;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                pSource = ( GR::u8*)m_pData + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  *pTarget++ = *pSource++;
                  *pTarget++ = *pSource++;
                  *pTarget++ = *pSource++;
                  *pTarget++ = 0xff;
                }
              }
              return true;
            }
            // GR::Graphic::IF_R8G8B8 nach GR::Graphic::IF_A8R8G8B8  --------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
            {
              GR::u8*           pTarget;

              if ( bColorKeying )
              {
                GR::u32   dwColor = 0x00000000;

                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( GR::u8*)m_pData + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *(GR::u8*)( &dwColor ) = *pSource++;
                    *( (GR::u8*)( &dwColor ) + 1 ) = *pSource++;
                    *( (GR::u8*)( &dwColor ) + 2 ) = *pSource++;

                    if ( ( TransparentColor & 0x00ffffff ) == dwColor )
                    {
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *( GR::u32* )pTarget = replacementColor;
                      }
                      else
                      {
                        *( GR::u32* )pTarget = dwColor;
                      }
                    }
                    else
                    {
                      *(GR::u32*)pTarget = dwColor | 0xff000000;
                    }
                    pTarget += 4;
                  }
                }
              }
              else
              {
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( GR::u8*)m_pData + 3 * iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *pTarget++ = *pSource++;
                    *pTarget++ = *pSource++;
                    *pTarget++ = *pSource++;
                    pTarget++;
                  }
                }
              }
              return true;
            }
          }
          break;
        case GR::Graphic::IF_R5G6B5:
          {
            GR::u16*    pSource = (GR::u16*)m_pData;

            // GR::Graphic::IF_R5G6B5 nach GR::Graphic::IF_X1R5G5B5 ---------------------------------------
            if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
            {
              GR::u16*   pTarget;
            

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;

                  *pTarget++ = ( ( dwColor & 0xf800 ) >> 1 ) + ( ( dwColor & 0x7c0) >> 1 ) + ( dwColor & 0x1f );
                }
              }
              return true;
            }
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A1R5G5B5 )
            {
              GR::u16*   pTarget;
            
              GR::u16      wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                                  | ( ( TransparentColor & 0x00fc00 ) >> 5 )
                                                  | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );

              GR::u16     replacementColor16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                                                          | ( ( replacementColor & 0x00fc00 ) >> 5 )
                                                          | ( ( replacementColor & 0x0000f8 ) >> 3 ) );


              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;

                  if ( ( bColorKeying )
                  &&   ( dwColor == wTransparentColor ) )
                  {
                    if ( ColorKeyReplacementColor != 0 )
                    {
                      *pTarget++ = replacementColor16;
                    }
                    else
                    {
                      *pTarget++ = ( ( dwColor & 0xf800 ) >> 1 ) + ( ( dwColor & 0x7c0 ) >> 1 ) + ( dwColor & 0x1f );
                    }
                  }
                  else
                  {
                    *pTarget++ = 0x8000 | ( ( dwColor & 0xf800 ) >> 1 ) + ( ( dwColor & 0x7c0 ) >> 1 ) + ( dwColor & 0x1f );
                  }
                }
              }
              return true;
            }
          }
          break;
        case GR::Graphic::IF_A1R5G5B5:
          {
            GR::u16*    pSource = (GR::u16*)m_pData;

            if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A1R5G5B5 )
            {
              // gleiches Format, nur Colorkeying
              GR::u16*     pTarget;
              GR::u16      wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                                  | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                                  | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );
          
              GR::u16      replacementColor16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                                                  | ( ( replacementColor & 0x00f800 ) >> 6 )
                                                  | ( ( replacementColor & 0x0000f8 ) >> 3 ) );

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;
                  if ( ( dwColor & 0x7fff ) == wTransparentColor )
                  {
                    if ( ColorKeyReplacementColor != 0 )
                    {
                      *pTarget++ = replacementColor16;
                    }
                    else
                    {
                      *pTarget++ = dwColor;
                    }
                  }
                  else
                  {
                    *pTarget++ = dwColor | 0x8000;
                  }
                }
              }
              return true;
            }
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
            {
              // fast gleiches Format, nur kein Colorkeying
              GR::u16*     pTarget;
              GR::u16      wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                                  | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                                  | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );
              GR::u16      replacementColor16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                | ( ( replacementColor & 0x00f800 ) >> 6 )
                | ( ( replacementColor & 0x0000f8 ) >> 3 ) );

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;
                  if ( ( dwColor & 0x7fff ) == wTransparentColor )
                  {
                    if ( ColorKeyReplacementColor != 0 )
                    {
                      *pTarget++ = replacementColor16;
                    }
                    else
                    {
                      *pTarget++ = dwColor;
                    }
                  }
                  else
                  {
                    *pTarget++ = dwColor;
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_A1R5G5B5 nach GR::Graphic::IF_R5G6B5 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
            {
              GR::u16*   pTarget;
            
              GR::u16      wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                                  | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                                  | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );
              GR::u16      replacementColor16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                | ( ( replacementColor & 0x00f800 ) >> 6 )
                | ( ( replacementColor & 0x0000f8 ) >> 3 ) );

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;

                  if ( ( ( dwColor & 0x8000 ) == 0 )
                  ||   ( ( dwColor & 0x7fff ) == wTransparentColor ) )
                  {
                    if ( ColorKeyReplacementColor != 0 )
                    {
                      *pTarget++ = replacementColor16;
                    }
                    else
                    {
                      *pTarget++ = ( ( dwColor & 0x7c00 ) << 1 )
                        + ( ( ( ( ( dwColor & 0x3e0 ) >> 5 ) * 63 + 15 ) / 31 ) << 5 )
                        + ( dwColor & 0x1f );
                    }
                  }
                  else
                  {
                    *pTarget++ = ( ( dwColor & 0x7c00 ) << 1 ) 
                              + ( ( ( ( ( dwColor & 0x3e0 ) >> 5 ) * 63 + 15 ) / 31 ) << 5 )
                              + ( dwColor & 0x1f );
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_A1R5G5B5 to GR::Graphic::IF_A8R8G8B8 ---------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
            {
              if ( bColorKeying )
              {
                GR::u8*   pTarget = NULL;
                GR::u16   wTransparentColor = ( GR::u16 )( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                                         | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                                         | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );

                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( ( GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( *pSource & 0x8000 )
                    &&   ( ( *pSource & 0x7fff ) == wTransparentColor ) )
                    {
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget++ = ( ( replacementColor & 0x001f ) * 255 / 31 );
                        *pTarget++ = ( ( replacementColor & 0x03e0 ) >> 5 ) * 255 / 31;
                        *pTarget++ = ( ( replacementColor & 0x7c00 ) >> 10 ) * 255 / 31;
                      }
                      else
                      {
                        *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                        *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                        *pTarget++ = ( ( *pSource & 0x7c00 ) >> 10 ) * 255 / 31;
                      }
                      *pTarget++ = 0;
                    }
                    else
                    {
                      *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                      *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                      *pTarget++ = ( ( *pSource & 0x7c00 ) >> 10 ) * 255 / 31;
                      if ( *pSource & 0x8000 )
                      {
                        *pTarget++ = 0xff;
                      }
                      else
                      {
                        *pTarget++ = 0x00;
                      }
                    }
                    pSource++;
                  }
                }
              }
              else
              {
                GR::u8*           pTarget;

                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                    *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                    *pTarget++ = ( ( *pSource++ & 0x7c00 ) >> 10 ) * 255 / 31;
                    if ( *pSource & 0x8000 )
                    {
                      *pTarget++ = 0xff;
                    }
                    else
                    {
                      *pTarget++ = 0x00;
                    }
                  }
                }
              }
              return true;
            }
          }
          break;
        case GR::Graphic::IF_X1R5G5B5:
          {
            GR::u16*    pSource = (GR::u16*)m_pData;

            // GR::Graphic::IF_X1R5G5B5 nach GR::Graphic::IF_R5G6B5 ---------------------------------------
            if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
            {
              GR::u16*   pTarget;
            

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;

                  *pTarget++ = 
                          ( ( dwColor & 0x7c00 ) << 1 ) 
                            + ( ( ( ( ( dwColor & 0x3e0 ) >> 5 ) * 63 + 15 ) / 31 ) << 5 )
                            + ( dwColor & 0x1f );
                }
              }
              return true;
            }
            // GR::Graphic::IF_X1R5G5B5 nach GR::Graphic::IF_A4R4G4B4  ------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A4R4G4B4 )
            {
              GR::u16*       pTarget;
              
              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 * 2 + ( i + iY1 ) * m_LineOffsetInBytes;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;
                  *pTarget++ = (GR::u16)( 0xf000
                             | ( ( dwColor & 0x7800 ) >> 3 )
                             | ( ( dwColor & 0x03c0 ) >> 2 )
                             | ( ( dwColor & 0x001e ) >> 1 ) );
                }
              }
              return true;
            }
            // GR::Graphic::IF_X1R5G5B5 nach GR::Graphic::IF_R8G8B8  --------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R8G8B8 )
            {
              GR::u8*           pTarget;

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                  *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                  *pTarget++ = ( ( *pSource++ & 0x7c00 ) >> 10 ) * 255 / 31;
                }
              }
              return true;
            }
            // GR::Graphic::IF_X1R5G5B5 nach GR::Graphic::IF_X8R8G8B8  ------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X8R8G8B8 )
            {
              if ( bColorKeying )
              {
                GR::u8*   pTarget;
                GR::u16   wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                                       | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                                       | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );

                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( *pSource == wTransparentColor )
                    {
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget = replacementColor;
                        pTarget += 4;
                      }
                      else
                      {
                        *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                        *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                        *pTarget++ = ( ( *pSource & 0x7c00 ) >> 10 ) * 255 / 31;
                        *pTarget++ = 0xff;
                      }
                    }
                    else
                    {
                      *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                      *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                      *pTarget++ = ( ( *pSource & 0x7c00 ) >> 10 ) * 255 / 31;
                      *pTarget++ = 0xff;
                    }
                    pSource++;
                  }
                }
              }
              else
              {
                GR::u8*           pTarget;

                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                    *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                    *pTarget++ = ( ( *pSource++ & 0x7c00 ) >> 10 ) * 255 / 31;
                    pTarget++;
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_X1R5G5B5 nach GR::Graphic::IF_A8R8G8B8  ------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
            {
              if ( bColorKeying )
              {
                GR::u8*   pTarget;
                GR::u16   wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                                       | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                                       | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );

                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( *pSource == wTransparentColor )
                    {
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget = replacementColor;
                        pTarget += 4;
                      }
                      else
                      {
                        *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                        *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                        *pTarget++ = ( ( *pSource & 0x7c00 ) >> 10 ) * 255 / 31;

                        *pTarget++ = 0;
                      }
                    }
                    else
                    {
                      *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                      *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                      *pTarget++ = ( ( *pSource & 0x7c00 ) >> 10 ) * 255 / 31;

                      *pTarget++ = 0xff;
                    }
                    pSource++;
                  }
                }
              }
              else
              {
                GR::u8*           pTarget;

                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *pTarget++ = ( ( *pSource & 0x001f ) * 255 / 31 );
                    *pTarget++ = ( ( *pSource & 0x03e0 ) >> 5 ) * 255 / 31;
                    *pTarget++ = ( ( *pSource++ & 0x7c00 ) >> 10 ) * 255 / 31;
                    *pTarget++ = 0xff;
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_X1R5G5B5 nach GR::Graphic::IF_A1R5G5B5  ------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A1R5G5B5 )
            {
              GR::u16*     pTarget;
              GR::u16      wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                          | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                          | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );
              GR::u16      replacementColor16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                | ( ( replacementColor & 0x00f800 ) >> 6 )
                | ( ( replacementColor & 0x0000f8 ) >> 3 ) );
          

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                if ( bColorKeying )
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u16   dwColor = *pSource++;
                    if ( ( dwColor & 0x7fff ) == wTransparentColor )
                    {
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *pTarget++ = replacementColor16;
                      }
                      else
                      {
                        *pTarget++ = dwColor;
                      }
                    }
                    else
                    {
                      *pTarget++ = dwColor | 0x8000;
                    }
                  }
                }
                else
                {
                  for ( int j = 0; j < iWidth; j++ )
                  {
                    *pTarget++ = *pSource++ | 0x8000;
                  }
                }
              }
              return true;
            }
            // GR::Graphic::IF_X1R5G5B5 nach GR::Graphic::IF_X1R5G5B5  ------------------------------------
            else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
            {
              GR::u16*     pTarget;
              GR::u16      wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                          | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                          | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );
              GR::u16      replacementColor16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                | ( ( replacementColor & 0x00f800 ) >> 6 )
                | ( ( replacementColor & 0x0000f8 ) >> 3 ) );
          

              for ( int i = 0; i < iHeight; i++ )
              {
                pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                pSource = ( (GR::u16*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes / 2;

                for ( int j = 0; j < iWidth; j++ )
                {
                  GR::u16   dwColor = *pSource++;
                  if ( ( dwColor & 0x7fff ) == wTransparentColor )
                  {
                    if ( ColorKeyReplacementColor != 0 )
                    {
                      *pTarget++ = replacementColor16;
                    }
                    else
                    {
                      *pTarget++ = dwColor;
                    }
                  }
                  else
                  {
                    *pTarget++ = dwColor;
                  }
                }
              }
              return true;
            }
          }
          break;
        case GR::Graphic::IF_PALETTED:
          {
            GR::Graphic::Palette    PalDummy;

            GR::Graphic::Palette*   pWorkPal = &PalDummy;

            GR::i32                 transparentIndex = -1;

            for ( int i = 0; i < 256; i++ )
            {
              int     r = m_Palette.Red( i );
              int     g = m_Palette.Green( i );
              int     b = m_Palette.Blue( i );

              if ( ( bColorKeying )
              &&   ( transparentIndex == -1 )
              &&   ( ( TransparentColor & 0x00ffffff ) == ( ( r << 16 ) | ( g << 8 ) | b ) ) )
              {
                transparentIndex = i;
              }
              pWorkPal->SetColor( i, r, g, b );
            }
            if ( pWorkPal )
            {
              GR::u8*    pSource = (GR::u8*)m_pData;

              // GR::Graphic::IF_PALETTED nach GR::Graphic::IF_X1R5G5B5 -------------------------------------
              if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
              {
                GR::u16*       pTarget;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                  pSource = ( (GR::u8*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u8    ucColor = *pSource++;

                    *pTarget++ = 
                          ( ( pWorkPal->Red(   ucColor ) >> 3 ) << 10 )
                        + ( ( pWorkPal->Green( ucColor ) >> 3 ) << 5 )
                        + ( ( pWorkPal->Blue(  ucColor ) >> 3 ) );
                  }
                }
                return true;
              }
              // GR::Graphic::IF_PALETTED nach GR::Graphic::IF_A1R5G5B5 -------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A1R5G5B5 )
              {
                GR::u16*       pTarget;
           
                GR::u16      wTransparentColor = (GR::u16)( ( ( TransparentColor & 0xf80000 ) >> 9 )
                                            | ( ( TransparentColor & 0x00f800 ) >> 6 )
                                            | ( ( TransparentColor & 0x0000f8 ) >> 3 ) );
                GR::u16      replacementColor16 = ( GR::u16 )( ( ( replacementColor & 0xf80000 ) >> 9 )
                  | ( ( replacementColor & 0x00f800 ) >> 6 )
                  | ( ( replacementColor & 0x0000f8 ) >> 3 ) );
            
            
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                  pSource = ( (GR::u8*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  if ( bColorKeying )
                  {
                    for ( int j = 0; j < iWidth; j++ )
                    {
                      GR::u8    ucColor = *pSource++;

                        GR::u16    wColor = ( ( pWorkPal->Red(   ucColor ) >> 3 ) << 10 )
                          + ( ( pWorkPal->Green( ucColor ) >> 3 ) << 5 )
                          + ( ( pWorkPal->Blue(  ucColor ) >> 3 ) );
                      if ( wColor == wTransparentColor )
                      {
                        if ( ColorKeyReplacementColor != 0 )
                        {
                          *pTarget++ = replacementColor16;
                        }
                        else
                        {
                          *pTarget++ = wColor;
                        }
                      }
                      else
                      {
                        *pTarget++ = wColor | 0x8000;
                      }
                    }
                  }
                  else
                  {
                    memcpy( pTarget, pSource, iWidth );
                  }
                }
                return true;
              }
              // GR::Graphic::IF_PALETTED nach GR::Graphic::IF_R5G6B5 ---------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
              {
                GR::u16*       pTarget;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u16*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes / 2;
                  pSource = ( (GR::u8*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u8    ucColor = *pSource++;
                    *pTarget++ = 
                          ( ( pWorkPal->Red(   ucColor ) >> 3 ) << 11 )
                        + ( ( pWorkPal->Green( ucColor ) >> 2 ) << 5 )
                        + ( ( pWorkPal->Blue(  ucColor ) >> 3 ) );
                  }
                }
                return true;
              }
              // GR::Graphic::IF_PALETTED nach GR::Graphic::IF_R8G8B8 ---------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R8G8B8 )
              {
                GR::u8*       pTarget;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u8*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u8    ucColor = *pSource++;

                    *pTarget++ = pWorkPal->Blue( ucColor );
                    *pTarget++ = pWorkPal->Green( ucColor );
                    *pTarget++ = pWorkPal->Red( ucColor );
                  }
                }
                return true;
              }
              // GR::Graphic::IF_PALETTED nach GR::Graphic::IF_X8R8G8B8 -------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X8R8G8B8 )
              {
                GR::u8*       pTarget;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u8*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u8    ucColor = *pSource++;

                    *pTarget++ = pWorkPal->Blue( ucColor );
                    *pTarget++ = pWorkPal->Green( ucColor );
                    *pTarget++ = pWorkPal->Red( ucColor );
                    pTarget++;
                  }
                }
                return true;
              }
              // GR::Graphic::IF_PALETTED nach GR::Graphic::IF_A8R8G8B8 -------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
              {
                GR::u8*       pTarget;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = ( (GR::u8*)m_pData ) + iX1 + ( i + iY1 ) * m_LineOffsetInBytes;

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    GR::u8    ucColor = *pSource++;

                    if ( ( bColorKeying )
                    &&   ( ucColor == transparentIndex ) )
                    {
                      if ( ColorKeyReplacementColor != 0 )
                      {
                        *( GR::u32* )pTarget = replacementColor;
                      }
                      else
                      {
                        *pTarget++ = pWorkPal->Red( ucColor );
                        *pTarget++ = pWorkPal->Green( ucColor );
                        *pTarget++ = pWorkPal->Blue( ucColor );
                        *pTarget++ = 0;
                      }
                    }
                    else
                    {
                      *pTarget++ = pWorkPal->Red( ucColor );
                      *pTarget++ = pWorkPal->Green( ucColor );
                      *pTarget++ = pWorkPal->Blue( ucColor );

                      *pTarget++ = 0xff;
                    }
                  }
                }
                return true;
              }
            }
          }
          break;
        case GR::Graphic::IF_INDEX4:
          {
            GR::Graphic::Palette    PalDummy;

            GR::Graphic::Palette*   pWorkPal = &PalDummy;

            for ( int i = 0; i < 16; i++ )
            {
              pWorkPal->SetColor( i, 
                                  m_Palette.Red( i ),
                                  m_Palette.Green( i ),
                                  m_Palette.Blue( i ) );
            }
            if ( pWorkPal )
            {
              GR::u8*    pSource = (GR::u8*)m_pData;

              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_INDEX8 -------------------------------------
              if ( pTargetData->m_ImageFormat == GR::Graphic::IF_INDEX8 )
              {
                GR::u8*       pTarget;
                GR::u8*       pSource;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( j & 1 )
                    {
                      *pTarget++ = ( *pSource++ ) & 0x0f;
                    }
                    else
                    {
                      *pTarget++ = ( *pSource ) >> 4;
                    }
                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_X1R5G5B5 ------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
              {
                GR::u16*       pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u16*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( j & 1 )
                    {
                      ucColor = ( *pSource++ ) & 0x0f;
                    }
                    else
                    {
                      ucColor = ( *pSource ) >> 4;
                    }
                    *pTarget++ = (GR::u16)( 
                          ( ( pWorkPal->Red(   ucColor ) >> 3 ) << 10 )
                        + ( ( pWorkPal->Green( ucColor ) >> 3 ) << 5 )
                        + ( ( pWorkPal->Blue(  ucColor ) >> 3 ) ) );

                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_R5G6B5 --------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
              {
                GR::u16*       pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u16*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( j & 1 )
                    {
                      ucColor = ( *pSource++ ) & 0x0f;
                    }
                    else
                    {
                      ucColor = ( *pSource ) >> 4;
                    }

                    *pTarget++ = (GR::u16)( 
                          ( ( pWorkPal->Red(   ucColor ) >> 3 ) << 11 )
                        + ( ( pWorkPal->Green( ucColor ) >> 2 ) << 5 )
                        + ( ( pWorkPal->Blue(  ucColor ) >> 3 ) ) );

                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_R8G8B8 --------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R8G8B8 )
              {
                GR::u8*       pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u8*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( j & 1 )
                    {
                      ucColor = ( *pSource++ ) & 0x0f;
                    }
                    else
                    {
                      ucColor = ( *pSource ) >> 4;
                    }
                    *pTarget++ = pWorkPal->Blue( ucColor );
                    *pTarget++ = pWorkPal->Green( ucColor );
                    *pTarget++ = pWorkPal->Red( ucColor );
                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_X8R8G8B8 ------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X8R8G8B8 )
              {
                GR::u32*      pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u32*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( j & 1 )
                    {
                      ucColor = ( *pSource++ ) & 0x0f;
                    }
                    else
                    {
                      ucColor = ( *pSource ) >> 4;
                    }
                    *pTarget++ = ( 0xff000000 
                                + ( pWorkPal->Red( ucColor ) << 16 ) 
                                + ( pWorkPal->Green( ucColor ) << 8 ) 
                                + ( pWorkPal->Blue( ucColor ) ) );
                  }
                }
                return true;
              }
            }
          }
          break;
        case GR::Graphic::IF_INDEX1:
          {
            GR::Graphic::Palette    PalDummy;

            GR::Graphic::Palette*   pWorkPal = &PalDummy;

            for ( int i = 0; i < 2; i++ )
            {
              pWorkPal->SetColor( i, 
                                  m_Palette.Red( i ),
                                  m_Palette.Green( i ),
                                  m_Palette.Blue( i ) );
            }
            if ( pWorkPal )
            {
              GR::u8*    pSource = (GR::u8*)m_pData;

              // GR::Graphic::IF_INDEX1 nach GR::Graphic::IF_INDEX8 -------------------------------------
              if ( pTargetData->m_ImageFormat == GR::Graphic::IF_INDEX8 )
              {
                GR::u8*       pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = ( (GR::u8*)pTargetData->m_pData ) + i * pTargetData->m_LineOffsetInBytes;
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( j & 7 ) == 7 )
                    {
                      ucColor = *pSource++ & 0x01;
                    }
                    else
                    {
                      ucColor = ( *pSource & ( 1 << ( 7 - j & 7 ) ) ) >> ( 7 - j & 7 );
                    }

                    if ( j & 1 )
                    {
                      *pTarget++ |= ucColor;
                    }
                    else
                    {
                      *pTarget = ( ucColor << 4 );
                    }
                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_X1R5G5B5 ------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X1R5G5B5 )
              {
                GR::u16*       pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u16*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( j & 7 ) == 7 )
                    {
                      ucColor = *pSource++ & 0x01;
                    }
                    else
                    {
                      ucColor = ( *pSource & ( 1 << ( 7 - j & 7 ) ) ) >> ( 7 - j & 7 );
                    }
                    *pTarget++ = (GR::u16)( 
                          ( ( pWorkPal->Red(   ucColor ) >> 3 ) << 10 )
                        + ( ( pWorkPal->Green( ucColor ) >> 3 ) << 5 )
                        + ( ( pWorkPal->Blue(  ucColor ) >> 3 ) ) );

                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_R5G6B5 --------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R5G6B5 )
              {
                GR::u16*       pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u16*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( j & 7 ) == 7 )
                    {
                      ucColor = *pSource++ & 0x01;
                    }
                    else
                    {
                      ucColor = ( *pSource & ( 1 << ( 7 - j & 7 ) ) ) >> ( 7 - j & 7 );
                    }

                    *pTarget++ = (GR::u16)( 
                          ( ( pWorkPal->Red(   ucColor ) >> 3 ) << 11 )
                        + ( ( pWorkPal->Green( ucColor ) >> 2 ) << 5 )
                        + ( ( pWorkPal->Blue(  ucColor ) >> 3 ) ) );

                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_R8G8B8 --------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_R8G8B8 )
              {
                GR::u8*       pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u8*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( j & 7 ) == 7 )
                    {
                      ucColor = *pSource++ & 0x01;
                    }
                    else
                    {
                      ucColor = ( *pSource & ( 1 << ( 7 - j & 7 ) ) ) >> ( 7 - j & 7 );
                    }
                    *pTarget++ = pWorkPal->Blue( ucColor );
                    *pTarget++ = pWorkPal->Green( ucColor );
                    *pTarget++ = pWorkPal->Red( ucColor );
                  }
                }
                return true;
              }
              // GR::Graphic::IF_INDEX4 nach GR::Graphic::IF_X8R8G8B8 ------------------------------------
              else if ( pTargetData->m_ImageFormat == GR::Graphic::IF_X8R8G8B8 )
              {
                GR::u32*      pTarget;
                GR::u8*       pSource;
                GR::u8        ucColor;
                
                for ( int i = 0; i < iHeight; i++ )
                {
                  pTarget = (GR::u32*)pTargetData->GetRowData( i );
                  pSource = (GR::u8*)GetRowData( i );

                  for ( int j = 0; j < iWidth; j++ )
                  {
                    if ( ( j & 7 ) == 7 )
                    {
                      ucColor = *pSource++ & 0x01;
                    }
                    else
                    {
                      ucColor = ( *pSource & ( 1 << ( 7 - j & 7 ) ) ) >> ( 7 - j & 7 );
                    }
                    *pTarget++ = ( 0xff000000 
                                + ( pWorkPal->Red( ucColor ) << 16 ) 
                                + ( pWorkPal->Green( ucColor ) << 8 ) 
                                + ( pWorkPal->Blue( ucColor ) ) );
                  }
                }
                return true;
              }
            }
          }
          break;
        case GR::Graphic::IF_UNKNOWN:
        case GR::Graphic::IF_COMPLEX:
        case GR::Graphic::IF_INDEX2:
        case GR::Graphic::IF_A4R4G4B4:
        case GR::Graphic::IF_A2B10G10R10:
        case GR::Graphic::IF_A2R10G10B10:
          break;
      }

      dh::Log( "ImageData::ConvertInto missing conversion code from %d to %d", m_ImageFormat, pTargetData->m_ImageFormat );
      return false;
    }



    ImageData* ImageData::ConvertTo( GR::Graphic::eImageFormat imageFormat, int iLineOffsetInBytes, bool bColorKeying, GR::u32 TransparentColor,
                                     int iX1, int iY1, int iWidth, int iHeight, GR::u32 ColorKeyReplacementColor )
    {
      if ( iLineOffsetInBytes == 0 )
      {
        iLineOffsetInBytes = m_Width * BytesProPixel();
      }
      if ( iWidth == 0 )
      {
        iWidth = m_Width;
      }
      if ( iHeight == 0 )
      {
        iHeight = m_Height;
      }

      ImageData*   pNewData = new ImageData();

      pNewData->CreateData( iWidth, iHeight, imageFormat, iLineOffsetInBytes );

      if ( ( m_ImageFormat == imageFormat )
      &&   ( ( !bColorKeying )
      ||     ( !FormatHasAlpha( imageFormat ) ) ) )
      {
        // Format stimmt schon
        for ( int i = 0; i < Height(); ++i )
        {
          memcpy( pNewData->GetRowData( i ), GetRowData( i ), BytesPerLine() );
        }
      }
      else if ( !ConvertInto( pNewData, bColorKeying, TransparentColor, iX1, iY1, iWidth, iHeight, ColorKeyReplacementColor ) )
      {
        delete pNewData;
        dh::Log( "ImageData::ConvertTo Unsupported Conversion %d to %d.\n", m_ImageFormat, imageFormat );
        return NULL;
      }

      return pNewData;
    }



    bool ImageData::ConvertSelfTo( GR::Graphic::eImageFormat imageFormat, int iLineOffsetInBytes, bool bColorKeying, GR::u32 TransparentColor,
                                    int iX1, int iY1, int iWidth, int iHeight,
                                    GR::u32 ColorKeyReplacementColor )
    {

      GR::Graphic::eImageFormat    fDummy = m_ImageFormat;

      if ( ( m_ImageFormat == imageFormat )
      &&   ( ( !bColorKeying )
      ||     ( !FormatHasAlpha( imageFormat ) ) ) )
      {
        // Format stimmt schon
        return true;
      }

      m_ImageFormat       = imageFormat;
      int   iBPL          = BytesPerLine();
      m_ImageFormat       = fDummy;

      if ( iLineOffsetInBytes == 0 )
      {
        iLineOffsetInBytes = iBPL;
      }
      if ( iWidth == 0 )
      {
        iWidth = m_Width;
      }
      if ( iHeight == 0 )
      {
        iHeight = m_Height;
      }

      ImageData*   pNewData = new ImageData();

      pNewData->CreateData( iWidth, iHeight, imageFormat, iLineOffsetInBytes );

      if ( !ConvertInto( pNewData, bColorKeying, TransparentColor, iX1, iY1, iWidth, iHeight, ColorKeyReplacementColor ) )
      {
        delete pNewData;
        dh::Log( "ImageData::ConvertSelfTo Unsupported Conversion %d to %d (%d,%x)", m_ImageFormat, imageFormat, bColorKeying, TransparentColor );
        return false;
      }

      if ( m_Owner )
      {
        delete[] (unsigned char*)m_pData;
      }

      m_ImageFormat       = imageFormat;
      m_BytesPerLine      = BytesPerLine();
      if ( iLineOffsetInBytes == 0 )
      {
        m_LineOffsetInBytes = m_BytesPerLine;
      }
      else
      {
        m_LineOffsetInBytes = iLineOffsetInBytes;
      }

      m_pData = new unsigned char[m_LineOffsetInBytes * iHeight];

      if ( m_pData == NULL )
      {
        dh::Log( "ImageData::ConvertSelfTo new failed to alloc %d bytes", m_LineOffsetInBytes * iHeight );
        return false;
      }

      m_Owner = true;
      m_TransparentColor = TransparentColor;

      memcpy( m_pData, pNewData->m_pData, pNewData->DataSize() );

      if ( m_ImageFormat == GR::Graphic::IF_PALETTED )
      {
        m_Palette = pNewData->m_Palette;
      }

      delete pNewData;

      return true;
    }



    void* ImageData::GetRowData( int iY ) const
    {
      if ( iY < 0 )
      {
        return m_pData;
      }
      if ( iY >= m_Height )
      {
        return m_pData;
      }
      return (void*)( ( ( GR::u8*)m_pData ) + iY * m_LineOffsetInBytes );
    }



    void* ImageData::GetRowColumnData( int iX, int iY ) const
    {
      if ( iY < 0 )
      {
        return m_pData;
      }
      if ( iY >= m_Height )
      {
        return m_pData;
      }
      if ( ( iX < 0 )
      ||   ( iX >= m_Width ) )
      {
        return m_pData;
      }

      if ( BitsProPixel() < 8 )
      {
        return (void*)( ( ( GR::u8*)m_pData ) + iY * m_LineOffsetInBytes + ( BitsProPixel() * iX ) / 8 );
      }
      return (void*)( ( ( GR::u8*)m_pData ) + iY * m_LineOffsetInBytes + BytesProPixel() * iX  );

    }



    GR::u32 ImageData::GetPixel( int iX, int iY ) const
    {

      GR::u8*   pData = ( GR::u8*)GetRowColumnData( iX, iY );

      switch ( m_ImageFormat )
      {
        case GR::Graphic::IF_A8:
        case GR::Graphic::IF_PALETTED:
          return *pData;
        case GR::Graphic::IF_A1R5G5B5:
        case GR::Graphic::IF_X1R5G5B5:
        case GR::Graphic::IF_R5G6B5:
        case GR::Graphic::IF_A4R4G4B4:
          return *(GR::u16*)pData;
        case GR::Graphic::IF_A8R8G8B8:
        case GR::Graphic::IF_X8R8G8B8:
          return *(GR::u32*)pData;
        case GR::Graphic::IF_R8G8B8:
          {
            GR::u32   dwResult = pData[0] + ( pData[1] << 8 ) + ( pData[2] << 16 );
            return dwResult;
          }
        default:
          dh::Log( "ImageData::GetPixel unsupported format %d\n", m_ImageFormat );
          return 0;
      }
      return 0;

    }



    GR::u32 ImageData::GetPixelRGB( int iX, int iY ) const
    {

      GR::u8*   pData = ( GR::u8*)GetRowColumnData( iX, iY );

      switch ( m_ImageFormat )
      {
        case GR::Graphic::IF_A8:
        case GR::Graphic::IF_PALETTED:
          return ( m_Palette.Red( *pData ) << 16 )
               | ( m_Palette.Green( *pData ) << 8 )
               | ( m_Palette.Blue( *pData ) ) | 0xff000000;
        case GR::Graphic::IF_A1R5G5B5:
          {
            GR::u16    wColor = *(GR::u16*)pData;

            return ( ( ( ( wColor & 0x8000 ) >> 15 ) * 255 ) << 24 )
                 | ( ( ( ( wColor & 0x7c00 ) >> 10 ) * 255 / 31 ) << 16 )
                 | ( ( ( ( wColor & 0x03e0 ) >> 5 ) * 255 / 31 ) << 8 )
                 | ( ( ( ( wColor & 0x001f ) ) * 255 / 31 ) );
          }
        case GR::Graphic::IF_X1R5G5B5:
          {
            GR::u16    wColor = *(GR::u16*)pData;

            return ( 0xff000000 )
                 | ( ( ( ( wColor & 0x7c00 ) >> 10 ) * 255 / 31 ) << 16 )
                 | ( ( ( ( wColor & 0x03e0 ) >> 5 ) * 255 / 31 ) << 8 )
                 | ( ( ( ( wColor & 0x001f ) ) * 255 / 31 ) );
          }
        case GR::Graphic::IF_R5G6B5:
          {
            GR::u16    wColor = *(GR::u16*)pData;

            return ( 0xff000000 )
                 | ( ( ( ( wColor & 0xf800 ) >> 11 ) * 255 / 31 ) << 16 )
                 | ( ( ( ( wColor & 0x07e0 ) >> 5 ) * 255 / 63 ) << 8 )
                 | ( ( ( ( wColor & 0x001f ) ) * 255 / 31 ) );
          }
        case GR::Graphic::IF_A4R4G4B4:
          {
            GR::u16    wColor = *(GR::u16*)pData;

            return ( ( ( ( wColor & 0xf000 ) >> 12 ) * 255 / 15 ) << 24 )
                 | ( ( ( ( wColor & 0x0f00 ) >> 8 ) * 255 / 15 ) << 16 )
                 | ( ( ( ( wColor & 0x00f0 ) >> 4 ) * 255 / 15 ) << 8 )
                 | ( ( ( ( wColor & 0x000f ) ) * 255 / 15 ) );
          }
          return *(GR::u16*)pData;
        case GR::Graphic::IF_A8R8G8B8:
          return *(GR::u32*)pData;
        case GR::Graphic::IF_X8R8G8B8:
          return *(GR::u32*)pData | 0xff000000;
        case GR::Graphic::IF_R8G8B8:
          {
            GR::u32   dwResult = pData[0] + ( pData[1] << 8 ) + ( pData[2] << 16 );
            return dwResult | 0xff000000;
          }
        default:
          dh::Log( "ImageData::GetPixel unsupported format %d\n", m_ImageFormat );
          return 0;
      }
      return 0;

    }



    void ImageData::SetOwnership( bool bOwner )
    {

      m_Owner = bOwner;

    }



    eImageFormat ImageData::ImageFormat() const
    {

      return m_ImageFormat;

    }



    void ImageData::TransparentColorUsed( bool bUsed )
    {

      m_TransparentColorUsed = bUsed;

    }



    bool ImageData::TransparentColorUsed() const
    {


      return m_TransparentColorUsed;

    }



    void ImageData::TransparentColor( GR::u32 Color )
    {
      m_TransparentColor = Color;
    }



    unsigned long ImageData::TransparentColor() const
    {
      return m_TransparentColor;
    }



    void ImageData::SetPalette( const GR::Graphic::Palette& Palette )
    {
      m_Palette = Palette;
    }



    GR::Graphic::Palette& ImageData::Palette()
    {
      return m_Palette;
    }



    const GR::Graphic::Palette& ImageData::Palette() const
    {
      return m_Palette;
    }



    bool ImageData::HasPaletteData() const
    {
      return ( m_Palette.Entries() != 0 );
    }



    eImageFormat ImageData::ImageFormatFromDepth( unsigned long dwDepth )
    {

      switch ( dwDepth )
      {
        case 1:
          return GR::Graphic::IF_INDEX1;
        case 2:
          return GR::Graphic::IF_INDEX2;
        case 4:
          return GR::Graphic::IF_INDEX4;
        case 8:
          return GR::Graphic::IF_INDEX8;
        case 15:
          return GR::Graphic::IF_X1R5G5B5;
        case 16:
          return GR::Graphic::IF_R5G6B5;
        case 24:
          return GR::Graphic::IF_R8G8B8;
        case 32:
          return GR::Graphic::IF_X8R8G8B8;
      }
      return GR::Graphic::IF_UNKNOWN;

    }



    unsigned long ImageData::DepthFromImageFormat( GR::Graphic::eImageFormat eImgFormat )
    {

      switch ( eImgFormat )
      {
        case GR::Graphic::IF_INDEX1:
          return 1;
        case GR::Graphic::IF_INDEX2:
          return 2;
        case GR::Graphic::IF_INDEX4:
          return 4;
        case GR::Graphic::IF_INDEX8:
        case GR::Graphic::IF_A8:
          return 8;
        case GR::Graphic::IF_A1R5G5B5:
        case GR::Graphic::IF_A4R4G4B4:
        case GR::Graphic::IF_R5G6B5:
        case GR::Graphic::IF_X1R5G5B5:
          return 16;
        case GR::Graphic::IF_R8G8B8:
          return 24;
        case GR::Graphic::IF_A8R8G8B8:
        case GR::Graphic::IF_X8R8G8B8:
        case GR::Graphic::IF_A2B10G10R10:
        case GR::Graphic::IF_A2R10G10B10:
          return 32;
        case GR::Graphic::IF_UNKNOWN:
        case GR::Graphic::IF_COMPLEX:
          return 0;
      }
      return 0;

    }



    bool ImageData::FormatHasAlpha( GR::Graphic::eImageFormat imgFormat )
    {
      return ( ( imgFormat == GR::Graphic::IF_A1R5G5B5 )
      ||       ( imgFormat == GR::Graphic::IF_A4R4G4B4 )
      ||       ( imgFormat == GR::Graphic::IF_A8 )
      ||       ( imgFormat == GR::Graphic::IF_A8R8G8B8 ) );
    }


  }
}