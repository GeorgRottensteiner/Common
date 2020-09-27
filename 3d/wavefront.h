#ifndef _wavefront_h
#define _wavefront_h

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <anim/interpolate.h>
#include <GR/GRTypes.h>
#include <pjlib/string/strtok.h>


#include<debug/debugclient.h>


//- (ruhig klotzen! wird nachher alles überflüssige abgesägt)
#ifndef WF_LOADBUFFERSIZE
#define WF_LOADBUFFERSIZE (20000)
#endif//WF_LOADBUFFERSIZE
#ifndef WF_LINEBUFFERSIZE
#define WF_LINEBUFFERSIZE (32000)
#endif//WF_LINEBUFFERSIZE

class CWF
{
public:
  typedef math::vector3t<float> vertex_t;
  typedef std::vector< vertex_t >  vertex_collection_t;

  //- vertices
  vertex_collection_t v_;   //- x,y,z used
  //- texture vertices
  vertex_collection_t vt_;  //- x,y used, z meist nicht
  //- normal vertices
  vertex_collection_t vn_;  //- x,y,z used

  typedef int ref_t;
  struct facenode_t
  {
    ref_t v_, vt_, vn_;
    facenode_t() : v_( ref_t() ), vt_( ref_t() ), vn_( ref_t() ) {}
    facenode_t( ref_t v, ref_t vt, ref_t vn ) : v_( v ), vt_( vt ), vn_( vn ) {}
  };
  typedef std::vector< facenode_t > face_t;
  typedef std::vector< face_t >     face_collection_t;

  //- faces
  face_collection_t f_;

  //- groups werden zunächst ignoriert


  //- fab four
  CWF() {}
  ~CWF() {}
  CWF( const CWF& rhs ) : v_( rhs.v_ ), vt_( rhs.vt_ ), vn_( rhs.vn_ ), f_( rhs.f_ ) {}
  CWF& operator=( const CWF& rhs )
  {
    if ( this == &rhs ) return *this;
    clear();
    v_  = rhs.v_;
    vt_ = rhs.vt_;
    vn_ = rhs.vn_;
    f_  = rhs.f_;
    return *this;
  }

  void clear()
  {
    v_.clear();
    vt_.clear();
    vn_.clear();
    f_.clear();
  }

  void save( std::ostream& os ) const
  {
    //profile p( "CWF::save( std::ostream& os ) const" );

    {
      for ( int i = 1; i < v_.size(); ++i )
        os << "v " << v_[i].x << " " << v_[i].y << " " << v_[i].z << "\n";
    }

    {
      for ( int i = 1; i < vt_.size(); ++i )
        os << "vt " << vt_[i].x << " " << vt_[i].y /* << " " << vt_[i].z */ << "\n";
    }

    {
      for ( int i = 1; i < vn_.size(); ++i )
        os << "vn " << vn_[i].x << " " << vn_[i].y << " " << vn_[i].z << "\n";
    }

    {
      for ( int i = 0; i < f_.size(); ++i )
      {
        const face_t& face = f_[i];
        os << "f";
        for ( int j = 0; j < face.size(); ++j )
        {
          os << " ";
          os << face[j].v_;
          os << "/";
          if ( face[j].vt_ != 0 ) os << face[j].vt_;
          os << "/";
          if ( face[j].vn_ != 0 ) os << face[j].vn_;
        }
        os << "\n";
      }
    }

  }

