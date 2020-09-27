#ifndef ITASK_H
#define ITASK_H



namespace GR
{

  struct ITask
  {

    protected:

      bool            m_Paused;

      bool            m_RemoveMe,
                      m_DeleteThisTask;

      int             m_Priority;


    public:

      ITask() :
        m_Paused( true ),
        m_RemoveMe( false ),
        m_DeleteThisTask( false ),
        m_Priority( 5000 )
      {
      }


      virtual void    Init()
      {
      }

      virtual void    Exit()
      {
      }

      virtual void    Update( const float )
      {
      }

      virtual void    Suspend()
      {
        m_Paused = true;
      }

      virtual void    Resume()
      {
        m_Paused = true;
      }

      void SetTaskPriority( int iPrio )
      {
        m_Priority = iPrio;
      }

      friend class CKernel;

  };


}    // namespace GR

#endif// ITASK_H

