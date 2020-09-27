#if !defined(AFX_GAMEOBJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_GAMEOBJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <Interface/IIOStream.h>
#include <Interface/IUpdateable.h>
#include <Interface/IAnimationManager.h>
#include <Interface/ICloneAble.h>
#include <Interface/IEventListener.h>

#include <math/tRect.h>
#include <math/vector2.h>
#include <math/vector3.h>



#include <Game/MovementPath.h>

#include <GR/GRTypes.h>

#include "XJumpEnums.h"



class CXJump;
class XRenderer;

class CLevelLayer;

struct tExtraData;

struct tObjectEvent
{
  GR::ip        m_iType,
                m_iParam,
                m_iParamValue;

  GR::String   m_strParam;

  tObjectEvent( GR::ip iType = 0, GR::ip iParam = 0, GR::ip iParamValue = 0, const GR::String& strParam = "" ) :
    m_iType( iType ),
    m_iParam( iParam ),
    m_iParamValue( iParamValue ),
    m_strParam( strParam )
  {
  }
};

class CGameObject : public ICloneAble, 
                    public IUpdateable<float>, 
                    public EventListener<tObjectEvent>
{

  public:

    DECLARE_CLONEABLE( CGameObject, "GameObject" )

    enum eEventType
    {
      ET_NONE = 0,
      ET_INIT,
      ET_START_FALLING,
      ET_END_FALLING,
      ET_MOVED,
      ET_BLOCKED_LEFT,
      ET_BLOCKED_RIGHT,
      ET_BLOCKED_TOP,
      ET_BLOCKED_BOTTOM,
      ET_COLLIDE,
      ET_CARRY_OBJECT,
      ET_DROP_OBJECT,
      ET_KILLED,
      ET_REMOVE,
      ET_TRIGGER,
      ET_LEAVE_TRIGGER,
      ET_USER,
      ET_MAP_CHANGED,
      ET_HURTS,
      ET_ENTER_REGION,
      ET_LEAVE_REGION,
      ET_TOUCH_FLOOR,
      ET_ENTER_SLEEP,
    };

    enum eGameObjectFlags
    {
      GOF_DEFAULT         = 0,
      GOF_FLOATING        = 0x00000001,
      GOF_JUMPING         = 0x00000002,
      GOF_ON_GROUND       = 0x00000004,
      GOF_TILE            = 0x00000008,     // keine Animation (eigentlich witzlos)
      GOF_NO_COLISSION    = 0x00000010,     // kollidiert nicht
      GOF_FACING_LEFT     = 0x00000020,
      GOF_FACING_UP       = 0x00000040,
      GOF_REMOVE_ME       = 0x00000080,     // im nächsten Frame rauswerfen!
      GOF_SPAWNED         = 0x00000100,     // schon aufgeweckt
      GOF_RESPAWN         = 0x00000200,     // wird jedesmal wieder aufgeweckt
      GOF_DECORATION      = 0x00000400,     // deco-Objekt, nur Effekt, keine Kolission, keine Hintergrundabfrage
      GOF_TEMP_LIFE       = 0x00000800,     // nur kurzlebig
      GOF_ALIGN_HCENTER   = 0x00001000,
      GOF_ALIGN_HRIGHT    = 0x00002000,
      GOF_ALIGN_VCENTER   = 0x00004000,
      GOF_ALIGN_VBOTTOM   = 0x00008000,
      GOF_EXTRA_DATA      = 0x00010000,     // ein Extra-Data
      GOF_PLATFORM        = 0x00020000,     // eine Plattform zum Draufstehen
      GOF_ON_PLATFORM     = 0x00040000,     // steht/liegt auf einer Plattform
      GOF_ON_PATH         = 0x00080000,     // folgt einem Pfad
      GOF_BLOCKING        = 0x00100000,     // blockiert den Spieler
      GOF_CLIMBING        = 0x00200000,     // auf einer Leiter oder Ähnlichem
      GOF_PUSHABLE        = 0x00400000,     // kann geschoben werden
      GOF_INVISIBLE       = 0x00800000,     // wird nicht angezeigt
      GOF_CAN_PUSH        = 0x01000000,     // kann Pushables schieben
      GOF_REVERSE_GRAVITY = 0x02000000,     // fällt nach oben (bzw. läuft an der Decke)
      GOF_HMIRROR         = 0x10000000,
      GOF_VMIRROR         = 0x20000000,
      GOF_ROTATE          = 0x40000000,     // Winkel in m_vPosition.z
      GOF_NO_TILE_COLLISION = 0x80000000,   // keine Block-Kollisionsprüfung
    };

    enum eGameObjectSecondaryFlags
    {
      GOF_EX_BEHIND_TILES     = 0x00000001,
    };

    GR::u32               m_dwAnimID,
                          m_dwFlags,
                          m_SecondaryFlags,   // noch mehr Flags!
                          m_dwID,             // eindeutige ID (für Scripts)
                          m_dwExtraData,      // Extra-Data-ID
                          m_dwType;           // Typ-Index in Objekt-Datenbank

    GR::i32               m_iZLayer;          // um Objekte vorne bzw. hinten zu halten

    math::vector3         m_vPosition,
                          m_vDelta,
                          m_vTargetPos;

    float                 m_fJumpPower,
                          m_fLifeDuration,
                          m_fWeight,
                          m_fWalkSpeed,
                          m_fFallHeight;

    bool                  m_bWalkToTarget;

    math::tRect<int>      m_vCollisionRect;

    tAnimationPos         m_animPos;

    XJump::tTriggerArea*  m_pCurrentTrigger;

    GR::tRect*            m_pCurrentRegion;

    GR::Game::Path::tPathPos    m_PathPos;

    CGameObject*          m_pSpawnBase;

    CGameObject*          m_pPlatform;

    CLevelLayer*          m_pLayer;

    std::list<CGameObject*>   m_listCarriedObjects;

    bool                  m_bStopMoving;


    CGameObject();
    virtual ~CGameObject();


    virtual void          Save( IIOStream& Stream );
    virtual void          Load( IIOStream& Stream );

    virtual bool          Move( math::vector3& vectDelta, bool bMovedByPlatform = false );
    virtual void          Update( const float fElapsedTime );

    virtual void          Jump( float fJumpStrength );
    virtual void          Die();

    float                 Weight() const;
                        
    void                  SetAnimation( GR::u32 dwAnimIndex );
    void                  SetAnimation( const GR::String& strAnim );
    bool                  IsAnimationSet( const GR::String& strAnim );
    bool                  IsAnimationSet( const GR::u32 dwAnimIndex );
                          
    virtual bool          ProcessEvent( const tObjectEvent& Event );
                          
    virtual void          SetPosition( int iX, int iY );
    GR::tPoint            TruePosition();

    GR::String           Type() const;

    tExtraData*           ExtraData() const;
                          
    virtual bool          IsJumping() const;
    virtual bool          IsFloating() const;
    virtual bool          IsClimbing() const;
    virtual bool          IsOnGround() const;
                          
    virtual void          Render( XRenderer& Renderer, const GR::tVector& vectOffset, size_t iTile );

    virtual void          ProcessActionType( const XJump::eActionType atType, XJump::eDirType Direction );

    GR::tRect             CollisionRect();

};

#endif // !defined(AFX_GAMEOBJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
