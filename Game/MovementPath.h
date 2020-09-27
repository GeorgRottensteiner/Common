#ifndef GAME_OBJECTPATH_H
#define GAME_OBJECTPATH_H



#include <vector>

#include <Math/vector3.h>

#include <Interface/IIOStream.h>


#undef ABSOLUTE
#undef RELATIVE

namespace GR
{

  namespace Game
  {
    namespace Path
    {
      namespace PathType
      {
        enum Value
        {
          ABSOLUTE = 0,
          RELATIVE,
        };
      }

      class MovementPath
      {
        public:

          struct tPathFrame
          {
            float             FrameLength;
            PathType::Value   Type;
            GR::tVector       StartPos;
            GR::tVector       EndPos;

            tPathFrame( const PathType::Value Type = PathType::ABSOLUTE,
                        const float Length = 1.0f,
                        const GR::tVector& StartPos = GR::tVector(),
                        const GR::tVector& EndPos = GR::tVector() ) :
              FrameLength( Length ),
              Type( Type ),
              StartPos( StartPos ),
              EndPos( EndPos )
            {
            }
          };


        protected:

          typedef std::vector<tPathFrame>   tFrames;

          tFrames           Frames;


        public:

          size_t AddFrame( const PathType::Value Type,
                           const float Length,
                           const GR::tVector& StartPos,
                           const GR::tVector& EndPos = GR::tVector() )
        {
          Frames.push_back( tPathFrame( Type, Length, StartPos, EndPos ) );

          return Frames.size() - 1;
        }



        void RemoveFrame( const size_t FramePos )
        {
          if ( FramePos >= Frames.size() )
          {
            return;
          }
          Frames.erase( Frames.begin() + FramePos );
        }



        void Save( IIOStream& File )
        {
          File.WriteU32( (unsigned long)Frames.size() );
          for ( size_t i = 0; i < Frames.size(); ++i )
          {
            File.WriteBlock( &Frames[i], sizeof( tPathFrame ) );
          }
        }



        void Load( IIOStream& File )
        {
          Frames.clear();

          GR::u32   count = File.ReadU32();

          for ( GR::u32 i = 0; i < count; ++i )
          {
            tPathFrame    Frame;

            File.ReadBlock( &Frame, sizeof( tPathFrame ) );

            Frames.push_back( Frame );
          }
        }



        size_t FrameCount() const
        {
          return Frames.size();
        }



        tPathFrame* Frame( size_t Frame )
        {
          if ( Frame >= Frames.size() )
          {
            return NULL;
          }
          return &Frames[Frame];
        }

        friend struct tPathPos;

      };

      struct tPathPos
      {
        size_t          CurrentFrame;
        float           ElapsedFrameTime;
        MovementPath*   pPath;



        tPathPos( MovementPath* pPathOwner = NULL ) :
          CurrentFrame( 0 ),
          ElapsedFrameTime( 0.0f ),
          pPath( pPathOwner )
        {
        }



        GR::tVector Update( const float ElapsedTime, const GR::tVector& Start )
        {
          if ( pPath == NULL )
          {
            return Start;
          }
          if ( CurrentFrame >= pPath->Frames.size() )
          {
            // drüber hinaus?
            return Start;
          }
          GR::tVector   curPos( Start );

          float   timeLeft = ElapsedTime;
          while ( timeLeft > 0.0f )
          {
            MovementPath::tPathFrame&   CurFrame = pPath->Frames[CurrentFrame];
            if ( ElapsedFrameTime + timeLeft >= CurFrame.FrameLength )
            {
              // Frame ist fertig
              if ( CurFrame.Type == PathType::ABSOLUTE )
              {
                curPos = CurFrame.EndPos;
              }
              else
              {
                curPos += CurFrame.StartPos * ( CurFrame.FrameLength - ElapsedFrameTime );
              }

              timeLeft -= CurFrame.FrameLength;
              ElapsedFrameTime = 0.0f;

              CurrentFrame++;
              if ( CurrentFrame >= pPath->Frames.size() )
              {
                CurrentFrame = 0;
              }
            }
            else
            {
              if ( CurFrame.Type == PathType::ABSOLUTE )
              {
                curPos = CurFrame.StartPos + ( CurFrame.EndPos - CurFrame.StartPos ) * ( ElapsedFrameTime + timeLeft ) / CurFrame.FrameLength;
              }
              else
              {
                curPos += CurFrame.StartPos * timeLeft / CurFrame.FrameLength;
              }

              ElapsedFrameTime += timeLeft;
              timeLeft = 0.0f;
            }
          }
          return curPos;
        }
      };


    }

  }

}

#endif // GAME_OBJECTPATH_H