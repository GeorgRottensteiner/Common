#include "CImageList.h"



namespace WindowsWrapper
{

  CImageList::CImageList() :
    m_hImageList( NULL )
  {
  }



  CImageList::~CImageList()
  {
    DeleteImageList();
  }



  BOOL CImageList::Create( int cx, int cy, UINT nFlags, int nInitial, int nGrow )
  {

    HIMAGELIST    hIL = ImageList_Create( cx, cy, nFlags, nInitial, nGrow );

    if ( hIL == NULL )
    {
      return FALSE;
    }
    Attach( hIL );
    return TRUE;

  }



  void CImageList::DeleteImageList()
  {

    if ( m_hImageList )
    {
      ImageList_Destroy( m_hImageList );
      m_hImageList = NULL;
    }

  }



  void CImageList::Attach( HIMAGELIST hIL )
  {

    if ( m_hImageList )
    {
      ImageList_Destroy( m_hImageList );
    }
    m_hImageList = hIL;

  }



  HIMAGELIST CImageList::GetSafeHandle()
  {

    return m_hImageList;

  }



  int CImageList::Add( HICON hIcon )
  {

    if ( m_hImageList == NULL )
    {
      return -1;
    }
    return ImageList_AddIcon( m_hImageList, hIcon );

  }



  BOOL CImageList::Remove( int nImage )
  {

    return ImageList_Remove( m_hImageList, nImage );

  }



  int CImageList::GetImageCount() const
  {

    return ImageList_GetImageCount( m_hImageList );

  }

}
