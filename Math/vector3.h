#ifndef _PJ_vector3t_h
#define _PJ_vector3t_h

#include <math.h>
#include <algorithm>

// ================================================
//@{
// Enthält mathematische Klassen und Funktionen.

// Zum Beispiel Rechnen.
//
// Und auch Addieren..
// @author Peter Jerz
// @version 1.0
//@}
// ================================================

namespace math
{
  //- contents:
  template<class T> class vector3t;
  template<class T> class matrix3t;
  template<class T> class basis3t;
  template<class T> class coordframe3t;

// ================================================
//@{
// 3D-Vektorklasse.

// vector3t ist ein Klassentemplate für 3D-Vektoren.
//
// Komponenten können wahlweise angesprochen werden mit x,y,z, mit r,g,b oder mit dem Operator [].
// Auch als Farbwert geeignet.
// @author Peter Jerz
// @version 1.0
//@}
// ================================================
  template <class T>
  class vector3t
  {
  public:

    //@{
    // Der von vector3t verwendete Skalartyp.
    // Geeignet sind vor allem float und double, aber auch Integers
    // können mit Einschränkungen verwendet werden.
    //@}
    typedef T scalar_type;

    scalar_type x, y, z;

    inline scalar_type& r() { return x; }
    inline scalar_type& g() { return y; }
    inline scalar_type& b() { return z; }
    inline const scalar_type& r() const { return x; }
    inline const scalar_type& g() const { return y; }
    inline const scalar_type& b() const { return z; }


    //@{
    // @returns Die Anzahl der Dimensionen von vector3t (3).
    //@}
    static inline long dimensions() { return 3; }

    //- structors
    //@{
    // Defaultkonstruktor.
    // Initialisiert alle Komponenten mit 0.
    //@}
    vector3t()                                                                       : x( 0 ),   y( 0 ),   z( 0 )   {}
    //@{
    // Konstruktion eines vector3t mit 3 Skalaren.
    // @param sx der x-wert
    // @param sy der y-wert
    // @param sz der z-wert
    //@}
    vector3t( const scalar_type sx, const scalar_type sy, const scalar_type sz )  : x( sx ),  y( sy ),  z( sz )  {}
    //@{
    // Kopierkonstruktor.
    //@}
    vector3t( const vector3t& v )                                                    : x( v.x ), y( v.y ), z( v.z ) {}
    //- destructor not needed

#ifdef __D3DX8MATH_H__

    //@{
    // Konstruktion eines vector3t aus einem D3DXVECTOR3.
    //@}
    vector3t( const D3DXVECTOR3& v ) : x( v.x ), y( v.y ), z( v.z ) {}

    //@{
    // Casting zum D3DXVECTOR3.
    //
    // @returns Einen entsprechenden D3DXVECTOR3.
    //@}
    operator D3DXVECTOR3() const { return D3DXVECTOR3( x, y, z ); }

#endif //__D3DX8MATH_H__

    //- set/reset
    //@{
    // Setzen aller Komponenten auf einen Wert.
    // Setzt alle Komponenten auf denselben Wert s.
    // @param s der Skalar
    // @returns Referenz auf den Vektor
    //@}
    vector3t& set(         const scalar_type  s ) { x = y = z = s; return *this; }

    //- set/reset
    //@{
    // Setzen aller Komponenten auf einen Wert.
    // Setzt alle Komponenten auf denselben Wert s.
    // @param s der Skalar
    // @returns Referenz auf den Vektor
    //@}
    vector3t& set( const vector3t& s ) 
    {
      x = s.x;
      y = s.y;
      z = s.y; 
      return *this;
    }

    //@{
    // Setzen aller Komponenten auf unterschiedliche Werte.
    // Setzt alle Komponenten auf die angegebenen Werte.
    // @param sx der x-Wert.
    // @param sy der y-Wert.
    // @param sz der z-Wert.
    // @returns Referenz auf den Vektor
    //@}
	  vector3t& set( const scalar_type& sx, const scalar_type& sy, const scalar_type& sz ) { x = sx; y = sy;  z = sz; return *this; }

    vector3t& offset( const scalar_type& DX, const scalar_type& DY, const scalar_type& DZ )
    {
      x += DX;
      y += DY;
      z += DZ;
      return *this;
    }

    //@{
    // Setzen aller Komponenten auf 0.
    // Setzt alle Komponenten auf denselben Wert 0.
    // @returns Referenz auf den Vektor
    //@}
    vector3t& clear()                             { x = y = z = 0; return *this; }



