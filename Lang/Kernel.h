#ifndef KERNEL_H
#define KERNEL_H


#include <list>

#include "ITask.h"



namespace GR
{

  class CKernel
  {

    protected:

      typedef std::list<ITask*>   tListTasks;

      tListTasks                  m_listTasks;


    public:



      ~CKernel()
      {
        tListTasks::iterator    it( m_listTasks.begin() );
        while ( it != m_listTasks.end() )
        {
          ITask*    pTask = *it;
          if ( pTask->m_DeleteThisTask )
          {
            delete pTask;
          }

          ++it;
        }
      }


      bool AddTask( ITask* pTask )
      {
        if ( pTask == NULL )
        {
          return false;
        }
        pTask->Init();
        tListTasks::iterator    it( m_listTasks.begin() );
        while ( it != m_listTasks.end() )
        {
          ITask*    pOtherTask = *it;

          if ( pOtherTask->m_Priority > pTask->m_Priority )
          {
            break;
          }
          ++it;
        }
        m_listTasks.insert( it, pTask );
        return true;
      }

      void SuspendTask( ITask* pTask )
      {
        if ( pTask )
        {
          pTask->Suspend();
        }
      }

      void ResumeTask( ITask* pTask )
      {
        if ( pTask )
        {
          pTask->Resume();
        }
      }

      void DetachTask( ITask* pTask )
      {
        if ( pTask )
        {
          m_listTasks.remove( pTask );
        }
      }

      void RemoveTask( ITask* pTask )
      {
        if ( pTask )
        {
          pTask->m_RemoveMe = true;
        }
      }

      void ShutDownTasks()
      {
        tListTasks::iterator    it( m_listTasks.begin() );
        while ( it != m_listTasks.end() )
        {
          ITask*    pTask = *it;

          pTask->m_RemoveMe = true;

          ++it;
        }
      }

      void PauseAllTasks()
      {
        tListTasks::iterator    it( m_listTasks.begin() );
        while ( it != m_listTasks.end() )
        {
          ITask*    pTask = *it;

          pTask->Suspend();
          ++it;
        }
      }

      void ResumeAllTasks()
      {
        tListTasks::iterator    it( m_listTasks.begin() );
        while ( it != m_listTasks.end() )
        {
          ITask*    pTask = *it;

          pTask->Resume();
          ++it;
        }
      }

      void KillAllTasks()
      {
        tListTasks::iterator    it( m_listTasks.begin() );
        while ( it != m_listTasks.end() )
        {
          ITask*    pTask = *it;

          pTask->Exit();
          if ( pTask->m_DeleteThisTask )
          {
            delete pTask;
          }
          it = m_listTasks.erase( it );
        }
      }

      void ExecuteTasks( const float fElapsedTime )
      {
        tListTasks::iterator    it( m_listTasks.begin() );
        while ( it != m_listTasks.end() )
        {
          ITask*    pTask = *it;

          if ( pTask->m_RemoveMe )
          {
            ++it;

            pTask->Exit();
            if ( pTask->m_DeleteThisTask )
            {
              delete pTask;
            }
            continue;
          }

          pTask->Update( fElapsedTime );

          ++it;
        }
      }

  };


}   

#endif // CKERNEL_H

