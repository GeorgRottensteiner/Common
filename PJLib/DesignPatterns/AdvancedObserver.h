//*************************************************************************************
//	Copyright 2000 Raymond A. Virzi, Jr.
//	You may freely use and distribute this source code provide the above copyright notice
//	and this statement remains intact within the source module or any modified versions of it.

///////////////////////////////////////////////////////////////
// Pattern Class: CObserver
// Derive from this class objects that depend on other objects via an 'update' function.
//
// CHANGE HISTORY:
// 05/03/00	- Initial Revision.
// 05/04/00 - Uploaded to Codeguru.
///////////////////////////////////////////////////////////////
// #include <warnings.h>

#ifndef __ADVANCEDOBSERVER_H
#define __ADVANCEDOBSERVER_H


#pragma warning (disable:4786)
#include <algorithm>
#include <utility>
#include <map>
#include <set>

 
class CAdvancedObserver
{
public:
  class Hint
  {
  public:
    virtual ~Hint() {}
  };


  typedef std::set<CAdvancedObserver*> plist_type;
	typedef std::map<CAdvancedObserver*, bool> tlist_type;	// key = pointer to subject, val = dirty flag

  //- Gefährlich?
  const plist_type& Observers() { return _plist; }

private:

	plist_type _plist;	// observers
	tlist_type _tlist;	// subjects
	unsigned char _flags;	// state flags

protected:

  void RemoveSubjectSafely(CAdvancedObserver& subject);
	// Removes subject without modifying the subject being removed
	void RemoveObserverSafely(CAdvancedObserver& observer);
	// Removes observer without modifying the observer being removed
	void SetDirtyFlag(CAdvancedObserver& subject, bool value);
	void NotifyCheck(CAdvancedObserver *from);		// call only when object is invalidated

  void SendNotify( Hint* pHint = 0 );
		// If observers call Notify() any other time than during an update,
		// it is a temporary perturbation and must be either be immediately
		// propagated or immediately erased by the next update.
	void Update( CAdvancedObserver *from, Hint* pHint, bool propagate = false);
	// Updates the objects and, if its value has changed, invlidates all its observers.

public: //- au ha
	CAdvancedObserver();
	virtual ~CAdvancedObserver();
	CAdvancedObserver(const CAdvancedObserver& other);	// copy constructor - starts from scratch!
	CAdvancedObserver& operator=(const CAdvancedObserver& other);// nothing copied
	void AddSubject(CAdvancedObserver& subject);
	// Registers a new subject with this observer.
	void RemoveSubject(CAdvancedObserver& subject);
	// Unregisters a subject that was previously registered.
	void RemoveAllSubjects();
	// Unregisters all subjects.
	void AddSubjectsOf(CAdvancedObserver& observer);
	// Registers all the subjects of another observer.


  bool IsDirty(CAdvancedObserver& subject) const;
	void Notify( Hint* pHint, bool propagate);		// Notify with propagation control
	void Notify( Hint* pHint );
	// Notifies all observers that this object has changed. Call only when
	// the object's value actually changes.
  virtual void Invalidate(CAdvancedObserver *from = NULL);
	// Invalidates the object and notifies all its observers.
	void Validate();		// clears all dirty flags
  
  
  
  
  bool IsObserver() const;
	bool IsSubject() const;

// 	virtual void Propagate(); 
	virtual void Propagate( Hint* pHint = 0 ); 

// 	virtual void OnUpdate();	// Virtual function to do the updating. Call Notify() if the value changes.
  virtual void OnUpdate( CAdvancedObserver *from, CAdvancedObserver::Hint* pHint = 0 );	// Virtual function to do the updating. Call Notify() if the value changes.
};


#endif//__ADVANCEDOBSERVER_H
