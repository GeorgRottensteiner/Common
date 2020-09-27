#include "DropSource.h"



CDropSource::CDropSource()
{

  m_cRefCount = 0;

}



CDropSource::~CDropSource()
{
}



STDMETHODIMP CDropSource::QueryInterface( REFIID refiid, void FAR* FAR* ppv )
{

  *ppv = NULL;
  if ( ( IID_IUnknown == refiid )
  ||   ( IID_IDropSource == refiid ) )
  {
    *ppv = this;
  }

  if ( NULL != *ppv )
  {
    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
  }
  return ResultFromScode( E_NOINTERFACE );

}



STDMETHODIMP_( ULONG ) CDropSource::AddRef()
{

  return ++m_cRefCount;

}



STDMETHODIMP_( ULONG ) CDropSource::Release()
{

  long tempCount;

  tempCount = --m_cRefCount;
  if ( tempCount == 0 )
  {
    delete this;
  }
  return tempCount; 

}



STDMETHODIMP CDropSource::GiveFeedback( DWORD dwEffect )
{

  return ResultFromScode( DRAGDROP_S_USEDEFAULTCURSORS );   

}



STDMETHODIMP CDropSource::QueryContinueDrag( BOOL fEscapePressed, DWORD grfKeyState )
{

  if ( fEscapePressed )
  {
    SendEvent( tDropEvent( tDropEvent::DE_CANCELLED ) );
    return ResultFromScode( DRAGDROP_S_CANCEL );
  }
  if ( !( grfKeyState & MK_LBUTTON ) )
  {
    // hier droppen!
    SendEvent( tDropEvent( tDropEvent::DE_DROPPED ) );
    return ResultFromScode( DRAGDROP_S_DROP );
  }

  return ResultFromScode( S_OK );

}
