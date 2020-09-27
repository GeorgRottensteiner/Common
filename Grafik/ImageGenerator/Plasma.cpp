#include <math.h>

#include <Grafik/ImageGenerator/Plasma.h>
#include <Grafik/ImageData.h>
#include <Grafik/ContextDescriptor.h>



PlasmaGenerator::PlasmaGenerator() :
  m_X1( 0 ),
  m_Y1( 0 ),
  m_X2( 0 ),
  m_Y2( 0 ),
  m_Speed( 1 )
{

  for( int i = 0; i < 64; i++ )
  {
    int k = ( i * 256 ) / 64;

    m_VirtualPalette[i] = ( k << 16 ) + ( 0 << 8 ) + ( 0 );

    m_VirtualPalette[i + 64] = ( -1 << 16 ) + ( k << 8 ) + ( 0 );

    m_VirtualPalette[i + 128] = ( -1 << 16 ) + ( -1 << 8 ) + ( k );

    m_VirtualPalette[i + 192] = ( ( 255 - k ) << 16 ) + ( -1 << 8 ) + ( -1 );
  }

  for ( int i = 0; i < 4096; i++ )
  {
    double d = ( (double)i / 4096 ) * 2 * 3.1415926535897931;
    m_SinusTabelle[i] = (int)( ( sin( d ) * 256 ) / 2 + 128 );
  }
}



void PlasmaGenerator::SetSpeed( int Speed )
{
  m_Speed = Speed;
}



bool PlasmaGenerator::Generate( GR::Graphic::ContextDescriptor& Target )
{
  int j = m_Y1;
  int k = m_Y2;

  for( int l = 0; l < Target.Height(); l++ )
  {
    int i1 = m_SinusTabelle[j] + m_SinusTabelle[k];
    int j1 = m_X1;
    int k1 = m_X2;
    for(int l1 = 0; l1 < Target.Width(); l1++)
    {
      unsigned char    bValue = (unsigned char)( ( m_SinusTabelle[j1] + m_SinusTabelle[k1] + i1 ) / 4 );

      //*pData++ = m_dwVirtualPalette[bValue];
      Target.PutPixelFast( l1, l, 0xff000000 | ( bValue << 16 ) );

      j1 = ( j1 + 8 ) & 0xfff;
      k1 = ( k1 + 17 ) & 0xfff;
    }
    j = j + 3 & 0xfff;
    k = k - 4 & 0xfff;
  }
  m_X1 = ( m_X1 + 3 * m_Speed ) & 0xfff;
  m_X2 = ( m_X2 - 9 * m_Speed ) & 0xfff;
  m_Y1 = ( m_Y1 + 7 * m_Speed ) & 0xfff;
  m_Y2 = ( m_Y2 - 9 * m_Speed ) & 0xfff;

  return true;
}



