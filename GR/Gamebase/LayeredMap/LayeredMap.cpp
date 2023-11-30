#include "LayeredMap.h"
#include "LayeredMap.h"
#include "TileLayer.h"
#include "SectionedLayer.h"
#include "FlagLayer.h"

#include <IO/FileChunk.h>
#include <IO/FileStream.h>

#include <Memory/MemoryStream.h>

#include <debug/debugclient.h>



namespace GR
{
  namespace Gamebase
  {

    LayeredMap::LayeredMap()
    {
    }



    LayeredMap::~LayeredMap()
    {
      Clear();
    }



    void LayeredMap::Clear()
    {
      std::vector<Layer*>::iterator   itL( Layers.begin() );
      while ( itL != Layers.end() )
      {
        delete *itL;

        ++itL;
      }
      Layers.clear();
      ExtraDatas.clear();
      Regions.clear();
    }



    bool LayeredMap::Load( IIOStream& ioIn )
    {
      Scripts.clear();
      GR::IO::FileChunk   chunk;

      while ( chunk.Read( ioIn ) )
      {
        MemoryStream    memIn( chunk.GetMemoryStream() );

        switch ( chunk.Type() )
        {
          case GR::Gamebase::FileChunks::MAP:
            // Map-Info
            {
              GR::u32       version = memIn.ReadU32();
              GR::String    name    = memIn.ReadString();
            }
            break;
          case GR::Gamebase::FileChunks::MAP_TRIGGER:
            {
              Trigger     trigger;

              trigger.ID          = memIn.ReadU32();
              trigger.Rect.Left   = memIn.ReadI32();
              trigger.Rect.Top    = memIn.ReadI32();
              trigger.Rect.Right  = trigger.Rect.Left + memIn.ReadI32();
              trigger.Rect.Bottom = trigger.Rect.Top + memIn.ReadI32();
              trigger.Type        = (Trigger::TriggerType)memIn.ReadU32();
              trigger.Flags       = memIn.ReadU32();
              trigger.EnterScript = memIn.ReadString();
              trigger.LeaveScript = memIn.ReadString();
              trigger.ExtraDataID = memIn.ReadU32();

              AddTrigger( trigger );
            }
            break;
          case GR::Gamebase::FileChunks::MAP_EXTRADATA:
            {
              GR::Game::ExtraData     extraData;

              extraData.ID      = memIn.ReadI32();
              extraData.Type    = ( GR::Game::ExtraDataType::Value)memIn.ReadU32();
              extraData.Param1  = memIn.ReadI32();
              extraData.Param2  = memIn.ReadI32();
              extraData.Param3  = memIn.ReadI32();
              extraData.Param4  = memIn.ReadI32();
              extraData.Param   = memIn.ReadString();

              ExtraDatas[extraData.ID] = extraData;
            }
            break;
          case GR::Gamebase::FileChunks::MAP_MOVEMENT_PATH:
            {
              GR::String    pathName = memIn.ReadString();

              GR::Game::Path::MovementPath&   path( MovementPaths[pathName] );

              path.Load( memIn );
            }
            break;
          case GR::Gamebase::FileChunks::MAP_REGION:
            {
              GR::tRect   region;

              region.Left   = memIn.ReadI32();
              region.Top    = memIn.ReadI32();
              region.Right  = region.Left + memIn.ReadI32();
              region.Bottom = region.Top + memIn.ReadI32();

              AddRegion( region );
            }
            break;
          case GR::Gamebase::FileChunks::SCRIPT:
            {
              GR::String  scriptName = memIn.ReadString();
              GR::String  script     = memIn.ReadString();

              Scripts[scriptName] = script;
            }
            break;
          case GR::Gamebase::FileChunks::LAYER_TILES:
            // Layer
            {
              TileLayer*    pLayer = new TileLayer();

              pLayer->Name          = memIn.ReadString();
              pLayer->Type          = (Layer::LayerType)memIn.ReadU32();
              pLayer->Tileset       = memIn.ReadString();
              pLayer->TileSpacingX  = memIn.ReadU32();
              pLayer->TileSpacingY  = memIn.ReadU32();
              int     Width         = memIn.ReadI32();
              int     Height        = memIn.ReadI32();

              pLayer->Resize( Width / pLayer->TileSpacingX, Height / pLayer->TileSpacingY );

              memIn.ReadBlock( pLayer->Data(), pLayer->DataSize() );

              pLayer->RelativeSpeed = !!memIn.ReadU8();

              // visible
              memIn.ReadU8();

              pLayer->Offset.x = memIn.ReadI32();
              pLayer->Offset.y = memIn.ReadI32();
              pLayer->Rotation = memIn.ReadF32();
              pLayer->RotationOffset.x = memIn.ReadI32();
              pLayer->RotationOffset.y = memIn.ReadI32();
              pLayer->Colorkeyed  = !!memIn.ReadU8();
              pLayer->ColorKey    = memIn.ReadU32();
              pLayer->Tinted      = !!memIn.ReadU8();
              pLayer->TintColor   = memIn.ReadU32();
              pLayer->Visibility  = (Layer::LayerVisibility)memIn.ReadU32();

              pLayer->m_pMap = this;
              Layers.push_back( pLayer );
            }
            break;
          case GR::Gamebase::FileChunks::LAYER_FLAGS:
            // Layer
            {
              FlagLayer*    pLayer = new FlagLayer();

              pLayer->Name          = memIn.ReadString();
              pLayer->Type          = (Layer::LayerType)memIn.ReadU32();
              pLayer->TileSpacingX  = memIn.ReadU32();
              pLayer->TileSpacingY  = memIn.ReadU32();
              int     Width         = memIn.ReadI32();
              int     Height        = memIn.ReadI32();

              pLayer->Resize( Width / pLayer->TileSpacingX, Height / pLayer->TileSpacingY );

              memIn.ReadBlock( pLayer->Data(), pLayer->DataSize() );

              pLayer->RelativeSpeed = !!memIn.ReadU8();

              // visible
              memIn.ReadU8();

              pLayer->Offset.x = memIn.ReadI32();
              pLayer->Offset.y = memIn.ReadI32();
              pLayer->Rotation = memIn.ReadF32();
              pLayer->RotationOffset.x = memIn.ReadI32();
              pLayer->RotationOffset.y = memIn.ReadI32();
              pLayer->Colorkeyed  = !!memIn.ReadU8();
              pLayer->ColorKey    = memIn.ReadU32();
              pLayer->Tinted      = !!memIn.ReadU8();
              pLayer->TintColor   = memIn.ReadU32();

              pLayer->m_pMap = this;
              Layers.push_back( pLayer );
            }
            break;
          case GR::Gamebase::FileChunks::LAYER_FREE_SECTION:
            // Layer
            {
              SectionedLayer*   pLayer = new SectionedLayer();

              pLayer->Name    = memIn.ReadString();
              pLayer->Type    = (Layer::LayerType)memIn.ReadU32();
              int     Width   = memIn.ReadI32();
              int     Height  = memIn.ReadI32();
              pLayer->RelativeSpeed = !!memIn.ReadU8();
              // visible
              memIn.ReadU8();

              pLayer->Offset.x = memIn.ReadI32();
              pLayer->Offset.y = memIn.ReadI32();
              pLayer->Rotation = memIn.ReadF32();
              pLayer->RotationOffset.x = memIn.ReadI32();
              pLayer->RotationOffset.y = memIn.ReadI32();
              pLayer->Colorkeyed  = !!memIn.ReadU8();
              pLayer->ColorKey    = memIn.ReadU32();
              pLayer->Tinted      = !!memIn.ReadU8();
              pLayer->TintColor   = memIn.ReadU32();
              pLayer->Visibility  = (Layer::LayerVisibility)memIn.ReadU32();

              pLayer->m_pMap = this;

              pLayer->Resize( Width, Height );

              Layers.push_back( pLayer );
            }
            break;
          case GR::Gamebase::FileChunks::LAYER_OBJECTS:
            // Objects
            {
              ObjectLayer*        pObjLayer = (ObjectLayer*)Layers.back();

              GR::IO::FileChunk   objLayerSubChunk;

              while ( objLayerSubChunk.Read( memIn ) )
              {
                MemoryStream      memInObjLayer = objLayerSubChunk.GetMemoryStream();

                switch ( objLayerSubChunk.Type() )
                {
                  case GR::Gamebase::FileChunks::LAYER_OBJECT:
                    {
                      GR::String objTemplate  = memInObjLayer.ReadString();
                      int posX                = memInObjLayer.ReadI32();
                      int posY                = memInObjLayer.ReadI32();
                      GR::u32 flags           = memInObjLayer.ReadU32();

                      if ( m_AddObjectHandler )
                      {
                        bool    addAtBack = false;
                        LayerObject* pObj = m_AddObjectHandler( objTemplate, posX, posY, flags, addAtBack );
                        if ( pObj )
                        {
                          pObj->Template = objTemplate;
                          if ( addAtBack )
                          {
                            pObjLayer->AddObjectOnTop( pObj );
                          }
                          else
                          {
                            pObjLayer->AddObject( pObj );
                          }
                          pObj->Flags |= flags;

                          memInObjLayer.ReadF32();    // angle
                          memInObjLayer.ReadF32();    // scalex
                          memInObjLayer.ReadF32();    // scaley
                          pObj->ExtraDataID = memInObjLayer.ReadU32();
                          pObj->ID          = memInObjLayer.ReadU32();
                        }
                      }
                      else
                      {
                        dh::Log( "LayeredMap: No Object Handler set, no objects will be created!" );
                      }
                    }
                    break;
                }
              }
            }
            break;
        }
      }
      return true;
    }



