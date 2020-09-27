#ifndef _PJ_TREEWRAPPER_H
#define _PJ_TREEWRAPPER_H


#pragma once

#pragma warning ( disable:4786 )
#include <string>
#include <list>




namespace PJ
{
//- Allzweck-TreeItemListe
typedef std::list< HTREEITEM > TreeItemList;


/////////////////////////////////////////////////////////////////////
//-
//-    CTreeViewHelper : nur allgemeine Funktionalität für CTreeCtrls
//-
/////////////////////////////////////////////////////////////////////
// GetLastItem  - Gets last item in the branch
// Returns      - Last item
// hItem        - Node identifying the branch. NULL will
//                return the last item in outine

class CGenericTreeViewHelper
{
private:
  CTreeCtrl& m_Ctrl;
  CGenericTreeViewHelper();
  CGenericTreeViewHelper& operator=( const CGenericTreeViewHelper& rhs );
public:
  CGenericTreeViewHelper( CTreeCtrl& ctrl )
    : m_Ctrl( ctrl ) {}
  CGenericTreeViewHelper( const CGenericTreeViewHelper& rhs )
    : m_Ctrl( rhs.m_Ctrl ) {}

  virtual ~CGenericTreeViewHelper() {};

  inline CTreeCtrl& GetTreeCtrl() const { return m_Ctrl; }

  ////////////////////////////////////////////////
  //-    TreeControl-Wrapperfunktionen
  ////////////////////////////////////////////////
  inline GR::String  GetItemText( const HTREEITEM hItem )                      { return GR::String( LPCTSTR( GetTreeCtrl().GetItemText( hItem ) ) ); }

  inline bool         SetItemText( const HTREEITEM hItem, const GR::String& text )   { return FALSE != GetTreeCtrl().SetItemText( hItem, text.c_str() ); }
  inline bool         SetItem( TVITEM* tvItem )                                       { return FALSE != GetTreeCtrl().SetItem( tvItem ); }
  inline bool         GetItem( TVITEM* tvItem )                                       { return FALSE != GetTreeCtrl().GetItem( tvItem ); }
  inline long         GetItemData( const HTREEITEM hItem ) const                      { return GetTreeCtrl().GetItemData( hItem ); }
  inline bool         SetItemData( const HTREEITEM hItem, const long data )           { return FALSE != GetTreeCtrl().SetItemData( hItem, data ); }
  inline HTREEITEM    GetRootItem()                               const               { return GetTreeCtrl().GetRootItem(); }
  inline HTREEITEM    GetChildItem(       const HTREEITEM hItem ) const               { return GetTreeCtrl().GetChildItem( hItem ); }
  inline HTREEITEM    GetParentItem(      const HTREEITEM hItem ) const               { return GetTreeCtrl().GetParentItem( hItem ); }
  inline HTREEITEM    GetNextSiblingItem( const HTREEITEM hItem ) const               { return GetTreeCtrl().GetNextSiblingItem( hItem ); }
  inline HTREEITEM    GetPrevSiblingItem( const HTREEITEM hItem ) const               { return GetTreeCtrl().GetPrevSiblingItem( hItem ); }

  // inline CImageList* SetImageList( CImageList * pImageList, int nImageListType ) { return GetTreeCtrl().SetImageList( pImageList, nImageListType ) }
  // inline BOOL Expand( HTREEITEM hItem, UINT nCode )                                     { return GetTreeCtrl().Expand( hitem, nCode ); }

  // inline HTREEITEM GetSelectedItem()  { return GetFirstSelectedItem(); }

  // inline HTREEITEM GetFirstSelectedItem( void )
  // {
    // if ( m_theSelection.empty() ) return NULL;
    // else                          return m_theSelection.front();
  // }

  // inline CEdit* EditLabel( HTREEITEM hItem ) { return GetTreeCtrl().EditLabel( hItem ); }


  // inline HTREEITEM GetPrevVisibleItem( HTREEITEM hItem )              { return GetTreeCtrl().GetPrevVisibleItem(); }
  // inline HTREEITEM GetNextVisibleItem( HTREEITEM hItem )              { return GetTreeCtrl().GetNextVisibleItem(); }

