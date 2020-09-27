// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XJUMPENUMS_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_XJUMPENUMS_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <GR/GRTypes.h>


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
  TC_CLINGABLE,     // Wände zum dran Klettern
  TC_WATER,         // Wasser, drin wird geschwommen
};

#endif // !defined(AFX_XJUMPENUMS_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
