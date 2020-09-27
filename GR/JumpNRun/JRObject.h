// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JROBJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_JROBJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#pragma warning( disable:4786 )
#include <string>
#include <map>
#include <list>
#include <vector>

#include <GR\LocalRegistry\LocalRegistry.h>
#include <GR\JumpNRun\AnimationList.h>


class CJRObject : public CLocalRegistry
{

  public:

    DWORD               m_dwAnimation;

    CJRObject();
    virtual ~CJRObject();


};

typedef std::list<CJRObject>                    tListJRObjects;

typedef std::map<GR::String,CJRObject>         tMapJRObjects;

typedef std::map<GR::String,tListJRObjects>    tMapJRObjectList;

#endif // !defined(AFX_JROBJECT_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