  // inline UINT GetItemState( HTREEITEM hItem, UINT nStateMask ) const  { return GetTreeCtl().GetItemState( hItem, nStateMask ); }

  // inline BOOL SelectItem( HTREEITEM hItem ) { return GetTreeCtrl().SelectItem( hItem ); }



  inline bool DeleteAllItems() const { return FALSE != GetTreeCtrl().DeleteAllItems(); }
  ////////////////////////////////////////////////
  //-    Test, ob zwei Items Gechwister sind
  ////////////////////////////////////////////////
  inline bool AreSiblings( const HTREEITEM hItem1, const HTREEITEM hItem2 )
  {
    return ( GetTreeCtrl().GetParentItem( hItem1 ) == GetTreeCtrl().GetParentItem( hItem2 ) );
  }


  /*-GetTreeItemLevel-----------------------------------------------------------+
   |                                                                            |
   +----------------------------------------------------------------------------*/
  inline long GetTreeItemLevel( const HTREEITEM treeItem ) const
  {
    long         dwLevel   = 0;
    HTREEITEM     treeDummy = treeItem;
    while ( treeDummy = GetParentItem( treeDummy ) ) ++dwLevel;
    return dwLevel;
  }


  // //-GetNextItem
  // HTREEITEM GetNextItem( const HTREEITEM hItem )
  // {
    // //- Get First child
    // HTREEITEM hChild = GetChildItem( hItem );
    // if ( hChild != NULL ) return hChild;
    // //- Return a previous sibling item if it exists
    // HTREEITEM hti;
    // if ( hti = GetNextSiblingItem( hItem ) ) return hti;
    // //- No children or siblings, try parent's sibling
    // HTREEITEM hParent = GetParentItem( hItem );
    // while ( hParent != NULL )
    // {
      // if ( hti = GetNextSiblingItem( hParent ) ) return hti;
      // hParent = GetParentItem( hParent );
    // }
    // //- No sibling, return NULL
    // return NULL;
  // }

  HTREEITEM GetNextItem( HTREEITEM hItem )
  {
    HTREEITEM hti = NULL;

    if ( GetTreeCtrl().ItemHasChildren(hItem) )
      hti = GetTreeCtrl().GetChildItem(hItem);

    if (hti == NULL)
    {
      while ( ( hti = GetTreeCtrl().GetNextSiblingItem(hItem) ) == NULL )
      {
	      if ( ( hItem = GetTreeCtrl().GetParentItem(hItem) ) == NULL )
          return NULL;
	    }
    }
    return hti;
  }

  HTREEITEM GetLastItem( HTREEITEM hItem )
  {
    // Last child of the last child of the last child ...
    HTREEITEM htiNext;

    if( hItem == NULL )
    {
      // Get the last item at the top level
      htiNext = GetRootItem();
      while( htiNext )
      {
        hItem = htiNext;
        htiNext = GetNextSiblingItem( htiNext );
      }
    }

    while( GetTreeCtrl().ItemHasChildren( hItem ) )
    {
      htiNext = GetChildItem( hItem );
      while( htiNext )
      {
        hItem = htiNext;
        htiNext = GetNextSiblingItem( htiNext );
      }
    }

    return hItem;
  }


  // GetPrevItem  - Get previous item as if outline was completely expanded
  // Returns              - The item immediately above the reference item
  // hItem                - The reference item
  HTREEITEM GetPrevItem( const HTREEITEM hItem )
  {
    HTREEITEM hti = GetPrevSiblingItem(hItem);
    if( hti == NULL ) hti = GetParentItem(hItem);
    else              hti = GetLastItem(hti);
    return hti;
  }

