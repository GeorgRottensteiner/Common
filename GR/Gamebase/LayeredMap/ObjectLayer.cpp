#include "ObjectLayer.h"
#include "LayeredMap.h"

#include <debug/debugclient.h>


namespace GR
{
  namespace Gamebase
  {

    ObjectLayer::ObjectLayer() :
      m_SectorWidth( 640 ),
      m_SectorHeight( 640 )
    {
    }



    ObjectLayer::~ObjectLayer()
    {
      ClearObjects();
    }



    void ObjectLayer::ClearObjects()
    {
      std::map<GR::tPoint,std::list<LayerObject*> >::iterator   itGrid( Objects.begin() );
      while ( itGrid != Objects.end() )
      {
        std::list<LayerObject*>&    listObj( itGrid->second );

        std::list<LayerObject*>::iterator   itO( listObj.begin() );
        while ( itO != listObj.end() )
        {
          delete *itO;

          ++itO;
        }

        ++itGrid;
      }
      Objects.clear();
    }



    bool ObjectLayer::IsObjectInGrid( LayerObject* pObj )
    {
      std::map<GR::tPoint, std::list<LayerObject*> >::iterator   itGrid( Objects.begin() );
      while ( itGrid != Objects.end() )
      {
        std::list<LayerObject*>&    listObj( itGrid->second );

        std::list<LayerObject*>::iterator   itO( listObj.begin() );
        while ( itO != listObj.end() )
        {
          LayerObject*    pGridObj( *itO );

          if ( pGridObj == pObj )
          {
            return true;
          }
          ++itO;
        }
        ++itGrid;
      }
      return false;
    }



    void ObjectLayer::AddObject( LayerObject* pObj )
    {
      if ( IsObjectInGrid( pObj ) )
      {
        return;
      }
      GR::tPoint      gridPos( GridPos( pObj ) );

      Objects[gridPos].push_back( pObj );
    }



    void ObjectLayer::AddObjectOnTop( LayerObject* pObj )
    {
      if ( IsObjectInGrid( pObj ) )
      {
        dh::Log( "ObjectLayer::AddObjectOnTop Object already in grid!" );
        return;
      }

      GR::tPoint      gridPos( GridPos( pObj ) );

      //dh::Log( "add object (%s/%x) to grid %d,%d on top", pObj->Template.c_str(), pObj, gridPos.x, gridPos.y );

      Objects[gridPos].push_front( pObj );
    }



    void ObjectLayer::RemoveObject( LayerObject* pObj )
    {
      GR::tPoint      gridPos( GridPos( pObj ) );

      //dh::Log( "Remove object (%s/%x) from grid %d,%d", pObj->Template.c_str(), pObj, gridPos.x, gridPos.y );

      if ( Objects.find( gridPos ) == Objects.end() )
      {
        //dh::Log( "Object %s was not in layer! (grid not used)", pObj->Template.c_str() );
        return;
      }
      Objects[gridPos].remove( pObj );
    }


    void ObjectLayer::RemoveObject( const GR::tPoint& GridPos, LayerObject* pObj )
    {
      if ( Objects.find( GridPos ) == Objects.end() )
      {
        //dh::Log( "Removing non existing obj pos 1 (grid pos not initd) (%s/%d)", pObj->Template.c_str(), pObj );
        return;
      }
      /*
      if ( std::find( Objects[GridPos].begin(), Objects[GridPos].end(), pObj ) == Objects[GridPos].end() )
      {
        dh::Log( "Removing non existing obj (%s/%d)", pObj->Template.c_str(), pObj );
        auto    itO( Objects.begin() );
        while ( itO != Objects.end() )
        {
          if ( std::find( itO->second.begin(), itO->second.end(), pObj ) != itO->second.end() )
          {
            dh::Log( "-was in grid %d,%d", itO->first.x, itO->first.y );
            break;
          }

          ++itO;
        }
      }*/
      Objects[GridPos].remove( pObj );
    }



    void ObjectLayer::SetObjectPos( LayerObject* pObj, const GR::tPoint& NewPos )
    {
      MoveObject( pObj, NewPos - pObj->Position );
    }



    void ObjectLayer::MoveObject( LayerObject* pObj, const GR::tPoint& Delta )
    {
      GR::tPoint      oldGridPos( GridPos( pObj ) );
      GR::tPoint      newPos( pObj->Position + Delta );
      GR::tPoint      newGridPos( GridPos( newPos ) );

      if ( oldGridPos != newGridPos )
      {
        RemoveObject( pObj );
        pObj->Position = newPos;
        AddObject( pObj );
      }
      else
      {
        pObj->Position = newPos;
      }
    }