    bool empty() const
    {
      return ( ( x == y )
        &&     ( y == z ) );
    }



    //- assignment
    //@{
    // Zuweisungsoperator.
    //
    // @returns Referenz auf den Vektor
    //@}
	  vector3t& operator = ( const vector3t&    v ) { if ( this != &v ) { x = v.x; y = v.y;  z = v.z; } return *this; }
    //- assignment with scalar
    //@{
    // Zuweisungsoperator von Skalar.
    // Setzt alle Komponenten auf denselben Wert s.
    //
    // @returns Referenz auf den Vektor
    //@}
    vector3t& operator = ( const scalar_type  s ) { x = y = z = s;   return *this; }
    //- more assignments
	  vector3t& operator +=( const vector3t&    v ) { x += v.x; y += v.y; z += v.z; return *this; }
	  vector3t& operator -=( const vector3t&    v ) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	  vector3t& operator +=( const scalar_type  s ) { x += s;   y += s;   z += s;   return *this; }
	  vector3t& operator -=( const scalar_type  s ) { x -= s;   y -= s;   z -= s;   return *this; }
	  vector3t& operator /=( const scalar_type  s ) { x /= s;   y /= s;   z /= s;   return *this; }
	  vector3t& operator *=( const scalar_type  s ) { x *= s;   y *= s;   z *= s;   return *this; }
	  vector3t& scale      ( const scalar_type  s ) { return (*this) *= s;                        }

    //- access components (danger!)
    scalar_type& operator[]( const int i ) { return *( (&x) + i ); }
    const scalar_type& operator[]( const int i ) const { return *( (&x) + i ); }

    //- randomize
	  vector3t& randomize( const vector3t& v ) {
      x = ( rand() % 10000 ) * v.x / scalar_type(10000);
      y = ( rand() % 10000 ) * v.y / scalar_type(10000);
      z = ( rand() % 10000 ) * v.z / scalar_type(10000);
      return *this;
    }
	  vector3t& randomize( const vector3t& vmin, const vector3t& vmax ) {
      x = ( rand() % 10000 ) * ( vmax.x - vmin.x ) / scalar_type(10000) + vmin.x;
      y = ( rand() % 10000 ) * ( vmax.y - vmin.y ) / scalar_type(10000) + vmin.y;
      z = ( rand() % 10000 ) * ( vmax.z - vmin.z ) / scalar_type(10000) + vmin.z;
      return *this;
    }
    vector3t& unit_randomize() { //testen!
      do {
        x = ( rand() % 10000 ) / scalar_type(5000) - 1;
        y = ( rand() % 10000 ) / scalar_type(5000) - 1;
        z = ( rand() % 10000 ) / scalar_type(5000) - 1;
      } while( length_squared() > scalar_type(1) );
      return *this;
    }

// ////////////////////////////////////////////////
// //-    bald ersetzen durch externe Matrix-Operationen
// ////////////////////////////////////////////////
    //@{
    // Rotieren um die x-Achse.
    // Ist ineffizient wegen internen Gebrauchs von sin() und cos().
    // Besser mit Rotationsmatrizen arbeiten!
    // @param winkel Rotationswinkel in Altgrad
    // @returns Referenz auf den Vektor
    // @todo Rotationsmatrizen besser unterstützen und diese Methode verwerfen!
    //@}
    vector3t& RotateX( const T winkel )
    { vector3t vTemp( *this );
      double w( winkel * 3.1415926535 / 180 );
      y = T( vTemp.y * cos( w ) - vTemp.z * sin( w ) );
      z = T( vTemp.y * sin( w ) + vTemp.z * cos( w ) );
      return *this; }
    //@{
    // Rotieren um die y-Achse.
    // Ist ineffizient wegen internen Gebrauchs von sin() und cos().
    // Besser mit Rotationsmatrizen arbeiten!
    // @param winkel Rotationswinkel in Altgrad
    // @returns Referenz auf den Vektor
    // @todo Rotationsmatrizen besser unterstützen und diese Methode verwerfen!
    //@}
    vector3t& RotateY( const T winkel )
    { vector3t vTemp( *this );
      double w( winkel * 3.1415926535 / 180 );
      x = T( vTemp.x * cos( w ) - vTemp.z * sin( w ) );
      z = T( vTemp.x * sin( w ) + vTemp.z * cos( w ) );
      return *this; }
    //@{
    // Rotieren um die z-Achse.
    // Ist ineffizient wegen internen Gebrauchs von sin() und cos().
    // Besser mit Rotationsmatrizen arbeiten!
    // @param winkel Rotationswinkel in Altgrad
    // @returns Referenz auf den Vektor
    // @todo Rotationsmatrizen besser unterstützen und diese Methode verwerfen!
    //@}
    vector3t& RotateZ( const T winkel )
    { vector3t vTemp( *this );
      double w( winkel * double( 3.1415926535 / 180 ) );
      x = T( vTemp.x * cos( w ) - vTemp.y * sin( w ) );
      y = T( vTemp.x * sin( w ) + vTemp.y * cos( w ) );
      return *this; }
// ////////////////////////////////////////////////
// //-    bald ersetzen durch Matrix-Operationen Ende
// ////////////////////////////////////////////////