    void LayeredMap::SetAddObjectHandler( tAddObjectHandlerFunction Function )
    {
      m_AddObjectHandler = Function;
    }



    void LayeredMap::SetEventHandler( tLayeredMapEventFunction Function )
    {
      m_EventHandler = Function;
    }



    Layer* LayeredMap::LayerByName( const GR::String& Layername )
    {
      std::vector<Layer*>::iterator   itL( Layers.begin() );
      while ( itL != Layers.end() )
      {
        Layer*    pLayer( *itL );

        if ( pLayer->Name == Layername )
        {
          return pLayer;
        }
        ++itL;
      }
      return NULL;
    }



    LayerObject* LayeredMap::AddObject( const GR::String& Layername, const GR::String& ObjType, int X, int Y, GR::u32 Flags )
    {
      LayerObject* pObj = NULL;

      if ( m_AddObjectHandler )
      {
        bool    dummy = false;
        pObj = m_AddObjectHandler( ObjType, X, Y, Flags, dummy );
      }
      else
      {
        pObj = new LayerObject();
        pObj->Position.set( X, Y );
        pObj->Flags = Flags;
      }
      if ( pObj )
      {
        pObj->Template = ObjType;

        ObjectLayer* pLayer = (ObjectLayer*)LayerByName( Layername );
        if ( pLayer )
        {
          pLayer->AddObject( pObj );
        }
        else
        {
          delete pObj;
          pObj = NULL;
        }
      }
      return pObj;
    }