  void load( std::istream& is )
  {
    // profile p1( "CWF::load( std::istream& is )" );
    using GR::String;
    clear();

    //- damit es schneller geht (ruhig klotzen! wird nachher alles überflüssige abgesägt)
    {
      v_.reserve(  WF_LOADBUFFERSIZE );
      vt_.reserve( WF_LOADBUFFERSIZE );
      vn_.reserve( WF_LOADBUFFERSIZE );
      f_.reserve(  WF_LOADBUFFERSIZE );
    }

    //- index 0 initialisieren (wavefront zählt ab 1)
    v_.push_back( vertex_t() );
    vt_.push_back( vertex_t() );
    vn_.push_back( vertex_t() );

    char  buf[WF_LINEBUFFERSIZE];
    char* p = 0;
    string str;


    char delimiter = '\n';

    while ( is )
    {
      is.getline( buf, WF_LINEBUFFERSIZE, delimiter );
      p = buf;

      switch ( *p )
      {
      case '\0': goto nextline; //- leerzeile

      case 'v': //- jetzt kommt irgendein vertex
        {
          ++p;
          if ( !*p ) break;

          vertex_collection_t* vc = 0;
          vertex_t vertex;

          switch( *p )
          {
          case ' ': //- normaler vertex
            vc = &v_;
            sscanf( p, " %e %e %e", &vertex.x, &vertex.y, &vertex.z );
            break;

          case 't': //- texturvertex
            vc = &vt_;
            sscanf( p, "t %e %e", &vertex.x, &vertex.y );
            break;

          case 'n': //- normalenvertex
            vc = &vn_;
            sscanf( p, "n %e %e %e", &vertex.x, &vertex.y, &vertex.z );
            break;

          default: goto nextline;
          }

          vc->push_back( vertex );
        }
        break;

      case 'f': //- jetzt kommt ein polygon
        {
          ++p;
          if ( *p != ' ' ) break; //- auf f folgte kein space - unbekannt

          ++p;

          str.assign( p );

          StringTok<string> tok( str ); //- könnte langsam sein (wenn alles klappt, optimieren!)
          face_t face;

          //- folgende verweise auf vertices muessen bemängelt werden:
          //- indizes < 1 und indizes > anzahl (je nach typ - v, vt oder vn)
          //- ausserdem muessen Faces mindestens drei punkte haben
          string strPoint;

          while(  ( strPoint = tok( " \t\0" ) ) != "" )
          {
            facenode_t fn;
            sscanf( strPoint.c_str(), "%d/%d/%d", &fn.v_, &fn.vt_, &fn.vn_ );
            face.push_back( fn );
          }

          f_.push_back( face );
        }
        break;

      case 'g': //- eine gruppe
      default:  //- alles andere ignorieren
        break;
      }

      nextline: ;
    }

    //- dies kopiert einmal alle vektoren, danach
    //- sind jedoch unnötig reservierte plätze freigegeben
    v_.swap(  vertex_collection_t( v_.begin(),  v_.end()  ) );
    vt_.swap( vertex_collection_t( vt_.begin(), vt_.end() ) );
    vn_.swap( vertex_collection_t( vn_.begin(), vn_.end() ) );
    f_.swap(  face_collection_t(   f_.begin(),  f_.end()  ) );
  }

  void interpolate( const CWF& a, const CWF& b, float morph )
  {
    //- dies verwendet nicht die effizienteste version von interpolate, ist aber sicherer
    anim::interpolate( v_.begin(),  v_.end(),  a.v_.begin(),  a.v_.end(),  b.v_.begin(),  b.v_.end(),  morph );
    anim::interpolate( vn_.begin(), vn_.end(), a.vn_.begin(), a.vn_.end(), b.vn_.begin(), b.vn_.end(), morph );
  }

  struct vertex_sort
  {
    bool operator()( const vertex_t& lhs, const vertex_t& rhs ) const
    {
      if ( lhs.x < rhs.x ) return true;
      if ( lhs.x > rhs.x ) return false;
      if ( lhs.y < rhs.y ) return true;
      if ( lhs.y > rhs.y ) return false;
      if ( lhs.z < rhs.z ) return true;
      return false;
    }
  };

