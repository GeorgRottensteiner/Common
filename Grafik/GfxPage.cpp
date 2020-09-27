#include "Palette.h"
#include "GfxPage.h"

#include <Misc/Misc.h>

#include <debug/debugclient.h>



namespace GR
{
  namespace Graphic
  {

    GFXPage::GFXPage()
    {
      ulType          = 0;
      ucBitsPerPixel  = 0;
      ulWidth         = 0;
      ulHeight        = 0;
      ulLineOffset    = 0;
      ulBorderLeft    = 0;
      ulBorderTop     = 0;
      ulBorderRight   = 0;
      ulBorderBottom  = 0;
      m_iXOffset      = 0;
      m_iYOffset      = 0;
      
      pData           = NULL;
    }



    GFXPage::~GFXPage()
    {
      Destroy();
    }



    bool GFXPage::Create( unsigned long ulNewWidth, unsigned long ulNewHeight, unsigned char ucBpp )
    {
      ucBitsPerPixel    = ucBpp;
      ulWidth           = ulNewWidth;
      ulHeight          = ulNewHeight;
      switch ( ucBpp )
      {
        case 1:
          ulLineOffset  = ulWidth / 8;
          if ( ulWidth % 8 )
          {
            ulLineOffset++;
          }
          break;
        case 2:
          ulLineOffset = ulWidth / 4;
          if ( ulWidth % 4 )
          {
            ulLineOffset++;
          }
          break;
        case 4:
          ulLineOffset  = ulWidth / 2;
          if ( ulWidth & 1 )
          {
            ulLineOffset++;
          }
          break;
        case 8:
          ulLineOffset  = ulWidth;
          break;
        case 15:
        case 16:
          ulLineOffset  = ulWidth * 2;
          break;
        case 24:
          ulLineOffset  = ulWidth * 3;
          break;
        case 32:
          ulLineOffset  = ulWidth * 4;
          break;
      }

      // auf 32bit padden
      while ( ulLineOffset % 4 )
      {
        ulLineOffset++;
      }
      ulBorderLeft      = 0;
      ulBorderTop       = 0;
      ulBorderRight     = ulWidth - 1;
      ulBorderBottom    = ulHeight - 1;

      return true;
    }



    bool GFXPage::Destroy()
    {
      ulType          = 0;
      ucBitsPerPixel  = 0;
      ulWidth         = 0;
      ulHeight        = 0;
      ulLineOffset    = 0;
      ulBorderLeft    = 0;
      ulBorderTop     = 0;
      ulBorderRight   = 0;
      ulBorderBottom  = 0;
      pData           = NULL;// Wenn pData != NULL was passiert dann mit dem Speicherbereich?

      return true;
    }



