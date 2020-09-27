#include <PJLib/DesignPatterns/AdvancedObserver.h>

#include <crtdbg.h>
// #define assert(a) _ASSERT(a)
#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <fstream>
using namespace std;               //- im Zweifelsfall entfernen

// #ifdef _DEBUG
// namespace 
// {
  // ofstream s_Log( "observerlog.txt" ); 
  // //- führt dies zu Macken?????
// }
// #endif//_DEBUG


// CAdvancedObserver state flags
#define S_NEEDS_UPDATE	0x01	// object needs to be updated
#define S_NEEDS_CHECK	  0x02	// object needs to check its subjects for update
#define S_ALLDIRTY		  0x04	// all subjects are dirty
#define S_CHANGED		    0x08	// true if a change occurred during OnUpdate()
// The following four flags are for cyclic operations
#define S_UPDATING		  0x10	// true during OnUpdate() call
#define S_PROPAGATING	  0x20	// true if last change is currently propagating
#define S_NOTIFYING		  0x40	// true during notification phase
#define S_CHECKING		  0x80	// true during check phase

// Macros for flag manipulation - these work within private scope of CObserver
#define set_flags(mask, value)	(value ? (_flags |= (mask)) : (_flags &= ~(mask)))
	// To set all bits in 'mask' to the same 'value'
#define get_flags(mask)			(_flags & (mask))
	// To get the bits specified in 'mask'. 
	// Use 'get_flags(mask) == mask' to test if all the flags are on
	// Use 'get_flags(mask) > 0' to test if at least one of the flags is on
 


/////////////////////////////////////////////////////////////////////
//-    RemoveSubjectSafely(CAdvancedObserver& subject)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::RemoveSubjectSafely(CAdvancedObserver& subject)
// Removes subject without modifying the subject being removed
{
	// Disconnect subject from this observer
	tlist_type::iterator at = _tlist.find(&subject);
	if(at != _tlist.end())
		_tlist.erase(at);
	
	Invalidate();
}

/////////////////////////////////////////////////////////////////////
//-    RemoveObserverSafely(CAdvancedObserver& observer)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::RemoveObserverSafely(CAdvancedObserver& observer)
// Removes observer without modifying the observer being removed
{
	// Disconnect observer from this subject
	plist_type::iterator at = _plist.find(&observer);
	if(at != _plist.end())
		_plist.erase(at);
}

/////////////////////////////////////////////////////////////////////
//-    SetDirtyFlag(CAdvancedObserver& subject, bool value)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::SetDirtyFlag(CAdvancedObserver& subject, bool value)
{
	tlist_type::iterator at = _tlist.find(&subject);
	if(at != _tlist.end())
	{
		at->second = value;
		if(!value)
			set_flags(S_ALLDIRTY, false);
	}
}

