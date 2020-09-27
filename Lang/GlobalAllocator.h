#ifndef GR_GLOBAL_ALLOCATOR_H
#define GR_GLOBAL_ALLOCATOR_H

#pragma once

#include <OS/OS.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <windows.h>
#endif

// Beispielaufruf
// std::list<GR::String, GR::STL::global_allocator<GR::String> > myList;

namespace GR
{

  namespace STL
  {

  template <class T> class global_allocator
  {

  public:

    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef T*        pointer;
    typedef const T*  const_pointer;
    typedef T&        reference;
    typedef const T&  const_reference;
    typedef T         value_type;

    global_allocator() {}
    global_allocator(const global_allocator&) {}



    pointer   allocate( size_type n, const void* = 0 )
    {
  #if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
      T* t = (T*)GlobalAlloc( GMEM_FIXED, n * sizeof( T ) );
  #else
      T* t = reinterpret_cast<pointer>( ::operator new( n * sizeof( T ) ) );
  #endif
      return t;
    }

    void deallocate( void* p, size_type n )
    {
      if ( p )
      {
  #if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
        GlobalFree( p );
  #else
        delete[] p;
  #endif
      }
    }

    bool              operator==( const global_allocator& rhsAlloc ) const
    {
      return ( this == &rhsAlloc );
    }

    bool              operator!=( const global_allocator& rhsAlloc ) const
    {
      return ( this != &rhsAlloc );
    }

    pointer           address(reference x) const { return &x; }
    const_pointer     address(const_reference x) const { return &x; }
    global_allocator<T>&  operator=(const global_allocator&) { return *this; }
    void              construct(pointer p, const T& val)
                      { new ((T*) p) T(val); }
    void              destroy(pointer p) { p->~T(); }

    size_type         max_size() const { return size_t(-1); }

    template <class U>
    struct rebind { typedef global_allocator<U> other; };

    template <class U>
    global_allocator(const global_allocator<U>&) {}

    template <class U>
    global_allocator& operator=(const global_allocator<U>&) { return *this; }
  };

  }

}



#endif // GR_GLOBAL_ALLOCATOR_H