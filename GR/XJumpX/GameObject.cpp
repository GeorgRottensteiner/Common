#include <GR/XJumpX/GameObject.h>
#include <GR/XJumpX/XJump.h>

#include <debug/debugclient.h>



CGameObject::CGameObject() :
  m_dwFlags( 0 ),
  m_SecondaryFlags( 0 ),
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
  m_vCollisionRect(),
  m_pSpawnBase( NULL ),
  m_pPlatform( NULL ),
  m_pCurrentTrigger( NULL ),
  m_pCurrentRegion( NULL ),
  m_bWalkToTarget( false ),
  m_pLayer( NULL ),
  m_fWeight( 40.0f ),
  m_iZLayer( 0 ),
  m_fWalkSpeed( 80.0f ),
  m_fFallHeight( 0.0f ),
  m_bStopMoving( false )
{

}



CGameObject::~CGameObject()
{

}



void CGameObject::Load( IIOStream& Stream )
{

  m_dwType      = Stream.ReadU32();
  m_dwAnimID    = Stream.ReadU32();
  m_dwID        = Stream.ReadU32();
  m_dwExtraData = Stream.ReadU32();
  m_dwFlags     = Stream.ReadU32();
  m_vPosition.x = Stream.ReadF32();
  m_vPosition.y = Stream.ReadF32();
  m_vPosition.z = Stream.ReadF32();
  m_vDelta.x    = Stream.ReadF32();
  m_vDelta.y    = Stream.ReadF32();
  m_vDelta.z    = Stream.ReadF32();

  m_vCollisionRect.Left = Stream.ReadU32();
  m_vCollisionRect.Top = Stream.ReadU32();
  int   iW = Stream.ReadU32();
  m_vCollisionRect.size( iW, Stream.ReadU32() );

  SetAnimation( m_dwAnimID );

  // Extradata für später
  GR::u32   dwVersion = Stream.ReadU32();

  if ( dwVersion >= 1 )
  {
    m_fWeight = Stream.ReadF32();
  }
  if ( dwVersion >= 2 )
  {
    m_iZLayer = Stream.ReadU32();
  }

}



void CGameObject::Save( IIOStream& Stream )
{

  Stream.WriteU32( m_dwType );
  Stream.WriteU32( m_dwAnimID );
  Stream.WriteU32( m_dwID );
  Stream.WriteU32( m_dwExtraData );
  Stream.WriteU32( m_dwFlags );
  Stream.WriteF32( m_vPosition.x );
  Stream.WriteF32( m_vPosition.y );
  Stream.WriteF32( m_vPosition.z );
  Stream.WriteF32( m_vDelta.x );
  Stream.WriteF32( m_vDelta.y );
  Stream.WriteF32( m_vDelta.z );

  Stream.WriteU32( m_vCollisionRect.position().x );
  Stream.WriteU32( m_vCollisionRect.position().y );
  Stream.WriteU32( m_vCollisionRect.width() );
  Stream.WriteU32( m_vCollisionRect.height() );

  // Extradata-Version für spätere Erweiterungen
  Stream.WriteU32( 2 );

  Stream.WriteF32( m_fWeight );
  Stream.WriteU32( m_iZLayer );

}



