#include "GameObject.h"
#include "JREngine.h"

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetAnimation.h>



namespace GR
{
  namespace Gamebase
  {

    GameObject::GameObject() :
      m_pObjectBelow( NULL ),
      m_pSpawnAncestor( NULL ),
      m_ProcessingFlags( 0 ),
      m_pEngine( NULL ),
      Type( 0 ),
      ZOrder( 0 ),
      Angle( 0.0f ),
      FallHeight( 0 ),
      FallPos( 0 ),
      JumpPos( 0.0f ),
      JumpArcHeight( 0 ),
      JumpDuration( 0.5f ),
      JumpStartPos( 0 ),
      LifeTime( 0.0f ),
      m_CustomRender( false )
    {
    }



    GameObject::~GameObject()
    {
    }



    GR::tRect GameObject::Bounds()
    {
      GR::tRect     curBounds( m_Bounds );

      curBounds.offset( Position );

      return curBounds;
    }



    void GameObject::Bounds( const GR::tRect& Bounds )
    {
      m_Bounds = Bounds;
    }



    void GameObject::Update( GR::Gamebase::JREngine& JREngine, const GR::f32 ElapsedTime, GR::up UserData )
    {
      if ( LifeTime > 0.0f )
      {
        LifeTime -= ElapsedTime;
        if ( LifeTime <= 0.0f )
        {
          LifeTime = 0.0f;

          DeleteMe();
        }
      }

      JREngine.MoveObject( this, m_Delta.x * ElapsedTime, m_Delta.y * ElapsedTime );

      HandleJump( JREngine, ElapsedTime );

      if ( Flags & GR::Gamebase::LayerObject::OF_HAS_MOVEMENT_PATH )
      {
        JREngine.HandleMovementPath( *this, ElapsedTime );
      }

      if ( ( !Animation.empty() )
      &&   ( !( m_ProcessingFlags & ProcessingFlags::NO_ANIMATION ) ) )
      {
        JREngine.m_pAnimManager->AdvanceAnimFrame( AnimPos, ElapsedTime * 1000.0f );
      }
    }



    void GameObject::UpdateFixed( GR::Gamebase::JREngine& JREngine, GR::up UserData )
    {
      HandleGravity( JREngine );
    }



    void GameObject::OnObjectEvent( JREngine& JREngine, const ObjectEvent& Event )
    {
      switch ( Event.Type )
      {
        case ObjectEventType::KILLED:
          DeleteMe();
          break;
        case ObjectEventType::DIE:
          break;
        case ObjectEventType::LAND:
          FallHeight = 0;
          break;
        case ObjectEventType::LAND_ON_OBJECT:
          m_pObjectBelow = Event.pOtherObject;
          break;
        case ObjectEventType::OBJECT_LANDED_ON_ME:
          m_ObjectsCarried.push_back( Event.pOtherObject );
          break;
        case ObjectEventType::OBJECT_GOT_OFF_ME:
          m_ObjectsCarried.remove( Event.pOtherObject );
          break;
        case ObjectEventType::GET_OFF_OBJECT:
          m_pObjectBelow = NULL;
          break;
      }
    }



    void GameObject::SetSection( const GR::String& ImageSection )
    {
      Animation.clear();
      Section = ImageSection;
    }



    void GameObject::SetAnimation( const GR::String& Anim )
    {
      if ( Animation != Anim )
      {
        Animation = Anim;

        Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
        if ( pLoader == NULL )
        {
          dh::Log( "GameObject::SetAnimation Missing AssetLoader Service" );
          AnimPos = tAnimationPos();
          return;
        }

        Xtreme::Asset::XAssetAnimation* pAnim = (Xtreme::Asset::XAssetAnimation*)pLoader->Asset( Xtreme::Asset::XA_ANIMATION, Animation.c_str() );
        if ( pAnim == NULL )
        {
          dh::Log( "GameObject::SetAnimation Animation %s not found", Animation.c_str() );
          AnimPos = tAnimationPos();
          return;
        }
        AnimPos = tAnimationPos( pAnim->m_AnimID );
      }
    }



    void GameObject::DeleteMe()
    {
      m_ProcessingFlags |= ProcessingFlags::DELETE_ME;
    }



