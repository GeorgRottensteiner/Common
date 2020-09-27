#include <Setup\Version.h>

#include <tchar.h>

#include <String/Convert.h>



bool GetFileVersion( const GR::String& FileName, GR::u32& Major, GR::u32& Minor, GR::u32& Revision, GR::u32& BuildNumber )
{

  static BOOL (FAR WINAPI *MyGetFileVersionInfoSize)( LPCWSTR, LPDWORD );
  static BOOL (FAR WINAPI *MyGetFileVersionInfo)( LPCWSTR, DWORD, DWORD, LPVOID );
  static BOOL (FAR WINAPI *MyVerQueryValue)( const LPVOID, LPCWSTR, LPVOID *, PUINT );


  // Rückgabewerte auf Dummies setzen
  Major       = 0;
  Minor       = 0;
  Revision    = 0;
  BuildNumber = 0;

  // damit wir die VERSION.LIB nicht zum Projekt linken müssen
  HINSTANCE hVersionLib = LoadLibraryA( "version.dll" );
  if ( hVersionLib == NULL )
  {
    return false;
  }

  MyGetFileVersionInfoSize = (int (_stdcall *)( LPCWSTR,unsigned long *))GetProcAddress( hVersionLib, "GetFileVersionInfoSizeW" );
  if ( MyGetFileVersionInfoSize == NULL )
  {
    FreeLibrary( hVersionLib );
    return false;
  }
  MyGetFileVersionInfo = (int (_stdcall *)( LPCWSTR, DWORD, DWORD, LPVOID ))GetProcAddress( hVersionLib, "GetFileVersionInfoW" );
  if ( MyGetFileVersionInfo == NULL )
  {
    FreeLibrary( hVersionLib );
    return false;
  }
  MyVerQueryValue= (int (_stdcall *)( const LPVOID, LPCWSTR, LPVOID *, PUINT ))GetProcAddress( hVersionLib, "VerQueryValueW" );
  if ( MyVerQueryValue == NULL )
  {
    FreeLibrary( hVersionLib );
    return false;
  }


  GR::Char*   lpVersion;

  UINT        uVersionLen;

  GR::u32     dwVerHnd = 0,         // An 'ignored' parameter, always '0'
              dwVerInfoSize = MyGetFileVersionInfoSize( GR::Convert::ToUTF16( FileName ).c_str(), (LPDWORD)&dwVerHnd );


  if ( !dwVerInfoSize )
  {
    // die Größe für das File-Info-Struct konnte nicht festgestellt werden
    FreeLibrary( hVersionLib );
    return false;
  }

  // Speicher für File-Info-Struct belegen
  GR::Char* lpstrVffInfo = (GR::Char*)malloc( dwVerInfoSize );
  if ( lpstrVffInfo == NULL )
  {
    FreeLibrary( hVersionLib );
    return false;
  }

  // Info anfordern
  if ( !MyGetFileVersionInfo( GR::Convert::ToUTF16( FileName ).c_str(), dwVerHnd, dwVerInfoSize, lpstrVffInfo ) )
  {
    free( lpstrVffInfo );
    FreeLibrary( hVersionLib );
    return false;
  }

  /* The below 'hex' value looks a little confusing, but
	 essentially what it is, is the hexidecimal representation
	 of a couple different values that represent the language
	 and character set that we are wanting string values for.
	 040904E4 is a very common one, because it means:
	 US English, Windows MultiLingual characterset
	 Or to pull it all apart:
	 04------        = SUBLANG_ENGLISH_USA
	 --09----        = LANG_ENGLISH
	 ----04E4 = 1252 = Codepage for Windows:Multilingual*/

  static GR::WChar  fileVersion[256];

  LPVOID            version = NULL;

  GR::u32           langD;

  BOOL              retVal;


  swprintf_s( fileVersion, 256, L"\\VarFileInfo\\Translation" );
  retVal = MyVerQueryValue( lpstrVffInfo, fileVersion, &version, (UINT*)&uVersionLen );
  if ( ( retVal ) 
  &&   ( uVersionLen == 4 ) )
  {
	memcpy( &langD, version, 4 );
    swprintf_s( fileVersion, 256, L"\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion",
                ( langD & 0xff00 ) >> 8, 
                langD & 0xff, 
                ( langD & 0xff000000 ) >> 24, 
                ( langD & 0xff0000 ) >> 16 );			
  }
  else 
  {
    swprintf_s( fileVersion, 256, L"\\StringFileInfo\\%04X04B0\\FileVersion", GetUserDefaultLangID() );
  }

  if ( !MyVerQueryValue( lpstrVffInfo, fileVersion, (LPVOID *)&lpVersion, (UINT *)&uVersionLen ) )
  {	
    free( lpstrVffInfo );
    FreeLibrary( hVersionLib );
    return FALSE;
  }

  // Now we have a string that looks like this :
  // "MajorVersion.MinorVersion.BuildNumber", so let's parse it

  FreeLibrary( hVersionLib );

  char*       sepChar = ".";
  if ( strchr( lpVersion, '.' ) == NULL )
  {
    sepChar = ",";
  }
  // Get first token (Major version number)
  GR::Char* tokenContext = NULL;
  GR::Char* token = strtok_s( lpVersion, sepChar, &tokenContext );
  if ( token == NULL )
  {
    free( lpstrVffInfo );
    return true;
  }
  Major = atoi( token );

  token = strtok_s( NULL, sepChar, &tokenContext );
  if ( token == NULL )
  {
    free( lpstrVffInfo );
    return true;
  }
  Minor = atoi( token );

  token = strtok_s( NULL, sepChar, &tokenContext );
  if ( token == NULL )
  {
    free( lpstrVffInfo );
    return true;
  }
  Revision = atoi( token );

  token = strtok_s( NULL, sepChar, &tokenContext );
  if ( token == NULL )
  {
    free( lpstrVffInfo );
    return true;
  }
  BuildNumber = atoi( token );

  free( lpstrVffInfo );

  return true;
}



