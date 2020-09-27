#include <Grafik\ContextDescriptor.h>
#include <Grafik/Image.h>
#include <Grafik/Palette.h>
#include <Grafik/GFXHelper.h>
#include <Grafik/RLEList.h>


#include <Misc/Misc.h>

#include <debug\debugclient.h>

#include <math.h>

#include <algorithm>



namespace GR
{
  namespace Graphic
  {


    ContextDescriptor::ContextDescriptor() 
      : ImageData(),
        PutPixelProc( NULL ),
        HLineProc( NULL ),
        GetPixelProc( NULL ),
        CopyLineProc( NULL ),
        CopyLineTransparentProc( NULL ),
        CopyAreaAlphaProc( NULL ),
        CopyAreaAlphaTransparentProc( NULL ),
        CopyAreaAlphaAndMaskProc( NULL ),
        CopyPixelProc( NULL ),
        CopyPixelAlphaProc( NULL ),
        CopyPixelMaskProc( NULL ),
        CopyAreaAlphaAndMaskScaledProc( NULL ),
        AlphaBoxProc( NULL ),
        m_ClipLeft( 0 ),
        m_ClipTop( 0 ),
        m_ClipRight( 0 ),
        m_ClipBottom( 0 ),
        m_BitDepth( 0 ),
        m_NothingVisible( true )
    {
    }


    ContextDescriptor::ContextDescriptor( const GR::Graphic::ImageData& ImageData ) 
      : GR::Graphic::ImageData(),
        PutPixelProc( NULL ),
        HLineProc( NULL ),
        GetPixelProc( NULL ),
        CopyLineProc( NULL ),
        CopyLineTransparentProc( NULL ),
        CopyAreaAlphaProc( NULL ),
        CopyAreaAlphaTransparentProc( NULL ),
        CopyAreaAlphaAndMaskProc( NULL ),
        CopyPixelProc( NULL ),
        CopyPixelAlphaProc( NULL ),
        CopyPixelMaskProc( NULL ),
        CopyAreaAlphaAndMaskScaledProc( NULL ),
        AlphaBoxProc( NULL ),
        m_ClipLeft( 0 ),
        m_ClipTop( 0 ),
        m_ClipRight( 0 ),
        m_ClipBottom( 0 ),
        m_BitDepth( 0 ),
        m_NothingVisible( true )
    {
      CreateData( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat(), ImageData.LineOffsetInBytes() );

      memcpy( m_pData, ImageData.Data(), ImageData.DataSize() );

      m_Palette = ImageData.Palette();
    }



    ContextDescriptor::ContextDescriptor( const ContextDescriptor& rhsCD ) 
      : ImageData(),
        PutPixelProc( NULL ),
        HLineProc( NULL ),
        GetPixelProc( NULL ),
        CopyLineProc( NULL ),
        CopyLineTransparentProc( NULL ),
        CopyAreaAlphaProc( NULL ),
        CopyAreaAlphaTransparentProc( NULL ),
        CopyAreaAlphaAndMaskProc( NULL ),
        CopyPixelProc( NULL ),
        CopyPixelAlphaProc( NULL ),
        CopyPixelMaskProc( NULL ),
        CopyAreaAlphaAndMaskScaledProc( NULL ),
        AlphaBoxProc( NULL ),
        m_ClipLeft( 0 ),
        m_ClipTop( 0 ),
        m_ClipRight( 0 ),
        m_ClipBottom( 0 ),
        m_BitDepth( 0 ),
        m_NothingVisible( true )
    {
      CreateData( rhsCD.Width(), rhsCD.Height(), rhsCD.ImageFormat(), rhsCD.LineOffsetInBytes() );

      for ( int i = 0; i < Height(); ++i )
      {
        memcpy( (GR::u8*)m_pData + i * LineOffsetInBytes(), (GR::u8*)rhsCD.m_pData + i * rhsCD.LineOffsetInBytes(), BytesPerLine() );
      }
      m_Palette = rhsCD.Palette();
    }



    ContextDescriptor::ContextDescriptor( GR::Graphic::GFXPage* pPage, GR::Graphic::Palette* pPal )
      : ImageData(),
        PutPixelProc( NULL ),
        HLineProc( NULL ),
        GetPixelProc( NULL ),
        CopyLineProc( NULL ),
        CopyLineTransparentProc( NULL ),
        CopyAreaAlphaProc( NULL ),
        CopyAreaAlphaTransparentProc( NULL ),
        CopyAreaAlphaAndMaskProc( NULL ),
        CopyPixelProc( NULL ),
        CopyPixelAlphaProc( NULL ),
        CopyPixelMaskProc( NULL ),
        CopyAreaAlphaAndMaskScaledProc( NULL ),
        AlphaBoxProc( NULL ),
        m_ClipLeft( 0 ),
        m_ClipTop( 0 ),
        m_ClipRight( 0 ),
        m_ClipBottom( 0 ),
        m_BitDepth( 0 ),
        m_NothingVisible( false )
    {
      Create( pPage, pPal );
    }



    ContextDescriptor::ContextDescriptor( GR::Graphic::Image& Image, int X, int Y, int Width, int Height, GR::Graphic::Palette* pPal )
      : ImageData(),
        PutPixelProc( NULL ),
        HLineProc( NULL ),
        GetPixelProc( NULL ),
        CopyLineProc( NULL ),
        CopyLineTransparentProc( NULL ),
        CopyAreaAlphaProc( NULL ),
        CopyAreaAlphaTransparentProc( NULL ),
        CopyAreaAlphaAndMaskProc( NULL ),
        CopyPixelProc( NULL ),
        CopyPixelAlphaProc( NULL ),
        CopyPixelMaskProc( NULL ),
        CopyAreaAlphaAndMaskScaledProc( NULL ),
        AlphaBoxProc( NULL ),
        m_ClipLeft( 0 ),
        m_ClipTop( 0 ),
        m_ClipRight( 0 ),
        m_ClipBottom( 0 ),
        m_BitDepth( 0 ),
        m_NothingVisible( false )
    {
      if ( Width == 0 )
      {
        Width = Image.GetWidth();
      }
      if ( Height == 0 )
      {
        Height = Image.GetHeight();
      }

      m_Width      = Width;
      m_Height     = Height;
      m_BitDepth  = Image.GetDepth();

      m_ImageFormat = ImageFormatFromDepth( m_BitDepth );

      AssignColorDepth();

      m_LineOffsetInBytes = Image.GetLineSize();

      m_pData = ( (GR::u8*)Image.GetData() ) + X * BytesProPixel() + Y * Image.GetLineSize();

      m_ClipLeft     = 0;
      m_ClipTop      = 0;
      m_ClipRight    = m_Width - 1;
      m_ClipBottom   = m_Height - 1;

      if ( m_BitDepth <= 8 )
      {
        if ( pPal )
        {
          m_Palette = *pPal;
        }
      }

      if ( Image.GetCompressList() )
      {
        // eine transp. Farbe
        TransparentColor( Image.GetTransparentColor32() );
        TransparentColorUsed();
      }
    }



    ContextDescriptor::ContextDescriptor( GR::Graphic::Image* pImage, GR::Graphic::Palette* pPal )
      : ImageData(),
        PutPixelProc( NULL ),
        HLineProc( NULL ),
        GetPixelProc( NULL ),
        CopyLineProc( NULL ),
        CopyLineTransparentProc( NULL ),
        CopyAreaAlphaProc( NULL ),
        CopyAreaAlphaTransparentProc( NULL ),
        CopyAreaAlphaAndMaskProc( NULL ),
        CopyPixelProc( NULL ),
        CopyPixelAlphaProc( NULL ),
        CopyPixelMaskProc( NULL ),
        CopyAreaAlphaAndMaskScaledProc( NULL ),
        AlphaBoxProc( NULL ),
        m_ClipLeft( 0 ),
        m_ClipTop( 0 ),
        m_ClipRight( 0 ),
        m_ClipBottom( 0 ),
        m_BitDepth( 0 ),
        m_NothingVisible( false )
    {
      Create( pImage, pPal );
    }



    ContextDescriptor& ContextDescriptor::operator=( const ContextDescriptor& rhsCD )
    {
      ImageData::operator=( rhsCD );

      AssignColorDepth();

      return *this;
    }



    ContextDescriptor::~ContextDescriptor()
    {
      ImageData::~ImageData();
    }



    bool ContextDescriptor::Create( GR::Graphic::Image* pImage, GR::Graphic::Palette* pPalette )
    {
      if ( pImage == NULL )
      {
        return false;
      }

      m_NothingVisible       = false;

      m_pData                 = pImage->GetData();
      m_Width                 = pImage->GetWidth();
      m_Height                = pImage->GetHeight();
      m_BitDepth              = pImage->GetDepth();
      m_Palette.Release();

      m_ClipLeft     = 0;
      m_ClipTop      = 0;
      m_ClipRight    = m_Width - 1;
      m_ClipBottom   = m_Height - 1;

      if ( ( m_BitDepth <= 8 )
      &&   ( pPalette ) )
      {
        m_Palette = *pPalette;
      }

      m_ImageFormat = ImageFormatFromDepth( m_BitDepth );
      AssignColorDepth();

      if ( pImage->GetCompressList() )
      {
        // eine transp. Farbe
        TransparentColor( pImage->GetTransparentColor32() );
        TransparentColorUsed();
      }
      return true;
    }



    bool ContextDescriptor::Create( GR::Graphic::GFXPage* pPage, GR::Graphic::Palette* pPalette )
    {
      if ( pPage == NULL )
      {
        return false;
      }

      m_NothingVisible          = false;
      m_pData                   = pPage->GetData();
      m_Width                   = pPage->GetWidth();
      m_Height                  = pPage->GetHeight();
      m_BitDepth                = pPage->GetDepth();
      m_Palette.Release();

      if ( m_BitDepth <= 8 )
      {
        if ( pPalette )
        {
          m_Palette = *pPalette;
        }
      }
      m_ImageFormat = ImageFormatFromDepth( m_BitDepth );

      AssignColorDepth();

      m_LineOffsetInBytes    = pPage->GetLineOffset();
      m_ClipLeft             = pPage->GetLeftBorder();
      m_ClipTop              = pPage->GetTopBorder();
      m_ClipRight            = pPage->GetRightBorder();
      m_ClipBottom           = pPage->GetBottomBorder();

      // Offset berücksichtigen!
      if ( ( pPage->GetOffsetX() )
      ||   ( pPage->GetOffsetY() ) )
      {
        char*   pTemp = (char*)m_pData;

        if ( (int)pPage->GetOffsetY() + m_Height < (int)pPage->GetTopBorder() )
        {
          // nichts sichtbar
          m_NothingVisible = true;
        }
        if ( (int)pPage->GetOffsetX() + m_Width < (int)pPage->GetLeftBorder() )
        {
          // nichts sichtbar
          m_NothingVisible = true;
        }

        if ( !m_NothingVisible )
        {
          pTemp += pPage->GetOffsetY() * m_LineOffsetInBytes;

          m_ClipLeft -= pPage->GetOffsetX();
          m_ClipTop -= pPage->GetOffsetY();
          m_ClipRight -= pPage->GetOffsetX();
          m_ClipBottom -= pPage->GetOffsetY();

          if ( m_ClipLeft < 0 )
          {
            m_ClipLeft = 0;
          }
          if ( m_ClipTop < 0 )
          {
            m_ClipTop = 0;
          }
      
          if ( pPage->GetOffsetY() + m_Height >= pPage->GetHeight() )
          {
            m_Height = pPage->GetHeight() - pPage->GetOffsetY();
            if ( m_ClipBottom >= m_Height )
            {
              m_ClipBottom = m_ClipTop + m_Height - 1;
            }
          }

          pTemp += pPage->GetOffsetX() * BytesProPixel();
          if ( pPage->GetOffsetX() + m_Width >= pPage->GetWidth() )
          {
            m_Width = pPage->GetWidth() - pPage->GetOffsetX();
            if ( m_ClipRight >= m_Width )
            {
              m_ClipRight = m_ClipLeft + m_Width - 1;
            }
          }
        }

        m_pData = pTemp;
      }
      return true;
    }



    void ContextDescriptor::AssignColorDepth()
    {
      AlphaBoxProc                    = NULL;
      PutPixelProc                    = NULL;
      HLineProc                       = NULL;
      GetPixelProc                    = NULL;
      CopyLineTransparentProc         = NULL;
      CopyPixelProc                   = NULL;
      CopyPixelMaskProc               = NULL;
      CopyAreaAlphaProc               = &ContextDescriptor::_CopyAreaAlphaSlow;
      CopyAreaAlphaTransparentProc    = NULL;
      CopyAreaAlphaAndMaskProc        = NULL;
      CopyPixelAlphaProc              = NULL;
      CopyAreaAlphaAndMaskScaledProc  = NULL;
      CopyLineProc                    = NULL;

      switch ( m_BitDepth )
      {
        case 1:
          // TODO LineSize!!!
          m_BytesPerLine               = m_Width / 8;
          m_LineOffsetInBytes          = m_Width / 8;
          if ( m_Width % 8 )
          {
            m_LineOffsetInBytes++;
            m_BytesPerLine++;
          }
          PutPixelProc                    = &ContextDescriptor::_PutPixel1;
          HLineProc                       = &ContextDescriptor::_HLine1;
          GetPixelProc                    = &ContextDescriptor::_GetPixel1;
          CopyLineProc                    = &ContextDescriptor::_CopyLine1;
          //CopyTransparentProc           = &ContextDescriptor::_CopyTransparent8;
          //CopyPixelProc                 = &ContextDescriptor::_CopyPixel8;
          //CopyPixelMaskProc             = &ContextDescriptor::_CopyPixelMask8;
          break;
        case 2:
          // TODO LineSize!!!
          m_BytesPerLine = m_Width / 4;
          m_LineOffsetInBytes = m_Width / 4;
          if ( m_Width % 4 )
          {
            m_LineOffsetInBytes++;
            m_BytesPerLine++;
          }
          PutPixelProc                    = &ContextDescriptor::_PutPixel2;
          HLineProc                       = &ContextDescriptor::_HLine2;
          GetPixelProc                    = &ContextDescriptor::_GetPixel2;
          CopyLineProc                    = &ContextDescriptor::_CopyLine2;
          //CopyTransparentProc           = &ContextDescriptor::_CopyTransparent8;
          //CopyPixelProc                 = &ContextDescriptor::_CopyPixel8;
          //CopyPixelMaskProc             = &ContextDescriptor::_CopyPixelMask8;
          break;
        case 4:
          // TODO LineSize!!!
          m_BytesPerLine                 = m_Width / 2;
          m_LineOffsetInBytes            = m_Width / 2;
          if ( m_Width & 1 )
          {
            m_LineOffsetInBytes++;
            m_BytesPerLine++;
          }
          PutPixelProc                    = &ContextDescriptor::_PutPixel4;
          HLineProc                       = &ContextDescriptor::_HLine4;
          GetPixelProc                    = &ContextDescriptor::_GetPixel4;
          CopyLineProc                    = &ContextDescriptor::_CopyLine4;
          CopyLineTransparentProc         = &ContextDescriptor::_CopyLineTransparent4;
          CopyAreaAlphaAndMaskProc        = &ContextDescriptor::_CopyAreaAlphaAndMask4;
          //CopyPixelProc                 = &ContextDescriptor::_CopyPixel8;
          //CopyPixelMaskProc             = &ContextDescriptor::_CopyPixelMask8;
          break;
        case 8:
          m_BytesPerLine               = m_Width;
          m_LineOffsetInBytes          = m_Width;

          PutPixelProc                  = &ContextDescriptor::_PutPixel8;
          HLineProc                     = &ContextDescriptor::_HLine8;
          GetPixelProc                  = &ContextDescriptor::_GetPixel8;
          CopyLineTransparentProc       = &ContextDescriptor::_CopyLineTransparent8;
          CopyPixelProc                 = &ContextDescriptor::_CopyPixel8;
          CopyPixelMaskProc             = &ContextDescriptor::_CopyPixelMask8;
          CopyLineProc                  = &ContextDescriptor::_CopyLine8;

          CopyAreaAlphaAndMaskProc        = &ContextDescriptor::_CopyAreaAlphaAndMask8;
          break;
        case 15:
          m_BytesPerLine                 = m_Width * 2;
          m_LineOffsetInBytes            = m_Width * 2;

          PutPixelProc                    = &ContextDescriptor::_PutPixel555;
          HLineProc                       = &ContextDescriptor::_HLine16;
          GetPixelProc                    = &ContextDescriptor::_GetPixel555;
          CopyLineTransparentProc         = &ContextDescriptor::_CopyLineTransparent16;
          CopyAreaAlphaProc               = &ContextDescriptor::_CopyAreaAlpha555;
          CopyAreaAlphaTransparentProc    = &ContextDescriptor::_CopyAreaAlphaTransparent555;
          CopyAreaAlphaAndMaskProc        = &ContextDescriptor::_CopyAreaAlphaAndMask555;
          CopyPixelProc                   = &ContextDescriptor::_CopyPixel16;
          CopyPixelAlphaProc              = &ContextDescriptor::_CopyPixelAlpha555;
          CopyPixelMaskProc               = &ContextDescriptor::_CopyPixelMask555;
          CopyAreaAlphaAndMaskScaledProc  = &ContextDescriptor::_CopyAreaAlphaAndMaskScaled555;
          AlphaBoxProc                    = &ContextDescriptor::_AlphaBox16;
          CopyLineProc                    = &ContextDescriptor::_CopyLine16;
          break;
        case 16:
          m_BytesPerLine                 = m_Width * 2;
          m_LineOffsetInBytes            = m_Width * 2;

          PutPixelProc                    = &ContextDescriptor::_PutPixel565;
          HLineProc                       = &ContextDescriptor::_HLine16;
          GetPixelProc                    = &ContextDescriptor::_GetPixel565;
          CopyLineTransparentProc         = &ContextDescriptor::_CopyLineTransparent16;
          CopyAreaAlphaProc               = &ContextDescriptor::_CopyAreaAlpha565;
          CopyAreaAlphaTransparentProc    = &ContextDescriptor::_CopyAreaAlphaTransparent565;
          CopyPixelProc                   = &ContextDescriptor::_CopyPixel16;
          CopyAreaAlphaAndMaskProc        = &ContextDescriptor::_CopyAreaAlphaAndMask565;
          CopyPixelAlphaProc              = &ContextDescriptor::_CopyPixelAlpha565;
          CopyPixelMaskProc               = &ContextDescriptor::_CopyPixelMask565;
          //CopyAreaAlphaAndMaskScaledProc  = &ContextDescriptor::_CopyAreaAlphaAndMaskScaled565;
          AlphaBoxProc                    = &ContextDescriptor::_AlphaBox16;
          CopyLineProc                    = &ContextDescriptor::_CopyLine16;
          break;
        case 24:
          m_BytesPerLine                 = m_Width * 3;
          m_LineOffsetInBytes            = m_Width * 3;

          PutPixelProc                    = &ContextDescriptor::_PutPixel24;
          HLineProc                       = &ContextDescriptor::_HLine24;
          GetPixelProc                    = &ContextDescriptor::_GetPixel24;
          CopyLineTransparentProc         = &ContextDescriptor::_CopyLineTransparent24;
          CopyAreaAlphaProc               = &ContextDescriptor::_CopyAreaAlpha24;
          CopyAreaAlphaTransparentProc    = &ContextDescriptor::_CopyAreaAlphaTransparent24;
          CopyAreaAlphaAndMaskProc        = &ContextDescriptor::_CopyAreaAlphaAndMask24;
          CopyPixelProc                   = &ContextDescriptor::_CopyPixel24;
          CopyPixelAlphaProc              = &ContextDescriptor::_CopyPixelAlpha24;
          CopyPixelMaskProc               = &ContextDescriptor::_CopyPixelMask24;
          CopyAreaAlphaAndMaskScaledProc  = &ContextDescriptor::_CopyAreaAlphaAndMaskScaled24;
          CopyLineProc                    = &ContextDescriptor::_CopyLine24;
          break;
        case 32:
          m_BytesPerLine                 = m_Width * 4;
          m_LineOffsetInBytes            = m_Width * 4;

          PutPixelProc                    = &ContextDescriptor::_PutPixel32;
          HLineProc                       = &ContextDescriptor::_HLine32;
          GetPixelProc                    = &ContextDescriptor::_GetPixel32;
          CopyLineTransparentProc         = &ContextDescriptor::_CopyLineTransparent32;
          CopyAreaAlphaProc               = &ContextDescriptor::_CopyAreaAlpha32;
          CopyAreaAlphaTransparentProc    = &ContextDescriptor::_CopyAreaAlphaTransparent32;
          CopyAreaAlphaAndMaskProc        = &ContextDescriptor::_CopyAreaAlphaAndMask32;
          CopyPixelProc                   = &ContextDescriptor::_CopyPixel32;
          CopyPixelAlphaProc              = &ContextDescriptor::_CopyPixelAlpha32;
          CopyPixelMaskProc               = &ContextDescriptor::_CopyPixelMask32;
          CopyAreaAlphaAndMaskScaledProc  = &ContextDescriptor::_CopyAreaAlphaAndMaskScaled32;
          CopyLineProc                    = &ContextDescriptor::_CopyLine32;
          break;
        default:
          dh::Log( "ContextDescriptor: unknown depth %d", m_BitDepth );
          break;
      }
    }



