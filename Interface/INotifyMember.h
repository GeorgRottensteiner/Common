#ifndef _INOTIFYMEMBER_H
#define _INOTIFYMEMBER_H


#include <list>



template <typename T> class INotifyMember
{

  protected:

    typedef INotifyMember<T>                tNotifyMember;

    typedef std::list<tNotifyMember*>       tNotifyMemberList;

    tNotifyMemberList       m_NotifyMembers;


    // diese Routine überschreiben, um Events zu bearbeiten
    virtual void            OnNotify( const T& NotifyMessage, tNotifyMember* pFromMember ) = 0;


  public:

    virtual ~INotifyMember() 
    {};

    virtual void Notify( const T& NotifyMessage )
    {
      tNotifyMemberList::iterator   it( m_NotifyMembers.begin() );
      while ( it != m_NotifyMembers.end() )
      {
        tNotifyMember*    pMember = *it;

        pMember->OnNotify( NotifyMessage, this );

        ++it;
      }
    }



    virtual void AddNotifyMember( tNotifyMember* pNewMember )
    {
      tNotifyMemberList::iterator   it( m_NotifyMembers.begin() );
      while ( it != m_NotifyMembers.end() )
      {
        tNotifyMember*    pMember = *it;

        if ( pMember != pNewMember )
        {
          pMember->m_NotifyMembers.push_back( pNewMember );
        }

        ++it;
      }
      if ( pNewMember != this )
      {
        // meine Member-Liste an das neue Member weitergeben
        tNotifyMemberList::iterator   it( m_NotifyMembers.begin() );
        while ( it != m_NotifyMembers.end() )
        {
          tNotifyMember*    pMember = *it;

          pNewMember->m_NotifyMembers.push_back( pMember );

          ++it;
        }
        m_NotifyMembers.push_back( pNewMember );
      }
    }



    virtual void RemoveNotifyMember( tNotifyMember* pNewMember )
    {
      // Andere benachrichtigen
      tNotifyMemberList::iterator   it( m_NotifyMembers.begin() );
      while ( it != m_NotifyMembers.end() )
      {
        tNotifyMember*    pMember = *it;

        if ( pMember == pNewMember )
        {
          it = m_NotifyMembers.erase( it );
          continue;
        }
        tNotifyMemberList::iterator   itOther( pMember->m_NotifyMembers.begin() );
        while ( itOther != pMember->m_NotifyMembers.end() )
        {
          if ( *itOther == pNewMember )
          {
            pMember->m_NotifyMembers.erase( itOther );
            break;
          }

          ++itOther;
        }

        ++it;
      }
    }

};

#endif// _INOTIFYMEMBER_H

