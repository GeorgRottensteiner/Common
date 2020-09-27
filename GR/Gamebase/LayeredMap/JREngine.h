#ifndef GR_GAMEBASE_JR_ENGINE_H
#define GR_GAMEBASE_JR_ENGINE_H

#include "LayeredMap.h"
#include "TileLayer.h"
#include "FlagLayer.h"
#include "ScriptHandler.h"

#include <Interface/IAnimationManager.h>

#include <GR/GRTypes.h>
#include <GR/LocalRegistry/LocalRegistry.h>
#include <Grafik/ContextDescriptor.h>

#include <map>
#include <list>



namespace Xtreme
{
  class XInput;
}



namespace GR
{
  namespace Gamebase
  {
    namespace ObjectEventType
    {
      enum Value;
    }
    class GameObject;
    class ObjectEvent;

    namespace Dir
    {
      enum Value
      {
        NONE  = 0,
        U     = 0x00000001,
        N     = U,
        D     = 0x00000002,
        S     = D,
        L     = 0x00000004,
        W     = L,
        R     = 0x00000008,
        E     = R,
        ALL   = 0x0000000f
      };
    }

    namespace TileType
    {
      enum Value
      {
        FREE              = 0,
        BLOCKING          = 0x0000000F,
        BLOCKING_LEFT     = 0x00000001,
        BLOCKING_RIGHT    = 0x00000002,
        BLOCKING_TOP      = 0x00000004,
        BLOCKING_BOTTOM   = 0x00000008,
        PLATFORM          = BLOCKING_TOP,
        DEADLY            = 0x000000F0,
        DEADLY_LEFT       = 0x00000010,
        DEADLY_RIGHT      = 0x00000020,
        DEADLY_TOP        = 0x00000040,
        DEADLY_BOTTOM     = 0x00000080,
        FLOAT             = 0x00000F00,
        FLOAT_DOWN        = 0x00000100,
        FLOAT_UP          = 0x00000200,
        FLOAT_LEFT        = 0x00000400,
        FLOAT_RIGHT       = 0x00000800,
        CONVEYOR_LEFT     = 0x00001000,
        CONVEYOR_RIGHT    = 0x00002000,
        WATER             = 0x00010000,
        CLIMBABLE         = 0x00020000,
        CUSTOM_COLLISION  = 0x00040000,
        FENCE_DOWN        = 0x00100000,
        FENCE_UP          = 0x00200000,
        FENCE_LEFT        = 0x00400000,
        FENCE_RIGHT       = 0x00800000,
        FENCE_ALL         = FENCE_DOWN | FENCE_UP | FENCE_LEFT | FENCE_RIGHT
      };
    }


    namespace MoveResult
    {
      enum Value
      {
        OK                = 0,
        BLOCKED_LEFT      = 0x00000001,
        BLOCKED_RIGHT     = 0x00000002,
        BLOCKED_UP        = 0x00000004,
        BLOCKED_DOWN      = 0x00000008,
        BLOCKED           = BLOCKED_LEFT | BLOCKED_RIGHT | BLOCKED_UP | BLOCKED_DOWN
      };
    }

    namespace CollisionReaction
    {
      enum Value
      {
        NONE             = 0,
        STOP_MOVER       = 0x00000001,
        STOP_OBSTACLE    = 0x00000002,
        KILL_MOVER       = 0x00000004,
        KILL_OBSTACLE    = 0x00000008
      };
    } 

    namespace TileBlockReaction
    {
      enum Value
      {
        NONE            = 0,
        STOP_MOVER      = 0x00000001,
        KILL_MOVER      = 0x00000002,
        REPLACE_TILE    = 0x00010000,
        REPLACE_FLAGS   = 0x00020000
      };
    }



    Dir::Value  DirFromDelta( const GR::tPoint& Delta );
    Dir::Value  DirFromDelta( int DX, int DY );
    Dir::Value  DirFromDelta( GR::f32 DX, GR::f32 DY );
    GR::f32     AngleFromDir( Dir::Value Dir );
    GR::tPoint  DeltaFromDir( GR::u32 Dir );

    class JREvent
    {
      public:

        enum JREventType
        {
          JRE_NONE = 0,
          JRE_MAP_CHANGED,
          JRE_LEAVE_MAP,
          JRE_CONTROLLED_OBJECT_CHANGED,
          JRE_BLOCKED_MOVEMENT,
          JRE_BLOCKED_MOVEMENT_BY_OTHER_OBJECT
        };

        JREventType       Type;
        GameObject*       pObject;
        GR::tPoint        Pos;
        GR::String        Param;
        GR::up            Param2;


        JREvent( JREventType Type = JRE_NONE, const GR::String& Param = GR::String() ) :
          Type( Type ),
          Param( Param ),
          Param2( 0 )
        {
        }

