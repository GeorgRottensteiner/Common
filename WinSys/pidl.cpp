#include <windows.h>
#include <shlobj.h>

#include "pidl.h"


namespace WIN
{

inline LPITEMIDLIST Pidl_GetNextItem( LPCITEMIDLIST pidl )
{

	if ( pidl == NULL )
  {
    return NULL;
  }
	return (LPITEMIDLIST)(LPBYTE)( ( (LPBYTE)pidl ) + pidl->mkid.cb );

}


UINT Pidl_GetSize( LPCITEMIDLIST pidl )
{

	UINT          cbTotal = 0;

	LPITEMIDLIST  pidlTemp = (LPITEMIDLIST)pidl;


	if ( pidlTemp )
	{
		while ( pidlTemp->mkid.cb )
		{
			cbTotal += pidlTemp->mkid.cb;
			pidlTemp = Pidl_GetNextItem( pidlTemp );
		}	

		// Requires a 16 bit zero value for the NULL terminator
		cbTotal += 2 * sizeof( BYTE );

	}
	return cbTotal;

}



LPITEMIDLIST Pidl_Create( LPMALLOC piMalloc, UINT cbSize )
{

	LPITEMIDLIST pidl = NULL;

	pidl = (LPITEMIDLIST)piMalloc->Alloc( cbSize );
	if ( pidl )
  {
		ZeroMemory( pidl, cbSize );
  }
	return pidl;

}



LPITEMIDLIST Pidl_Concatenate( LPMALLOC piMalloc, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2 )
{

	LPITEMIDLIST  pidlNew;

	UINT          cb1, 
                cb2 = 0;


	if ( pidl1 )
  {
		cb1 = Pidl_GetSize( pidl1 ) - ( 2 * sizeof( BYTE ) );
  }
	cb2 = Pidl_GetSize( pidl2 );


  // Create a new ITEMIDLIST that is the size of both pidl1 and pidl2, then
	// copy pidl1 and pidl2 to the new list.
	//
	pidlNew = Pidl_Create( piMalloc, cb1 + cb2 );
	if ( pidlNew )
	{
		if ( pidl1 )
    {
			CopyMemory( pidlNew, pidl1, cb1 );
    }
		CopyMemory( ( (LPBYTE)pidlNew ) + cb1, pidl2, cb2 );
	}

	return pidlNew;

}



LPITEMIDLIST Pidl_Copy( LPMALLOC piMalloc, LPCITEMIDLIST pidlSource )
{

	LPITEMIDLIST  pidlTarget = NULL;

	UINT          cbSource = 0;


	if ( NULL == pidlSource )
  {
		return NULL;
  }

	// Allocate the new ITEMIDLIST
	cbSource    = Pidl_GetSize( pidlSource );
	pidlTarget  = (LPITEMIDLIST)piMalloc->Alloc( cbSource );
	if ( !pidlTarget )
  {
		return NULL;
  }

	// Copy the source to the target
	CopyMemory( pidlTarget, pidlSource, cbSource );

	return pidlTarget;

}



LPITEMIDLIST Pidl_GetFullyQualified( LPSHELLFOLDER piParentFolder, LPITEMIDLIST pidl )
{

	// Start by getting a pointer to the IShellFolder for the desktop since
	// all fully qualified pidls are paths from the desktop to the itemid.
	LPSHELLFOLDER piDesktopFolder;

	HRESULT hr = SHGetDesktopFolder( &piDesktopFolder );
	if ( FAILED( hr ) )
  {
		return NULL;
  }

	// Get the name of the item for parsing.  We use this to get the fully
	// qualified pidl from the desktop folder.
	TCHAR szBuffer[MAX_PATH];
	if ( !Pidl_GetDisplayName( piParentFolder, pidl, SHGDN_FORPARSING, szBuffer, sizeof( szBuffer ) ) )
	{
		piDesktopFolder->Release();
		return NULL;     	
	}	

	// Convert the path to a Unicode string which is required for the call
	// to IShellFolder::ParseDisplayName().
	OLECHAR szOleChar[MAX_PATH];

	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szBuffer, -1, szOleChar, sizeof( szOleChar ) );

	// Finally ask the desktop folder to give us a fully qualified pidl for
	// the item.
	ULONG         ulEaten, 
                ulAttribs;
	LPITEMIDLIST  pidlFQ;

	hr = piDesktopFolder->ParseDisplayName( NULL, NULL, szOleChar, &ulEaten, &pidlFQ, &ulAttribs );
	piDesktopFolder->Release();
	
	if ( FAILED( hr ) )	
  {
		return NULL;
  }

	return pidlFQ;

}	



