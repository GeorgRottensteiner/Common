#ifndef TRECT_H
#define TRECT_H

#include <Math/vector2.h>
#include <Math/mathtemplates.h>


namespace math
{

  template <class T> class tRect
  {

    public:

      typedef T     scalar_type;


      scalar_type   Left,
                    Top,
                    Right,
                    Bottom;

      tRect( const math::vector2<T>& Pos, const math::vector2<T>& Size ) :
        Left( Pos.x ),
        Top( Pos.y ),
        Right( Left + Size.x ),
        Bottom( Top + Size.y )
      {
      }



      tRect( scalar_type Left = 0, scalar_type Top = 0, scalar_type Width = 0, scalar_type Height = 0 ) :
        Left( Left ),
        Top( Top ),
        Right( Left + Width ),
        Bottom( Top + Height )
      {
      }



      tRect& Clear()
      {
        return Set( 0, 0, 0, 0 );
      }



      tRect& Set( const scalar_type& X, const scalar_type& Y, const scalar_type& Width, const scalar_type& Height )
      {
        Left   = X;
        Top    = Y;
        Right  = Left + Width;
        Bottom = Top + Height;

        Normalize();

        return *this;
      }



      tRect& Offset( const scalar_type& OffsetX, const scalar_type& OffsetY )
      {
        Left   += OffsetX;
        Top    += OffsetY;
        Right  += OffsetX;
        Bottom += OffsetY;

        return *this;
      }



      tRect& Offset( const math::vector2<scalar_type>& Offset )
      {
        Left   += Offset.x;
        Top    += Offset.y;
        Right  += Offset.x;
        Bottom += Offset.y;

        return *this;
      }



      tRect& operator= ( const tRect& OtherRect )
      {
        if ( this != &OtherRect )
        {
          Left   = OtherRect.Left;
          Top    = OtherRect.Top;
          Right  = OtherRect.Right;
          Bottom = OtherRect.Bottom;
        }
        return *this;
      }



      tRect& operator/ ( const T Divisor )
      {
        if ( Divisor == 0 )
        {
          return *this;
        }

        Left     /= Divisor;
        Top      /= Divisor;
        Right    /= Divisor;
        Bottom   /= Divisor;

        return *this;
      }



      tRect& operator*= ( const T Multiplier )
      {
        if ( Multiplier == 1 )
        {
          return *this;
        }

        Left     *= Multiplier;
        Top      *= Multiplier;
        Right    *= Multiplier;
        Bottom   *= Multiplier;

        return *this;
      }



      bool operator==( const tRect& OtherRect ) const
      {
        return ( ( Left == OtherRect.Left )
        &&       ( Top == OtherRect.Top )
        &&       ( Right == OtherRect.Right )
        &&       ( Bottom == OtherRect.Bottom ) );
      }


      scalar_type Width() const
      {
        return Right - Left;
      }



      scalar_type Height() const
      {
        return Bottom - Top;
      }



      void Width( const scalar_type Width )
      {
        Right = Left + Width;
      }



      void Height( const scalar_type Height )
      {
        Bottom = Top + Height;
      }



      tRect& Normalize()
      {
        if ( Right < Left )
        {
          std::swap( Left, Right );
        }
        if ( Bottom < Top )
        {
          std::swap( Top, Bottom );
        }
        return *this;
      }



      bool Empty() const
      {
        return ( ( Width() == 0 ) 
            ||   ( Height() == 0 ) );
      }



      math::vector2<scalar_type> Center() const
      {
        return math::vector2<scalar_type>( Left + Width() / 2, Top + Height() / 2 );
      }



      scalar_type CenterX() const
      {
        return (scalar_type)( Left + Width() / 2 );
      }



      scalar_type CenterY() const
      {
        return (scalar_type)( Top + Height() / 2 );
      }



      tRect& Inflate( scalar_type DeltaX, scalar_type DeltaY )
      {
        Left   -= DeltaX;
        Top    -= DeltaY;
        Right  += DeltaX;
        Bottom += DeltaY;

        return *this;
      }



      tRect& MoveTop( scalar_type TopDeltaY )
      {
        Top += TopDeltaY;

        Normalize();
        return *this;
      }



      tRect& MoveLeft( scalar_type TopDeltaX )
      {
        Left += TopDeltaX;

        Normalize();
        return *this;
      }



      tRect& MoveBottom( scalar_type TopDeltaY )
      {
        Bottom += TopDeltaY;

        Normalize();
        return *this;
      }



      tRect& MoveRight( scalar_type TopDeltaX )
      {
        Right += TopDeltaX;

        Normalize();
        return *this;
      }



      tRect& Combine( const tRect& OtherRect )
      {
        Normalize();
        if ( OtherRect.Left < Left )
        {
          Left = OtherRect.Left;
        }
        if ( OtherRect.Right > Right )
        {
          Right = OtherRect.Right;
        }
        if ( OtherRect.Top < Top )
        {
          Top = OtherRect.Top;
        }
        if ( OtherRect.Bottom > Bottom )
        {
          Bottom = OtherRect.Bottom;
        }
        return *this;
      }



      tRect Intersection( const tRect& rhs ) const
      {
        tRect   rectTemp;

        rectTemp.Left    = math::maxValue( rhs.Left, Left );
        rectTemp.Right   = math::minValue( rhs.Right, Right );
        rectTemp.Top     = math::maxValue( rhs.Top, Top );
        rectTemp.Bottom  = math::minValue( rhs.Bottom, Bottom );

        if ( rectTemp.Left > rectTemp.Right )
        {
          return tRect();
        }
        if ( rectTemp.Top > rectTemp.Bottom )
        {
          return tRect();
        }

        return rectTemp;
      }



      bool Contains( scalar_type X, scalar_type Y ) const
      {
        if ( ( X >= Left )
        &&   ( X < Right )
        &&   ( Y >= Top )
        &&   ( Y < Bottom ) )
        {
          return true;
        }
        return false;
      }



      bool Contains( const math::vector2<scalar_type>& tPoint ) const
      {
        return Contains( tPoint.x, tPoint.y );
      }



      bool Contains( const tRect& OtherRect ) const
      {
        return ( ( Contains( OtherRect.Left, OtherRect.Top ) )
        &&       ( Contains( OtherRect.Right - 1, OtherRect.Bottom - 1 ) ) );
      }



      bool Intersects( const tRect& OtherRect ) const
      {
        if ( ( Right <= OtherRect.Left )
        ||   ( Left >= OtherRect.Right )
        ||   ( Bottom <= OtherRect.Top )
        ||   ( Top >= OtherRect.Bottom ) )
        {
          return false;
        }
        return true;
      }



      bool Intersects( const tRect& OtherRect, tRect& IntersectionArea ) const
      {
        IntersectionArea = Intersection( OtherRect );

        return !IntersectionArea.Empty();
      }



      math::vector2<scalar_type> Position() const
      {
        return math::vector2<scalar_type>( Left, Top );
      }



      math::vector2<scalar_type> Size() const
      {
        return math::vector2<scalar_type>( Width(), Height() );
      }



      tRect& Position( scalar_type iX, scalar_type iY )
      {
        Left  = iX;
        Top   = iY;

        return *this;
      }



      math::vector2<scalar_type> EndPosition() const
      {
        return Position() + Size();
      }



      tRect& Size( scalar_type Width, scalar_type Height )
      {
        Right    = Left + Width;
        Bottom   = Top + Height;

        return *this;
      }

  };

}

#endif 