        JREvent( JREventType Type, GameObject* pObj ) :
          Type( Type ),
          pObject( pObj ),
          Param2( 0 )
        {
        }
    };

    class JREngine
    {
      public:

        enum CameraMode
        {
          CAM_FIXED,
          CAM_FOLLOW_OBJECT,
          CAM_FIXED_ON_OBJECT,
          CAM_TARGET_POSITION
        };



        struct TileBlockEvent
        {
          const GameObject*     pObj;
          GR::u16               Tile;
          GR::u32               Flags;
          GR::u32               MoveDir;
          GR::tPoint            TilePos;
          GR::u32               TileBlockReaction;
          TileLayer*            pLayer;


          TileBlockEvent( GameObject* pObj, GR::u16 Tile, GR::u32 Flags, const GR::tPoint& TilePos, GR::u32 MoveDir, TileLayer* pLayer ) :
            pObj( pObj ),
            Tile( Tile ),
            Flags( Flags ),
            TilePos( TilePos ),
            TileBlockReaction( TileBlockReaction::NONE ),
            MoveDir( MoveDir ),
            pLayer( pLayer )
          {
          }
        };



        ScriptHandler           m_ScriptHandler;


      protected:

        struct MovedObjectInfo
        {
          GameObject*     pMovedObject;
          GR::tPoint      OldGridPos;


          MovedObjectInfo( GameObject* pMovedObject = NULL, const GR::tPoint& OldGridPos = GR::tPoint() ) :
            pMovedObject( pMovedObject ),
            OldGridPos( OldGridPos )
          {
          }
        };

        typedef fastdelegate::FastDelegate3<GameObject*,GameObject*,GR::Gamebase::Dir::Value,bool>      tCanCollideEventFunction;
        typedef fastdelegate::FastDelegate3<GameObject*,GameObject*,GR::Gamebase::Dir::Value,bool>      tObjectBlocksObjectEventFunction;
        typedef fastdelegate::FastDelegate2<GameObject*,GR::Gamebase::Dir::Value,bool>                  tObjectBlocksFunction;
        typedef fastdelegate::FastDelegate3<GameObject*,GameObject*,GR::Gamebase::Dir::Value,GR::u32>   tCollisionHandlerEventFunction;
        typedef fastdelegate::FastDelegate1<TileBlockEvent&,bool>                                       tTileBlockHandlerEventFunction;
        typedef fastdelegate::FastDelegate2<GameObject*,const GR::Gamebase::ObjectEvent&>               tObjectEventHandlerFunction;
        typedef fastdelegate::FastDelegate1<const GR::Gamebase::JREvent&>                               tJREventHandlerFunction;

        typedef std::map<std::pair<GR::Gamebase::ObjectEventType::Value,GR::Gamebase::GameObject*>,tObjectEventHandlerFunction>    tObjectEventHandler;


        GameObject*             m_pControlledObject;

        GR::tPoint              m_RenderSize;
        GR::tPoint              m_CurrentOffsetTarget;
        GR::tPoint              m_CurrentOffset;

        CameraMode              m_CameraMode;
        GR::tPoint              m_CameraStartPos;
        GR::f32                 m_CameraMoveTime;
        GR::tPoint              m_CameraTargetPos;
        int                     m_CameraTargetTime;
        GR::f32                 m_CameraMoveDelay;

        GR::up                  m_ObjectUserData;

        GR::String              m_CurrentMap;
        GR::String              m_MapPath;
        GR::String              m_MainLayer;
        GR::String              m_ControlObjectName;

        LayeredMap::tAddObjectHandlerFunction   m_AddObjectHandler;

        tCanCollideEventFunction          m_CanCollideHandler;
        tObjectBlocksObjectEventFunction  m_ObjectBlocksObjectHandler;
        tObjectBlocksFunction             m_ObjectBlockHandler;
        tCollisionHandlerEventFunction    m_CollisionHandler;
        tTileBlockHandlerEventFunction    m_TileBlockHandler;
        tObjectEventHandler               m_ObjectEventHandler;
        tJREventHandlerFunction           m_JREventHandler;

        std::map<GameObject*,MovedObjectInfo>   m_MovedObjects;

        std::list<std::pair<GR::u32,GameObject*> >      m_ExtraDataQueue;

        std::map<GR::String, GR::Graphic::ContextDescriptor>     m_CollisionImageCache;

        bool                    m_MapChanged;

        bool                    m_MovieMode;

        LocalRegistry           m_GameVars;



        bool                    OnLayeredMapEvent( const GR::Gamebase::LayeredMap::LayeredMapEvent& Event );


        




      public:

        TileLayer*              m_pCollisionLayer;
        TileLayer*              m_pGameLayer;
        FlagLayer*              m_pFlagLayer;

        LayeredMap              m_Map;

