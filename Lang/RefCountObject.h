#pragma once


// include as public base member of your class, so ISmartPointer works with it

namespace GR
{

  class RefCountObject
  {

    public:

	    RefCountObject();
	    RefCountObject( const RefCountObject& );

	    // Ensures that the reference count is zero
	    virtual ~RefCountObject();

	    // Returns this RefCountObject
	    RefCountObject&   operator=( const RefCountObject& );


	    void			AddReference();
      long      RemoveReference();

	    // Decrements reference count by one and deletes the RefCountObject if the count reaches zero
	    void			Release();



    private:

	    long	    m_References;


  };


}

