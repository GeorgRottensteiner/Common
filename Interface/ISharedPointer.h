#ifndef GR_ISHARED_POINTER_H
#define GR_ISHARED_POINTER_H


/**
 * Thread-safe smart pointer to an object of class T.
 * T must implement at least addReference() and release() methods.
 * release() must destroy the object if no more references are left.
 * Initial reference count of an object must be 0.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */

#include <lang/RefCountObject.h>



template <class T> class ISharedPointer
{

  private:

    typedef T container_type;

    class CTheObject : public GR::CRefCountObject
    {
      public:

        container_type*          m_SharedObject;

        CTheObject() :
          m_SharedObject( 0 )
        {
        }

        CTheObject( T* pOther ) :
          m_SharedObject( pOther )
        {
          addReference();
        }

    };

    typedef CTheObject  TSharedCointer;

  public:

	  // Null pointer
	  ISharedPointer()																			
    {
      m_object = 0;
    }

	  // Releases reference to the object
	  ~ISharedPointer()																			
    {
      if ( m_object )
      {
        if ( m_RefCounter.removeReference() == 0 )
        {
          delete m_object;
          m_object = NULL;
        }
      }
    }

	  // Increments object reference count and stores the reference
	  ISharedPointer( const ISharedPointer<T>& other )														
    {
      m_object = 0;
      TSharedCointer* obj = other.ptr(); 
      if ( obj ) 
      {
        obj->addReference(); 
        m_object = obj;
      }
    }

	  // Increments object reference count and stores the reference
	  ISharedPointer( T* other )																	
    {
      m_object = 0;
      if ( other )
      {
        m_object = new CTheObject( other );
      }
    }

	  // Releases old reference if any, increments other object reference 
	  // count and stores the new reference. 
	  ISharedPointer<T>& operator=( const ISharedPointer<T>& other )										
    {
      TSharedCointer* obj = other.ptr(); 
      if ( obj )
      {
        obj->addReference();
      }
      if ( m_object ) 
      {
        m_object->release(); 
      }
      m_object = obj; 
      return *this;
    }

	  // Returns true if the references point to the same unique object
	  bool	operator==( const TSharedCointer* other ) const										
    {
      return m_object == other;
    }

	  // Returns true if the references point to the same unique object
	  bool	operator==( const ISharedPointer<T>& other ) const									
    {
      return m_object == other.m_object;
    }
  	
	  // Returns true if the references point to different unique objects
	  bool	operator!=( const TSharedCointer* other ) const										
    {
      return m_object != other;
    }

	  // Returns true if the references point to different unique objects
	  bool	operator!=( const ISharedPointer<T>& other ) const									
    {
      return m_object != other.m_object;
    }

	  // Access to the object
	  T&		operator*() const														
    {
      return *m_object.m_SharedObject;
    }

	  // Access to the object
	  T*		operator->() const														
    {
      return m_object->m_SharedObject;
    }

	  // Access to the object
	  operator T*() const																
    {
      return m_object->m_SharedObject;
    }

	  // Access to the object
	  TSharedCointer*		ptr() const																
    {
      return m_object;
    }

    bool IsObjectValid() const
    {
      return !!m_object;
    }


  private:

	  CTheObject*             m_object;

    GR::CRefCountObject     m_RefCounter;

};


#endif // GR_ISHARED_POINTER_H
