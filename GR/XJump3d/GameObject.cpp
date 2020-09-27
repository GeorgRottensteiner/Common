// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#include <GR/XJump3d/GameObject.h>
#include <GR/XJump3d/XJump.h>

#include <debug/debugclient.h>



/*-Members--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGameObject::CGameObject() :
  m_dwFlags( 0 ),
  m_dwExtraData( 0 ),
  m_dwAnimID( 0 ),
  m_dwID( 0 ),
  m_dwType( 0 ),
  m_vPosition( 0, 0, 0 ),
  m_vDelta( 0, 0, 0 ),
  m_vTargetPos( 0, 0, 0 ),
  m_fJumpPower( 0.0f ),
  m_fLifeDuration( 0.0f ),
  m_animPos(),
  m_vColissionRect(),
  m_pSpawnBase( NULL ),
  m_pPlatform( NULL ),
  m_pCurrentTrigger( NULL ),
  m_bWalkToTarget( false ),
  m_pLayer( NULL ),
  m_fWeight( 80.0f ),
  m_iZLayer( 0 ),
  m_fWalkSpeed( 80.0f )
{

}



CGameObject::~CGameObject()
{

}



/*-Load-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGameObject::Load( IIOStream& Stream )
{

  m_dwType      = Stream.ReadLong();
  m_dwAnimID    = Stream.ReadLong();
  m_dwID        = Stream.ReadLong();
  m_dwExtraData = Stream.ReadLong();
  m_dwFlags     = Stream.ReadLong();
  m_vPosition.x = Stream.ReadFloat();
  m_vPosition.y = Stream.ReadFloat();
  m_vPosition.z = Stream.ReadFloat();
  m_vDelta.x    = Stream.ReadFloat();
  m_vDelta.y    = Stream.ReadFloat();
  m_vDelta.z    = Stream.ReadFloat();

  m_vColissionRect.m_iLeft = Stream.ReadLong();
  m_vColissionRect.m_iTop = Stream.ReadLong();
  int   iW = Stream.ReadLong();
  m_vColissionRect.size( iW, Stream.ReadLong() );

  SetAnimation( m_dwAnimID );

  // Extradata für später
  GR::u32   dwVersion = Stream.ReadLong();

  if ( dwVersion >= 1 )
  {
    m_fWeight = Stream.ReadFloat();
  }
  if ( dwVersion >= 2 )
  {
    m_iZLayer = Stream.ReadLong();
  }

}



/*-Save-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGameObject::Save( IIOStream& Stream )
{

  Stream.WriteLong( m_dwType );
  Stream.WriteLong( m_dwAnimID );
  Stream.WriteLong( m_dwID );
  Stream.WriteLong( m_dwExtraData );
  Stream.WriteLong( m_dwFlags );
  Stream.WriteFloat( m_vPosition.x );
  Stream.WriteFloat( m_vPosition.y );
  Stream.WriteFloat( m_vPosition.z );
  Stream.WriteFloat( m_vDelta.x );
  Stream.WriteFloat( m_vDelta.y );
  Stream.WriteFloat( m_vDelta.z );

  Stream.WriteLong( m_vColissionRect.position().x );
  Stream.WriteLong( m_vColissionRect.position().y );
  Stream.WriteLong( m_vColissionRect.size().x );
  Stream.WriteLong( m_vColissionRect.size().y );

  // Extradata-Version für spätere Erweiterungen
  Stream.WriteLong( 2 );

  Stream.WriteFloat( m_fWeight );
  Stream.WriteLong( m_iZLayer );

}



/*-Move-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CGameObject::Move( math::vector3& vectDelta, bool bMovedByPlatform )
{

  CXJump&   XJump = CXJump::Instance();

  if ( m_dwFlags & GOF_DECORATION )
  {
    m_vPosition += vectDelta;
    return true;
  }

  int     iX = (int)m_vPosition.x;
  int     iY = (int)m_vPosition.y;

  GR::tRect   rcColission = ColissionRect();

  GR::tPoint  ptMoved( 0, 0 );


  CXJump::eActionType   atType;

  bool    bBlocked = false;

  if ( !bMovedByPlatform )
  {
    int     iGravityY = 1;
    if ( m_dwFlags & GOF_REVERSE_GRAVITY )
    {
      iGravityY = -1;
    }
    if ( ( XJump.BlockedAt( this, iX, iY + iGravityY, CXJump::DIR_DOWN, atType, !bMovedByPlatform ) )
    //||   ( !XJump.IsAreaFree( iX, rcColission.m_iTop, rcColission.width(), 1 ) )
    ||   ( XJump.OnTopOfPlatform( this ) ) )
    {
      if ( ( !IsJumping() )
      &&   ( !IsFloating() ) )
      {
        if ( !IsOnGround() )
        {
          bBlocked = true;
          m_dwFlags |= GOF_ON_GROUND;
          ProcessEvent( tObjectEvent( ET_END_FALLING ) );
          m_vDelta.y = 0.0f;
          m_fJumpPower = 0.0f;
        }
        if ( vectDelta.y > 0 )
        {
          vectDelta.y = 0;
        }
        if ( atType == CXJump::AT_KILLED )
        {
          ProcessEvent( tObjectEvent( ET_KILLED ) );
          Die();
        }
      }
    }
    else
    {
      if ( ( IsOnGround() )
      &&   ( !( m_dwFlags & GOF_ON_PLATFORM ) )
      &&   ( !IsJumping() )
      &&   ( !IsFloating() ) )
      {
        ProcessEvent( tObjectEvent( ET_START_FALLING ) );
      }
      m_dwFlags &= ~GOF_ON_GROUND;
    }
  }

  int     iX2 = (int)( m_vPosition.x + vectDelta.x );
  int     iY2 = (int)( m_vPosition.y + vectDelta.y );

  while ( ( iX != iX2 )
  ||      ( iY != iY2 ) )
  {
    int   iAllowedStepOffset = 0;
    if ( IsOnGround() )
    {
      iAllowedStepOffset = 3;
    }

    if ( iX < iX2 )
    {
      int   iStepNow = 0;
      if ( !XJump.StepPossible( this, iX + 1, iY, iAllowedStepOffset, iStepNow, atType, CXJump::DIR_RIGHT, !bMovedByPlatform ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_RIGHT ) );
        if ( atType == CXJump::AT_KILLED )
        {
          ProcessEvent( tObjectEvent( ET_KILLED ) );
          Die();
        }
      }
      else if ( !XJump.IsAreaFree( iX + 1, rcColission.m_iTop - iStepNow, 1, rcColission.height() ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_RIGHT ) );
      }
      else
      {
        if ( IsOnGround() )
        {
          ptMoved.x++;
          iX++;
          m_vPosition.x += 1.0f;
          vectDelta.x -= 1.0f;

          rcColission.offset( 1, -iStepNow );

          if ( vectDelta.y > 0.0f )
          {
            vectDelta.y = 0.0f;
          }
          m_vPosition.y -= iStepNow;
          iY -= iStepNow;
          iY2 = iY;
        }
        else if ( iStepNow <= 0 )
        {
          ptMoved.x++;
          iX++;
          m_vPosition.x += 1.0f;
          vectDelta.x -= 1.0f;

          rcColission.offset( 1, 0 );
        }
        else
        {
          // blockiert
          bBlocked = true;
          iX2 = iX;
          m_vPosition.x = (float)iX;
          m_vDelta.x = 0.0f;
          vectDelta.x = 0.0f;
          ProcessEvent( tObjectEvent( ET_BLOCKED_RIGHT ) );
        }
      }
    }
    else if ( iX > iX2 )
    {
      int iStepNow = 0;
      if ( !XJump.StepPossible( this, iX - 1, iY, iAllowedStepOffset, iStepNow, atType, CXJump::DIR_LEFT, !bMovedByPlatform ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_LEFT ) );
        if ( atType == CXJump::AT_KILLED )
        {
          ProcessEvent( tObjectEvent( ET_KILLED ) );
          Die();
        }
      }
      else if ( !XJump.IsAreaFree( iX - 1, rcColission.m_iTop - iStepNow, 1, rcColission.height() ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_LEFT ) );
      }
      else
      {
        if ( IsOnGround() )
        {
          ptMoved.x--;
          iX--;
          m_vPosition.x -= 1.0f;
          vectDelta.x += 1.0f;

          rcColission.offset( -1, -iStepNow );

          if ( vectDelta.y > 0.0f )
          {
            vectDelta.y = 0.0f;
          }
          m_vPosition.y -= iStepNow;
          iY -= iStepNow;
          iY2 = iY;
        }
        else if ( iStepNow <= 0 )
        {
          ptMoved.x--;
          iX--;
          m_vPosition.x -= 1.0f;
          vectDelta.x += 1.0f;

          rcColission.offset( -1, 0 );
        }
        else
        {
          // blockiert
          bBlocked = true;
          iX2 = iX;
          m_vPosition.x = (float)iX;
          m_vDelta.x = 0.0f;
          vectDelta.x = 0.0f;
          ProcessEvent( tObjectEvent( ET_BLOCKED_LEFT ) );
        }
      }
    }
    if ( iY < iY2 )
    {
      if ( ( XJump.BlockedAt( this, iX, iY + 1, CXJump::DIR_DOWN, atType, !bMovedByPlatform ) )
      ||   ( XJump.OnTopOfPlatform( this ) ) )
      //||   ( !XJump.IsAreaFree( iX, rcColission.m_iBottom, 1, 1 ) ) )
      {
        // Y blockiert
        bBlocked = true;
        iY2 = iY;
        m_vPosition.y = (float)iY;
        vectDelta.y = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_BOTTOM ) );
        if ( ( !IsJumping() )
        &&   ( !IsFloating() ) )
        {
          m_dwFlags |= GOF_ON_GROUND;
          m_vDelta.y = 0.0f;
          ProcessEvent( tObjectEvent( ET_END_FALLING ) );
        }
        if ( atType == CXJump::AT_KILLED )
        {
          ProcessEvent( tObjectEvent( ET_KILLED ) );
          Die();
        }
      }
      else
      {
        ptMoved.y++;
        iY++;
        m_vPosition.y += 1.0f;
        vectDelta.y -= 1.0f;
        m_dwFlags &= ~GOF_ON_GROUND;

        rcColission.offset( 0, 1 );
      }
    }
    else if ( iY > iY2 )
    {
      if ( ( XJump.BlockedAt( this, iX, iY - 1, CXJump::DIR_UP, atType, !bMovedByPlatform ) )
      ||   ( !XJump.IsAreaFree( iX, rcColission.m_iTop - 1, 1, rcColission.height() ) ) )
      {
        // Y blockiert
        bBlocked = true;
        iY2 = iY;
        m_vPosition.y = (float)iY;
        m_vDelta.y = 0.0f;
        vectDelta.y = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_TOP ) );
        if ( atType == CXJump::AT_KILLED )
        {
          ProcessEvent( tObjectEvent( ET_KILLED ) );
          Die();
        }
      }
      else
      {
        ptMoved.y--;
        iY--;
        m_vPosition.y -= 1.0f;
        vectDelta.y += 1.0f;
        m_dwFlags &= ~GOF_ON_GROUND;

        rcColission.offset( 0, -1 );

        // noch auf einer Leiter?
        if ( IsClimbing() )
        {
          CLevelLayer*  pLayer = XJump.m_currentMap.m_pMainLayer;
          int   iTileX = (int)m_vPosition.x / pLayer->m_vTileSize.x;
          int   iTileY = ( (int)m_vPosition.y + 1 ) / pLayer->m_vTileSize.y;

          if ( !XJump.IsTileInCategory( TC_CLIMBABLE, pLayer->GetFeld( iTileX, iTileY ) ) )
          {
            m_dwFlags &= ~GOF_CLIMBING;
            vectDelta.y = 0.0f;
            m_vDelta.y = 0.0f;
          }
        }
      }
    }

    if ( m_bWalkToTarget )
    {
      if ( iX == (int)floorf( m_vTargetPos.x ) )
      {
        m_bWalkToTarget = false;
        CXJump::Instance().m_LuaHandler.DoString( "g_TargetReached = 1;" );
      }
    }
  }

  m_vPosition += vectDelta;

  if ( ( ptMoved.x )
  ||   ( ptMoved.y ) )
  {
    ProcessEvent( tObjectEvent( ET_MOVED, (GR::up)&ptMoved ) );
  }

  return !bBlocked;

}



/*-Update---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGameObject::Update( const float fElapsedTime )
{

  if ( m_dwFlags & GOF_REMOVE_ME )
  {
    return;
  }

  CXJump::Instance().m_AnimationManager.AdvanceAnimFrame( m_animPos, fElapsedTime );

  if ( m_bWalkToTarget )
  {
    if ( m_vTargetPos.x < m_vPosition.x )
    {
      m_vDelta.x = -m_fWalkSpeed;
      m_dwFlags |= GOF_FACING_LEFT;
    }
    else if ( m_vTargetPos.x > m_vPosition.x )
    {
      m_vDelta.x = m_fWalkSpeed;
      m_dwFlags &= ~GOF_FACING_LEFT;
    }
  }

  if ( m_dwFlags & GOF_TEMP_LIFE )
  {
    m_fLifeDuration -= fElapsedTime;
    if ( m_fLifeDuration <= 0.0f )
    {
      Die();
      return;
    }
  }
  if ( m_dwFlags & GOF_ON_PATH )
  {
    float   fTime = fElapsedTime;
    while ( fTime > 0.0f )
    {
      GR::GAME::CObjectPath::tPathFrame*  pFrame = m_pathPos.m_pPath->Frame( m_pathPos.m_iCurrentFrame );

      if ( m_pathPos.m_fElapsedFrameTime + fTime >= pFrame->m_fFrameLength )
      {
        float   fDelta = pFrame->m_fFrameLength - m_pathPos.m_fElapsedFrameTime;

        math::vector3   vecDelta = pFrame->m_vPos * fDelta / pFrame->m_fFrameLength;
        Move( vecDelta );

        fTime -= fDelta;

        if ( m_pathPos.m_iCurrentFrame + 1 >= m_pathPos.m_pPath->Frames() )
        {
          m_pathPos.m_iCurrentFrame = 0;
        }
        else
        {
          m_pathPos.m_iCurrentFrame++;
        }
        m_pathPos.m_fElapsedFrameTime = 0.0f;
      }
      else
      {
        math::vector3   vecDelta = pFrame->m_vPos * fTime / pFrame->m_fFrameLength;
        Move( vecDelta );
        m_pathPos.m_fElapsedFrameTime += fTime;
        fTime = 0.0f;
      }
    }
    return;
  }

  math::vector3   vectMovement = fElapsedTime * m_vDelta;

  if ( IsJumping() )
  {
    float   fOldPower = m_fJumpPower;

    float   fDelta = fElapsedTime * CXJump::Instance().Gravity().y * Weight() * 0.5f;

    m_fJumpPower -= fDelta;
    if ( m_fJumpPower <= 0.0f )
    {
      m_dwFlags &= ~GOF_JUMPING;
      fDelta = m_fJumpPower;
      m_fJumpPower = 0.0f;
    }

    m_vDelta.y = -m_fJumpPower;
  }

  Move( vectMovement );

  if ( ( !IsFloating() )
  &&   ( !IsClimbing() )
  &&   ( !IsOnGround() ) )
  {
    // 0.5f damit's ungefähr passt und man reale Werte einsetzen kann
    m_vDelta += fElapsedTime * CXJump::Instance().Gravity() * Weight() * 0.5f;
    if ( m_vDelta.y > 0.0f )
    {
      if ( IsJumping() )
      {
        ProcessEvent( tObjectEvent( ET_START_FALLING ) );
      }
      m_dwFlags &= ~GOF_JUMPING;
    }
  }

}



/*-Jump-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGameObject::Jump( float fJumpStrength )
{

  if ( ( !IsJumping() )
  &&   ( ( IsOnGround() )
  ||     ( IsClimbing() ) ) )
  {
    if ( IsClimbing() )
    {
      m_dwFlags &= ~GOF_CLIMBING;
    }
    //m_vDelta.y = -fJumpStrength;
    m_fJumpPower = fJumpStrength;
    m_dwFlags |= GOF_JUMPING;
  }

}



/*-SetAnimation---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGameObject::SetAnimation( GR::u32 dwAnimIndex )
{

  if ( m_animPos.m_dwAnimationId != dwAnimIndex )
  {
    m_animPos = tAnimationPos();
    m_animPos.m_dwAnimationId = dwAnimIndex;
  }

}



/*-ProcessEvent---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CGameObject::ProcessEvent( const tObjectEvent& Event )
{

  return false;

}



/*-IsJumping------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CGameObject::IsJumping() const
{

  return ( m_dwFlags & GOF_JUMPING ) != 0;

}



/*-IsFloating-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CGameObject::IsFloating() const
{

  return ( m_dwFlags & GOF_FLOATING ) != 0;

}



/*-IsClimbing-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CGameObject::IsClimbing() const
{

  return ( m_dwFlags & GOF_CLIMBING ) != 0;

}



/*-IsOnGround-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CGameObject::IsOnGround() const
{

  return ( m_dwFlags & GOF_ON_GROUND ) != 0;

}



/*-Die------------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGameObject::Die()
{

  ProcessEvent( tObjectEvent( ET_REMOVE ) );
  m_dwFlags |= GOF_REMOVE_ME;

}



/*-SetPosition----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CGameObject::SetPosition( int iX, int iY )
{

  m_vPosition.x = (float)iX;
  m_vPosition.y = (float)iY;

}



/*-TruePosition---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

GR::tPoint CGameObject::TruePosition()
{

  GR::tPoint      Position( (int)m_vPosition.x, (int)m_vPosition.y );


  CXJump::tTileInfo* pTileInfo = CXJump::Instance().GetTileInfo( m_dwAnimID | 0x8000 );

  if ( pTileInfo == NULL )
  {
    return Position;
  }

  if ( m_dwFlags & GOF_ALIGN_VBOTTOM )
  {
    Position.y -= pTileInfo->iTileHeight - 1;
  }
  else if ( m_dwFlags & GOF_ALIGN_VCENTER )
  {
    Position.y -= pTileInfo->iTileHeight / 2;
  }
  if ( m_dwFlags & GOF_ALIGN_HRIGHT )
  {
    Position.x -= pTileInfo->iTileWidth - 1;
  }
  else if ( m_dwFlags & GOF_ALIGN_HCENTER )
  {
    Position.x -= pTileInfo->iTileWidth / 2;
  }

  return Position;

}



void CGameObject::Render( CD3DViewer& Viewer, const D3DXVECTOR3& vectOffset, size_t iTile )
{

  CXJump::Instance().DisplayObject( Viewer,
                                    iTile,
                                    (int)( m_vPosition.x - vectOffset.x ),
                                    (int)( m_vPosition.y - vectOffset.y ),
                                    m_dwFlags );

}



GR::String CGameObject::Type() const
{

  if ( m_dwType >= CXJump::Instance().m_vectKnownObjects.size() )
  {
    return "";
  }
  return CXJump::Instance().m_vectKnownObjects[m_dwType].first;

}



GR::tRect CGameObject::ColissionRect()
{

  GR::tRect   rcCollide( m_vColissionRect );

  rcCollide.offset( TruePosition() );

  return rcCollide;

}



float CGameObject::Weight() const
{

  return m_fWeight;

}