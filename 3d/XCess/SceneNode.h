#ifndef _SCENENODE_H
#define _SCENENODE_H

#include <list>

#include <DX8/Frustum.h>
#include <DX8/Orientation.h>

#include <math/matrix4.h>

#include "XSObject.h"
#include "RenderManager.h"





class CSceneNode : public COrientation
{

  protected:

    GR::String                          m_strName;

    CBoundingBox                        m_BoundingBox;


  public:

    typedef std::list<CXSObject*>       tListObjects;

    typedef std::list<CSceneNode*>      tListChilds;


    tListChilds                         m_listChilds;

    tListObjects                        m_listObjects;

    CSceneNode*                         m_pParent;



    CSceneNode( const GR::String& strName ) :
      m_pParent( NULL ),
      m_strName( strName )
    {
    }

    CXSObject*            FindObject( const CXSID& xsID, bool bSearchChilds = true )
    {
      tListObjects::iterator    itRA( m_listObjects.begin() );
      while ( itRA != m_listObjects.end() )
      {
        CXSObject*    pObj = *itRA;

        if ( pObj->ID() == xsID )
        {
          return pObj;
        }

        ++itRA;
      }
      if ( !bSearchChilds )
      {
        return NULL;
      }

      tListChilds::iterator    it( m_listChilds.begin() );
      while ( it != m_listChilds.end() )
      {
        CSceneNode*   pNode = *it;

        CXSObject*    pResult = pNode->FindObject( xsID );
        if ( pResult )
        {
          return pResult;
        }

        ++it;
      }
      return NULL;
    }

    CSceneNode*           FindNode( const GR::String& strName, bool bSearchChilds = true )
    {
      if ( m_strName == strName )
      {
        return this;
      }
      if ( !bSearchChilds )
      {
        return NULL;
      }

      tListChilds::iterator    it( m_listChilds.begin() );
      while ( it != m_listChilds.end() )
      {
        CSceneNode*   pNode = *it;

        CSceneNode*   pResult = pNode->FindNode( strName );
        if ( pResult )
        {
          return pResult;
        }

        ++it;
      }
      return NULL;
    }

    virtual void          Add( CXSObject* pObject )
    {
      if ( pObject == NULL )
      {
        return;
      }
      pObject->m_pNode = this;
      m_listObjects.push_back( pObject );
      m_BoundingBox.Combine( pObject->BoundingBox() );
    }

    bool                  Remove( CXSObject* pObject )
    {
      m_BoundingBox.Clear();

      tListObjects::iterator    itRA( m_listObjects.begin() );
      while ( itRA != m_listObjects.end() )
      {
        CXSObject*    pObj = *itRA;

        if ( pObj == pObject )
        {
          itRA = m_listObjects.erase( itRA );
          delete pObject;
          return true;
        }

        m_BoundingBox.Combine( pObj->BoundingBox() );

        ++itRA;
      }
      tListChilds::iterator    it( m_listChilds.begin() );
      while ( it != m_listChilds.end() )
      {
        CSceneNode*   pNode = *it;

        pNode->Remove( pObject );
        m_BoundingBox.Combine( pNode->BoundingBox() );

        ++it;
      }
      return false;
    }

    void                  Remove( CSceneNode* pRemoveNode )
    {
      m_BoundingBox.Clear();
      tListChilds::iterator    it( m_listChilds.begin() );
      while ( it != m_listChilds.end() )
      {
        CSceneNode*   pNode = *it;

        if ( pNode == pRemoveNode )
        {
          it = m_listChilds.erase( it );
        }
        else
        {
          pNode->Remove( pRemoveNode );
          m_BoundingBox.Combine( pNode->BoundingBox() );
          ++it;
        }
      }
    }

    void RecalcBoundingBox()
    {
      m_BoundingBox.Clear();

      tListObjects::iterator    itRA( m_listObjects.begin() );
      while ( itRA != m_listObjects.end() )
      {
        CXSObject*    pObj = *itRA;

        m_BoundingBox.Combine( pObj->TransformedBoundingBox() );

        ++itRA;
      }
      tListChilds::iterator    it( m_listChilds.begin() );
      while ( it != m_listChilds.end() )
      {
        CSceneNode*   pNode = *it;

        m_BoundingBox.Combine( pNode->BoundingBox() );

        ++it;
      }
    }

    CSceneNode*           Add( CSceneNode* pNode = NULL )
    {
      if ( pNode == NULL )
      {
        pNode = new CSceneNode( "" );
      }
      m_listChilds.push_back( pNode );
      pNode->m_pParent = this;
      m_BoundingBox.Combine( pNode->BoundingBox() );

      return pNode;
    }

    virtual ~CSceneNode()
    {
      tListChilds::iterator    it( m_listChilds.begin() );
      while ( it != m_listChilds.end() )
      {
        CSceneNode*   pChild = *it;

        delete *it;
        ++it;
      }
      tListObjects::iterator    itRA( m_listObjects.begin() );
      while ( itRA != m_listObjects.end() )
      {
        CXSObject*    pRA = *itRA;

        delete pRA;
        ++itRA;
      }
    }