    size_t ContextDescriptor::MapColorToPalette( GR::u32 Color )
    {
      if ( m_Palette.Entries() == 0 )
      {
        return 0;
      }
      return m_Palette.FindNearestIndex( Color );
    }



    size_t ContextDescriptor::MapColor( GR::u32 Color )
    {
      switch ( m_BitDepth )
      {
        case 1:
        case 2:
        case 4:
        case 8:
          return MapColorToPalette( Color );
        case 32:
          return Color;
        case 15:
          return (GR::u16)( ( ( ( Color & 0xff0000 ) >> 19 ) << 10 )
                       + ( ( ( Color & 0x00ff00 ) >> 11 ) <<  5 )
                       + ( ( ( Color & 0x0000ff ) >>  3 ) ) );
        case 16:
          return (GR::u16)( ( ( ( Color & 0xf80000 ) >> 19 ) << 11 )
                       + ( ( ( Color & 0x00fc00 ) >> 10 ) <<  5 )
                       + ( ( ( Color & 0x0000f8 ) >>  3 ) ) );
        case 24:
          return Color & 0xffffff;
      }
      return Color;
    }



    void ContextDescriptor::_PutPixel1( int X, int Y, GR::u32 Color )
    {
      GR::u8*   pTempData = ((GR::u8*)m_pData ) + ( X / 8 ) + Y * m_LineOffsetInBytes;

      GR::u8    ucMask = 1 << ( 7 - X % 8 );

      if ( Color )
      {
        *pTempData |= ucMask;
      }
      else
      {
        *pTempData &= ~ucMask;
      }
    }



    void ContextDescriptor::_PutPixel2( int X, int Y, GR::u32 Color )
    {
      GR::u8*   pTempData = ( ( GR::u8* )m_pData ) + ( X / 4 ) + Y * m_LineOffsetInBytes;

      GR::u8    ucMask = 3 << ( 2 * ( 3 - X % 4 ) );

      Color &= 0x03;
      Color <<= ( 2 * ( 3 - X % 4 ) );

      *pTempData &= ~ucMask;
      *pTempData |= Color;
    }



    void ContextDescriptor::_PutPixel4( int X, int Y, GR::u32 Color )
    {
      GR::u8*   pData = ((GR::u8*)m_pData ) + ( X >> 1 ) + Y * m_LineOffsetInBytes;
      if ( X & 1 )
      {
        *pData &= 0xf0;
        *pData |= ( Color & 0x0f );
      }
      else
      {
        *pData &= 0x0f;
        *pData |= ( Color << 4 );
      }
    }



    void ContextDescriptor::_PutPixel8( int X, int Y, GR::u32 Color )
    {
      ( (GR::u8*)m_pData )[X + Y * m_LineOffsetInBytes] = (GR::u8)Color;
    }


    void ContextDescriptor::_PutPixel555( int X, int Y, GR::u32 Color )
    {
      ( (GR::u16*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 1 )] = (GR::u16)Color;
    }


    void ContextDescriptor::_PutPixel565( int X, int Y, GR::u32 Color )
    {
      ( (GR::u16*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 1 )] = (GR::u16)Color;
    }