    void GFXPage::PutPixel( signed long slXPos, signed long slYPos, unsigned long ulColor, bool bIgnoreOffset )
    {
      if ( !bIgnoreOffset )
      {
        slXPos += m_iXOffset;
        slYPos += m_iYOffset;
      }

      if ( ( slXPos >= (signed long)ulBorderLeft )
      &&   ( slYPos >= (signed long)ulBorderTop )
      &&   ( slXPos <= (signed long)ulBorderRight )
      &&   ( slYPos <= (signed long)ulBorderBottom ) )
      {
        switch ( ucBitsPerPixel )
        {
          case 1:
            {
              GR::u8*   pTempData = (( GR::u8*)pData ) + ( slXPos / 8 ) + slYPos * ulLineOffset;

              GR::u8    ucMask = ( GR::u8 )( 1 << ( 7 - slXPos % 8 ) );

              if ( ulColor )
              {
                *pTempData |= ucMask;
              }
              else
              {
                *pTempData &= ~ucMask;
              }
            }
            break;
          case 4:
            {
              GR::u8*   pTempData = (( GR::u8*)pData ) + ( slXPos >> 1 ) + slYPos * ulLineOffset;
              if ( slXPos & 1 )
              {
                *pTempData &= 0xf0;
                *pTempData |= ulColor;
              }
              else
              {
                *pTempData &= 0x0f;
                *pTempData |= ( ulColor << 4 );
              }
            }
            break;
          case 8:
            *( ( GR::u8 *)pData + slXPos + slYPos * ulLineOffset ) = ( GR::u8 )ulColor;
            break;
          case 15:
          case 16:
            *(GR::u16 *)( ( GR::u8 *)pData + slXPos * 2 + slYPos * ulLineOffset ) = ( GR::u16 )ulColor;
            break;
          case 24:
            *( GR::u16 *)( ( GR::u8 *)pData + slXPos * 3 + slYPos * ulLineOffset ) = ( GR::u16 )ulColor;
            *( GR::u8 *)( ( GR::u8 *)pData + slXPos * 3 + slYPos * ulLineOffset + 2 ) = ( GR::u8 )( ulColor >> 16 );
            break;
          case 32:
            *(unsigned long *)( ( GR::u8 *)pData + slXPos * 4 + slYPos * ulLineOffset ) = ulColor;
            break;
          default:
            dh::Log( "GFXPage::PutPixel unsupported depth %d\n", ucBitsPerPixel );
            break;
        }
      }

    }



    unsigned long GFXPage::GetPixel( signed long slXPos, signed long slYPos )
    {

      slXPos += m_iXOffset;
      slYPos += m_iYOffset;

      if ( ( slXPos >= 0 )
      &&   ( slYPos >= 0 )
      &&   ( slXPos < (signed long)ulWidth )
      &&   ( slYPos < (signed long)ulHeight ) )
      {
        switch ( ucBitsPerPixel )
        {
          case 1:
            {
              GR::u8*   pTempData = (( GR::u8*)pData ) + ( slXPos / 8 ) + slYPos * ulLineOffset;

              GR::u8    ucMask = ( GR::u8 )( 1 << ( 7 - slXPos % 8 ) );

              if ( *pTempData & ucMask )
              {
                return 1;
              }
              return 0;
            }
            break;
          case 4:
            {
              GR::u8*   pTempData = (( GR::u8*)pData ) + ( slXPos >> 1 ) + slYPos * ulLineOffset;
              if ( slXPos & 1 )
              {
                return ( *pTempData & 0x0f );
              }
              else
              {
                return ( ( *pTempData & 0xf0 ) >> 4 );
              }
            }
            break;
          case 8:
            return *( ( GR::u8 *)pData + slXPos + slYPos * ulLineOffset );
          case 15:
          case 16:
            return *( GR::u16 *)( ( GR::u8 *)pData + slXPos * 2 + slYPos * ulLineOffset );
          case 24:
            return ( *(unsigned long *)( ( GR::u8 *)pData + slXPos * 3 + slYPos * ulLineOffset ) & 0x00ffffff );
          case 32:
            return *(unsigned long *)( ( GR::u8 *)pData + slXPos * 4 + slYPos * ulLineOffset );
          default:
            dh::Log( "GFXPage::GetPixel unsupported depth %d\n", ucBitsPerPixel );
            break;
        }
      }

      return 0;

    }



    void GFXPage::DarkenPixel( signed long slXPos, signed long slYPos, unsigned long ulFaktor )
    {

      slXPos += m_iXOffset;
      slYPos += m_iYOffset;

      unsigned long   ulColor;

      if ( ( ucBitsPerPixel == 15 )
      ||   ( ucBitsPerPixel == 16 ) )
      {
        ulColor = GetPixel( slXPos, slYPos );
        if ( ulFaktor > 64 )
        {
          ulFaktor = 64;
        }
        while ( ulFaktor-- )
        {
          if ( ulColor & COLOR_MASK_RED )
          {
            ulColor -= COLOR_MASK_RED;
          }
          if ( ulColor & COLOR_MASK_GREEN )
          {
            ulColor -= COLOR_MASK_GREEN;
          }
          if ( ulColor & COLOR_MASK_BLUE )
          {
            ulColor -= COLOR_MASK_BLUE;
          }
        }
        PutPixel( slXPos, slYPos, ulColor );
      }

    }