  HTREEITEM InsertAnItem( const CString& strText,
                         int iImage,
                         HTREEITEM treeParent      = TVI_ROOT,
                         HTREEITEM treeInsertAfter = TVI_LAST  )
  //HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST
  {
    TV_ITEM             tvitemDummy;
    TV_INSERTSTRUCT     tvinsDummy;
    char szText[500];
    wsprintf( szText, "%s", LPCTSTR( strText ) );
    memset( &tvitemDummy, 0, sizeof( tvitemDummy ) );
    memset( &tvinsDummy,  0, sizeof( tvinsDummy ) );
    tvitemDummy.mask            = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvitemDummy.pszText         = szText;
    tvitemDummy.cchTextMax      = strText.GetLength();
    tvitemDummy.iImage          = iImage;
    tvitemDummy.iSelectedImage  = iImage;
    tvinsDummy.hParent          = treeParent;
    tvinsDummy.hInsertAfter     = treeInsertAfter;
    tvinsDummy.item             = tvitemDummy;
    HTREEITEM treeDummy         = GetTreeCtrl().InsertItem( &tvinsDummy );
    // GetTreeCtrl().Expand( treeDummy, TVE_EXPAND );
    return treeDummy;
  }
//  GetTreeHeirarchy	 -	Retrieves the tree heirarchy as a string comprising of item
			//	labels from root node to selected node
//  hTItem		 -	The HTREEITEM of selected item
//  cstrTreeHierarchy  -	String to store the Tree heirarchy
//  bTopToBottom	 -	TRUE  if the path should be from root to selected item
			//	FALSE if the path should be from the selected item to the root


  // void GetTreeHierarchy(HTREEITEM hTItem, CString &cstrTreeHierarchy, const BOOL bTopToBottom)
  // {
// 	  HTREEITEM hTRootItem = GetParentItem(hTItem);
//
    // if ( hTRootItem == NULL )
    // {
// 		  cstrTreeHierarchy += (GetItemText(hTItem)+"\\");
// 		  return;
// 	  }

// 	  if (!bTopToBottom)    cstrTreeHierarchy += (GetItemText(hTItem)+"\\");
//
    // GetTreeHierarchy(hTRootItem, cstrTreeHierarchy);
//
    // if (bTopToBottom) 	  cstrTreeHierarchy += (GetItemText(hTItem)+"\\");
  // }



  // CopyItem             - Copies an item to a new location
  // Returns              - Handle of the new item
  // hItem                - Item to be copied
  // htiNewParent         - Handle of the parent for new item
  // htiAfter             - Item after which the new item should be created
  HTREEITEM CopyItem( HTREEITEM hItem, HTREEITEM htiNewParent,
                                       HTREEITEM htiAfter = TVI_LAST )
  {
    TV_INSERTSTRUCT         tvstruct;
    HTREEITEM               hNewItem;
    CString                 sText;

    // get information of the source item
    tvstruct.item.hItem = hItem;
    tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE |
                            TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    GetTreeCtrl().GetItem(&tvstruct.item);
    sText = GetTreeCtrl().GetItemText( hItem );

    tvstruct.item.cchTextMax = sText.GetLength();
    tvstruct.item.pszText = sText.LockBuffer();

    // Insert the item at proper location
    tvstruct.hParent = htiNewParent;
    tvstruct.hInsertAfter = htiAfter;
    tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    hNewItem = GetTreeCtrl().InsertItem(&tvstruct);
    sText.ReleaseBuffer();

    // Now copy item data and item state.
    GetTreeCtrl().SetItemData( hNewItem, GetTreeCtrl().GetItemData( hItem ));
    GetTreeCtrl().SetItemState( hNewItem, GetTreeCtrl().GetItemState( hItem, TVIS_STATEIMAGEMASK ),
                                                    TVIS_STATEIMAGEMASK );

    // Call virtual function to allow further processing in derived class
    OnItemCopied( hItem, hNewItem );

    return hNewItem;
  }

protected:
  virtual void OnItemCopied(HTREEITEM /*hItem*/, HTREEITEM /*hNewItem*/ )
  {
          // Virtual function
    //- hier kann der Abglaich stattfinden
  }


public:

