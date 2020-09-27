#ifndef _PJ_d3_bodystate3t_h
#define _PJ_d3_bodystate3t_h

#pragma warning ( disable : 4786 )

#include <math\vector3t.h>




//- hinweis von pj:
//-  habe ich am 8.2.2003 von cyberaffinity übernommen,
//- um 3d-sachen voranzutreiben. noch baustelle!







// #include <iostream>

// #pragma warning ( disable : 4786 )

namespace d3
{
  ////////////////////////////////////////////////
  //-    bodystate3t, represents the state of a rigid body 
  ////////////////////////////////////////////////
  template <typename T>
  class bodystate3t
  {
  public:
    //@{                             
    // Der von bodystate3t verwendete Skalartyp.
    // Geeignet sind vor allem float und double, aber auch Integers
    // können mit Einschränkungen verwendet werden.    
    //@}                             
    typedef T                     scalar_type;

    //@{                             
    // Der von bodystate3t verwendete Vektortyp.
    //@}                             
    typedef math::vector3t<scalar_type> vector_type;

    //@{                             
    // Der von bodystate3t verwendete Matrixtyp.
    //@}                             
    typedef math::matrix3t<    scalar_type> matrix_type;
    typedef math::coordframe3t<scalar_type> frame_type;


    frame_type coordframe;
    
    //- props:
    
    vector_type V; //- linear  velocity
    vector_type W; //- angular velocity

    matrix_type I;     //-         inertia tensor in World space, kg m m
    matrix_type I_inv; //- inverse inertia tensor in World space, kg m m

     //- sekundär
    vector_type F; //- temp, akkumulierte lineare forces auf center of gravity
    vector_type T; //- temp, torque, akkumulierte forces auf rotation
    

    //- methods

    CBodyState() {}
    CBodyState(  const vector_type& v, const vector_type& w  ) : V( v ), W( w ) {}
  
    // const CVector& velocity() const                 { return V; }
    // void               velocity( const CVector& v ) { V = v; }
  
    // const CVector& angularVelocity() const                 { return W; }
    // void               angularVelocity( const CVector& w ) { W = w; }
  
    // const CMatrix& inertiaTensor() const                 { return I; }
    // // void               inertiaTensor( const TCMatrix<T>& i ) { I = i; }
    // const CMatrix& inverseInertiaTensor() const                 { return I_inv; }
    // void           calculateInertiaTensor( const CVector& v ) ;
  
    // //- Ausgabe
    // friend inline std::ostream& operator <<( std::ostream& o, const CBodyState& v ); 

    // //- Eingabe
    // friend inline std::istream& operator >>( std::istream& i, CBodyState& v ); 
  };

  // //- Ausgabe
  // inline std::ostream& operator <<( std::ostream& o, const CBodyState& v ) 
  // { 
    // o << v.CoordFrame           << "\n\n"; 
    // o << v.V                    << "\n\n";
    // o << v.W                    << "\n\n"; 
    // o << v.F << "\n\n";
    // o << v.T << "\n\n";
    // o << v.I                    << "\n\n";
    // o << v.I_inv                << "\n\n"; 
    // return o; 
  // }

  // //- Eingabe
  // inline std::istream& operator >>( std::istream& i, CBodyState& v ) 
  // { 
    // i >>  v.CoordFrame  
      // >>  v.V 
      // >>  v.W  
      // >>  v.F 
      // >>  v.T 
      // >>  v.I
      // >>  v.I_inv; 
    // return i; 
  // }
}

#endif //_PJ_d3_bodystate3t_h