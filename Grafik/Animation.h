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
        GR::Graphic::Image*   pImage;
        GR::f32               Length;

        tAnimFrame( GR::Graphic::Image* pImage = NULL,
                    GR::f32 Length = 0.0f ) :
                    pImage( pImage ),
                    Length( Length )
        {
        }
      };

      unsigned long               Position;

      typedef std::vector<tAnimFrame>     tVectImages;

      tVectImages                 m_Images;


    public:

      struct tFramePos
      {
        GR::up        Frame;
        GR::f32       FrameTimeElapsed;
        Animation*    pAnimation;

        tFramePos( Animation* pAnim = NULL ) :
          pAnimation( pAnim ),
          Frame( 0 ),
          FrameTimeElapsed( 0.0f )
        {
        }

        void SetAnimation( Animation* pAnim )
        {
          pAnimation = pAnim;
          Frame = 0;
          FrameTimeElapsed = 0.0f;
        }

        void Update( const float ElapsedTime )
        {
          if ( ( pAnimation == NULL )
          ||   ( Frame >= pAnimation->m_Images.size() ) )
          {
            return;
          }

          float   delta = ElapsedTime;

          while ( delta > 0.0f )
          {
            const tAnimFrame&   curFrame = pAnimation->m_Images[Frame];

            if ( curFrame.Length == 0.0f )
            {
              // letzer Frame, dieser Frame bleibt
              return;
            }

            if ( FrameTimeElapsed + delta < curFrame.Length )
            {
              // der Frame ist noch nicht fertig
              FrameTimeElapsed += delta;
              return;
            }

            delta -= curFrame.Length - FrameTimeElapsed;
            FrameTimeElapsed = 0.0f;

            if ( pAnimation->GetType() & AT_FORWARD )
            {
              Frame++;
              if ( ( Frame >= pAnimation->m_Images.size() )
              &&   ( pAnimation->GetType() & AT_LOOP ) )
              {
                Frame = 0;
              }
            }
            else if ( pAnimation->GetType() & AT_PING )
            {
              Frame++;
              if ( Frame >= pAnimation->m_Images.size() - 1 )
              {
                Frame = pAnimation->m_Images.size() - 1;

                GR::u32   dwType = pAnimation->GetType();
                dwType     &= ~AT_PING;
                pAnimation->SetType( dwType | AT_PONG );
              }
            }
            else if ( pAnimation->GetType() & AT_PONG )
            {
              if ( Frame > 0 )
              {
                Frame--;
              }
              else
              {
                GR::u32   dwType = pAnimation->GetType();
                dwType     &= ~AT_PONG;
                pAnimation->SetType( dwType | AT_PING );
              }
            }
            else if ( pAnimation->GetType() & AT_REVERSE )
            {
              if ( Frame > 0 )
              {
                Frame--;
              }
              else if ( pAnimation->GetType() & AT_LOOP )
              {
                if ( !pAnimation->m_Images.empty() )
                {
                  Frame = pAnimation->m_Images.size() - 1;
                }
              }
            }
          }
        }



        bool PutAnimation( GR::Graphic::GFXPage* pPage, GR::i32 X, GR::i32 Y, GR::u32 Flags = IMAGE_METHOD_OPTIMAL, GR::Graphic::Image* pMaskImage = NULL )
        {
          if ( ( pAnimation == NULL )
          ||   ( pPage == NULL ) )
          {
            return false;
          }
          pAnimation->SetPosition( (GR::u32)Frame );
          return pAnimation->PutAnimation( pPage, X, Y, Flags, pMaskImage );
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

      Animation( const GR::Char* FileName );

      Animation( IIOStream& Stream );

      // Entfernt das Animationsobjekt
      ~Animation();

      void Reset( void );

      bool AddFrame( GR::Graphic::Image* pImage, GR::f32 Length );
      bool InsertFrame( GR::u32 Index, GR::Graphic::Image* pImage, GR::f32 Length );
      bool InsertFrameBehind( GR::u32 Index, GR::Graphic::Image* pImage, GR::f32 Length );

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

      bool PutAnimation( GR::Graphic::GFXPage* pActualPage, signed long XPos, signed long YPos, GR::u32 Flags = IMAGE_METHOD_OPTIMAL, GR::Graphic::Image* pMaskImage = NULL );

      void Compress( GR::u32 TransparentColor );


    protected:

      GR::u32                   m_Type;


      void Set( GR::u32 NewType );


  };

}    // namespace GR


#endif // ANIMATION_H



