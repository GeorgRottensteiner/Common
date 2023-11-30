#ifndef GR_GAMEBASE_LAYERED_MAP_H
#define GR_GAMEBASE_LAYERED_MAP_H



#include <Lang/FastDelegate.h>

#include <Interface/IIOStream.h>

#include <vector>
#include <set>

#include "Layer.h"
#include "ObjectLayer.h"

#include <Game/ExtraData.h>
#include <Game/MovementPath.h>



namespace GR
{
  namespace Gamebase
  {

    namespace FileChunks
    {
      enum FileChunkTypes
      {
        // layered map
        MAP                 = 0x7000,     // map base info
        LAYER_TILES         = 0x7010,
        LAYER_FREE_SECTION  = 0x7011,
        LAYER_FLAGS         = 0x7012,
        LAYER_OBJECTS       = 0x7020,
        LAYER_OBJECT        = 0x7021,
        MAP_REGION          = 0x7030,
        MAP_TRIGGER         = 0x7031,
        MAP_EXTRADATA       = 0x7032,
        MAP_MOVEMENT_PATH   = 0x7033,

        SCRIPT              = 0x8000
      };
    }

    class LayeredMap
    {
      public:

        struct LayeredMapEvent
        {
          enum EventType
          {
            ET_UNKNOWN,
            ET_TRIGGER_ENTER,
            ET_TRIGGER_INSIDE,
            ET_TRIGGER_LEAVE,
            ET_TRIGGER_TRIGGERED
          };

          LayerObject*  pObject;
          LayerObject*  pOtherObject;
          EventType     Type;
          GR::u32       ElementID;

          LayeredMapEvent( EventType Type = ET_UNKNOWN, LayerObject* pObj = NULL ) :
            Type( Type ),
            pObject( pObj ),
            pOtherObject( NULL ),
            ElementID( 0 )
          {
          }
        };

        struct Trigger
        {
          enum TriggerType
          {
            TT_UNKNOWN = 0,
            TT_EXTRA_DATA
          };

          enum Flags
          {
            NONE                  = 0,
            TRIGGER_ON_ENTER      = 0x00000001,
            TRIGGER_ON_INSIDE     = 0x00000002,
            TRIGGER_ON_LEAVE      = 0x00000004,
            TRIGGER_ONCE          = 0x00000008,
            TRIGGER_ON_ACTION     = 0x00000010,
            TRIGGERED             = 0x00010000
          };

          TriggerType     Type;
          GR::u32         Flags;
          GR::String      EnterScript;
          GR::String      LeaveScript;
          GR::u32         ID;
          GR::u32         ExtraDataID;
          GR::tRect       Rect;


          Trigger() :
            Type( TT_UNKNOWN ),
            Flags( TRIGGER_ON_ENTER ),
            ID( -1 ),
            ExtraDataID( -1 )
          {
          }
        };

        typedef std::map<GR::u32,Trigger>                       tTrigger;
        typedef std::map<GR::u32,GR::Game::ExtraData>           tExtraData;

        // const GR::String& Template, int X, int Y, GR::u32, bool& AddInBack
        typedef fastdelegate::FastDelegate5<const GR::String&,int,int,GR::u32,bool&,LayerObject*> tAddObjectHandlerFunction;
        typedef fastdelegate::FastDelegate1<const LayeredMapEvent&,bool>             tLayeredMapEventFunction;


      private:


        typedef std::map<GR::tPoint,std::list<GR::tRect> >            tRegion;
        typedef std::map<GR::tPoint,std::set<GR::u32> >               tTriggerGrid;
        typedef std::map<GR::String, GR::Game::Path::MovementPath>    tMovementPath;

        tAddObjectHandlerFunction m_AddObjectHandler;
        tLayeredMapEventFunction  m_EventHandler;



      public:

        std::vector<Layer*>       Layers;

        std::map<GR::String, GR::String>     Scripts;

        tRegion                   Regions;

        tTriggerGrid              TriggerGrid;
        tTrigger                  Triggers;
        tExtraData                ExtraDatas;

        GR::tPoint                DisplayOffset;

        GR::tRect                 CurrentRegion;

        tMovementPath             MovementPaths;


        LayeredMap();
        ~LayeredMap();


        void                      Clear();
        bool                      Load( IIOStream& ioIn );

        LayerObject*              AddObject( const GR::String& Layername, const GR::String& ObjType, int X, int Y, GR::u32 Flags = 0 );
        LayerObject*              AddObjectOnTop( const GR::String& Layername, const GR::String& ObjType, int X, int Y, GR::u32 Flags = 0 );

        void                      SetAddObjectHandler( tAddObjectHandlerFunction Function );
        void                      SetEventHandler( tLayeredMapEventFunction Function );

        bool                      RaiseEvent( const LayeredMapEvent& Event );


        LayerObject*              FindObjectByType( const GR::String& Layername, const GR::String& ObjType );
        LayerObject*              FindObjectByType( const GR::String& ObjType );
        LayerObject*              FindObjectByID( GR::u32 ObjectID, ObjectLayer* pLayer = NULL );

        Layer*                    LayerByName( const GR::String& Layername );

        void                      AddRegion( const GR::tRect& Region );
        bool                      ActualiseRegion( const GR::tRect& Bounds );
        bool                      ActualiseRegion( const GR::tPoint& Pos );

        void                      AddTrigger( const Trigger& Trigger );
        bool                      CheckTrigger( LayerObject* pObject );

        GR::Game::ExtraData*      AddExtraData();
        void                      SetExtraData( GR::u32 ID, const GR::Game::ExtraData& Data );
        GR::Game::ExtraData*      FindExtraData( GR::u32 ExtraDataID );


    };

  }
}

#endif // GR_GAMEBASE_LAYERED_MAP_H