    vector3t& flip_x()    { x = -x; return *this; } //@- Invertieren der x-Komponente.
    vector3t& flip_y()    { y = -y; return *this; } //@- Invertieren der y-Komponente.
    vector3t& flip_z()    { z = -z; return *this; } //@- Invertieren der z-Komponente.

    vector3t& normalize()
    {
      if ( length() <= 0.0f )
      {
        return *this;
      }
      return (*this) /= length();
    }

    vector3t normalized() const
    {
      if ( length() <= 0.0f )
      {
        return vector3t( *this );
      }
      return vector3t( *this ) / length();
    }

    ////////////////////////////////////////////////
    //- const-functions
    ////////////////////////////////////////////////
    vector3t operator +( const scalar_type s )  const { return vector3t(  x + s,   y + s,   z + s    ); }
    vector3t operator -( const scalar_type s )  const { return vector3t(  x - s,   y - s,   z - s    ); }
    vector3t operator *( const scalar_type s )  const { return vector3t(  x * s,   y * s,   z * s    ); }
    vector3t operator /( const scalar_type s )  const { return vector3t(  x / s,   y / s,   z / s    ); }
    vector3t operator +( const vector3t&   v )  const { return vector3t(  x + v.x, y + v.y, z + v.z  ); }
    vector3t operator -( const vector3t&   v )  const { return vector3t(  x - v.x, y - v.y, z - v.z  ); }
    vector3t operator -()                       const { return vector3t(  -x,      -y,      -z       ); }

    //- length and magnitude are the same
    scalar_type length()                        const { return scalar_type( sqrt( length_squared() ) ); }
    scalar_type length_squared()                const { return x * x + y * y + z * z; }
    scalar_type magnitude()                     const { return length(); }
    scalar_type magnitude_squared()             const { return length_squared(); }
    scalar_type distance_squared( const vector3t& rhs ) const { return ( rhs - *this ).length_squared(); } //@- Distanz zu einem anderen Punkt.

    //- comparisons
    bool operator==(     const vector3t&   v )  const { return  x == v.x && y == v.y && z == v.z; }
    bool operator!=(     const vector3t&   v )  const { return  !operator==( v ); }
    bool operator<(     const vector3t&   v )  const
    {
      if ( x < v.x )
      {
        return true;
      }
      else if ( x > v.x )
      {
        return false;
      }
      if ( y < v.y )
      {
        return true;
      }
      else if ( y > v.y )
      {
        return false;
      }
      return z < v.z;
    }

// ////////////////////////////////////////////////
// //-    bald ersetzen durch externe Matrix-Operationen
// ////////////////////////////////////////////////
    //- rotieren (waren: RotateXYZ())
    //@{
    // Rotieren um die x-Achse.
    // Ist ineffizient wegen internen Gebrauchs von sin() und cos().
    // Besser mit Rotationsmatrizen arbeiten!
    // @param winkel Rotationswinkel in Altgrad
    // @returns Einen rotierten Vektor
    // @todo Rotationsmatrizen besser unterstützen und diese Methode verwerfen!
    //@}
    vector3t RotatedX( const T winkel ) const
    { vector3t vTemp( *this );
      double w = winkel * 3.1415926535 / 180;
      vTemp.y = T( y * cos( w ) - z * sin( w ) );
      vTemp.z = T( y * sin( w ) + z * cos( w ) );
      return vTemp; }
    //@{
    // Rotieren um die y-Achse.
    // Ist ineffizient wegen internen Gebrauchs von sin() und cos().
    // Besser mit Rotationsmatrizen arbeiten!
    // @param winkel Rotationswinkel in Altgrad
    // @returns Einen rotierten Vektor
    // @todo Rotationsmatrizen besser unterstützen und diese Methode verwerfen!
    //@}
    vector3t RotatedY( const T winkel ) const
    { vector3t vTemp( *this );
      double w( winkel * 3.1415926535 / 180 );
      vTemp.x = T( x * cos( w ) - z * sin( w ) );
      vTemp.z = T( x * sin( w ) + z * cos( w ) );
      return vTemp; }
    //@{
    // Rotieren um die z-Achse.
    // Ist ineffizient wegen internen Gebrauchs von sin() und cos().
    // Besser mit Rotationsmatrizen arbeiten!
    // @param winkel Rotationswinkel in Altgrad
    // @returns Einen rotierten Vektor
    // @todo Rotationsmatrizen besser unterstützen und diese Methode verwerfen!
    //@}
    vector3t RotatedZ( const T winkel ) const
    { vector3t vTemp( *this );
      double w( winkel * 3.1415926535 / 180 );
      vTemp.x = T( x * cos( w ) - y * sin( w ) );
      vTemp.y = T( x * sin( w ) + y * cos( w ) );
      return vTemp; }
// ////////////////////////////////////////////////
// //-    bald ersetzen durch Matrix-Operationen Ende
// ////////////////////////////////////////////////

