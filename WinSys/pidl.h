#ifndef PIDL_H
#define PIDL_H

#include <shlobj.h>


#ifdef __cplusplus
extern "C" {
#endif


namespace WIN
{

  //  FUNCTION:   Pidl_GetSize 
  //  PURPOSE:    Returns the total number of bytes in an ITEMIDLIST.
  //  PARAMETERS:
  //      pidl - Pointer to the ITEMIDLIST that you want the size of.

  UINT Pidl_GetSize( LPCITEMIDLIST pidl );

  //  FUNCTION:   Pidl_Concatenate
  //  PURPOSE:    Creates a new ITEMIDLIST with pidl2 appended to pidl1.
  //
  //  PARAMETERS:
  //		piMalloc - Pointer to the allocator interface that should create the
  //				   new ITEMIDLIST.
  //    pidl1 	 - Pointer to an ITEMIDLIST that contains the root.
  //		pidl2    - Pointer to an ITEMIDLIST that contains what should be 
  //				   appended to the root.
  //
  //  RETURN VALUE:
  //  	Returns a new ITEMIDLIST if successful, NULL otherwise.  

  LPITEMIDLIST Pidl_Concatenate( LPMALLOC piMalloc, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2 );
                                
  //  FUNCTION:   Pidl_Copy 
  //  PURPOSE:    Creates a new copy of an ITEMIDLIST.
  //
  //  PARAMETERS:
  //      piMalloc   - Pointer to the allocator interfaced to be used to allocate
  //				     the new ITEMIDLIST.
  //		pidlSource - The ITEMIDLIST that you want to create a copy of
  //
  //  RETURN VALUE:
  //      Returns a pointer to the new ITEMIDLIST, or NULL if an error occurs.
                                          
  LPITEMIDLIST Pidl_Copy( LPMALLOC piMalloc, LPCITEMIDLIST pidlSource );

  //  FUNCTION:   Pidl_GetFullyQualified 
  //  PURPOSE:    Takes a relative PIDL and it's parent IShellFolder, and returns
  //				a fully qualified ITEMIDLIST.
  //
  //  PARAMETERS:
  //      piParentFolder - Pointer to the IShellFolder of the parent folder.
  //		pidl		   - ITEMIDLIST relative to piParentFolder
  //
  //  RETURN VALUE:
  //      Returns a fully qualified ITEMIDLIST or NULL if there is a problem.

  LPITEMIDLIST Pidl_GetFullyQualified( LPSHELLFOLDER piParentFolder, LPITEMIDLIST pidl );

  //  FUNCTION:   Pidl_GetDisplayName 
  //  PURPOSE:    Returns the display name for the item pointed to by pidl.  The
  //				function assumes the pidl is relative to piFolder.  If piFolder
  //				is NULL, the function assumes the item is fully qualified.
  //
  //  PARAMETERS:
  //      piFolder - Pointer to the IShellFolder for the folder containing the 
  //				   item.
  //		pidl	 - Pointer to an ITEMIDLIST relative to piFolder that we want
  //				   the display name for.
  //		dwFlags  - Flags to pass to ISF::GetDisplayNameOf().
  //		pszName  - Pointer to the string where the display name is returned.
  //		cchMax   - Maximum number of characters in pszName.
  //
  //  RETURN VALUE:
  //      Returns TRUE if successful, FALSE otherwise.

  BOOL Pidl_GetDisplayName( LPSHELLFOLDER piFolder, LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pszName, UINT cchMax );

  //  FUNCTION:   Pidl_GetRelative
  //
  //  PURPOSE:    Takes a fully qualified pidl and returns the the relative pidl
  //				and the root part of that pidl.
  //
  //  PARAMETERS:
  //      piMalloc - Pointer to the allocator used for creating the new pidls.
  //		pidlFQ   - Pointer to the fully qualified ITEMIDLIST that needs to be
  //				   parsed.
  //		pidlRoot - Points to the pidl that will contain the root after parsing.
  //		pidlItem - Points to the item relative to pidlRoot after parsing.
  //
  //  RETURN VALUE:
  //      Returns TRUE if successful, FALSE otherwise.

  BOOL Pidl_GetRelative( LPMALLOC piMalloc, LPITEMIDLIST pidlFQ, LPITEMIDLIST* ppidlRoot, LPITEMIDLIST* ppidlRelative );

  //  FUNCTION:   Pidl_GetFromPath 
  //
  //  PURPOSE:    This routine takes a full path to a file and converts that
  //				to a fully qualified ITEMIDLIST.
  //
  //  PARAMETERS:
  //      pszFile  - Full path to the file.
  //  RETURN VALUE:
  //      Returns a fully qualified ITEMIDLIST, or NULL if an error occurs.
  //

  LPITEMIDLIST Pidl_GetFromPath( LPTSTR pszFile );

  //  FUNCTION:   Pidl_GetFileFolder 
  //
  //  PURPOSE:    This routine takes a fully qualified pidl for a folder and returns 
  //				the IShellFolder pointer for that pidl
  //
  //  PARAMETERS:
  //		pidl	 - Pointer to a fully qualified ITEMIDLIST for the folder
  //
  //  RETURN VALUE:
  //      returns a Pointer to the IShellFolder for the folder, NULL otherwise.

  LPSHELLFOLDER Pidl_GetFileFolder( LPITEMIDLIST pidl );

  //  FUNCTION:   Pidl_GetFromParentFolder 
  //
  //  PURPOSE:    This routine takes a Shell folder for the parent and the FileName in the folder
  //				and converts that to a relative ITEMIDLIST.
  //
  //  PARAMETERS:
  //      pParentFolder - Pointer to the IShellFolder for the folder containing the 
  //				   fileName.
  //      pszFile  - file name in the folder.
  //
  //  RETURN VALUE:
  //      Returns a relative ITEMIDLIST, or NULL if an error occurs.

  LPITEMIDLIST Pidl_GetFromParentFolder( LPSHELLFOLDER pParentFolder, LPTSTR pszFile );

  //  FUNCTION:   Pidl_Dump 
  //
  //  PURPOSE:    Displays the contents of a PIDL in a readable form.  The output
  //				is displayed in the debugger.
  //
  //  PARAMETERS:
  //      piParent - If the PIDL is relative to a folder, this is the pointer to 
  //				   the IShellFolder for that folder.
  //		pidl	 - Pointer to the ITEMIDLIST that needs to be dumped.

  void Pidl_Dump( LPSHELLFOLDER piParent, LPCITEMIDLIST pidl );

};


#ifdef __cplusplus
}
#endif

#endif // PIDL_H
