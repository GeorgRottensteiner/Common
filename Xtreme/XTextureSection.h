#ifndef XTEXTURESECTION_H_INCLUDED
#define XTEXTURESECTION_H_INCLUDED



#include <GR/GRTypes.h>

#include "XTexture.h"



struct XTextureSection
{
  enum eFlags
  {
    TSF_DEFAULT       = 0x00000000,
    TSF_ROTATE_90     = 0x00000001,
    TSF_ROTATE_180    = 0x00000002,
    TSF_ROTATE_270    = 0x00000003,
    TSF_ROTATION_MASK = 0x00000003,
    TSF_H_MIRROR      = 0x00000004,
    TSF_V_MIRROR      = 0x00000008,
    TSF_HCENTER       = 0x00000010,
    TSF_VCENTER       = 0x00000020,
    TSF_ALIGN_BOTTOM  = 0x00000040,
    TSF_ALIGN_RIGHT   = 0x00000080,
    TSF_WRAPPED_H     = 0x00000100,
    TSF_WRAPPED_V     = 0x00000200,

    TSF_CENTERED    = TSF_HCENTER | TSF_VCENTER,
  };
  XTexture*                 m_pTexture;
  int                       m_Width,
                            m_Height,
                            m_XOffset,
                            m_YOffset;
  GR::u32                   m_Flags;



  XTextureSection( XTexture* pTexture = NULL, int X = 0, int Y = 0, int Width = -1, int Height = -1, GR::u32 Flags = TSF_DEFAULT ) :
    m_pTexture( pTexture ),
    m_Width( Width ),
    m_Height( Height ),
    m_XOffset( X ),
    m_YOffset( Y ),
    m_Flags( Flags )
  {
    if ( ( m_pTexture )
    &&   ( Width == -1 )
    &&   ( Height == -1 ) )
    {
      m_Width  = m_pTexture->m_SurfaceSize.x;
      m_Height = m_pTexture->m_SurfaceSize.y;
    }
    if ( m_Width == -1 )
    {
      m_Width = 0;
    }
    if ( m_Height == -1 )
    {
      m_Height = 0;
    }
  }



  void GetTrueUV( GR::tFPoint& UV1, GR::tFPoint& UV2, GR::tFPoint& UV3, GR::tFPoint& UV4 ) const
  {
    GetTrueUV( UV1, UV2, UV3, UV4, m_Flags );
  }



