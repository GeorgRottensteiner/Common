#include "JREngine.h"
#include "LayeredMap.h"
#include "ObjectLayer.h"
#include "GameObject.h"
#include "ScriptHandler.h"

#include <Xtreme/XInput.h>
#include <Xtreme/XAsset/XAssetTileset.h>
#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetLayeredMap.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>

#include <String/Path.h>

#include <IO/FileLocator.h>
#include <IO/FileStream.h>

#include <Grafik/ContextDescriptor.h>



namespace GR
{
  namespace Gamebase
  {
    Dir DirFromDelta( const GR::tPoint& Delta )
    {
      return DirFromDelta( Delta.x, Delta.y );
    }



    GR::f32 AngleFromDir( Dir Dir )
    {
      switch ( Dir )
      {
        case Dir::N | Dir::W:
          return 45.0f;
        case Dir::N | Dir::E:
          return 315.0f;
        case Dir::S | Dir::W:
          return 135.0f;
        case Dir::S | Dir::E:
          return 225.0f;
        case Dir::N:
          return 0.0f;
        case Dir::S:
          return 180.0f;
        case Dir::W:
          return 90.0f;
        case Dir::E:
          return 270.0f;
      }
      return 0.0f;
    }



    GR::tPoint DeltaFromDir( GR::u32 Dir )
    {
      switch ( Dir )
      {
        case Dir::N | Dir::W:
          return GR::tPoint( -1, -1 );
        case Dir::N | Dir::E:
          return GR::tPoint( 1, -1 );
        case Dir::S | Dir::W:
          return GR::tPoint( -1, 1 );
        case Dir::S | Dir::E:
          return GR::tPoint( 1, 1 );
        case Dir::N:
          return GR::tPoint( 0, -1 );
        case Dir::S:
          return GR::tPoint( 0, 1 );
        case Dir::W:
          return GR::tPoint( -1, 0 );
        case Dir::E:
          return GR::tPoint( 1, 0 );
      }
      return GR::tPoint();
    }



    Dir DirFromDelta( int DX, int DY )
    {
      Dir   resultDir = Dir::NONE;
      if ( DX < 0 )
      {
        resultDir = (Dir)( resultDir | Dir::W );
      }
      if ( DX > 0 )
      {
        resultDir = (Dir)( resultDir | Dir::E );
      }
      if ( DY < 0 )
      {
        resultDir = (Dir)( resultDir | Dir::N );
      }
      if ( DY > 0 )
      {
        resultDir = (Dir)( resultDir | Dir::S );
      }
      return resultDir;
    }



    Dir DirFromDelta( GR::f32 DX, GR::f32 DY )
    {
      Dir resultDir = Dir::NONE;
      if ( DX < 0 )
      {
        resultDir = (Dir)( resultDir | Dir::W );
      }
      if ( DX > 0 )
      {
        resultDir = (Dir)( resultDir | Dir::E );
      }
      if ( DY < 0 )
      {
        resultDir = (Dir)( resultDir | Dir::N );
      }
      if ( DY > 0 )
      {
        resultDir = (Dir)( resultDir | Dir::S );
      }
      return resultDir;
    }



    static JREngine*    s_pJREngine = NULL;



    JREngine::JREngine( const GR::tPoint& RenderSize, IAnimationManager<GR::String>* pAnimManager ) :
      m_pControlledObject( NULL ),
      m_pGameLayer( NULL ),
      m_pFlagLayer( NULL ),
      m_pCollisionLayer( NULL ),
      m_RenderSize( RenderSize ),
      m_pAnimManager( pAnimManager ),
      m_AllObjectsActive( false ),
      m_MovieMode( false ),
      m_CameraMode( CAM_FIXED ),
      m_CameraMoveTime( 0.0f ),
      m_CameraMoveDelay( 0.0f ),
      m_ObjectUserData( 0 ),
      m_MapChanged( false ),
      m_GravityFactor( 1.0f ),
      m_SinglePixelWidthBoundsForMovement( true ),
      m_BlockMovingOutsideMap( false )
    {
      s_pJREngine = this;

      Loona<ScriptEngine>::Register( m_ScriptHandler );

      Loona<ScriptEngine>::SetVar( m_ScriptHandler, new ScriptEngine( this ), "JREngine" );
      Loona<ScriptMap>::SetVar( m_ScriptHandler, new ScriptMap( this, &m_Map ), "Map" );

      RegisterScript( &JREngine::MovieStart, "MovieStart" );
      RegisterScript( &JREngine::MovieEnd, "MovieEnd" );
      RegisterScript( &JREngine::CameraMoveTo, "CameraMoveTo" );
      RegisterScript( &JREngine::CameraFollowControlledObject, "CameraFollowControlledObject" );
      RegisterScript( &JREngine::GetCurrentCameraPosition, "GetCurrentCameraPosition" );
      RegisterScript( &JREngine::SetFeld, "SetFeld" );
      RegisterScript( &JREngine::HidePlayer, "HidePlayer" );
      RegisterScript( &JREngine::ShowPlayer, "ShowPlayer" );
      RegisterScript( &JREngine::FreezePlayer, "FreezePlayer" );
      RegisterScript( &JREngine::UnfreezePlayer, "UnfreezePlayer" );
      RegisterScript( &JREngine::SetGameVar, "SetGameVar" );
      RegisterScript( &JREngine::GetGameVar, "GetGameVar" );

      m_Map.SetEventHandler( fastdelegate::MakeDelegate( this, &JREngine::OnLayeredMapEvent ) );
    }



    JREngine::~JREngine()
    {
      s_pJREngine = NULL;
      ClearObjects();
    }



    void JREngine::ClearMap()
    {
      ClearObjects();
      m_Map.Clear();
      m_AwakeSectors.clear();

      m_pGameLayer = NULL;
      m_pFlagLayer = NULL;
      m_pCollisionLayer = NULL;
    }



    GR::String JREngine::MapPath( const GR::String& MapName )
    {
      Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader != NULL )
      {
        Xtreme::Asset::XAssetLayeredMap* pMap = (Xtreme::Asset::XAssetLayeredMap*)pLoader->Asset( Xtreme::Asset::XA_LAYERED_MAP, MapName.c_str() );
        if ( pMap != NULL )
        {
          return GR::IO::FileLocator::Instance().LocateFile( pMap->MapFile().c_str() );
        }
      }
      dh::Log( "JREngine::MapPath unknown map %s", MapName.c_str() );
      return GR::String();
    }



    void JREngine::DetermineFlagLayer()
    {
      for ( size_t i = 0; i < m_Map.Layers.size(); ++i )
      {
        if ( m_Map.Layers[i]->Type == Layer::LT_FLAG_LAYER )
        {
          m_pFlagLayer = (FlagLayer*)m_Map.Layers[i];
          break;
        }
      }
    }



    void JREngine::InitObjectExtraData( GR::Gamebase::GameObject& Object )
    {
      auto pExtraData = m_Map.FindExtraData( Object.ExtraDataID );
      if ( ( pExtraData != NULL )
      &&   ( pExtraData->Type == GR::Game::ExtraDataType::PATH ) )
      {
        if ( m_Map.MovementPaths.find( pExtraData->Param ) == m_Map.MovementPaths.end() )
        {
          dh::Log( "Trying to set unknown path (%s)", pExtraData->Param.c_str() );
        }
        else
        {
          Object.m_MovementPathPos  = GR::Game::Path::tPathPos( &m_Map.MovementPaths[pExtraData->Param] );
          Object.Flags              |= GR::Gamebase::GameObject::OF_HAS_MOVEMENT_PATH;
        }
      }
    }



    void JREngine::InitMap( const GR::String& MapName, const GR::String& MainLayerName, const GR::String& PlayerObject )
    {
      InitMapFromFile( MapPath( MapName ), MapName, MainLayerName, PlayerObject );
    }



    void JREngine::InitMapFromFile( const GR::String& MapNameFilename, const GR::String& MapName, const GR::String& MainLayerName, const GR::String& PlayerObject )
    {
      if ( !m_CurrentMap.empty() )
      {
        RaiseJREvent( JREvent( JREvent::JRE_LEAVE_MAP, m_CurrentMap ) );
      }

      ClearMap();

      GR::IO::FileStream    inFile( MapNameFilename );
      if ( !m_Map.Load( inFile ) )
      {
        dh::Log( "Map Load failed for (%s)", MapNameFilename.c_str() );
        return;
      }

      m_MapChanged        = true;
      m_CurrentMap        = MapName;
      m_MainLayer         = MainLayerName;
      m_ControlObjectName = PlayerObject;
      m_pControlledObject = (GameObject*)m_Map.FindObjectByType( m_MainLayer, m_ControlObjectName );
      m_pGameLayer        = (GR::Gamebase::TileLayer*)m_Map.LayerByName( m_MainLayer );
      m_pCollisionLayer   = (GR::Gamebase::TileLayer*)m_Map.LayerByName( "Collision" );

      m_ScriptHandler.DoString( "mainLayer = Map:GetLayer( \"" + m_MainLayer + "\" )" );
      m_ScriptHandler.DoString( "fgLayer = Map:GetLayer( \"Foreground\" )" );

      std::vector<Layer*>::iterator   itL( m_Map.Layers.begin() );
      while ( itL != m_Map.Layers.end() )
      {
        GR::Gamebase::ObjectLayer*    pLayer( ( GR::Gamebase::ObjectLayer*)*itL );

        std::map<GR::tPoint, std::list<LayerObject*> >::iterator   itGrid( pLayer->Objects.begin() );
        while ( itGrid != pLayer->Objects.end() )
        {
          std::list<LayerObject*>&    listObj( itGrid->second );

          std::list<LayerObject*>::iterator   itO( listObj.begin() );
          while ( itO != listObj.end() )
          {
            GR::Gamebase::GameObject*    pGridObj( ( GR::Gamebase::GameObject*)*itO );

            RaiseObjectEvent( pGridObj, GR::Gamebase::ObjectEventType::CREATED );
            InitObjectExtraData( *pGridObj );

            ++itO;
          }
          ++itGrid;
        }
        ++itL;
      }

      m_TileFlags.clear();
      if ( m_pGameLayer )
      {
        DetermineFlagLayer();
        Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
        if ( pLoader != NULL )
        {
          Xtreme::Asset::XAssetTileset* pTileset = (Xtreme::Asset::XAssetTileset*)pLoader->Asset( Xtreme::Asset::XA_TILESET, m_pGameLayer->Tileset.c_str() );
          if ( pTileset != NULL )
          {
            for ( size_t i = 0; i < pTileset->m_Tiles.size(); ++i )
            {
              m_TileFlags.push_back( pTileset->Flags( i ) );
            }
          }
        }
      }
      if ( m_pControlledObject )
      {
        CenterCameraOnObject( m_pControlledObject );

        if ( m_pGameLayer )
        {
          GR::tPoint    objGridPos( m_pGameLayer->GridPos( m_pControlledObject ) );

          for ( int i = -1; i <= 1; ++i )
          {
            for ( int j = -1; j <= 1; ++j )
            {
              AwakenObjects( objGridPos + GR::tPoint( i, j ) );
              m_AwakeSectors.insert( objGridPos + GR::tPoint( i, j ) );
            }
          }
        }
        // force initial collision
        CheckCollisions( m_pControlledObject, m_pControlledObject->Bounds(), GR::Gamebase::Dir::NONE );
      }
      RaiseJREvent( JREvent( JREvent::JRE_MAP_CHANGED, MapName ) );
      RunMapScript( "OnLoad" );
    }



    void JREngine::SetRenderSize( const GR::tPoint& Size )
    {
      m_RenderSize = Size;
    }



    void JREngine::ClipToRect( GR::tPoint& Pos, const GR::tRect& ClipRect )
    {
      if ( Pos.x >= ClipRect.Right - m_RenderSize.x )
      {
        Pos.x = ClipRect.Right - m_RenderSize.x;
      }
      if ( Pos.y >= ClipRect.Bottom - m_RenderSize.y )
      {
        Pos.y = ClipRect.Bottom - m_RenderSize.y;
      }
      if ( Pos.x < ClipRect.Left )
      {
        Pos.x = ClipRect.Left;
      }
      if ( Pos.y < ClipRect.Top )
      {
        Pos.y = ClipRect.Top;
      }
    }