  // CopyBranch           - Copies all items in a branch to a new location
  // Returns              - The new branch node
  // htiBranch            - The node that starts the branch
  // htiNewParent - Handle of the parent for new branch
  // htiAfter             - Item after which the new branch should be created
  HTREEITEM CopyBranch( HTREEITEM htiBranch, HTREEITEM htiNewParent,
                                                  HTREEITEM htiAfter = TVI_LAST )
  {
    HTREEITEM hChild;

    HTREEITEM hNewItem = CopyItem( htiBranch, htiNewParent, htiAfter );
    hChild = GetTreeCtrl().GetChildItem(htiBranch);
    while( hChild != NULL)
    {
            // recursively transfer all the items
            CopyBranch(hChild, hNewItem);
            hChild = GetTreeCtrl().GetNextSiblingItem( hChild );
    }
    return hNewItem;
  }









};



// /////////////////////////////////////////////////////////////////////
// //-
// //-    CMFCTreeHelper NÖTIG?
// //-
// /////////////////////////////////////////////////////////////////////


// class CMFCTreeHelper
// {
// private:
  // mutable CTreeCtrl& m_Ctrl;
  // CMFCTreeHelper();
  // CMFCTreeHelper& operator=( const CMFCTreeHelper& rhs );
  // // CMFCTreeHelper& operator=( const CMFCTreeHelper& rhs )
  // // {
    // // if ( this == &rhs ) return *this;
    // // m_Ctrl = rhs.m_Ctrl;
    // // return *this;
  // // }
// public:
  // CMFCTreeHelper( CTreeCtrl& ctrl )
    // : m_Ctrl( ctrl ) {}
  // CMFCTreeHelper( const CMFCTreeHelper& rhs )
    // : m_Ctrl( rhs.m_Ctrl ) {}

  // inline CTreeCtrl& GetTreeCtrl() const { return m_Ctrl; }

  // ////////////////////////////////////////////////
  // //-    TreeControl-Wrapperfunktionen
  // ////////////////////////////////////////////////
  // inline GR::String  GetItemText( const HTREEITEM hItem )                      { return GR::String( LPCTSTR( GetTreeCtrl().GetItemText( hItem ) ) ); }
  // inline bool         SetItemText( const HTREEITEM hItem, const GR::String& text )   { return FALSE != GetTreeCtrl().SetItemText( hItem, text.c_str() ); }
  // inline bool         SetItem( TVITEM* tvItem )                                       { return FALSE != GetTreeCtrl().SetItem( tvItem ); }
  // inline bool         GetItem( TVITEM* tvItem )                                       { return FALSE != GetTreeCtrl().GetItem( tvItem ); }
  // inline long         GetItemData( const HTREEITEM hItem ) const                      { return GetTreeCtrl().GetItemData( hItem ); }
  // inline bool         SetItemData( const HTREEITEM hItem, const long data )           { return FALSE != GetTreeCtrl().SetItemData( hItem, data ); }
  // inline HTREEITEM    GetRootItem()                               const               { return GetTreeCtrl().GetRootItem(); }
  // inline HTREEITEM    GetChildItem(       const HTREEITEM hItem ) const               { return GetTreeCtrl().GetChildItem( hItem ); }
  // inline HTREEITEM    GetParentItem(      const HTREEITEM hItem ) const               { return GetTreeCtrl().GetParentItem( hItem ); }
  // inline HTREEITEM    GetNextSiblingItem( const HTREEITEM hItem ) const               { return GetTreeCtrl().GetNextSiblingItem( hItem ); }
  // inline HTREEITEM    GetPrevSiblingItem( const HTREEITEM hItem ) const               { return GetTreeCtrl().GetPrevSiblingItem( hItem ); }

  // inline bool DeleteAllItems() const { return FALSE != GetTreeCtrl().DeleteAllItems(); }
  // ////////////////////////////////////////////////
  // //-    Test, ob zwei Items Gechwister sind
  // ////////////////////////////////////////////////
  // bool AreSiblings( const HTREEITEM hItem1, const HTREEITEM hItem2 )
  // {
    // return ( GetTreeCtrl().GetParentItem( hItem1 ) == GetTreeCtrl().GetParentItem( hItem2 ) );
  // }


