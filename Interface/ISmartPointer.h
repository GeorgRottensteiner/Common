#ifndef ISMARTPOINTER_H
#define ISMARTPOINTER_H


/**
 * Thread-safe smart pointer to an object of class T.
 * T must implement at least addReference() and release() methods.
 * release() must destroy the object if no more references are left.
 * Initial reference count of an object must be 0.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class ISmartPointer
{
  public:

	  // Null pointer
	  ISmartPointer()																			
    {
      m_object = 0;
    }

	  // Releases reference to the object
	  ~ISmartPointer()																			
    {
      if ( m_object )
      {
        m_object->release();
      }
    }

	  // Increments object reference count and stores the reference
	  ISmartPointer( const ISmartPointer<T>& other )														
    {
      m_object = 0;
      T* obj = other.ptr(); 
      if ( obj ) 
      {
        obj->addReference(); 
        m_object = obj;
      }
    }

	  // Increments object reference count and stores the reference
	  ISmartPointer( T* other )																	
    {
      m_object = 0;
      if ( other )
      {
        other->addReference(); 
        m_object = other;
      }
    }

	  // Releases old reference if any, increments other object reference 
	  // count and stores the new reference. 
	  ISmartPointer<T>& operator=( const ISmartPointer<T>& other )										
    {
      T* obj = other.ptr(); 
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
	  bool	operator==( const T* other ) const										
    {
      return m_object == other;
    }

	  // Returns true if the references point to the same unique object
	  bool	operator==( const ISmartPointer<T>& other ) const									
    {
      return m_object == other.m_object;
    }
  	
	  // Returns true if the references point to different unique objects
	  bool	operator!=( const T* other ) const										
    {
      return m_object != other;
    }

	  // Returns true if the references point to different unique objects
	  bool	operator!=( const ISmartPointer<T>& other ) const									
    {
      return m_object != other.m_object;
    }

	  // Access to the object
	  T&		operator*() const														
    {
      return *m_object;
    }

	  // Access to the object
	  T*		operator->() const														
    {
      return m_object;
    }

	  // Access to the object
	  operator T*() const																
    {
      return m_object;
    }

	  // Access to the object
	  T*		ptr() const																
    {
      return m_object;
    }

    bool IsObjectValid() const
    {
      return !!m_object;
    }


  private:

	  T* m_object;

};


#endif // ISMARTPOINTER_H
