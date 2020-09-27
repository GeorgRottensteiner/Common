#ifndef INCLUDE_CIMAGELIST_H
#define INCLUDE_CIMAGELIST_H



#include <windows.h>
#include <commctrl.h>



namespace WindowsWrapper
{

  class CImageList
  {

    protected:

    HIMAGELIST            m_hImageList;


    public:


    CImageList();
    virtual ~CImageList();


    BOOL                  Create( int cx, int cy,
                                  UINT nFlags,
                                  int nInitial,
                                  int nGrow );
    void                  DeleteImageList();

    int                   GetImageCount() const;

    HIMAGELIST            GetSafeHandle();

    void                  Attach( HIMAGELIST hIL );

    int                   Add( HICON hIcon );
    BOOL                  Remove( int nImage );

  };

}

#endif // INCLUDE_CIMAGELIST_H



