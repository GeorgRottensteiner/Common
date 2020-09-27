#ifndef XS_RENDER_MANAGER_H
#define XS_RENDER_MANAGER_H


#include <DX8/IDX8Shader.h>
#include <DX8/IShadedRenderAble.h>

#include "XSObject.h"

#include <list>
#include <map>


class CRenderManager
{

  protected:

    typedef std::list<IShadedRenderAble*>               tListRenderAbles;

    typedef std::map<GR::eShaderType,tListRenderAbles>  tMapRenderAbles;


    tMapRenderAbles                     m_mapRenderAbles;


  public:


    static CRenderManager& Instance()
    {
      static CRenderManager  g_Instance;

      return g_Instance;
    }

    void CRenderManager::ClearQueue()
    {
      m_mapRenderAbles.clear();
    }

    void CRenderManager::Render( CD3DViewer& Viewer )
    {
      tMapRenderAbles::iterator   it( m_mapRenderAbles.begin() );
      while ( it != m_mapRenderAbles.end() )
      {
        tListRenderAbles&   listRA = it->second;

        switch ( it->first )
        {
          case GR::ST_FLAT:
            GR::CDX8ShaderFlat::Apply( Viewer );
            break;
          case GR::ST_ALPHA_BLEND:
            GR::CDX8ShaderAlphaBlend::Apply( Viewer );
            break;
          case GR::ST_ALPHA_TEST:
            GR::CDX8ShaderAlphaTest::Apply( Viewer );
            break;
          case GR::ST_ALPHA_BLEND_AND_TEST:
            GR::CDX8ShaderAlphaBlendAndTest::Apply( Viewer );
            break;
          case GR::ST_ADDITIVE:
            GR::CDX8ShaderAdditive::Apply( Viewer );
            break;
          case GR::ST_50_PERCENT_BLEND:
            GR::CDX8Shader50PercentBlend::Apply( Viewer );
            break;
          default:
            dh::Log( "invalid shader enum %d\n", it->first );
            break;
        }

        tListRenderAbles::iterator    itRA( listRA.begin() );
        while ( itRA != listRA.end() )
        {
          IShadedRenderAble*    pRA = *itRA;

          CXSObject*  pXSObject = (CXSObject*)pRA;

          Viewer.SetTransform( D3DTS_WORLD, pXSObject->Transformation() );

          pRA->Render( Viewer );

          ++itRA;
        }

        ++it;
      }

    }

    void AddToQueue( IShadedRenderAble* pSRA )
    {
      if ( pSRA )
      {
        m_mapRenderAbles[pSRA->m_eShader].push_back( pSRA );
      }
    }

};


#endif  // XS_RENDER_MANAGER_H

