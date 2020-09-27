#pragma once


// CMultiTreeCtrl

class CMultiTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CMultiTreeCtrl)


  public:


	  CMultiTreeCtrl();
	  virtual ~CMultiTreeCtrl();

    BOOL                SelectItem( HTREEITEM hItem );
    BOOL                UnselectItem( HTREEITEM hItem );
    void                RemoveSelection();
    bool                IsItemSelected( HTREEITEM hItem );
    size_t              GetSelectedItemCount();

    HTREEITEM           GetNextItem( HTREEITEM hItem );

    HTREEITEM           CopyItem( HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter = TVI_LAST );
    HTREEITEM           CopyBranch( HTREEITEM htiBranch, HTREEITEM htiNewParent, HTREEITEM htiAfter = TVI_LAST );

    size_t              GetItemLevel( HTREEITEM hItem );

    HTREEITEM           GetNextSelectedItem( HTREEITEM hItem = NULL );

    CImageList*         CreateDragImageEx( HTREEITEM hItem );


  protected:

    CImageList*	        m_pDragImage;

	  BOOL		            m_bLDragging;

	  HTREEITEM	          m_hitemDrag,
                        m_hitemDrop;


	  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnTvnSelchanging(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
protected:
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


