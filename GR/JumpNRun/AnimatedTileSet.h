// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMATEDTILESET_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_ANIMATEDTILESET_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <GR\JumpNRun\TileSet.h>

#include <string>
#include <map>
#include <vector>

struct tAnimationFrame
{
  int     iImageIndex;
  DWORD   dwFrameDauer;

  tAnimationFrame() :
    iImageIndex( 0 ),
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
    int CurrentFrame()
    {
      if ( singleAnim.empty() )
      {
        return 0;
      }
      if ( m_iAnimationFrame >= singleAnim.size() )
      {
        m_iAnimationFrame = 0;
      }
      return singleAnim[m_iAnimationFrame].iImageIndex;
    }
};

typedef std::vector<tAnimation>    tVectAnimation;




class CAnimatedTileSet
{
  public:

    tVectAnimation    m_vectAnimatedTiles;

    CTileSet          m_TileSet;

    GR::String       m_strFileName;


    CAnimatedTileSet();
    ~CAnimatedTileSet();


    void              Update( DWORD dwElapsedTicks );
    GR::CImage*       GetImage( int iIndex );

    bool              Save( const char* szFileName );
    bool              Load( const char* szFileName );

};

typedef std::map<GR::String,CAnimatedTileSet>    tMapAnimatedTileSets;

#endif // !defined(AFX_ANIMATEDTILESET_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