    void GFXPage::LightenPixel( signed long slXPos, signed long slYPos, unsigned long ulFaktor )
    {

      slXPos += m_iXOffset;
      slYPos += m_iYOffset;

      unsigned long   ulColor;

      if ( ( ucBitsPerPixel == 15 )
      ||   ( ucBitsPerPixel == 16 ) )
      {
        ulColor = GetPixel( slXPos, slYPos );
        if ( ulFaktor > 64 )
        {
          ulFaktor = 64;
        }
        while ( ulFaktor )
        {
          if ( ( ulColor & COLOR_MASK_RED ) != COLOR_MASK_RED )
          {
            ulColor += COLOR_MASK_RED;
          }
          if ( ( ulColor & COLOR_MASK_GREEN ) != COLOR_MASK_GREEN )
          {
            ulColor += COLOR_MASK_GREEN;
          }
          if ( ( ulColor & COLOR_MASK_BLUE ) != COLOR_MASK_BLUE )
          {
            ulColor += COLOR_MASK_BLUE;
          }
        }
        PutPixel( slXPos, slYPos, ulColor );
      }

    }



    void GFXPage::ShadowPixel( signed long slXPos, signed long slYPos )
    {
      slXPos += m_iXOffset;
      slYPos += m_iYOffset;

      if ( ( ucBitsPerPixel == 15 )
      ||   ( ucBitsPerPixel == 16 ) )
      {
        PutPixel( slXPos, slYPos, ( GetPixel( slXPos, slYPos ) >> 1 ) & 0x3def );
      }
    }



