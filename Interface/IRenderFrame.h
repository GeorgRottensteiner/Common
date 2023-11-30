#ifndef IRENDERFRAME_H
#define IRENDERFRAME_H

#include <GR/GRTypes.h>

#include <OS/OS.h>

#include <Xtreme/XDisplayMode.h>

#include <Lang/Service.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#endif

#include "IEventProducer.h"
#include "IEventListener.h"
#include "IEventHandler.h"



namespace GR
{

  struct tRenderFrameEvent
  {
    enum eRenderFrameEvent
    {
      RFE_NONE = 0,
      RFE_WINDOW_SIZE_CHANGED,
      RFE_WINDOW_MOVED,
      RFE_WINDOW_DISPLAY_MODE_CHANGED,
    };

    eRenderFrameEvent   m_Event;

    GR::tPoint          m_ptPoint;

    GR::up              m_dwParam;


    tRenderFrameEvent( const eRenderFrameEvent rfEvent = RFE_NONE, const GR::up dwParam = 0, const GR::tPoint& ptPos = GR::tPoint() ) :
      m_Event( rfEvent ),
      m_ptPoint( ptPos ),
      m_dwParam( dwParam )
    {
    }
  };

  class RenderFrame : public ::EventHandler<tRenderFrameEvent>,
                      public GR::Service::Service
  {
    protected:


    public:

      XRendererDisplayMode    m_DisplayMode;

      GR::tPoint              m_NativeScreenSize;

#if OPERATING_SYSTEM == OS_WINDOWS
      HWND                m_hwndMain;
#endif


      RenderFrame() 
#if OPERATING_SYSTEM == OS_WINDOWS
        : m_hwndMain( NULL )
#endif
      {
      }



      virtual ~RenderFrame()
      {
      }



      int Width()
      {
        return m_DisplayMode.Width;
      }


      int Height()
      {
        return m_DisplayMode.Height;
      }



      void SetSize( int Width, int Height )
      {
        m_DisplayMode.Width   = Width;
        m_DisplayMode.Height  = Height;
      }



  };

  typedef IEventListener<tRenderFrameEvent> IRenderFrameListener;
  typedef EventListener<tRenderFrameEvent> RenderFrameListener;

}  // namespace GR

#endif// _IUPDATEABLE_H