bool CGameObject::Move( math::vector3& vectDelta, bool bMovedByPlatform )
{

  CXJump&   XJump = CXJump::Instance();

  if ( m_dwFlags & GOF_DECORATION )
  {
    m_vPosition += vectDelta;
    return true;
  }

  bool CheckTileCollision = !( m_dwFlags & GOF_NO_TILE_COLLISION );

  int     iX = (int)m_vPosition.x;
  int     iY = (int)m_vPosition.y;

  GR::tRect   rcCollision = CollisionRect();

  GR::tPoint  ptMoved( 0, 0 );


  XJump::eActionType   atType;

  bool    bBlocked = false;

  if ( !bMovedByPlatform )
  {
    int     iGravityY = 1;
    if ( m_dwFlags & GOF_REVERSE_GRAVITY )
    {
      iGravityY = -1;
    }
    if ( ( XJump.BlockedAt( this, iX, iY + iGravityY, XJump::DIR_DOWN, atType, !bMovedByPlatform ) )
    //||   ( !XJump.IsAreaFree( iX, rcColission.m_iTop, rcColission.width(), 1 ) )
    ||   ( ( !bMovedByPlatform )
    &&     ( XJump.OnTopOfPlatform( this ) ) ) )
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
          m_fFallHeight = 0.0f;
        }
        if ( vectDelta.y > 0 )
        {
          vectDelta.y = 0;
        }
        ProcessEvent( tObjectEvent( ET_TOUCH_FLOOR, iX, iY + 1 ) );
        ProcessActionType( atType, XJump::DIR_DOWN );
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
        m_fFallHeight = 0.0f;
      }
      m_dwFlags &= ~GOF_ON_GROUND;
    }
  }

  if ( m_bStopMoving )
  {
    m_bStopMoving = false;
    return false;
  }

  int     iX2 = (int)( m_vPosition.x + vectDelta.x );
  int     iY2 = (int)( m_vPosition.y + vectDelta.y );

  int     iOrigX = iX;
  int     iOrigY = iY;

  while ( ( iX != iX2 )
  ||      ( iY != iY2 ) )
  {
    int   iAllowedStepOffset = 0;
    if ( IsOnGround() )
    {
      iAllowedStepOffset = 4;
    }
      
    if ( iX < iX2 )
    {
      int   iStepNow = 0;
      if ( !XJump.StepPossible( this, iX + 1, iY, iAllowedStepOffset, iStepNow, atType, XJump::DIR_RIGHT, !bMovedByPlatform ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_RIGHT, iX + 1, iY ) );
        ProcessActionType( atType, XJump::DIR_RIGHT );
      }
      else if ( ( CheckTileCollision )
      &&        ( !XJump.IsAreaFree( iX + 1, rcCollision.Top - iStepNow, 1, rcCollision.height() ) ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_RIGHT, iX + 1, iY ) );
      }
      else
      {
        if ( IsOnGround() )
        {
          ptMoved.x++;
          iX++;
          m_vPosition.x += 1.0f;
          vectDelta.x -= 1.0f;

          rcCollision.offset( 1, -iStepNow );

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

          rcCollision.offset( 1, 0 );
        }
        else
        {
          // blockiert
          bBlocked = true;
          iX2 = iX;
          m_vPosition.x = (float)iX;
          m_vDelta.x = 0.0f;
          vectDelta.x = 0.0f;
          ProcessEvent( tObjectEvent( ET_BLOCKED_RIGHT, iX + 1, iY ) );
        }
      }
    }
    else if ( iX > iX2 )
    {
      int iStepNow = 0;
      if ( !XJump.StepPossible( this, iX - 1, iY, iAllowedStepOffset, iStepNow, atType, XJump::DIR_LEFT, !bMovedByPlatform ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_LEFT, iX - 1, iY ) );
        ProcessActionType( atType, XJump::DIR_LEFT );
      }
      else if ( ( CheckTileCollision )
      &&        ( !XJump.IsAreaFree( iX - 1, rcCollision.Top - iStepNow, 1, rcCollision.height() ) ) )
      {
        // X blockiert
        bBlocked = true;
        iX2 = iX;
        m_vPosition.x = (float)iX;
        m_vDelta.x = 0.0f;
        vectDelta.x = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_LEFT, iX - 1, iY ) );
      }
      else
      {
        if ( IsOnGround() )
        {
          ptMoved.x--;
          iX--;
          m_vPosition.x -= 1.0f;
          vectDelta.x += 1.0f;

          rcCollision.offset( -1, -iStepNow );

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

          rcCollision.offset( -1, 0 );
        }
        else
        {
          // blockiert
          bBlocked = true;
          iX2 = iX;
          m_vPosition.x = (float)iX;
          m_vDelta.x = 0.0f;
          vectDelta.x = 0.0f;
          ProcessEvent( tObjectEvent( ET_BLOCKED_LEFT, iX - 1, iY ) );
        }
      }
    }

    if ( m_bStopMoving )
    {
      m_bStopMoving = false;
      return false;
    }

    if ( iY < iY2 )
    {
      if ( ( XJump.BlockedAt( this, iX, iY + 1, XJump::DIR_DOWN, atType, !bMovedByPlatform ) )
      ||   ( ( !bMovedByPlatform )
      &&     ( XJump.OnTopOfPlatform( this ) ) ) )
      //||   ( !XJump.IsAreaFree( iX, rcColission.m_iBottom, 1, 1 ) ) )
      {
        // Y blockiert
        bBlocked = true;
        iY2 = iY;
        m_vPosition.y = (float)iY;
        vectDelta.y = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_BOTTOM, iX, iY + 1 ) );
        if ( ( !IsJumping() )
        &&   ( !IsFloating() ) )
        {
          m_dwFlags |= GOF_ON_GROUND;
          m_vDelta.y = 0.0f;
          ProcessEvent( tObjectEvent( ET_END_FALLING ) );
          m_fFallHeight = 0.0f;
          ProcessEvent( tObjectEvent( ET_TOUCH_FLOOR, iX, iY + 1 ) );
        }
        ProcessActionType( atType, XJump::DIR_DOWN );
      }
      else
      {
        ptMoved.y++;
        iY++;
        m_vPosition.y += 1.0f;
        vectDelta.y -= 1.0f;
        m_dwFlags &= ~GOF_ON_GROUND;

        if ( ( !IsJumping() )
        &&   ( !IsFloating() ) )
        {
          if ( !IsOnGround() )
          {
            m_fFallHeight += 1.0f;
          }
        }

        rcCollision.offset( 0, 1 );
      }
    }
    else if ( iY > iY2 )
    {
      if ( ( XJump.BlockedAt( this, iX, iY - 1, XJump::DIR_UP, atType, !bMovedByPlatform ) )
      ||   ( ( CheckTileCollision )
      &&     ( !XJump.IsAreaFree( iX, rcCollision.Top - 1, 1, 1 ) ) ) )
      {
        // Y blockiert
        bBlocked = true;
        iY2 = iY;
        m_vPosition.y = (float)iY;
        m_vDelta.y = 0.0f;
        vectDelta.y = 0.0f;
        ProcessEvent( tObjectEvent( ET_BLOCKED_TOP, iX, rcCollision.Top - 1 ) );

        ProcessActionType( atType, XJump::DIR_UP );
      }
      else
      {
        ptMoved.y--;
        iY--;
        m_vPosition.y -= 1.0f;
        vectDelta.y += 1.0f;
        m_dwFlags &= ~GOF_ON_GROUND;

        rcCollision.offset( 0, -1 );

        // noch auf einer Leiter?
        if ( IsClimbing() )
        {
          CLevelLayer*  pLayer = XJump.m_currentMap.m_pMainLayer;
          int   iTileX = (int)m_vPosition.x / pLayer->m_vTileSize.x;
          int   iTileY = ( (int)m_vPosition.y + 1 ) / pLayer->m_vTileSize.y;

          if ( !XJump.IsTileInCategory( XJump::TC_CLIMBABLE, pLayer->Field( iTileX, iTileY ) ) )
          {
            m_dwFlags &= ~GOF_CLIMBING;
            vectDelta.y = 0.0f;
            m_vDelta.y = 0.0f;
          }
        }
      }
    }

    if ( m_bStopMoving )
    {
      m_bStopMoving = false;
      return false;
    }

    if ( m_bWalkToTarget )
    {
      if ( iX == (int)floorf( m_vTargetPos.x ) )
      {
        m_bWalkToTarget = false;
        m_vDelta.clear();
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
      GR::Game::Path::MovementPath::tPathFrame*  pFrame = m_PathPos.pPath->Frame( m_PathPos.CurrentFrame );

      if ( m_PathPos.ElapsedFrameTime + fTime >= pFrame->FrameLength )
      {
        float   fDelta = pFrame->FrameLength - m_PathPos.ElapsedFrameTime;

        math::vector3   vecDelta = ( pFrame->StartPos - pFrame->EndPos ) * fDelta / pFrame->FrameLength;
        Move( vecDelta );

        fTime -= fDelta;

        if ( m_PathPos.CurrentFrame + 1 >= m_PathPos.pPath->FrameCount() )
        {
          m_PathPos.CurrentFrame = 0;
        }
        else
        {
          m_PathPos.CurrentFrame++;
        }
        m_PathPos.ElapsedFrameTime = 0.0f;
      }
      else
      {
        math::vector3   vecDelta = ( pFrame->StartPos - pFrame->EndPos ) * fTime / pFrame->FrameLength;
        Move( vecDelta );
        m_PathPos.ElapsedFrameTime += fTime;
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
        m_fFallHeight = 0.0f;
      }
      m_dwFlags &= ~GOF_JUMPING;
    }
  }

}



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