  // //-GetNextItem
  // HTREEITEM GetNextItem( const HTREEITEM hItem )
  // {
    // //- Get First child
    // HTREEITEM hChild = GetChildItem( hItem );
    // if ( hChild != NULL ) return hChild;
    // //- Return a previous sibling item if it exists
    // HTREEITEM hti;
    // if ( hti = GetNextSiblingItem( hItem ) ) return hti;
    // //- No children or siblings, try parent's sibling
    // HTREEITEM hParent = GetParentItem( hItem );
    // while ( hParent != NULL )
    // {
      // if ( hti = GetNextSiblingItem( hParent ) ) return hti;
      // hParent = GetParentItem( hParent );
    // }
    // //- No sibling, return NULL
    // return NULL;
  // }

  // // //-GetNextTreeItem
  // // HTREEITEM GetNextTreeItem( const HTREEITEM hItem ) //- ( Tiefe zuerst )
  // // {
    // // // Child-Item vorhanden?
    // // if ( GetChildItem( hItem ) )        return GetChildItem( hItem );
    // // // gleichwertiges Item vorhanden?
    // // if ( GetNextSiblingItem( hItem ) )  return GetNextSiblingItem( hItem );
    // // // dann müssen wir wieder eine/mehrere Ebenen zurück
    // // HTREEITEM hDummy = GetParentItem( hItem );
    // // while ( GetNextSiblingItem( hDummy ) == NULL )
    // // {
      // // hDummy = GetParentItem( hDummy );
      // // if ( hDummy == NULL ) // dann müssen ja alle durch sein
        // // return NULL;
    // // }
    // // return GetNextSiblingItem( hDummy );
  // // }

  // /*-GetTreeItemLevel-----------------------------------------------------------+
   // |                                                                            |
   // +----------------------------------------------------------------------------*/
  // long GetTreeItemLevel( const HTREEITEM treeItem ) const
  // {
    // long         dwLevel   = 0;
    // HTREEITEM     treeDummy = treeItem;
    // while ( treeDummy = GetParentItem( treeDummy ) ) ++dwLevel;
    // return dwLevel;
  // }



  // // ////////////////////////////////////////////////
  // // //-    GetAllChilds
  // // ////////////////////////////////////////////////
  // // //- Alle Childs in Liste holen
  // // TreeItemList CDialogEditorTreeView::GetAllChilds( const HTREEITEM hItem )
  // // {
    // // TreeItemList theChildList;
    // // _AttachChilds( hItem, theChildList );
    // // return theChildList;
  // // }

  // // ////////////////////////////////////////////////
  // // //-    _AttachChilds
  // // ////////////////////////////////////////////////
  // // //- private, rekursive unterfunktion von GetAllChilds
  // // void CDialogEditorTreeView::_AttachChilds( const HTREEITEM hItem, TreeItemList &listChilds )
  // // {
    // // HTREEITEM hChildItem;

    // // // Erstes Child finden
    // // hChildItem = GetChildItem( hItem );

    // // while ( hChildItem != NULL )
    // // {
      // // //- an Liste hängen
      // // listChilds.push_back( hChildItem );

      // // //- Child soll seine Kinder auch anhängen
      // // _AttachChilds( hChildItem, listChilds );

      // // //- und mit nächstem Geschwister weitermachen
      // // hChildItem =  GetNextSiblingItem( hChildItem );
    // // }
  // // }





// };


// /////////////////////////////////////////////////////////////////////
// //-
// //-    CTreeWrapper
// //- Kennt den TYP der Itemdatas UND Iteratoren
// //-
// /////////////////////////////////////////////////////////////////////

// template < class T >
// class CTreeWrapper
// {
// private:
  // CMFCTreeHelper m_Helper;
  // CTreeWrapper();
// public:
  // CTreeWrapper( CTreeCtrl& ctrl )
    // : m_Helper( ctrl ) {}
  // CTreeWrapper( const CTreeWrapper& rhs )
    // : m_Helper( rhs.m_Helper ) {}
  // CTreeWrapper& operator=( const CTreeWrapper& rhs )
  // {
    // if ( this == &rhs ) return *this;
    // m_Helper = rhs.m_Helper;
    // return *this;
  // }

