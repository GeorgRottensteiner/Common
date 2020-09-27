#ifndef ANIMATION_H
#define ANIMATION_H



#include <Grafik/Image.h>

#include <GR/GRTypes.h>

#include <vector>



namespace GR
{

  class Animation
  {

    private:

      struct tAnimFrame
      {
        GR::Graphic::Image*   m_pImage;
        GR::f32               m_fLength;

        tAnimFrame( GR::Graphic::Image* pImage = NULL,
                    GR::f32 fLength = 0.0f ) :
                    m_pImage( pImage ),
                    m_fLength( fLength )
        {
        }
      };

      unsigned long               Position;

      typedef std::vector<tAnimFrame>     tVectImages;

      tVectImages                 m_Images;


    public:

      struct tFramePos
      {
        GR::up        m_Frame;
        GR::f32       m_FrameTimeElapsed;
        Animation*   m_pAnimation;

        tFramePos( Animation* pAnim = NULL ) :
          m_pAnimation( pAnim ),
          m_Frame( 0 ),
          m_FrameTimeElapsed( 0.0f )
        {
        }

        void SetAnimation( Animation* pAnim )
        {
          m_pAnimation = pAnim;
          m_Frame = 0;
          m_FrameTimeElapsed = 0.0f;
        }

        void Update( const float ElapsedTime )
        {
          if ( ( m_pAnimation == NULL )
          ||   ( m_Frame >= m_pAnimation->m_Images.size() ) )
          {
            return;
          }

          float   fDelta = ElapsedTime;

          while ( fDelta > 0.0f )
          {
            const tAnimFrame&   curFrame = m_pAnimation->m_Images[m_Frame];

            if ( curFrame.m_fLength == 0.0f )
            {
              // letzer Frame, dieser Frame bleibt
              return;
            }

            if ( m_FrameTimeElapsed + fDelta < curFrame.m_fLength )
            {
              // der Frame ist noch nicht fertig
              m_FrameTimeElapsed += fDelta;
              return;
            }

            fDelta -= curFrame.m_fLength - m_FrameTimeElapsed;
            m_FrameTimeElapsed = 0.0f;

            if ( m_pAnimation->GetType() & AT_FORWARD )
            {
              m_Frame++;
              if ( ( m_Frame >= m_pAnimation->m_Images.size() )
              &&   ( m_pAnimation->GetType() & AT_LOOP ) )
              {
                m_Frame = 0;
              }
            }
            else if ( m_pAnimation->GetType() & AT_PING )
            {
              m_Frame++;
              if ( m_Frame >= m_pAnimation->m_Images.size() - 1 )
              {
                m_Frame = m_pAnimation->m_Images.size() - 1;

                GR::u32   dwType = m_pAnimation->GetType();
                dwType     &= ~AT_PING;
                m_pAnimation->SetType( dwType | AT_PONG );
              }
            }
            else if ( m_pAnimation->GetType() & AT_PONG )
            {
              if ( m_Frame > 0 )
              {
                m_Frame--;
              }
              else
              {
                GR::u32   dwType = m_pAnimation->GetType();
                dwType     &= ~AT_PONG;
                m_pAnimation->SetType( dwType | AT_PING );
              }
            }
            else if ( m_pAnimation->GetType() & AT_REVERSE )
            {
              if ( m_Frame > 0 )
              {
                m_Frame--;
              }
              else if ( m_pAnimation->GetType() & AT_LOOP )
              {
                if ( !m_pAnimation->m_Images.empty() )
                {
                  m_Frame = m_pAnimation->m_Images.size() - 1;
                }
              }
            }
          }
        }



        bool PutAnimation( GR::Graphic::GFXPage* pPage, GR::i32 iX, GR::i32 iY, GR::u32 ulFlags = IMAGE_METHOD_OPTIMAL, GR::Graphic::Image *pMaskImage = NULL )
        {
          if ( ( m_pAnimation == NULL )
          ||   ( pPage == NULL ) )
          {
            return false;
          }
          m_pAnimation->SetPosition( (GR::u32)m_Frame );
          return m_pAnimation->PutAnimation( pPage, iX, iY, ulFlags, pMaskImage );
        }
      };

      enum eAnimType
      {
        AT_INVALID    = 0,
        AT_FORWARD    = 0x00000001,
        AT_LOOP       = 0x00000002,
        AT_PING       = 0x00000004,
        AT_PONG       = 0x00000008,
        AT_REVERSE    = 0x00000010,
      };

      // Erzeugt ein leeres Animationsobjekt
      Animation( GR::u32 NewType = AT_FORWARD | AT_LOOP );

      Animation( const GR::Char* lpszFileName );

      Animation( IIOStream& Stream );

      // Entfernt das Animationsobjekt
      ~Animation();

      void Reset( void );

      bool AddFrame( GR::Graphic::Image* pImage, GR::f32 fLength );
      bool InsertFrame( GR::u32 Index, GR::Graphic::Image* pImage, GR::f32 fLength );
      bool InsertFrameBehind( GR::u32 Index, GR::Graphic::Image* pImage, GR::f32 fLength );

      bool DeleteFrame( GR::Graphic::Image* pImage );

      bool RemoveFrame( GR::Graphic::Image* pImage );

      bool Load( const GR::String& FileName );

      bool Load( IIOStream& Stream );

      bool Save( const GR::String& FileName );

      bool LoadAni( const GR::String& FileName, unsigned char ucBpp );

      bool LoadAniAt( IIOStream& ioIn, unsigned char ucBpp );

      GR::u32 GetType();

      GR::u16 GetWidth();

      GR::u16 GetHeight();

      GR::u32 GetFrames();

      GR::u32 GetPosition();

      GR::Graphic::Image* GetFirstImage();

      GR::Graphic::Image* GetImage( GR::u32 Number );

      void SetType( GR::u32 ulNewType );

      void SetPosition( GR::u32 ulNewPosition );

      void SetFrame( GR::u32 Nr, GR::Graphic::Image* pImage );

      void Next();

      bool PutAnimation( GR::Graphic::GFXPage *pActualPage, signed long slXPos, signed long slYPos, GR::u32 ulFlags = IMAGE_METHOD_OPTIMAL, GR::Graphic::Image *pMaskImage = NULL );

      void Compress( GR::u32 TransparentColor );


    protected:

      GR::u32                   m_Type;


      void Set( GR::u32 NewType );


  };

}    // namespace GR


#endif // ANIMATION_H