bool CGameObject::IsAnimationSet( const GR::String& strAnim )
{
  if ( CXJump::Instance().m_mapAnimations.find( strAnim ) == CXJump::Instance().m_mapAnimations.end() )
  {
    return false;
  }
  return IsAnimationSet( CXJump::Instance().m_mapAnimations[strAnim] );
}



bool CGameObject::IsAnimationSet( const GR::u32 dwAnimIndex )
{
  return ( m_animPos.AnimationId == dwAnimIndex );
}



void CGameObject::SetAnimation( GR::u32 dwAnimIndex )
{
  if ( m_animPos.AnimationId != dwAnimIndex )
  {
    m_animPos = tAnimationPos();
    m_animPos.AnimationId = dwAnimIndex;

    if ( m_pSpawnBase )
    {
      m_pSpawnBase->SetAnimation( dwAnimIndex );
    }
  }
}



void CGameObject::SetAnimation( const GR::String& strAnim )
{
  if ( CXJump::Instance().m_mapAnimations.find( strAnim ) != CXJump::Instance().m_mapAnimations.end() )
  {
    GR::u32   dwNewAnimID = CXJump::Instance().m_mapAnimations[strAnim];

    SetAnimation( dwNewAnimID );
  }
}



bool CGameObject::ProcessEvent( const tObjectEvent& Event )
{
  if ( Event.m_iType == CGameObject::ET_CARRY_OBJECT )
  {
    m_listCarriedObjects.push_back( (CGameObject*)Event.m_iParam );
  }
  else if ( Event.m_iType == CGameObject::ET_REMOVE )
  {
    m_dwFlags |= GOF_REMOVE_ME;
    if ( m_pPlatform )
    {
      m_pPlatform->m_listCarriedObjects.remove( this );
      m_pPlatform = NULL;
    }
  }
  else if ( Event.m_iType == CGameObject::ET_DROP_OBJECT )
  {
     m_listCarriedObjects.remove( (CGameObject*)Event.m_iParam );
  }
  else if ( Event.m_iType == CGameObject::ET_INIT )
  {
    tExtraData*   pData = ExtraData();
    if ( pData )
    {
      if ( pData->m_Type == tExtraData::EX_PATH )
      {
        m_dwFlags |= GOF_ON_PATH;
      }
    }
  }
  else if ( Event.m_iType == CGameObject::ET_MOVED )
  {
    GR::tPoint*   pPoint = (GR::tPoint*)Event.m_iParam;

    size_t    iSize = m_listCarriedObjects.size();

    CLevelLayer::tListObjects::iterator   it( m_listCarriedObjects.begin() );
    while ( it != m_listCarriedObjects.end() )
    {
      CGameObject*    pObj = *it;

      ++it;

      //dh::Log( "Platform moving carried object %d,%d", pPoint->x, pPoint->y );

      CXJump::tPoint    ObjPos = TruePosition();
      math::tRect<int>   vecCR = m_vCollisionRect;
      vecCR.offset( ObjPos );

      CXJump::tPoint      ObjPos2 = pObj->TruePosition();
      math::tRect<int>    vecCR2 = pObj->m_vCollisionRect;
      vecCR2.offset( ObjPos2 );

      /*
      pObject->m_pPlatform = pObj;
      if ( !( pObject->m_dwFlags & CGameObject::GOF_ON_PLATFORM ) )
      {
        pObject->m_dwFlags |= CGameObject::GOF_ON_PLATFORM;

        // fest auf die Höhe setzen
        
      }
      */

      if ( !pObj->Move( math::vector3( (float)pPoint->x, (float)pPoint->y, 0.0f ), true ) )
      {
        dh::Log( "Moved object was blocked?" );
      }
      else
      {
        pObj->m_vPosition.y = (GR::f32)( vecCR.Top - 1 );
        //dh::Log( "Moved object y set to %d (my y %d)", vecCR.m_iTop - 1, ObjPos.y ); 
      }
    }
  }

  return false;

}



