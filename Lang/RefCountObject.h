#ifndef _LANG_CRefCountObject_H
#define _LANG_CRefCountObject_H


namespace GR
{


class CRefCountObject
{

  public:

	  /** 
	  * Initializes reference count to zero. 
	  * @param flags CRefCountObject creation flags.
	  */
	  CRefCountObject();

	  /** Initializes reference count to zero. */
	  CRefCountObject( const CRefCountObject& );

	  /** Ensures that the reference count is zero. */
	  virtual ~CRefCountObject();

	  /** Returns this CRefCountObject. */
	  CRefCountObject&			operator=( const CRefCountObject& );

	  /** Increments reference count by one. */
	  void			addReference();
    long      removeReference();

	  /** 
	  * Decrements reference count by one and 
	  * deletes the CRefCountObject if the count reaches zero. 
	  */
	  void			release();

  private:
	  long	    m_refs;
};


}

#endif // _LANG_CRefCountObject_H
