#ifndef GR_VECTOR2_H
#define GR_VECTOR2_H



#include <math.h>

namespace math
{
  template <class T> class vector2
  {
    public:

      typedef T         scalar_type;

      scalar_type       x;
      scalar_type       y;



      long Dimensions() 
      { 
        return 2; 
      }



      vector2() : 
        x( 0 ),   
        y( 0 )   
      {
      }

      vector2( const scalar_type& sx, const scalar_type& sy ) : 
        x( sx ),  
        y( sy )  
      {
      }

      vector2( const vector2& v ) : 
        x( v.x ), 
        y( v.y ) 
      {
      }



      vector2& Set( const scalar_type  s ) 
      { 
        x = y = s; 
        return *this; 
      }



	    vector2& Set( const scalar_type& sx, const scalar_type& sy ) 
      { 
        x = sx; 
        y = sy; 
        return *this;
      }



      vector2& Clear()
      { 
        x = y = 0; 
        return *this; 
      }



	    vector2& operator= ( const vector2& v ) 
      { 
        if ( this != &v ) 
        { 
          x = v.x; 
          y = v.y; 
        } 
        return *this;
      }



      vector2& operator= ( const scalar_type s ) 
      { 
        x = y = s;   
        return *this;
      }



	    vector2& operator+=( const vector2& v ) 
      { 
        x += v.x; 
        y += v.y; 
        return *this; 
      }



	    vector2& operator-=( const vector2& v ) 
      { 
        x -= v.x; 
        y -= v.y; 
        return *this; 
      }



	    vector2& operator+=( const scalar_type s ) 
      { 
        x += s;   
        y += s;   
        return *this; 
      }



	    vector2& operator-=( const scalar_type s ) 
      { 
        x -= s;   
        y -= s;   
        return *this; 
      }



	    vector2& operator/=( const scalar_type s ) 
      { 
        x /= s;   
        y /= s;   
        return *this; 
      }



	    vector2& operator*=( const scalar_type s ) 
      { 
        x *= s;   
        y *= s;   
        return *this; 
      }



      vector2& operator%=( const scalar_type s ) 
      { 
        x %= s;   
        y %= s;   
        return *this; 
      }



	    vector2& Scale( const scalar_type s ) 
      { 
        return (*this) *= s;                        
      }



      vector2& Offset( const scalar_type x, const scalar_type y )
      {
        this->x += x;
        this->y += y;

        return *this;
      }



      scalar_type& operator[]( const int i ) 
      { 
        return *( (&x) + i ); 
      }



      const scalar_type& operator[]( const int i ) const 
      { 
        return *( (&x) + i ); 
      }



      vector2& RotateZ( const double AngleDegrees )
      {
        vector2  vTemp( *this );
        double    w( AngleDegrees * double( 3.1415926535 / 180 ) );

        x = T( vTemp.x * cos( w ) - vTemp.y * sin( w ) );
        y = T( vTemp.x * sin( w ) + vTemp.y * cos( w ) );
        return *this;
      }



      vector2& FlipX()
      {
        x = -x;
        return *this;
      }



      vector2& FlipY()
      {
        y = -y;
        return *this;
      }



      vector2 Normalize()
      {
        float   len = Length();

        if ( len == 0 )
        {
          return *this;
        }
        return operator/=( len );
      }



      vector2 Normalized() const
      {
        if ( Length() <= 0.0f )
        {
          return vector2( *this );
        }
        return vector2( *this ) / Length();
      }



      vector2 operator+( const scalar_type s ) const 
      { 
        return vector2( x + s, y + s ); 
      }



      vector2 operator-( const scalar_type s ) const 
      { 
        return vector2( x - s, y - s ); 
      }



      vector2 operator*( const scalar_type s ) const 
      { 
        return vector2( x * s, y * s ); 
      }



      vector2 operator/( const scalar_type s )  const 
      { 
        return vector2( x / s, y / s ); 
      }



      vector2 operator%( const scalar_type s ) const 
      { 
        return vector2( x % s, y % s ); 
      }



      vector2 operator+( const vector2& v ) const 
      { 
        return vector2( x + v.x, y + v.y );
      }



      vector2 operator-( const vector2& v ) const 
      { 
        return vector2( x - v.x, y - v.y ); 
      }



      vector2 operator-() const 
      { 
        return vector2( -x, -y ); 
      }



      float Length() const
      {
        return sqrtf( (float)LengthSquared() );
      }



      scalar_type LengthSquared() const
      {
        return x * x + y * y;
      }



      float Length( const vector2& rhs ) const
      {
        return ( *this - rhs ).Length();
      }



      scalar_type LengthSquared( const vector2& rhs ) const
      {
        return ( *this - rhs ).LengthSquared();
      }



      bool operator==( const vector2& v ) const 
      { 
        return ( x == v.x )
          &&   ( y == v.y ); 
      }



      bool operator!=( const vector2& v ) const 
      { 
        return !operator==( v ); 
      }



      bool operator< ( const vector2& v ) const
      {
	      return ( ( x < v.x )
           ||    ( ( !( v.x < x ) )
           &&      ( ( y < v.y ) ) ) );
      }



      vector2 RotatedZ( const T AngleDegrees ) const
      { 
        vector2  vTemp( *this );
        double    w( AngleDegrees * 3.1415926535 / 180 );

        vTemp.x = T( x * cos( w ) - y * sin( w ) );
        vTemp.y = T( x * sin( w ) + y * cos( w ) );
        return vTemp; 
      }



      vector2 FlippedX() const 
      { 
        return vector2( -x,  y ); 
      }



      vector2 FlippedY() const 
      { 
        return vector2( x, -y ); 
      }



      vector2<float> Unit() const
      {
        return vector2<float>( (float)x, (float)y ).Normalized();
      }



      float Dot( const vector2& v ) const
      {
        return (float)x * (float)v.x + (float)y * (float)v.y;
      }



      float Angle( const vector2& v2 )
      {
        return 180.0f * acosf( Unit().Dot( v2.Unit() ) ) / 3.1415926f;
      }



      float AngleTowards( const vector2& v2 )
      {
        return ( v2 - *this ).Angle();
      }



      float Angle() const
      {
        return atan2f( (float)y, (float)x ) * 180.0f / 3.1415926f;
      }



      scalar_type Cross( const vector2& v ) const
      {
        return x * v.y - y * v.x;
      }



      friend inline vector2 operator*( const scalar_type& s, const vector2& v ) 
      { 
        return vector2( v.x * s, v.y * s ); 
      }



      float Distance( const vector2& rhs ) const 
      { 
        return ( rhs - *this ).Length(); 
      }



      float DistanceSquared( const vector2& rhs ) const
      {
        return ( rhs - *this ).LengthSquared();
      }



	    vector2& Interpolate( const vector2& rhs, const scalar_type& s )
      {
        x += s * ( rhs.x - x );
        y += s * ( rhs.y - y );
        return *this;
      }



	    vector2& Interpolate( const vector2& v1, const vector2& v2, const scalar_type& s )
      {
        x = v1.x + s * ( v2.x - v1.x );
        y = v1.y + s * ( v2.y - v1.y );
        return *this;
      }



      vector2& Truncate( const float& maximum )
      {
        float l = Length();
        if ( l > maximum )
        {
          *this *= maximum / l;
        }
        return *this;
      }



      scalar_type Perpendicular( const vector2& v2 ) const
      {
        return x * v2.y - y * v2.x;
      }



      vector2 ProjectOn( const vector2& V2 ) const
      {
        vector2    NormV2( V2.Normalized() );

        return NormV2 * NormV2.Dot( *this );
      }



      vector2 NormalRH() const
      {
        return vector2( -y, x );
      }



      vector2 NormalLH() const
      {
        return vector2( y, -x );
      }
  };

}

#endif // GR_VECTOR2_H
