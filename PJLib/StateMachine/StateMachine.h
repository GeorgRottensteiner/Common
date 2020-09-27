#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <ios>
#include <iostream>
#include <memory>


namespace PJ
{
  template< class FSM, typename MSG > class State;

  /////////////////////////////////////////////////////////////////////
  //-    StateMachine
  /////////////////////////////////////////////////////////////////////
  template< class FSM, typename MSG >
  class StateMachine // Finite State Machine
  {
  public:
    StateMachine();
    StateMachine( const StateMachine& rhs );
    virtual ~StateMachine();
    virtual StateMachine& operator=( const StateMachine& rhs ); 
    virtual StateMachine* clone(); // nötig?

    virtual void ReceiveMessage( const MSG& message );
    virtual void Execute( const float deltaTime = 0.0f );

    //- Attention: this will destroy the current state!
    virtual void SetNewState( State< FSM, MSG >& newState );
  protected:
    std::auto_ptr< State< FSM, MSG > > m_State;
  };


  /////////////////////////////////////////////////////////////////////
  //-    State
  /////////////////////////////////////////////////////////////////////
  template< class FSM, typename MSG >
  class State
  {
  public:
    State( FSM& sm ) : m_StateMachine( sm ) 
    { 
      std::cout << "State " << std::hex << this << ": ich bin neu\n"; 
    }
    
    virtual ~State() 
    { 
      std::cout << "State " << std::hex << this << ": ich sterbe\n"; 
    }
    
    virtual State* clone() 
    { 
      return new State( *this ); 
    }
    
    virtual State* clone( FSM& sm ) 
    { 
      return new State( sm );  
    }

    virtual void ReceiveMessage( const MSG& message ) {} //- kein verhalten!!!!
    virtual void Execute( const float deltaTime = 0.0f ) {} ; // = 0
  
  protected:
    State( const State& rhs )
      : m_StateMachine( rhs.m_StateMachine )
    {
    }  
    
    FSM& Machine() { return m_StateMachine; }

  private:
    State();
    State& operator=( const State& rhs ); //ni 
    FSM& m_StateMachine;
  };


  /////////////////////////////////////////////////////////////////////
  //-    StateMachine - Methoden
  /////////////////////////////////////////////////////////////////////
  
  template< class FSM, typename MSG >
  StateMachine<FSM, MSG>::StateMachine<FSM, MSG>()  {}

  template< class FSM, typename MSG >
  StateMachine<FSM, MSG>::StateMachine<FSM, MSG>( const StateMachine<FSM, MSG>& rhs ) 
  { 
    m_State = std::auto_ptr< State< FSM, MSG > >( rhs.m_State->clone() );
  }

  template< class FSM, typename MSG >
  StateMachine<FSM, MSG>::~StateMachine<FSM, MSG>() {} //nix zu tun dank autoptr
 
  
  template< class FSM, typename MSG >
  void StateMachine<FSM, MSG>::SetNewState( State< FSM, MSG >& newState )
  {
    m_State = std::auto_ptr< State< FSM, MSG > >( &newState );
    //- nochmal nachdenken!!!!!!!!!!!!
  }

  template< class FSM, typename MSG >
  StateMachine<FSM, MSG>& StateMachine<FSM, MSG>::operator=( const StateMachine<FSM, MSG>& rhs ) 
  {
    if ( this == &rhs ) return *this;
    m_State = std::auto_ptr< State< FSM, MSG > >( rhs.m_State->clone() );
    return *this;
  }

  template< class FSM, typename MSG >
  StateMachine<FSM, MSG>* StateMachine<FSM, MSG>::clone() // nötig?
  {
    return new StateMachine<FSM, MSG>( *this );
  }

  template< class FSM, typename MSG >
  void StateMachine<FSM, MSG>::Execute( const float deltaTime )
  {
    if ( m_State.get() ) m_State->Execute( deltaTime );
  }

  template< class FSM, typename MSG >
  void StateMachine<FSM, MSG>::ReceiveMessage( const MSG& message )
  {
    if ( m_State.get() ) m_State->ReceiveMessage( message );
  }


}

#endif // _STATEMACHINE_H