        std::list<GameObject*>  m_AwakeObjects;
        std::set<GR::tPoint>    m_AwakeSectors;
        bool                    m_AllObjectsActive;

        IAnimationManager<GR::String>*      m_pAnimManager;

        std::vector<GR::u32>    m_TileFlags;

        GR::f32                 m_GravityFactor;



        JREngine( const GR::tPoint& RenderSize, IAnimationManager<GR::String>* pAnimManager );
        ~JREngine();


        void                    InitMap( const GR::String& MapName, const GR::String& MainLayerName = "Main", const GR::String& PlayerObject = "Player" );
        void                    WarpToMap( const GR::String& MapName, int X, int Y );

        void                    SetRenderSize( const GR::tPoint& Size );

        void                    ClearObjects();
        void                    ClearMap();

        void                    AddObjectToGrid( GameObject* pObj );
        GameObject*             SpawnObject( const GR::String& ObjType, int X, int Y, GR::u32 Flags = 0, bool Sleeping = false );

        void                    RemoveObjectFromGrid( GameObject* pObj );

        void                    RemoveObject( GameObject* pObj );
        GR::u32                 MoveObject( GameObject* pObj, GR::f32 DX, GR::f32 DY, GR::Gamebase::Layer* pLayer = NULL, GameObject* pMovingParent = NULL );
        GR::u32                 MoveObjectFixed( GameObject* pObj, int DX, int DY, GR::Gamebase::Layer* pLayer = NULL, GameObject* pMovingParent = NULL );
        GR::u32                 MoveObjectNonBlocking( GameObject* pObj, GR::f32 DX, GR::f32 DY );
        GR::u32                 MoveObjectNonBlockingFixed( GameObject* pObj, GR::i32 DX, GR::i32 DY );
        void                    SetObjectPos( GameObject* pObj, const GR::tPoint& NewPos );

        void                    AwakenObject( GameObject* pObj );
        bool                    SleepObject( GameObject* pObj );
        void                    AwakenObjects( const GR::tPoint& Sector );
        void                    SleepObjects( const GR::tPoint& Sector );
        void                    AwakenAllObjects( bool Permanent = false );

        void                    SetObjectUserData( GR::up UserData );
        void                    SetObjectOnTop( GameObject* pObject );

        GameObject*             ControlledObject();

        GameObject*             FindObjectByType( const GR::String& Layer, const GR::String& ObjType );
        GameObject*             FindObjectByType( const GR::String& Type );
        GameObject*             FindObjectByID( GR::u32 ID );
        bool                    FindObjectsAt( const GR::tPoint& Position, std::list<GameObject*>& ObjectList );
        bool                    FindObjectsInArea( const GR::tRect& Area, GR::Gamebase::Layer* pLayer, std::list<GameObject*>& ObjectList );
        GameObject*             FindCollidingObject( GameObject* pCollider, const GR::String& Type );
        GameObject*             FindCollidingObject( GameObject* pCollider, int Type );
        int                     GetObjectCountByType( const GR::String& ObjType );

        bool                    CheckCollisions( GameObject* pObj, const GR::tRect& Bounds, Dir::Value Dir );
        bool                    IsObjectBlockedByOtherObjects( GameObject* pObj, const GR::tRect& Bounds, GR::Gamebase::Dir::Value Dir, GameObject* pMovingParent = NULL );
        bool                    IsBlockedByObjects( const GR::tRect& Bounds, GR::Gamebase::Dir::Value Dir );

        bool                    IsAreaBlocked( GameObject* pObj, int DX, int DY, Dir::Value Dir, GameObject* pMovingParent = NULL );
        bool                    IsAreaBlocked( GameObject* pObj, const GR::tRect& Bounds, Dir::Value Dir, GameObject* pMovingParent = NULL );
        bool                    IsAreaBlocked( GameObject* pObj, int DX, int DY, Dir::Value Dir, const GR::String& Layer, GameObject* pMovingParent = NULL );
        bool                    IsAreaBlocked( GameObject* pObj, const GR::tRect& Bounds, Dir::Value Dir, const GR::String& Layer, GameObject* pMovingParent = NULL );
        bool                    IsAreaBlocked( GameObject* pObj, const GR::tRect& Bounds, Dir::Value Dir, GR::Gamebase::Layer* pLayer, GameObject* pMovingParent = NULL );

        bool                    IsTileBlocking( GameObject* pObj, int TileX, int TileY, Dir::Value DIR, GR::Gamebase::TileLayer* pLayer = NULL );

        void                    CenterCameraOnObject( GameObject* pObj );
        void                    CenterCameraOnPosition( const GR::tPoint& Pos );

        void                    RunScriptFromFile( const GR::String& Filename );
        void                    RunMapScript( const GR::String& ScriptName );
        void                    RunScript( const GR::String& Script );
        void                    RegisterScript( lua_CFunction pFunction, const GR::String& Name );
        void                    RegisterScript( LuaInstance::tHandlerFunction Function, const GR::String& Name );