    void JREngine::CenterCameraOnPosition( const GR::tPoint& Pos )
    {
      GR::tPoint      screenOffset;
      screenOffset.x = Pos.x - m_RenderSize.x / 2;
      screenOffset.y = Pos.y - m_RenderSize.y / 2;
      m_Map.ActualiseRegion( Pos );
      if ( m_Map.CurrentRegion.Width() )
      {
        ClipToRect( screenOffset, m_Map.CurrentRegion );
      }
      else
      {
        ClipToRect( screenOffset, GR::tRect( 0, 0, 
                                             m_pGameLayer->Width() * (int)m_pGameLayer->TileSpacingX,
                                             m_pGameLayer->Height() * (int)m_pGameLayer->TileSpacingY ) );
      }
      m_CurrentOffsetTarget = screenOffset;
      m_CurrentOffset       = screenOffset;  
      m_Map.DisplayOffset   = screenOffset;
    }



    void JREngine::CenterCameraOnObject( GameObject* pObj )
    {
      CenterCameraOnPosition( pObj->Position );
    }



    void JREngine::ClearObjects()
    {
      m_ObjectEventHandler.clear();
      std::list<GameObject*>::iterator    itAO( m_AwakeObjects.begin() );
      while ( itAO != m_AwakeObjects.end() )
      {
        GameObject*   pObj = (GameObject*)*itAO;

        if ( pObj->m_pSpawnAncestor != NULL )
        {
          delete pObj;
        }

        ++itAO;
      }
      m_AwakeObjects.clear();

      if ( m_pGameLayer )
      {
        std::map<GR::tPoint,std::list<LayerObject*> >::iterator   itGrid( m_pGameLayer->Objects.begin() );
        while ( itGrid != m_pGameLayer->Objects.end() )
        {
          std::list<LayerObject*>&    listObj( itGrid->second );

          std::list<LayerObject*>::iterator   itO( listObj.begin() );
          while ( itO != listObj.end() )
          {
            GameObject*    pLayerObj( (GameObject*)*itO );

            delete pLayerObj;
            ++itO;
          }
          ++itGrid;
        }
        m_pGameLayer->Objects.clear();
      }
    }



