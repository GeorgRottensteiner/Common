#include <String/StringUtil.h>

#include <String/Convert.h>

#include <tchar.h>

#include "RegistryHelper.h"




bool Registry::SetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, const GR::Char* Value )
{
  HKEY          hkeySub;

  unsigned long dwSize;

  GR::WString   utf16Branch = GR::Strings::UTF8ToUTF16( Branch );
  GR::WString   utf16Key    = GR::Strings::UTF8ToUTF16( Key );
  GR::WString   utf16Value  = GR::Strings::UTF8ToUTF16( Value );

  // must be null terminated!
  utf16Value.append( L"X" );
  utf16Value[utf16Value.length() - 1] = 0;

  GR::WChar   emptyString[2];
  emptyString[0] = 0;


  if ( RegOpenKeyExW( hKeyMain, utf16Branch.c_str(), 0, KEY_SET_VALUE, &hkeySub ) != ERROR_SUCCESS )
  {
    if ( RegCreateKeyExW( hKeyMain, utf16Branch.c_str(), 0, emptyString, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_SET_VALUE, NULL, &hkeySub, &dwSize ) != ERROR_SUCCESS )
    {
      return false;
    }
  }
  if ( Value )
  {
    dwSize = 512;
    if ( RegSetValueExW( hkeySub, utf16Key.c_str(), 0, REG_SZ, (BYTE*)utf16Value.c_str(), (unsigned long)( utf16Value.length() * 2 ) ) != ERROR_SUCCESS )
    {
      RegCloseKey( hkeySub );
      return false;
    }
  }

  RegCloseKey( hkeySub );

  return true;
}



