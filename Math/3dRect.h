#ifndef GR_3DRECT_H_
#define GR_3DRECT_H_

#include <Math/vector3.h>


namespace math
{

  template <class T> class t3dRect
  {

    public:

      typedef math::vector3t<T>     vect_type;


      vect_type     Position,
                    Size;


      t3dRect( const vect_type& Pos, const vect_type& Size ) :
        Position( Pos ),
        Size( Size )
      {
      }



      t3dRect( T MinX = 0, T MinY = 0, T MinZ = 0,
               T Width = 0, T Height = 0, T Depth = 0 ) :
        Position( MinX, MinY, MinZ ),
        Size( Width, Height, Depth )
      {
      }



      t3dRect&      clear()
      {
        return set( 0, 0, 0, 0 );
      }



      t3dRect& set( const T X, const T Y, const T Z,
                    const T Width, const T Height, const T Depth )
      {
        Position.set( X, Y, Z );
        Size.set( Width, Height, Depth );

        return *this;
      }



      t3dRect& set( const vect_type& vPos, const vect_type& vSize )
      {
        Position  = vPos;
        Size      = vSize;

        return *this;
      }



      t3dRect& offset( const T OffsetX, const T OffsetY, const T OffsetZ )
      {
        Position += vect_type( OffsetX, OffsetY, OffsetZ );
        return *this;
      }



      t3dRect& offset( const vect_type& Offset )
      {
        Position += Offset;
        return *this;
      }



      bool operator== ( const t3dRect& rhs )
      {
        return ( ( Position == rhs.Position )
        &&       ( Size == rhs.Size ) );
      }



      t3dRect& operator/ ( const T Divisor )
      {
        if ( Divisor == 0 )
        {
          return *this;
        }

        Position  /= Divisor;
        Size      /= Divisor;

        return *this;
      }



      T Width() const
      {
        return Size.x;
      }



      T Height() const
      {
        return Size.y;
      }



      T Depth() const
      {
        return Size.z;
      }



      void Width( const T Width )
      {
        Size.x = Width;
      }



      void Height( const T Height )
      {
        Size.y = Height;
      }



      void Depth( const T Depth )
      {
        Size.z = Depth;
      }



      t3dRect& normalize()
      {
        if ( Size.x < 0 )
        {
          Position.x += Size.x;
          Size.x      = -Size.x;
        }
        if ( Size.y < 0 )
        {
          Position.y += Size.y;
          Size.y      = -Size.y;
        }
        if ( Size.z < 0 )
        {
          Position.z += Size.z;
          Size.z      = -Size.z;
        }
        return *this;
      }



      vect_type Center() const
      {
        return vect_type( Position + Size / 2 );
      }



      t3dRect& inflate( T DeltaX, T DeltaY, T DeltaZ )
      {
        Position.offset( -DeltaX, -DeltaY, -DeltaZ );
        Size.offset( 2 * DeltaX, 2 * DeltaY, 2 * DeltaZ );

        return *this;
      }



      t3dRect& combine( const t3dRect& rhs )
      {
        normalize();
        if ( rhs.Position.x < Position.x )
        {
          Position.x = rhs.Position.x;
        }
        if ( rhs.Position.x + rhs.Size.x > Position.x + Size.x )
        {
          Size.x = rhs.Position.x + rhs.Size.x - Position.x;
        }
        if ( rhs.Position.y < Position.y )
        {
          Position.y = rhs.Position.y;
        }
        if ( rhs.Position.y + rhs.Size.y > Position.y + Size.y )
        {
          Size.y = rhs.Position.y + rhs.Size.y - Position.y;
        }
        if ( rhs.Position.z < Position.z )
        {
          Position.z = rhs.Position.z;
        }
        if ( rhs.Position.z + rhs.Size.z > Position.z + Size.z )
        {
          Size.z = rhs.Position.z + rhs.Size.z - Position.z;
        }
        return *this;
      }



      bool contains( T X, T Y, T Z ) const
      {
        if ( ( X >= Position.x )
        &&   ( X < Position.x + Size.x )
        &&   ( Y >= Position.y )
        &&   ( Y < Position.y + Size.y )
        &&   ( Z >= Position.z )
        &&   ( Z < Position.z + Size.z ) )
        {
          return true;
        }
        return false;
      }



      bool contains( const vect_type& Point ) const
      {
        return contains( Point.x, Point.y, Point.z );
      }



      //- ist anderes Rect enthalten?
      bool contains( const t3dRect& rhs ) const
      {
        return ( ( contains( rhs.Position ) )
        &&       ( contains( rhs.Position + rhs.Size ) ) );
      }



      //- prüfen, ob sich zwei rects schneiden
      bool intersects( const t3dRect& rhs ) const
      {
        if ( ( Position.x + Size.x <= rhs.Position.x )
        ||   ( Position.x >= rhs.Position.x + rhs.Size.x )
        ||   ( Position.y + Size.y <= rhs.Position.y )
        ||   ( Position.y >= rhs.Position.y + rhs.Size.y )
        ||   ( Position.z + Size.z <= rhs.Position.z )
        ||   ( Position.z >= rhs.Position.z + rhs.Size.z ) )
        {
          return false;
        }
        return true;
      }



      vect_type EndPosition() const
      {
        return Position + Size;
      }



      t3dRect& SetPosition( T X, T Y, T Z )
      {
        Position.set( X, Y, Z );

        return *this;
      }



      t3dRect& SetSize( T Width, T Height, T Depth )
      {
        Size.set( Width, Height, Depth );

        return *this;
      }



      t3dRect operator+ ( const vect_type& vect )
      {
        t3dRect   temp( *this );

        temp.offset( vect );

        return temp;
      }

  };

}

#endif //GR_3DRECT_H_