    int ObjectLayer::GetObjectCountByType( const GR::String& ObjType )
    {
      int     count = 0;

      std::map<GR::tPoint, std::list<LayerObject*> >::iterator   itGrid( Objects.begin() );
      while ( itGrid != Objects.end() )
      {
        std::list<LayerObject*>&    listObj( itGrid->second );

        std::list<LayerObject*>::iterator   itO( listObj.begin() );
        while ( itO != listObj.end() )
        {
          LayerObject*    pObj( *itO );

          if ( pObj->Template == ObjType )
          {
            ++count;
          }
          ++itO;
        }
        ++itGrid;
      }

      return count;
    }



    LayerObject* ObjectLayer::FindCollidingObject( LayerObject* pCollider, const GR::String& ObjType )
    {
      std::map<GR::tPoint, std::list<LayerObject*> >::iterator   itGrid( Objects.begin() );
      while ( itGrid != Objects.end() )
      {
        std::list<LayerObject*>&    listObj( itGrid->second );

        std::list<LayerObject*>::iterator   itO( listObj.begin() );
        while ( itO != listObj.end() )
        {
          LayerObject*    pObj( *itO );

          if ( ( pObj->Template == ObjType )
          &&   ( pObj->Bounds().Intersects( pCollider->Bounds() ) ) )
          {
            return pObj;
          }
          ++itO;
        }
        ++itGrid;
      }
      return NULL;
    }



    LayerObject* ObjectLayer::FindObjectByID( GR::u32 ID )
    {
      std::map<GR::tPoint,std::list<LayerObject*> >::iterator   itGrid( Objects.begin() );
      while ( itGrid != Objects.end() )
      {
        std::list<LayerObject*>&    listObj( itGrid->second );

        std::list<LayerObject*>::iterator   itO( listObj.begin() );
        while ( itO != listObj.end() )
        {
          LayerObject*    pObj( *itO );

          if ( pObj->ID == ID )
          {
            return pObj;
          }
          ++itO;
        }
        ++itGrid;
      }
      return NULL;
    }



    LayerObject* ObjectLayer::FindObjectByType( const GR::String& ObjType )
    {
      std::map<GR::tPoint,std::list<LayerObject*> >::iterator   itGrid( Objects.begin() );
      while ( itGrid != Objects.end() )
      {
        std::list<LayerObject*>&    listObj( itGrid->second );

        std::list<LayerObject*>::iterator   itO( listObj.begin() );
        while ( itO != listObj.end() )
        {
          LayerObject*    pObj( *itO );

          if ( pObj->Template == ObjType )
          {
            return pObj;
          }
          ++itO;
        }
        ++itGrid;
      }
      return NULL;
    }



    bool ObjectLayer::FindObjectsAt( const GR::tPoint& Position, std::list<LayerObject*>& ObjectList )
    {
      GR::tPoint    gridKey( Position.x / m_SectorWidth, Position.y / m_SectorHeight );
      if ( Position.x < 0 )
      {
        --gridKey.x;
      }
      if ( Position.y < 0 )
      {
        --gridKey.y;
      }
      std::map<GR::tPoint,std::list<LayerObject*> >::iterator   itGrid( Objects.find( gridKey ) );
      if ( itGrid == Objects.end() )
      {
        return false;
      }
      std::list<LayerObject*>&    listObj( itGrid->second );

      std::list<LayerObject*>::iterator   itO( listObj.begin() );
      while ( itO != listObj.end() )
      {
        LayerObject*    pObj( *itO );

        if ( pObj->Position == Position )
        {
          ObjectList.push_back( pObj );
        }
        ++itO;
      }
      return !ObjectList.empty();
    }



    GR::i32 ObjectLayer::SectorWidth() const
    {
      return m_SectorWidth;
    }



    GR::i32 ObjectLayer::SectorHeight() const
    {
      return m_SectorHeight;
    }



    GR::tPoint ObjectLayer::GridPos( LayerObject* pObj )
    {
      return GridPos( pObj->Position );
    }



    GR::tPoint ObjectLayer::GridPos( const GR::tPoint& Pos )
    {
      GR::tPoint    gridKey( Pos.x / m_SectorWidth, Pos.y / m_SectorHeight );
      if ( Pos.x < 0 )
      {
        --gridKey.x;
      }
      if ( Pos.y < 0 )
      {
        --gridKey.y;
      }
      return gridKey;
    }



  }
}