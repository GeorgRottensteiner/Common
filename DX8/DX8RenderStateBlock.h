#ifndef DX8RENDERSTATEBLOCK_H
#define DX8RENDERSTATEBLOCK_H

#include <GR/GRTypes.h>

#include "DX8Texture.h"


class CDX8RenderStateBlock
{

  public:

    typedef std::map<D3DTEXTURESTAGESTATETYPE,DWORD>    tMapTextureStageStates;
    typedef std::map<D3DRENDERSTATETYPE,DWORD>          tMapRenderStates;


    tMapTextureStageStates            m_mapTextureStageStates[8];

    tMapRenderStates                  m_mapRenderStates;

    GR::u32                           m_dwVertexShader;


    CDX8RenderStateBlock() :
      m_dwVertexShader( 0 )
    {
    }

    ~CDX8RenderStateBlock()
    {
    }

    virtual HRESULT SetRenderState( D3DRENDERSTATETYPE dwState, GR::u32 dwValue )
    {
      tMapRenderStates::iterator  it( m_mapRenderStates.find( dwState ) );
      if ( it != m_mapRenderStates.end() )
      {
        if ( ( it->first == dwState )
        &&   ( it->second == dwValue ) )
        {
          // schon gesetzt
          return S_OK;
        }
      }
      m_mapRenderStates[dwState] = dwValue;
      return E_FAIL;
    }

    virtual HRESULT SetTextureStageState( GR::u32 dwStage, D3DTEXTURESTAGESTATETYPE dwState, GR::u32 dwValue )
    {
      if ( dwStage >= 8 )
      {
        return S_OK;
      }
      tMapTextureStageStates::iterator  it( m_mapTextureStageStates[dwStage].find( dwState ) );
      if ( it != m_mapTextureStageStates[dwStage].end() )
      {
        if ( ( it->first == dwState )
        &&   ( it->second == dwValue ) )
        {
          // schon gesetzt
          return S_OK;
        }
      }
      m_mapTextureStageStates[dwStage][dwState] = dwValue;
      return E_FAIL;
    }

    virtual HRESULT SetVertexShader( GR::u32 dwHandle )
    {
      if ( m_dwVertexShader == dwHandle )
      {
        return S_OK;
      }
      m_dwVertexShader = dwHandle;
      return E_FAIL;
    }

    GR::u32 GetRenderState( D3DRENDERSTATETYPE dwState )
    {
      tMapRenderStates::iterator  it( m_mapRenderStates.find( dwState ) );
      if ( it == m_mapRenderStates.end() )
      {
        return 0;
      }
      return it->second;
    }

    GR::u32 GetTextureStageState( DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState )
    {
      if ( dwStage >= 8 )
      {
        return 0;
      }

      tMapTextureStageStates::iterator  it( m_mapTextureStageStates[dwStage].find( dwState ) );
      if ( it == m_mapTextureStageStates[dwStage].end() )
      {
        return 0;
      }
      return it->second;
    }

    void ClearRenderStates()
    {
      for ( int i = 0; i < 8; ++i )
      {
        m_mapTextureStageStates[i].clear();
      }
      m_mapRenderStates.clear();
      m_dwVertexShader = 0;
    }

    void Apply( CDX8RenderStateBlock& RenderStateBlock )
    {
      for ( int i = 0; i < 8; ++i )
      {
        tMapTextureStageStates::iterator  it( m_mapTextureStageStates[i].begin() );
        while ( it != m_mapTextureStageStates[i].end() )
        {
          RenderStateBlock.SetTextureStageState( i, it->first, it->second );

          ++it;
        }
      }
      tMapRenderStates::iterator  it( m_mapRenderStates.begin() );
      while ( it != m_mapRenderStates.end() )
      {
        RenderStateBlock.SetRenderState( it->first, it->second );

        ++it;
      }
      RenderStateBlock.SetVertexShader( m_dwVertexShader );
    }


};

#endif // DX8RENDERSTATEBLOCK_H