bool GetResourceVersion( HINSTANCE hInstance, const GR::Char* szResName, GR::u32& Major, GR::u32& Minor, GR::u32& Revision, GR::u32& BuildNumber )
{
  static BOOL (FAR WINAPI *MyVerQueryValue)( const LPVOID, LPWSTR, LPVOID *, PUINT );

  HINSTANCE   hVersionLib;

  // Rückgabewerte auf Dummies setzen
  Major       = 0;
  Minor       = 0;
  Revision    = 0;
  BuildNumber = 0;


  LPWSTR      resourceName = 0;

  if ( IS_INTRESOURCE( szResName ) )
  {
    resourceName = (LPWSTR)szResName;
  }
  else
  {
    resourceName = (LPWSTR)GR::Convert::ToUTF16( szResName ).c_str();
  }
  HRSRC hrSrc = FindResourceW( hInstance, resourceName, RT_VERSION );
  if ( hrSrc == NULL )
  {
    //MessageBox( NULL, "not found", "oh", MB_TOPMOST );
    return FALSE;
  }

  HGLOBAL hGlob = LoadResource( hInstance, hrSrc );
  if ( hGlob == NULL )
  {
    //MessageBox( NULL, "LoadResource failed", "oh", MB_TOPMOST );
    return FALSE;
  }

  void *pData = LockResource( hGlob );

  // damit wir die VERSION.LIB nicht zum Projekt linken müssen
  hVersionLib = LoadLibraryA( "version.dll" );
  if ( hVersionLib == NULL )
  {
    //MessageBox( NULL, "LoadLibrary failed", "oh", MB_TOPMOST );
    return FALSE;
  }

  MyVerQueryValue= (int (_stdcall *)( const LPVOID, LPWSTR, LPVOID *, PUINT ))GetProcAddress( hVersionLib, "VerQueryValueW" );
  if ( MyVerQueryValue == NULL )
  {
    FreeLibrary( hVersionLib );
    //MessageBox( NULL, "MyVerQueryValue failed", "oh", MB_TOPMOST );
    return FALSE;
  }


  UINT        uVersionLen;

  DWORD       dwVerHnd = 0,         // An 'ignored' parameter, always '0'
              dwVerInfoSize = SizeofResource( hInstance, hrSrc );


  if ( !dwVerInfoSize )
  {
    // die Größe für das File-Info-Struct konnte nicht festgestellt werden
    FreeLibrary( hVersionLib );
    //MessageBox( NULL, "SizeofResource failed", "oh", MB_TOPMOST );
    return FALSE;
  }

  GR::u8*   pTempData = new GR::u8[dwVerInfoSize + 4];

  memcpy( pTempData, pData, dwVerInfoSize );
  memset( pTempData + dwVerInfoSize, 0, 4 );

	/* The below 'hex' value looks a little confusing, but
		 essentially what it is, is the hexidecimal representation
		 of a couple different values that represent the language
		 and character set that we are wanting string values for.
		 040904E4 is a very common one, because it means:
		 US English, Windows MultiLingual characterset
		 Or to pull it all apart:
		 04------        = SUBLANG_ENGLISH_USA
		 --09----        = LANG_ENGLISH
		 ----04E4 = 1252 = Codepage for Windows:Multilingual*/

	static GR::WChar     fileVersion[256];

  /*
	LPVOID          version = NULL;

	DWORD           langD;

	BOOL            retVal;


	sprintf( fileVersion, "\\VarFileInfo\\Translation" );
	retVal = MyVerQueryValue( pData, fileVersion, &version, (UINT*)&uVersionLen );
	if ( ( retVal ) 
  &&   ( uVersionLen == 4 ) )
	{
		memcpy( &langD, version, 4 );
		sprintf( fileVersion, "\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion",
             ( langD & 0xff00 ) >> 8, 
             langD & 0xff, 
             ( langD & 0xff000000 ) >> 24, 
             ( langD & 0xff0000 ) >> 16 );			

    MessageBox( NULL, fileVersion, "oh", MB_TOPMOST );
	}
	else 
  {
		sprintf( fileVersion, "\\StringFileInfo\\%04X04B0\\FileVersion", GetUserDefaultLangID() );
  }
  */

  // irgendwie klappt das drüber nicht beim Lesen der Ressource
  LPVOID          version = NULL;

  swprintf_s( fileVersion, 256, L"\\" );
	if ( !MyVerQueryValue( pTempData, fileVersion, &version, (UINT *)&uVersionLen ) )
  {	
    FreeLibrary( hVersionLib );
		return FALSE;
  }

  VS_FIXEDFILEINFO    *pVFFI = (VS_FIXEDFILEINFO*)version;

  Major       = ( ( pVFFI->dwProductVersionMS & 0xffff0000 ) >> 16 );
  Minor       =     pVFFI->dwProductVersionMS & 0xffff;
  Revision    = ( ( pVFFI->dwProductVersionLS & 0xffff0000 ) >> 16 );
  BuildNumber =     pVFFI->dwProductVersionLS & 0xffff;
  
  delete[] pTempData;

  FreeLibrary( hVersionLib );

  return TRUE;
}
