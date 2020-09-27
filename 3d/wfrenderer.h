#ifndef _PJ_wfrenderer_h
#define _PJ_wfrenderer_h

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <vector>

#include <3d/wavefront.h>

namespace WFRenderer
{
  //- interface für alle WF-Renderer
  class IWFRenderer
  {
  public:
    virtual void Render( const CWF& wf ) = 0; 
    virtual ~IWFRenderer() = 0 {};
    IWFRenderer() {}
  private:
    // IWFRenderer(); // n.i.
    IWFRenderer( const IWFRenderer& ); // n.i.
    IWFRenderer& operator=( const IWFRenderer& ); // n.i.
  };

  class CWFDXRenderer : public IWFRenderer
  {
  public:
    CWFDXRenderer( IDirect3DDevice8& p3ddevice ) : m_pd3dDevice( &p3ddevice ) { }
  protected:
    IDirect3DDevice8* m_pd3dDevice;
    CWFDXRenderer(); // n.i.
    CWFDXRenderer( const CWFDXRenderer& ); // n.i.
    CWFDXRenderer& operator=( const CWFDXRenderer& ); // n.i.
    virtual ~CWFDXRenderer() {}
  };

  class CTexWireframe : public CWFDXRenderer
  {
  public:
    CTexWireframe( IDirect3DDevice8& p3ddevice ) : CWFDXRenderer( p3ddevice ) { }
    virtual void Render( const CWF& wf )
    {
      m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE );
      int p = 0;
      typedef std::vector<tMyVertexNOTEX> vertexvector;
      vertexvector vertexData( 80 ); //- unterstützung von 80-Ecken