  void GetTrueUV( GR::tFPoint& UV1, GR::tFPoint& UV2, GR::tFPoint& UV3, GR::tFPoint& UV4, GR::u32 Flags ) const
  {
    if ( m_pTexture == NULL )
    {
      UV1.Set( 0.0f, 0.0f );
      UV2.Set( 0.0f, 0.0f );
      UV3.Set( 0.0f, 0.0f );
      UV4.Set( 0.0f, 0.0f );
      return;
    }

    GR::tPoint    p1;
    GR::tPoint    p2;
    GR::tPoint    p3;
    GR::tPoint    p4;

    switch ( Flags & XTextureSection::TSF_ROTATION_MASK )
    {
      case 0:
        // no rotation
        if ( ( Flags & XTextureSection::TSF_H_MIRROR )
        &&   ( Flags & XTextureSection::TSF_V_MIRROR ) )
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset;
          p4.x = m_XOffset;
          p4.y = m_YOffset;
        }
        else if ( Flags & XTextureSection::TSF_H_MIRROR )
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset;
          p2.x = m_XOffset;
          p2.y = m_YOffset;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset;
          p4.y = m_YOffset + m_Height;
        }
        else if ( Flags & XTextureSection::TSF_V_MIRROR )
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset;
          p3.y = m_YOffset;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset;
        }
        else
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset;
          p3.x = m_XOffset;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset + m_Height;
        }
        break;
      case XTextureSection::TSF_ROTATE_90:
        if ( ( Flags & XTextureSection::TSF_H_MIRROR )
        &&   ( Flags & XTextureSection::TSF_V_MIRROR ) )
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset;
          p2.y = m_YOffset;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset;
        }
        else if ( Flags & XTextureSection::TSF_H_MIRROR )
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset;
          p2.x = m_XOffset;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset + m_Height;
        }
        else if ( Flags & XTextureSection::TSF_V_MIRROR )
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset;
          p3.x = m_XOffset;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset;
          p4.y = m_YOffset;
        }
        else
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset;
          p3.y = m_YOffset;
          p4.x = m_XOffset;
          p4.y = m_YOffset + m_Height;
        }
        break;
      case XTextureSection::TSF_ROTATE_180:
        if ( ( Flags & XTextureSection::TSF_H_MIRROR )
        &&   ( Flags & XTextureSection::TSF_V_MIRROR ) )
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset;
          p3.x = m_XOffset;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset + m_Height;
        }
        else if ( Flags & XTextureSection::TSF_H_MIRROR )
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset;
          p3.y = m_YOffset;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset;
        }
        else if ( Flags & XTextureSection::TSF_V_MIRROR )
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset;
          p2.x = m_XOffset;
          p2.y = m_YOffset;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset;
          p4.y = m_YOffset + m_Height;
        }
        else
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset;
          p4.x = m_XOffset;
          p4.y = m_YOffset;
        }
        break;
      case XTextureSection::TSF_ROTATE_270:
        if ( ( Flags & XTextureSection::TSF_H_MIRROR )
        &&   ( Flags & XTextureSection::TSF_V_MIRROR ) )
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset;
          p3.y = m_YOffset;
          p4.x = m_XOffset;
          p4.y = m_YOffset + m_Height;
        }
        else if ( Flags & XTextureSection::TSF_H_MIRROR )
        {
          p1.x = m_XOffset + m_Width;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset + m_Width;
          p2.y = m_YOffset;
          p3.x = m_XOffset;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset;
          p4.y = m_YOffset;
        }
        else if ( Flags & XTextureSection::TSF_V_MIRROR )
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset;
          p2.x = m_XOffset;
          p2.y = m_YOffset + m_Height;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset + m_Height;
        }
        else
        {
          p1.x = m_XOffset;
          p1.y = m_YOffset + m_Height;
          p2.x = m_XOffset;
          p2.y = m_YOffset;
          p3.x = m_XOffset + m_Width;
          p3.y = m_YOffset + m_Height;
          p4.x = m_XOffset + m_Width;
          p4.y = m_YOffset;
        }
        break;
    }

    UV1.x = (float)p1.x / (float)m_pTexture->m_SurfaceSize.x;
    UV1.y = (float)p1.y / (float)m_pTexture->m_SurfaceSize.y;
    UV2.x = (float)p2.x / (float)m_pTexture->m_SurfaceSize.x;
    UV2.y = (float)p2.y / (float)m_pTexture->m_SurfaceSize.y;
    UV3.x = (float)p3.x / (float)m_pTexture->m_SurfaceSize.x;
    UV3.y = (float)p3.y / (float)m_pTexture->m_SurfaceSize.y;
    UV4.x = (float)p4.x / (float)m_pTexture->m_SurfaceSize.x;
    UV4.y = (float)p4.y / (float)m_pTexture->m_SurfaceSize.y;
  }



  int VisualWidth() const
  {
    if ( ( ( m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_270 )
    ||   ( ( m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_90 ) )
    {
      return m_Height;
    }
    return m_Width;
  }



  int VisualHeight() const
  {
    if ( ( ( m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_270 )
    ||   ( ( m_Flags & XTextureSection::TSF_ROTATION_MASK ) == XTextureSection::TSF_ROTATE_90 ) )
    {
      return m_Width;
    }
    return m_Height;
  }

};



#endif // XTEXTURESECTION_H_INCLUDED
