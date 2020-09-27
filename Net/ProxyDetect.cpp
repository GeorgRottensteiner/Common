#include <winsock2.h>

#include "ProxyDetect.h"

#include <windows.h>
#include <ws2tcpip.h>
#include <wininet.h>
#include <urlmon.h>


#pragma comment( lib, "wininet.lib" )
#pragma comment( lib, "urlmon.lib" )



namespace GR
{


  namespace
  {

    DWORD __stdcall ResolveHostName( LPSTR lpszHostName, LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize )
    {
      DWORD             dwIPAddressSize;
      struct addrinfo   Hints;
      LPADDRINFO        lpAddrInfo;
      LPADDRINFO        IPv4Only;
      DWORD             error;

      // Figure out first whether to resolve a name or an address literal.
      // If getaddrinfo( ) with the AI_NUMERICHOST flag succeeds, then
      // lpszHostName points to a string representation of an IPv4 or IPv6
      // address. Otherwise, getaddrinfo( ) should return EAI_NONAME.
      ZeroMemory( &Hints, sizeof( struct addrinfo ) );

      Hints.ai_flags    = AI_NUMERICHOST;    // Only check for address literals.
      Hints.ai_family   = PF_UNSPEC;        // Accept any protocol family.
      Hints.ai_socktype = SOCK_STREAM;    // Constrain results to stream socket.
      Hints.ai_protocol = IPPROTO_TCP;    // Constrain results to TCP.

      error = getaddrinfo( lpszHostName, NULL, &Hints, &lpAddrInfo );
      if ( error != EAI_NONAME )
      {
        if ( error != 0 )
        {
          error = ( error == EAI_MEMORY ) ? ERROR_NOT_ENOUGH_MEMORY : ERROR_INTERNET_NAME_NOT_RESOLVED;
          goto quit;
        }
        freeaddrinfo( lpAddrInfo );

        // An IP address (either v4 or v6) was passed in, so if there is
        // room in the lpszIPAddress buffer, copy it back out and return.
        dwIPAddressSize = lstrlenA( lpszHostName );

        if ( ( *lpdwIPAddressSize < dwIPAddressSize )
        ||   ( lpszIPAddress == NULL ) )
        {
          *lpdwIPAddressSize = dwIPAddressSize + 1;
          error = ERROR_INSUFFICIENT_BUFFER;
          goto quit;
        }
        strcpy_s( lpszIPAddress, *lpdwIPAddressSize, lpszHostName );
        goto quit;
      }

      // Call getaddrinfo( ) again, this time with no flag set.
      Hints.ai_flags = 0;
      error = getaddrinfo( lpszHostName, NULL, &Hints, &lpAddrInfo );
      if ( error != 0 )
      {
        error = ( error == EAI_MEMORY ) ? ERROR_NOT_ENOUGH_MEMORY : ERROR_INTERNET_NAME_NOT_RESOLVED;
        goto quit;
      }

      // Convert the IP address in addrinfo into a string.
      // (the following code only handles IPv4 addresses)
      IPv4Only = lpAddrInfo;
      while ( IPv4Only->ai_family != AF_INET )
      {
        IPv4Only = IPv4Only->ai_next;
        if ( IPv4Only == NULL )
        {
          error = ERROR_INTERNET_NAME_NOT_RESOLVED;
          goto quit;
        }
      }
      error = getnameinfo( IPv4Only->ai_addr,
                           IPv4Only->ai_addrlen,
                           lpszIPAddress,
                           *lpdwIPAddressSize,
                           NULL,
                           0,
                           NI_NUMERICHOST );
      if ( error != 0 )
      {
        error = ERROR_INTERNET_NAME_NOT_RESOLVED;
      }
      quit:
      return error;
    }



    BOOL __stdcall IsResolvable( LPSTR lpszHost )
    {
      char    szDummy[255];
      DWORD   dwDummySize = sizeof( szDummy ) - 1;

      if ( ResolveHostName( lpszHost, szDummy, &dwDummySize ) )
      {
        return FALSE;
      }
      return TRUE;
    }



    DWORD __stdcall GetIPAddress( LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize )
    {
      char szHostBuffer[255];

      if ( gethostname( szHostBuffer, sizeof( szHostBuffer ) - 1 ) != ERROR_SUCCESS )
      {
        return ERROR_INTERNET_INTERNAL_ERROR;
      }
      return ResolveHostName( szHostBuffer, lpszIPAddress, lpdwIPAddressSize );
    }



