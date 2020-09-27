#if !defined(AFX_XJUMPENUMS_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_XJUMPENUMS_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <GR/GRTypes.h>

#include <Xtreme/XTextureSection.h>


namespace XJump
{

struct tTriggerArea
{
  math::tRect<int>    m_Rect;
  GR::u32             m_dwExtraData;

  tTriggerArea() :
    m_dwExtraData( 0 ),
    m_Rect()
  {
  }
};

enum eTileCategory
{
  TC_DEFAULT = 0,
  TC_CLIMBABLE,     // Leitern, Ranken, etc.
  TC_CLINGABLE_TOP,   // Wände zum dran Klettern
  TC_CLINGABLE_LEFT,  // Wände zum dran Klettern
  TC_CLINGABLE_RIGHT, // Wände zum dran Klettern
  TC_WATER,         // Wasser, drin wird geschwommen
  TC_DOOR,
};

enum eDirType
{
  DIR_NONE,
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT,
  DIR_FORCED_DOWN,
};

enum eActionType
{
  AT_NONE,
  AT_KILLED,
  AT_HURTS,
  AT_CUSTOM,
};

struct tTileInfo
{
  int           iTileSetIndex,
                iTileX,
                iTileY,
                iTileWidth,
                iTileHeight;
  GR::u32       dwFlags;

  tTileInfo() :
    iTileSetIndex( 0 ),
    iTileX( 0 ),
    iTileY( 0 ),
    iTileWidth( 0 ),
    iTileHeight( 0 ),
    dwFlags( XTextureSection::TSF_DEFAULT )
  {
  }
};

};

#endif // !defined(AFX_XJUMPENUMS_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
