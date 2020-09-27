// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#include <GR\JumpNRun\AnimationList.h>

#include <IO\File.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimationList::CAnimationList()
{

}



CAnimationList::~CAnimationList()
{

}



void CAnimationList::Update( DWORD dwElapsedTicks )
{

  tVectAnimation::iterator    it( m_vectAnimations.begin() );
  while ( it != m_vectAnimations.end() )
  {
    tAnimation&   Anim = *it;

    Anim.Update( dwElapsedTicks );

    ++it;
  }


}



CImage* CAnimationList::GetImage( int iIndex )
{

  if ( iIndex >= m_vectAnimations.size() )
  {
    return NULL;
  }

  return m_vectAnimations[iIndex].CurrentFrame();

}