    BOOL __stdcall IsInNet( LPSTR lpszIPAddress, LPSTR lpszDest, LPSTR lpszMask )
    {
      DWORD dwDest    = inet_addr( lpszDest );
      DWORD dwIpAddr  = inet_addr( lpszIPAddress );
      DWORD dwMask    = inet_addr( lpszMask );

      if ( ( dwDest == INADDR_NONE )
      ||   ( dwIpAddr == INADDR_NONE )
      ||   ( ( dwIpAddr & dwMask ) != dwDest ) )
      {
        return FALSE;
      }
      return TRUE;
    }
  }



  GR::String ProxyDetect::DetectFromURL( const GR::String& URL )
  {
    GR::String     baseURL;

    DWORD           ProxyEnable = 0;


    size_t    slashPos = URL.find( "//" );
    if ( slashPos != GR::String::npos )
    {
      baseURL = URL.substr( slashPos + 2, URL.find( "/", slashPos + 2 ) - slashPos - 2 );
    }

    char    WPADLocation[1024] = "";
    char    TempPath[MAX_PATH];
    char    TempFile[MAX_PATH];
    char    proxyBuffer[1024];
    char*   proxy = proxyBuffer;
    DWORD   dwProxyHostNameLength = 1024;
    DWORD   returnVal;
    HMODULE hModJS;

    // Declare and populate an AutoProxyHelperVtbl structure, and then
    // place a pointer to it in a containing AutoProxyHelperFunctions
    // structure, which will be passed to InternetInitializeAutoProxyDll
    AutoProxyHelperVtbl OurVtbl = {
                                    IsResolvable,
                                    GetIPAddress,
                                    ResolveHostName,
                                    IsInNet,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL
                                  };

    // Declare function pointers for the three autoproxy functions
    pfnInternetInitializeAutoProxyDll   pInternetInitializeAutoProxyDll;
    pfnInternetDeInitializeAutoProxyDll pInternetDeInitializeAutoProxyDll;
    pfnInternetGetProxyInfo             pInternetGetProxyInfo;

    AutoProxyHelperFunctions            HelperFunctions;


    HelperFunctions.lpVtbl = &OurVtbl;

    ZeroMemory( proxy, 1024 );

    if ( !( hModJS = LoadLibraryA( "jsproxy.dll" ) ) )
    {
      return "";
    }

    if ( ( !( pInternetInitializeAutoProxyDll = (pfnInternetInitializeAutoProxyDll)GetProcAddress( hModJS, "InternetInitializeAutoProxyDll" ) ) )
    ||   ( !( pInternetDeInitializeAutoProxyDll = (pfnInternetDeInitializeAutoProxyDll)GetProcAddress( hModJS, "InternetDeInitializeAutoProxyDll" ) ) )
    ||   ( !( pInternetGetProxyInfo = (pfnInternetGetProxyInfo)GetProcAddress( hModJS, "InternetGetProxyInfo" ) ) ) )
    {
      FreeLibrary( hModJS );
      return "";
    }

    if ( !DetectAutoProxyUrl( WPADLocation, sizeof( WPADLocation ), PROXY_AUTO_DETECT_TYPE_DHCP | PROXY_AUTO_DETECT_TYPE_DNS_A ) )
    {
      FreeLibrary( hModJS );
      return "";
    }

    //printf("\n  WPAD Location is: %s\n", WPADLocation);

    GetTempPathA( sizeof( TempPath ) / sizeof( TempPath[0] ), TempPath );
    GetTempFileNameA( TempPath, NULL, 0, TempFile );
    URLDownloadToFileA( NULL, WPADLocation, TempFile, 0, NULL );

    if ( !( returnVal = pInternetInitializeAutoProxyDll( 0, TempFile, NULL, &HelperFunctions, NULL ) ) )
    {
      FreeLibrary( hModJS );
      return "";
    }

    //printf("\n  InternetInitializeAutoProxyDll returned: %d\n", returnVal);

    // Delete the temporary file
    // (or, to examine the auto-config script, comment out the
    // file delete and substitute the following printf call)
    // printf( "\n  The auto-config script temporary file is:\n    %s\n", TempFile);
    DeleteFileA( TempFile );

    if ( !pInternetGetProxyInfo( (LPSTR)URL.c_str(), URL.length(), (LPSTR)baseURL.c_str(), baseURL.length(), &proxy, &dwProxyHostNameLength ) )
    {
      pInternetDeInitializeAutoProxyDll( NULL, 0 );
      FreeLibrary( hModJS );
      return "";
    }

    //printf("\n  Proxy is: %s\n", proxy);

    pInternetDeInitializeAutoProxyDll( NULL, 0 );
    return proxy;
  }


} // namespace GR






