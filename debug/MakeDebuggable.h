#ifndef MAKE_DEBUGGABLE_H
#define MAKE_DEBUGGABLE_H

#if defined _MSC_VER
  #define MAKE_DEBUGGABLE __pragma(optimize("", off))
#elif defined __clang__
  #define MAKE_DEBUGGABLE _Pragma ("clang optimize off")
#else
  #define MAKE_DEBUGGABLE
#endif


#endif // MAKE_DEBUGGABLE_H