    void GFXPage::HLine( signed long slXStart, signed long slXEnd, signed long slYPos, unsigned long ulColor, bool bIgnoreOffset )
    {

      if ( !bIgnoreOffset )
      {
        slXStart += m_iXOffset;
        slXEnd   += m_iXOffset;
        slYPos   += m_iYOffset;
      }

      signed long     slTemp;

      if ( slXStart > slXEnd )
      {
        slTemp    = slXStart;
        slXStart  = slXEnd;
        slXEnd    = slTemp;
      }
      if ( ( slXEnd >= (signed long)ulBorderLeft )
      &&   ( slYPos >= (signed long)ulBorderTop )
      &&   ( slXStart <= (signed long)ulBorderRight )
      &&   ( slYPos <= (signed long)ulBorderBottom ) )
      {
        if ( slXStart < (signed long)ulBorderLeft )
        {
          slXStart = (signed long)ulBorderLeft;
        }
        if ( slXEnd > (signed long)ulBorderRight )
        {
          slXEnd = (signed long)ulBorderRight;
        }
        if ( slXStart == slXEnd )
        {
          PutPixel( slXStart, slYPos, ulColor );
        }
        else
        {
          switch ( ucBitsPerPixel )
          {
            case 1:
              {
                int   iStartByte = slXStart >> 3,
                      iEndByte = slXEnd >> 3,
                      iStartFract = slXStart - ( iStartByte << 3 ),
                      iEndFract = slXEnd - ( iEndByte << 3 );

                // alles innerhalb eines Bytes
                if ( iStartByte == iEndByte )
                {
                  for ( int i = slXStart; i <= slXEnd; ++i )
                  {
                    PutPixel( i, slYPos, ulColor );
                  }
                  break;
                }
                if ( iStartFract )
                {
                  // Anfangs und End-Byte nicht gleich
                  while ( iStartFract < 8 )
                  {
                    PutPixel( slXStart++, slYPos, ulColor );
                    ++iStartFract;
                  }
                }
                if ( iEndFract != 7 )
                {
                  // Anfangs und End-Byte nicht gleich
                  while ( iEndFract > -1 )
                  {
                    PutPixel( slXEnd--, slYPos, ulColor );
                    --iEndFract;
                  }
                }
                // jetzt sollten nur noch volle Bytes da sein
                if ( slXStart != slXEnd )
                {
                  CMisc::FillValue( ( GR::u8 *)pData + ( slXStart >> 3 ) + slYPos * ulLineOffset,
                                 ( slXEnd - slXStart + 1 ) >> 3,
                                 ( GR::u8 )( 255 * ( ulColor & 1 ) ) );
                }
              }
              break;
            case 4:
              if ( slXStart & 1 )
              {
                PutPixel( slXStart, slYPos, ulColor );
                slXStart++;
              }
              if ( ( slXEnd > slXStart )
              &&   ( !( slXEnd & 1 ) ) )
              {
                PutPixel( slXEnd, slYPos, ulColor );
                slXEnd--;
              }
              if ( slXStart < slXEnd )
              {
                CMisc::FillValue( ( GR::u8 *)pData + ( slXStart >> 1 ) + slYPos * ulLineOffset, ( slXEnd - slXStart + 1 ) / 2, (GR::u8)( ulColor + ( ulColor << 4 ) ) );
              }
              break;
            case 8:
              CMisc::FillValue( ( GR::u8 *)pData + slXStart + slYPos * ulLineOffset, slXEnd - slXStart + 1, ( GR::u8 )ulColor );
              break;
            case 15:
            case 16:
              CMisc::FillValue16( ( GR::u16 *)( ( GR::u8 *)pData + slXStart * 2 + slYPos * ulLineOffset ), ( slXEnd - slXStart + 1 ) * 2, ( GR::u16 )ulColor );
              break;
            case 24:
              CMisc::FillValue24( (GR::u32*)( ( GR::u8 *)pData + slXStart * 3 + slYPos * ulLineOffset ), ( slXEnd - slXStart + 1 ) * 3, ulColor );
              break;
            case 32:
              CMisc::FillValue32( (GR::u32*)( ( GR::u8 *)pData + slXStart * 4 + slYPos * ulLineOffset ), ( slXEnd - slXStart + 1 ) * 4, ulColor );
              break;
            default:
              dh::Log( "GFXPage::HLine unsupported depth %d\n", ucBitsPerPixel );
              break;
          }
        }
      }

    }



    void GFXPage::HLineAlpha( signed long slXStart, signed long slXEnd, signed long slYPos, unsigned long ulColor, unsigned char ucAlpha )
    {

      // Baustelle
      slXStart  += m_iXOffset;
      slXEnd    += m_iXOffset;
      slYPos    += m_iYOffset;

    }



    void GFXPage::HLineAlpha50( signed long slXStart, signed long slXEnd, signed long slYPos, unsigned long ulColor )
    {

      // Baustelle
      slXStart  += m_iXOffset;
      slXEnd    += m_iXOffset;
      slYPos    += m_iYOffset;

    }



    void GFXPage::SetPalette( GR::Graphic::Palette* pPalette )
    {
    }



    void GFXPage::GetPalette( GR::Graphic::Palette *pPalette )
    {
    }



    GR::u8 GFXPage::ComputeClipCode( int iX, int iY )
    {
      GR::u8    bCode = 0;

      if ( iY > (int)ulBorderBottom )
      {
        bCode = 1;
      }
      else if ( iY < (int)ulBorderTop )
      {
        bCode = 2;
      }
      if ( iX > (int)ulBorderRight )
      {
        bCode |= 8;
      }
      else if ( iX < (int)ulBorderLeft )
      {
        bCode |= 4;
      }
      return bCode;
    }