      CWF::face_collection_t::const_iterator itf( wf.f_.begin() ), itfend( wf.f_.end() );
      for ( ; itf != itfend; ++itf )
      {
        const CWF::face_t&  f     = *itf;
        for ( p = 0; p < f.size(); ++p )
        {
          const CWF::facenode_t&  fn  = f[p];
          tMyVertexNOTEX&         v   = vertexData[p];
          v.position = wf.vt_ [ fn.vt_  ];
          v.normal   = wf.vn_[ fn.vn_ ];
          v.colorDiffuse = 0xffffffff;
        }
        const CWF::facenode_t&  fn  = f[0];
        tMyVertexNOTEX&         v   = vertexData[p];
        v.position = wf.vt_ [ fn.vt_  ];
        v.normal   = wf.vn_[ fn.vn_ ];
        v.colorDiffuse = 0xffffffff;
        m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINESTRIP, f.size(), &vertexData[0], sizeof( tMyVertexNOTEX ) );
      }
    }
  private:
    struct tMyVertexNOTEX
    {
      CWF::vertex_t position;
      CWF::vertex_t normal;
      D3DCOLOR      colorDiffuse;    // The color
    };
  };

  class CTexTexture : public CWFDXRenderer
  {
  public:
    CTexTexture( IDirect3DDevice8& p3ddevice ) : CWFDXRenderer( p3ddevice ) { }
    virtual void Render( const CWF& wf )
    {
      m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
      m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
      int p = 0;
      typedef std::vector<tMyVertexXYZ_NORMAL_DIFFUSE_TEX1> vertexvector;
      vertexvector vertexData( 80 ); //- unterstützung von 80-Ecken
      CWF::face_collection_t::const_iterator itf( wf.f_.begin() ), itfend( wf.f_.end() );
      for ( ; itf != itfend; ++itf )
      {
        const CWF::face_t&  f     = *itf;
        for ( p = 0; p < f.size(); ++p )
        {
          const CWF::facenode_t&            fn  = f[p];
          tMyVertexXYZ_NORMAL_DIFFUSE_TEX1& v   = vertexData[p];
          v.position  = wf.vt_[ fn.vt_ ];
          v.normal    = wf.vn_[ fn.vn_ ];
          // const CWF::vertex_t& tex  = wf.vt_[ fn.vt_ ]; v.fU = tex.x; v.fV = (bTexFlipY? (1-tex.y) : tex.y);
          const CWF::vertex_t& tex  = wf.vt_[ fn.vt_ ]; v.fU = tex.x; v.fV = 1-tex.y;
          v.colorDiffuse = 0xffffffff;
        }
        m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, f.size() - 2, &vertexData[0], sizeof( tMyVertexXYZ_NORMAL_DIFFUSE_TEX1 ) );
      }
    }
  private:
    struct tMyVertexXYZ_NORMAL_DIFFUSE_TEX1
    {
      CWF::vertex_t position;
      CWF::vertex_t normal;
      D3DCOLOR      colorDiffuse;    // The color
      float         fU, fV;
    };
  };

  class CWireframe : public CWFDXRenderer
  {
  public:
    CWireframe( IDirect3DDevice8& p3ddevice ) : CWFDXRenderer( p3ddevice ) { }
    virtual void Render( const CWF& wf ) 
    {
      m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE );
      int p = 0;
      typedef std::vector<tMyVertexNOTEX> vertexvector;
      vertexvector vertexData( 80 ); //- unterstützung von 80-Ecken

      CWF::face_collection_t::const_iterator itf( wf.f_.begin() ), itfend( wf.f_.end() );
      for ( ; itf != itfend; ++itf )
      {
        const CWF::face_t&  f     = *itf;
        for ( p = 0; p < f.size(); ++p )
        {
          const CWF::facenode_t&  fn  = f[p];
          tMyVertexNOTEX&         v   = vertexData[p];
          v.position = wf.v_ [ fn.v_  ];
          v.normal   = wf.vn_[ fn.vn_ ];
          v.colorDiffuse = 0xffffffff;
        }
        const CWF::facenode_t&  fn  = f[0];
        tMyVertexNOTEX&         v   = vertexData[p];
        v.position = wf.v_ [ fn.v_  ];
        v.normal   = wf.vn_[ fn.vn_ ];
        v.colorDiffuse = 0xffffffff;
        m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINESTRIP, f.size(), &vertexData[0], sizeof( tMyVertexNOTEX ) );
      }
    }
  private:
    struct tMyVertexNOTEX
    {
      CWF::vertex_t position;
      CWF::vertex_t normal;
      D3DCOLOR      colorDiffuse;    // The color
    };
  };

  class CTexture : public CWFDXRenderer
  {
  public:
    CTexture( IDirect3DDevice8& p3ddevice ) : CWFDXRenderer( p3ddevice ) { }
    virtual void Render( const CWF& wf ) 
    {
      m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
      int p = 0;
      typedef std::vector<tMyVertexXYZ_NORMAL_DIFFUSE_TEX1> vertexvector;
      vertexvector vertexData( 80 ); //- unterstützung von 80-Ecken
      CWF::face_collection_t::const_iterator itf( wf.f_.begin() ), itfend( wf.f_.end() );
      for ( ; itf != itfend; ++itf )
      {
        const CWF::face_t&  f     = *itf;
        for ( p = 0; p < f.size(); ++p )
        {
          const CWF::facenode_t&            fn  = f[p];
          tMyVertexXYZ_NORMAL_DIFFUSE_TEX1& v   = vertexData[p];
          v.position  = wf.v_ [ fn.v_  ];
          v.normal    = wf.vn_[ fn.vn_ ];
          // const CWF::vertex_t& tex  = wf.vt_[ fn.vt_ ]; v.fU = tex.x; v.fV = (bTexFlipY? (1-tex.y) : tex.y);
          const CWF::vertex_t& tex  = wf.vt_[ fn.vt_ ]; v.fU = tex.x; v.fV = 1-tex.y;
          v.colorDiffuse = 0xffffffff;
        }
        m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, f.size() - 2, &vertexData[0], sizeof( tMyVertexXYZ_NORMAL_DIFFUSE_TEX1 ) );
      }
    }
  private:
    struct tMyVertexXYZ_NORMAL_DIFFUSE_TEX1
    {
      CWF::vertex_t position;
      CWF::vertex_t normal;
      D3DCOLOR      colorDiffuse;    // The color
      float         fU, fV;
    };
  };

  class CNoTexture : public CWFDXRenderer
  {
  public:
    CNoTexture( IDirect3DDevice8& p3ddevice ) : CWFDXRenderer( p3ddevice ) { }
    virtual void Render( const CWF& wf )
    {
      m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE );
      int p = 0;
      typedef std::vector<tMyVertexNOTEX> vertexvector;
      vertexvector vertexData( 80 );
      CWF::face_collection_t::const_iterator itf( wf.f_.begin() ), itfend( wf.f_.end() );
      for ( ; itf != itfend; ++itf )
      {
        const CWF::face_t&  f     = *itf;
        for ( p = 0; p < f.size(); ++p )
        {
          const CWF::facenode_t&  fn  = f[p];
          tMyVertexNOTEX&         v   = vertexData[p];
          v.position = wf.v_ [ fn.v_  ];
          v.normal   = wf.vn_[ fn.vn_ ];
          v.colorDiffuse = 0xffffffff;
        }
        m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, f.size() - 2, &vertexData[0], sizeof( tMyVertexNOTEX ) );
      }
    }
  private:
    struct tMyVertexNOTEX
    {
      CWF::vertex_t position;
      CWF::vertex_t normal;
      D3DCOLOR      colorDiffuse;    // The color
    };
  };

}

#endif//_PJ_wfrenderer_h