bool Registry::SetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, void* pData, size_t iDataSize )
{
  HKEY          hkeySub;

  unsigned long         dwSize;


  GR::WString   utf16Branch = GR::Strings::UTF8ToUTF16( Branch );
  GR::WString   utf16Key = GR::Strings::UTF8ToUTF16( Key );

  GR::WChar   emptyString[2];
  emptyString[0] = 0;

  if ( RegOpenKeyExW( hKeyMain, utf16Branch.c_str(), 0, KEY_SET_VALUE, &hkeySub ) != ERROR_SUCCESS )
  {
    if ( RegCreateKeyExW( hKeyMain, utf16Branch.c_str(), 0, emptyString, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_SET_VALUE, NULL, &hkeySub, &dwSize ) != ERROR_SUCCESS )
    {
      return false;
    }
  }
  dwSize = 512;
  if ( RegSetValueExW( hkeySub, utf16Key.c_str(), 0, REG_BINARY, (BYTE*)pData, (unsigned long)iDataSize ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    return false;
  }

  RegCloseKey( hkeySub );

  return true;
}



bool Registry::GetKey( HKEY hKeyMain, const GR::String& Branch, const GR::String& Key, GR::String& Value )
{
  HKEY          hkeySub;

  unsigned long bufferSize,
                valueType;


  GR::WString   utf16Branch = GR::Strings::UTF8ToUTF16( Branch );
  GR::WString   utf16Key = GR::Strings::UTF8ToUTF16( Key );
  GR::WString   utf16Value = GR::Strings::UTF8ToUTF16( Value );

  Value.clear();
  if ( RegOpenKeyExW( hKeyMain, utf16Branch.c_str(), 0, KEY_QUERY_VALUE, &hkeySub ) != ERROR_SUCCESS )
  {
    return false;
  }

  if ( RegQueryValueExW( hkeySub, utf16Key.c_str(), NULL, &valueType, NULL, &bufferSize ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    return false;
  }

  GR::u8*   pValue = new GR::u8[bufferSize + 1]();
  if ( pValue == NULL )
  {
    RegCloseKey( hkeySub );
    return false;
  }
  pValue[bufferSize] = 0;

  if ( RegQueryValueExW( hkeySub, utf16Key.c_str(), NULL, &valueType, (BYTE*)pValue, &bufferSize ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    Value.clear();
    return false;
  }
  RegCloseKey( hkeySub );

  if ( valueType == REG_EXPAND_SZ )
  {
    GR::WString    result;
    result.resize( 32768 );
    ExpandEnvironmentStringsW( (GR::WChar*)pValue, &result[0], 32768 );
    Value = GR::Strings::UTF16ToUTF8( result );
  }
  else if ( valueType == REG_SZ )
  {
    Value = GR::Convert::ToUTF8( ( GR::WChar* )pValue );
  }
  else if ( valueType == REG_MULTI_SZ )
  {
    Value = GR::Convert::ToUTF8( ( GR::WChar* )pValue );
  }
  delete[] pValue;
  return true;
}



bool Registry::GetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, unsigned long& dwValue )
{
  HKEY          hkeySub;

  unsigned long dwSize,
                dwType;


  GR::WString   utf16Branch = GR::Strings::UTF8ToUTF16( Branch );
  GR::WString   utf16Key = GR::Strings::UTF8ToUTF16( Key );

  dwValue = 0;
  if ( RegOpenKeyExW( hKeyMain, utf16Branch.c_str(), 0, KEY_QUERY_VALUE, &hkeySub ) != ERROR_SUCCESS )
  {
    return false;
  }
  dwSize = sizeof( dwValue );
  if ( RegQueryValueExW( hkeySub, utf16Key.c_str(), NULL, &dwType, (BYTE*)&dwValue, &dwSize ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    return false;
  }
  RegCloseKey( hkeySub );

  return true;

}



bool Registry::GetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, void* pData, int iDataSize )
{
  HKEY          hkeySub;

  unsigned long dwSize,
                dwType;


  GR::WString   utf16Branch = GR::Strings::UTF8ToUTF16( Branch );
  GR::WString   utf16Key = GR::Strings::UTF8ToUTF16( Key );

  ZeroMemory( pData, iDataSize );
  if ( RegOpenKeyExW( hKeyMain, utf16Branch.c_str(), 0, KEY_QUERY_VALUE, &hkeySub ) != ERROR_SUCCESS )
  {
    return false;
  }
  dwSize = iDataSize;
  if ( RegQueryValueExW( hkeySub, utf16Key.c_str(), NULL, &dwType, (BYTE*)pData, &dwSize ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    return false;
  }
  RegCloseKey( hkeySub );

  return true;

}



bool Registry::DeleteBranch( HKEY hKeyMain, const GR::Char* Key )
{
  HKEY          hkeySub;

  GR::WString   utf16Key = GR::Strings::UTF8ToUTF16( Key );


  if ( RegOpenKeyExW( hKeyMain, NULL, 0, KEY_SET_VALUE, &hkeySub ) != ERROR_SUCCESS )
  {
    // den gibt's ja eh nicht
    return true;
  }
  if ( RegDeleteKeyW( hkeySub, utf16Key.c_str() ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    return false;
  }
  RegCloseKey( hkeySub );

  return true;

}



bool Registry::DeleteKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key )
{
  HKEY          hkeySub;

  GR::WString   utf16Branch = GR::Strings::UTF8ToUTF16( Branch );
  GR::WString   utf16Key = GR::Strings::UTF8ToUTF16( Key );


  if ( RegOpenKeyExW( hKeyMain, utf16Branch.c_str(), 0, KEY_SET_VALUE, &hkeySub ) != ERROR_SUCCESS )
  {
    return false;
  }
  if ( RegDeleteValueW( hkeySub, utf16Key.c_str() ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    return false;
  }
  RegCloseKey( hkeySub );

  return true;
}



bool Registry::IsBranchEmpty( HKEY hKeyMain, const GR::Char* Branch )
{
  HKEY          hkeySub;

  GR::WString   utf16Branch = GR::Strings::UTF8ToUTF16( Branch );


  if ( RegOpenKeyExW( hKeyMain, utf16Branch.c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkeySub ) != ERROR_SUCCESS )
  {
    return true;
  }

  unsigned long   dwSubKeys,
                  dwNumValues;

  if ( RegQueryInfoKey( hkeySub,
                        NULL,
                        NULL,
                        NULL,
                        &dwSubKeys,
                        NULL,
                        NULL,
                        &dwNumValues,
                        NULL,
                        NULL,
                        NULL,
                        NULL ) != ERROR_SUCCESS )
  {
    RegCloseKey( hkeySub );
    return true;
  }

  RegCloseKey( hkeySub );
  if ( ( dwSubKeys == 0 )
  &&   ( dwNumValues == 0 ) )
  {
    return true;
  }
  return false;
}



void Registry::RegisterFileType( const GR::Char* Extension, const GR::Char* AppDescription, 
                                 const GR::Char* IconPath, const GR::Char* OpenCommand )
{
  GR::String      key = AppDescription;
  key += ".Document";
  SetKey( HKEY_CLASSES_ROOT, Extension, NULL, key.c_str() );

  SetKey( HKEY_CLASSES_ROOT, key.c_str(), NULL, AppDescription );

  key = AppDescription;
  key += ".Document\\DefaultIcon";
  SetKey( HKEY_CLASSES_ROOT, key.c_str(), NULL, IconPath );

  key = AppDescription;
  key += ".Document\\shell\\open\\command";
  SetKey( HKEY_CLASSES_ROOT, key.c_str(), NULL, OpenCommand );

}



bool Registry::IsFileTypeRegisteredTo( const GR::Char* Extension, const GR::Char* AppDescription )
{
  GR::String     key = AppDescription;

  key += ".Document";
  GR::String      temp;

  GetKey( HKEY_CLASSES_ROOT, Extension, "", temp );


  if ( _stricmp( temp.c_str(), key.c_str() ) == 0 )
  {
    return true;
  }
  return false;
}



void Registry::UnregisterFileType( const GR::Char* Extension, const GR::Char* AppDescription )
{
  if ( !IsFileTypeRegisteredTo( Extension, AppDescription ) )
  {
    return;
  }

  DeleteBranch( HKEY_CLASSES_ROOT, Extension );
}



int Registry::IncreaseReference( const GR::Char* BranchName, const GR::Char* KeyName, bool bCurrentUser )
{
  GR::String        temp;
  GR::String        referencesKey = "Software\\";
  referencesKey += BranchName;

  HKEY    hKey = HKEY_LOCAL_MACHINE;
  if ( bCurrentUser )
  {
    hKey = HKEY_CURRENT_USER;
  }

  if ( GetKey( hKey, referencesKey.c_str(), KeyName, temp ) )
  {
    // Referenz hoch
    int iReferences = GR::Convert::ToI32( temp );
    iReferences++;

    temp = Misc::Format() << iReferences;
    SetKey( hKey, referencesKey.c_str(), KeyName, temp.c_str() );

    return iReferences;
  }
  // es gibt noch keinen Referenzeintrag, sehr gut
  SetKey( HKEY_CURRENT_USER, referencesKey.c_str(), KeyName, "1" );
  return 1;
}



bool Registry::DecreaseReference( const GR::Char* BranchName, const GR::Char* KeyName, bool bCurrentUser )
{
  GR::String        temp;
  GR::String        referencesKey = "Software\\";

  referencesKey += BranchName;

  HKEY            hKey = HKEY_LOCAL_MACHINE;

  if ( bCurrentUser )
  {
    hKey = HKEY_CURRENT_USER;
  }

  if ( Registry::GetKey( hKey, referencesKey.c_str(), KeyName, temp ) )
  {
    // Referenz runter
    int iReferences = GR::Convert::ToI32( temp );
    iReferences--;
    if ( iReferences <= 0 )
    {
      Registry::DeleteKey( hKey, referencesKey.c_str(), KeyName );

      if ( Registry::IsBranchEmpty( hKey, referencesKey.c_str() ) )
      {
        // Wenn Key empty, weg damit
        Registry::DeleteBranch( hKey, referencesKey.c_str() );
      }
    }
    else
    {
      SetKey( hKey, referencesKey.c_str(), KeyName, GR::Convert::ToStringA( iReferences ).c_str() );

      // false bedeutet, noch Referenzen darauf, nicht löschen
      return false;
    }
  }
  return true;
}



bool Registry::AddUnInstallEntry( const GR::Char* RegistryKey, 
                                  const GR::Char* Description, 
                                  const GR::Char* UninstallCommand )
{
  unsigned long     dwResult,
                    dwResult2;

  HKEY              hKey,
                    hChildKey;

  GR::Char          hs[500],
                    hsClass[500];


  // Prepare to navigate to the CurrentVersion key.
  hKey = HKEY_LOCAL_MACHINE;
  wsprintfA( hs, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion" );
  hChildKey = NULL;

  if ( !RegOpenKeyExA( hKey, hs, 0, KEY_READ | KEY_WRITE, &hChildKey ) == ERROR_SUCCESS )
  {
    return false;
  }
  // alles ok
  RegCloseKey( hKey );

  hKey = hChildKey;
  hChildKey = NULL;

  GR::String   regKeyChild = "Uninstall\\";
  regKeyChild += RegistryKey;
  wsprintfA( hsClass, "" );

  dwResult = 0;
  dwResult2 = 0;

  if ( !RegCreateKeyExA( hKey, regKeyChild.c_str(), 0, hsClass, REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, &hChildKey, &dwResult ) == ERROR_SUCCESS )
  {
    return false;
  }
  switch ( dwResult )
  {
    case REG_CREATED_NEW_KEY:
      // Strings eintragen, Handles freigeben, fertig

      RegSetValueExA( hChildKey, "DisplayName", 0, REG_SZ, (unsigned char *)Description, (unsigned long)strlen( Description ) );
      RegSetValueExA( hChildKey, "UninstallString", 0, REG_SZ, (unsigned char *)UninstallCommand, (unsigned long)strlen( UninstallCommand ) );

      RegCloseKey( hChildKey );
      RegCloseKey( hKey );
      break;
    case REG_OPENED_EXISTING_KEY:
      // Der Key war bereits eingetragen, löschen und neu eintragen
      RegCloseKey( hChildKey );
      RegDeleteKeyA( hKey, regKeyChild.c_str() );
      dwResult2 = 0;
      RegCreateKeyExA( hKey, regKeyChild.c_str(), 0, hsClass, REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS, NULL, &hChildKey, &dwResult2 );

      if ( dwResult2 == REG_CREATED_NEW_KEY )
      {
        // jetzt stimmt der Eintrag
        RegSetValueExA( hChildKey, "DisplayName", 0, REG_SZ, (unsigned char *)Description, (unsigned long)strlen( Description ) );

        RegSetValueExA( hChildKey, "UninstallString", 0, REG_SZ, (unsigned char *)UninstallCommand, (unsigned long)strlen( UninstallCommand ) );

        RegCloseKey( hChildKey );
        RegCloseKey( hKey );
        return true;
      }
      // The second attempt did not create a new key.
      return false;
    default:        // other status returned?
      RegCloseKey( hChildKey );
      RegCloseKey( hKey );
      break;
  }
  return true;
}