    void GFXPage::ClippedLine( int iX1, int iY1, int iX2, int iY2, GR::u32 Color, bool bIgnoreOffset )
    {
      int   CodeBottom = 1,
            CodeTop    = 2,
            CodeLeft   = 4,
            CodeRight  = 8;

      GR::u8  OutCode0 = 0,
            OutCode1 = 0,
            OutCodeOut;
      int   iX = 0,
            iY = 0; 

  
      int iClipLeft    = ulBorderLeft  ;
      int iClipRight   = ulBorderRight ;
      int iClipTop     = ulBorderTop   ;
      int iClipBottom  = ulBorderBottom;

      //- ignore ignore offset!!!
      if ( !bIgnoreOffset )
      {
        iX1           += m_iXOffset;
        iX2           += m_iXOffset;
        iY1           += m_iYOffset;
        iY2           += m_iYOffset;
      }
  
      OutCode0 = ComputeClipCode( iX1, iY1 );
      OutCode1 = ComputeClipCode( iX2, iY2 );

      while ( ( OutCode0 != 0 )
      ||      ( OutCode1 != 0 ) )
      {
        if ( OutCode0 & OutCode1 )
        {
          // komplett außerhalb bei einer Seite
          return;
        }
    
        if ( OutCode0 > 0 )
        {
          OutCodeOut = OutCode0;
        }
        else
        {
          OutCodeOut = OutCode1;
        } 

        if ( ( OutCodeOut & CodeBottom ) == CodeBottom )
        {
          // Clip the line to the bottom of the viewport
          iY = iClipBottom;
          iX = iX1 + ( iX2 - iX1 ) * ( iY - iY1 ) / ( iY2 - iY1 );
        }
        else if ( ( OutCodeOut & CodeTop ) == CodeTop )
        {
          // Clip the line to the top of the viewport
          iY = iClipTop;
          iX = iX1 + ( iX2 - iX1 ) * ( iY - iY1 ) / ( iY2 - iY1 );
        }
        else if ( ( OutCodeOut & CodeRight ) == CodeRight )
        {
          // Clip the line to the right edge of the viewport
          iX = iClipRight;
          iY = iY1 + ( iY2 - iY1 ) * ( iX - iX1 ) / ( iX2 - iX1 );
        }
        else if ( ( OutCodeOut & CodeLeft ) == CodeLeft )
        {
          // Clip the line to the left edge of the viewport
          iX = iClipLeft;
          iY = iY1 + ( iY2 - iY1 ) * ( iX - iX1 ) / ( iX2 - iX1 );
        }

        if ( OutCodeOut == OutCode0 )
        {
          // Modify the first coordinate
          iX1 = iX; 
          iY1 = iY;
          OutCode0 = ComputeClipCode( iX1, iY1 );
        }
        else
        {
          iX2 = iX;
          iY2 = iY;
          OutCode1 = ComputeClipCode( iX2, iY2 );
        }
      }

      Line( iX1, iY1, iX2, iY2, Color, true );
    }