  // inline CTreeCtrl& GetTreeCtrl() { return m_Helper.GetTreeCtrl(); }


// private:
  // // ////////////////////////////////////////////////
  // // //-    GetItemElement
  // // ////////////////////////////////////////////////
  // // T GetItemElement( const HTREEITEM hItem ) const
  // // {
    // // if ( !hItem ) return NULL;

    // // return static_cast<T>( m_Helper.GetItemData( hItem ) );
  // // }








// public:

  // /////////////////////////////////////////////////////////////////////
  // //-
  // //-    Iteratoren
  // //-
  // /////////////////////////////////////////////////////////////////////



  // class iterator
  // {
    // friend CTreeWrapper;
  // private:
    // CMFCTreeHelper*  m_pHelper;
    // HTREEITEM        m_treeCurrent;
    // T*               m_ptr;

    // //- Initialisierender Konstruktor
    // iterator( const CMFCTreeHelper& treectrl, const HTREEITEM item )
      // : m_pHelper( &treectrl ),
        // m_treeCurrent( item ),
        // m_ptr( NULL )
    // {
      // if ( m_myTree && m_treeCurrent )
      // {
        // m_ptr = static_cast<T*> m_pHelper.GetItemData( m_treeCurrent )
      // }

      // // _deref();
    // }

  // public:
    // //- Konstruktor
    // iterator()
      // : m_pHelper( NULL ),
        // m_treeCurrent( NULL ),
        // m_ptr( NULL )
    // {
    // }

    // //- Copy-Constructor
    // iterator( const iterator& rhs )
      // : m_pHelper(       rhs.m_pHelper ),
        // m_treeCurrent(  rhs.m_treeCurrent )
        // m_ptr(          rhs.m_ptr )
    // {
    // }

    // //- Zuweisung
    // iterator& operator=( const iterator& rhs )
    // {
      // if ( this == &rhs ) return *this;
      // m_pHelper     =  rhs.m_pHelper;
      // m_treeCurrent =  rhs.m_treeCurrent;
      // m_ptr         =  rhs.m_ptr;
      // return *this;
    // }

    // //- Vergleich
    // bool operator==( const iterator& rhs ) const
    // {
      // if ( m_ptr != rhs.m_ptr ) return false;
      // return true;
    // }

    // bool operator!=( const iterator& rhs ) const
    // {
      // return !operator==( rhs );
    // }

    // //- Dereferierung
    // T& operator *()
    // {
      // _deref();
      // return *m_ptr;
    // }

    // T* operator ->() { return &**this; }


    // ////////////////////////////////////////////////
    // //-    Increment (depth-first, backtracing)
    // ////////////////////////////////////////////////
    // iterator& operator ++()
    // {
      // _inc();
      // _deref();
      // return *this;
    // }

    // iterator operator ++( int )
    // {
      // iterator temp( *this );
      // _inc();
      // _deref();
      // return temp;
    // }

  // private:
    // void _deref()
    // {
      // if ( !m_Helper  )
      // {
        // m_treeCurrent = NULL;
      // }

      // if ( !m_treeCurrent )
      // {
        // m_ptr         = NULL;
        // return;
      // }

      // m_ptr = reinterpret_cast<T*>( m_Helper->GetItemData( m_treeCurrent ) );
    // }

    // void _inc()
    // {
      // if ( !m_Helper || !m_treeCurrent ) return;

      // m_treeCurrent = m_Helper->GetNextItem( m_TreeCurrent );
    // }
  // };



  // iterator begin() { return iterator( m_Helper, m_Helper.GetRootItem()  ); }
  // iterator end()   { return iterator( m_Helper, NULL                    ); }


  // // insert()  {}
  // // erase()   {}


// };

}; //namespace PJ

#endif//_PJ_TREEWRAPPER_H