    //- mirror
    vector3t flipped_x() const { return vector3t( -x,  y,  z ); } //@- Vektor mit invertierter x-Komponente.
    vector3t flipped_y() const { return vector3t(  x, -y,  z ); } //@- Vektor mit invertierter y-Komponente.
    vector3t flipped_z() const { return vector3t(  x,  y, -z ); } //@- Vektor mit invertierter z-Komponente.

    vector3t    unit() const { return vector3t( *this ).normalize(); } //@- normalisierter Vektor.

    vector3t    cross( const vector3t& v ) const
    {
      return vector3t( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
    }

    scalar_type dot( const vector3t& v ) const { return x * v.x + y * v.y + z * v.z; } //@- Punkt- oder Skalarprodukt.

    //- scale
    friend inline vector3t operator*( const scalar_type& s, const vector3t& v ) { return vector3t( v.x * s, v.y * s, v.z * s ); } //@- skalierter Vektor.

    //- normal of a triangle, given by 3 points (clockwise) (is it correct?)
    friend inline vector3t normal( const vector3t& v1, const vector3t& v2, const vector3t& v3 )
    {
      return ( v2 - v1 ).cross( v3 - v1 ).normalize();
    }

    //- angle (radiant) between 2 vectors.
    scalar_type angle( const vector3t& v2 )
    {
      return (scalar_type)acosf( unit().dot( v2.unit() ) ) * 180.0f / 3.1415926f;
    }

    // //- angle (degree) between 2 vectors.
    // friend inline T angle_degree( const vector3t& v1, const vector3t& v2 ) { return 180 * angle( v1, v2 ) / 3.1415926535; } //@- angle (degree) between 2 vectors.


    // //- aberration (abweichung der winkel) between 2 vectors.
    // friend inline T aberration( const vector3t& v1, const vector3t& v2 ) { return 1 - (T)v1.unit().dot( v2.unit() ); } //@- aberration between 2 vectors.



    /////////////////////////////////////////////////////////////////////
    //-     neu:
    /////////////////////////////////////////////////////////////////////

    scalar_type distance( const vector3t& rhs ) const { return ( rhs - *this ).length(); } //@- Distanz zu einem anderen Punkt.

    //@{
    // Vektorbetrag annähern.
    // Scheint einen Fehler zu haben.
    // @returns Länge des Vektors
    // @todo Testen und reparieren!
    //@}
    scalar_type approximate_length() const { //testen!
      //- könnte eventuell noch optimiert werden - kein casting?
      scalar_type a = ( x < 0 )? x : -x;
      scalar_type b = ( y < 0 )? y : -y;
      scalar_type c = ( z < 0 )? z : -z;
      if( a < b ) std::swap( a, b );
      if( a < c ) std::swap( a, c );
      return a * scalar_type( 0.9375 ) + ( b + c ) * scalar_type( 0.375 );
    }

    //@{
    // Abstand zu einem anderen Punkt annähern.
    // Scheint einen Fehler zu haben.
    // @param rhs Der andere Punkt.
    // @returns Distanz zu rhs.
    // @todo Testen und reparieren!
    //@}
    scalar_type approximate_distance( const vector3t& rhs ) const { //testen!
      return vector3t( *this - rhs ).approximate_length();
    }

    //@{
    // Interpolierung mit anderem Vektor.
    // @param rhs Der andere Vektor.
    // @param s Der Interpolationsfaktor (normalerweise zwischen 0 und 1).
    // @returns Referenz auf den Vektor.
    //@}
	  vector3t& interpolate( const vector3t& rhs, const scalar_type& s ) {  //testen!
      x += s * ( rhs.x - x );
      y += s * ( rhs.y - y );
      z += s * ( rhs.z - z );
      return *this;
    }

    //@{
    // Interpolierung mit anderem Vektor.
    // @param rhs Der andere Vektor.
    // @param s Der Interpolationsfaktor (normalerweise zwischen 0 und 1).
    // @returns Einen interpolierten Vektor.
    //@}
	  vector3t interpolated( const vector3t& rhs, const scalar_type& s ) {  //testen!
      return vector3t(  x + s * ( rhs.x - x ),
                        y + s * ( rhs.y - y ),
                        z + s * ( rhs.z - z ) );
    }

    //@{
    // Interpolierung mit zwischen zwei anderen Vektoren.
    // @param v1 Der andere Vektor.
    // @param v2 Der andere Vektor.
    // @param s Der Interpolationsfaktor (normalerweise zwischen 0 und 1).
    // @returns Referenz auf den Vektor.
    //@}
	  vector3t& interpolate( const vector3t& v1, const vector3t& v2, const scalar_type& s ) { //testen!
      x = v1.x + s * ( v2.x - v1.x );
      y = v1.y + s * ( v2.y - v1.y );
      z = v1.z + s * ( v2.z - v1.z );
      return *this;
    }

     //testen!
    vector3t& approximate_normalize()  { return *this /= approximate_length(); }
    vector3t  approximate_unit() const { return *this /  approximate_length(); }

    vector3t& truncate( const scalar_type& maximum )  //testen!
    {
      scalar_type l = length();
      if ( l > maximum ) *this *= maximum / l;
      return *this;
    }

    vector3t& approximate_truncate( const scalar_type& maximum )  //testen!
    {
      scalar_type l = approximate_length();
      if ( l > maximum ) *this *= maximum / l;
      return *this;
    }
  };

