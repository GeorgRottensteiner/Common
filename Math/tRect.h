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

      tRect( scalar_type iLeft = 0, scalar_type iTop = 0, scalar_type iWidth = 0, scalar_type iHeight = 0 ) :
        Left( iLeft ),
        Top( iTop ),
        Right( iLeft + iWidth ),
        Bottom( iTop + iHeight )
      {
      }

      tRect&      clear()
      {
        return set( 0, 0, 0, 0 );
      }



      tRect& set( const scalar_type& X, const scalar_type& Y, const scalar_type& Width, const scalar_type& Height )
      {
        Left   = X;
        Top    = Y;
        Right  = Left + Width;
        Bottom = Top + Height;

        return *this;
      }



      tRect& offset( const scalar_type& OffsetX, const scalar_type& OffsetY )
      {
        Left   += OffsetX;
        Top    += OffsetY;
        Right  += OffsetX;
        Bottom += OffsetY;

        return *this;
      }



      tRect& offset( const math::vector2<scalar_type>& Offset )
      {
        Left   += Offset.x;
        Top    += Offset.y;
        Right  += Offset.x;
        Bottom += Offset.y;

        return *this;
      }

      tRect& operator = ( const tRect& rcRHS )
      {
        if ( this != &rcRHS )
        {
          Left   = rcRHS.Left;
          Top    = rcRHS.Top;
          Right  = rcRHS.Right;
          Bottom = rcRHS.Bottom;
        }
        return *this;
      }



      tRect& operator/ ( const T iDivisor )
      {
        if ( iDivisor == 0 )
        {
          return *this;
        }

        Left     /= iDivisor;
        Top      /= iDivisor;
        Right    /= iDivisor;
        Bottom   /= iDivisor;

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

      bool operator==( const tRect& rcRHS)const
      {
        return ( ( Left == rcRHS.Left )
        &&       ( Top == rcRHS.Top )
        &&       ( Right == rcRHS.Right )
        &&       ( Bottom == rcRHS.Bottom ) );
      }

      scalar_type width() const
      {
        return Right - Left;
      }

      scalar_type height() const
      {
        return Bottom - Top;
      }

      void width( const scalar_type iWidth )
      {
        Right = Left + iWidth;
      }

      void height( const scalar_type iHeight )
      {
        Bottom = Top + iHeight;
      }

      tRect& normalize()
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



      bool empty() const
      {
        return ( ( width() == 0 ) && ( height() == 0 ) );
      }



      math::vector2<scalar_type> center() const
      {
        return math::vector2<scalar_type>( Left + width() / 2, Top + height() / 2 );
      }

      tRect&      inflate( scalar_type iDeltaX, scalar_type iDeltaY )
      {
        Left   -= iDeltaX;
        Top    -= iDeltaY;
        Right  += iDeltaX;
        Bottom += iDeltaY;

        return *this;
      }

      tRect&      MoveTop( scalar_type iTopDeltaY )
      {
        Top += iTopDeltaY;
        return *this;
      }

      tRect&      MoveLeft( scalar_type iTopDeltaX )
      {
        Left += iTopDeltaX;
        return *this;
      }

      tRect&      MoveBottom( scalar_type iTopDeltaY )
      {
        Bottom += iTopDeltaY;
        return *this;
      }

      tRect&      MoveRight( scalar_type iTopDeltaX )
      {
        Right += iTopDeltaX;
        return *this;
      }

      tRect&      combine( const tRect& rhs )
      {
        normalize();
        if ( rhs.Left < Left )
        {
          Left = rhs.Left;
        }
        if ( rhs.Right > Right )
        {
          Right = rhs.Right;
        }
        if ( rhs.Top < Top )
        {
          Top = rhs.Top;
        }
        if ( rhs.Bottom > Bottom )
        {
          Bottom = rhs.Bottom;
        }
        return *this;
      }

      tRect intersection( const tRect& rhs )
      {
        normalize();

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

      bool contains( scalar_type iX, scalar_type iY ) const
      {
        if ( ( iX >= Left )
        &&   ( iX < Right )
        &&   ( iY >= Top )
        &&   ( iY < Bottom ) )
        {
          return true;
        }
        return false;
      }

      bool contains( const math::vector2<scalar_type>& tPoint ) const
      {
        return contains( tPoint.x, tPoint.y );
      }

      //- ist anderes Rect enthalten?
      bool contains( const tRect& rhs ) const
      {
        return ( ( contains( rhs.Left, rhs.Top ) )
        &&       ( contains( rhs.Right - 1, rhs.Bottom - 1 ) ) );
      }

      //- prüfen, ob sich zwei rects schneiden
      bool intersects( const tRect& rhs ) const
      {
        if ( ( Right <= rhs.Left )
        ||   ( Left >= rhs.Right )
        ||   ( Bottom <= rhs.Top )
        ||   ( Top >= rhs.Bottom ) )
        {
          return false;
        }
        return true;
      }

      math::vector2<scalar_type> position() const
      {
        return math::vector2<scalar_type>( Left, Top );
      }

      math::vector2<scalar_type> size() const
      {
        return math::vector2<scalar_type>( width(), height() );
      }

      tRect& position( scalar_type iX, scalar_type iY )
      {
        Left  = iX;
        Top   = iY;

        return *this;
      }

      math::vector2<scalar_type> endposition() const
      {
        return position() + size();
      }

      tRect& size( scalar_type iWidth, scalar_type iHeight )
      {
        Right    = Left + iWidth;
        Bottom   = Top + iHeight;

        return *this;
      }

  };

}

#endif // TRECT_H
