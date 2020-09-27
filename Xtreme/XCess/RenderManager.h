#ifndef XS_RENDER_MANAGER_H
#define XS_RENDER_MANAGER_H


#include <Xtreme/IShadedRenderAble.h>

#include "XSObject.h"

#include <list>
#include <map>


class CRenderManager
{

  protected:

    typedef std::list<IShadedRenderAble*>               tListRenderAbles;

    typedef std::map<XRenderer::eShaderType,tListRenderAbles>  tMapRenderAbles;


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

    void CRenderManager::Render( XRenderer& Renderer )
    {
      tMapRenderAbles::iterator   it( m_mapRenderAbles.begin() );
      while ( it != m_mapRenderAbles.end() )
      {
        tListRenderAbles&   listRA = it->second;

        Renderer.SetShader( it->first );

        tListRenderAbles::iterator    itRA( listRA.begin() );
        while ( itRA != listRA.end() )
        {
          IShadedRenderAble*    pRA = *itRA;

          CXSObject*  pXSObject = (CXSObject*)pRA;

          Renderer.SetTransform( XRenderer::TT_WORLD, pXSObject->Transformation() );

          pRA->Render( Renderer );

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