  //- oft benötigt: 3D-Vektoren mit floats
  typedef vector3t< float > vector3;


  ////////////////////////////////////////////////
  //-    matrix3t (3x3), for matrix computations
  ////////////////////////////////////////////////
  // ================================================
  //@{
  // 3D-Matrixklasse.
  // matrix3t ist ein Klassentemplate für 3D-Matrizen zur Zusammenarbeit mit vector3t.
  // @author Peter Jerz
  // @version 1.0
  //@}
  // ================================================
  template <class T>
  class matrix3t
  {
  public:

    //@{
    // Der von basis3t verwendete Skalartyp.
    // Geeignet sind vor allem float und double, aber auch Integers
    // können mit Einschränkungen verwendet werden.
    //@}
    typedef T                     scalar_type;

    //@{
    // Der von basis3t verwendete Vektortyp.
    //@}
    typedef vector3t<scalar_type> vector_type;

    vector_type C[3];

    matrix3t()  { C[0].x = 1.0; C[1].y = 1.0; C[2].z = 1.0; }
    matrix3t( const vector_type& c0, const vector_type& c1, const vector_type& c2 ) { C[0] = c0; C[1] = c1; C[2] = c2; }
    matrix3t( const matrix3t& rhs )
    {
      C[0]=rhs.C[0]; C[1]=rhs.C[1]; C[2]=rhs.C[2];
    }

    vector_type& operator[]( const long i ) { return C[i]; }

    bool operator==( const matrix3t& rhs ) const    { return C[0] == rhs.C[0] && C[1] == rhs.C[1] && C[2] == rhs.C[2]; }
    bool operator!=( const matrix3t& rhs ) const    { return !operator==( rhs ); }

    matrix3t& operator=(  const matrix3t&     rhs ) { C[0] =  rhs.C[0]; C[1] =  rhs.C[1]; C[2] =  rhs.C[2]; return *this; }
    matrix3t& operator+=( const matrix3t&     rhs ) { C[0] += rhs.C[0]; C[1] += rhs.C[1]; C[2] += rhs.C[2]; return *this; }
    matrix3t& operator-=( const matrix3t&     rhs ) { C[0] -= rhs.C[0]; C[1] -= rhs.C[1]; C[2] -= rhs.C[2]; return *this; }
    matrix3t& operator*=( const scalar_type&  rhs ) { C[0] *= rhs;      C[1] *= rhs;      C[2] *= rhs;      return *this; }
    matrix3t& operator*=( const matrix3t&     rhs )
    {
      matrix3t temp( *this );
      C[0] = temp * rhs.C[0];
      C[1] = temp * rhs.C[1];
      C[2] = temp * rhs.C[2];
      return *this;
    }