/////////////////////////////////////////////////////////////////////
//-    NotifyCheck(CAdvancedObserver *from)	
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::NotifyCheck(CAdvancedObserver *from)		
// call only when object is invalidated
{
	if(!get_flags(S_CHECKING))
	{
		if(from)
			SetDirtyFlag(*from, true);

		if(!get_flags(S_NEEDS_CHECK))
		{
			set_flags(S_NEEDS_CHECK, true);
			if(!get_flags(S_NOTIFYING))		// save some time
			{
				plist_type::iterator pos = _plist.begin();
				set_flags(S_NOTIFYING, true);
				while(pos != _plist.end())
				{
					(*pos)->NotifyCheck(this);
					++pos;
				}
				set_flags(S_NOTIFYING, false);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
//-    SendNotify()
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::SendNotify( Hint* pHint /* = 0 */ )
{
	// If observers call Notify() any other time than during an update,
	// it is a temporary perturbation and must be either be immediately
	// propagated or immediately erased by the next update.
	if(!get_flags(S_NEEDS_UPDATE | S_UPDATING) && IsObserver())
		set_flags(S_NEEDS_UPDATE, true);

	// Invalidate observers - must be done before propagation
	plist_type::iterator pos;
	set_flags(S_NOTIFYING, true);
	pos = _plist.begin();
	while(pos != _plist.end())
	{
		(*pos)->Invalidate(this);
		++pos;
	}
	set_flags(S_NOTIFYING, false);

	if(get_flags(S_PROPAGATING))
	{
		// update all observers immediately
		pos = _plist.begin();
		while(pos != _plist.end())
		{
			(*pos)->Update(this,pHint,true);	// continue depth-first propagation
			++pos;
		}
		if(!get_flags(S_UPDATING))
		{
			set_flags(S_PROPAGATING, false);	// set back to default - insurance only
			if(IsObserver())	// perturbation - re-notify after propagation
				Notify(pHint);
		}
	}
}

/////////////////////////////////////////////////////////////////////
//-    CAdvancedObserver()
/////////////////////////////////////////////////////////////////////
CAdvancedObserver::CAdvancedObserver()
{
	_flags = 0;
}

/////////////////////////////////////////////////////////////////////
//-    ~CAdvancedObserver()
/////////////////////////////////////////////////////////////////////
CAdvancedObserver::~CAdvancedObserver()
{
	// Remove this subject from all observers
	plist_type::iterator pos = _plist.begin();
	while(pos != _plist.end())
	{
		(*pos)->RemoveSubjectSafely(*this);
		++pos;
	}
	// Remove this observer from all subjects - do this after observers are gone
	RemoveAllSubjects();
}

/////////////////////////////////////////////////////////////////////
//-    CAdvancedObserver(const CAdvancedObserver& other)
/////////////////////////////////////////////////////////////////////
CAdvancedObserver::CAdvancedObserver(const CAdvancedObserver& other)	
// copy constructor - starts from scratch!
{
	_flags = 0;
}

/////////////////////////////////////////////////////////////////////
//-    operator=(const CAdvancedObserver& other)
/////////////////////////////////////////////////////////////////////
CAdvancedObserver& CAdvancedObserver::operator=(const CAdvancedObserver& other) 
{
	// nothing copied
  return *this;
}

/////////////////////////////////////////////////////////////////////
//-    AddSubject(CAdvancedObserver& subject)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::AddSubject(CAdvancedObserver& subject)
// Registers a new subject with this observer.
{
	// Connect observer and subject
	_tlist.insert(std::make_pair(&subject,false));
	subject._plist.insert(this);

	Invalidate();
}

/////////////////////////////////////////////////////////////////////
//-    RemoveSubject(CAdvancedObserver& subject)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::RemoveSubject(CAdvancedObserver& subject)
// Unregisters a subject that was previously registered.
{
	// Disconnect this observer from subject
	subject.RemoveObserverSafely(*this);
	// Disconnect subject from this observer
	RemoveSubjectSafely(subject);
}

/////////////////////////////////////////////////////////////////////
//-    RemoveAllSubjects()
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::RemoveAllSubjects()
// Unregisters all subjects.
{
	tlist_type::iterator tpos = _tlist.begin();
	while(tpos != _tlist.end())
	{
		tpos->first->RemoveObserverSafely(*this);
		++tpos;
	}
	_tlist.clear();
	Invalidate();
}

/////////////////////////////////////////////////////////////////////
//-    AddSubjectsOf(CAdvancedObserver& observer)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::AddSubjectsOf(CAdvancedObserver& observer)
// Registers all the subjects of another observer.
{
	tlist_type::iterator tpos = observer._tlist.begin();
	while(tpos != observer._tlist.end())
	{
		AddSubject(*(tpos->first));
		++tpos;
	}
}

/////////////////////////////////////////////////////////////////////
//-    OnUpdate()
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::OnUpdate( CAdvancedObserver *from, Hint* pHint /* = 0 */ )	
// Virtual function to do the updating. Call Notify() if the value changes.
{
// #ifdef _DEBUG
  // s_Log << "ONUPDATE  " << typeid( *this ).name() << "/CAdvancedObserver::OnUpdate( 0x" << hex << from << ", 0x" << pHint << " )\n";  
  // if ( from ) s_Log << "  from  is a " << typeid( *from ).name() << "\n";
  // if ( pHint ) s_Log << "  pHint is a " << typeid( *pHint ).name() << "\n";
  // s_Log.flush();
// #endif//_DEBUG
}	

/////////////////////////////////////////////////////////////////////
//-    Update(bool propagate/*  = false */)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::Update( CAdvancedObserver *from, Hint* pHint, bool propagate/*  = false */)
// Updates the objects and, if its value has changed, invlidates all its observers.
{
	if(get_flags(S_PROPAGATING))	// freeze during propagation phase
		return;
	if(get_flags(S_UPDATING | S_CHECKING))	// cyclic check or update - wait until next cycle
		return;

	while(get_flags(S_NEEDS_CHECK | S_NEEDS_UPDATE))
	{		
		if(get_flags(S_NEEDS_CHECK) && !get_flags(S_NEEDS_UPDATE))
		{
			set_flags(S_NEEDS_CHECK, false);	// validate here for cyclic safety

			// check the subjects
			set_flags(S_CHECKING, true);
			tlist_type::iterator tpos = _tlist.begin();
			while(tpos != _tlist.end())
			{
				if(tpos->second)	// subject is dirty - may need update
				{
					tpos->first->Update(from,pHint);
					if(get_flags(S_NEEDS_UPDATE))	// stop searching as soon as we know we need to udpate
						break;
				}
				++tpos;
			}
			set_flags(S_CHECKING, false);
		}

		if(get_flags(S_NEEDS_UPDATE))
		{
			// don't validate here - Notify() will handle
			set_flags(S_UPDATING, true);
			set_flags(S_PROPAGATING, propagate);
			set_flags(S_CHANGED, false);
			OnUpdate( from, pHint );		// the rest of the subjects will be checked here automatically
			Validate();		// clear dirty flags after update
			if(get_flags(S_CHANGED))
				SendNotify(pHint);		// this could invalidate us again (cyclic)
			set_flags(S_PROPAGATING | S_UPDATING, false);
		}
	}
}

/////////////////////////////////////////////////////////////////////
//-    IsDirty(CAdvancedObserver& subject) const
/////////////////////////////////////////////////////////////////////
bool CAdvancedObserver::IsDirty(CAdvancedObserver& subject) const
{
	tlist_type::const_iterator at = _tlist.find(&subject);
	if(at != _tlist.end())
	{
		return (get_flags(S_ALLDIRTY) ? true : at->second);
	}
	else
		return false;
}

/////////////////////////////////////////////////////////////////////
//-    Notify(bool propagate)		
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::Notify( Hint* pHint, bool propagate)		
// Notify with propagation control
{
	bool oldprop = get_flags(S_PROPAGATING) > 0;
	set_flags(S_PROPAGATING, propagate);
	Notify( pHint );
	set_flags(S_PROPAGATING, oldprop);
}

/////////////////////////////////////////////////////////////////////
//-    Notify()
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::Notify( Hint* pHint = 0 )
// Notifies all observers that this object has changed. Call only when
// the object's value actually changes.
{
	if(get_flags(S_UPDATING))
		set_flags(S_CHANGED, true);		// delayed notification during update
	else
		SendNotify(pHint);
}

/////////////////////////////////////////////////////////////////////
//-    Invalidate(CAdvancedObserver *from /* = NULL */)
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::Invalidate(CAdvancedObserver *from /* = NULL */)
// Invalidates the object and notifies all its observers.
{
	if(!get_flags(S_UPDATING) || get_flags(S_NOTIFYING))	// can't invalidate during update unless a cycle occurred
	{
		if(from)
			SetDirtyFlag(*from, true);
		else
			set_flags(S_ALLDIRTY, true);

		if(!get_flags(S_NEEDS_UPDATE))
		{
			set_flags(S_NEEDS_UPDATE, true);
			NotifyCheck(NULL);
		}
	}
}

/////////////////////////////////////////////////////////////////////
//-    Validate()
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::Validate()		
// clears all dirty flags
{
	tlist_type::iterator tpos = _tlist.begin();
	while(tpos != _tlist.end())
	{
		tpos->second = false;
		++tpos;
	}
	set_flags(S_ALLDIRTY | S_NEEDS_UPDATE | S_NEEDS_CHECK, false);
}

/////////////////////////////////////////////////////////////////////
//-    IsObserver() const
/////////////////////////////////////////////////////////////////////
bool CAdvancedObserver::IsObserver() const
{ 
  return !_tlist.empty();
}

/////////////////////////////////////////////////////////////////////
//-    IsSubject() const 
/////////////////////////////////////////////////////////////////////
bool CAdvancedObserver::IsSubject() const 
{ 
  return !_plist.empty();
}

/////////////////////////////////////////////////////////////////////
//-    Propagate() 
/////////////////////////////////////////////////////////////////////
void CAdvancedObserver::Propagate( Hint* pHint /* = 0 */ ) 
{ 
// #ifdef _DEBUG
  // s_Log << "\nPROPAGATE " << typeid( *this ).name() << "/CAdvancedObserver::Propagate( 0x" << hex << pHint << " )\n";  
// #endif//_DEBUG

  Notify(pHint,true);		// propagate current value
}


