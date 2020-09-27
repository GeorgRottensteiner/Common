// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMATIONLIST_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_ANIMATIONLIST_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CImage;

#include <string>
#include <map>
#include <vector>

struct tAnimationFrame
{
  CImage*    pImage;
  DWORD         dwFrameDauer;

  tAnimationFrame() :
    pImage( NULL ),
    dwFrameDauer( 0 )
    {
    }
};

typedef std::vector<tAnimationFrame>    tSingleAnimation;

struct tAnimation
{
  int                 m_iAnimationFrame;
  DWORD               m_dwElapsedFrameDauer;
  tSingleAnimation    singleAnim;

  tAnimation() :
    m_iAnimationFrame( 0 ),
    m_dwElapsedFrameDauer( 0 )
    {
    }

    void Update( DWORD dwTicks )
    {
      if ( singleAnim.empty() )
      {
        return;
      }
      do
      {
        if ( m_iAnimationFrame >= singleAnim.size() )
        {
          m_iAnimationFrame = 0;
        }

        tAnimationFrame&    curFrame = singleAnim[m_iAnimationFrame];
        if ( curFrame.dwFrameDauer == 0 )
        {
          return;
        }
        // dieser Frame läuft noch
        if ( dwTicks < curFrame.dwFrameDauer - m_dwElapsedFrameDauer )
        {
          m_dwElapsedFrameDauer += dwTicks;
          return;
        }
        dwTicks -= curFrame.dwFrameDauer - m_dwElapsedFrameDauer;
        m_dwElapsedFrameDauer = 0;
        ++m_iAnimationFrame;
      }
      while ( dwTicks );
    }
    CImage* CurrentFrame()
    {
      if ( singleAnim.empty() )
      {
        return NULL;
      }
      if ( m_iAnimationFrame >= singleAnim.size() )
      {
        m_iAnimationFrame = 0;
      }
      return singleAnim[m_iAnimationFrame].pImage;
    }
};

typedef std::vector<tAnimation>    tVectAnimation;



class CAnimationList
{
  public:

    tVectAnimation    m_vectAnimations;

    GR::String       m_strDescription;


    CAnimationList();
    ~CAnimationList();


    void              Update( DWORD dwElapsedTicks );
    CImage*        GetImage( int iIndex );

};

typedef std::map<GR::String,CAnimationList>      tMapAnimationLists;

#endif // !defined(AFX_ANIMATIONLIST_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