    CBoundingBox BoundingBox()
    {
      CBoundingBox      BB = m_BoundingBox;

      CSceneNode*       pParent = m_pParent;

      math::matrix4     matComplete = Transform();
      matComplete.Inverse();

      while ( pParent )
      {
        matComplete *= pParent->Transform().Inverse();
        //matComplete = pParent->Transform().Inverse() * matComplete;

        pParent = pParent->m_pParent;
      }

      BB.Transform( matComplete );

      return BB;
    }

    CXSObject* PickObject( const math::vector3& vect1, const math::vector3& vect2, math::vector3& vPickPoint )
    {
      math::vector3   vHitNear,
                      vHitFar;

      /*
      if ( !BoundingBox().IntersectWithRay( vect1, vect2, vHitNear, vHitFar ) )
      {
        return NULL;
      }
      */

      math::vector3     vectT1, vectT2;

      math::matrix4     matNodeTransform = Transform().Inverse();

      matNodeTransform.TransformCoord( vect1, vectT1 );
      matNodeTransform.TransformCoord( vect2, vectT2 );


      float         fDistance = FLT_MAX;
      CXSObject*    pCurrentPickedObject = NULL;

      tListObjects::iterator    it( m_listObjects.begin() );
      while ( it != m_listObjects.end() )
      {
        CXSObject*   pRenderAble = *it;

        if ( !( pRenderAble->Flags() & CXSObject::XS_PICKABLE ) )
        {
          ++it;
          continue;
        }
        if ( pRenderAble->BoundingBox().IntersectWithRay( vectT1, vectT2, vHitNear, vHitFar ) )
        {
          float   fLength = ( vHitNear - vect1 ).length();

          //dh::Log( "Hitting %s", pRenderAble->ID().ToString() );

          if ( fLength < fDistance )
          {
            fDistance = fLength;
            pCurrentPickedObject = pRenderAble;

            vPickPoint = vHitNear;
          }
        }

        ++it;
      }

      // check child nodes
      CXSObject*    pChildPick = NULL;

      tListChilds::iterator    itChilds( m_listChilds.begin() );
      while ( itChilds != m_listChilds.end() )
      {
        CSceneNode*   pNode = *itChilds;

        pChildPick = pNode->PickObject( vect1, vect2, vHitNear );
        if ( pChildPick )
        {
          float   fLength = ( vHitNear - vect1 ).length();

          if ( fLength < fDistance )
          {
            fDistance = fLength;
            pCurrentPickedObject = pChildPick;

            vPickPoint = vHitNear;
          }
        }

        ++itChilds;
      }

      return pCurrentPickedObject;
    }

    void QueueRenderAbles( CRenderManager& RManager, CFrustum& Frustum )
    {
      tListObjects::iterator    it( m_listObjects.begin() );
      while ( it != m_listObjects.end() )
      {
        CXSObject*   pRenderAble = *it;

        if ( Frustum.IntersectWithBoundingBox( pRenderAble->TransformedBoundingBox() ) )
        {
          pRenderAble->ToQueue( RManager, Frustum );
        }

        ++it;
      }
    }

    math::matrix4 Transform() const
    {
      math::matrix4   matTemp;

      memcpy( &matTemp, &GetMatrix(), sizeof( math::matrix4 ) );

      return matTemp;
    }

    GR::String Name() const
    {
      return m_strName;
    }

    void PreRender( CD3DViewer& Viewer )
    {
      math::matrix4   matTemp = Transform();

      CSceneNode*   pParent = m_pParent;

      while ( pParent )
      {
        matTemp = pParent->Transform() * matTemp;
        pParent = pParent->m_pParent;
      }

      D3DXMATRIX    d3dxmatTemp;
      memcpy( &d3dxmatTemp, &matTemp, sizeof( math::matrix4 ) );

      Viewer.SetTransform( D3DTS_WORLD, &d3dxmatTemp );
    }

    virtual void          ToQueue( CRenderManager& RManager, CFrustum& Frustum )
    {
      QueueRenderAbles( RManager, Frustum );

      tListChilds::iterator    itNode( m_listChilds.begin() );
      while ( itNode != m_listChilds.end() )
      {
        CSceneNode*   pNode = *itNode;

        pNode->ToQueue( RManager, Frustum );

        ++itNode;
      }
    }

    virtual bool          IsVisible( const CFrustum& Frustum )
    {
      return Frustum.IntersectWithBoundingBox( m_BoundingBox );
    }

    virtual bool          Update( const float fElapsedTime )
    {
      tListObjects::iterator    it( m_listObjects.begin() );
      while ( it != m_listObjects.end() )
      {
        CXSObject*   pRenderAble = *it;

        pRenderAble->Update( fElapsedTime );

        ++it;
      }

      tListChilds::iterator    itNode( m_listChilds.begin() );
      while ( itNode != m_listChilds.end() )
      {
        CSceneNode*   pNode = *itNode;

        pNode->Update( fElapsedTime );

        ++itNode;
      }
      return false;
    }


};

#endif// _SCENENODE_H