    //- addition/subtraction
    matrix3t operator+( const matrix3t& rhs ) const { return matrix3t( C[0] + rhs.C[0], C[1] + rhs.C[1], C[2] + rhs.C[2] ); }
    matrix3t operator-( const matrix3t& rhs ) const { return matrix3t( C[0] - rhs.C[0], C[1] - rhs.C[1], C[2] - rhs.C[2] ); }
    //- post-multiply with scalar
    matrix3t operator*( const scalar_type& rhs )        const { return matrix3t( C[0] * rhs, C[1] * rhs, C[2] *rhs ); }
    //- pre-multiply with scalar
    friend inline matrix3t operator*( const scalar_type& lhs, const matrix3t& rhs ) { return rhs * lhs; }
    //- post-multiply with vector
    vector_type operator*( const vector_type& rhs ) const
    { return C[0] * rhs.x + C[1] * rhs.y + C[2] * rhs.z; }
    //- pre-multiply with vector
    friend inline vector_type operator*( const vector_type& lhs, const matrix3t& rhs )
    { return vector_type( rhs.C[0].dot(lhs), rhs.C[1].dot(lhs), rhs.C[2].dot(lhs) ); }
    //- post-multiply with matrix
    matrix3t operator*( const matrix3t& rhs ) const
    { return matrix3t( (*this) * rhs.C[0], (*this) * rhs.C[1], (*this) * rhs.C[2] ); }
    //- transpose
    matrix3t transposed() const
    { return matrix3t( vector_type( C[0].x, C[1].x, C[2].x ),
                       vector_type( C[0].y, C[1].y, C[2].y ),
                       vector_type( C[0].z, C[1].z, C[2].z ) ); }
    //- determinant
    scalar_type determinant() const { return C[0].dot( C[1].cross( C[2] ) ); }

    //- dyadisches Produkt
    friend inline matrix3t tensor( const vector_type& lhs, const vector_type& rhs)
    {
      matrix3t t;
      t.C[0].x = lhs.x * rhs.x; t.C[1].x = lhs.x * rhs.y; t.C[2].x = lhs.x * rhs.z;
      t.C[0].y = lhs.y * rhs.x; t.C[1].y = lhs.y * rhs.y; t.C[2].y = lhs.y * rhs.z;
      t.C[0].z = lhs.z * rhs.x; t.C[1].z = lhs.z * rhs.y; t.C[2].z = lhs.z * rhs.z;
      return t;
    }

    matrix3t inversed() const
    {
      scalar_type det = determinant();
      assert( det != scalar_type(0) );
      det = 1/det; //- optimier...
      matrix3t result;
      result.C[0].x = ( C[1].y * C[2].z - C[1].z * C[2].y ) * det;
      result.C[1].x = ( C[1].z * C[2].x - C[1].x * C[2].z ) * det;
      result.C[2].x = ( C[1].x * C[2].y - C[1].y * C[2].x ) * det;
      result.C[0].y = ( C[0].z * C[2].y - C[0].y * C[2].z ) * det;
      result.C[1].y = ( C[0].x * C[2].z - C[0].z * C[2].x ) * det;
      result.C[2].y = ( C[0].y * C[2].x - C[0].x * C[2].y ) * det;
      result.C[0].z = ( C[0].y * C[1].z - C[0].z * C[1].y ) * det;
      result.C[1].z = ( C[0].z * C[1].x - C[0].x * C[1].z ) * det;
      result.C[2].z = ( C[0].x * C[1].y - C[0].y * C[1].x ) * det;
      return result;
    }


    matrix3t& identity()
    {
      *this = matrix3t();
      return *this;
    }

    // matrix3t& scale( const vector_type& rhs )
    // {
      // // *this = matrix3t();
      // return this;
    // }







    //- Zur Rotationsmatrix machen um beliebigen Vektor als Achse mit beliebigem
    //- Winkel
    //- entnommen von Crystal Space
    //- // I didn't write this code... I got it from an on-line tutorial.
    //- // not conviced it doesn't have discontinuity...
    //- // the un-simplified formula was really strange and they didn't really
    //- // explain it.  Kind of looked like it could have come from quaternion theory.
    // void R_from_vector_and_angle( ctVector3 pvec, real theta, ctMatrix3 &pR )
    // {
        // double  angleRad = theta,
// 	        c = cos(angleRad),
// 	        s = sin(angleRad),
// 	        t = 1.0 - c;

