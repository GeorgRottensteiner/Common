#pragma once

namespace GR
{

class CColor
{

  public:

    float           m_fR,
                    m_fG,
                    m_fB,
                    m_fA;


    CColor( unsigned long dwARGB = 0xffffffff )
    {

      m_fA = ( ( dwARGB & 0xff000000 ) >> 24 ) / 255.0f;
      m_fR = ( ( dwARGB & 0x00ff0000 ) >> 16 ) / 255.0f;
      m_fG = ( ( dwARGB & 0x0000ff00 ) >>  8 ) / 255.0f;
      m_fB = ( ( dwARGB & 0x000000ff )       ) / 255.0f;

    }

    CColor( float fR, float fG, float fB, float fA ) :
      m_fR( fR ),
      m_fG( fG ),
      m_fB( fB ),
      m_fA( fA )
    {

    }

    unsigned long AsARGB() const
    {
      return   ( ( (unsigned char)( m_fA * 255 ) ) << 24 )
             + ( ( (unsigned char)( m_fR * 255 ) ) << 16 )
             + ( ( (unsigned char)( m_fG * 255 ) ) << 8 )
             +   ( (unsigned char)( m_fB * 255 ) );
    }

    operator unsigned long() const 
    { 
      return AsARGB(); 
    }


};


};