    void GFXPage::Line( signed long slXStart, signed long slYStart, signed long slXEnd,
                            signed long slYEnd, unsigned long ulColor, bool bIgnoreOffset )
    {

      if ( !bIgnoreOffset )
      {
        slXStart  += m_iXOffset;
        slXEnd    += m_iXOffset;
        slYStart  += m_iYOffset;
        slYEnd    += m_iYOffset;
      }

      int dy = slYEnd - slYStart;
      int dx = slXEnd - slXStart;
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

      PutPixel( slXStart, slYStart, ulColor, true );
      if ( dx > dy ) 
      {
        int fraction = dy - ( dx >> 1 );

        while ( slXStart != slXEnd ) 
        {
          if ( fraction >= 0 ) 
          {
            slYStart += stepy;
            fraction -= dx;
          }
          slXStart += stepx;
          fraction += dy;
          PutPixel( slXStart, slYStart, ulColor, true );
        }
      } 
      else 
      {
        int fraction = dx - ( dy >> 1 );

        while ( slYStart != slYEnd ) 
        {
          if ( fraction >= 0 ) 
          {
            slXStart += stepx;
            fraction -= dy;
          }
          slYStart += stepy;
          fraction += dx;
          PutPixel( slXStart, slYStart, ulColor, true );
        }
      }

      /*
      signed int      slXStep   = 1,
                      slYStep   = 1,
                      slYDelta  = slYEnd - slYStart,
                      slXDelta  = slXEnd - slXStart,
                      slMark    = 0,
                      i;

      if ( ( slXStart == slXEnd )
      &&   ( slYStart == slYEnd ) )
      {
        // Linie hat nur einen Punkt
        PutPixel( slXStart, slYStart, ulColor, TRUE );
        return;
      }
      if ( slYStart == slYEnd )
      {
        // Linie ist horizontal
        if ( slXEnd < slXStart )
        {
          HLine( slXEnd, slXStart, slYStart, ulColor, TRUE );
        }
        else
        {
          HLine( slXStart, slXEnd, slYStart, ulColor, TRUE );
        }
        return;
      }
      if ( slXStart == slXEnd )
      {
        // Linie ist vertikal
        if ( slYEnd < slYStart )
        {
          for ( i = slYEnd; i <= slYStart; i++ )
          {
            PutPixel( slXStart, i, ulColor, TRUE );
          }
        }
        else
        {
          for ( i = slYStart; i <= slYEnd; i++ )
          {
            PutPixel( slXStart, i, ulColor, TRUE );
          }
        }
        return;
      }
      if ( slXDelta < 0 )
      {
        slXDelta  = - slXDelta;
        slXStep   = - 1;
      }
      if ( slYDelta < 0 )
      {
        slYDelta  = - slYDelta;
        slYStep   = - 1;
      }
      if ( slXDelta > slYDelta )
      {
        for ( i = 0; i <= slXDelta; i++ )
        {
          PutPixel( slXStart, slYStart, ulColor, TRUE );
          slXStart  += slXStep;
          slMark    += slYDelta;
          if ( slMark >= slXDelta )
          {
            slMark    -= slXDelta;
            slYStart  += slYStep;
		      }
	      }
      }
      else
      {
        for ( i = 0; i <= slYDelta; i++ )
        {
          PutPixel( slXStart, slYStart, ulColor, TRUE );
          slYStart  += slYStep;
          slMark    += slXDelta;
          if ( slMark >= slYDelta )
          {
            slMark    -= slYDelta;
            slXStart  += slXStep;
          }
        }
      }
      */

    }



    void GFXPage::Box( signed long slXStart, signed long slYStart, signed long slXEnd,
                           signed long slYEnd, unsigned long ulColor, bool bIgnoreOffset )
    {

      if ( !bIgnoreOffset )
      {
        slXStart  += m_iXOffset;
        slXEnd    += m_iXOffset;
        slYStart  += m_iYOffset;
        slYEnd    += m_iYOffset;
      }

      signed long     slLine,
                      slTemp;

      if ( slXStart > slXEnd )
      {
        slTemp    = slXStart;
        slXStart  = slXEnd;
        slXEnd    = slTemp;
      }
      if ( slYStart > slYEnd )
      {
        slTemp    = slYStart;
        slYStart  = slYEnd;
        slYEnd    = slTemp;
      }
      for ( slLine = slYStart; slLine <= slYEnd; slLine++ )
      {
        HLine( slXStart, slXEnd, slLine, ulColor, true );
      }

    }



    void GFXPage::Rectangle( signed long slXStart, signed long slYStart, signed long slXEnd,
                                 signed long slYEnd, unsigned long ulColor )
    {

      slXStart  += m_iXOffset;
      slXEnd    += m_iXOffset;
      slYStart  += m_iYOffset;
      slYEnd    += m_iYOffset;

      signed long     slTemp;

      if ( slXStart > slXEnd )
      {
        slTemp    = slXStart;
        slXStart  = slXEnd;
        slXEnd    = slTemp;
      }
      if ( slYStart > slYEnd )
      {
        slTemp    = slYStart;
        slYStart  = slYEnd;
        slYEnd    = slTemp;
      }
      HLine( slXStart, slXEnd, slYStart, ulColor, true );
      HLine( slXStart, slXEnd, slYEnd, ulColor, true );
      Line( slXStart, slYStart, slXStart, slYEnd, ulColor, true );
      Line( slXEnd, slYStart, slXEnd, slYEnd, ulColor, true );
    }



