#pragma once



// Thread-safe smart pointer to an object of class T.
// T must implement at least addReference() and release() methods.
// release() must destroy the object if no more references are left.
// Initial reference count of an object must be 0.
// @author Jani Kajala (jani.kajala@helsinki.fi)
// Works best in conjunction with GR::RefCountObject

template <class T> class ISmartPointer
{
  public:

	  ISmartPointer()																			
    {
      m_pObject = 0;
    }



	  ~ISmartPointer()																			
    {
      if ( m_pObject != NULL )
      {
        m_pObject->Release();
      }
    }



	  // Increments object reference count and stores the reference
	  ISmartPointer( const ISmartPointer<T>& other )														
    {
      m_pObject = NULL;
      T* obj = other.ptr(); 
      if ( obj ) 
      {
        obj->AddReference(); 
        m_pObject = obj;
      }
    }



	  // Increments object reference count and stores the reference
	  ISmartPointer( T* other )																	
    {
      m_pObject = NULL;
      if ( other )
      {
        other->AddReference(); 
        m_pObject = other;
      }
    }



	  // Releases old reference if any, increments other object reference 
	  // count and stores the new reference. 
	  ISmartPointer<T>& operator=( const ISmartPointer<T>& other )										
    {
      T* obj = other.ptr(); 
      if ( obj )
      {
        obj->AddReference();
      }
      if ( m_pObject != NULL )
      {
        m_pObject->Release();
      }
      m_pObject = obj;
      return *this;
    }



	  // Returns true if the references point to the same unique object
	  bool	operator==( const T* other ) const										
    {
      return m_pObject == other;
    }



	  // Returns true if the references point to the same unique object
	  bool	operator==( const ISmartPointer<T>& other ) const									
    {
      return m_pObject == other.m_pObject;
    }
  	


	  // Returns true if the references point to different unique objects
	  bool	operator!=( const T* other ) const										
    {
      return m_pObject != other;
    }



	  bool	operator!=( const ISmartPointer<T>& other ) const									
    {
      return m_pObject != other.m_pObject;
    }



	  T& operator*() const														
    {
      return *m_pObject;
    }

	  // Access to the object
	  T* operator->() const														
    {
      return m_pObject;
    }



	  operator T*() const																
    {
      return m_pObject;
    }



	  T* ptr() const																
    {
      return m_pObject;
    }



    bool IsObjectValid() const
    {
      return !!m_pObject;
    }



  private:

	  T*    m_pObject;

};


