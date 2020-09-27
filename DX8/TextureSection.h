#ifndef TEXTURESECTION_H_INCLUDED
#define TEXTURESECTION_H_INCLUDED



#include <GR/GRTypes.h>

#include <DX8\DX8Viewer.h>



struct tTextureSection
{
  enum eFlags
  {
    TSF_DEFAULT     = 0x00000000,
    TSF_ROTATE_90   = 0x00000001,
    TSF_ROTATE_180  = 0x00000002,
    TSF_ROTATE_270  = 0x00000003,
    TSF_H_MIRROR    = 0x00000004,
    TSF_V_MIRROR    = 0x00000008,
    TSF_HCENTER     = 0x00000010,
    TSF_VCENTER     = 0x00000020,

    TSF_CENTERED    = TSF_HCENTER | TSF_VCENTER,
  };
  CDX8Texture*              m_pTexture;
  int                       m_Width,
                            m_Height,
                            m_XOffset,
                            m_YOffset;
  float                     m_TU[2],
                            m_TV[2];
  GR::u32                   m_Flags;

  tTextureSection( CDX8Texture* pTexture = NULL, int iX = 0, int iY = 0, int iWidth = 0, int iHeight = 0, GR::u32 dwFlags = TSF_DEFAULT ) :
    m_pTexture( pTexture ),
    m_Width( iWidth ),
    m_Height( iHeight ),
    m_XOffset( iX ),
    m_YOffset( iY ),
    m_Flags( dwFlags )
  {
    CalcTexCoords();
  }

  void CalcTexCoords()
  {
    if ( m_pTexture )
    {
      m_TU[0] = (float)m_XOffset / (float)m_pTexture->m_vSize.x;
      m_TV[0] = (float)m_YOffset / (float)m_pTexture->m_vSize.y;
      m_TU[1] = (float)( m_XOffset + m_Width ) / (float)m_pTexture->m_vSize.x;
      m_TV[1] = (float)( m_YOffset + m_Height ) / (float)m_pTexture->m_vSize.y;
    }
    else
    {
      m_TU[0] = 0.0f;
      m_TV[0] = 0.0f;
      m_TU[1] = 0.0f;
      m_TV[1] = 0.0f;
    }
  }
};



#endif // __TEXTURESECTION_H_INCLUDED__
