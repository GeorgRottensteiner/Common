#ifndef GR_OS_H
#define GR_OS_H

// Operating systems
#undef OS_WINDOWS
#define OS_WINDOWS                1
#define OS_TANDEM                 2
#define OS_ANDROID                3
#define OS_WEB                    4
#define OS_LINUX                  5

#undef OPERATING_SUB_SYSTEM
#define OS_SUB_DESKTOP            0
#define OS_SUB_UNIVERSAL_APP      1
#define OS_SUB_SDL                2
#define OS_SUB_GUARDIAN           3
#define OS_SUB_OSS                4
#define OS_SUB_ANDROID            5
#define OS_SUB_WINDOWS_PHONE      6

// Endianness
#undef OS_ENDIAN
#define OS_ENDIAN_BIG             1       // the weird other one
#define OS_ENDIAN_LITTLE          2       // x86/x64 (Windows)

// Environment (16/32/64)
#undef OS_ENVIRONMENT
#define OS_ENVIRONMENT_16         16
#define OS_ENVIRONMENT_32         32
#define OS_ENVIRONMENT_64         64



// universal app?
#ifdef GR_ENVIRONMENT_SDL
//#pragma message( "Compiling SDL" )
#define OPERATING_SUB_SYSTEM  OS_SUB_SDL
#elif GR_FORCE_WINDOWS_PHONE_81
//#elif WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
//#pragma message( "Compiling Windows Phone 8.1" )
#define OPERATING_SUB_SYSTEM  OS_SUB_WINDOWS_PHONE
#elif ( defined WINAPI_FAMILY ) && ( ( WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP ) || ( WINAPI_FAMILY == WINAPI_FAMILY_APP ) )
//#pragma message( "Compiling UWP" )
#define OPERATING_SUB_SYSTEM  OS_SUB_UNIVERSAL_APP
#elif ( defined _WIN64 ) || ( defined _WIN32 )
//#pragma message( "Compiling Desktop" )
#define OPERATING_SUB_SYSTEM  OS_SUB_DESKTOP
#endif




#if defined EMSCRIPTEN
#define OPERATING_SYSTEM  OS_WEB
#define OS_ENDIAN         OS_ENDIAN_LITTLE
// TODO - is only available as 64bit!
#define OS_ENVIRONMENT    OS_ENVIRONMENT_64
#define OPERATING_SUB_SYSTEM    OS_SUB_SDL

#elif defined _WIN64
// check for _WIN64 first, since _WIN32 is also set in 64 bit environments
#define OPERATING_SYSTEM  OS_WINDOWS
#define OS_ENDIAN         OS_ENDIAN_LITTLE
#define OS_ENVIRONMENT    OS_ENVIRONMENT_64

#elif ( defined _WIN32 ) || ( defined WIN32 )

#define OPERATING_SYSTEM  OS_WINDOWS
#define OS_ENDIAN         OS_ENDIAN_LITTLE
#define OS_ENVIRONMENT    OS_ENVIRONMENT_32

#elif defined ( _XOPEN_SOURCE ) && ( defined __TANDEM )

#define OPERATING_SYSTEM  OS_TANDEM
#define OS_ENDIAN         OS_ENDIAN_BIG
#define OS_ENVIRONMENT    OS_ENVIRONMENT_32

#define TANDEM_OSS
#define OPERATING_SUB_SYSTEM    OS_SUB_OSS

#elif defined __TANDEM

#define OPERATING_SYSTEM  OS_TANDEM
#define OS_ENDIAN         OS_ENDIAN_BIG
#define OS_ENVIRONMENT    OS_ENVIRONMENT_32

#define TANDEM_GUARDIAN
#define OPERATING_SUB_SYSTEM    OS_SUB_GUARDIAN

#elif defined __ANDROID__

#define OPERATING_SYSTEM  OS_ANDROID
#define OS_ENDIAN         OS_ENDIAN_LITTLE
// TODO - 64bit is also possible!
#define OS_ENVIRONMENT    OS_ENVIRONMENT_32
#define OPERATING_SUB_SYSTEM    OS_SUB_ANDROID

#elif defined(__LP64__) || defined(_LP64)

#define OPERATING_SYSTEM  OS_LINUX
#define OS_ENDIAN         OS_ENDIAN_BIG
#define OS_ENVIRONMENT    OS_ENVIRONMENT_64
#define OPERATING_SUB_SYSTEM    OS_SUB_DESKTOP

#else

#define OPERATING_SYSTEM  OS_LINUX
#define OS_ENDIAN         OS_ENDIAN_BIG
#define OS_ENVIRONMENT    OS_ENVIRONMENT_32
#define OPERATING_SUB_SYSTEM    OS_SUB_DESKTOP

#endif




#endif // GR_OS_H
