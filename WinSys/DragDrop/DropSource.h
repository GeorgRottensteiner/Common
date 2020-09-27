#ifndef DROPSRC_H
#define DROPSRC_H

#include <windows.h>
#include <ole2.h>

#include <Interface/IEventProducer.h>


struct tDropEvent
{
  enum eDropType
  {
    DE_DROPPED = 0,
    DE_CANCELLED = 1,
  };

  eDropType   m_Type;

  tDropEvent( eDropType dType = DE_CANCELLED ) :
    m_Type( dType )
  {
  }
};



class CDropSource : public IDropSource, public IEventProducer<tDropEvent>
{

  private:

    long m_cRefCount;


  public:

    CDropSource();
    ~CDropSource();

    //IUnknown members
    STDMETHOD( QueryInterface )( REFIID, void FAR* FAR* );
    STDMETHOD_( ULONG, AddRef )();
    STDMETHOD_( ULONG, Release )();

    //IDataObject members
    STDMETHOD( GiveFeedback )( DWORD dwEffect);
    STDMETHOD( QueryContinueDrag )( BOOL fEscapePressed, DWORD grfKeyState );

};

#endif