    LayerObject* LayeredMap::AddObjectOnTop( const GR::String& Layername, const GR::String& ObjType, int X, int Y, GR::u32 Flags )
    {
      LayerObject* pObj = NULL;

      if ( m_AddObjectHandler )
      {
        bool    dummy = false;
        pObj = m_AddObjectHandler( ObjType, X, Y, Flags, dummy );
      }
      else
      {
        pObj = new LayerObject();
        pObj->Position.set( X, Y );
        pObj->Flags = Flags;
      }
      if ( pObj )
      {
        pObj->Template = ObjType;

        ObjectLayer* pLayer = (ObjectLayer*)LayerByName( Layername );
        if ( pLayer )
        {
          pLayer->AddObjectOnTop( pObj );
        }
        else
        {
          delete pObj;
          pObj = NULL;
        }
      }
      return pObj;
    }



    LayerObject* LayeredMap::FindObjectByType( const GR::String& Layername, const GR::String& ObjType )
    {
      std::vector<Layer*>::iterator   itL( Layers.begin() );
      while ( itL != Layers.end() )
      {
        Layer*    pLayer( *itL );

        if ( pLayer->Name == Layername )
        {
          if ( ( pLayer->Type == Layer::LT_TILE_LAYER )
          ||   ( pLayer->Type == Layer::LT_FREE_SECTION_LAYER ) )
          {
            ObjectLayer*    pObjLayer = (ObjectLayer*)pLayer;

            return pObjLayer->FindObjectByType( ObjType );
          }
        }
        ++itL;
      }
      return NULL;
    }