    void GameObject::KillMe()
    {
      m_pEngine->RaiseObjectEvent( this, GR::Gamebase::ObjectEventType::KILLED );
    }



    void GameObject::SendToSleep()
    {
      m_ProcessingFlags |= ProcessingFlags::SEND_TO_SLEEP;
    }



    void GameObject::HandleJump( GR::Gamebase::JREngine& JREngine, GR::f32 ElapsedTime )
    {
      if ( IsJumping() )
      {
        JumpPos += ElapsedTime;

        int   deltaY = 0;
        if ( JumpPos >= JumpDuration )
        {
          // reached end of jump
          deltaY = -( JumpArcHeight - ( JumpStartPos - Position.y ) );
          Flags &= ~GR::Gamebase::LayerObject::OF_JUMPING;
        }
        else
        {
          int   targetYPosOnArc = (int)( JumpArcHeight * sinf( 3.1415926f * ( JumpPos * 90.0f / JumpDuration ) / 180.0f ) );
          deltaY = ( JumpStartPos - targetYPosOnArc ) - Position.y;
        }
        GR::u32 moveResult = JREngine.MoveObjectFixed( this, 0, deltaY );
        if ( moveResult != 0 )
        {
          Flags &= ~GR::Gamebase::LayerObject::OF_JUMPING;
        }
        // re-check if platform is still below
        if ( m_pObjectBelow )
        {
          GR::tRect   bounds( Bounds() );

          bounds = GR::tRect( bounds.Left, bounds.Bottom, bounds.width(), 1 );

          // the call changes the carrier!
          if ( !JREngine.IsObjectBlockedByOtherObjects( this, bounds, GR::Gamebase::Dir::D ) )
          {
          }
        }
      }
    }



    void GameObject::HandleGravity( GR::Gamebase::JREngine& JREngine )
    {
      if ( ( IsJumping() )
      ||   ( IsFloating() ) )
      {
        return;
      }

      GR::tRect   bounds = Bounds();
      bounds.Top = bounds.Bottom;
      bounds.Bottom = bounds.Top + 1;

      // 1 pixel width
      bounds.Left = bounds.Left + bounds.width() / 2;
      bounds.Right = bounds.Left + 1;
      if ( !JREngine.IsAreaBlocked( this, bounds, GR::Gamebase::Dir::D, "All" ) )
      {
        // falling
        if ( !IsFalling() )
        {
          FallPos = 0;

          JREngine.RaiseObjectEvent( this, GR::Gamebase::ObjectEventType::START_FALLING );
        }
        Flags &= ~GR::Gamebase::LayerObject::OF_ON_GROUND;

        ++FallPos;
        m_Delta.y = FallPos * (GR::f32)JREngine.m_pGameLayer->TileSpacingY * JREngine.m_GravityFactor;
      }
      else
      {
        if ( !IsOnGround() )
        {
          Flags |= GR::Gamebase::LayerObject::OF_ON_GROUND;
          FallPos = 0;
          Flags &= ~GR::Gamebase::LayerObject::OF_JUMPING;

          JREngine.RaiseObjectEvent( this, GR::Gamebase::ObjectEventType::LAND );
        }
        m_Delta.y = 0;
      }
    }



    void GameObject::Jump( int JumpHeight )
    {
      if ( ( !IsJumping() )
      &&   ( ( IsOnGround() )
      ||     ( IsClimbing() ) ) )
      {
        ForceJump( JumpHeight );
      }
    }



    void GameObject::ForceJump( int JumpHeight )
    {
      if ( IsClimbing() )
      {
        Flags &= ~GR::Gamebase::GameObject::OF_CLIMBING;
      }
      JumpArcHeight = JumpHeight;
      Flags         |= GR::Gamebase::GameObject::OF_JUMPING;
      Flags         &= ~GR::Gamebase::GameObject::OF_ON_GROUND;
      JumpPos       = 0.0f;
      JumpStartPos  = Position.y;

      FallPos       = 0;

      m_pEngine->RaiseObjectEvent( this, GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::JUMP ) );
    }



    void GameObject::AbortMove()
    {
      m_ProcessingFlags |= GR::Gamebase::ProcessingFlags::ABORT_MOVE;
    }



  }
}