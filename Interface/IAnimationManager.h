#ifndef IANIMATIONMANAGER_H
#define IANIMATIONMANAGER_H



#include <vector>
#include <map>

#include <GR/GRTypes.h>

#include <Interface/IIOStream.h>



struct tAnimationPos
{
  GR::u32     AnimationId;
  size_t      Frame;
  float       FrameDelayPos;



  tAnimationPos( GR::u32 ID = 0 ) :
    AnimationId( ID ),
    Frame( 0 ),
    FrameDelayPos( 0 )
  {
  }



  void Start( GR::u32 ID = -1 )
  {
    if ( ID != -1 )
    {
      AnimationId = ID;
    }

    Frame = 0;
    FrameDelayPos = 0.0f;
  }

};



template <typename FRM> class KeyFramedAnimation
{
  public:

    bool      Looping;



    struct tAnimationFrame
    {
      FRM       Frame;
      float     Length;

      tAnimationFrame() :
        Frame( 0 ),
        Length( 1.0f )
      {
      }

      tAnimationFrame( FRM Frame, float Length ) :
        Frame( Frame ),
        Length( Length )
      {
      }
    };

    typedef std::vector<tAnimationFrame> tVectAnimationFrames;

    tVectAnimationFrames      Frames;


    KeyFramedAnimation() :
      Looping( false )
    {
      Frames.clear();
    }



    void AddFrame( FRM Frame, float Length = 1.0f )
    {
      Frames.push_back( tAnimationFrame( Frame, Length ) );
    }



    void RemoveFrame( GR::u32 Frame )
    {
      if ( Frame >= (GR::u32)Frames.size() )
      {
        return;
      }
      Frames.erase( Frames.begin() + Frame );
    }



    GR::f32 Length() const
    {
      GR::f32       length = 0.0f;

      typename tVectAnimationFrames::const_iterator    it( Frames.begin() );
      while ( it != Frames.end() )
      {
        const tAnimationFrame& frame( *it );

        if ( frame.Length == -1.0f )
        {
          // ein Endlos-Frame
          return -1.0f;
        }
        length += frame.Length;

        ++it;
      }
      return length;
    }

};



