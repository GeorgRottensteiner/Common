#ifndef GR_TYPES_H
#define GR_TYPES_H

//#include <string>
#include <String/GRstring.h>

#include <Math\vector2.h>
#include <Math\vector3.h>
#include <Math\tRect.h>
#include <Math\3dRect.h>

#include <OS/OS.h>


#ifndef NULL
#define NULL 0L
#endif 

namespace GR
{

  typedef unsigned char                                   u8;
  typedef unsigned short                                  u16;
  typedef unsigned int                                    u32;
  #if OPERATING_SYSTEM == OS_WINDOWS
  typedef unsigned __int64                                u64;
  #else
  typedef unsigned long long                              u64;
  #endif

  typedef signed char                                     i8;
  typedef signed short                                    i16;
  typedef signed int                                      i32;
  #if OPERATING_SYSTEM == OS_WINDOWS
  typedef signed __int64                                  i64;
  #else
  typedef signed long long                                i64;
  #endif
  typedef float                                           f32;
  typedef double                                          f64;

  #if OS_ENVIRONMENT == OS_ENVIRONMENT_64
  #if _MSC_VER > 1200
  typedef unsigned __int64                                up;       // Größe eines Pointers
  typedef signed __int64                                  ip;       // Größe eines Pointers
  #else
  //typedef unsigned int                                    up;       // Größe eines Pointers
  //typedef int                                             ip;       // Größe eines Pointers
  typedef unsigned long long                              up;       // Größe eines Pointers
  typedef long long                                       ip;       // Größe eines Pointers
  #endif
  #else
  #if _MSC_VER > 1200
  typedef __w64 unsigned int                              up;       // Größe eines Pointers
  typedef __w64 int                                       ip;       // Größe eines Pointers
  #else
  typedef unsigned int                                    up;       // Größe eines Pointers
  typedef int                                             ip;       // Größe eines Pointers
  #endif
  #endif

  #ifdef UNICODE
  typedef std::wstring                                    tString;
  typedef wchar_t                                         tChar;
  #else
  typedef GR::string                                      tString;
  typedef char                                            tChar;
  #endif
  typedef char                                            Char;
  typedef wchar_t                                         WChar;

  typedef std::wstring                                    WString;
  typedef GR::string                                      String;

  typedef math::vector2<i32>                              tPoint;
  typedef math::vector2<f32>                              tFPoint;
  typedef math::vector3t<i32>                             t3dPoint;
  typedef math::vector3t<f32>                             tVector;
  typedef math::tRect<i32>                                tRect;
  typedef math::tRect<f32>                                tFRect;
  typedef math::t3dRect<i32>                              tBounds;
  typedef math::t3dRect<f32>                              tFBounds;


}    // namespace GR


#endif // ABSTRACTEDIT_H