    void ContextDescriptor::_PutPixel24( int X, int Y, GR::u32 Color )
    {
      ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes ]    = (GR::u8)( Color & 0x0ff );
      ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 1] = (GR::u8)( ( Color & 0x0ff00 ) >> 8 );
      ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 2] = (GR::u8)( ( Color & 0x0ff0000 ) >> 16 );
    }



    void ContextDescriptor::_PutPixel32( int X, int Y, GR::u32 Color )
    {
      ( (GR::u32*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 2 )] = Color;
    }



    void ContextDescriptor::PutPixel( int X, int Y, GR::u32 Color )
    {
      if ( m_pData == NULL )
      {
        return;
      }
      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X < m_ClipLeft )
      ||   ( Y < m_ClipTop )
      ||   ( X > m_ClipRight )
      ||   ( Y > m_ClipBottom ) )
      {
        return;
      }
      (this->*PutPixelProc)( X, Y, (GR::u32)MapColor( Color ) );
    }



    void ContextDescriptor::PutPixelFast( int X, int Y, GR::u32 Color )
    {
      (this->*PutPixelProc)( X, Y, (GR::u32)MapColor( Color ) );
    }



    void ContextDescriptor::PutDirectPixel( int X, int Y, GR::u32 Color )
    {
      if ( m_pData == NULL )
      {
        return;
      }
      if ( ( X < 0 )
      ||   ( Y < 0 )
      ||   ( X >= m_Width )
      ||   ( Y >= m_Height ) )
      {
        return;
      }

      switch ( m_BitDepth )
      {
        case 1:
          {
            GR::u8*   pTempData = ( (GR::u8*)m_pData ) + ( X / 8 ) + Y * m_LineOffsetInBytes;

            GR::u8    ucMask = 1 << ( 7 - X % 8 );

            if ( Color )
            {
              *pTempData |= ucMask;
            }
            else
            {
              *pTempData &= ~ucMask;
            }
          }
          break;
        case 2:
          {
            GR::u8*   pTempData = ( ( GR::u8* )m_pData ) + ( X / 4 ) + Y * m_LineOffsetInBytes;

            GR::u8    ucMask = 3 << ( 2 * ( 3 - X % 4 ) );

            *pTempData &= ~ucMask;

            Color <<= ( 2 * ( 3 - X % 4 ) );

            *pTempData |= Color;
          }
          break;
        case 4:
          {
            GR::u8*   pData = ( (GR::u8*)m_pData ) + ( X / 2 ) + Y * m_LineOffsetInBytes;
            if ( X & 1 )
            {
              *pData &= 0xf0;
              *pData |= Color;
            }
            else
            {
              *pData &= 0x0f;
              *pData |= ( Color << 4 );
            }
          }
          break;
        case 8:
          ( (GR::u8*)m_pData )[X + Y * m_LineOffsetInBytes] = (GR::u8)Color;
          break;
        case 15:
        case 16:
          ( (GR::u16*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 1 )] = (GR::u16)Color;
          break;
        case 24:
          ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes ]    = (GR::u8)( Color & 0x0ff );
          ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 1] = (GR::u8)( ( Color & 0x0ff00 ) >> 8 );
          ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 2] = (GR::u8)( ( Color & 0x0ff0000 ) >> 16 );
          break;
        case 32:
          ( (GR::u32*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 2 )] = Color;
          break;
        default:
          dh::Log( "ContextDescriptor::PutDirectPixel unsupported depth %d\n", m_BitDepth );
          break;
      }
    }



    GR::u32 ContextDescriptor::_GetPixel1( int X, int Y ) const
    {
      GR::u8*   pTempData = ( (GR::u8*)m_pData ) + ( X / 8 ) + Y * m_LineOffsetInBytes;

      GR::u8    ucMask = 1 << ( 7 - X % 8 );

      if ( *pTempData & ucMask )
      {
        return ( m_Palette.Red( 1 ) << 16 )
           +   ( m_Palette.Green( 1 ) << 8 )
           +   ( m_Palette.Blue( 1 ) );
      }
      return ( m_Palette.Red( 0 ) << 16 )
           + ( m_Palette.Green( 0 ) << 8 )
           + ( m_Palette.Blue( 0 ) );
    }



    GR::u32 ContextDescriptor::_GetPixel2( int X, int Y ) const
    {
      GR::u8* pTempData = ( ( GR::u8* )m_pData ) + ( X / 4 ) + Y * m_LineOffsetInBytes;

      GR::u8    mask = 3 << ( 2 * ( 3 - X % 4 ) );

      GR::u8    color = ( *pTempData & mask );

      color >>= ( 2 * ( 3 - X % 4 ) );

      return ( m_Palette.Red( color ) << 16 )
           + ( m_Palette.Green( color ) << 8 )
           + ( m_Palette.Blue( color ) );
    }



    GR::u32 ContextDescriptor::_GetPixel4( int X, int Y ) const
    {
      GR::u8    Value = ( (GR::u8*)m_pData )[X / 2 + Y * m_LineOffsetInBytes];

      if ( X & 1 )
      {
        Value &= 0xf;
      }
      else
      {
        Value >>= 4;
      }

      return ( m_Palette.Red( Value ) << 16 )
           + ( m_Palette.Green( Value ) << 8 )
           + ( m_Palette.Blue( Value ) );
    }



    GR::u32 ContextDescriptor::_GetPixel8( int X, int Y ) const
    {
      GR::u32 Color = ( (GR::u8*)m_pData )[X + Y * m_LineOffsetInBytes];

      return ( m_Palette.Red( (GR::u8)Color ) << 16 )
           + ( m_Palette.Green( (GR::u8)Color ) << 8 )
           + ( m_Palette.Blue( (GR::u8)Color ) );
    }




    GR::u32 ContextDescriptor::_GetPixel555( int X, int Y ) const
    {
      return GFX::ColorConvert555to32( ( (GR::u16*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 1 )] );
    }



    GR::u32 ContextDescriptor::_GetPixel565( int X, int Y ) const
    {
      return GFX::ColorConvert565to32( ( (GR::u16*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 1 )] );
    }



    GR::u32 ContextDescriptor::_GetPixel24( int X, int Y ) const
    {
      return   ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes ]
           | ( ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 1] << 8 )
           | ( ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 2] << 16 );
    }



    GR::u32 ContextDescriptor::_GetPixel32( int X, int Y ) const
    {
      return ( (GR::u32*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 2 )];
    }



    GR::u32 ContextDescriptor::GetPixel( int X, int Y ) const
    {
      if ( m_pData == NULL )
      {
        return 0;
      }
      if ( m_NothingVisible )
      {
        return 0;
      }
      if ( ( X < m_ClipLeft )
      ||   ( Y < m_ClipTop )
      ||   ( X > m_ClipRight )
      ||   ( Y > m_ClipBottom ) )
      {
        return 0;
      }
      return (this->*GetPixelProc)( X, Y );
    }



    GR::u32 ContextDescriptor::GetDirectPixel( int X, int Y )
    {
      GR::u32     Color;

      if ( m_pData == NULL )
      {
        return 0;
      }
      if ( ( X < 0 )
      ||   ( Y < 0 )
      ||   ( X >= m_Width )
      ||   ( Y >= m_Height ) )
      {
        return 0;
      }
      Color = 0;
      switch ( m_BitDepth )
      {
        case 1:
          {
            GR::u8*   pTempData = ( (GR::u8*)m_pData ) + ( X / 8 ) + Y * m_LineOffsetInBytes;

            GR::u8    ucMask = 1 << ( 7 - X % 8 );

            if ( *pTempData & ucMask )
            {
              return 1;
            }
          }
          return 0;
        case 4:
          {
            GR::u8*   pTempData = ( (GR::u8*)m_pData ) + ( X >> 1 ) + Y * m_LineOffsetInBytes;
            if ( X & 1 )
            {
              return ( ( *pTempData ) & 0x0f );
            }
            else
            {
              return ( ( ( *pTempData ) & 0xf0 ) >> 4 );
            }
          }
          break;
        case 8:
          return (GR::u8)( (GR::u8*)m_pData )[X + Y * m_LineOffsetInBytes];
        case 15:
        case 16:
          return ( (GR::u16*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 1 )];
        case 24:
          Color =   ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes] + 
                  ( ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 1] << 8 ) +
                  ( ( (GR::u8*)m_pData )[3 * X + Y * m_LineOffsetInBytes + 2] << 16 );
          return Color;
        case 32:
          return ( (GR::u32*)m_pData )[X + Y * ( m_LineOffsetInBytes >> 2 )];
        default:
          dh::Log( "ContextDescriptor::GetDirectPixel unsupported depth %d\n", m_BitDepth );
          break;
      }
      return 0;
    }



    void ContextDescriptor::_CopyLineTransparent4( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u8*   pSrc = (GR::u8*)GetRowColumnData( X, Y );
      GR::u8*   pTgt = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );

      GR::u8    src = 0;

      while ( Width )
      {
        if ( X & 1 )
        {
          src = ( *pSrc & 0x0f );
          pSrc++;
        }
        else
        {
          src = ( *pSrc ) >> 4;
        }
        if ( src != Transparent )
        {
          if ( ZX & 1 )
          {
            *pTgt &= 0xf0;
            *pTgt |= src;
          }
          else
          {
            *pTgt &= 0x0f;
            *pTgt |= ( src << 4 );
          }
        }
        if ( ZX & 1 )
        {
          pTgt++;
        }
        Width--;
        X++;
        ZX++;
      }
    }



    void ContextDescriptor::_CopyLineTransparent8( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u8*   pSrc = (GR::u8*)GetRowColumnData( X, Y );
      GR::u8*   pTgt = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );

      for ( int i = 0; i < Width; i++ )
      {
        if ( *pSrc != Transparent )
        {
          *pTgt = *pSrc;
        }
        pSrc++;
        pTgt++;
      }
    }



    void ContextDescriptor::_CopyLineTransparent16( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u16*    pSrc = (GR::u16*)GetRowColumnData( X, Y );
      GR::u16*    pTgt = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY );

      for ( int i = 0; i < Width; i++ )
      {
        if ( *pSrc != Transparent )
        {
          *pTgt = *pSrc;
        }
        ++pSrc;
        ++pTgt;
      }
    }



    void ContextDescriptor::_CopyLineTransparent24( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u8*   pSrc = (GR::u8*)GetRowColumnData( X, Y );
      GR::u8*   pTgt = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );

      for ( int i = 0; i < Width; i++ )
      {
        if ( ( ( *(GR::u32*)pSrc ) & 0x00ffffff ) != Transparent )
        {
          *pTgt++ = *pSrc++;
          *pTgt++ = *pSrc++;
          *pTgt++ = *pSrc++;
        }
        else
        {
          pSrc += 3;
          pTgt += 3;
        }
      }
    }



    void ContextDescriptor::_CopyLineTransparent32( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u32*  pSrc = (GR::u32*)GetRowColumnData( X, Y );
      GR::u32*  pTgt = (GR::u32*)pCDTarget->GetRowColumnData( ZX, ZY );

      for ( int i = 0; i < Width; i++ )
      {
        if ( *pSrc != Transparent )
        {
          *pTgt = *pSrc;
        }
        pSrc++;
        pTgt++;
      }
    }



    void ContextDescriptor::CopyLineTransparent( int X, int Y, int Width, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      (this->*CopyLineTransparentProc)( X, Y, Width, ZX, ZY, Transparent, pCDTarget );
    }



    void ContextDescriptor::_HLine1( int X1, int X2, int Y, GR::u32 Color )
    {
      int   startByte  = X1 >> 3,
            endByte    = X2 >> 3,
            startFract = X1 - ( startByte << 3 ),
            endFract   = X2 - ( endByte << 3 );

      // alles innerhalb eines Bytes
      if ( startByte == endByte )
      {
        for ( int i = X1; i <= X2; ++i )
        {
          PutPixel( i, Y, Color );
        }
        return;
      }
      if ( startFract )
      {
        // Anfangs und End-GR::u8 nicht gleich
        while ( startFract < 8 )
        {
          PutPixel( X1++, Y, Color );
          ++startFract;
        }
      }
      if ( endFract != 7 )
      {
        // Anfangs und End-GR::u8 nicht gleich
        while ( endFract > -1 )
        {
          PutPixel( X2--, Y, Color );
          --endFract;
        }
      }
      // jetzt sollten nur noch volle Bytes da sein
      if ( X1 != X2 )
      {
        CMisc::FillValue( (GR::u8*)m_pData + ( X1 >> 3 ) + Y * m_LineOffsetInBytes, 
                          ( X2 - X1 + 1 ) >> 3,
                          (GR::u8)( 255 * ( Color & 1 ) ) );
      }
    }



    void ContextDescriptor::_HLine2( int X1, int X2, int Y, GR::u32 Color )
    {
      int   startByte = X1 / 4,
            endByte = X2 / 4,
            startFract = X1 - ( startByte * 4 ),
            endFract = X2 - ( endByte * 4 );

      // alles innerhalb eines Bytes
      if ( startByte == endByte )
      {
        for ( int i = X1; i <= X2; ++i )
        {
          PutPixel( i, Y, Color );
        }
        return;
      }
      if ( startFract )
      {
        // Anfangs und End-GR::u8 nicht gleich
        while ( startFract < 4 )
        {
          PutPixel( X1++, Y, Color );
          ++startFract;
        }
      }
      if ( endFract != 3 )
      {
        // Anfangs und End-GR::u8 nicht gleich
        while ( endFract > -1 )
        {
          PutPixel( X2--, Y, Color );
          --endFract;
        }
      }
      // jetzt sollten nur noch volle Bytes da sein
      if ( X1 != X2 )
      {
        Color = Color & 0x03;
        CMisc::FillValue( ( GR::u8* )m_pData + ( X1 / 4 ) + Y * m_LineOffsetInBytes,
                          ( X2 - X1 + 1 ) / 4,
                          ( GR::u8 )( ( Color << 6 ) | ( Color << 4 ) | ( Color << 2 ) | Color ) );
      }
    }



    void ContextDescriptor::_HLine4( int X1, int X2, int Y, GR::u32 Color )
    {
      if ( X1 & 1 )
      {
        PutPixel( X1, Y, Color );
        X1++;
      }
      if ( ( X2 > X1 )
      &&   ( !( X2 & 1 ) ) )
      {
        PutPixel( X2, Y, Color );
        X2--;
      }
      if ( X1 < X2 )
      {
        CMisc::FillValue( (GR::u8 *)m_pData + ( X1 >> 1 ) + Y * m_LineOffsetInBytes, ( X2 - X1 + 1 ) / 2, (GR::u8)( Color + ( Color << 4 ) ) );
      }
    }



    void ContextDescriptor::_HLine8( int X1, int X2, int Y, GR::u32 Color )
    {
      memset( (GR::u8*)m_pData + X1 + Y * m_LineOffsetInBytes, (GR::u32)MapColor( Color ), X2 - X1 + 1 );
    }



    void ContextDescriptor::_HLine16( int X1, int X2, int Y, GR::u32 Color )
    {
      CMisc::FillValue16( (GR::u16*)m_pData + X1 + Y * m_LineOffsetInBytes / 2, ( X2 - X1 + 1 ) * 2, (GR::u16)MapColor( Color ) );
    }



    void ContextDescriptor::_HLine24( int X1, int X2, int Y, GR::u32 Color )
    {
      CMisc::FillValue24( (GR::u32*)( (GR::u8*)m_pData + 3 * X1 + Y * m_LineOffsetInBytes ), ( X2 - X1 + 1 ) * 3, (GR::u32)MapColor( Color ) );
    }



    void ContextDescriptor::_HLine32( int X1, int X2, int Y, GR::u32 Color )
    {
      CMisc::FillValue32( (GR::u32*)m_pData + X1 + Y * ( m_LineOffsetInBytes >> 2 ), ( X2 - X1 + 1 ) * 4, (GR::u32)MapColor( Color ) );
    }



    void ContextDescriptor::HLine( int X1, int X2, int Y, GR::u32 Color )
    {
      if ( X2 < X1 )
      {
        std::swap( X1, X2 );
      }

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( Y < m_ClipTop )
      ||   ( Y > m_ClipBottom )
      ||   ( ( X1 < m_ClipLeft )
      &&     ( X2 < m_ClipLeft ) )
      ||   ( ( X1 > m_ClipRight )
      &&     ( X2 > m_ClipRight ) ) )
      {
        // nichts zu tun!
        return;
      }

      if ( X1 < m_ClipLeft )
      {
        X1 = m_ClipLeft;
      }
      if ( X2 > m_ClipRight )
      {
        X2 = m_ClipRight;
      }
      (this->*HLineProc)( X1, X2, Y, Color );
    }



    void ContextDescriptor::CopyLine( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( Y < m_ClipTop )
      ||   ( Y > m_ClipBottom )
      ||   ( X + Width <= m_ClipLeft )
      ||   ( X > m_ClipRight ) )
      {
        // nichts zu tun!
        return;
      }
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }

      if ( ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZY < pCDTarget->m_ClipTop )
      ||   ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZX + Width < pCDTarget->m_ClipLeft ) )
      {
        // nichts zu tun!
        return;
      }

      if ( X + Width - 1 > m_ClipRight )
      {
        Width = m_ClipRight - X + 1;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      (this->*CopyLineProc)( X, Y, Width, ZX, ZY, pCDTarget );
    }



    void ContextDescriptor::_CopyLine1( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      GR::u8*   pSource = (GR::u8*)GetRowColumnData( X, Y );
      GR::u8*   pTarget = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );

      if ( Width <= 8 )
      {
        while ( Width )
        {
          pCDTarget->PutDirectPixel( ZX, ZY, GetDirectPixel( X, Y ) );
          X++;
          ZX++;
          Width--;
        }
        return;
      }

      // slow but safe
      while ( Width )
      {
        pCDTarget->PutDirectPixel( ZX, ZY, GetDirectPixel( X, Y ) );
        X++;
        ZX++;
        Width--;
      }
      return;
    }



    void ContextDescriptor::_CopyLine2( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      GR::u8* pSource = ( GR::u8* )GetRowColumnData( X, Y );
      GR::u8* pTarget = ( GR::u8* )pCDTarget->GetRowColumnData( ZX, ZY );

      if ( Width <= 8 )
      {
        while ( Width )
        {
          pCDTarget->PutDirectPixel( ZX, ZY, GetDirectPixel( X, Y ) );
          X++;
          ZX++;
          Width--;
        }
        return;
      }

      // slow but safe
      while ( Width )
      {
        pCDTarget->PutDirectPixel( ZX, ZY, GetDirectPixel( X, Y ) );
        X++;
        ZX++;
        Width--;
      }
      return;
    }



    void ContextDescriptor::_CopyLine4( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      if ( ( X & 1 ) == ( ZX & 1 ) )
      {
        if ( X & 1 )
        {
          // vorne ein GR::u8
          GR::u8*   pSource = (GR::u8*)GetRowColumnData( X, Y );
          GR::u8*   pTarget = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );

          *pTarget &= 0xf0;
          *pTarget |= ( *pSource & 0x0f );


          memcpy( (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY ) + 1,
                         (GR::u8*)GetRowColumnData( X, Y ) + 1,
                         ( Width - 1 ) >> 1 );
          if ( ( Width & 1 ) == 0 )
          {
            // hinten ein GR::u8
            pSource += ( ( Width - 1 ) >> 1 );
            pTarget += ( ( Width - 1 ) >> 1 );
            *pTarget &= 0xf0;
            *pTarget |= ( *pSource & 0x0f );
          }
        }
        else
        {
          memcpy( (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY ),
                         (GR::u8*)GetRowColumnData( X, Y ),
                         Width / 2 );

          if ( Width & 1 )
          {
            // noch ein GR::u8
            GR::u8*   pSource = (GR::u8*)GetRowColumnData( X, Y );
            GR::u8*   pTarget = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );
            pSource += ( Width >> 1 );
            pTarget += ( Width >> 1 );
            *pTarget &= 0x0f;
            *pTarget |= ( *pSource & 0xf0 );
          }
        }
        return;
      }

      // hier müssen Bits verschoben werden!
      GR::u8*   pSource = (GR::u8*)GetRowColumnData( X, Y );
      GR::u8*   pTarget = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );

      while ( Width )
      {
        if ( X & 1 )
        {
          *pTarget =  ( *pTarget & 0x0f )
                    | ( ( *pSource & 0x0f ) << 4 );
          pSource++;
        }
        else
        {
          *pTarget =  ( *pTarget & 0xf0 )
                    | ( ( *pSource & 0xf0 ) >> 4 );
          pTarget++;
        }

        Width--;
        X++;
      }
    }



    void ContextDescriptor::_CopyLine8( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      memcpy( (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY ),
              (GR::u8*)GetRowColumnData( X, Y ),
              Width );
    }



    void ContextDescriptor::_CopyLine16( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      memcpy( (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY ),
              (GR::u8*)GetRowColumnData( X, Y ),
              Width * 2 );
    }



    void ContextDescriptor::_CopyLine24( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      memcpy( (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY ),
              (GR::u8*)GetRowColumnData( X, Y ),
              Width * 3 );
    }



    void ContextDescriptor::_CopyLine32( int X, int Y, int Width, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      memcpy( (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY ),
              (GR::u8*)GetRowColumnData( X, Y ),
              Width * 4 );
    }



    void ContextDescriptor::Line( int X1, int Y1, int X2, int Y2, GR::u32 Color )
    {
      int dy = Y2 - Y1;
      int dx = X2 - X1;
      int stepx, stepy;

      if ( dy < 0 ) 
      { 
        dy = -dy;  
        stepy = -1; 
      } 
      else 
      { 
        stepy = 1; 
      }
      if ( dx < 0 ) 
      { 
        dx = -dx;  
        stepx = -1; 
      } 
      else 
      { 
        stepx = 1; 
      }

      dy <<= 1;
      dx <<= 1;

      PutPixel( X1, Y1, Color );
      if ( dx > dy ) 
      {
        int fraction = dy - ( dx >> 1 );

        while ( X1 != X2 ) 
        {
          if ( fraction >= 0 ) 
          {
            Y1 += stepy;
            fraction -= dx;
          }
          X1 += stepx;
          fraction += dy;
          PutPixel( X1, Y1, Color );
        }
      } 
      else 
      {
        int fraction = dx - ( dy >> 1 );

        while ( Y1 != Y2 ) 
        {
          if ( fraction >= 0 ) 
          {
            X1 += stepx;
            fraction -= dy;
          }
          Y1 += stepy;
          fraction += dx;
          PutPixel( X1, Y1, Color );
        }
      }
    }



    void ContextDescriptor::AALine( int X1, int Y1, int X2, int Y2, GR::u32 Color )
    {
      int xd = X2 - X1;
      int yd = Y2 - Y1;


      if ( ( xd == 0 )
      ||   ( yd == 0 ) )
      {
        Line( X1, Y1, X2, Y2, Color );
        return;
      }

      int r = ( Color & 0xff0000 );
      int g = ( Color & 0x00ff00 );
      int b = ( Color & 0xff );

      PutPixel( X1, Y1, Color );
      PutPixel( X2, Y2, Color );

      if ( abs( xd ) > abs( yd ) )
      {
        if ( X1 > X2 )
        {
          int tmp = X1; X1 = X2; X2 = tmp;
          tmp = Y1; Y1 = Y2; Y2 = tmp;
          xd = X2 - X1;
          yd = Y2 - Y1;
        }

        int grad = yd * 65536 / xd;
        int yf = Y1 * 65536;

        for ( int x = X1 + 1; x < X2; ++x )
        {
          yf = yf + grad ;
          int w = ( yf >> 8 ) & 0xff;
          int y = yf >> 16;

          PutPixelAlpha( x, y,      Color, 255 - w );
          PutPixelAlpha( x, y + 1,  Color, w );
        }
      }
      else
      {
        if ( Y1 > Y2 )
        {
          int tmp = X1; X1 = X2; X2 = tmp;
          tmp = Y1; Y1 = Y2; Y2 = tmp;
          xd = X2 - X1;
          yd = Y2 - Y1;
        }

        int grad = xd * 65536 / yd;
        int xf = X1 * 65536;

        for ( int y = Y1 + 1; y < Y2; ++y )
        {
          xf = xf + grad ;
          int w = ( xf >> 8 ) & 0xff;
          int x = xf >> 16;

          PutPixelAlpha( x, y,      Color, 255 - w );
          PutPixelAlpha( x + 1, y,  Color, w );
        }
      }
    }



    void ContextDescriptor::Box( int X, int Y, int Width, int Height, GR::u32 Color )
    {
      for ( int i = 0; i < Height; i++ )
      {
        HLine( X, X + Width - 1, Y + i, Color );
      }
    }



    void ContextDescriptor::Rectangle( int X, int Y, int Width, int Height, GR::u32 Color )
    {
      HLine( X, X + Width - 1, Y, Color );
      HLine( X, X + Width - 1, Y + Height - 1, Color );
      Line( X, Y, X, Y + Height - 1, Color );
      Line( X + Width - 1, Y, X + Width - 1, Y + Height - 1, Color );
    }



    void ContextDescriptor::Ellipse( int x, int y, int a, int b, GR::u32 ucColor )
    {
      int         z,
                  e,
                  f,
                  g,
                  h,
                  aq,
                  bq,
                  dx,
                  dy,
                  r,
                  s,
                  t;

  
      a = abs( a );
      b = abs( b );
      PutPixel( x + a, y, ucColor );
      PutPixel( x - a, y, ucColor );
      e = x - a;
      g = y;
      f = x + a;
      h = y;
      aq = a * a;
      bq = b * b;
      dx = ( aq << 1 );
      dy = ( bq << 1 );
      r = a * bq;
      s = ( r << 1 );
      t = 0;
      z = a;
      while ( z > 0 )
      {
        if ( r > 0 )
        {
          g++;
          h--;
          t += dx;
          r -= t;
        }
        if ( r <= 0 )
        {
          z--;
          e++;
          f--;
          s -= dy;
          r += s;
        }
        PutPixel( e, g, ucColor );
        PutPixel( e, h, ucColor );
        PutPixel( f, g, ucColor );
        PutPixel( f, h, ucColor );
      }
    }



    void ContextDescriptor::FilledEllipse( int x, int y, int a, int b, GR::u32 ucColor )
    {
      int         z,
                  e,
                  f,
                  g,
                  h,
                  aq,
                  bq,
                  dx,
                  dy,
                  r,
                  s,
                  t;

      a = abs( a );
      b = abs( b );

      HLine( x - a, x + a, y, ucColor );
      e = x - a;
      g = y;
      f = x + a;
      h = y;
      aq = a * a;
      bq = b * b;
      dx = ( aq << 1 );
      dy = ( bq << 1 );
      r = a * bq;
      s = ( r << 1 );
      t = 0;
      z = a;
      while ( z > 0 )
      {
        if ( r > 0 )
        {
          g++;
          h--;
          t += dx;
          r -= t;
        }
        if ( r <= 0 )
        {
          z--;
          e++;
          f--;
          s -= dy;
          r += s;
        }
        HLine( e, f, g, ucColor );
        HLine( e, f, h, ucColor );
      }
    }



    void ContextDescriptor::Circle( int x, int y, int Radius, GR::u32 Color )
    {
      Ellipse( x, y, Radius, Radius, Color );
    }



    void ContextDescriptor::FilledCircle( int x, int y, int Radius, GR::u32 Color )
    {
      FilledEllipse( x, y, Radius, Radius, Color );
    }



    void ContextDescriptor::CopyArea( int X1, int Y1, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          dh::Log( "CopyArea: ConvertSelfTo failed" );
          dh::Log( "Format not matching  (%d != %d)", m_ImageFormat, pCDTarget->ImageFormat() );
          return;
        }
        //dh::Log( "CopyArea - %x (%d,%d) Converted To %d!", this, Width(), Height(), m_BitDepth );
      }
      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      for ( int y = 0; y < Height; y++ )
      {
        (this->*CopyLineProc)( X1, Y1 + y, Width, ZX, ZY + y, pCDTarget );
      }
    }



    void ContextDescriptor::CopyAreaSimple( int X1, int Y1, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( ( m_ImageFormat != pCDTarget->ImageFormat() )
      &&   ( BytesProPixel() != pCDTarget->BytesProPixel() ) )
      {
        dh::Log( "ContextDescriptor::CopyArea  Format not matching  (%d != %d)",
                m_ImageFormat, pCDTarget->ImageFormat() );
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          dh::Log( "CopyArea: ConvertSelfTo failed" );
          return;
        }
        dh::Log( "CopyArea - %x (%d,%d) Converted To %d!", this, this->Width(), this->Height(), m_BitDepth );
      }
      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      if ( ( FormatHasAlpha( m_ImageFormat ) )
      &&   ( m_ImageFormat != pCDTarget->m_ImageFormat ) )
      {
        // da soll alpha geblittet werden!
        // was jetzt noch übrig ist kopieren!
        for ( int y = 0; y < Height; y++ )
        {
          for ( int x = 0; x < Width; ++x )
          {
            GR::u32   Pixel = GetPixel( x, y );
            pCDTarget->PutPixelAlpha( x, y, Pixel, ( Pixel & 0xff000000 ) >> 24 );
          }
          //(this->*CopyLineProc)( X1, Y1 + y, Width, ZX, ZY + y, pCDTarget );
        }
        return;
      }

      for ( int y = 0; y < Height; y++ )
      {
        (this->*CopyLineProc)( X1, Y1 + y, Width, ZX, ZY + y, pCDTarget );
      }
    }



    void ContextDescriptor::CopyAreaTransparent( int X1, int Y1, int Width, int Height, int ZX, int ZY, 
                                                  GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      GR::u32   Transparent = (GR::u32)MapColor( Color );
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      for ( int y = 0; y < Height; y++ )
      {
        CopyLineTransparent( X1, Y1 + y,
                             Width,
                             ZX, ZY + y,
                             Transparent,
                             pCDTarget );
      }
    }



    void ContextDescriptor::CopyAreaAdditive( int X1, int Y1, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        dh::Log( "ContextDescriptor::CopyArea  Format not matching  (%d != %d)",
                m_ImageFormat, pCDTarget->ImageFormat() );
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          dh::Log( "CopyArea: ConvertSelfTo failed" );
          return;
        }
        dh::Log( "CopyArea - %x (%d,%d) Converted To %d!", this, this->Width(), this->Height(), m_BitDepth );
      }
      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // TODO - einzelne Fälle optimieren!
      for ( int y = 0; y < Height; y++ )
      {
        for ( int x = 0; x < Width; ++x )
        {
          GR::u32   PixelSource = pCDTarget->GetPixel( ZX + x, ZY + y );
          GR::u32   PixelTarget = GetPixel( X1 + x, Y1 + y );

          int       iA = ( ( PixelTarget & 0xff000000 ) >> 24 ) + ( ( PixelSource & 0xff000000 ) >> 24 );
          int       r = ( ( PixelTarget & 0x00ff0000 ) >> 16 ) + ( ( PixelSource & 0x00ff0000 ) >> 16 );
          int       g = ( ( PixelTarget & 0x0000ff00 ) >> 8 )  + ( ( PixelSource & 0x0000ff00 ) >> 8 );
          int       b = ( ( PixelTarget & 0x000000ff ) + ( PixelSource & 0x000000ff ) );

          if ( iA > 255 )
          {
            iA = 255;
          }
          if ( r > 255 )
          {
            r = 255;
          }
          if ( g > 255 )
          {
            g = 255;
          }
          if ( b > 255 )
          {
            b = 255;
          }

          pCDTarget->PutPixel( ZX + x, ZY + y, ( iA << 24 ) + ( r << 16 ) + ( g << 8 ) + b );
        }
      }
    }



    void ContextDescriptor::CopyAreaAdditiveTransparent( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        dh::Log( "ContextDescriptor::CopyArea  Format not matching  (%d != %d)",
                m_ImageFormat, pCDTarget->ImageFormat() );
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          dh::Log( "CopyArea: ConvertSelfTo failed" );
          return;
        }
        dh::Log( "CopyArea - %x (%d,%d) Converted To %d!", this, this->Width(), this->Height(), m_BitDepth );
      }
      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      GR::u32   TransparentMapped = (GR::u32)MapColor( Transparent );

      // TODO - einzelne Fälle optimieren!
      for ( int y = 0; y < Height; y++ )
      {
        for ( int x = 0; x < Width; ++x )
        {
          GR::u32   PixelSource = pCDTarget->GetPixel( ZX + x, ZY + y );

          GR::u32   PixelTarget = GetPixel( X1 + x, Y1 + y );

          if ( (GR::u32)MapColor( PixelTarget ) == TransparentMapped )
          {
            continue;
          }

          int       a = ( ( PixelTarget & 0xff000000 ) >> 24 ) + ( ( PixelSource & 0xff000000 ) >> 24 );
          int       r = ( ( PixelTarget & 0x00ff0000 ) >> 16 ) + ( ( PixelSource & 0x00ff0000 ) >> 16 );
          int       g = ( ( PixelTarget & 0x0000ff00 ) >> 8 )  + ( ( PixelSource & 0x0000ff00 ) >> 8 );
          int       b = ( ( PixelTarget & 0x000000ff ) + ( PixelSource & 0x000000ff ) );

          if ( a > 255 )
          {
            a = 255;
          }
          if ( r > 255 )
          {
            r = 255;
          }
          if ( g > 255 )
          {
            g = 255;
          }
          if ( b > 255 )
          {
            b = 255;
          }

          pCDTarget->PutPixel( ZX + x, ZY + y, ( a << 24 ) + ( r << 16 ) + ( g << 8 ) + b );
        }
      }
    }



    void ContextDescriptor::CopyAreaAdditiveTransparentColorized( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Transparent, ContextDescriptor* pCDTarget, GR::u32 Color )
    {
      if ( Color == 0xffffffff )
      {
        CopyAreaAdditiveTransparent( X1, Y1, Width, Height, ZX, ZY, Transparent, pCDTarget );
        return;
      }

      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        dh::Log( "ContextDescriptor::CopyArea  Format not matching  (%d != %d)",
                m_ImageFormat, pCDTarget->ImageFormat() );
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          dh::Log( "CopyArea: ConvertSelfTo failed" );
          return;
        }
        dh::Log( "CopyArea - %x (%d,%d) Converted To %d!", this, this->Width(), this->Height(), m_BitDepth );
      }
      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      GR::u32 TransparentMapped = (GR::u32)MapColor( Transparent );

      // TODO - einzelne Fälle optimieren!
      for ( int y = 0; y < Height; y++ )
      {
        for ( int x = 0; x < Width; ++x )
        {
          GR::u32   PixelTarget = pCDTarget->GetPixel( ZX + x, ZY + y );

          GR::u32   PixelSource = GetPixel( X1 + x, Y1 + y );

          if ( (GR::u32)MapColor( PixelSource ) == TransparentMapped )
          {
            continue;
          }

          PixelSource = GFX::Modulate( PixelSource, Color );

          int       a = ( ( PixelTarget & 0xff000000 ) >> 24 ) + ( ( PixelSource & 0xff000000 ) >> 24 );
          int       r = ( ( PixelTarget & 0x00ff0000 ) >> 16 ) + ( ( PixelSource & 0x00ff0000 ) >> 16 );
          int       g = ( ( PixelTarget & 0x0000ff00 ) >> 8 )  + ( ( PixelSource & 0x0000ff00 ) >> 8 );
          int       b = ( ( PixelTarget & 0x000000ff ) + ( PixelSource & 0x000000ff ) );

          if ( a > 255 )
          {
            a = 255;
          }
          if ( r > 255 )
          {
            r = 255;
          }
          if ( g > 255 )
          {
            g = 255;
          }
          if ( b > 255 )
          {
            b = 255;
          }

          pCDTarget->PutPixel( ZX + x, ZY + y, ( a << 24 ) + ( r << 16 ) + ( g << 8 ) + b );
        }
      }
    }



    void ContextDescriptor::CopyRotated( int X, int Y, float Angle, float ScaleX, float ScaleY, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int     deltaUProZeile = (int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProZeile = (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY ),

              deltaUProSpalte =  (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProSpalte = -(int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY );

      float   srcRotCX = (float)m_Width / 2.0f,
              srcRotCY = (float)m_Height / 2.0f,
              dstRotCX = float( X ),
              dstRotCY = float( Y );

      int     X1 = 0,
              Y1 = 0,
              X2 = pCDTarget->m_Width - 1,
              Y2 = pCDTarget->m_Height - 1;

      double  cx, 
              cy, 
              sx, 
              sy;

      int     destinationWidthHalf, 
              destinationHeightHalf;

      double sanglezoom, canglezoom;

      // Determine destination width and height by rotating a centered source box 
      sanglezoom = sin( -Angle * ( 3.1415926f / 180.0f ) );
      canglezoom = cos( -Angle * ( 3.1415926f / 180.0f ) );

      cx = canglezoom * ( m_Width / 2 ) * ScaleX;
      cy = canglezoom * ( m_Height / 2 ) * ScaleY;
      sx = sanglezoom * ( m_Width / 2 ) * ScaleX;
      sy = sanglezoom * ( m_Height / 2 ) * ScaleY;

      destinationWidthHalf = math::maxValue( (int)ceil( math::maxValue(
                                        math::maxValue(
                                                          math::maxValue( fabs( cx + sy ),
                                                                      fabs( cx - sy ) ), 
                                                               fabs( -cx + sy ) ), 
                                                        fabs( -cx - sy ) ) ), 
                                      1 );

      destinationHeightHalf = math::maxValue( (int)ceil( math::maxValue(
                                        math::maxValue(
                                                           math::maxValue( fabs( sx + cy ),
                                                                       fabs( sx - cy ) ), 
                                                                fabs( -sx + cy ) ), 
                                                         fabs( -sx - cy ) ) ),
                                       1 );

      X1 = X - destinationWidthHalf;
      Y1 = Y - destinationHeightHalf;
      X2 = X + destinationWidthHalf;
      Y2 = Y + destinationHeightHalf;

      if ( X1 < pCDTarget->m_ClipLeft )
      {
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int   startingu = (int)( srcRotCX * 65536.0 ),
            startingv = (int)( srcRotCY * 65536.0 );

      startingu -= (int)( dstRotCY * deltaUProZeile + dstRotCX * deltaUProSpalte );
      startingv -= (int)( dstRotCY * deltaVProZeile + dstRotCX * deltaVProSpalte );
      startingu += (int)( Y1 * deltaUProZeile + X1 * deltaUProSpalte );
      startingv += (int)( Y1 * deltaVProZeile + X1 * deltaVProSpalte );

      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      for ( int y = Y1; y <= Y2; y++) 
      {
        u = rowu;
        v = rowv;
        for ( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            CopyPixel( pCDTarget->GetRowColumnData( x, y ), GetRowColumnData( su, sv ) );
          }
          u += deltaUProSpalte;
          v += deltaVProSpalte;
        }
        rowu += deltaUProZeile;
        rowv += deltaVProZeile;
      }
    }



    void ContextDescriptor::CopyRotatedTransparent( int X, int Y, float Angle, float ScaleX, float ScaleY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int     deltaUProZeile = (int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProZeile = (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY ),

              deltaUProSpalte =  (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProSpalte = -(int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY );

      float   srcRotCX = (float)m_Width / 2.0f,
              srcRotCY = (float)m_Height / 2.0f,
              dstRotCX = float( X ),
              dstRotCY = float( Y );

      int     X1 = 0,
              Y1 = 0,
              X2 = pCDTarget->m_Width - 1,
              Y2 = pCDTarget->m_Height - 1;

      double  cx, 
              cy, 
              sx, 
              sy;

      int     destinationWidthHalf, 
              destinationHeightHalf;

      double sanglezoom, canglezoom;

      // Determine destination width and height by rotating a centered source box 
      sanglezoom = sin( -Angle * ( 3.1415926f / 180.0f ) );
      canglezoom = cos( -Angle * ( 3.1415926f / 180.0f ) );

      cx = canglezoom * ( m_Width / 2 ) * ScaleX;
      cy = canglezoom * ( m_Height / 2 ) * ScaleY;
      sx = sanglezoom * ( m_Width / 2 ) * ScaleX;
      sy = sanglezoom * ( m_Height / 2 ) * ScaleY;

      destinationWidthHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                          math::maxValue( fabs( cx + sy ),
                                                                      fabs( cx - sy ) ), 
                                                               fabs( -cx + sy ) ), 
                                                        fabs( -cx - sy ) ) ), 
                                      1 );

      destinationHeightHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                           math::maxValue( fabs( sx + cy ),
                                                                       fabs( sx - cy ) ), 
                                                                fabs( -sx + cy ) ), 
                                                         fabs( -sx - cy ) ) ),
                                       1 );

      X1 = X - destinationWidthHalf;
      Y1 = Y - destinationHeightHalf;
      X2 = X + destinationWidthHalf;
      Y2 = Y + destinationHeightHalf;

      if ( X1 < pCDTarget->m_ClipLeft )
      {
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int   startingu = (int)( srcRotCX * 65536.0 ),
            startingv = (int)( srcRotCY * 65536.0 );

      startingu -= (int)( dstRotCY * deltaUProZeile + dstRotCX * deltaUProSpalte );
      startingv -= (int)( dstRotCY * deltaVProZeile + dstRotCX * deltaVProSpalte );
      startingu += (int)( Y1 * deltaUProZeile + X1 * deltaUProSpalte );
      startingv += (int)( Y1 * deltaVProZeile + X1 * deltaVProSpalte );

      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      GR::u32   Pixel,
              TrueTransparent = (GR::u32)MapColor( Transparent );

      for( int y = Y1; y <= Y2; y++) 
      {
        u = rowu;
        v = rowv;
        for( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            Pixel = GetDirectPixel( su, sv );

            if ( Pixel != TrueTransparent )
            {
              CopyPixel( pCDTarget->GetRowColumnData( x, y ),
                         GetRowColumnData( su, sv ) );
            }
          }
          u += deltaUProSpalte;
          v += deltaVProSpalte;
        }
        rowu += deltaUProZeile;
        rowv += deltaVProZeile;
      }
    }



    void ContextDescriptor::CopyRotatedAlpha( int X, int Y, float Angle, float ScaleX, float ScaleY, int Alpha, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int     deltaUProZeile = (int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProZeile = (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY ),

              deltaUProSpalte =  (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProSpalte = -(int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY );

      float   srcRotCX = (float)m_Width / 2.0f,
              srcRotCY = (float)m_Height / 2.0f,
              dstRotCX = float( X ),
              dstRotCY = float( Y );

      int     X1 = 0,
              Y1 = 0,
              X2 = pCDTarget->m_Width - 1,
              Y2 = pCDTarget->m_Height - 1;

      double  cx, 
              cy, 
              sx, 
              sy;

      int     destinationWidthHalf, 
              destinationHeightHalf;

      double sanglezoom, canglezoom;

      // Determine destination width and height by rotating a centered source box 
      sanglezoom = sin( -Angle * ( 3.1415926f / 180.0f ) );
      canglezoom = cos( -Angle * ( 3.1415926f / 180.0f ) );

      cx = canglezoom * ( m_Width / 2 ) * ScaleX;
      cy = canglezoom * ( m_Height / 2 ) * ScaleY;
      sx = sanglezoom * ( m_Width / 2 ) * ScaleX;
      sy = sanglezoom * ( m_Height / 2 ) * ScaleY;

      destinationWidthHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                          math::maxValue( fabs( cx + sy ),
                                                                      fabs( cx - sy ) ), 
                                                               fabs( -cx + sy ) ), 
                                                        fabs( -cx - sy ) ) ), 
                                      1 );

      destinationHeightHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                           math::maxValue( fabs( sx + cy ),
                                                                       fabs( sx - cy ) ), 
                                                                fabs( -sx + cy ) ), 
                                                         fabs( -sx - cy ) ) ),
                                       1 );

      X1 = X - destinationWidthHalf;
      Y1 = Y - destinationHeightHalf;
      X2 = X + destinationWidthHalf;
      Y2 = Y + destinationHeightHalf;

      if ( X1 < pCDTarget->m_ClipLeft )
      {
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int   startingu = (int)( srcRotCX * 65536.0 ),
            startingv = (int)( srcRotCY * 65536.0 );

      startingu -= (int)( dstRotCY * deltaUProZeile + dstRotCX * deltaUProSpalte );
      startingv -= (int)( dstRotCY * deltaVProZeile + dstRotCX * deltaVProSpalte );
      startingu += (int)( Y1 * deltaUProZeile + X1 * deltaUProSpalte );
      startingv += (int)( Y1 * deltaVProZeile + X1 * deltaVProSpalte );

      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      for( int y = Y1; y <= Y2; y++) 
      {
        u = rowu;
        v = rowv;
        for( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            CopyPixelAlpha( pCDTarget->GetRowColumnData( x, y ),
                            GetRowColumnData( su, sv ),
                            Alpha );
          }
          u += deltaUProSpalte;
          v += deltaVProSpalte;
        }
        rowu += deltaUProZeile;
        rowv += deltaVProZeile;
      }
    }



    void ContextDescriptor::CopyAreaAlphaAndMaskRotated( int X, int Y, float Angle, float ScaleX, float ScaleY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( Alpha == 0 )
      {
        return;
      }

      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int     deltaUProZeile = (int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProZeile = (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY ),

              deltaUProSpalte =  (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProSpalte = -(int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY );

      float   srcRotCX = (float)m_Width / 2.0f,
              srcRotCY = (float)m_Height / 2.0f,
              dstRotCX = float( X ),
              dstRotCY = float( Y );

      int     X1 = 0,
              Y1 = 0,
              X2 = pCDTarget->m_Width - 1,
              Y2 = pCDTarget->m_Height - 1;

      double  cx, 
              cy, 
              sx, 
              sy;

      int     destinationWidthHalf, 
              destinationHeightHalf;

      double sanglezoom, canglezoom;

      // Determine destination width and height by rotating a centered source box 
      sanglezoom = sin( -Angle * ( 3.1415926f / 180.0f ) );
      canglezoom = cos( -Angle * ( 3.1415926f / 180.0f ) );

      cx = canglezoom * ( m_Width / 2 ) * ScaleX;
      cy = canglezoom * ( m_Height / 2 ) * ScaleY;
      sx = sanglezoom * ( m_Width / 2 ) * ScaleX;
      sy = sanglezoom * ( m_Height / 2 ) * ScaleY;

      destinationWidthHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                          math::maxValue( fabs( cx + sy ),
                                                                      fabs( cx - sy ) ), 
                                                               fabs( -cx + sy ) ), 
                                                        fabs( -cx - sy ) ) ), 
                                      1 );

      destinationHeightHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                           math::maxValue( fabs( sx + cy ),
                                                                       fabs( sx - cy ) ), 
                                                                fabs( -sx + cy ) ), 
                                                         fabs( -sx - cy ) ) ),
                                       1 );

      X1 = X - destinationWidthHalf;
      Y1 = Y - destinationHeightHalf;
      X2 = X + destinationWidthHalf;
      Y2 = Y + destinationHeightHalf;

      if ( X1 < pCDTarget->m_ClipLeft )
      {
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int   startingu = (int)( srcRotCX * 65536.0 ),
            startingv = (int)( srcRotCY * 65536.0 );

      startingu -= (int)( dstRotCY * deltaUProZeile + dstRotCX * deltaUProSpalte );
      startingv -= (int)( dstRotCY * deltaVProZeile + dstRotCX * deltaVProSpalte );
      startingu += (int)( Y1 * deltaUProZeile + X1 * deltaUProSpalte );
      startingv += (int)( Y1 * deltaVProZeile + X1 * deltaVProSpalte );

      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      int     maskAlpha = 0;

      for ( int y = Y1; y <= Y2; y++ ) 
      {
        u = rowu;
        v = rowv;
        for ( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            maskAlpha = *(GR::u8*)pCDMask->GetRowColumnData( su, sv );

            maskAlpha = ( maskAlpha * Alpha ) / 255;

            if ( maskAlpha == 255 )
            {
              pCDTarget->PutPixel( x, y, GetPixel( su, sv ) );
            }
            else if ( maskAlpha > 0 )
            {
              pCDTarget->PutPixelAlpha( x, y, GetPixel( su, sv ), maskAlpha );
            }
          }
          u += deltaUProSpalte;
          v += deltaVProSpalte;
        }
        rowu += deltaUProZeile;
        rowv += deltaVProZeile;
      }
    }



    void ContextDescriptor::CopyAreaAlphaAndMaskRotatedColorized( int X, int Y, float Angle, float ScaleX, float ScaleY, int Alpha, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( Alpha == 0 )
      {
        return;
      }
      if ( Color == 0xffffffff )
      {
        CopyAreaAlphaAndMaskRotated( X, Y, Angle, ScaleX, ScaleY, Alpha, pCDMask, pCDTarget );
        return;
      }

      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int     deltaUProZeile = (int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProZeile = (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY ),

              deltaUProSpalte =  (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProSpalte = -(int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY );

      float   srcRotCX = (float)m_Width / 2.0f,
              srcRotCY = (float)m_Height / 2.0f,
              dstRotCX = float( X ),
              dstRotCY = float( Y );

      int     X1 = 0,
              Y1 = 0,
              X2 = pCDTarget->m_Width - 1,
              Y2 = pCDTarget->m_Height - 1;

      double  cx, 
              cy, 
              sx, 
              sy;

      int     destinationWidthHalf, 
              destinationHeightHalf;

      double sanglezoom, canglezoom;

      // Determine destination width and height by rotating a centered source box 
      sanglezoom = sin( -Angle * ( 3.1415926f / 180.0f ) );
      canglezoom = cos( -Angle * ( 3.1415926f / 180.0f ) );

      cx = canglezoom * ( m_Width / 2 ) * ScaleX;
      cy = canglezoom * ( m_Height / 2 ) * ScaleY;
      sx = sanglezoom * ( m_Width / 2 ) * ScaleX;
      sy = sanglezoom * ( m_Height / 2 ) * ScaleY;

      destinationWidthHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                          math::maxValue( fabs( cx + sy ),
                                                                      fabs( cx - sy ) ), 
                                                               fabs( -cx + sy ) ), 
                                                        fabs( -cx - sy ) ) ), 
                                      1 );

      destinationHeightHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                           math::maxValue( fabs( sx + cy ),
                                                                       fabs( sx - cy ) ), 
                                                                fabs( -sx + cy ) ), 
                                                         fabs( -sx - cy ) ) ),
                                       1 );

      X1 = X - destinationWidthHalf;
      Y1 = Y - destinationHeightHalf;
      X2 = X + destinationWidthHalf;
      Y2 = Y + destinationHeightHalf;

      if ( X1 < pCDTarget->m_ClipLeft )
      {
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int   startingu = (int)( srcRotCX * 65536.0 ),
            startingv = (int)( srcRotCY * 65536.0 );

      startingu -= (int)( dstRotCY * deltaUProZeile + dstRotCX * deltaUProSpalte );
      startingv -= (int)( dstRotCY * deltaVProZeile + dstRotCX * deltaVProSpalte );
      startingu += (int)( Y1 * deltaUProZeile + X1 * deltaUProSpalte );
      startingv += (int)( Y1 * deltaVProZeile + X1 * deltaVProSpalte );

      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      int     maskAlpha = 0;

      for ( int y = Y1; y <= Y2; y++ ) 
      {
        u = rowu;
        v = rowv;
        for ( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            maskAlpha = *(GR::u8*)pCDMask->GetRowColumnData( su, sv );

            maskAlpha = ( maskAlpha * Alpha ) / 255;

            if ( maskAlpha == 255 )
            {
              GR::u32       Pixel = GetPixel( su, sv );
              Pixel = GFX::Modulate( Pixel, Color );

              pCDTarget->PutPixel( x, y, Pixel );
            }
            else if ( maskAlpha > 0 )
            {
              GR::u32       Pixel = GetPixel( su, sv );
              Pixel = GFX::Modulate( Pixel, Color );

              pCDTarget->PutPixelAlpha( x, y, Pixel, maskAlpha );
            }
          }
          u += deltaUProSpalte;
          v += deltaVProSpalte;
        }
        rowu += deltaUProZeile;
        rowv += deltaVProZeile;
      }
    }



    void ContextDescriptor::CopyRotatedAdditive( int X, int Y, float Angle, float ScaleX, float ScaleY, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int     deltaUProZeile = (int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProZeile = (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY ),

              deltaUProSpalte =  (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProSpalte = -(int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY );

      float   srcRotCX = (float)m_Width / 2.0f,
              srcRotCY = (float)m_Height / 2.0f,
              dstRotCX = float( X ),
              dstRotCY = float( Y );

      int     X1 = 0,
              Y1 = 0,
              X2 = pCDTarget->m_Width - 1,
              Y2 = pCDTarget->m_Height - 1;

      double  cx, 
              cy, 
              sx, 
              sy;

      int     destinationWidthHalf, 
              destinationHeightHalf;

      double sanglezoom, canglezoom;

      // Determine destination width and height by rotating a centered source box 
      sanglezoom = sin( -Angle * ( 3.1415926f / 180.0f ) );
      canglezoom = cos( -Angle * ( 3.1415926f / 180.0f ) );

      cx = canglezoom * ( m_Width / 2 ) * ScaleX;
      cy = canglezoom * ( m_Height / 2 ) * ScaleY;
      sx = sanglezoom * ( m_Width / 2 ) * ScaleX;
      sy = sanglezoom * ( m_Height / 2 ) * ScaleY;

      destinationWidthHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                          math::maxValue( fabs( cx + sy ),
                                                                      fabs( cx - sy ) ), 
                                                               fabs( -cx + sy ) ), 
                                                        fabs( -cx - sy ) ) ), 
                                      1 );

      destinationHeightHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                           math::maxValue( fabs( sx + cy ),
                                                                       fabs( sx - cy ) ), 
                                                                fabs( -sx + cy ) ), 
                                                         fabs( -sx - cy ) ) ),
                                       1 );

      X1 = X - destinationWidthHalf;
      Y1 = Y - destinationHeightHalf;
      X2 = X + destinationWidthHalf;
      Y2 = Y + destinationHeightHalf;

      if ( X1 < pCDTarget->m_ClipLeft )
      {
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      int   startingu = (int)( srcRotCX * 65536.0 ),
            startingv = (int)( srcRotCY * 65536.0 );

      startingu -= (int)( dstRotCY * deltaUProZeile + dstRotCX * deltaUProSpalte );
      startingv -= (int)( dstRotCY * deltaVProZeile + dstRotCX * deltaVProSpalte );
      startingu += (int)( Y1 * deltaUProZeile + X1 * deltaUProSpalte );
      startingv += (int)( Y1 * deltaVProZeile + X1 * deltaVProSpalte );

      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      GR::u32   Pixel,
                TrueTransparent = (GR::u32)MapColor( Transparent );

      for( int y = Y1; y <= Y2; y++) 
      {
        u = rowu;
        v = rowv;
        for( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            Pixel = GetDirectPixel( su, sv );

            if ( Pixel != TrueTransparent )
            {
              GR::u32   PixelTarget = pCDTarget->GetPixel( x, y );
              GR::u32   PixelSource = GetPixel( su, sv );

              int       iA = ( ( PixelTarget & 0xff000000 ) >> 24 ) + ( ( PixelSource & 0xff000000 ) >> 24 );
              int       r = ( ( PixelTarget & 0x00ff0000 ) >> 16 ) + ( ( PixelSource & 0x00ff0000 ) >> 16 );
              int       g = ( ( PixelTarget & 0x0000ff00 ) >> 8 )  + ( ( PixelSource & 0x0000ff00 ) >> 8 );
              int       b = ( ( PixelTarget & 0x000000ff ) + ( PixelSource & 0x000000ff ) );

              if ( iA > 255 )
              {
                iA = 255;
              }
              if ( r > 255 )
              {
                r = 255;
              }
              if ( g > 255 )
              {
                g = 255;
              }
              if ( b > 255 )
              {
                b = 255;
              }

              pCDTarget->PutPixel( x, y, ( iA << 24 ) + ( r << 16 ) + ( g << 8 ) + b );

              /*
              CopyPixel( pCDTarget->GetRowColumnData( x, y ),
                         GetRowColumnData( su, sv ) );
                         */
            }
          }
          u += deltaUProSpalte;
          v += deltaVProSpalte;
        }
        rowu += deltaUProZeile;
        rowv += deltaVProZeile;
      }
    }



    void ContextDescriptor::CopyRotatedAdditiveColorized( int X, int Y, float Angle, float ScaleX, float ScaleY, GR::u32 Transparent, GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int     deltaUProZeile = (int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProZeile = (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY ),

              deltaUProSpalte =  (int)( cos( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleX ),
              deltaVProSpalte = -(int)( sin( -Angle * 3.1415926f / 180.0f ) * 65536.0 / ScaleY );

      float   srcRotCX = (float)m_Width / 2.0f,
              srcRotCY = (float)m_Height / 2.0f,
              dstRotCX = float( X ),
              dstRotCY = float( Y );

      int     X1 = 0,
              Y1 = 0,
              X2 = pCDTarget->m_Width - 1,
              Y2 = pCDTarget->m_Height - 1;

      double  cx, 
              cy, 
              sx, 
              sy;

      int     destinationWidthHalf, 
              destinationHeightHalf;

      double sanglezoom, canglezoom;

      // Determine destination width and height by rotating a centered source box 
      sanglezoom = sin( -Angle * ( 3.1415926f / 180.0f ) );
      canglezoom = cos( -Angle * ( 3.1415926f / 180.0f ) );

      cx = canglezoom * ( m_Width / 2 ) * ScaleX;
      cy = canglezoom * ( m_Height / 2 ) * ScaleY;
      sx = sanglezoom * ( m_Width / 2 ) * ScaleX;
      sy = sanglezoom * ( m_Height / 2 ) * ScaleY;

      destinationWidthHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                          math::maxValue( fabs( cx + sy ),
                                                                      fabs( cx - sy ) ), 
                                                               fabs( -cx + sy ) ), 
                                                        fabs( -cx - sy ) ) ), 
                                      1 );

      destinationHeightHalf = math::maxValue( (int)ceil( math::maxValue(
        math::maxValue(
                                                           math::maxValue( fabs( sx + cy ),
                                                                       fabs( sx - cy ) ), 
                                                                fabs( -sx + cy ) ), 
                                                         fabs( -sx - cy ) ) ),
                                       1 );

      X1 = X - destinationWidthHalf;
      Y1 = Y - destinationHeightHalf;
      X2 = X + destinationWidthHalf;
      Y2 = Y + destinationHeightHalf;

      if ( X1 < pCDTarget->m_ClipLeft )
      {
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      int   startingu = (int)( srcRotCX * 65536.0 ),
            startingv = (int)( srcRotCY * 65536.0 );

      startingu -= (int)( dstRotCY * deltaUProZeile + dstRotCX * deltaUProSpalte );
      startingv -= (int)( dstRotCY * deltaVProZeile + dstRotCX * deltaVProSpalte );
      startingu += (int)( Y1 * deltaUProZeile + X1 * deltaUProSpalte );
      startingv += (int)( Y1 * deltaVProZeile + X1 * deltaVProSpalte );

      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      GR::u32   Pixel,
                TrueTransparent = (GR::u32)MapColor( Transparent );

      for ( int y = Y1; y <= Y2; y++) 
      {
        u = rowu;
        v = rowv;
        for ( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            Pixel = GetDirectPixel( su, sv );

            if ( Pixel != TrueTransparent )
            {
              GR::u32   PixelTarget = pCDTarget->GetPixel( x, y );
              GR::u32   PixelSource = GFX::Modulate( GetPixel( su, sv ), Color );

              int       iA = ( ( PixelTarget & 0xff000000 ) >> 24 ) + ( ( PixelSource & 0xff000000 ) >> 24 );
              int       r = ( ( PixelTarget & 0x00ff0000 ) >> 16 ) + ( ( PixelSource & 0x00ff0000 ) >> 16 );
              int       g = ( ( PixelTarget & 0x0000ff00 ) >> 8 )  + ( ( PixelSource & 0x0000ff00 ) >> 8 );
              int       b = ( ( PixelTarget & 0x000000ff ) + ( PixelSource & 0x000000ff ) );

              if ( iA > 255 )
              {
                iA = 255;
              }
              if ( r > 255 )
              {
                r = 255;
              }
              if ( g > 255 )
              {
                g = 255;
              }
              if ( b > 255 )
              {
                b = 255;
              }

              pCDTarget->PutPixel( x, y, ( iA << 24 ) + ( r << 16 ) + ( g << 8 ) + b );
            }
          }
          u += deltaUProSpalte;
          v += deltaVProSpalte;
        }
        rowu += deltaUProZeile;
        rowv += deltaVProZeile;
      }
    }



    void ContextDescriptor::_CopyAreaAlphaSlow( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }
      if ( Alpha == 0 )
      {
        return;
      }
      if ( Alpha == 255 )
      {
        CopyArea( X1, Y1, Width, Height, ZX, ZY, pCDTarget );
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      GR::u32       Src,
                  Target;


      for ( int j = 0; j < Height; j++ )
      {
        for ( int i = 0; i < Width; ++i )
        {
          Src     = GetPixel( X1, Y1 + j );
          Target  = pCDTarget->GetPixel( ZX + i, ZY + j );

          pCDTarget->PutPixel( ZX + i, ZY + j, GFX::AlphaFade( Src, Target, Alpha ) );
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlpha555( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget )
    {
      if ( m_NothingVisible )
      {
        return;
      }
      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      GR::u16        *pP1,
                  *pDest;

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u16*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha555( pDest, pP1, Alpha );
          ++pDest;

          ++pP1;
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlpha565( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget )
    {
      if ( m_NothingVisible )
      {
        return;
      }
      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      GR::u16*  pP1;
      GR::u16*  pDest;

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u16*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha565( pDest, pP1, Alpha );
          ++pDest;

          ++pP1;
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlpha24( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget )
    {
      if ( m_NothingVisible )
      {
        return;
      }
      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      GR::u8*   pP1;
      GR::u8*   pDest;

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha24( pDest, pP1, Alpha );
          pDest += 3;
          pP1 += 3;
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlpha32( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget )
    {
      if ( m_NothingVisible )
      {
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      GR::u32*    pP1;
      GR::u32*    pDest;

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u32*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1   = (GR::u32*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha32( pDest, pP1, Alpha );
          ++pDest;
          ++pP1;
        }
      }
    }



    void ContextDescriptor::MirrorH( int X1, int Y1, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }

      if ( m_NothingVisible )
      {
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        //X1 += pCDTarget->m_ClipLeft - ZX;
        xOffset += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        //Y1 += pCDTarget->m_ClipTop - ZY;
        yOffset += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      GR::u32*    pP1;
      GR::u32*    pDest;

      for ( int j = 0; j < Height; j++ )
      {
        for ( int i = 0; i < Width; ++i )
        {
          pP1   = (GR::u32*)GetRowColumnData( X1 + m_Width - 1 - i - xOffset, Y1 + j + yOffset );
          pDest = (GR::u32*)pCDTarget->GetRowColumnData( ZX + i, ZY + j );

          CopyPixel( pDest, pP1 );
        }
      }
    }



    void ContextDescriptor::MirrorV( int X1, int Y1, int Width, int Height, int ZX, int ZY, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }

      if ( m_NothingVisible )
      {
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        //X1 += pCDTarget->m_ClipLeft - ZX;
        xOffset += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        //Y1 += pCDTarget->m_ClipTop - ZY;
        yOffset += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // was jetzt noch übrig ist kopieren!
      GR::u32     *pP1,
                  *pDest;

      for ( int j = 0; j < Height; j++ )
      {
        pP1   = (GR::u32*)GetRowColumnData( X1 + xOffset, Y1 + m_Height - 1 - j - yOffset );
        pDest = (GR::u32*)pCDTarget->GetRowColumnData( ZX, ZY + j );

        memcpy( pDest, pP1, Width * BytesProPixel() );
      }
    }



    void ContextDescriptor::CopyAreaAlpha( int X, int Y, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDTarget )
    {
      if ( Alpha == 0 )
      {
        return;
      }
      // convert before the call; we might have to use a different proc!
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      (this->*CopyAreaAlphaProc)( X, Y, Width, Height, ZX, ZY, Alpha, pCDTarget );
    }



    void ContextDescriptor::_CopyAreaAlphaTransparent555( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u16*    pP1;
      GR::u16*    pDest;

      GR::u32     TrueTransparent = (GR::u32)MapColor( Transparent );

      int         r,
                  g,
                  b,
                  r2,
                  g2,
                  b2;

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u16*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          if ( *pP1 != TrueTransparent )
          {
            if ( Alpha == 255 )
            {
              *pDest = *pP1;
            }
            else
            {
              r = ( ( *pP1 & 0x7c00 ) >> 10 );
              g = ( ( *pP1 & 0x03e0 ) >>  5 );
              b = ( ( *pP1 & 0x001f )       );

              r2 = ( ( *pDest & 0x7c00 ) >> 10 );
              g2 = ( ( *pDest & 0x03e0 ) >>  5 );
              b2 = ( ( *pDest & 0x001f )       );

              r = ( ( r * Alpha ) + ( r2 * ( 255 - Alpha ) ) ) / 255;
              g = ( ( g * Alpha ) + ( g2 * ( 255 - Alpha ) ) ) / 255;
              b = ( ( b * Alpha ) + ( b2 * ( 255 - Alpha ) ) ) / 255;

              *pDest = (GR::u16)( ( r << 10 ) + ( g << 5 ) + b );
            }
          }
          ++pDest;

          ++pP1;
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlphaTransparent565( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u16*    pP1;
      GR::u16*    pDest;

      GR::u32     TrueTransparent = (GR::u32)MapColor( Transparent );

      int         r,
                  g,
                  b,
                  r2,
                  g2,
                  b2;

      GR::u16     dest;


      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u16*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          if ( *pP1 != TrueTransparent )
          {
            if ( Alpha == 255 )
            {
              *pDest = *pP1;
            }
            else
            {
              r = ( ( *pP1 & 0xfe00 ) >> 11 );
              g = ( ( *pP1 & 0x07e0 ) >>  5 );
              b = ( ( *pP1 & 0x001f )       );

              dest = *pDest;

              r2 = ( ( dest & 0xfe00 ) >> 11 );
              g2 = ( ( dest & 0x07e0 ) >>  5 );
              b2 = ( ( dest & 0x001f )       );

              r = ( ( r * Alpha ) + ( r2 * ( 255 - Alpha ) ) ) / 255;
              g = ( ( g * Alpha ) + ( g2 * ( 255 - Alpha ) ) ) / 255;
              b = ( ( b * Alpha ) + ( b2 * ( 255 - Alpha ) ) ) / 255;

              *pDest = (GR::u16)( ( r << 11 ) + ( g << 5 ) + b );
            }
          }
          ++pDest;

          ++pP1;
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlphaTransparent24( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u8*     pP1;
      GR::u8*     pDest;

      GR::u32     dummy,
                  dummy2,
                  TrueTransparent = (GR::u32)MapColor( Transparent ),
                  l;

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          GR::u32   Pixel = ( pP1[0] + ( pP1[1] << 8 ) + ( pP1[2] << 16 ) );
          if ( Pixel != TrueTransparent )
          {
            l = 0;
            do
            {
              dummy = *pP1;
              dummy2 = *pDest;

              dummy = ( dummy * Alpha ) + dummy2 * ( 255 - Alpha );

              *pDest = (GR::u8)( dummy / 255 );

              pDest++;
              pP1++;

              l++;
            }
            while ( l < 3 );
          }
          else
          {
            pDest += 3;
            pP1 += 3;
          }
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlphaTransparent32( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      GR::u8*     pP1;
      GR::u8*     pDest;

      GR::u32     dummy,
                  dummy2,
                  TrueTransparent = (GR::u32)MapColor( Transparent ),
                  l;

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          if ( *( (GR::u32*)pP1 ) != Transparent )
          {
            l = 0;
            do
            {
              dummy = *pP1;
              dummy2 = *pDest;

              dummy = ( dummy * Alpha ) + dummy2 * ( 32 - Alpha );

              *pDest = (GR::u8)( dummy >> 5 );

              ++pDest;
              ++pP1;

              ++l;
            }
            while ( l < 3 );
          }
          else
          {
            pDest += 3;
            pP1 += 3;
          }
          ++pDest;
          ++pP1;
        }
      }
    }



    void ContextDescriptor::CopyAreaAlphaTransparent( int X, int Y, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Transparent, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }
      if ( Alpha == 0 )
      {
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X > m_ClipRight )
      ||   ( Y > m_ClipBottom )
      ||   ( X + Width <= m_ClipLeft )
      ||   ( Y + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X;
        Width -= xOffset;
        X = m_ClipLeft;
      }
      if ( Y < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y;
        Height -= yOffset;
        Y = m_ClipTop;
      }
      if ( X + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X + 1;
      }
      if ( Y + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      // Funktionspointer aufrufen
      if ( CopyAreaAlphaTransparentProc )
      {
        (this->*CopyAreaAlphaTransparentProc)( X, Y, Width, Height, ZX, ZY, Alpha, Transparent, pCDTarget );
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMask8( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      GR::u8*     pP1;
      GR::u8*     pDest;
      GR::u8*     pAlpha;

      int         maskAlpha = 0;

      GR::u32     dummy,
                  dummy2,
                  l;

      for ( int j = 0; j < Height; j++ )
      {
        pAlpha = (GR::u8*)pCDMask->GetRowColumnData( X1, Y1 + j );
        pDest = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          l = 0;
          maskAlpha = *pAlpha++;

          maskAlpha = maskAlpha * Alpha / 255;

          if ( maskAlpha == 0 )
          {
            pDest++;
            pP1++;
          }
          else if ( maskAlpha == 255 )
          {
            *pDest++ = *pP1++;
          }
          else
          {
            // TODO - echtes Alphablenden
            dummy = *pP1;
            dummy2 = *pDest;

            dummy = ( dummy * maskAlpha ) + dummy2 * ( 255 - maskAlpha );

            *pDest = (GR::u8)( dummy / 255 );

            pDest++;
            pP1++;
          }
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMask4( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      int       maskAlpha = 0;

      GR::u32   dummy,
                dummy2,
                l;

      for ( int j = 0; j < Height; j++ )
      {
        for ( int i = 0; i < Width; ++i )
        {
          l = 0;
          maskAlpha = pCDMask->GetDirectPixel( X1 + i, Y1 + j );

          maskAlpha = maskAlpha * Alpha / 255;

          if ( maskAlpha == 0 )
          {
          }
          else if ( maskAlpha == 255 )
          {
            pCDTarget->PutDirectPixel( ZX + i, ZY + j, GetDirectPixel( X1 + i, Y1 + j ) );
          }
          else
          {
            // TODO - echtes Alphablenden
            dummy  = GetPixel( X1 + i, Y1 + j );
            dummy2 = pCDTarget->GetPixel( X1 + i, Y1 + j );

            dummy = ( dummy * maskAlpha ) + dummy2 * ( 255 - maskAlpha );

            PutPixel( ZX + i, ZY + j, dummy );
          }
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMask555( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      GR::u16*    pP1;
      GR::u16*    pDest;

      GR::u16     wDestPixel;

      GR::u8*     pAlpha;

      int         maskAlpha = 0;

      int         r,
                  g,
                  b,
                  r2,
                  g2,
                  b2;

      for ( int j = 0; j < Height; j++ )
      {
        pAlpha = (GR::u8*)pCDMask->GetRowColumnData( X1, Y1 + j );
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u16*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          maskAlpha = *pAlpha++;

          if ( maskAlpha > 0 )
          {
            maskAlpha = maskAlpha * Alpha / 255;

            wDestPixel = *pDest;

            r = ( ( *pP1 & 0x7c00 ) >> 10 ) * 255 / 31;
            g = ( ( *pP1 & 0x03e0 ) >> 5 ) * 255 / 31;
            b = ( ( *pP1 & 0x001f ) ) * 255 / 31;

            r2 = ( ( wDestPixel & 0x7c00 ) >> 10 ) * 255 / 31;
            g2 = ( ( wDestPixel & 0x03e0 ) >> 5 ) * 255 / 31;
            b2 = ( ( wDestPixel & 0x001f ) ) * 255 / 31;

            r = ( r * maskAlpha + r2 * ( 255 - maskAlpha ) ) / 255;
            g = ( g * maskAlpha + g2 * ( 255 - maskAlpha ) ) / 255;
            b = ( b * maskAlpha + b2 * ( 255 - maskAlpha ) ) / 255;

            *pDest = (GR::u16)( ( ( r >> 3 ) << 10 )
                           + ( ( g >> 3 ) << 5 )
                           + ( ( b >> 3 ) ) );
          }
          ++pDest;

          ++pP1;
        }
      }

    }



    void ContextDescriptor::_CopyAreaAlphaAndMask565( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      GR::u16*    pP1;
      GR::u16*    pDest;

      GR::u16     wDestPixel;

      GR::u8*     pAlpha;

      int         maskAlpha = 0;

      int         r,
                  g,
                  b,
                  r2,
                  g2,
                  b2;

      for ( int j = 0; j < Height; j++ )
      {
        pAlpha = (GR::u8*)pCDMask->GetRowColumnData( X1, Y1 + j );
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u16*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          maskAlpha = *pAlpha++;

          if ( maskAlpha > 0 )
          {
            maskAlpha = maskAlpha * Alpha / 255;

            wDestPixel = *pDest;

            r = ( ( *pP1 & 0xf800 ) >> 11 ) * 255 / 31;
            g = ( ( *pP1 & 0x07e0 ) >> 5 ) * 255 / 63;
            b = ( ( *pP1 & 0x001f ) ) * 255 / 31;

            r2 = ( ( wDestPixel & 0xf800 ) >> 11 ) * 255 / 31;
            g2 = ( ( wDestPixel & 0x07e0 ) >> 5 ) * 255 / 63;
            b2 = ( ( wDestPixel & 0x001f ) ) * 255 / 31;

            r = ( r * maskAlpha + r2 * ( 255 - maskAlpha ) ) / 255;
            g = ( g * maskAlpha + g2 * ( 255 - maskAlpha ) ) / 255;
            b = ( b * maskAlpha + b2 * ( 255 - maskAlpha ) ) / 255;

            *pDest = (GR::u16)( ( ( r >> 3 ) << 11 )
                           + ( ( g >> 2 ) << 5 )
                           + ( ( b >> 3 ) ) );
          }
          ++pDest;

          ++pP1;
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMask24( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      GR::u8*     pP1;
      GR::u8*     pDest;
      GR::u8*     pAlpha;

      int         maskAlpha = 0;

      GR::u32     dummy,
                  dummy2,
                  l;

      for ( int j = 0; j < Height; j++ )
      {
        pAlpha = (GR::u8*)pCDMask->GetRowColumnData( X1, Y1 + j );
        pDest = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          l = 0;
          maskAlpha = *pAlpha++;

          if ( maskAlpha == 0 )
          {
            pDest += 3;
            pP1 += 3;
          }
          else
          {
            maskAlpha = maskAlpha * Alpha / 255;
            do
            {
              dummy = *pP1;
              dummy2 = *pDest;

              dummy = ( dummy * maskAlpha ) + dummy2 * ( 255 - maskAlpha );

              *pDest = (GR::u8)( dummy / 255 );

              pDest++;
              pP1++;

              l++;
            }
            while ( l < 3 );
          }
        }
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMask32( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      GR::u8*     pP1;
      GR::u8*     pDest;

      GR::u8*     pAlpha;

      int         maskAlpha = 0;

      GR::u32     dummy,
                  dummy2,
                  l;

      for ( int j = 0; j < Height; j++ )
      {
        pAlpha = (GR::u8*)pCDMask->GetRowColumnData( X1, Y1 + j );
        pDest = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pP1 = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          l = 0;
          maskAlpha = *pAlpha++;

          if ( maskAlpha == 0 )
          {
            pDest += 3;
            pP1 += 3;
          }
          else
          {
            maskAlpha = maskAlpha * Alpha / 255;
            do
            {
              dummy = *pP1;
              dummy2 = *pDest;

              dummy = ( dummy * maskAlpha ) + dummy2 * ( 255 - maskAlpha );

              *pDest = (GR::u8)( dummy / 255 );

              ++pDest;
              ++pP1;

              ++l;
            }
            while ( l < 3 );
          }
          ++pDest;
          ++pP1;
        }
      }
    }



    void ContextDescriptor::CopyAreaAlphaAndMask( int X, int Y, int Width, int Height, int ZX, int ZY, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( pCDMask == NULL )
      {
        return;
      }
      if ( pCDMask->m_BitDepth != 8 )
      {
        return;
      }
      if ( ( pCDMask->m_Width != m_Width )
      ||   ( pCDMask->m_Height != m_Height ) )
      {
        return;
      }
      if ( Alpha == 0 )
      {
        return;
      }
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X > m_ClipRight )
      ||   ( Y > m_ClipBottom )
      ||   ( X + Width <= m_ClipLeft )
      ||   ( Y + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X;
        Width -= xOffset;
        X = m_ClipLeft;
      }
      if ( Y < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y;
        Height -= yOffset;
        Y = m_ClipTop;
      }
      if ( X + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X + 1;
      }
      if ( Y + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }
      (this->*CopyAreaAlphaAndMaskProc)( X, Y, Width, Height, ZX, ZY, Alpha, pCDMask, pCDTarget );
    }



    void ContextDescriptor::_CopyAreaAlphaAndMaskColorized( int X1, int Y1, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      GR::u8*     pAlpha;

      int         maskAlpha = 0;


      for ( int j = 0; j < Height; j++ )
      {
        pAlpha = (GR::u8*)pCDMask->GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          maskAlpha = *pAlpha++;

          if ( maskAlpha > 0 )
          {
            maskAlpha = maskAlpha * Alpha / 255;

            GR::u32     Pixel = GetPixel( X1 + i, Y1 + j );

            Pixel = GFX::Modulate( Pixel, Color );

            int   pixelAlpha = ( Pixel & 0xff000000 ) >> 24;

            pixelAlpha = ( pixelAlpha * maskAlpha ) / 255;

            Pixel = ( Pixel & 0x00ffffff ) | ( pixelAlpha << 24 );

            pCDTarget->PutPixel( ZX + i, ZY + j, Pixel );
          }
        }
      }
    }



    void ContextDescriptor::CopyAreaAlphaAndMaskColorized( int X, int Y, int Width, int Height, int ZX, int ZY, int Alpha, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( Color == 0xffffffff )
      {
        CopyAreaAlphaAndMask( X, Y, Width, Height, ZX, ZY, Alpha, pCDMask, pCDTarget );
        return;
      }

      if ( pCDMask == NULL )
      {
        return;
      }
      if ( pCDMask->m_BitDepth != 8 )
      {
        return;
      }
      if ( ( pCDMask->m_Width != m_Width )
      ||   ( pCDMask->m_Height != m_Height ) )
      {
        return;
      }
      if ( Alpha == 0 )
      {
        return;
      }
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X > m_ClipRight )
      ||   ( Y > m_ClipBottom )
      ||   ( X + Width <= m_ClipLeft )
      ||   ( Y + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X;
        Width -= xOffset;
        X = m_ClipLeft;
      }
      if ( Y < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y;
        Height -= yOffset;
        Y = m_ClipTop;
      }
      if ( X + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X + 1;
      }
      if ( Y + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }
      (this->*CopyAreaAlphaAndMaskProc)( X, Y, Width, Height, ZX, ZY, Alpha, pCDMask, pCDTarget );
    }



    GR::Graphic::Image* ContextDescriptor::CreateImage()
    {
      GR::Graphic::Image* pImage = new GR::Graphic::Image( m_Width, m_Height, (GR::u8)m_BitDepth );

      for ( int i = 0; i < m_Height; i++ )
      {
        memcpy( ( (GR::u8*)pImage->GetData() ) + i * BytesProPixel() * m_Width,
                       ( (GR::u8*)m_pData ) + i * m_LineOffsetInBytes,
                       BytesPerLine() );
      }
      return pImage;
    }



    void ContextDescriptor::_CopyPixel8( void* pDest, void* pSource )
    {
      *(GR::u8*)pDest = *(GR::u8*)pSource;
    }



    void ContextDescriptor::_CopyPixel16( void* pDest, void* pSource )
    {
      *(GR::u16*)pDest = *(GR::u16*)pSource;
    }



    void ContextDescriptor::_CopyPixel24( void* pDest, void* pSource )
    {
      *(GR::u16*)pDest = *(GR::u16*)pSource;
      *( ( (GR::u8*)pDest ) + 2 ) = *( ( (GR::u8*)pSource ) + 2 );
    }



    void ContextDescriptor::_CopyPixel32( void* pDest, void* pSource )
    {
      *(GR::u32*)pDest = *(GR::u32*)pSource;
    }



    void ContextDescriptor::CopyPixel( void* pDest, void* pSource )
    {
      (this->*CopyPixelProc)( pDest, pSource );
    }



    void ContextDescriptor::_CopyPixelAlpha555( void* pDest, void* pSource, int Alpha )
    {
      int R = ( ( *(GR::u16*)pSource & 0x7c00 ) >> 10 );
      int G = ( ( *(GR::u16*)pSource & 0x03e0 ) >>  5 );
      int B = ( ( *(GR::u16*)pSource & 0x001f )       );

      int R2 = ( ( *(GR::u16*)pDest & 0x7c00 ) >> 10 );
      int G2 = ( ( *(GR::u16*)pDest & 0x03e0 ) >>  5 );
      int B2 = ( ( *(GR::u16*)pDest & 0x001f )       );

      R = ( ( R * Alpha ) + ( R2 * ( 255 - Alpha ) ) ) / 255;
      G = ( ( G * Alpha ) + ( G2 * ( 255 - Alpha ) ) ) / 255;
      B = ( ( B * Alpha ) + ( B2 * ( 255 - Alpha ) ) ) / 255;

      *(GR::u16*)pDest = (GR::u16)( ( R << 10 ) + ( G << 5 ) + B );
    }



    void ContextDescriptor::_CopyPixelAlpha565( void* pDest, void* pSource, int Alpha )
    {
      int R = ( ( *(GR::u16*)pSource & 0xf800 ) >> 11 );
      int G = ( ( *(GR::u16*)pSource & 0x07e0 ) >>  5 );
      int B = ( ( *(GR::u16*)pSource & 0x001f )       );

      int R2 = ( ( *(GR::u16*)pDest & 0xf800 ) >> 11 );
      int G2 = ( ( *(GR::u16*)pDest & 0x07e0 ) >>  5 );
      int B2 = ( ( *(GR::u16*)pDest & 0x001f )       );

      R = ( ( R * Alpha ) + ( R2 * ( 255 - Alpha ) ) ) / 255;
      G = ( ( G * Alpha ) + ( G2 * ( 255 - Alpha ) ) ) / 255;
      B = ( ( B * Alpha ) + ( B2 * ( 255 - Alpha ) ) ) / 255;

      *(GR::u16*)pDest = (GR::u16)( ( R << 11 ) + ( G << 5 ) + B );
    }



    void ContextDescriptor::_CopyPixelAlpha24( void* pDest, void* pSource, int Alpha )
    {
      GR::u8*     pSrc = (GR::u8*)pSource;
      GR::u8*     pTgt = (GR::u8*)pDest;

      // Rot
      *(GR::u8*)pTgt = (GR::u8)( ( ( *(GR::u8*)pSrc++ * Alpha ) + *(GR::u8*)pTgt * ( 255 - Alpha ) ) / 255 );
      ++pTgt;
      // Grün
      *(GR::u8*)pTgt = (GR::u8)( ( ( *(GR::u8*)pSrc++ * Alpha ) + *(GR::u8*)pTgt * ( 255 - Alpha ) ) / 255 );
      ++pTgt;
      // Blau
      *(GR::u8*)pTgt = (GR::u8)( ( ( *(GR::u8*)pSrc++ * Alpha ) + *(GR::u8*)pTgt * ( 255 - Alpha ) ) / 255 );
      ++pTgt;
    }



    void ContextDescriptor::_CopyPixelAlpha32( void* pDest, void* pSource, int Alpha )
    {
      GR::u8*     pSrc = (GR::u8*)pSource;
      GR::u8*     pTgt = (GR::u8*)pDest;

      // Rot
      *(GR::u8*)pTgt = (GR::u8)( ( ( *(GR::u8*)pSrc++ * Alpha ) + *(GR::u8*)pTgt * ( 255 - Alpha ) ) / 255 );
      ++pTgt;
      // Grün
      *(GR::u8*)pTgt = (GR::u8)( ( ( *(GR::u8*)pSrc++ * Alpha ) + *(GR::u8*)pTgt * ( 255 - Alpha ) ) / 255 );
      ++pTgt;
      // Blau
      *(GR::u8*)pTgt = (GR::u8)( ( ( *(GR::u8*)pSrc++ * Alpha ) + *(GR::u8*)pTgt * ( 255 - Alpha ) ) / 255 );
      ++pTgt;
    }



    void ContextDescriptor::CopyPixelAlpha( void* pDest, void* pSource, int Alpha )
    {
      (this->*CopyPixelAlphaProc)( pDest, pSource, Alpha );
    }



    void ContextDescriptor::PutPixelAlpha( int X, int Y, GR::u32 Color, int Alpha )
    {
      if ( m_pData == NULL )
      {
        return;
      }
      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X < m_ClipLeft )
      ||   ( Y < m_ClipTop )
      ||   ( X > m_ClipRight )
      ||   ( Y > m_ClipBottom ) )
      {
        return;
      }

      if ( Alpha >= 255 )
      {
        PutPixelFast( X, Y, Color );
        return;
      }
      else if ( Alpha <= 0 )
      {
        return;
      }

      GR::u32   Pixel = GetPixel( X, Y );

      int ro = Pixel & 0xff0000;
      int go = Pixel & 0xff00;
      int bo = Pixel & 0xff;

      int r = Color & 0xff0000;
      int g = Color & 0xff00;
      int b = Color & 0xff;

      int rnew = ( ro + ( ( Alpha * ( r - ro ) ) >> 8 ) ) & 0xff0000;
      int gnew = ( go + ( ( Alpha * ( g - go ) ) >> 8 ) ) & 0xff00;
      int bnew = ( bo + ( ( Alpha * ( b - bo ) ) >> 8 ) ) & 0xff;

      PutPixel( X, Y, rnew + gnew + bnew );
    }



    void ContextDescriptor::_CopyPixelMask8( void* pDest, void* pSource, void* pMask )
    {
      if ( *(GR::u8*)pMask )
      {
        *(GR::u8*)pDest = *(GR::u8*)pSource;
      }
    }



    void ContextDescriptor::_CopyPixelMask555( void* pDest, void* pSource, void* pMask )
    {
      if ( *(GR::u8*)pMask )
      {
        *(GR::u16*)pDest = *(GR::u16*)pSource;
      }
    }



    void ContextDescriptor::_CopyPixelMask565( void* pDest, void* pSource, void* pMask )
    {
      if ( *(GR::u8*)pMask )
      {
        *(GR::u16*)pDest = *(GR::u16*)pSource;
      }
    }



    void ContextDescriptor::_CopyPixelMask24( void* pDest, void* pSource, void* pMask )
    {
      if ( *(GR::u8*)pMask )
      {
        *(GR::u8*)pDest = *(GR::u8*)pSource;
        *( (GR::u8*)pDest + 1 ) = *( (GR::u8*)pSource + 1 );
        *( (GR::u8*)pDest + 2 ) = *( (GR::u8*)pSource + 2 );
      }
    }



    void ContextDescriptor::_CopyPixelMask32( void* pDest, void* pSource, void* pMask )
    {
      if ( *(GR::u8*)pMask )
      {
        *(GR::u32*)pDest = *(GR::u32*)pSource;
      }
    }



    void ContextDescriptor::CopyPixelMask( void* pDest, void* pSource, void* pMask )
    {
      (this->*CopyPixelMaskProc)( pDest, pSource, pMask );
    }



    void ContextDescriptor::CopyAreaScaled( int SX, int SY, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      float   ScaleX = (float)ZWidth / (float)Width;
      float   ScaleY = (float)ZHeight / (float)Height;

      int     deltaUProZeile = (int)( 65536.0 / ScaleX ),

              deltaVProSpalte = (int)( 65536.0 / ScaleY );

      int     X1 = ZX,
              Y1 = ZY,
              X2 = ZX + ZWidth - 1,
              Y2 = ZY + ZHeight - 1;

      int   startingu = 0,
            startingv = 0;


      if ( X1 < pCDTarget->m_ClipLeft )
      {
        startingu += ( pCDTarget->m_ClipLeft - X1 ) * 65536;
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        startingv += ( pCDTarget->m_ClipTop - Y1 ) * deltaVProSpalte;
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      for( int y = Y1; y <= Y2; y++) 
      {
        u = startingu;
        //u = rowu;
        v = rowv;
        for( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            CopyPixel( pCDTarget->GetRowColumnData( x, y ),
                       GetRowColumnData( su, sv ) );
          }
          u += deltaUProZeile;
        }
        //rowu += deltaUProZeile;
        rowv += deltaVProSpalte;
      }
    }



    void ContextDescriptor::CopyAreaScaledTransparent( int SX, int SY, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, GR::u32 TransparentColor, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      TransparentColor = (GR::u32)MapColor( TransparentColor );

      float   ScaleX = (float)ZWidth / (float)Width;
      float   ScaleY = (float)ZHeight / (float)Height;

      int     deltaUProZeile = (int)( 65536.0 / ScaleX ),

              deltaVProSpalte = (int)( 65536.0 / ScaleY );

      int     X1 = ZX,
              Y1 = ZY,
              X2 = ZX + ZWidth - 1,
              Y2 = ZY + ZHeight - 1;

      int   startingu = 0,
            startingv = 0;


      if ( X1 < pCDTarget->m_ClipLeft )
      {
        startingu += ( pCDTarget->m_ClipLeft - X1 ) * deltaUProZeile;
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        startingv += ( pCDTarget->m_ClipTop - Y1 ) * deltaVProSpalte;
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      for ( int y = Y1; y <= Y2; y++) 
      {
        u = startingu;
        v = rowv;
        for ( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            CopyLineTransparent( su, sv, 1,
                                 x, y, 
                                 TransparentColor,
                                 pCDTarget );
          }
          u += deltaUProZeile;
        }
        rowv += deltaVProSpalte;
      }
    }




    void ContextDescriptor::_CopyAeraAlphaMaskWithColor555( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      GR::u8*       pMask;
      GR::u16*       pDest;

      Color = (GR::u32)pCDTarget->MapColor( Color );

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pMask = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha555( pDest, &Color, *pMask );

          ++pDest;
          ++pMask;
        }
      }
    }



    void ContextDescriptor::_CopyAeraAlphaMaskWithColor565( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      GR::u8*       pMask;
      GR::u16*       pDest;

      Color = (GR::u32)pCDTarget->MapColor( Color );

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u16*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pMask = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha565( pDest, &Color, *pMask );

          ++pDest;
          ++pMask;
        }
      }
    }



    void ContextDescriptor::_CopyAeraAlphaMaskWithColor24( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      GR::u8*       pMask;
      GR::u8*       pDest;

      Color = (GR::u32)pCDTarget->MapColor( Color );

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pMask = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha24( pDest, &Color, *pMask );

          pDest += 3;
          ++pMask;
        }
      }
    }



    void ContextDescriptor::_CopyAeraAlphaMaskWithColor32( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      GR::u8*       pMask;
      GR::u32*       pDest;

      Color = (GR::u32)pCDTarget->MapColor( Color );

      for ( int j = 0; j < Height; j++ )
      {
        pDest = (GR::u32*)pCDTarget->GetRowColumnData( ZX, ZY + j );
        pMask = (GR::u8*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          _CopyPixelAlpha32( pDest, &Color, *pMask );

          ++pDest;
          ++pMask;
        }
      }
    }



    void ContextDescriptor::CopyAlphaMaskWithColor( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      if ( pCDTarget == NULL )
      {
        return;
      }
      if ( m_BitDepth != 8 )
      {
        // BAUSTELLE Logging
        return;
      }
      if ( m_NothingVisible )
      {
        return;
      }

      int         xOffset = 0,
                  yOffset = 0;

      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }
      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      if ( pCDTarget->m_BitDepth == 15 )
      {
        _CopyAeraAlphaMaskWithColor555( X1, Y1, Width, Height, ZX, ZY, Color, pCDTarget );
      }
      else if ( pCDTarget->m_BitDepth == 16 )
      {
        _CopyAeraAlphaMaskWithColor565( X1, Y1, Width, Height, ZX, ZY, Color, pCDTarget );
      }
      else if ( pCDTarget->m_BitDepth == 24 )
      {
        _CopyAeraAlphaMaskWithColor24( X1, Y1, Width, Height, ZX, ZY, Color, pCDTarget );
      }
      else if ( pCDTarget->m_BitDepth == 32 )
      {
        _CopyAeraAlphaMaskWithColor32( X1, Y1, Width, Height, ZX, ZY, Color, pCDTarget );
      }
    }



    void ContextDescriptor::SetClipRect( int X1, int Y1, int X2, int Y2 )
    {
      if ( ( X1 < 0 )
      ||   ( X1 >= m_Width )
      ||   ( X1 > X2 ) )
      {
        m_ClipLeft = 0;
      }
      else
      {
        m_ClipLeft = X1;
      }
      if ( ( Y1 < 0 )
      ||   ( Y1 >= m_Height )
      ||   ( Y1 > Y2 ) )
      {
        m_ClipTop = 0;
      }
      else
      {
        m_ClipTop = Y1;
      }
      if ( ( X2 < 0 )
      ||   ( X2 >= m_Width ) )
      {
        m_ClipRight = m_Width - 1;
      }
      else
      {
        m_ClipRight = X2;
      }
      if ( ( Y2 < 0 )
      ||   ( Y2 >= m_Height ) )
      {
        m_ClipBottom = m_Height - 1;
      }
      else
      {
        m_ClipBottom = Y2;
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMaskScaled555( int X, int Y, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( pCDMask == NULL )
      {
        return;
      }
      if ( pCDMask->m_BitDepth != 8 )
      {
        return;
      }
      if ( ( pCDMask->m_Width != m_Width )
      ||   ( pCDMask->m_Height != m_Height ) )
      {
        return;
      }
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }

      if ( m_NothingVisible )
      {
        return;
      }

      float   ScaleX = (float)ZWidth / (float)Width;
      float   ScaleY = (float)ZHeight / (float)Height;

      int     deltaUProZeile = (int)( 65536.0 / ScaleX ),

              deltaVProSpalte = (int)( 65536.0 / ScaleY );

      int     X1 = ZX,
              Y1 = ZY,
              X2 = ZX + ZWidth - 1,
              Y2 = ZY + ZHeight - 1;

      int   startingu = 0,
            startingv = 0;


      if ( X1 < pCDTarget->m_ClipLeft )
      {
        startingu += ( pCDTarget->m_ClipLeft - X1 ) * 65536;//deltaUProZeile;
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        startingv += ( pCDTarget->m_ClipTop - Y1 ) * deltaVProSpalte;
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      GR::u16        *pP1,
                  *pDest;

      GR::u8        *pAlpha;

      int         maskAlpha = 0;

      GR::u32       Dummy,
                  Dummy2;

      for( int y = Y1; y <= Y2; y++) 
      {
        u = startingu;
        v = rowv;
        for( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            pAlpha = (GR::u8*)pCDMask->GetRowColumnData( su, sv );
            pDest = (GR::u16*)pCDTarget->GetRowColumnData( x, y );
            pP1 = (GR::u16*)GetRowColumnData( su, sv );

            maskAlpha = *pAlpha++;

            maskAlpha *= Alpha;
            maskAlpha /= 255;

            if ( maskAlpha == 0 )
            {
            }
            else
            {
              Dummy = ( (GR::u32)( *pP1 & 0x7c00 ) << 10 ) + ( (GR::u32)( *pP1 & 0x3e0 ) << 5 ) + (GR::u32)( *pP1 & 0x1f );
              Dummy2 = ( (GR::u32)( *pDest & 0x7c00 ) << 10 ) + ( (GR::u32)( *pDest & 0x3e0 ) << 5 ) + (GR::u32)( *pDest & 0x1f );

              Dummy = ( Dummy * maskAlpha ) + Dummy2 * ( 32 - maskAlpha );

              *pDest = (GR::u16)( ( ( Dummy & 0x3e000000 ) >> 15 ) + ( ( Dummy & 0xf8000 ) >> 10 ) + ( ( Dummy & 0x003e0 ) >> 5 ) );
            }
          }
          u += deltaUProZeile;
        }
        rowv += deltaVProSpalte;
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMaskScaled24( int X, int Y, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( pCDMask == NULL )
      {
        return;
      }
      if ( pCDMask->m_BitDepth != 8 )
      {
        return;
      }
      if ( ( pCDMask->m_Width != m_Width )
      ||   ( pCDMask->m_Height != m_Height ) )
      {
        return;
      }
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      float   ScaleX = (float)ZWidth / (float)Width;
      float   ScaleY = (float)ZHeight / (float)Height;

      int     deltaUProZeile = (int)( 65536.0 / ScaleX ),

              deltaVProSpalte = (int)( 65536.0 / ScaleY );

      int     X1 = ZX,
              Y1 = ZY,
              X2 = ZX + ZWidth - 1,
              Y2 = ZY + ZHeight - 1;

      int   startingu = 0,
            startingv = 0;


      if ( X1 < pCDTarget->m_ClipLeft )
      {
        startingu += ( pCDTarget->m_ClipLeft - X1 ) * 65536;//deltaUProZeile;
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        startingv += ( pCDTarget->m_ClipTop - Y1 ) * deltaVProSpalte;
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      GR::u8        *pP1,
                  *pDest;

      GR::u8        *pAlpha;

      int         maskAlpha = 0;

      GR::u32       dummy,
                  dummy2;

      int         iLoops;

      for ( int y = Y1; y <= Y2; y++) 
      {
        u = startingu;
        v = rowv;
        for ( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            pAlpha = (GR::u8*)pCDMask->GetRowColumnData( su, sv );
            pDest = (GR::u8*)pCDTarget->GetRowColumnData( x, y );
            pP1 = (GR::u8*)GetRowColumnData( su, sv );

            maskAlpha = *pAlpha;

            maskAlpha *= Alpha;
            maskAlpha /= 255;

            if ( maskAlpha == 0 )
            {
            }
            else
            {
              iLoops = 3;
              while ( iLoops-- )
              {
                dummy = *pP1++;
                dummy2 = *pDest;

                dummy = ( dummy * maskAlpha ) + dummy2 * ( 32 - maskAlpha );

                *pDest++ = (GR::u8)( dummy >> 5 );
              }
            }
          }
          u += deltaUProZeile;
        }
        rowv += deltaVProSpalte;
      }
    }



    void ContextDescriptor::_CopyAreaAlphaAndMaskScaled32( int X, int Y, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( pCDMask == NULL )
      {
        return;
      }
      if ( pCDMask->m_BitDepth != 8 )
      {
        return;
      }
      if ( ( pCDMask->m_Width != m_Width )
      ||   ( pCDMask->m_Height != m_Height ) )
      {
        return;
      }
      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }
      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          return;
        }
      }
      if ( m_NothingVisible )
      {
        return;
      }

      float   ScaleX = (float)ZWidth / (float)Width;
      float   ScaleY = (float)ZHeight / (float)Height;

      int     deltaUProZeile = (int)( 65536.0 / ScaleX ),

              deltaVProSpalte = (int)( 65536.0 / ScaleY );

      int     X1 = ZX,
              Y1 = ZY,
              X2 = ZX + ZWidth - 1,
              Y2 = ZY + ZHeight - 1;

      int   startingu = 0,
            startingv = 0;


      if ( X1 < pCDTarget->m_ClipLeft )
      {
        startingu += ( pCDTarget->m_ClipLeft - X1 ) * 65536;//deltaUProZeile;
        X1 = pCDTarget->m_ClipLeft;
      }
      if ( Y1 < pCDTarget->m_ClipTop )
      {
        startingv += ( pCDTarget->m_ClipTop - Y1 ) * deltaVProSpalte;
        Y1 = pCDTarget->m_ClipTop;
      }
      if ( X2 > pCDTarget->m_ClipRight )
      {
        X2 = pCDTarget->m_ClipRight;
      }
      if ( Y2 > pCDTarget->m_ClipBottom )
      {
        Y2 = pCDTarget->m_ClipBottom;
      }

      // für Debug-Zwecke
      //pCDTarget->Rectangle( X1, Y1, X2, Y2, 0xffffff );


      int     rowu = startingu;
      int     rowv = startingv;

      int     u,v,
              su,sv;

      GR::u8* pP1;
      GR::u8* pDest;

      GR::u8* pAlpha;

      int     maskAlpha = 0;

      GR::u32 dummy,
              dummy2;

      int     loops;

      for ( int y = Y1; y <= Y2; y++) 
      {
        u = startingu;
        v = rowv;
        for ( int x = X1; x <= X2; x++ )
        {
          su = u >> 16;
          sv = v >> 16;
          if ( ( su >= 0 )
          &&   ( sv >= 0 )
          &&   ( su < m_Width )
          &&   ( sv < m_Height ) )
          {
            pAlpha = (GR::u8*)pCDMask->GetRowColumnData( su, sv );
            pDest = (GR::u8*)pCDTarget->GetRowColumnData( x, y );
            pP1 = (GR::u8*)GetRowColumnData( su, sv );

            maskAlpha = *pAlpha;

            maskAlpha *= Alpha;
            maskAlpha /= 255;

            if ( maskAlpha > 0 )
            {
              loops = 3;
              while ( loops-- )
              {
                dummy = *pP1++;
                dummy2 = *pDest;

                dummy = ( dummy * maskAlpha ) + dummy2 * ( 32 - maskAlpha );

                *pDest++ = (GR::u8)( dummy >> 5 );
              }
            }
          }
          u += deltaUProZeile;
        }
        rowv += deltaVProSpalte;
      }
    }



    void ContextDescriptor::CopyAreaAlphaAndMaskScaled( int X, int Y, int Width, int Height, int ZX, int ZY, int ZWidth, int ZHeight, int Alpha, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      (this->*CopyAreaAlphaAndMaskScaledProc)( X, Y, Width, Height, ZX, ZY, ZWidth, ZHeight, Alpha, pCDMask, pCDTarget );
    }



    GR::Graphic::Image* ContextDescriptor::GetImage( const int PosX, 
                                                     const int PosY,
                                                     const int Width,
                                                     const int Height )
    {
      if ( m_pData == NULL )
      {
        return NULL;
      }

      int             X1         = PosX,
                      X2         = PosX + Width - 1,
                      Y1         = PosY,
                      Y2         = PosY + Height - 1,
                      newWidth   = Width,
                      xOffset    = 0,
                      yOffset    = 0,
                      i;

      if ( X1 < 0 )
      {
        newWidth += X1;
        xOffset = -X1;
        X1 = 0;
      }
      if ( Y1 < 0 )
      {
        yOffset = -Y1;
        Y1 = 0;
      }

      if ( X2 >= m_Width )
      {
        X2 = m_Width - 1;
        newWidth = X2 - X1 + 1;
      }
      if ( Y2 >= m_Height )
      {
        Y2 = m_Height - 1;
      }

      GR::Graphic::Image* pImage = new GR::Graphic::Image( Width, Height, (GR::u8)m_BitDepth );

      GR::u8*     pSource = ( GR::u8* )m_pData;
      GR::u8*     pDestination = (GR::u8*)pImage->GetData();


      for ( i = Y1; i <= Y2; i++ )
      {
        memcpy( ( (GR::u8*)pImage->GetData() ) + xOffset * BytesProPixel() + ( i - Y1 + yOffset ) * BytesProPixel() * Width,
                ( (GR::u8*)m_pData ) + X1 * BytesProPixel() + i * m_Width * BytesProPixel(),
                BytesProPixel() * newWidth );
      }
      return pImage;
    }



    void ContextDescriptor::Attach( int Width, int Height, int LineOffsetInBytes, eImageFormat imageFormat, void* pData )
    {
      ImageData::Attach( Width, Height, LineOffsetInBytes, imageFormat, pData );

      m_BitDepth    = BitsProPixel();

      m_ClipLeft     = 0;
      m_ClipTop      = 0;
      m_ClipRight    = m_Width - 1;
      m_ClipBottom   = m_Height - 1;

      m_NothingVisible = false;

      AssignColorDepth();

      if ( LineOffsetInBytes )
      {
        m_LineOffsetInBytes = LineOffsetInBytes;
      }
    }



    bool ContextDescriptor::CreateData( int Width, int Height, eImageFormat imageFormat, int LineOffsetInBytes )
    {
      bool    bResult = ImageData::CreateData( Width, Height, imageFormat, LineOffsetInBytes );

      if ( bResult )
      {
        m_BitDepth    = BitsProPixel();
        m_ClipLeft     = 0;
        m_ClipTop      = 0;
        m_ClipRight    = m_Width - 1;
        m_ClipBottom   = m_Height - 1;
        m_NothingVisible = false;
        AssignColorDepth();
      }
      return bResult;
    }



    void ContextDescriptor::_AlphaBox16( int X1, int Y1, int Width, int Height, GR::u32 Color, int Alpha )
    {
      GR::u16*      pP1 = NULL;

      GR::u32       SourceColor = (GR::u32)MapColor( Color );

      for ( int j = 0; j < Height; j++ )
      {
        pP1   = (GR::u16*)GetRowColumnData( X1, Y1 + j );
        for ( int i = 0; i < Width; ++i )
        {
          CopyPixelAlpha( pP1, &SourceColor, Alpha );

          ++pP1;
        }
      }
    }



    void ContextDescriptor::AlphaBox( int X, int Y, int Width, int Height, GR::u32 Color, int Alpha )
    {
      if ( AlphaBoxProc )
      {
        int         xOffset = 0,
                    yOffset = 0;

        if ( m_NothingVisible )
        {
          return;
        }
        if ( ( X > m_ClipRight )
        ||   ( Y > m_ClipBottom )
        ||   ( X + Width <= m_ClipLeft )
        ||   ( Y + Height <= m_ClipTop ) )
        {
          // Source völlig außerhalb
          return;
        }
        if ( X < m_ClipLeft )
        {
          // links clippen
          xOffset = m_ClipLeft - X;
          Width -= xOffset;
          X = m_ClipLeft;
        }
        if ( Y < m_ClipTop )
        {
          // oben clippen
          yOffset = m_ClipTop - Y;
          Height -= yOffset;
          Y = m_ClipTop;
        }
        if ( X + Width - 1 > m_ClipRight )
        {
          // rechts clippen
          Width = m_ClipRight - X + 1;
        }
        if ( Y + Height - 1 > m_ClipBottom )
        {
          // unten clippen
          Height = m_ClipBottom - Y + 1;
        }
        (this->*AlphaBoxProc)( X, Y, Width, Height, Color, Alpha );
      }
    }



    ContextDescriptor* ContextDescriptor::ConvertTo( eImageFormat imageFormat, int LineOffsetInBytes, bool ColorKeying, GR::u32 TransparentColor,
                                                     int X1, int Y1, int Width, int Height )
    {
      ContextDescriptor*   pCDNew = new ContextDescriptor();

      if ( Width == 0 )
      {
        Width = m_Width;
      }
      if ( Height == 0 )
      {
        Height = m_Height;
      }

      if ( !pCDNew->CreateData( Width, Height, imageFormat, LineOffsetInBytes ) )
      {
        delete pCDNew;
        dh::Log( "ContextDescriptor::ConvertTo: CreateData failed" );
        return NULL;
      }

      if ( !ConvertInto( pCDNew, ColorKeying, TransparentColor, X1, Y1, Width, Height ) )
      {
        delete pCDNew;
        dh::Log( "ContextDescriptor::ConvertTo: ConvertInto failed" );
        return NULL;
      }
      return pCDNew;
    }



    ContextDescriptor* ContextDescriptor::ConvertTo( unsigned long Depth, int LineOffsetInBytes, bool ColorKeying, GR::u32 TransparentColor,
                                                     int X1, int Y1, int Width, int Height )
    {
      return ConvertTo( ImageFormatFromDepth( Depth ), LineOffsetInBytes, ColorKeying, TransparentColor, X1, Y1, Width, Height );
    }



    bool ContextDescriptor::ConvertSelfTo( eImageFormat imageFormat, int LineOffsetInBytes, bool ColorKeying, GR::u32 TransparentColor,
                                           int X1, int Y1, int Width, int Height )
    {
      if ( !ImageData::ConvertSelfTo( imageFormat, LineOffsetInBytes, ColorKeying, TransparentColor, X1, Y1, Width, Height ) )
      {
        return false;
      }

      m_BitDepth    = BitsProPixel();
      AssignColorDepth();

      //dh::Log( "Converted to %d (%x)", m_BitDepth, this );

      return true;
    }



    bool ContextDescriptor::ConvertSelfTo( unsigned long Depth, int LineOffsetInBytes, bool ColorKeying, GR::u32 TransparentColor,
                                           int X1, int Y1, int Width, int Height )
    {
      return ConvertSelfTo( ImageFormatFromDepth( Depth ), LineOffsetInBytes, ColorKeying, TransparentColor, X1, Y1, Width, Height );
    }



    void ContextDescriptor::CopyAreaCompressed( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::Graphic::RLEList& rleList, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( rleList.m_pData == NULL )
      {
        return;
      }

      if ( m_ImageFormat != pCDTarget->ImageFormat() )
      {
        if ( !ConvertSelfTo( pCDTarget->ImageFormat() ) )
        {
          dh::Log( "ContextDescriptor::CopyAreaCompressed Conversion from %d to %d failed",
                   m_ImageFormat, pCDTarget->ImageFormat() );
          return;
        }
      }

      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }

      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }

      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        xOffset += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        yOffset += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      GR::u8*   pDestinationBase  = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );
      GR::u8*   pSourceBase       = (GR::u8*)GetRowColumnData( X1, Y1 );

      GR::i32   step = BytesProPixel();

      GR::u8*   pDestination;
      GR::u8*   pSource;
      GR::u8*   pCompress    = (GR::u8*)rleList.m_pData;

      GR::u16         wSectors;

      signed long     offset,
                      delta,
                      size,
                      i;

      if ( pCompress != NULL )
      {
        int   iSectorType = 0;

        for ( i = 0; i < yOffset; i++ )
        {
          wSectors        = *(GR::u16*)pCompress;
          pCompress      += 2 + wSectors;
        }
        for ( i = 0; i < Height; i++ )
        {
          int   iXTemp = xOffset;
          pDestination    = pDestinationBase;
          pSource         = pSourceBase;
          offset          = xOffset;
          delta           = Width;
          wSectors        = *(GR::u16*)pCompress;
          pCompress      += 2;
          while ( wSectors )
          {
            size = (signed long)*pCompress & GR::Graphic::RLEList::RLE_MAX_COMPRESS_SECTOR;
            iSectorType = ( *pCompress & ~GR::Graphic::RLEList::RLE_MAX_COMPRESS_SECTOR );
            if ( size == 0 )
            {
              size = this->Width();
            }
            if ( offset )
            {
              if ( offset >= size )
              {
                // noch ausserhalb
                offset -= size;
                size    = 0;
              }
              else
              {
                // schon innerhalb
                size  -= offset;
                offset = 0;
              }
            }
            if ( size )
            {
              if ( delta )
              {
                if ( delta >= size )
                {
                  // noch komplett innerhalb
                  delta -= size;
                }
                else
                {
                  size = delta;
                  delta   = 0;
                }
                if ( iSectorType == GR::Graphic::RLEList::ST_OPAQUE )
                {
                  memcpy( pDestination, pSource, size * step );
                  iXTemp += size;
                }
                else if ( iSectorType == GR::Graphic::RLEList::ST_SMALL_ALPHA )
                {
                  if ( pCDMask == NULL )
                  {
                    memcpy( pDestination, pSource, size * step );
                    iXTemp += size;
                  }
                  else
                  {
                    int Steps = size;

                    GR::u8*   pDest = pDestination;
                    GR::u8*   pSrc = pSource;

                    while ( Steps > 0 )
                    {
                      --Steps;

                      CopyPixelAlpha( pDest, pSrc, pCDMask->GetDirectPixel( xOffset + iXTemp, yOffset + i ) );
                      pDest += step;
                      pSrc += step;
                      iXTemp++;
                    }
                  }
                }
                else
                {
                  iXTemp += size;
                }
              }
              pDestination  += size * step;
              pSource       += size * step;
            }
            pCompress++;
            wSectors--;
          }
          pDestinationBase += pCDTarget->LineOffsetInBytes();
          pSourceBase      += LineOffsetInBytes();
        }
      }
    }



    void ContextDescriptor::CopyAreaColorized( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::u32 Color, ContextDescriptor* pCDTarget )
    {
      if ( Color == 0xffffffff )
      {
        CopyArea( X1, Y1, Width, Height, ZX, ZY, pCDTarget );
        return;
      }

      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }

      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }

      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        xOffset += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        yOffset += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      for ( int y = 0; y < Height; ++y )
      {
        for ( int x = 0; x < Width; ++x )
        {
          pCDTarget->PutPixel( ZX + x, ZY + y, GFX::Modulate( GetPixel( X1 + x, Y1 + y ), Color ) );
        }
      }
    }



    void ContextDescriptor::CopyAreaCompressedColorize( int X1, int Y1, int Width, int Height, int ZX, int ZY, GR::Graphic::RLEList& rleList, GR::u32 Color, ContextDescriptor* pCDMask, ContextDescriptor* pCDTarget )
    {
      if ( Color == 0xffffffff )
      {
        CopyAreaCompressed( X1, Y1, Width, Height, ZX, ZY, rleList, pCDMask, pCDTarget );
        return;
      }

      if ( rleList.m_pData == NULL )
      {
        return;
      }
      int   xOffset = 0,
            yOffset = 0;

      if ( m_NothingVisible )
      {
        return;
      }
      if ( ( X1 > m_ClipRight )
      ||   ( Y1 > m_ClipBottom )
      ||   ( X1 + Width <= m_ClipLeft )
      ||   ( Y1 + Height <= m_ClipTop ) )
      {
        // Source völlig außerhalb
        return;
      }
      if ( X1 < m_ClipLeft )
      {
        // links clippen
        xOffset = m_ClipLeft - X1;
        Width -= xOffset;
        X1 = m_ClipLeft;
      }
      if ( Y1 < m_ClipTop )
      {
        // oben clippen
        yOffset = m_ClipTop - Y1;
        Height -= yOffset;
        Y1 = m_ClipTop;
      }
      if ( X1 + Width - 1 > m_ClipRight )
      {
        // rechts clippen
        Width = m_ClipRight - X1 + 1;
      }
      if ( Y1 + Height - 1 > m_ClipBottom )
      {
        // unten clippen
        Height = m_ClipBottom - Y1 + 1;
      }

      if ( pCDTarget == NULL )
      {
        pCDTarget = this;
      }

      ZX += xOffset;
      ZY += yOffset;
      if ( ( ZX > pCDTarget->m_ClipRight )
      ||   ( ZY > pCDTarget->m_ClipBottom )
      ||   ( ZX + Width <= pCDTarget->m_ClipLeft )
      ||   ( ZY + Height <= pCDTarget->m_ClipTop ) )
      {
        // Target völlig außerhalb
        return;
      }
      if ( ZX < pCDTarget->m_ClipLeft )
      {
        // links clippen
        Width -= pCDTarget->m_ClipLeft - ZX;
        X1 += pCDTarget->m_ClipLeft - ZX;
        xOffset += pCDTarget->m_ClipLeft - ZX;
        ZX = pCDTarget->m_ClipLeft;
      }
      if ( ZY < pCDTarget->m_ClipTop )
      {
        // oben clippen
        Height -= pCDTarget->m_ClipTop - ZY;
        Y1 += pCDTarget->m_ClipTop - ZY;
        yOffset += pCDTarget->m_ClipTop - ZY;
        ZY = pCDTarget->m_ClipTop;
      }
      if ( ZX + Width - 1 > pCDTarget->m_ClipRight )
      {
        // rechts clippen
        Width = pCDTarget->m_ClipRight - ZX + 1;
      }
      if ( ZY + Height - 1 > pCDTarget->m_ClipBottom )
      {
        // unten clippen
        Height = pCDTarget->m_ClipBottom - ZY + 1;
      }

      GR::u8*   pDestinationBase  = (GR::u8*)pCDTarget->GetRowColumnData( ZX, ZY );
      GR::u8*   pSourceBase       = (GR::u8*)GetRowColumnData( X1, Y1 );

      GR::i32   step = BytesProPixel();

      GR::u8*   pDestination;
      GR::u8*   pSource;
      GR::u8*   pCompress    = (GR::u8*)rleList.m_pData;

      GR::u16         wSectors;

      signed long     offset,
                      delta,
                      size,
                      i;

      if ( pCompress != NULL )
      {
        int SectorType = 0;

        for ( i = 0; i < yOffset; i++ )
        {
          wSectors        = *(GR::u16*)pCompress;
          pCompress      += 2 + wSectors;
        }
        for ( i = 0; i < Height; i++ )
        {
          int XPos = 0;

          pDestination    = pDestinationBase;
          pSource         = pSourceBase;
          offset          = xOffset;
          delta           = Width;
          wSectors        = *(GR::u16*)pCompress;
          pCompress      += 2;
          while ( wSectors )
          {
            size = (signed long)*pCompress & GR::Graphic::RLEList::RLE_MAX_COMPRESS_SECTOR;
            SectorType = ( *pCompress & ~GR::Graphic::RLEList::RLE_MAX_COMPRESS_SECTOR );
            if ( size == 0 )
            {
              size = this->Width();
            }
            if ( offset )
            {
              if ( offset >= size )
              {
                // noch ausserhalb
                offset -= size;
                size = 0;
              }
              else
              {
                // schon innerhalb
                size -= offset;
                offset = 0;
              }
            }
            if ( size )
            {
              if ( delta )
              {
                if ( delta >= size )
                {
                  // noch komplett innerhalb
                  delta -= size;
                }
                else
                {
                  size = delta;
                  delta = 0;
                }
                if ( SectorType != GR::Graphic::RLEList::ST_TRANSPARENT )
                {
                  // Arschlahm, es lebe hoch!
                  for ( int TempX = 0; TempX < size; ++TempX )
                  {
                    int   Alpha = 255;

                    if ( pCDMask )
                    {
                      Alpha = pCDMask->GetDirectPixel( xOffset + XPos + TempX, yOffset + i );
                    }

                    pCDTarget->PutPixelAlpha( ZX + XPos + TempX,
                                              ZY + i, 
                                              GFX::Modulate( Color, GetPixel( X1 + XPos + TempX, Y1 + i ) ),
                                              Alpha );
                  }
                }
              }
              XPos += size;
              pDestination  += size * step;
              pSource       += size * step;
            }
            pCompress++;
            wSectors--;
          }
          pDestinationBase += pCDTarget->LineOffsetInBytes();
          pSourceBase      += LineOffsetInBytes();
        }
      }
    }



    void ContextDescriptor::Clear()
    {
      *this = ContextDescriptor();
    }



    bool ContextDescriptor::ReplaceColor( GR::u32 OldColor, GR::u32 NewColor )
    {
      bool      ColorFound = false;

      for ( int i = 0; i < m_Width; ++i )
      {
        for ( int j = 0; j < m_Height; ++j )
        {
          if ( GetPixelRGB( i, j ) == OldColor )
          {
            PutPixel( i, j, NewColor );
            ColorFound = true;
          }
        }
      }
      return ColorFound;
    }

  }

}