    LayerObject* LayeredMap::FindObjectByType( const GR::String& ObjType )
    {
      std::vector<Layer*>::iterator   itL( Layers.begin() );
      while ( itL != Layers.end() )
      {
        Layer*    pLayer( *itL );

        if ( ( pLayer->Type == Layer::LT_TILE_LAYER )
        ||   ( pLayer->Type == Layer::LT_FREE_SECTION_LAYER ) )
        {
          ObjectLayer*    pObjLayer = (ObjectLayer*)pLayer;

          return pObjLayer->FindObjectByType( ObjType );
        }
        ++itL;
      }
      return NULL;
    }



    void LayeredMap::AddRegion( const GR::tRect& Region )
    {
      GR::tPoint    gridKey( Region.Left / 1024, Region.Top / 1024 );

      int     x1 = Region.Left / 1024;
      int     y1 = Region.Top / 1024;
      int     x2 = Region.Right / 1024;
      int     y2 = Region.Bottom / 1024;

      for ( int x = x1; x <= x2; ++x )
      {
        for ( int y = y1; y <= y2; ++y )
        {
          Regions[GR::tPoint( x, y )].push_back( Region );
        }
      }
    }



    void LayeredMap::AddTrigger( const Trigger& Trigger )
    {
      GR::tPoint    gridKey( Trigger.Rect.Left / 1024, Trigger.Rect.Top / 1024 );

      TriggerGrid[gridKey].insert( Trigger.ID );
      Triggers[Trigger.ID] = Trigger;
    }



    bool LayeredMap::ActualiseRegion( const GR::tRect& Bounds )
    {
      if ( CurrentRegion.intersects( Bounds ) )
      {
        return true;
      }

      int X1 = Bounds.Left / 1024;
      int Y1 = Bounds.Top / 1024;
      int X2 = Bounds.Right / 1024;
      int Y2 = Bounds.Bottom / 1024;

      for ( int x = X1; x <= X2; ++x )
      {
        for ( int y = Y1; y <= Y2; ++y )
        {
          GR::tPoint    gridKey( x, y );

          tRegion::iterator    it( Regions.find( gridKey ) );
          if ( it == Regions.end() )
          {
            continue;
          }
          std::list<GR::tRect>::iterator    itR( it->second.begin() );
          while ( itR != it->second.end() )
          {
            if ( itR->intersects( Bounds ) )
            {
              CurrentRegion = *itR;
              return true;
            }

            ++itR;
          }
        }
      }
      CurrentRegion = GR::tRect();
      return false;
    }