    void JREngine::AddObjectToGrid( GameObject* pObj )
    {
      if ( m_pGameLayer != NULL )
      {
        m_pGameLayer->AddObject( pObj );
      }
      if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::AWAKENED )
      {
        m_AwakeObjects.remove( pObj );
        m_AwakeObjects.push_back( pObj );
      }
    }



    void JREngine::RemoveObjectFromGrid( GameObject* pObj )
    {
      //dh::Log( "Remove object from grid (%s/%x)", pObj->Template.c_str(), pObj );
      if ( m_pGameLayer != NULL )
      {
        m_pGameLayer->RemoveObject( pObj );
      }
    }

    

    void JREngine::RemoveObject( GameObject* pObj )
    {
      //dh::Log( "Remove object (%s/%x)", pObj->Template.c_str(), pObj );
      if ( m_pGameLayer != NULL )
      {
        m_pGameLayer->RemoveObject( pObj );
      }
      m_AwakeObjects.remove( pObj );
    }



    GameObject* JREngine::SpawnObject( const GR::String& ObjType, int X, int Y, GR::u32 Flags, bool Sleeping )
    {
      GameObject* pObj = NULL;

      bool    addAtBack = false;
      if ( m_AddObjectHandler )
      {
        pObj = (GR::Gamebase::GameObject*)m_AddObjectHandler( ObjType, X, Y, Flags, addAtBack );
      }
      else
      {
        pObj = new GameObject();
        pObj->Position.Set( X, Y );
        pObj->Flags = Flags;
      }
      if ( pObj )
      {
        pObj->m_pEngine = this;
        pObj->Template  = ObjType;

        RaiseObjectEvent( pObj, GR::Gamebase::ObjectEventType::CREATED );

        if ( ( Sleeping )
        &&   ( !( pObj->Flags & GR::Gamebase::LayerObject::OF_ALWAYS_AWAKE ) ) )
        {
          AddObjectToGrid( pObj );
        }
        else
        {
          pObj->m_ProcessingFlags |= ProcessingFlags::AWAKENED;

          bool    inserted = false;
          if ( pObj->ZOrder != 0 )
          {
            auto    itO( m_AwakeObjects.begin() );
            while ( itO != m_AwakeObjects.end() )
            {
              GR::Gamebase::GameObject* pOrigObj( *itO );
              if ( pOrigObj->ZOrder > pObj->ZOrder )
              {
                m_AwakeObjects.insert( itO, pObj );
                inserted = true;
                break;
              }
              ++itO;
            }
          }
          if ( !inserted )
          {
            if ( addAtBack )
            {
              m_AwakeObjects.push_front( pObj );
            }
            else
            {
              m_AwakeObjects.push_back( pObj );
            }
          }
          RaiseObjectEvent( pObj, GR::Gamebase::ObjectEventType::AWAKEN );
        }
        InitObjectExtraData( *pObj );
      }
      return pObj;
    }



    GR::u32 JREngine::MoveObject( GameObject* pObj, GR::f32 DX, GR::f32 DY, GR::Gamebase::Layer* pLayer, GameObject* pMovingParent )
    {
      GR::u32       moveResult = 0;

      pObj->FractPos.Offset( DX, DY );

      GR::tRect     objBounds( pObj->Bounds() );
      GR::tPoint    oldGridPos = m_pGameLayer->GridPos( pObj );

      GR::tPoint    movedDelta;
      bool          axisXBlocked = false;
      bool          axisYBlocked = false;

      if ( m_SinglePixelWidthBoundsForMovement )
      {
        // make width 1 pixel
        objBounds.Left = objBounds.Left + objBounds.Width() / 2;
        objBounds.Right = objBounds.Left + 1;
      }


      while ( ( pObj->FractPos.x >= 1.0f )
      ||      ( pObj->FractPos.x < 0.0f )
      ||      ( pObj->FractPos.y >= 1.0f )
      ||      ( pObj->FractPos.y < 0.0f ) )
      {
        axisXBlocked = false;
        axisYBlocked = false;
        if ( pObj->FractPos.x >= 1.0f )
        {
          if ( ( IsAreaBlocked( pObj, GR::tRect( objBounds.Right, objBounds.Top, 1, objBounds.Height() ), Dir::R, pLayer, pMovingParent ) )
          ||   ( IsObjectBlockedByOtherObjects( pObj, objBounds, Dir::R, pMovingParent ) ) )
          {
            axisXBlocked = true;
            moveResult |= MoveResult::BLOCKED_RIGHT;

            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::BLOCKED_RIGHT ) );
          }
          else
          {
            objBounds.Offset( 1, 0 );
            PerformMove( pObj, 1, 0 );
            pObj->FractPos.x -= 1.0f;
            ++movedDelta.x;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_R ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }
        if ( pObj->FractPos.x < 0.0f )
        {
          if ( ( IsAreaBlocked( pObj, GR::tRect( objBounds.Left - 1, objBounds.Top, 1, objBounds.Height() ), Dir::L, pLayer, pMovingParent ) )
          ||   ( IsObjectBlockedByOtherObjects( pObj, objBounds, Dir::L, pMovingParent ) ) )
          {
            axisXBlocked = true;
            moveResult |= MoveResult::BLOCKED_LEFT;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::BLOCKED_LEFT ) );
          }
          else
          {
            objBounds.Offset( -1, 0 );
            PerformMove( pObj, -1, 0 );
            pObj->FractPos.x += 1.0f;
            --movedDelta.x;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_L ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }
        if ( pObj->FractPos.y >= 1.0f )
        {
          GR::tRect   downBounds( objBounds.Left, objBounds.Bottom, objBounds.Width(), 1 );
          if ( ( IsAreaBlocked( pObj, downBounds, Dir::D, pLayer, pMovingParent ) )
          ||   ( IsObjectBlockedByOtherObjects( pObj, downBounds, Dir::D, pMovingParent ) ) )
          {
            axisYBlocked = true;
            moveResult |= MoveResult::BLOCKED_DOWN;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::BLOCKED_S ) );
          }
          else
          {
            objBounds.Offset( 0, 1 );
            PerformMove( pObj, 0, 1 );
            pObj->FractPos.y -= 1.0f;
            ++movedDelta.y;

            if ( ( !pObj->IsJumping() )
            &&   ( !pObj->IsFloating() )
            &&   ( !pObj->IsOnGround() ) )
            {
              ++pObj->FallHeight;
            }
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_D ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }
        if ( pObj->FractPos.y < 0.0f )
        {
          if ( ( IsAreaBlocked( pObj, GR::tRect( objBounds.Left, objBounds.Top - 1, objBounds.Width(), 1 ), Dir::U, pLayer, pMovingParent ) )
          ||   ( IsObjectBlockedByOtherObjects( pObj, objBounds, Dir::U, pMovingParent ) ) )
          {
            axisYBlocked = true;
            moveResult |= MoveResult::BLOCKED_UP;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::BLOCKED_N ) );
          }
          else
          {
            objBounds.Offset( 0, -1 );
            PerformMove( pObj, 0, -1 );
            pObj->FractPos.y += 1.0f;
            --movedDelta.y;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_U ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }
        if ( ( ( pObj->FractPos.x >= 1.0f )
        ||     ( pObj->FractPos.x < 0.0f ) )
        &&   ( ( pObj->FractPos.y >= 1.0f )
        ||     ( pObj->FractPos.y < 0.0f ) ) )
        {
          // tried to move in both dirs, both dirs blocked
          if ( ( axisXBlocked )
          &&   ( axisYBlocked ) )
          {
            pObj->FractPos.x = 0.0f;
            pObj->FractPos.y = 0.0f;
          }
        }
        else if ( axisXBlocked )
        {
          pObj->FractPos.x = 0.0f;
        }
        else if ( axisYBlocked )
        {
          pObj->FractPos.y = 0.0f;
        }
      }
      pObj->m_ProcessingFlags &= ~GR::Gamebase::ProcessingFlags::ABORT_MOVE;
      if ( ( movedDelta.x != 0 )
      ||   ( movedDelta.y != 0 ) )
      {
        //RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::MOVED, movedDelta ) );
        /*
        if ( IsOutsideAwakeArea( pObj ) )
        {
          SleepObject( pObj );
        }*/
      }

      GR::tPoint    newGridPos = m_pGameLayer->GridPos( pObj );
      if ( oldGridPos != newGridPos )
      {
        m_pGameLayer->RemoveObject( oldGridPos, pObj );
        m_pGameLayer->AddObject( pObj );
      }

      return moveResult;
    }



    bool JREngine::IsOutsideAwakeArea( GR::Gamebase::GameObject* pObj )
    {
      GR::tPoint      objSector( m_pGameLayer->GridPos( pObj ) );

      return ( m_AwakeSectors.find( objSector ) == m_AwakeSectors.end() );
    }



    GR::u32 JREngine::MoveObjectNonBlocking( GameObject* pObj, GR::f32 DX, GR::f32 DY )
    {
      GR::u32       moveResult = 0;

      pObj->FractPos.Offset( DX, DY );

      GR::tRect     objBounds( pObj->Bounds() );

      GR::tPoint    movedDelta;


      while ( ( pObj->FractPos.x >= 1.0f )
      ||      ( pObj->FractPos.x < 0.0f )
      ||      ( pObj->FractPos.y >= 1.0f )
      ||      ( pObj->FractPos.y < 0.0f ) )
      {
        if ( pObj->FractPos.x >= 1.0f )
        {
          objBounds.Offset( 1, 0 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( 1, 0 ) );
          pObj->FractPos.x -= 1.0f;
          ++movedDelta.x;
        }
        if ( pObj->FractPos.x < 0.0f )
        {
          objBounds.Offset( -1, 0 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( -1, 0 ) );
          pObj->FractPos.x += 1.0f;
          --movedDelta.x;
        }
        if ( pObj->FractPos.y >= 1.0f )
        {
          objBounds.Offset( 0, 1 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( 0, 1 ) );
          pObj->FractPos.y -= 1.0f;
          ++movedDelta.y;
        }
        if ( pObj->FractPos.y < 0.0f )
        {
          objBounds.Offset( 0, -1 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( 0, -1 ) );
          pObj->FractPos.y += 1.0f;
          --movedDelta.x;
        }
      }
      if ( ( movedDelta.x != 0 )
      ||   ( movedDelta.y != 0 ) )
      {
        CheckCollisions( pObj, pObj->Bounds(), DirFromDelta( DX, DY ) );

        RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::MOVED, movedDelta ) );
      }
      return moveResult;
    }



    GR::u32 JREngine::MoveObjectNonBlockingFixed( GameObject* pObj, GR::i32 DX, GR::i32 DY )
    {
      GR::u32       moveResult = 0;

      GR::tRect     objBounds( pObj->Bounds() );

      GR::tPoint    movedDelta;


      while ( ( DX >= 1 )
      ||      ( DX < 0 )
      ||      ( DY >= 1 )
      ||      ( DY < 0 ) )
      {
        if ( DX >= 1 )
        {
          objBounds.Offset( 1, 0 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( 1, 0 ) );
          --DX;
          ++movedDelta.x;
        }
        if ( DX < 0 )
        {
          objBounds.Offset( -1, 0 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( -1, 0 ) );
          ++DX;
          --movedDelta.x;
        }
        if ( DY >= 1 )
        {
          objBounds.Offset( 0, 1 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( 0, 1 ) );
          --DY;
          ++movedDelta.y;
        }
        if ( DY < 0 )
        {
          objBounds.Offset( 0, -1 );
          m_pGameLayer->MoveObject( pObj, GR::tPoint( 0, -1 ) );
          ++DY;
          --movedDelta.x;
        }
      }
      if ( ( movedDelta.x != 0 )
      ||   ( movedDelta.y != 0 ) )
      {
        RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::MOVED, movedDelta ) );
      }
      return moveResult;
    }



    void JREngine::RaiseObjectEvent( GameObject* pObj, const ObjectEvent& Event )
    {
      tObjectEventHandler::iterator   it( m_ObjectEventHandler.find( std::make_pair( Event.Type, pObj ) ) );
      if ( it != m_ObjectEventHandler.end() )
      {
        it->second( pObj, Event );
      }
      pObj->OnObjectEvent( *this, Event );
    }



    void JREngine::RaiseJREvent( const JREvent& Event )
    {
      if ( m_JREventHandler )
      {
        m_JREventHandler( Event );
      }
    }



    bool JREngine::QueryEvent( const GR::Gamebase::QueryEvent& Event )
    {
      if ( m_QueryHandler )
      {
        return m_QueryHandler( Event );
      }
      return false;
    }



    void JREngine::PerformMove( GameObject* pObj, int DX, int DY )
    {
      if ( !( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::AWAKENED ) )
      {
        m_pGameLayer->MoveObject( pObj, GR::tPoint( DX, DY ) );
      }
      else
      {
        pObj->Position.Offset( DX, DY );
      }

      RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::MOVED, GR::tPoint( DX, DY ) ) );

      std::list<GameObject*>::iterator    itO( pObj->m_ObjectsCarried.begin() );
      while ( itO != pObj->m_ObjectsCarried.end() )
      {
        GameObject*   pObjToCheck = *itO;
        ++itO;
        MoveObjectFixed( pObjToCheck, DX, DY, NULL, pObj );
      }

      // check if the carrier object is still below
      if ( pObj->m_pObjectBelow )
      {
        GR::tRect   bounds( pObj->Bounds() );

        bounds = GR::tRect( bounds.Left, bounds.Bottom, bounds.Width(), 1 );

        // the call changes the carrier!
        if ( !IsObjectBlockedByOtherObjects( pObj, bounds, GR::Gamebase::Dir::D ) )
        {
        }
      }
    }



    void JREngine::SetObjectPos( GameObject* pObj, const GR::tPoint& NewPos )
    {
      if ( m_pGameLayer == NULL )
      {
        dh::Log( "JREngine::SetObjectPos with NULL layer" );
        return;
      }

      if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::AWAKENED )
      {
        pObj->Position = NewPos;
      }
      else
      {
        m_pGameLayer->SetObjectPos( pObj, NewPos );
      }
      if ( pObj == m_pControlledObject )
      {
        UpdateCamera( 0.0f );
        AwakenObjects( m_pGameLayer->GridPos( m_pControlledObject ) );
      }
    }



    GR::u32 JREngine::MoveObjectFixed( GameObject* pObj, int DX, int DY, GR::Gamebase::Layer* pLayer, GameObject* pMovingParent )
    {
      GR::tRect     objBounds( pObj->Bounds() );
      GR::u32       moveResult = MoveResult::OK;
      GR::tPoint    oldGridPos = m_pGameLayer->GridPos( pObj );
      GR::tPoint    movedDelta;
      bool          axisXBlocked = false;
      bool          axisYBlocked = false;

      // make width 1 pixel
      objBounds.Left = objBounds.Left + objBounds.Width() / 2;
      objBounds.Right = objBounds.Left + 1;

      while ( ( DX != 0 )
      ||      ( DY != 0 ) )
      {
        axisXBlocked = false;
        axisYBlocked = false;
        if ( DX >= 1 )
        {
          if ( IsAreaBlocked( pObj, GR::tRect( objBounds.Right, objBounds.Top, 1, objBounds.Height() ), Dir::R, pLayer, pMovingParent ) )
          {
            moveResult |= MoveResult::BLOCKED_RIGHT;
            axisXBlocked = true;
          }
          else
          {
            objBounds.Offset( 1, 0 );
            PerformMove( pObj, 1, 0 );
            --DX;
            ++movedDelta.x;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_R ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }
        if ( DX < 0 )
        {
          if ( IsAreaBlocked( pObj, GR::tRect( objBounds.Left - 1, objBounds.Top, 1, objBounds.Height() ), Dir::L, pLayer, pMovingParent ) )
          {
            moveResult |= MoveResult::BLOCKED_LEFT;
            axisXBlocked = true;
          }
          else
          {
            objBounds.Offset( -1, 0 );
            PerformMove( pObj, -1, 0 );
            ++DX;
            --movedDelta.x;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_L ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }
        if ( DY >= 1 )
        {
          if ( IsAreaBlocked( pObj, GR::tRect( objBounds.Left, objBounds.Bottom, objBounds.Width(), 1 ), Dir::D, pLayer, pMovingParent ) )
          {
            moveResult |= MoveResult::BLOCKED_DOWN;
            axisYBlocked = true;
          }
          else
          {
            objBounds.Offset( 0, 1 );
            PerformMove( pObj, 0, 1 );
            --DY;
            ++movedDelta.y;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_D ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }
        if ( DY < 0 )
        {
          if ( IsAreaBlocked( pObj, GR::tRect( objBounds.Left, objBounds.Top - 1, objBounds.Width(), 1 ), Dir::U, pLayer, pMovingParent ) )
          {
            moveResult |= MoveResult::BLOCKED_UP;
            axisYBlocked = true;
          }
          else
          {
            objBounds.Offset( 0, -1 );
            PerformMove( pObj, 0, -1 );
            ++DY;
            --movedDelta.y;
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::MOVED_U ) );
          }
          if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::ABORT_MOVE )
          {
            break;
          }
        }

        if ( ( DX != 0 )
        &&   ( DY != 0 ) )
        {
          // both dirs blocked
          if ( ( axisXBlocked )
          &&   ( axisYBlocked ) )
          {
            DX = 0;
            DY = 0;
          }
        }
        else if ( axisXBlocked )
        {
          DX = 0;
        }
        else if ( axisYBlocked )
        {
          DY = 0;
        }
      }

      pObj->m_ProcessingFlags &= ~GR::Gamebase::ProcessingFlags::ABORT_MOVE;


      // total move delta
      /*
      if ( ( movedDelta.x != 0 )
      ||   ( movedDelta.y != 0 ) )
      {
        RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::MOVED, movedDelta ) );
      }*/

      if ( m_MovedObjects.find( pObj ) == m_MovedObjects.end() )
      {
        m_MovedObjects[pObj] = MovedObjectInfo( pObj, oldGridPos );
      }

      GR::tPoint    newGridPos = m_pGameLayer->GridPos( pObj );
      if ( oldGridPos != newGridPos )
      {
        m_pGameLayer->RemoveObject( oldGridPos, pObj );
        m_pGameLayer->AddObject( pObj );
      }

      return moveResult;
    }



    GameObject* JREngine::FindObjectByID( GR::u32 ID )
    {
      std::list<GameObject*>::iterator    itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*   pObj( *itO );
        
        if ( pObj->ID == ID )
        {
          return pObj;
        }
        ++itO;
      }
      return (GameObject*)m_Map.FindObjectByID( ID );
    }



    GameObject* JREngine::FindObjectByType( const GR::String& ObjType )
    {
      std::list<GameObject*>::iterator    itAO( m_AwakeObjects.begin() );
      while ( itAO != m_AwakeObjects.end() )
      {
        GameObject*   pObj = (GameObject*)*itAO;

        if ( pObj->Template == ObjType )
        {
          return pObj;
        }
        ++itAO;
      }
      return (GameObject*)m_Map.FindObjectByType( ObjType );
    }



    GameObject* JREngine::FindObjectByType( const GR::String& Layer, const GR::String& ObjType )
    {
      std::list<GameObject*>::iterator    itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*   pObj( *itO );
        
        if ( pObj->Template == ObjType )
        {
          return pObj;
        }
        ++itO;
      }
      return (GameObject*)m_Map.FindObjectByType( Layer, ObjType );
    }



    void JREngine::ProcessExtraData( GR::u32 ExtraDataID, GameObject* pObject )
    {
      GR::Gamebase::LayeredMap::tExtraData::iterator    it( m_Map.ExtraDatas.find( ExtraDataID ) );
      if ( it != m_Map.ExtraDatas.end() )
      {
        HandleExtraData( it->second, pObject );
      }
    }



    GameObject* JREngine::FindCollidingObject( GameObject* pCollider, int Type )
    {
      std::list<GameObject*>::iterator    itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject* pObj = *itO;

        if ( ( pObj->Bounds().Intersects( pCollider->Bounds() ) )
        &&   ( pObj->Type == Type ) )
        {
          return pObj;
        }
        ++itO;
      }
      return NULL;
    }



    GameObject* JREngine::FindCollidingObject( GameObject* pCollider, const GR::String& Type )
    {
      std::list<GameObject*>::iterator    itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*   pObj = *itO;

        if ( ( pObj->Bounds().Intersects( pCollider->Bounds() ) )
        &&   ( pObj->Template == Type ) )
        {
          return pObj;
        }
        ++itO;
      }
      return NULL;
    }



    bool JREngine::FindObjectsInArea( const GR::tRect& Area, GR::Gamebase::Layer* pLayer, std::list<GameObject*>& ObjectList )
    {
      std::list<GameObject*>::iterator    itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*   pObj = *itO;

        if ( pObj->Bounds().Intersects( Area ) )
        {
          ObjectList.push_back( pObj );
        }
        ++itO;
      }
      return !ObjectList.empty();
    }



    bool JREngine::FindObjectsAt( const GR::tPoint& Position, std::list<GameObject*>& ObjectList )
    {
      std::list<GameObject*>::iterator    itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*   pObj = *itO;

        if ( pObj->Position == Position )
        {
          ObjectList.push_back( pObj );
        }
        ++itO;
      }
      return !ObjectList.empty();
    }



    bool JREngine::Collide( GameObject* pObj1, GameObject* pObj2, GR::Gamebase::Dir Dir )
    {
      // call handler?
      // TODO - Can collide?
      if ( ( CanCollide( pObj1, pObj2, Dir ) )
      ||   ( CanCollide( pObj2, pObj1, OppositeDir( Dir ) ) ) )
      {
        return pObj1->Bounds().Intersects( pObj2->Bounds() );
      }
      return false;
    }



    bool JREngine::IsTileBlocking( GameObject* pObj, int TileX, int TileY, Dir MoveDir, GR::Gamebase::TileLayer* pLayer )
    {
      if ( pLayer == NULL )
      {
        pLayer = m_pGameLayer;
      }

      GR::u16     field = pLayer->Field( TileX, TileY );
      GR::u32     flagFields = 0;
      if ( m_pFlagLayer )
      {
        flagFields = m_pFlagLayer->Field( TileX, TileY );
      }
      else
      {
        if ( field < m_TileFlags.size() )
        {
          flagFields = m_TileFlags[field];
        }
      }

      bool    handledByHandler = false;

      if ( ( field != 65535 )
      ||   ( flagFields != 0 ) )
      {
        if ( m_TileBlockHandler )
        {
          TileBlockEvent    tbEvent( pObj, field, flagFields, GR::tPoint( TileX, TileY ), MoveDir, pLayer );
          if ( m_TileBlockHandler( tbEvent ) )
          {
            handledByHandler = true;
            if ( tbEvent.TileBlockReaction & TileBlockReaction::STOP_MOVER )
            {
              return true;
            }
            if ( tbEvent.TileBlockReaction & TileBlockReaction::REPLACE_TILE )
            {
              pLayer->SetField( TileX, TileY, tbEvent.Tile );
            }
            if ( tbEvent.TileBlockReaction & TileBlockReaction::REPLACE_FLAGS )
            {
              if ( m_pFlagLayer )
              {
                m_pFlagLayer->SetField( TileX, TileY, tbEvent.Flags );
              }
            }
            if ( tbEvent.TileBlockReaction & TileBlockReaction::KILL_MOVER )
            {
              GR::Gamebase::ObjectEvent   killEvent( GR::Gamebase::ObjectEventType::KILLED, pObj->Position );

              RaiseObjectEvent( pObj, killEvent );
            }
          }
        }
        if ( !handledByHandler )
        {
          GR::u32 blockFlags = 0;
          bool    handledByHandler = false;

          switch ( MoveDir )
          {
            case Dir::U:
              blockFlags |= TileType::BLOCKING_BOTTOM;
              break;
            case Dir::D:
              blockFlags |= TileType::BLOCKING_TOP;
              break;
            case Dir::L:
              blockFlags |= TileType::BLOCKING_RIGHT;
              break;
            case Dir::R:
              blockFlags |= TileType::BLOCKING_LEFT;
              break;
            case Dir::NONE:
              blockFlags |= TileType::BLOCKING;
              break;
          }

          if ( ( field < m_TileFlags.size() )
          &&   ( m_TileFlags[field] & blockFlags ) )
          {
            return true;
          }
        }
      }
      return false;
    }



    const GR::Graphic::ContextDescriptor& JREngine::FetchCollisionCD( TileLayer* pLayer, GR::u16 Field )
    {
      GR::String     tileKey = Misc::Format( "%1%_%2%" ) << pLayer->Tileset << Field;

      auto    itCC( m_CollisionImageCache.find( tileKey ) );
      if ( itCC != m_CollisionImageCache.end() )
      {
        return itCC->second;
      }

      Xtreme::Asset::XAssetLoader* pLoader = ( Xtreme::Asset::XAssetLoader* )GR::Service::Environment::Instance().Service( "AssetLoader" );
      if ( pLoader != NULL )
      {
        Xtreme::Asset::XAssetTileset* pTileSet = ( Xtreme::Asset::XAssetTileset* )pLoader->Asset( Xtreme::Asset::XA_TILESET, pLayer->Tileset.c_str() );
        if ( pTileSet != NULL )
        {
          if ( Field < pTileSet->m_Tiles.size() )
          {
            GR::String     tileSection = pTileSet->m_Tiles[Field].Section;

            Xtreme::Asset::XAssetImageSection* pImageSection = ( Xtreme::Asset::XAssetImageSection* )pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, tileSection.c_str() );
            if ( pImageSection != NULL )
            {
              m_CollisionImageCache[tileKey] = pImageSection->CD();
              return m_CollisionImageCache[tileKey];
            }
          }
        }
      }

      static    GR::Graphic::ContextDescriptor    cdEmpty;

      return cdEmpty;
    }



    bool JREngine::IsAreaBlocked( GameObject* pObj, int DX, int DY, Dir Dir, GameObject* pMovingParent )
    {
      GR::tRect   bounds( pObj->Bounds() );

      bounds.Offset( DX, DY );

      return IsAreaBlocked( pObj, bounds, Dir, pMovingParent );
    }



    bool JREngine::IsAreaBlocked( GameObject* pObj, int DX, int DY, Dir Dir, const GR::String& Layer, GameObject* pMovingParent )
    {
      GR::tRect   bounds( pObj->Bounds() );

      bounds.Offset( DX, DY );

      if ( Layer == "All" )
      {
        for ( size_t i = 0; i < m_Map.Layers.size(); ++i )
        {
          if ( ( m_Map.Layers[i]->Type == GR::Gamebase::Layer::LT_TILE_LAYER )
          ||   ( m_Map.Layers[i]->Type == GR::Gamebase::Layer::LT_COLLISION_LAYER ) )
          {
            if ( IsAreaBlocked( pObj, bounds, Dir, (GR::Gamebase::TileLayer*)m_Map.Layers[i], pMovingParent ) )
            {
              return true;
            }
          }
        }
        return false;
      }
      return IsAreaBlocked( pObj, bounds, Dir, Layer, pMovingParent );
    }



    bool JREngine::IsAreaBlocked( GameObject* pObj, const GR::tRect& Bounds, Dir Dir, GameObject* pMovingParent )
    {
      return IsAreaBlocked( pObj, Bounds, Dir, m_pGameLayer, pMovingParent );
    }



    bool JREngine::IsAreaBlocked( GameObject* pObj, const GR::tRect& Bounds, Dir Dir, const GR::String& Layer, GameObject* pMovingParent )
    {
      if ( Layer == "All" )
      {
        for ( size_t i = 0; i < m_Map.Layers.size(); ++i )
        {
          if ( ( m_Map.Layers[i]->Type == GR::Gamebase::Layer::LT_TILE_LAYER )
          ||   ( m_Map.Layers[i]->Type == GR::Gamebase::Layer::LT_COLLISION_LAYER ) )
          {
            if ( IsAreaBlocked( pObj, Bounds, Dir, m_Map.Layers[i], pMovingParent ) )
            {
              return true;
            }
          }
        }
        return false;
      }

      GR::Gamebase::Layer*        pLayer = m_Map.LayerByName( Layer );
      if ( ( pLayer == NULL )
      ||   ( pLayer->Type != GR::Gamebase::Layer::LT_TILE_LAYER ) )
      {
        return false;
      }
      GR::Gamebase::TileLayer*    pTileLayer = (GR::Gamebase::TileLayer*)pLayer;
      return IsAreaBlocked( pObj, Bounds, Dir, pTileLayer, pMovingParent );
    }



    GR::u32 JREngine::FieldFlags( GR::Gamebase::TileLayer* pLayer, int X, int Y )
    {
      GR::u16     field = pLayer->Field( X, Y );
      GR::u32     flagFields = 0;
      if ( m_pFlagLayer )
      {
        flagFields = m_pFlagLayer->Field( X, Y );

        // if flags from flag layer is 0, override from tile
        if ( ( flagFields == 0 )
        &&   ( field < m_TileFlags.size() ) )
        {
          flagFields = m_TileFlags[field];
        }
      }
      else
      {
        if ( field < m_TileFlags.size() )
        {
          flagFields = m_TileFlags[field];
        }
      }
      return flagFields;
    }



    bool JREngine::IsAreaBlocked( GameObject* pObj, const GR::tRect& Bounds, Dir Dir, GR::Gamebase::Layer* pLayerBase, GameObject* pMovingParent )
    {
      if ( pLayerBase == NULL )
      {
        return IsAreaBlocked( pObj, Bounds, Dir, "All", pMovingParent );
      }
      if ( ( pLayerBase->Type != GR::Gamebase::Layer::LT_TILE_LAYER )
      &&   ( pLayerBase->Type != GR::Gamebase::Layer::LT_COLLISION_LAYER ) )
      {
        return IsAreaBlocked( pObj, Bounds, Dir, pMovingParent );
      }
      GR::Gamebase::TileLayer*  pLayer = ( GR::Gamebase::TileLayer* )pLayerBase;
      bool    blocked = false;

      // tile based layer allows movement once the tile border is crossed
      if ( ( ( pLayerBase->Type == GR::Gamebase::Layer::LT_TILE_LAYER )
      &&     ( ( ( Dir == Dir::U )
      &&         ( ( ( Bounds.Top + 1 ) % pLayer->TileSpacingY ) == 0 ) )
      ||       ( ( Dir == Dir::R )
      &&         ( ( ( Bounds.Right - 1 ) % pLayer->TileSpacingX ) == 0 ) )
      ||       ( ( Dir == Dir::L )
      &&         ( ( ( Bounds.Left + 1 ) % pLayer->TileSpacingX ) == 0 ) )
      ||       ( ( Dir == Dir::D )
      &&         ( ( ( Bounds.Bottom - 1 ) % pLayer->TileSpacingY ) == 0 ) )
      ||       ( Dir == Dir::NONE )
      ||       ( Dir == Dir::ALL ) ) )
      ||   ( pLayerBase->Type == GR::Gamebase::Layer::LT_COLLISION_LAYER ) )
      {
        int     X1 = Bounds.Left / pLayer->TileSpacingX;
        int     Y1 = Bounds.Top / pLayer->TileSpacingY;
        int     X2 = ( Bounds.Right - 1 ) / pLayer->TileSpacingX;
        int     Y2 = ( Bounds.Bottom - 1 ) / pLayer->TileSpacingY;
        if ( Bounds.Left < 0 )
        {
          --X1;
        }
        if ( Bounds.Top < 0 )
        {
          --Y1;
        }
        if ( Bounds.Right - 1 < 0 )
        {
          --X2;
        }
        if ( Bounds.Bottom - 1 < 0 )
        {
          --Y2;
        }

        GR::u32     blockFlags = 0;
        bool        handledByHandler = false;

        switch ( Dir )
        {
          case Dir::U:
            blockFlags |= TileType::BLOCKING_BOTTOM | TileType::FENCE_DOWN;
            break;
          case Dir::D:
            blockFlags |= TileType::BLOCKING_TOP | TileType::FENCE_UP;
            break;
          case Dir::L:
            blockFlags |= TileType::BLOCKING_RIGHT | TileType::FENCE_RIGHT;
            break;
          case Dir::R:
            blockFlags |= TileType::BLOCKING_LEFT | TileType::FENCE_LEFT;
            break;
          case Dir::NONE:
            blockFlags |= TileType::BLOCKING | TileType::FENCE_ALL;
            break;
          case Dir::ALL:
            blockFlags |= TileType::BLOCKING | TileType::FENCE_ALL;
            break;
        }

        for ( int x = X1; x <= X2; ++x )
        {
          for ( int y = Y1; y <= Y2; ++y )
          {
            if ( m_BlockMovingOutsideMap )
            {
              if ( x < 0 )
              {
                if ( !( pObj->Flags & GameObject::ObjectInfoFlags::OF_ALLOW_LEAVING_MAP ) )
                {
                  blocked = true;
                }
                else if ( QueryEvent( GR::Gamebase::QueryEvent( GR::Gamebase::QueryEventType::ALLOW_MOVE_OUTSIDE_MAP_W, GR::tPoint( x, y ), pObj ) ) )
                {
                  blocked = true;
                }
              }
              if ( x >= pLayer->Width() )
              {
                if ( !( pObj->Flags & GameObject::ObjectInfoFlags::OF_ALLOW_LEAVING_MAP ) )
                {
                  blocked = true;
                }
                else if ( QueryEvent( GR::Gamebase::QueryEvent( GR::Gamebase::QueryEventType::ALLOW_MOVE_OUTSIDE_MAP_E, GR::tPoint( x, y ), pObj ) ) )
                {
                  blocked = true;
                }
              }
              if ( y < 0 )
              {
                if ( !( pObj->Flags & GameObject::ObjectInfoFlags::OF_ALLOW_LEAVING_MAP ) )
                {
                  blocked = true;
                }
                else if ( QueryEvent( GR::Gamebase::QueryEvent( GR::Gamebase::QueryEventType::ALLOW_MOVE_OUTSIDE_MAP_N, GR::tPoint( x, y ), pObj ) ) )
                {
                  blocked = true;
                }
              }
              if ( y >= pLayer->Height() ) 
              {
                if ( !( pObj->Flags & GameObject::ObjectInfoFlags::OF_ALLOW_LEAVING_MAP ) )
                {
                  blocked = true;
                }
                else if ( QueryEvent( GR::Gamebase::QueryEvent( GR::Gamebase::QueryEventType::ALLOW_MOVE_OUTSIDE_MAP_S, GR::tPoint( x, y ), pObj ) ) )
                {
                  blocked = true;
                }
              }
            }
            GR::u32     flagFields = FieldFlags( pLayer, x, y );
            GR::u16     field = pLayer->Field( x, y );

            if ( Dir == Dir::L )
            {
              GR::u32     flagFieldsLeft = FieldFlags( pLayer, x + 1, y );
              if ( flagFieldsLeft & GR::Gamebase::TileType::FENCE_LEFT )
              {
                blocked = true;
              }
            }
            else if ( Dir == Dir::R )
            {
              GR::u32     flagFieldsLeft = FieldFlags( pLayer, x - 1, y );
              if ( flagFieldsLeft & GR::Gamebase::TileType::FENCE_RIGHT )
              {
                blocked = true;
              }
            }
            else if ( Dir == Dir::U )
            {
              GR::u32     flagFieldsLeft = FieldFlags( pLayer, x, y + 1 );
              if ( flagFieldsLeft & GR::Gamebase::TileType::FENCE_UP )
              {
                blocked = true;
              }
            }
            else if ( Dir == Dir::D )
            {
              GR::u32     flagFieldsLeft = FieldFlags( pLayer, x, y - 1 );
              if ( flagFieldsLeft & GR::Gamebase::TileType::FENCE_DOWN )
              {
                blocked = true;
              }
            }

            if ( pLayerBase->Type == GR::Gamebase::Layer::LT_COLLISION_LAYER )
            {
              const GR::Graphic::ContextDescriptor&   cdCollisionTile( FetchCollisionCD( pLayer, field ) );

              int     xOffset = ( Bounds.Left - x * pLayer->TileSpacingX ) % pLayer->TileSpacingX;
              int     yOffset = ( Bounds.Top - y * pLayer->TileSpacingY ) % pLayer->TileSpacingY;

              int     width = math::minValue( Bounds.Width(), pLayer->TileSpacingX );
              int     height = math::minValue( Bounds.Height(), pLayer->TileSpacingY );

              for ( int i = 0; i < width; ++i )
              {
                for ( int j = 0; j < height; ++j )
                {
                  GR::u32     collisionPixel = cdCollisionTile.GetPixel( xOffset + i, yOffset + j );
                  if ( ( collisionPixel == 0x00ffffff )       // blocking
                  ||   ( ( collisionPixel == 0x00ff0000 )     // blocking downwards only (platforms)
                  &&     ( Dir == GR::Gamebase::Dir::D ) ) )
                  {
                    blockFlags |= TileType::BLOCKING;

                    GR::Gamebase::JREvent   blockEvent( GR::Gamebase::JREvent::JRE_BLOCKED_MOVEMENT, pObj );
                    blockEvent.Pos.Set( x, y );
                    RaiseJREvent( blockEvent );
                    blocked = true;
                    i = width;
                    break;
                  }
                  if ( collisionPixel == 0x00ff00ff )
                  {
                    // deadly for player
                    blockFlags |= TileType::BLOCKING;

                    if ( pObj == m_pControlledObject )
                    {
                      RaiseObjectEvent( pObj, GR::Gamebase::ObjectEventType::KILLED );
                    }
                    blocked = true;
                    i = width;
                    break;
                  }
                }
              }
            }

            
            if ( ( field != 65535 )
            ||   ( flagFields != 0 ) )
            {
              handledByHandler = false;
              if ( m_TileBlockHandler )
              {
                TileBlockEvent    tbEvent( pObj, field, flagFields, GR::tPoint( x, y ), Dir, pLayer );
                if ( m_TileBlockHandler( tbEvent ) )
                {
                  handledByHandler = true;
                  if ( tbEvent.TileBlockReaction & TileBlockReaction::STOP_MOVER )
                  {
                    blocked = true;

                    GR::Gamebase::JREvent   blockEvent( GR::Gamebase::JREvent::JRE_BLOCKED_MOVEMENT, pObj );
                    blockEvent.Pos.Set( x, y );
                    RaiseJREvent( blockEvent );
                  }
                  if ( tbEvent.TileBlockReaction & TileBlockReaction::REPLACE_TILE )
                  {
                    pLayer->SetField( x, y, tbEvent.Tile );
                  }
                  if ( tbEvent.TileBlockReaction & TileBlockReaction::REPLACE_FLAGS )
                  {
                    if ( m_pFlagLayer )
                    {
                      m_pFlagLayer->SetField( x, y, tbEvent.Flags );
                    }
                  }
                  if ( tbEvent.TileBlockReaction & TileBlockReaction::KILL_MOVER )
                  {
                    GR::Gamebase::ObjectEvent   killEvent( GR::Gamebase::ObjectEventType::KILLED, pObj->Position );

                    RaiseObjectEvent( pObj, killEvent );
                  }
                }
              }
              if ( !handledByHandler )
              {
                if ( Dir == Dir::ALL )
                {
                  if ( flagFields == blockFlags )
                  {
                    blocked = true;
                  }
                }
                else if ( flagFields & blockFlags )
                {
                  blocked = true;
                }
                if ( flagFields & TileType::DEADLY )
                {
                  GR::Gamebase::ObjectEvent   killEvent( GR::Gamebase::ObjectEventType::KILLED, pObj->Position );

                  RaiseObjectEvent( pObj, killEvent );
                  blocked = true;
                }
              }
            }
            // TODO - all layers?
            /*
            if ( m_pFGLayer )
            {
              GR::u16     field = m_pFGLayer->Field( x, y );
              if ( field != 65535 )
              {
                if ( m_TileFlags[field] & blockFlags )
                {
                  blocked = true;
                }
              }
            }
            if ( m_pBGLayer )
            {
              GR::u16     field = m_pBGLayer->Field( x, y );
              if ( field != 65535 )
              {
                if ( m_TileFlags[field] & blockFlags )
                {
                  blocked = true;
                }
              }
            }
            */
          }
        }
      }
      if ( ( pObj != NULL )
      &&   ( IsObjectBlockedByOtherObjects( pObj, Bounds, Dir, pMovingParent ) ) )
      {
        GR::Gamebase::JREvent   blockEvent( GR::Gamebase::JREvent::JRE_BLOCKED_MOVEMENT_BY_OTHER_OBJECT, pObj );

        RaiseJREvent( blockEvent );
        return true;
      }
      if ( ( pObj == NULL )
      &&   ( IsBlockedByObjects( Bounds, Dir ) ) )
      {
        GR::Gamebase::JREvent   blockEvent( GR::Gamebase::JREvent::JRE_BLOCKED_MOVEMENT_BY_OTHER_OBJECT, pObj );
        RaiseJREvent( blockEvent );
        return true;
      }
      return blocked;
    }



    void JREngine::RunMapScript( const GR::String& ScriptName )
    {
      if ( m_Map.Scripts.find( ScriptName ) == m_Map.Scripts.end() )
      {
        return;
      }
      RunScript( m_Map.Scripts[ScriptName] );
    }



    void JREngine::RunScript( const GR::String& Script )
    {
      m_ScriptHandler.StartScriptAsThread( Script );
    }



    void JREngine::RunScriptFromFile( const GR::String& Filename )
    {
      m_ScriptHandler.DoFile( Filename );
    }



    void JREngine::SetAddObjectHandler( LayeredMap::tAddObjectHandlerFunction Function )
    {
      m_AddObjectHandler = Function;
      m_Map.SetAddObjectHandler( Function );
    }



    void JREngine::SetCanCollideHandler( tCanCollideEventFunction Function )
    {
      m_CanCollideHandler = Function;
    }



    void JREngine::SetCollisionHandler( tCollisionHandlerEventFunction Function )
    {
      m_CollisionHandler = Function;
    }



    void JREngine::SetObjectBlocksObjectHandler( tObjectBlocksObjectEventFunction Function )
    {
      m_ObjectBlocksObjectHandler = Function;
    }



    void JREngine::SetObjectBlocksHandler( tObjectBlocksFunction Function )
    {
      m_ObjectBlockHandler = Function;
    }



    void JREngine::RegisterScript( lua_CFunction pFunction, const GR::String& Name )
    {
      m_ScriptHandler.Register( pFunction, Name.c_str() );
    }



    void JREngine::RegisterScript( LuaInstance::tHandlerFunction Function, const GR::String& Name )
    {
      m_ScriptHandler.RegisterHandler( Function, Name.c_str() );
    }



    void JREngine::SetQueryHandler( tQueryHandlerFunction Function )
    {
      m_QueryHandler = Function;
    }



    void JREngine::SetJREventHandler( tJREventHandlerFunction Function )
    {
      m_JREventHandler = Function;
    }



    void JREngine::SetTileBlockHandler( tTileBlockHandlerEventFunction Function )
    {
      m_TileBlockHandler = Function;
    }



    bool JREngine::OnLayeredMapEvent( const GR::Gamebase::LayeredMap::LayeredMapEvent& Event )
    {
      switch ( Event.Type )
      {
        case GR::Gamebase::LayeredMap::LayeredMapEvent::ET_TRIGGER_ENTER:
          {
            GR::Gamebase::LayeredMap::tTrigger::iterator    it( m_Map.Triggers.find( Event.pObject->CurrentTrigger ) );
            if ( it == m_Map.Triggers.end() )
            {
              break;
            }
            GR::Gamebase::LayeredMap::Trigger& trigger( it->second );

            if ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ON_ENTER )
            {
              if ( ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ONCE )
              &&   ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGERED ) )
              {
                // was already triggered
                break;
              }
              trigger.Flags |= GR::Gamebase::LayeredMap::Trigger::TRIGGERED;

              FireTrigger( trigger, ( GR::Gamebase::GameObject* )Event.pObject );
            }
          }
          break;
        case GR::Gamebase::LayeredMap::LayeredMapEvent::ET_TRIGGER_INSIDE:
          {
            GR::Gamebase::LayeredMap::tTrigger::iterator    it( m_Map.Triggers.find( Event.pObject->CurrentTrigger ) );
            if ( it == m_Map.Triggers.end() )
            {
              break;
            }
            GR::Gamebase::LayeredMap::Trigger& trigger( it->second );

            if ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ON_INSIDE )
            {
              if ( ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ONCE )
              &&   ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGERED ) )
              {
              }
              else
              {
                RunScript( trigger.EnterScript );
                FireTrigger( trigger, (GR::Gamebase::GameObject*)Event.pObject );
                return true;
              }
            }
          }
          break;
        case GR::Gamebase::LayeredMap::LayeredMapEvent::ET_TRIGGER_FULL_INSIDE:
          {
            GR::Gamebase::LayeredMap::tTrigger::iterator    it( m_Map.Triggers.find( Event.pObject->CurrentTrigger ) );
            if ( it == m_Map.Triggers.end() )
            {
              break;
            }
            GR::Gamebase::LayeredMap::Trigger& trigger( it->second );

            if ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ON_FULL_INSIDE )
            {
              if ( ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ONCE )
              &&   ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGERED ) )
              {
              }
              else
              {
                RunScript( trigger.EnterScript );
                FireTrigger( trigger, (GR::Gamebase::GameObject*)Event.pObject );
                return true;
              }
            }
          }
          break;
        case GR::Gamebase::LayeredMap::LayeredMapEvent::ET_TRIGGER_LEAVE:
          {
            GR::Gamebase::LayeredMap::tTrigger::iterator    it( m_Map.Triggers.find( Event.pObject->CurrentTrigger ) );
            if ( it == m_Map.Triggers.end() )
            {
              break;
            }
            GR::Gamebase::LayeredMap::Trigger& trigger( it->second );

            if ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ON_LEAVE )
            {
              if ( ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ONCE )
              &&   ( trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGERED ) )
              {
              }
              else
              {
                RunScript( trigger.LeaveScript );
                FireTrigger( trigger, (GR::Gamebase::GameObject*)Event.pObject );
                return true;
              }
            }
          }
          break;
      }
      return false;
    }



    void JREngine::FireTrigger( const GR::Gamebase::LayeredMap::Trigger& Trigger, GameObject* pObj )
    {
      GR::Gamebase::LayeredMap::LayeredMapEvent  eventTrigger( GR::Gamebase::LayeredMap::LayeredMapEvent::ET_TRIGGER_TRIGGERED, pObj );
      eventTrigger.ElementID = Trigger.ID;

      if ( !m_Map.RaiseEvent( eventTrigger ) )
      {
        HandleEvent( eventTrigger );
      }
    }



    void JREngine::HandleEvent( const GR::Gamebase::LayeredMap::LayeredMapEvent& Event )
    {
      switch ( Event.Type )
      {
        case GR::Gamebase::LayeredMap::LayeredMapEvent::ET_TRIGGER_TRIGGERED:
          {
            GR::Gamebase::LayeredMap::Trigger& Trigger( m_Map.Triggers[Event.ElementID] );

            if ( Trigger.Flags & GR::Gamebase::LayeredMap::Trigger::TRIGGER_ONCE )
            {
              Trigger.Flags |= GR::Gamebase::LayeredMap::Trigger::TRIGGERED;
            }

            switch ( Trigger.Type )
            {
              case GR::Gamebase::LayeredMap::Trigger::TT_EXTRA_DATA:
                HandleExtraData( m_Map.ExtraDatas[Trigger.ExtraDataID], (GameObject*)Event.pObject );
                break;
            }
          }

          break;
      }
    }



    void JREngine::HandleExtraData( const GR::Game::ExtraData& ExtraData, GameObject* pObj, bool NoQueueing )
    {
      switch ( ExtraData.Type )
      {
        case GR::Game::ExtraDataType::SCRIPT:
          if ( pObj != NULL )
          {
            GR::String      scriptObjectName = Misc::Format( "ExtraData_Object_%1%_%2%" ) << pObj->ID << pObj->ExtraDataID;
            GR::String      scriptThreadName = "Thread" + scriptObjectName;

            if ( m_ScriptHandler.IsThreadRunning( scriptThreadName ) )
            {
              return;
            }
            Loona<ScriptGameObject>::PushObject( m_ScriptHandler, new ScriptGameObject( this, pObj ) );
            m_ScriptHandler.SetGlobal( scriptObjectName );
            m_ScriptHandler.DoString( "this = " + scriptObjectName );

            m_ScriptHandler.StartScriptAsThread( ExtraData.Param, scriptThreadName, scriptObjectName );
          }
          else
          {
            m_ScriptHandler.StartScriptAsThread( ExtraData.Param );
          }
          break;
        case GR::Game::ExtraDataType::WARP:
          if ( pObj == NULL )
          {
            dh::Log( "JREngine::HandleExtraData: Warp without object" );
          }
          else
          {
            // x in param1
            // y in param2
            // map in param
            if ( ExtraData.Param.empty() )
            {
              // simple warp
              RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::WARP ) );
              
              //SetObjectPos( pObj, GR::tPoint( ExtraData.Param1 * m_pGameLayer->TileSpacingX + m_pGameLayer->TileSpacingX / 2, ExtraData.Param2 * m_pGameLayer->TileSpacingY + m_pGameLayer->TileSpacingY - 1 ) );
              SetObjectPos( pObj, GR::tPoint( ExtraData.Param1, ExtraData.Param2 ) );
              if ( pObj == m_pControlledObject )
              {
                CenterCameraOnObject( pObj );
              }

              RaiseJREvent( JREvent( JREvent::JRE_WARP_COMPLETED, pObj ) );
            }
            else
            {
              // load map
              if ( NoQueueing )
              {
                // get a copy of extradata, it's gone after InitMap
                GR::Game::ExtraData   extraData( ExtraData );

                WarpToMap( ExtraData.Param, extraData.Param1, extraData.Param2 );
                RaiseJREvent( JREvent( JREvent::JRE_WARP_COMPLETED, m_pControlledObject ) );
              }
              else
              {
                m_ExtraDataQueue.push_back( std::make_pair( ExtraData.ID, pObj ) );
              }
            }
          }
          break;
      }
    }



    void JREngine::WarpToMap( const GR::String& MapName, int X, int Y )
    {
      if ( m_CurrentMap == MapName )
      {
        // no need to warp!
        SetObjectPos( m_pControlledObject, GR::tPoint( X, Y ) );
        CenterCameraOnObject( m_pControlledObject );
        return;
      }

      auto  pOldObject = m_pControlledObject;
      if ( ( m_pControlledObject != NULL )
      &&   ( m_pGameLayer != NULL ) )
      {
        m_AwakeObjects.remove( m_pControlledObject );
        m_pControlledObject->AbortMove();
        RemoveObjectFromGrid( m_pControlledObject );
      }

      InitMap( Path::Append( m_MapPath, MapName ), m_MainLayer, m_ControlObjectName );
      m_pControlledObject = pOldObject;
      if ( ( m_pControlledObject == NULL )
      &&   ( m_pGameLayer != NULL ) )
      {
        dh::Log( "Create new controlled" );
        m_pControlledObject = (GameObject*)m_Map.AddObject( m_MainLayer, m_ControlObjectName, X, Y );
      }
      else if ( ( m_pControlledObject != NULL )
      &&        ( m_pGameLayer != NULL ) )
      {
        m_AwakeObjects.push_back( m_pControlledObject );
      }

      if ( m_pControlledObject )
      {
        AwakenObject( m_pControlledObject );
        SetObjectPos( m_pControlledObject, GR::tPoint( X, Y ) );
        CenterCameraOnObject( m_pControlledObject );
      }
      RaiseJREvent( JREvent( JREvent::JRE_CONTROLLED_OBJECT_CHANGED, m_pControlledObject ) );
    }



    GR::Gamebase::Dir JREngine::OppositeDir( GR::Gamebase::Dir Dir )
    {
      switch ( Dir )
      {
        case GR::Gamebase::Dir::D:
          return GR::Gamebase::Dir::U;
        case GR::Gamebase::Dir::U:
          return GR::Gamebase::Dir::D;
        case GR::Gamebase::Dir::L:
          return GR::Gamebase::Dir::R;
        case GR::Gamebase::Dir::R:
          return GR::Gamebase::Dir::L;
      }
      return GR::Gamebase::Dir::NONE;
    }



    void JREngine::OnCollide( GameObject* pObj1, GameObject* pObj2, GR::Gamebase::Dir Dir, GR::u32& CollisionReaction )
    {
      RaiseObjectEvent( pObj1, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::COLLIDE, pObj2 ) );
      RaiseObjectEvent( pObj2, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::COLLIDE, pObj1 ) );

      if ( m_CollisionHandler )
      {
        CollisionReaction = m_CollisionHandler( (GameObject*)pObj1, (GameObject*)pObj2, Dir );
        if ( CollisionReaction == CollisionReaction::NONE )
        {
          CollisionReaction = m_CollisionHandler( (GameObject*)pObj2, (GameObject*)pObj1, OppositeDir( Dir ) );

          GR::u32     reverseReaction = 0;

          if ( CollisionReaction & CollisionReaction::KILL_MOVER )
          {
            reverseReaction |= CollisionReaction::KILL_OBSTACLE;
          }
          if ( CollisionReaction & CollisionReaction::KILL_OBSTACLE )
          {
            reverseReaction |= CollisionReaction::KILL_MOVER;
          }
          if ( CollisionReaction & CollisionReaction::STOP_MOVER )
          {
            reverseReaction |= CollisionReaction::STOP_OBSTACLE;
          }
          if ( CollisionReaction & CollisionReaction::STOP_OBSTACLE )
          {
            reverseReaction |= CollisionReaction::STOP_MOVER;
          }
          CollisionReaction = reverseReaction;
        }
      }
    }



    bool JREngine::CanCollide( GameObject* pObj1, GameObject* pObj2, GR::Gamebase::Dir Dir )
    {
      if ( m_CanCollideHandler )
      {
        return ( ( m_CanCollideHandler( pObj1, pObj2, Dir ) )
        ||       ( m_CanCollideHandler( pObj2, pObj1, OppositeDir( Dir ) ) ) );
      }
      return false;
    }



    bool JREngine::DoesObjectBlockObject( GameObject* pMover, GameObject* pObstacle, GR::Gamebase::Dir Dir )
    {
      if ( m_ObjectBlocksObjectHandler )
      {
        return m_ObjectBlocksObjectHandler( pMover, pObstacle, Dir );
      }
      return false;
    }



    bool JREngine::DoesObjectBlock( GameObject* pObstacle, GR::Gamebase::Dir Dir )
    {
      if ( m_ObjectBlockHandler )
      {
        return m_ObjectBlockHandler( pObstacle, Dir );
      }
      return false;
    }



    bool JREngine::CheckCollisions( GameObject* pObj, const GR::tRect& Bounds, GR::Gamebase::Dir Dir )
    {
      // only check awakeobjects
      bool    blocked = false;

      std::list<GameObject*>::iterator   itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*    pLayerObj( *itO );

        if ( ( pObj != pLayerObj )
        &&   ( !( pLayerObj->m_ProcessingFlags & ProcessingFlags::DELETE_ME ) )
        &&   ( !( pObj->m_ProcessingFlags & ProcessingFlags::DELETE_ME ) )
        &&   ( Collide( pObj, pLayerObj, Dir ) ) )
        {
          GR::u32     collisionReaction = CollisionReaction::NONE;

          OnCollide( pObj, pLayerObj, Dir, collisionReaction );
          if ( collisionReaction & CollisionReaction::STOP_MOVER )
          {
            blocked = true;
          }
          if ( collisionReaction & CollisionReaction::KILL_MOVER )
          {
            RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::KILLED, pLayerObj ) );
          }
          if ( collisionReaction & CollisionReaction::KILL_OBSTACLE )
          {
            RaiseObjectEvent( pLayerObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::KILLED, pObj ) );
          }
        }

        ++itO;
      }
      return blocked;
    }



    bool JREngine::IsObjectBlockedByOtherObjects( GameObject* pObj, const GR::tRect& Bounds, GR::Gamebase::Dir Dir, GameObject* pMovingParent )
    {
      GR::tPoint      GridPos = m_pGameLayer->GridPos( pObj );
      GameObject*     pCarryingObject = NULL;

      bool    blocked = false;

      std::list<GameObject*>::iterator   itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*    pLayerObj( *itO );

        if ( ( pObj != pLayerObj )
        //&&   ( pLayerObj != pMovingParent )
        &&   ( !( pObj->m_ProcessingFlags & ProcessingFlags::DELETE_ME ) )
        &&   ( !( pLayerObj->m_ProcessingFlags & ProcessingFlags::DELETE_ME ) )
        &&   ( Bounds.Intersects( pLayerObj->Bounds() ) ) )
        {
          if ( DoesObjectBlockObject( pObj, pLayerObj, Dir ) )
          {
            if ( Dir == GR::Gamebase::Dir::D )
            {
              pCarryingObject = (GameObject*)pLayerObj;
            }
            blocked = true;
          }
          if ( ( CanCollide( pObj, pLayerObj, Dir ) )
          ||   ( CanCollide( pLayerObj, pObj, OppositeDir( Dir ) ) ) )
          {
            GR::u32     collisionReaction = CollisionReaction::NONE;

            OnCollide( pObj, pLayerObj, Dir, collisionReaction );
            if ( collisionReaction & CollisionReaction::STOP_MOVER )
            {
              blocked = true;
            }
            if ( collisionReaction & CollisionReaction::KILL_MOVER )
            {
              RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::KILLED, pObj ) );
            }
            if ( collisionReaction & CollisionReaction::KILL_OBSTACLE )
            {
              RaiseObjectEvent( pLayerObj, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::KILLED, pLayerObj ) );
            }
          }
        }

        ++itO;
      }

      if ( pMovingParent == NULL )
      {
        if ( Dir == GR::Gamebase::Dir::D )
        {
          if ( pCarryingObject != pObj->m_pObjectBelow )
          {
            if ( pObj->m_pObjectBelow != NULL )
            {
              GameObject*   pOldPlatform = pObj->m_pObjectBelow;

              RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::GET_OFF_OBJECT, pObj->m_pObjectBelow ) );
              RaiseObjectEvent( pOldPlatform, GR::Gamebase::ObjectEvent( ObjectEventType::OBJECT_GOT_OFF_ME, pObj ) );
            }
            pObj->m_pObjectBelow = pCarryingObject;
            if ( pCarryingObject != NULL )
            {
              RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::LAND_ON_OBJECT, pCarryingObject ) );
              RaiseObjectEvent( pCarryingObject, GR::Gamebase::ObjectEvent( ObjectEventType::OBJECT_LANDED_ON_ME, pObj ) );
            }
          }
        }
      }
      return blocked;
    }



    bool JREngine::IsBlockedByObjects( const GR::tRect& Bounds, GR::Gamebase::Dir Dir )
    {
      GR::tPoint      GridPos = m_pGameLayer->GridPos( Bounds.Position() );
      GameObject*     pCarryingObject = NULL;

      bool    blocked = false;

      std::list<GameObject*>::iterator   itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject*    pLayerObj( *itO );

        if ( Bounds.Intersects( pLayerObj->Bounds() ) )
        {
          if ( DoesObjectBlock( pLayerObj, Dir ) )
          {
            blocked = true;
          }
        }

        ++itO;
      }
      return blocked;
    }



    void JREngine::UpdateScripts( const GR::f32 ElapsedTime )
    {
      m_ScriptHandler.ResumeThreads( ElapsedTime );
    }



    void JREngine::ObjectIsDeleted( GameObject* pObj )
    {
      // uncarry
      if ( pObj->m_pObjectBelow != NULL )
      {
        pObj->m_pObjectBelow->m_ObjectsCarried.remove( pObj );
        pObj->m_pObjectBelow = NULL;
      }
      auto    itC( pObj->m_ObjectsCarried.begin() );
      while ( itC != pObj->m_ObjectsCarried.end() )
      {
        auto pCarried = *itC;

        pCarried->m_pObjectBelow = NULL;

        ++itC;
      }
      pObj->m_ObjectsCarried.clear();


      //dh::Log( "Deleting (%s/%x/%d)", pObj->Template.c_str(), pObj, pObj->ExtraDataID );
      RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::DELETED ) );

      // remove all handlers
      tObjectEventHandler::iterator   it( m_ObjectEventHandler.begin() );
      while ( it != m_ObjectEventHandler.end() )
      {
        if ( it->first.second == pObj )
        {
          it = m_ObjectEventHandler.erase( it );
        }
        else
        {
          ++it;
        }
      }
      if ( m_pControlledObject == pObj )
      {
        m_pControlledObject = NULL;
      }
      delete pObj;
    }



    void JREngine::Update( Xtreme::XInput* pInput, const GR::f32 ElapsedTime )
    {
      UpdateScripts( ElapsedTime );

      // TODO - awoken objects. vs. sleeping!
      GR::tPoint    controlledObjectGridPos;

      if ( m_pControlledObject )
      {
        controlledObjectGridPos = m_pGameLayer->GridPos( m_pControlledObject );

        for ( int i = -1; i <= 1; ++i )
        {
          for ( int j = -1; j <= 1; ++j )
          {
            GR::tPoint    gridPos( controlledObjectGridPos.x + i, controlledObjectGridPos.y + j );

            if ( m_AwakeSectors.find( gridPos ) == m_AwakeSectors.end() )
            {
              AwakenObjects( gridPos );
              m_AwakeSectors.insert( gridPos );
            }

            std::map<GR::tPoint, std::list<LayerObject*> >::iterator   itGrid( m_pGameLayer->Objects.find( gridPos ) );
            if ( itGrid == m_pGameLayer->Objects.end() )
            {
              continue;
            }
            std::list<LayerObject*>&    listObj( itGrid->second );

            std::list<LayerObject*>::iterator   itO( listObj.begin() );
            while ( itO != listObj.end() )
            {
              GameObject*    pLayerObj( (GameObject*)*itO );

              if ( pLayerObj->m_ProcessingFlags & ProcessingFlags::DELETE_ME )
              {
                ObjectIsDeleted( pLayerObj );
                m_AwakeObjects.remove( pLayerObj );
                m_pGameLayer->RemoveObject( pLayerObj );
                itO = listObj.erase( itO );
              }
              else
              {
                ++itO;
              }
            }
          }
        }

        std::set<GR::tPoint>::iterator    itAS( m_AwakeSectors.begin() );
        while ( itAS != m_AwakeSectors.end() )
        {
          const GR::tPoint&     aSector( *itAS );

          if ( ( aSector.x < controlledObjectGridPos.x - 1 )
            || ( aSector.x > controlledObjectGridPos.x + 1 )
            || ( aSector.y < controlledObjectGridPos.y - 1 )
            || ( aSector.y > controlledObjectGridPos.y + 1 ) )
          {
            SleepObjects( aSector );
            itAS = m_AwakeSectors.erase( itAS );
          }
          else
          {
            // are objects off screen?

            ++itAS;
          }
        }
      }
      // for both
      m_MapChanged = false;
      std::list<GR::Gamebase::GameObject*>::iterator   itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GR::Gamebase::GameObject*    pLayerObj( (GameObject*)*itO );

        if ( pLayerObj->m_ProcessingFlags & ProcessingFlags::DELETE_ME )
        {
          m_pGameLayer->RemoveObject( pLayerObj );
          ObjectIsDeleted( pLayerObj );
          itO = m_AwakeObjects.erase( itO );
          continue;
        }
        else if ( pLayerObj->m_ProcessingFlags & ProcessingFlags::NO_UPDATE )
        {
          ++itO;
          continue;
        }
        else if ( pLayerObj->m_ProcessingFlags & ProcessingFlags::SEND_TO_SLEEP )
        {
          if ( SleepObject( pLayerObj ) )
          {
            itO = m_AwakeObjects.erase( itO );
            continue;
          }
        }
        pLayerObj->Update( *this, ElapsedTime, m_ObjectUserData );

        if ( m_MapChanged )
        {
          // the object list is invalid now!
          break;
        }

        CheckCollisions( pLayerObj, pLayerObj->Bounds(), GR::Gamebase::Dir::NONE );
        ++itO;
      }

      if ( m_pControlledObject != NULL )
      {
        controlledObjectGridPos = m_pGameLayer->GridPos( m_pControlledObject );
        itO = m_AwakeObjects.begin();
        while ( itO != m_AwakeObjects.end() )
        {
          GR::Gamebase::GameObject*    pLayerObj( (GameObject*)*itO );

          if ( pLayerObj->Flags & GR::Gamebase::GameObject::OF_ALWAYS_AWAKE )
          {
            ++itO;
            continue;
          }

          GR::tPoint    objGridPos( m_pGameLayer->GridPos( pLayerObj ) );

          if ( ( objGridPos.x < controlledObjectGridPos.x - 1 )
          ||   ( objGridPos.x > controlledObjectGridPos.x + 1 )
          ||   ( objGridPos.y < controlledObjectGridPos.y - 1 )
          ||   ( objGridPos.y > controlledObjectGridPos.y + 1 ) )
          {
            //dh::Log( "Sending to sleep (%s/%x/%d)", pLayerObj->Template.c_str(), pLayerObj, pLayerObj->ExtraDataID );
            pLayerObj->SendToSleep();
          }
          ++itO;
        }
      }
      ReseatMovedObjectsInSector();

      std::list<std::pair<GR::u32,GameObject*> >::iterator    itED( m_ExtraDataQueue.begin() );
      while ( itED != m_ExtraDataQueue.end() )
      {
        HandleExtraData( m_Map.ExtraDatas[itED->first], itED->second, true );

        ++itED;
      }
      m_ExtraDataQueue.clear();

      UpdateCamera( ElapsedTime );
    }



    void JREngine::UpdateFixedStep( Xtreme::XInput* pInput, const GR::f32 ElapsedTime )
    {
      std::list<GameObject*>::iterator    itO( m_AwakeObjects.begin() );
      while ( itO != m_AwakeObjects.end() )
      {
        GameObject* pObj( *itO );
        ++itO;

        if ( pObj->m_ProcessingFlags & ( ProcessingFlags::DELETE_ME | ProcessingFlags::SEND_TO_SLEEP | ProcessingFlags::NO_UPDATE ) )
        {
          continue;
        }
        pObj->UpdateFixed( *this, ElapsedTime, m_ObjectUserData );
      }

      ReseatMovedObjectsInSector();
      UpdateCamera( ElapsedTime );
    }



    void JREngine::HandleMovementPath( GR::Gamebase::GameObject& Object, GR::f32 ElapsedTime )
    {
      if ( Object.Flags & GR::Gamebase::GameObject::OF_HAS_MOVEMENT_PATH )
      {
        GR::f32   timeToUse = ElapsedTime;
        while ( timeToUse > 0.0f )
        {
          GR::Game::Path::MovementPath::tPathFrame*  pFrame = Object.m_MovementPathPos.pPath->Frame( Object.m_MovementPathPos.CurrentFrame );

          if ( Object.m_MovementPathPos.ElapsedFrameTime + timeToUse >= pFrame->FrameLength )
          {
            GR::f32   delta = pFrame->FrameLength - Object.m_MovementPathPos.ElapsedFrameTime;

            math::vector3   vecDelta = ( pFrame->StartPos - pFrame->EndPos ) * delta / pFrame->FrameLength;

            MoveObject( &Object, vecDelta.x, vecDelta.y );

            timeToUse -= delta;

            if ( Object.m_MovementPathPos.CurrentFrame + 1 >= Object.m_MovementPathPos.pPath->FrameCount() )
            {
              Object.m_MovementPathPos.CurrentFrame = 0;
            }
            else
            {
              ++Object.m_MovementPathPos.CurrentFrame;
            }
            Object.m_MovementPathPos.ElapsedFrameTime = 0.0f;
          }
          else
          {
            math::vector3   vecDelta = ( pFrame->StartPos - pFrame->EndPos ) * timeToUse / pFrame->FrameLength;

            MoveObject( &Object, vecDelta.x, vecDelta.y );

            Object.m_MovementPathPos.ElapsedFrameTime += timeToUse;
            timeToUse = 0.0f;
          }
        }
      }
    }



    void JREngine::ReseatMovedObjectsInSector()
    {
      /*
      // put moved objects in correct sector
      std::map<GameObject*,MovedObjectInfo>::iterator   itMO( m_MovedObjects.begin() );
      while ( itMO != m_MovedObjects.end() )
      {
        const MovedObjectInfo& oInfo( itMO->second );

        GR::tPoint  gridPos = m_pGameLayer->GridPos( itMO->first );

        if ( gridPos != oInfo.OldGridPos )
        {
          m_pGameLayer->RemoveObject( oInfo.OldGridPos, oInfo.pMovedObject );
          m_pGameLayer->AddObject( oInfo.pMovedObject );
        }

        ++itMO;
      }*/
      m_MovedObjects.clear();
    }



    void JREngine::UpdateCamera( GR::f32 ElapsedTime )
    {
      if ( m_pControlledObject )
      {
        GR::tPoint      screenOffset;

        screenOffset.x = m_pControlledObject->Position.x - m_RenderSize.x / 2;
        screenOffset.y = m_pControlledObject->Position.y - m_RenderSize.y / 2;

        m_Map.ActualiseRegion( m_pControlledObject->Bounds() );
        if ( m_Map.CurrentRegion.Width() )
        {
          ClipToRect( screenOffset, m_Map.CurrentRegion );
        }
        else
        {
          ClipToRect( screenOffset, GR::tRect( 0, 0, 
                                               m_pGameLayer->DisplayWidth(),
                                               m_pGameLayer->DisplayHeight() ) );
        }
        m_CurrentOffsetTarget = screenOffset;
        m_CurrentOffset = m_Map.DisplayOffset;

        m_Map.CheckTrigger( m_pControlledObject );
      }

      switch ( m_CameraMode )
      {
        case CAM_FOLLOW_OBJECT:
          if ( m_CameraTargetTime > 0 )
          {
            m_CameraMoveTime += ElapsedTime * 1000.0f;
            if ( m_CameraMoveTime > m_CameraTargetTime )
            {
              m_CameraTargetTime  = 0;
              m_CurrentOffset     = m_CurrentOffsetTarget;
            }
            else
            {
              m_CurrentOffset.x = (int)( m_CameraStartPos.x + ( m_CurrentOffsetTarget.x - m_CameraStartPos.x ) * m_CameraMoveTime / m_CameraTargetTime );
              m_CurrentOffset.y = (int)( m_CameraStartPos.y + ( m_CurrentOffsetTarget.y - m_CameraStartPos.y ) * m_CameraMoveTime / m_CameraTargetTime );
            }
          }
          else
          {
            m_CameraMoveDelay += ElapsedTime;
            while ( m_CameraMoveDelay >= 0.002f )
            {
              GR::tPoint      offset( m_CurrentOffsetTarget - m_CurrentOffset );
              GR::f32         distance = offset.Length();
              const GR::f32   cameraAdjustSpeed = 15.0f;

              if ( cameraAdjustSpeed >= distance )
              {
                m_CurrentOffset = m_CurrentOffsetTarget;
              }
              else
              {
                GR::f32     factor = cameraAdjustSpeed / distance;
                m_CurrentOffset = m_CurrentOffset + GR::tPoint( (int)( offset.x * factor ), (int)( offset.y * factor ) );
              }
              m_CameraMoveDelay -= 0.002f;
            }
          }
          break;
        case CAM_FIXED_ON_OBJECT:
          m_CurrentOffset = m_CurrentOffsetTarget;
          break;
        case CAM_FIXED:
          m_CurrentOffset = m_CameraTargetPos;
          break;
        case CAM_TARGET_POSITION:
          m_CameraMoveTime += ElapsedTime * 1000.0f;
          if ( m_CameraMoveTime > m_CameraTargetTime )
          {
            m_CameraMode = CAM_FIXED;
            m_CurrentOffset = m_CameraTargetPos;
          }
          else
          {
            m_CurrentOffset.x = (int)( m_CameraStartPos.x + ( m_CameraTargetPos.x - m_CameraStartPos.x ) * m_CameraMoveTime / m_CameraTargetTime );
            m_CurrentOffset.y = (int)( m_CameraStartPos.y + ( m_CameraTargetPos.y - m_CameraStartPos.y ) * m_CameraMoveTime / m_CameraTargetTime );
          }
          break;
      }
      m_Map.DisplayOffset = m_CurrentOffset;
    }



    void JREngine::AwakenObject( GameObject* pObj )
    {
      if ( !( pObj->m_ProcessingFlags & ProcessingFlags::AWAKENED ) )
      {
        if ( pObj->Flags & GR::Gamebase::LayerObject::OF_SPAWNABLE )
        {
          if ( pObj->Flags & GR::Gamebase::LayerObject::OF_SPAWNED )
          {
            // already spawned
            return;
          }
          pObj->Flags |= GR::Gamebase::LayerObject::OF_SPAWNED;

          GameObject* pClone = (GameObject*)m_Map.AddObject( m_MainLayer, pObj->Template, pObj->Position.x, pObj->Position.y );
          pClone->m_pSpawnAncestor  = pObj;
          pObj = pClone;
        }
        else
        {
          RemoveObjectFromGrid( pObj );
        }

        //dh::Log( "Awaken Object (%s/%x/%d)", pObj->Template.c_str(), pObj, pObj->ExtraDataID );
        pObj->m_ProcessingFlags |= ProcessingFlags::AWAKENED;
        pObj->m_ProcessingFlags &= ~ProcessingFlags::SEND_TO_SLEEP;

        if ( std::find( m_AwakeObjects.begin(), m_AwakeObjects.end(), pObj ) != m_AwakeObjects.end() )
        {
          dh::Log( "Object is already in awake object list!" );
        }
        else
        {
          if ( pObj->ZOrder > 0 )
          {
            // find place to sort object into
            bool    added = false;
            std::list<GR::Gamebase::GameObject*>::iterator    it( m_AwakeObjects.begin() );
            while ( it != m_AwakeObjects.end() )
            {
              GR::Gamebase::GameObject*   pOtherObj( *it );

              if ( pOtherObj->ZOrder > pObj->ZOrder )
              {
                added = true;
                m_AwakeObjects.insert( it, pObj );
                break;
              }
              ++it;
            }
            if ( !added )
            {
              m_AwakeObjects.push_back( pObj );
            }
          }
          else
          {
            m_AwakeObjects.push_back( pObj );
          }
          RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::AWAKEN ) );
        }
      }
    }



    bool JREngine::SleepObject( GameObject* pObj )
    {
      if ( pObj->Flags & GR::Gamebase::LayerObject::OF_ALWAYS_AWAKE )
      {
        pObj->m_ProcessingFlags &= ~ProcessingFlags::SEND_TO_SLEEP;
        return false;
      }
      if ( pObj->m_ProcessingFlags & ProcessingFlags::AWAKENED )
      {
        if ( pObj->m_pSpawnAncestor )
        {
          // was a clone, notify parent
          pObj->m_pSpawnAncestor->Flags &= ~GR::Gamebase::LayerObject::OF_SPAWNED;
          pObj->m_pSpawnAncestor = NULL;
          pObj->DeleteMe();
        }
        else
        {
          //dh::Log( "Add Object (%s/%x)", pObj->Template.c_str(), pObj );
          AddObjectToGrid( pObj );
          //dh::Log( "Sleep Object (%s/%x), remove", pObj->Template.c_str(), pObj );
          pObj->SendToSleep();
        }
        //dh::Log( "Sleep Object (%s/%x/%d)", pObj->Template.c_str(), pObj, pObj->ExtraDataID );
        pObj->m_ProcessingFlags &= ~( ProcessingFlags::AWAKENED | ProcessingFlags::SEND_TO_SLEEP );

        RaiseObjectEvent( pObj, GR::Gamebase::ObjectEvent( ObjectEventType::TO_SLEEP ) );
      }
      return true;
    }



    void JREngine::AwakenObjects( const GR::tPoint& Sector )
    {
      if ( m_AwakeSectors.find( Sector ) != m_AwakeSectors.end() )
      {
        return;
      }
      std::map<GR::tPoint,std::list<LayerObject*> >::iterator   itGrid( m_pGameLayer->Objects.find( Sector ) );
      if ( itGrid == m_pGameLayer->Objects.end() )
      {
        return;
      }
      std::list<LayerObject*>&    listObj( itGrid->second );

      //dh::Log( "Awaken sector %d,%d (%d objects)", Sector.x, Sector.y, listObj.size() );

      std::list<LayerObject*>::iterator   itO( listObj.begin() );
      while ( itO != listObj.end() )
      {
        GameObject*    pLayerObj( (GameObject*)*itO );

        if ( pLayerObj->Flags & GR::Gamebase::LayerObject::OF_NOT_AWAKENABLE )
        {
          ++itO;
          continue;
        }

        if ( !( pLayerObj->Flags & GR::Gamebase::LayerObject::OF_SPAWNABLE ) )
        {
          itO = listObj.erase( itO );
        }
        else
        {
          ++itO;
        }
        AwakenObject( pLayerObj );
      }
    }



    void JREngine::SleepObjects( const GR::tPoint& Sector )
    {
      std::set<GR::tPoint>::iterator    itAS( m_AwakeSectors.find( Sector ) );
      if ( itAS == m_AwakeSectors.end() )
      {
        return;
      }
      std::map<GR::tPoint,std::list<LayerObject*> >::iterator   itGrid( m_pGameLayer->Objects.find( Sector ) );
      if ( itGrid == m_pGameLayer->Objects.end() )
      {
        return;
      }

      std::list<LayerObject*>&    listObj( itGrid->second );

      //dh::Log( "Sleep sector %d,%d (%d objects)", Sector.x, Sector.y, listObj.size() );

      std::list<LayerObject*>::iterator   itO( listObj.begin() );
      while ( itO != listObj.end() )
      {
        GameObject*    pLayerObj( (GameObject*)*itO );

        ++itO;

        if ( ( !( pLayerObj->Flags & GR::Gamebase::LayerObject::OF_ALWAYS_AWAKE ) )
        &&   ( pLayerObj != m_pControlledObject ) )
        {
          SleepObject( pLayerObj );
        }
      }
    }



    void JREngine::AwakenAllObjects( bool Permanent )
    {
      m_AllObjectsActive = Permanent;
      if ( m_pGameLayer )
      {
        std::map<GR::tPoint,std::list<GR::Gamebase::LayerObject*> >::iterator   itLO( m_pGameLayer->Objects.begin() );
        while ( itLO != m_pGameLayer->Objects.end() )
        {
          AwakenObjects( itLO->first );
          ++itLO;
        }
      }
    }



    void JREngine::SetObjectEventHandler( GameObject* pObject, GR::Gamebase::ObjectEventType::Value EventType, tObjectEventHandlerFunction Function )
    {
      m_ObjectEventHandler[std::make_pair( EventType, pObject )] = Function;
    }



    void JREngine::SetControlledObject( GameObject* pObject, CameraMode Mode, int MilliSeconds )
    {
      m_pControlledObject = pObject;
      m_CameraMode        = Mode;
      m_CameraMoveTime    = 0;
      m_CameraTargetTime  = MilliSeconds;
      m_CameraStartPos    = m_CurrentOffset;
    }



    void JREngine::CameraMoveTo( int X, int Y, int TimeMS )
    {
      m_CameraMode = CAM_TARGET_POSITION;
      m_CameraStartPos = m_Map.DisplayOffset;
      m_CameraTargetPos.Set( X - m_RenderSize.x / 2, Y - m_RenderSize.y / 2 );
      m_CameraTargetTime = TimeMS;
      m_CameraMoveTime = 0.0f;
    }



    bool JREngine::MovieMode()
    {
      return m_MovieMode;
    }



    int JREngine::MovieStart( LuaInstance& Lua )
    {
      Lua.PopAll();

      s_pJREngine->m_MovieMode = true;
      return 0;
    }



    int JREngine::MovieEnd( LuaInstance& Lua )
    {
      Lua.PopAll();

      s_pJREngine->m_MovieMode = false;
      return 0;
    }



    int JREngine::CameraMoveTo( LuaInstance& Lua )
    {
      if ( Lua.GetTop() == 3 )
      {
        // X, Y, time (ms)
        s_pJREngine->CameraMoveTo( Lua.ToNumber( 1 ), Lua.ToNumber( 2 ), Lua.ToNumber( 3 ) );
      }
      else if ( Lua.GetTop() == 2 )
      {
        // X, Y
        s_pJREngine->CameraMoveTo( Lua.ToNumber( 1 ), Lua.ToNumber( 2 ) );
      }
      else
      {
        dh::Log( "JREngine::CameraMoveTo expected X,Y or X,Y,time(ms)" );
      }
      Lua.PopAll();
      return 0;
    }



    int JREngine::GetCurrentCameraPosition( LuaInstance& Lua )
    {
      Lua.PopAll();

      int   camX = s_pJREngine->m_CurrentOffset.x;
      int   camY = s_pJREngine->m_CurrentOffset.y;

      Lua.PushNumber( camY );
      Lua.PushNumber( camX );
      return 2;
    }



    int JREngine::SetFeld( LuaInstance& Lua )
    {
      if ( Lua.GetTop() == 4 )
      {
        // Layer, X, Y, Tile
        auto pLayer = s_pJREngine->m_Map.LayerByName( Lua.ToString( 1 ) );
        if ( ( pLayer != NULL )
        &&   ( ( pLayer->Type == GR::Gamebase::Layer::LayerType::LT_COLLISION_LAYER )
        ||     ( pLayer->Type == GR::Gamebase::Layer::LayerType::LT_TILE_LAYER ) ) )
        {
          GR::Gamebase::TileLayer*  pTileLayer = ( GR::Gamebase::TileLayer* )pLayer;

          pTileLayer->SetField( Lua.ToNumber( 2 ), Lua.ToNumber( 3 ), (GR::u16)Lua.ToNumber( 4 ) );
        }
      }
      else
      {
        dh::Log( "JREngine::SetFeld expected Layer, X, Y, Tile" );
      }
      Lua.PopAll();
      return 0;
    }



    int JREngine::CameraFollowControlledObject( LuaInstance& Lua )
    {
      int     time = 0;
      if ( Lua.GetTop() == 1 )
      {
        // move to player pos, ms
        time = Lua.ToNumber( -1 );
      }
      Lua.PopAll();

      s_pJREngine->SetControlledObject( s_pJREngine->m_pControlledObject, GR::Gamebase::JREngine::CAM_FOLLOW_OBJECT, time );
      return 0;
    }



    int JREngine::HidePlayer( LuaInstance& Lua )
    {
      Lua.PopAll();

      if ( s_pJREngine->m_pControlledObject )
      {
        s_pJREngine->m_pControlledObject->Flags |= GR::Gamebase::LayerObject::OF_HIDDEN;
      }
      return 0;
    }



    int JREngine::ShowPlayer( LuaInstance& Lua )
    {
      Lua.PopAll();

      if ( s_pJREngine->m_pControlledObject )
      {
        s_pJREngine->m_pControlledObject->Flags &= ~GR::Gamebase::LayerObject::OF_HIDDEN;
      }
      return 0;
    }



    int JREngine::UnfreezePlayer( LuaInstance& Lua )
    {
      Lua.PopAll();

      if ( s_pJREngine->m_pControlledObject )
      {
        s_pJREngine->m_pControlledObject->m_ProcessingFlags &= ~GR::Gamebase::ProcessingFlags::NO_UPDATE;
      }
      return 0;
    }



    int JREngine::FreezePlayer( LuaInstance& Lua )
    {
      Lua.PopAll();

      if ( s_pJREngine->m_pControlledObject )
      {
        s_pJREngine->m_pControlledObject->m_ProcessingFlags |= GR::Gamebase::ProcessingFlags::NO_UPDATE;
      }
      return 0;
    }



    int JREngine::SetGameVar( LuaInstance& Lua )
    {
      if ( Lua.GetTop() == 2 )
      {
        s_pJREngine->m_GameVars.SetVar( Lua.ToString( -2 ), Lua.ToString( -1 ) );
      }

      Lua.PopAll();
      return 0;
    }



    int JREngine::GetGameVar( LuaInstance& Lua )
    {
      GR::String    result;

      if ( Lua.GetTop() == 1 )
      {
        result = s_pJREngine->m_GameVars.GetVar( Lua.ToString( -1 ) );
      }

      Lua.PopAll();
      Lua.PushString( result );
      return 1;
    }



    void JREngine::SetObjectUserData( GR::up UserData )
    {
      m_ObjectUserData = UserData;
    }



    void JREngine::SetCameraMode( CameraMode Mode )
    {
      m_CameraMode = Mode;
    }



    GameObject* JREngine::ControlledObject()
    {
      return m_pControlledObject;
    }



    void JREngine::SetObjectOnTop( GameObject* pObject )
    {
      // TODO - check, if object actually inside
      size_t  origSize = m_AwakeObjects.size();
      m_AwakeObjects.remove( pObject );
      if ( origSize != m_AwakeObjects.size() )
      {
        m_AwakeObjects.push_back( pObject );
      }
    }



    int JREngine::GetObjectCountByType( const GR::String& ObjType )
    {
      int     count = m_pGameLayer->GetObjectCountByType( ObjType );

      auto itAO( m_AwakeObjects.begin() );
      while ( itAO != m_AwakeObjects.end() )
      {
        auto& awakeObject( *itAO );

        if ( awakeObject->Template == ObjType )
        {
          ++count;
        }

        ++itAO;
      }

      return count;
    }



    GR::Game::ExtraData* JREngine::ExtraData( GR::u32 ExtraDataID )
    {
      auto  itED( m_Map.ExtraDatas.find( ExtraDataID ) );
      if ( itED == m_Map.ExtraDatas.end() )
      {
        return NULL;
      }
      return &itED->second;
    }



    GR::String JREngine::CurrentMap()
    {
      return m_CurrentMap;
    }

  }
}