    void GFXPage::Ellipse( int x, int y, int a, int b, GR::u32 ucColor )
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


    void GFXPage::LargeEllipse( int x, int y, int a, int b, GR::u32 ucColor )
    {

      GR::i64     z,
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
        PutPixel( (long)e, (long)g, ucColor );
        PutPixel( (long)e, (long)h, ucColor );
        PutPixel( (long)f, (long)g, ucColor );
        PutPixel( (long)f, (long)h, ucColor );
      }

    }



    void GFXPage::FilledEllipse( int x, int y, int a, int b, GR::u32 ucColor )
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


      Line( x - a, y, x + a, y, ucColor );
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
        Line( e, g, f, g, ucColor );
        Line( e, h, f, h, ucColor );
      }

    }




    unsigned long GFXPage::GetType()
    {

      return ulType;

    }



    int GFXPage::GetWidth()
    {

      return ulWidth;

    }



    int GFXPage::GetHeight()
    {

      return ulHeight;

    }



    unsigned char GFXPage::GetDepth()
    {

      return ucBitsPerPixel;

    }



    unsigned long GFXPage::GetLineOffset()
    {

      return ulLineOffset;

    }



    unsigned long GFXPage::GetLeftBorder()
    {

      return ulBorderLeft;

    }



    unsigned long GFXPage::GetRightBorder()
    {

      return ulBorderRight;

    }



    unsigned long GFXPage::GetTopBorder()
    {

      return ulBorderTop;

    }



    unsigned long GFXPage::GetBottomBorder()
    {

      return ulBorderBottom;

    }



    void *GFXPage::GetData()
    {

      return pData;

    }



    /*
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    HDC GFXPage::GetDC()
    {
      return hDC;
    }



    void GFXPage::ReleaseDC()
    {
    }
#endif


    HWND GFXPage::GetWindowObject()
    {

      return hWindow;

    }



#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    HPALETTE GFXPage::GetPaletteObject()
    {
      return hPalette;
    }



    HBITMAP GFXPage::GetBitmapObject()
    {
      return hBitmap;
    }
#endif
    */



    unsigned long GFXPage::GetRGB( unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha )
    {

      switch ( ucBitsPerPixel )
      {
        case 8:
          // Baustelle
          break;
        case 15:
          return ( (unsigned long)( ucRed & 0x1f ) << 10 ) + ( (unsigned long)( ucGreen & 0x1f ) << 5 ) + ( ucBlue & 0x1f );
        case 16:
          return ( (unsigned long)( ucRed & 0x1f ) << 11 ) + ( (unsigned long)( ucGreen & 0x3f ) << 5 ) + ( ucBlue & 0x1f );
        case 24:
          return ( (unsigned long)ucRed << 16 ) + ( (unsigned long)ucGreen << 8 ) + ucBlue;
        case 32:
          return ( (unsigned long)ucAlpha << 24 ) + ( (unsigned long)ucRed << 16 ) + ( (unsigned long)ucGreen << 8 ) + ucBlue;
      }

      return 0;

    }



    unsigned long GFXPage::GetRGB256( unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha )
    {

      switch ( ucBitsPerPixel )
      {
        case 8:
          // Baustelle
          break;
        case 15:
          return ( (unsigned long)( ucRed >> 3 ) << 10 ) + ( (unsigned long)( ucGreen >> 3 ) << 5 ) + ( ucBlue >> 3 );
        case 16:
          return ( (unsigned long)( ucRed >> 3 ) << 11 ) + ( (unsigned long)( ucGreen >> 2 ) << 6 ) + ( ucBlue >> 3 );
        case 24:
          return ( (unsigned long)ucRed << 16 ) + ( (unsigned long)ucGreen << 8 ) + ucBlue;
        case 32:
          return ( (unsigned long)ucAlpha << 24 ) + ( (unsigned long)ucRed << 16 ) + ( (unsigned long)ucGreen << 8 ) + ucBlue;
      }

      return 0;

    }



    unsigned long GFXPage::GetRGB256( GR::u32 Color )
    {
      switch ( ucBitsPerPixel )
      {
        case 8:
          // Baustelle
          break;
        case 15:
          return ( ( Color & 0xf80000 ) >> 9 ) + ( ( Color & 0xf800 ) >> 6 ) + ( ( Color & 0xf8 ) >> 3 );
        case 16:
          return ( ( Color & 0xf80000 ) >> 8 ) + ( ( Color & 0xfc00 ) >> 5 ) + ( ( Color & 0xf8 ) >> 3 );
        case 24:
          return Color;
        case 32:
          return Color;
      }
      return 0;
    }



    void GFXPage::SetType( unsigned long ulNewType )
    {

      ulType = ulNewType;
      if ( ulNewType == PAGE_TYPE_555 )
      {
        ucBitsPerPixel = 15;
      }

    }



    void GFXPage::SetLineOffset( unsigned long ulNewLineOffset )
    {

      ulLineOffset = ulNewLineOffset;

    }



    /*
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    void GFXPage::SetHDCObject( HDC hNewDC )
    {
      if ( hDC )
      {
        DeleteDC( hDC );
      }
      hDC = hNewDC;
    }



    void GFXPage::SetPaletteObject( HPALETTE hNewPalette )
    {
      if ( hPalette )
      {
        DeleteObject( hPalette );
      }
      hPalette = hNewPalette;
    }



    void GFXPage::SetBitmapObject( HBITMAP hNewBitmap )
    {
      if ( hBitmap )
      {
        DeleteObject( hBitmap );
      }
      hBitmap = hNewBitmap;
    }
#endif
*/



    void GFXPage::SetRange( int iLeft, int iTop, int iRight, int iBottom )
    {

      if ( ( iLeft == -1 )
      &&   ( iTop == -1 )
      &&   ( iRight == -1 )
      &&   ( iBottom == -1 ) )
      {
        iLeft = 0;
        iTop = 0;
        iRight = ulWidth - 1;
        iBottom = ulHeight - 1;
      }

      int         iTemp;


      if ( iLeft > iRight )
      {
        iTemp    = iLeft;
        iLeft    = iRight;
        iRight   = iTemp;
      }
      if ( iTop > iBottom )
      {
        iTemp    = iTop;
        iTop     = iBottom;
        iBottom  = iTemp;
      }
      if ( iLeft < 0 )
      {
        iLeft = 0;
      }
      if ( iLeft >= (int)GetWidth() )
      {
        iLeft = (int)GetWidth() - 1;
      }
      ulBorderLeft = iLeft;

      if ( iRight >= (int)GetWidth() )
      {
        iRight = (int)GetWidth() - 1;
      }
      if ( iRight < 0 )
      {
        iRight = 0;
      }
      ulBorderRight = iRight;

      if ( iTop < 0 )
      {
        iTop = 0;
      }
      if ( iTop >= (int)GetHeight() )
      {
        iTop = (int)GetHeight() - 1;
      }
      ulBorderTop = iTop;

      if ( iBottom >= (int)GetHeight() )
      {
        iBottom = (int)GetHeight() - 1;
      }
      if ( iBottom < 0 )
      {
        iBottom = 0;
      }
      ulBorderBottom = iBottom;

    }



    void GFXPage::SetOffset( int iNewX, int iNewY )
    {

      m_iXOffset = iNewX;
      m_iYOffset = iNewY;

    }

  }

}