        void                    RaiseObjectEvent( GameObject* pObj, const ObjectEvent& Event );
        void                    RaiseJREvent( const JREvent& Event );

        void                    ProcessExtraData( GR::u32 ExtraDataID, GameObject* pObject = NULL );

        GR::String              CurrentMap();

        void                    SetAddObjectHandler( LayeredMap::tAddObjectHandlerFunction Function );
        void                    SetObjectEventHandler( GameObject* pObject, GR::Gamebase::ObjectEventType::Value EventType, tObjectEventHandlerFunction Function );
        void                    SetCanCollideHandler( tCanCollideEventFunction Function );
        void                    SetObjectBlocksObjectHandler( tObjectBlocksObjectEventFunction Function );
        void                    SetObjectBlocksHandler( tObjectBlocksFunction Function );
        void                    SetCollisionHandler( tCollisionHandlerEventFunction Function );
        void                    SetTileBlockHandler( tTileBlockHandlerEventFunction Function );
        void                    SetJREventHandler( tJREventHandlerFunction Function );

        void                    Update( Xtreme::XInput* pInput, const GR::f32 ElapsedTime );
        void                    UpdateFixedStep( Xtreme::XInput* pInput, const GR::f32 ElapsedTime );
        void                    UpdateScripts( const GR::f32 ElapsedTime );

        void                    HandleMovementPath( GR::Gamebase::GameObject& Object, GR::f32 ElapsedTime );

        void                    SetControlledObject( GameObject* pObject, CameraMode Mode = CAM_FOLLOW_OBJECT, int MilliSeconds = 0 );

        bool                    IsOutsideAwakeArea( GR::Gamebase::GameObject* pObj );

        static GR::Gamebase::Dir::Value   OppositeDir( GR::Gamebase::Dir::Value Dir );


        void                    FireTrigger( const GR::Gamebase::LayeredMap::Trigger& Trigger, GameObject* pObj );

        void                    CameraMoveTo( int X, int Y, int TimeMS = 0 );
        void                    SetCameraMode( CameraMode Mode );

        bool                    MovieMode();

        void                    UpdateCamera( GR::f32 ElapsedTime );


        GR::Game::ExtraData*    ExtraData( GR::u32 ExtraDataID );



      protected:

        GR::u32                 FieldFlags( GR::Gamebase::TileLayer* pLayer, int X, int Y );

        void                    ClipToRect( GR::tPoint& Pos, const GR::tRect& ClipRect );

        void                    OnCollide( GameObject* pObj1, GameObject* pObj2, GR::Gamebase::Dir::Value Dir, GR::u32& CollisionReaction );
        bool                    CanCollide( GameObject* pObj1, GameObject* pObj2, GR::Gamebase::Dir::Value Dir );
        bool                    DoesObjectBlockObject( GR::Gamebase::GameObject* pMover, GR::Gamebase::GameObject* pObstacle, GR::Gamebase::Dir::Value Dir );
        bool                    DoesObjectBlock( GameObject* pObstacle, GR::Gamebase::Dir::Value Dir );

        bool                    Collide( GameObject* pObj1, GameObject* pObj2, GR::Gamebase::Dir::Value Dir );

        void                    PerformMove( GameObject* pObj, int DX, int DY );
        void                    ReseatMovedObjectsInSector();

        void                    DetermineFlagLayer();

        void                    ObjectIsDeleted( GameObject* pObj );

        void                    HandleEvent( const GR::Gamebase::LayeredMap::LayeredMapEvent& Event );
        void                    HandleExtraData( const GR::Game::ExtraData& ExtraData, GameObject* pObj = NULL, bool NoQueueing = false );

        GR::String              MapPath( const GR::String& MapName );

        const GR::Graphic::ContextDescriptor& FetchCollisionCD( TileLayer* pLayer, GR::u16 Field );


        static int              MovieStart( LuaInstance& Lua );
        static int              MovieEnd( LuaInstance& Lua );
        static int              CameraMoveTo( LuaInstance& Lua );
        static int              CameraFollowControlledObject( LuaInstance& Lua );
        static int              SetFeld( LuaInstance& Lua );
        static int              ShowPlayer( LuaInstance& Lua );
        static int              HidePlayer( LuaInstance& Lua );
        static int              FreezePlayer( LuaInstance& Lua );
        static int              UnfreezePlayer( LuaInstance& Lua );
        static int              SetGameVar( LuaInstance& Lua );
        static int              GetGameVar( LuaInstance& Lua );
        static int              GetCurrentCameraPosition( LuaInstance& Lua );


        void                    InitObjectExtraData( GR::Gamebase::GameObject& Object );

    };

  }
}


#endif // GR_GAMEBASE_JR_ENGINE_H