  //- mässig nützliche funktion, da nicht gewährleistet ist, dass
  //- zwei optimierte wfs nach der optimierung noch interpolierbar sind
  void optimize()
  {
    // profile p1( "CWF::optimize()" );
    /*
    dh() << "vor optimize: \n"
      << dh::notify
      << " v: "  << v_.size()  - 1 << "\n"
      << " vt: " << vt_.size() - 1 << "\n"
      << " vn: " << vn_.size() - 1 << "\n"
      << " f: "  << f_.size()      << "\n";
      */

    typedef std::map < vertex_t, ref_t, vertex_sort > vertex_ref_collection_t;

    vertex_ref_collection_t v_map, vt_map, vn_map;

    vertex_collection_t v_new;
    vertex_collection_t vt_new;
    vertex_collection_t vn_new;
    v_new .reserve( v_.size()  );
    vt_new.reserve( vt_.size() );
    vn_new.reserve( vn_.size() );
    v_new .push_back( vertex_t() );
    vt_new.push_back( vertex_t() );
    vn_new.push_back( vertex_t() );

    {
      for ( int i = 0; i < f_.size(); ++i )
      {
        face_t& face = f_[i];
        for ( int j = 0; j < face.size(); ++j )
        {
          facenode_t& fn = face[j];

          if ( fn.v_ )
          {
            vertex_t& v = v_[ fn.v_ ];
            vertex_ref_collection_t::iterator it = v_map.find( v );
            if ( it == v_map.end() )
            {
              fn.v_ = v_map[v] = v_new.size();
              v_new.push_back( v );
            }
            else
            {
              fn.v_ = it->second;
            }
          }

          if ( fn.vt_ )
          {
            vertex_t& v = vt_[ fn.vt_ ];
            vertex_ref_collection_t::iterator it = vt_map.find( v );
            if ( it == vt_map.end() )
            {
              fn.vt_ = vt_map[v] = vt_new.size();
              vt_new.push_back( v );
            }
            else
            {
              fn.vt_ = it->second;
            }
          }

          if ( fn.vn_ )
          {
            vertex_t& v = vn_[ fn.vn_ ];
            vertex_ref_collection_t::iterator it = vn_map.find( v );
            if ( it == vn_map.end() )
            {
              fn.vn_ = vn_map[v] = vn_new.size();
              vn_new.push_back( v );
            }
            else
            {
              fn.vn_ = it->second;
            }
          }
        }
      }
    }

    //- man könnte auch noch freigewordenen speicher freigeben
    v_.swap(  v_new  );
    vt_.swap( vt_new );
    vn_.swap( vn_new );

    // dh() << "nach optimize: \n"
      // << dh::notify
      // << " v: "  << v_.size()  - 1 << "\n"
      // << " vt: " << vt_.size() - 1 << "\n"
      // << " vn: " << vn_.size() - 1 << "\n"
      // << " f: "  << f_.size()      << "\n";
  }



    vertex_t xxnormal( const vertex_t& v1, const vertex_t& v2, const vertex_t& v3 )
    {
      return ( v2 - v1 ).cross( v3 - v1 ).normalize();
    }

  void calculate_face_normals()
  {
    //- normalenvector leeren
    vn_.clear();
    vn_.reserve( WF_LOADBUFFERSIZE );
    vn_.push_back( vertex_t() );

    //- alle faces durchgehen
    int p = 0;
    face_collection_t::iterator itf( f_.begin() ), itfend( f_.end() );
    for ( ; itf != itfend; ++itf )
    {
      face_t& f = *itf;
      if ( f.size() < 3 ) continue;

      //- normale berechnen (faces mit mehr als 3 ecken werden als trianglefans behandelt
      vertex_t normal;

      for ( p = 0; p < f.size() - 2; ++p )
      {
        normal += xxnormal( (math::vector3)v_[ f[0  ].v_ ],
                              (math::vector3)v_[ f[p+1].v_ ],
                              (math::vector3)v_[ f[p+2].v_ ] );
      }
      normal /= ( f.size() - 2 );

      //- normale in normalenvector aufnehmen
      vn_.push_back( normal );

      //- normalenindex in aktuellen flächenvertices setzen
      int index  = vn_.size() - 1;
      for ( p = 0; p < f.size(); ++p )
        f[p].vn_ = index;
    }

    //- überflüssigen speicher freigeben
    vn_.swap( vertex_collection_t( vn_.begin(), vn_.end() ) );
  }


  void flip_normals()
  {
    vertex_collection_t::iterator itf( vn_.begin() ), itfend( vn_.end() );
    for ( ; itf != itfend; ++itf )
      (*itf) *= -1.0f;
  }








};

inline std::istream& operator>>( std::istream& is, CWF& wf )
{
  wf.load( is );
  return is;
}

inline std::ostream& operator<<( std::ostream& os, const CWF& wf )
{
  wf.save( os );
  return os;
}

#endif//_wavefront_h