bool CGameObject::IsJumping() const
{

  return ( m_dwFlags & GOF_JUMPING ) != 0;

}



bool CGameObject::IsFloating() const
{

  return ( m_dwFlags & GOF_FLOATING ) != 0;

}



bool CGameObject::IsClimbing() const
{

  return ( m_dwFlags & GOF_CLIMBING ) != 0;

}



bool CGameObject::IsOnGround() const
{

  return ( m_dwFlags & ( GOF_ON_GROUND | GOF_ON_PLATFORM ) ) != 0;

}



void CGameObject::Die()
{

  ProcessEvent( tObjectEvent( ET_REMOVE ) );

}



void CGameObject::SetPosition( int iX, int iY )
{

  m_vPosition.x = (float)iX;
  m_vPosition.y = (float)iY;

}



GR::tPoint CGameObject::TruePosition()
{

  GR::tPoint      Position( (int)m_vPosition.x, (int)m_vPosition.y );


  XJump::tTileInfo* pTileInfo = CXJump::Instance().GetTileInfo( m_dwAnimID | 0x8000 );

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



void CGameObject::Render( XRenderer& Renderer, const GR::tVector& vectOffset, size_t iTile )
{

  CXJump::Instance().DisplayObject( Renderer,
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



GR::tRect CGameObject::CollisionRect()
{

  GR::tRect   rcCollide( m_vCollisionRect );

  GR::tPoint      Position( (int)m_vPosition.x, (int)m_vPosition.y );

  if ( m_dwFlags & GOF_ALIGN_VBOTTOM )
  {
    Position.y -= m_vCollisionRect.height() - 1;
  }
  else if ( m_dwFlags & GOF_ALIGN_VCENTER )
  {
    Position.y -= m_vCollisionRect.height() / 2;
  }
  if ( m_dwFlags & GOF_ALIGN_HRIGHT )
  {
    Position.x -= m_vCollisionRect.width() - 1;
  }
  else if ( m_dwFlags & GOF_ALIGN_HCENTER )
  {
    Position.x -= m_vCollisionRect.width() / 2;
  }

  rcCollide.offset( Position );

  return rcCollide;

}



float CGameObject::Weight() const
{

  return m_fWeight;

}



void CGameObject::ProcessActionType( const XJump::eActionType atType, XJump::eDirType Direction )
{

  if ( atType == XJump::AT_KILLED )
  {
    ProcessEvent( tObjectEvent( ET_KILLED ) );
    Die();
  }
  else if ( atType == XJump::AT_HURTS )
  {
    ProcessEvent( tObjectEvent( ET_HURTS ) );
  }

}



tExtraData* CGameObject::ExtraData() const
{

  if ( !( m_dwFlags & GOF_EXTRA_DATA ) )
  {
    return NULL;
  }
  return CXJump::Instance().m_currentMap.GetExtraData( m_dwExtraData );

}