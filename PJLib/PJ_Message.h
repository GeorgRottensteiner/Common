#ifndef _PJ_MESSAGE_H_INCLUDED
#define _PJ_MESSAGE_H_INCLUDED

#pragma warning ( disable : 4786 )
#pragma once

namespace PJ
{
  class CMessage
  {
  private:
    long m_iClientNumber;
    long m_iActionType;     
    long m_iActionValue;

  public:
    CMessage( const long clientnumber, const long actiontype, const long actionvalue = 0 )
      : m_iClientNumber( clientnumber ),
        m_iActionType(   actiontype ),
        m_iActionValue(  actionvalue ) {}
        
    CMessage( const CMessage& rhs )
      : m_iClientNumber( rhs.m_iClientNumber ),
        m_iActionType(   rhs.m_iActionType ),
        m_iActionValue(  rhs.m_iActionValue ) {}
        
    CMessage& operator=( const CMessage& rhs )
    {
      if ( this != &rhs )
      {
        m_iClientNumber = rhs.m_iClientNumber;
        m_iActionType   = rhs.m_iActionType;
        m_iActionValue  = rhs.m_iActionValue;
      }
      return *this;
    }

    inline long Client() const           { return  m_iClientNumber;       }
    inline void Client( const long rhs ) {         m_iClientNumber = rhs; }
    inline long Type() const             { return  m_iActionType;         }
    inline void Type( const long rhs )   {         m_iActionType = rhs;   }
    inline long Value() const            { return  m_iActionValue;        }
    inline void Value( const long rhs )  {         m_iActionValue = rhs;  }
  };

};

#endif//_PJ_MESSAGE_H_INCLUDED