BOOL Pidl_GetDisplayName( LPSHELLFOLDER piFolder, LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pszName, UINT cchMax )
{

	BOOL    fSuccess = TRUE;
	STRRET  str;
	BOOL    fDesktop = FALSE;
	

	// Check to see if a parent folder was specified.  If not, get a pointer
	// to the desktop folder.
	if ( NULL == piFolder )
	{
		HRESULT hr = SHGetDesktopFolder( &piFolder );
		if ( FAILED( hr ) )
    {
			return FALSE;
    }
	}

	// Get the display name from the folder.  Then do any conversions necessary
	// depending on the type of string returned.

	if ( NOERROR == piFolder->GetDisplayNameOf( pidl, dwFlags, &str ) )
	{
		switch ( str.uType )
		{
			case STRRET_WSTR:
				WideCharToMultiByte( CP_ACP, 0, str.pOleStr, -1, pszName, cchMax, NULL, NULL );
				break;
			case STRRET_OFFSET:
				lstrcpyn( pszName, (LPTSTR)pidl + str.uOffset, cchMax );
				break;
			case STRRET_CSTR:
				lstrcpyn( pszName, (LPTSTR) str.cStr, cchMax );
				break;
			default:
				fSuccess = FALSE;
				break;
		}
	}
	else
  {
		fSuccess = FALSE;
  }

	// Free the desktop folder interface if we allocated it.
	if ( fDesktop )
	{
		piFolder->Release();
		piFolder = NULL;
	}

	return fSuccess;

}



BOOL Pidl_GetRelative( LPMALLOC piMalloc, LPITEMIDLIST pidlFQ, LPITEMIDLIST* ppidlRoot, LPITEMIDLIST* ppidlItem )
{

	LPITEMIDLIST      pidlTemp, 
                    pidlNext;


	// Check to see if the pidlFQ is NULL (ie points to the desktop)
	if ( !pidlFQ )
	{
		*ppidlRoot = NULL;
		*ppidlItem = NULL;
		return TRUE;
	}

	// Create a copy of the fully qualified list.  When we get to the last
	// item in the list, we'll just truncate it there by inserting the 16-bit
	// zero value.  
	*ppidlRoot = Pidl_Copy( piMalloc, pidlFQ );
	pidlTemp = *ppidlRoot;

	// Now walk through the list looking for the last item
	while ( pidlTemp->mkid.cb )
	{
		// Get the next item and see if it's the NULL terminator at the end
		// of the list.
		pidlNext = Pidl_GetNextItem( pidlTemp );

		if ( 0 == pidlNext->mkid.cb )
		{
			// pidlTemp currently is pointing at the last item in the list, 
			// which is the relative item we want to copy to pidlItem.
			*ppidlItem = Pidl_Copy( piMalloc, pidlTemp );

			// Add the NULL item to terminate the list
			pidlTemp->mkid.cb = 0;
			pidlTemp->mkid.abID[0] = 0;
		}

		pidlTemp = pidlNext;
	}
	
	return TRUE;

}	



LPITEMIDLIST Pidl_GetFromPath( LPTSTR pszFile )
{

	// First get a pointer to the shell folder

	LPSHELLFOLDER     piDesktop;

	if ( FAILED( SHGetDesktopFolder( &piDesktop ) ) )
  {
		return NULL;
  }

	// Now convert the path to a Unicode string.  ISF::ParseDisplayName
	// requires a Unicode string.
	OLECHAR     olePath[MAX_PATH];

	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszFile, -1, olePath, sizeof( olePath ) );

	// Convert the path to an ITEMIDLIST
	LPITEMIDLIST  pidl;

	ULONG         chEaten, 
                dwAttributes;

	if ( FAILED( piDesktop->ParseDisplayName( NULL, NULL, olePath, &chEaten, &pidl, &dwAttributes ) ) )
  {
		return NULL;
  }
	else
  {
		return pidl;
  }

}



LPSHELLFOLDER Pidl_GetFileFolder( LPITEMIDLIST pidl )
{

	LPSHELLFOLDER       piDesktopFolder, 
                      pParentFolder;


	HRESULT hr = SHGetDesktopFolder( &piDesktopFolder );
	if ( FAILED( hr ) )
  {
		return NULL;
  }

	hr = piDesktopFolder->BindToObject( pidl, NULL, IID_IShellFolder, (LPVOID*)&pParentFolder ); 
	piDesktopFolder->Release();

	if ( FAILED( hr ) )
  {
		return NULL;
  }
	return pParentFolder;

}



LPITEMIDLIST Pidl_GetFromParentFolder( LPSHELLFOLDER pParentFolder, LPTSTR pszFile )
{

 	OLECHAR       olePath[MAX_PATH];

	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszFile, -1, olePath, sizeof( olePath ) );

	// Convert the path to an ITEMIDLIST
	LPITEMIDLIST      pidl;

	ULONG             chEaten, 
                    dwAttributes;

	if ( FAILED( pParentFolder->ParseDisplayName( NULL, NULL, olePath, &chEaten, &pidl, &dwAttributes ) ) )
  {
		return NULL;
  }
  return pidl;

}



}; // namespace WIN