        // pvec = pvec/pvec.Norm();
        // pR.set(t * pvec[0] * pvec[0] + c,
// 		         t * pvec[0] * pvec[1] - s * pvec[2],
// 		         t * pvec[0] * pvec[2] + s * pvec[1],
// 		    t * pvec[0] * pvec[1] + s * pvec[2],
// 		         t * pvec[1] * pvec[1] + c,
// 		         t * pvec[1] * pvec[2] - s * pvec[0],
// 		    t * pvec[0] * pvec[1] - s * pvec[1],
// 		         t * pvec[1] * pvec[2] + s * pvec[0],
// 		         t * pvec[2] * pvec[2] + c);

    // }

    //- Hmm, funktioniert nur für Vektoren kollinear mit Hauptachsen :-( wo ist fehler?
    matrix3t& rotate( const vector_type& vAxis, const scalar_type& theta )
    {
      scalar_type angleRad = theta;
	    scalar_type COS = scalar_type( cos(angleRad) );
	    scalar_type SIN = scalar_type( sin(angleRad) );
	    scalar_type t   = scalar_type(1.0) - COS;
      vector_type axis( vAxis.unit() );
      C[0][0] = t * axis.x * axis.x + COS;
		  C[1][0] = t * axis.x * axis.y - SIN * axis.z;
		  C[2][0] = t * axis.x * axis.z + SIN * axis.y;
      C[0][1] = t * axis.x * axis.y + SIN * axis.z;
		  C[1][1] = t * axis.y * axis.y + COS;
		  C[2][1] = t * axis.y * axis.z - SIN * axis.x;
      C[0][2] = t * axis.x * axis.y - SIN * axis.y;
		  C[1][2] = t * axis.y * axis.z + SIN * axis.x;
		  C[2][2] = t * axis.z * axis.z + COS;
      return *this;
    }

  };


  ////////////////////////////////////////////////
  //-    basis3t (3x3), represents angular orientation in space
  ////////////////////////////////////////////////
  // ================================================
  //@{
  // 3D-Orientierungsklasse.
  // basis3t ist ein Klassentemplate zur Repräsentation einer
  // Orientierung im dreidimensionalen Raum
  // zur Zusammenarbeit mit vector3t.
  // basis3t speichert Informationen über die Ausrichtung
  // eines Körpers im dreidimensionalen Raum.
  // @author Peter Jerz
  // @version 1.0
  //@}
  // ================================================
  template <class T>
  class basis3t
  {
  public:

    //@{
    // Der von basis3t verwendete Skalartyp.
    // Geeignet sind vor allem float und double, aber auch Integers
    // können mit Einschränkungen verwendet werden.
    //@}
    typedef T                     scalar_type;

    //@{
    // Der von coordframe3t verwendete Vektortyp.
    //@}
    typedef vector3t<scalar_type> vector_type;

    //@{
    // Der von coordframe3t verwendete Matrixtyp.
    //@}
    typedef matrix3t<scalar_type> matrix_type;

    //@{
    // Orientierung der Basis relativ zur übergeordneten Basis.
    //@}
    matrix_type R;

    basis3t() {}
    basis3t( const vector_type& c0, const vector_type& c1, const vector_type& c2 ) : R( c0, c1, c2 ) {}
    basis3t( const matrix_type& rhs ) : R( rhs ) {}
    basis3t( const basis3t&     rhs ) : R( rhs.R ) {}

    vector_type& operator[]( const long i )  { return R.C[i]; }

    vector_type& x() { return R.C[0]; }
    vector_type& y() { return R.C[1]; }
    vector_type& z() { return R.C[2]; }
    matrix_type& basis() { return R; }
    void basis( const vector_type& c0, const vector_type& c1, const vector_type& c2 )
    { R[0] = c0;  R[1] = c1;  R[2] = c2; }

    //@{
    // Basis rechtshändig um die x-Achse rotieren.
    // @param a Rotationswinkel in Radiant.
    //@}
    void rotate_x( const scalar_type& a )
    { if ( 0 != a ) {
        vector_type b1(  y() * scalar_type(cos( a )) + z() * scalar_type(sin( a )) );
        vector_type b2( -y() * scalar_type(sin( a )) + z() * scalar_type(cos( a )) );
        R[1] = b1.unit();
        R[2] = b2.unit();
      }
    }