template <typename FRM> class IAnimationManager
{
  public:

    typedef KeyFramedAnimation<FRM>           tAnimType;

    typedef std::vector<tAnimType>            tVectAnimations;

    tVectAnimations                           Animations;



    IAnimationManager()
    {
      Animations.clear();
    }



    void Clear()
    {
      Animations.clear();
    }



    int RegisterAnimation( tAnimType& Animation )
    {
      Animations.push_back( Animation );

      return (int)Animations.size() - 1;
    }



    GR::f32 GetAnimLength( const GR::u32 AnimationID ) const
    {
      if ( AnimationID >= (GR::u32)Animations.size() )
      {
        return 0.0f;
      }
      GR::f32   length = 0.0f;

      const tAnimType& anim( Animations[AnimationID] );

      for ( size_t frame = 0; frame < anim.Frames.size(); ++frame )
      {
        length += anim.Frames[frame].Length;
      }

      return length;
    }



    FRM GetAnimFrame( const tAnimationPos& FramePos )
    {
      if ( FramePos.AnimationId >= Animations.size() )
      {
        return FRM();
      }
      tAnimType& anim = Animations[FramePos.AnimationId];

      if ( anim.Frames.empty() )
      {
        return FRM();
      }
      if ( FramePos.Frame >= anim.Frames.size() )
      {
        return anim.Frames[0].Frame;
      }
      return anim.Frames[FramePos.Frame].Frame;
    }



    FRM AdvanceAnimByFrame( tAnimationPos& FramePos )
    {
      if ( FramePos.AnimationId >= Animations.size() )
      {
        return FRM();
      }
      tAnimType& anim = Animations[FramePos.AnimationId];

      if ( FramePos.Frame < anim.Frames.size() )
      {
        if ( anim.Frames[FramePos.Frame].Length == -1 )
        {
          // -1 bedeutet, auf ewig dieser Frame
          return anim.Frames[FramePos.Frame].Frame;
        }
      }
      if ( anim.Frames.empty() )
      {
        return FRM();
      }
      FramePos.Frame++;
      if ( FramePos.Frame >= anim.Frames.size() )
      {
        FramePos.Frame = 0;
      }
      return anim.Frames[FramePos.Frame].Frame;
    }



    FRM AdvanceAnimFrame( tAnimationPos& FramePos, float ElapsedTime )
    {
      if ( FramePos.AnimationId >= Animations.size() )
      {
        return FRM();
      }
      tAnimType& anim = Animations[FramePos.AnimationId];

      if ( anim.Frames.empty() )
      {
        return FRM();
      }

      if ( FramePos.Frame < anim.Frames.size() )
      {
        typename tAnimType::tAnimationFrame&    Frame = anim.Frames[FramePos.Frame];
        if ( Frame.Length == -1.0f )
        {
          // -1 bedeutet, auf ewig dieser Frame
          return Frame.Frame;
        }
        if ( Frame.Length == 0 )
        {
          // 0 bedeutet, auf ewig dieser Frame
          return Frame.Frame;
        }
      }
      if ( FramePos.Frame >= anim.Frames.size() )
      {
        FramePos.Frame = 0;
      }
      do
      {
        typename tAnimType::tAnimationFrame&  animFrame = anim.Frames[FramePos.Frame];

        if ( animFrame.Length <= 0.0f )
        {
          // Endlos-Frame
          return animFrame.Frame;
        }
        if ( FramePos.FrameDelayPos + ElapsedTime > animFrame.Length )
        {
          ElapsedTime -= animFrame.Length - FramePos.FrameDelayPos;
          FramePos.Frame++;
          if ( FramePos.Frame >= anim.Frames.size() )
          {
            FramePos.Frame = 0;
          }
          FramePos.FrameDelayPos = 0.0f;
        }
        else
        {
          FramePos.FrameDelayPos += ElapsedTime;
          ElapsedTime = 0;
        }
        // ACHTUNG - kann zur Endlosschleife werden!
      }
      while ( ElapsedTime > 0.0f );

      return anim.Frames[FramePos.Frame].Frame;
    }



    bool IsAnimationCompleted( tAnimationPos& FramePos )
    {
      if ( FramePos.AnimationId >= Animations.size() )
      {
        return true;
      }
      tAnimType& anim = Animations[FramePos.AnimationId];

      if ( FramePos.Frame >= anim.Frames.size() )
      {
        return true;
      }

      if ( anim.Frames[FramePos.Frame].Length == -1.0f )
      {
        return true;
      }
      return false;

    }


    void Save( IIOStream& Stream )
    {
      Stream.WriteU32( (GR::u32)Animations.size() );

      typename tVectAnimations::iterator    it( Animations.begin() );
      while ( it != Animations.end() )
      {
        tAnimType&    Anim = *it;

        Stream.WriteU32( (GR::u32)Anim.Frames.size() );
        for ( size_t i = 0; i < Anim.Frames.size(); ++i )
        {
          Stream.WriteBlock( &Anim.Frames[i], sizeof( KeyFramedAnimation<FRM>::tAnimationFrame ) );
        }

        ++it;
      }
    }



    void Load( IIOStream& Stream )
    {
      Animations.clear();

      int   iAnimCount = Stream.ReadU32();

      Animations.resize( iAnimCount );
      for ( int i = 0; i < iAnimCount; ++i )
      {
        int           iFrames = Stream.ReadU32();

        Animations[i].Frames.resize( iFrames );

        for ( int j = 0; j < iFrames; j++ )
        {
          Stream.ReadBlock( &Animations[i].Frames[j], sizeof( KeyFramedAnimation<FRM>::tAnimationFrame ) );
        }
      }
    }

};




#endif // IANIMATIONMANAGER_H