    bool LayeredMap::ActualiseRegion( const GR::tPoint& Pos )
    {
      if ( CurrentRegion.contains( Pos ) )
      {
        return true;
      }

      int x = Pos.x / 1024;
      int y = Pos.y / 1024;

      GR::tPoint    gridKey( x, y );

      tRegion::iterator    it( Regions.find( gridKey ) );
      if ( it == Regions.end() )
      {
        CurrentRegion = GR::tRect();
        return false;
      }
      std::list<GR::tRect>::iterator    itR( it->second.begin() );
      while ( itR != it->second.end() )
      {
        if ( itR->contains( Pos ) )
        {
          CurrentRegion = *itR;
          return true;
        }

        ++itR;
      }
      CurrentRegion = GR::tRect();
      return false;
    }



    bool LayeredMap::RaiseEvent( const LayeredMapEvent& Event )
    {
      if ( m_EventHandler )
      {
        if ( m_EventHandler( Event ) )
        {
          // event was handled
          return true;
        }
      }
      // event was not handled
      return false;
    }



    bool LayeredMap::CheckTrigger( LayerObject* pObject )
    {
      if ( pObject == NULL )
      {
        return false;
      }
      GR::tRect   bounds( pObject->Bounds() );

      // still inside current trigger?
      if ( pObject->CurrentTrigger != -1 )
      {
        if ( Triggers[pObject->CurrentTrigger].Rect.intersects( bounds ) )
        {
          RaiseEvent( LayeredMapEvent( LayeredMapEvent::ET_TRIGGER_INSIDE, pObject ) );
          return true;
        }
        RaiseEvent( LayeredMapEvent( LayeredMapEvent::ET_TRIGGER_LEAVE, pObject ) );
        pObject->CurrentTrigger = -1;
      }

      int X1 = bounds.Left / 1024;
      int Y1 = bounds.Top / 1024;
      int X2 = bounds.Right / 1024;
      int Y2 = bounds.Bottom / 1024;

      for ( int x = X1; x <= X2; ++x )
      {
        for ( int y = Y1; y <= Y2; ++y )
        {
          GR::tPoint    gridKey( x, y );

          tTriggerGrid::iterator    it( TriggerGrid.find( gridKey ) );
          if ( it == TriggerGrid.end() )
          {
            continue;
          }
          std::set<GR::u32>::iterator    itT( it->second.begin() );
          while ( itT != it->second.end() )
          {
            if ( Triggers[*itT].Rect.intersects( bounds ) )
            {
              pObject->CurrentTrigger = *itT;
              RaiseEvent( LayeredMapEvent( LayeredMapEvent::ET_TRIGGER_ENTER, pObject ) );
              return true;
            }

            ++itT;
          }
        }
      }
      return false;
    }



    GR::Game::ExtraData* LayeredMap::AddExtraData()
    {
      GR::u32     extraDataID = 1;

      while ( ExtraDatas.find( extraDataID ) != ExtraDatas.end() )
      {
        ++extraDataID;
      }
      ExtraDatas[extraDataID].ID = extraDataID;

      return &ExtraDatas[extraDataID];
    }



    void LayeredMap::SetExtraData( GR::u32 ID, const GR::Game::ExtraData& Data )
    {
      ExtraDatas[ID] = Data;
    }



    GR::Game::ExtraData* LayeredMap::FindExtraData( GR::u32 ExtraDataID )
    {
      auto    itED( ExtraDatas.find( ExtraDataID ) );
      if ( itED != ExtraDatas.end() )
      {
        return &itED->second;
      }
      return NULL;
    }



    LayerObject* LayeredMap::FindObjectByID( GR::u32 ObjectID, ObjectLayer* pLayer )
    {
      if ( pLayer != NULL )
      {
        return pLayer->FindObjectByID( ObjectID );
      }
      std::vector<Layer*>::iterator   itL( Layers.begin() );
      while ( itL != Layers.end() )
      {
        ObjectLayer*    pLayer = (ObjectLayer*)( *itL );

        LayerObject*    pObj = pLayer->FindObjectByID( ObjectID );
        if ( pObj != NULL )
        {
          return pObj;
        }

        ++itL;
      }
      return NULL;
    }

  }
}