    //@{
    // Basis rechtshändig um die y-Achse rotieren.
    // @param a Rotationswinkel in Radiant.
    //@}
    void rotate_y( const scalar_type& a )
    { if ( 0 != a ) {
        vector_type b2(  z() * scalar_type(cos( a )) + x() * scalar_type(sin( a )) );
        vector_type b0( -z() * scalar_type(sin( a )) + x() * scalar_type(cos( a )) );
        R[2] = b2.unit();
        R[0] = b0.unit();
      }
    }

    //@{
    // Basis rechtshändig um die z-Achse rotieren.
    // @param a Rotationswinkel in Radiant.
    //@}
    void rotate_z( const scalar_type& a )
    { if ( 0 != a ) {
        vector_type b0(  x() * scalar_type(cos( a )) + y() * scalar_type(sin( a )) );
        vector_type b1( -x() * scalar_type(sin( a )) + y() * scalar_type(cos( a )) );
        R[0] = b0.unit();
        R[1] = b1.unit();
      }
    }

    //- rotate around the basis unit vector u by theta (radians)
    void rotate( const scalar_type& theta, const vector_type& u )
    {
      matrix_type temp;
      R *= temp.rotate( u, theta );
    }

    //- rotate, length of da is theta, unit direction of da is u
    void rotate( const vector_type& da );

    //@{
    // Eine Richtung lokalisieren.
    // @param v zu lokalisierende Richtung.
    // @returns lokalisierte Richtung.
    //@}
    vector_type localizeVector( const vector_type& v ) const
    { return vector_type( R.C[0].dot( v ), R.C[1].dot( v ), R.C[2].dot( v ) ); }

    //@{
    // Eine Richtung globalisieren.
    // @param v zu globalisierende Richtung.
    // @returns globalisierte Richtung.
    //@}
    vector_type globalizeVector( const vector_type& v ) const
    { return R.C[0] * v.x + R.C[1] * v.y + R.C[2] * v.z; }

  };


  ////////////////////////////////////////////////
  //-    coordframe3t (3x3), represents angular and translational information
  ////////////////////////////////////////////////
  // ================================================
  //@{
  // 3D-Bezugssystem.

  // coordframe3t ist ein Klassentemplate zur Repräsentation eines
  // Bezugssystems im dreidimensionalen Raum
  // zur Zusammenarbeit mit vector3t.
  // coordframe3t speichert Informationen über die Ausrichtung und
  // Position eines Körpers im dreidimensionalen Raum.
  // @author Peter Jerz
  // @version 1.0
  //@}
  // ================================================
  template <class T>
  class coordframe3t : public basis3t<T>
  {
  public:
    //@{
    // Der von coordframe3t verwendete Skalartyp.
    // Geeignet sind vor allem float und double, aber auch Integers
    // können mit Einschränkungen verwendet werden.
    //@}
    typedef T                      scalar_type;
    //@{
    // Der von coordframe3t verwendete Vektortyp.
    //@}
    typedef vector3t<scalar_type>  vector_type;
    //@{
    // Der von coordframe3t verwendete Basistyp.
    //@}
    typedef basis3t<scalar_type>   basis_type;

    //@{
    // Ursprung des Bezugssystem relativ zum übergeordneten System.
    //@}
    vector_type O;

    coordframe3t() {}
    coordframe3t(  const vector_type& o,
                   const vector_type& c0,
                   const vector_type& c1,
                   const vector_type& c2 ) : O( o ), basis3t<T>( c0, c1, c2 ) {}
    coordframe3t(  const vector_type& o, const basis_type& b ) : O( o ), basis3t<T>( b ) {}

    const vector_type& position() const                 { return O; }
    void               position( const vector_type& p ) { O = p; }

    //@{
    // Einen Punkt lokalisieren.
    // @param p zu lokalisierender Punkt.
    // @returns lokalisierten Punkt.
    //@}
    vector_type localizePoint(  const vector_type& p ) const { return localizeVector( p - O ); }

    //@{
    // Einen Punkt globalisieren.
    // @param p zu globalisierender Punkt.
    // @returns globalisierten Punkt.
    //@}
    vector_type globalizePoint( const vector_type& p ) const { return globalizeVector( p ) + O; }

    //@{
    // Bezugssystem verschieben.
    // @param d Differenz, um die verschoben werden soll..
    //@}
    void translate( const vector_type& d ) { O += d; }

  };

}

#endif //_PJ_vector3t_h
