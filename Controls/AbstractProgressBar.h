#ifndef ABSTRACTPROGRESSBAR_H
#define ABSTRACTPROGRESSBAR_H



#include <string>

#include <GR/GRTypes.h>



namespace GUI
{
  template <class BASECLASS> class AbstractProgressBar : public BASECLASS
  {

    public:

    protected:

      GR::u32               m_FullLength,
                            m_CurrentPos;


    public:


      // usings are required for annoying standard conform ignored lookup of base class members
      // alternative would be prefixing everything with "this->" which is even worse
      using BASECLASS::m_ClassName;
      using BASECLASS::m_Style;
      using BASECLASS::m_ComponentFlags;
      using BASECLASS::m_Position;
      using BASECLASS::m_Width;
      using BASECLASS::m_Height;
      using BASECLASS::m_TextAlignment;
      using BASECLASS::m_pFont;
      using BASECLASS::Width;
      using BASECLASS::Height;
      using BASECLASS::m_ClientRect;
      using BASECLASS::RecalcClientRect;
      using BASECLASS::Style;
      using BASECLASS::IsEnabled;
      using BASECLASS::IsVisible;
      using BASECLASS::IsFocused;
      using BASECLASS::LocalToScreen;
      using BASECLASS::SetCapture;
      using BASECLASS::ReleaseCapture;
      using BASECLASS::IsMouseInside;
      using BASECLASS::GenerateEvent;
      using BASECLASS::GenerateEventForParent;
      using BASECLASS::Add;
      using BASECLASS::Delete;



      AbstractProgressBar( GR::u32 Id = 0 ) :
        BASECLASS( Id )
      {
        m_ClassName = "ProgressBar";

        m_FullLength = 100;
        m_CurrentPos = 0;
      }



      AbstractProgressBar( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id = 0 ) :
        BASECLASS( Id )
      {
        m_ClassName = "ProgressBar";

        m_Position.x = NewX;
        m_Position.y = NewY;

        m_Width   = NewWidth;
        m_Height  = NewHeight;

        m_FullLength = 100;
        m_CurrentPos = 0;

        RecalcClientRect();
      }



      virtual void SetRange( const GR::u32 FullLength )
      {
        m_FullLength = FullLength;
        if ( m_CurrentPos > m_FullLength )
        {
          m_CurrentPos = m_FullLength;
          GenerateEventForParent( OET_PROGRESS_POSCHANGE, m_CurrentPos );
        }
      }



      virtual void SetPos( const GR::u32 NewPos )
      {
        GR::u32   posToSet = NewPos;

        if ( posToSet > m_FullLength )
        {
          posToSet = m_FullLength;
        }
        if ( posToSet != m_CurrentPos )
        {
          m_CurrentPos = posToSet;
          GenerateEventForParent( OET_PROGRESS_POSCHANGE, m_CurrentPos );
        }
      }

  };

}

#endif // ABSTRACTPROGRESSBAR_H



