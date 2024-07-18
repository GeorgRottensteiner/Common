#ifndef GR_GAMEBASE_OBJECT_LAYER_H
#define GR_GAMEBASE_OBJECT_LAYER_H

#include "Layer.h"

#include <GR/GRTypes.h>

#include <map>
#include <list>



namespace GR
{
  namespace Gamebase
  {
    class LayerObject
    {
      public:

        enum ObjectInfoFlags
        {
          OF_NONE                 = 0,
          OF_SPAWNABLE            = 0x00000001,
          OF_SPAWNED              = 0x00000002,
          OF_RESPAWN_OFFSCREEN    = 0x00000004,
          OF_NOT_AWAKENABLE       = 0x00000008,
          OF_ALWAYS_AWAKE         = 0x00000010,
          OF_ON_PLATFORM          = 0x00004000,
          OF_CLIMBING             = 0x00008000,
          OF_REVERSE_GRAVITY      = 0x00010000,
          OF_ON_GROUND            = 0x00020000,
          OF_FLOATING             = 0x00040000,
          OF_JUMPING              = 0x00080000,
          OF_HMIRROR              = 0x00100000,
          OF_VMIRROR              = 0x00200000,
          OF_DECORATION           = 0x00400000,     // no collision checks
          OF_NO_TILE_COLLISION    = 0x00800000,
          OF_ALIGN_HCENTER        = 0x01000000,
          OF_ALIGN_HRIGHT         = 0x02000000,
          OF_ALIGN_VCENTER        = 0x04000000,
          OF_ALIGN_VBOTTOM        = 0x08000000,
          OF_HIDDEN               = 0x10000000,
          OF_FACING_LEFT          = 0x20000000,
          OF_HAS_MOVEMENT_PATH    = 0x40000000,
          OF_ALLOW_LEAVING_MAP    = 0x80000000      // this object can move outside the map (albeit m_BlockMovingOutsideMap set)
        };

        enum ObjectInfoFlagsExtended
        {
          OFX_DONT_TRIGGER        = 0x0000000000000001
        };


        GR::String          Template;
        GR::tPoint          Position;
        GR::u32             CurrentTrigger;
        GR::u32             Flags;
        GR::u64             FlagsExtendeded;
        GR::u32             ExtraDataID;
        GR::u32             ID;


        LayerObject() :
          CurrentTrigger( -1 ),
          Flags( 0 ),
          FlagsExtendeded( 0 ),
          ExtraDataID( 0 ),
          ID( 0 )
        {
        }

        virtual ~LayerObject()
        {
        }

        virtual GR::tRect Bounds()
        {
          return GR::tRect();
        }

        bool IsJumping() const
        {
          return ( Flags & OF_JUMPING ) != 0;
        }

        bool IsFloating() const
        {
          return ( Flags & OF_FLOATING ) != 0;
        }

        bool IsClimbing() const
        {
          return ( Flags & OF_CLIMBING ) != 0;
        }

        bool IsOnGround() const
        {
          return ( Flags & ( OF_ON_GROUND | OF_ON_PLATFORM ) ) != 0;
        }

        bool IsFalling() const
        {
          return ( ( !IsOnGround() )
                && ( !IsClimbing() )
                && ( !IsFloating() ) );
        }
    };



    class ObjectLayer : public Layer
    {
      protected:

        GR::i32           m_SectorWidth;
        GR::i32           m_SectorHeight;

      public:

        std::map<GR::tPoint,std::list<LayerObject*> >      Objects;



        ObjectLayer();
        virtual ~ObjectLayer();



        void          ClearObjects();

        void          AddObject( LayerObject* pObj );
        void          AddObjectOnTop( LayerObject* pObj );
        void          RemoveObject( LayerObject* pObj );
        void          RemoveObject( const GR::tPoint& GridPos, LayerObject* pObj );
        void          MoveObject( LayerObject* pObj, const GR::tPoint& Delta );
        void          SetObjectPos( LayerObject* pObj, const GR::tPoint& NewPos );
        LayerObject*  FindObjectByType( const GR::String& ObjType );
        LayerObject*  FindObjectByID( GR::u32 ID );
        bool          FindObjectsAt( const GR::tPoint& Position, std::list<LayerObject*>& ObjectList );
        LayerObject*  FindCollidingObject( LayerObject* pCollider, const GR::String& Type );

        bool          IsObjectInGrid( LayerObject* pObj );

        int           GetObjectCountByType( const GR::String& ObjType );

        GR::i32       SectorWidth() const;
        GR::i32       SectorHeight() const;

        GR::tPoint    GridPos( LayerObject* pObj );
        GR::tPoint    GridPos( const GR::tPoint& Pos );
                

    };

  }
}


#endif // GR_GAMEBASE_OBJECT_LAYER_H
