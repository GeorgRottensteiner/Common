// CullBox.h: interface for the CCullBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CULLBOX_H__FEFC0E5E_44C9_482C_A372_BD12DDFD40D1__INCLUDED_)
#define AFX_CULLBOX_H__FEFC0E5E_44C9_482C_A372_BD12DDFD40D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <GR/GRTypes.h>
#include <math\rectt.h>

template <class T> class COctree
{

  typedef math::vector3t<float>     tVector3;

  protected:

  public:

    size_t                                m_iMaxObjects,
                                          m_iUsedChildNodes;

    typedef math::rectt<tVector3>         tRect3f;

    typedef std::pair<tVector3,T>         tObjectPair;

    typedef std::list<tObjectPair>        tListOctreeObjects;

    tVector3                              m_vectSideRadius;

    COctree<T>*                           m_pParent;

    COctree<T>*                           m_pNodes[8];

    enum OctreeSide
    {
      OS_X0_Y0_Z0 = 0,
      OS_X1_Y0_Z0,
      OS_X0_Y1_Z0,
      OS_X1_Y1_Z0,
      OS_X0_Y0_Z1,
      OS_X1_Y0_Z1,
      OS_X0_Y1_Z1,
      OS_X1_Y1_Z1,
      OS_INVALID,
    };

    tRect3f                           m_BoundingRect;



    std::list<tObjectPair>            m_ListObjects;


    COctree() :
      m_pParent( NULL ),
      m_iMaxObjects( 5 ),
      m_vectSideRadius( 50.0f, 50.0f, 50.0f ),
      m_iUsedChildNodes( 0 )
    {
      memset( m_pNodes, 0, 8 * sizeof( COctree<T>* ) );
      m_BoundingRect.position( -m_vectSideRadius );
      m_BoundingRect.size( 2 * m_vectSideRadius );

      /*
      dh::Log( "ParentNode created Radius %d,%d,%d  Pos %d,%d,%d\n", (int)m_vectSideRadius.x, (int)m_vectSideRadius.y, (int)m_vectSideRadius.z,
               (int)m_BoundingRect.position().x, (int)m_BoundingRect.position().y, (int)m_BoundingRect.position().z );
      */
    }

    COctree( const tVector3& vectPosition, const tVector3& SideRadius, int iMaxObjects = 5 ) :
      m_pParent( NULL ),
      m_iMaxObjects( iMaxObjects ),
      m_vectSideRadius( SideRadius ),
      m_iUsedChildNodes( 0 )
    {
      m_BoundingRect.position( vectPosition );
      m_BoundingRect.size( SideRadius * 2.0f );
      memset( m_pNodes, 0, 8 * sizeof( COctree<T>* ) );

      /*
      dh::Log( "ChildNode created Radius %d,%d,%d  Pos %d,%d,%d\n", (int)m_vectSideRadius.x, (int)m_vectSideRadius.y, (int)m_vectSideRadius.z,
               (int)m_BoundingRect.position().x, (int)m_BoundingRect.position().y, (int)m_BoundingRect.position().z );
      */
    }

    COctree( const tRect3f& BoundingRect, int iMaxObjects = 5 ) :
      m_pParent( NULL ),
      m_iMaxObjects( iMaxObjects ),
      m_BoundingRect( BoundingRect ),
      m_iUsedChildNodes( 0 )
    {
      m_vectSideRadius = m_BoundingRect.size() / 2.0f;
      memset( m_pNodes, 0, 8 * sizeof( COctree<T>* ) );

      /*
      dh::Log( "ChildNode created Radius %d,%d,%d  Pos %d,%d,%d\n", (int)m_vectSideRadius.x, (int)m_vectSideRadius.y, (int)m_vectSideRadius.z,
               (int)m_BoundingRect.position().x, (int)m_BoundingRect.position().y, (int)m_BoundingRect.position().z );
      */
    }


    ~COctree()
    {
      for ( int i = 0; i < 8; i++ )
      {
        if ( m_pNodes[i] )
        {
          delete m_pNodes[i];
          m_pNodes[i] = NULL;
        }
      }
    }

    void clear()
    {
      for ( int i = 0; i < 8; i++ )
      {
        if ( m_pNodes[i] )
        {
          delete m_pNodes[i];
          m_pNodes[i] = NULL;
        }
      }
      m_ListObjects.clear();
      m_iUsedChildNodes = 0;
    }

    void CreateNode( OctreeSide osNode )
    {
      if ( osNode == OS_INVALID )
      {
        return;
      }
      // diesen Node gibt es schon
      if ( m_pNodes[osNode] )
      {
        return;
      }

      tRect3f   rectChild;

      rectChild.size( m_vectSideRadius );
      rectChild.position( m_BoundingRect.position() +
                tVector3( ( osNode % 2 ) * m_vectSideRadius.x, ( osNode % 4 ) / 2 * m_vectSideRadius.y, osNode / 4 * m_vectSideRadius.z ) );

      m_pNodes[osNode] = new COctree( rectChild, m_iMaxObjects );
      m_pNodes[osNode]->m_pParent = this;
      m_iUsedChildNodes++;
    }

    bool contains( const tVector3& vectPos ) const
    {
      return m_BoundingRect.contains( vectPos );
    }

    bool intersects( const tRect3f& rectBB ) const
    {
      return m_BoundingRect.intersects( rectBB );
    }

    OctreeSide DetermineChildNode( const tVector3& vectPos )
    {
      if ( !m_BoundingRect.contains( vectPos ) )
      {
        return OS_INVALID;
      }

      tRect3f   rectChild;

      rectChild.size( m_vectSideRadius );

      for ( int iX = 0; iX < 2; ++iX )
      {
        for ( int iY = 0; iY < 2; ++iY )
        {
          for ( int iZ = 0; iZ < 2; ++iZ )
          {
            rectChild.position( m_BoundingRect.position() +
                      tVector3( iX * m_vectSideRadius.x, iY * m_vectSideRadius.y, iZ * m_vectSideRadius.z ) );

            if ( rectChild.contains( vectPos ) )
            {
              //dh::Log( "Contains mit %d,%d,%d\n", iX, iY, iZ );
              return OS_X0_Y0_Z0 + iZ * 4 + iY * 2 + iX;
            }
          }
        }
      }

      return OS_INVALID;

    }

    bool InsertInChildNode( const T& Object, const tVector3& vectPos )
    {
      if ( !m_BoundingRect.contains( vectPos ) )
      {
        dh::Log( "InsertInChildNode - does not contain\n" );
        return false;
      }

      tRect3f   rectChild;

      rectChild.size( m_vectSideRadius );

      for ( int iX = 0; iX < 2; ++iX )
      {
        for ( int iY = 0; iY < 2; ++iY )
        {
          for ( int iZ = 0; iZ < 2; ++iZ )
          {
            rectChild.position( m_BoundingRect.position()
                    + tVector3( iX * m_vectSideRadius.x, iY * m_vectSideRadius.y, iZ * m_vectSideRadius.z ) );

            if ( rectChild.contains( vectPos ) )
            {
              OctreeSide    newOSide = (OctreeSide)( OS_X0_Y0_Z0 + iZ * 4 + iY * 2 + iX );

              /*
              dh::Log( "Contains mit %d,%d,%d\n", iX, iY, iZ );
              dh::Log( "rectChild bei %d,%d,%d - %d,%d,%d\n",
                  (int)rectChild.position().x,
                  (int)rectChild.position().y,
                  (int)rectChild.position().z,
                  (int)( rectChild.position().x + rectChild.size().x ),
                  (int)( rectChild.position().y + rectChild.size().y ),
                  (int)( rectChild.position().z + rectChild.size().z ) );
                  */
              if ( m_pNodes[newOSide] == NULL )
              {
                m_pNodes[newOSide] = new COctree( rectChild, m_iMaxObjects );
                m_pNodes[newOSide]->m_pParent = this;
                m_iUsedChildNodes++;
              }
              return m_pNodes[newOSide]->insert( Object, vectPos );
            }
          }
        }
      }

      // hier hat das einsetzen nicht hingehauen
      dh::Log( "insert mit %d,%d,%d\n", (int)vectPos.x, (int)vectPos.y, (int)vectPos.z );
      for ( iX = 0; iX < 2; ++iX )
      {
        for ( int iY = 0; iY < 2; ++iY )
        {
          for ( int iZ = 0; iZ < 2; ++iZ )
          {
            rectChild.position( m_BoundingRect.position()
                    + tVector3( iX * m_vectSideRadius.x, iY * m_vectSideRadius.y, iZ * m_vectSideRadius.z ) );
            dh::Log( "rectChild bei %d,%d,%d - %d,%d,%d\n",
                (int)rectChild.position().x,
                (int)rectChild.position().y,
                (int)rectChild.position().z,
                (int)( rectChild.position().x + rectChild.size().x ),
                (int)( rectChild.position().y + rectChild.size().y ),
                (int)( rectChild.position().z + rectChild.size().z ) );
          }
        }
      }
      dh::Log( "InsertInChildNode - did not find suiting childnode\n" );
      return false;
    }

    bool insert( const T& Object, const tVector3& vectPos )
    {
      if ( !m_BoundingRect.contains( vectPos ) )
      {
        // BAUSTELLE - Selbst erweitern?
        dh::Log( "outside %d,%d,%d < %d,%d,%d < %d,%d,%d\n",
            (int)m_BoundingRect.position().x,
            (int)m_BoundingRect.position().y,
            (int)m_BoundingRect.position().z,
            (int)vectPos.x,
            (int)vectPos.y,
            (int)vectPos.z,
            (int)( m_BoundingRect.position().x + m_BoundingRect.size().x ),
            (int)( m_BoundingRect.position().y + m_BoundingRect.size().y ),
            (int)( m_BoundingRect.position().z + m_BoundingRect.size().z ) );
        return false;
      }

      if ( m_iUsedChildNodes )
      {
        return InsertInChildNode( Object, vectPos );
      }

      if ( m_ListObjects.size() < m_iMaxObjects )
      {
        m_ListObjects.push_back( std::make_pair( vectPos, Object ) );
      }

      if ( m_ListObjects.size() < m_iMaxObjects )
      {
        return true;
      }

      // jetzt ist das Maximum erreicht, Objekte in Child-Nodes aufteilen
      tListOctreeObjects::iterator    it( m_ListObjects.begin() );
      while ( it != m_ListObjects.end() )
      {
        InsertInChildNode( it->second, it->first );

        ++it;
      }

      m_ListObjects.clear();

      return true;
    }

    bool erase( const T& Object )
    {

      if ( m_iUsedChildNodes )
      {
        for ( int i = 0; i < 8; i++ )
        {
          if ( m_pNodes[i] )
          {
            if ( m_pNodes[i]->erase( Object ) )
            {
              if ( m_pNodes[i]->empty() )
              {
                delete m_pNodes[i];
                m_pNodes[i] = NULL;
                m_iUsedChildNodes--;
              }
              return true;
            }
          }
        }
        return false;
      }
      tListOctreeObjects::iterator    it( m_ListObjects.begin() );
      while ( it != m_ListObjects.end() )
      {
        T&  ListObj = it->second;

        if ( ListObj == Object )
        {
          m_ListObjects.erase( it );
          return true;
        }

        ++it;
      }

      return false;
    }

    bool empty() const
    {
      if ( m_iUsedChildNodes )
      {
        return false;
      }
      return m_ListObjects.empty();
    }

    int size() const
    {
      int   iSize = m_ListObjects.size();

      for ( int i = 0; i < 8; i++ )
      {
        if ( m_pNodes[i] )
        {
          iSize += m_pNodes[i]->size();
        }
      }
      return iSize;
    }

    int nodecount() const
    {
      int   iCount = 1;

      for ( int i = 0; i < 8; i++ )
      {
        if ( m_pNodes[i] )
        {
          iCount += m_pNodes[i]->nodecount();
        }
      }
      return iCount;
    }

    bool erase( const T& Object, const tVector3& vectPos )
    {
      if ( m_iUsedChildNodes )
      {
        OctreeSide  oSide = DetermineChildNode( vectPos );

        if ( ( oSide != OS_INVALID )
        &&   ( m_pNodes[oSide] ) )
        {
          if ( m_pNodes[oSide]->erase( Object ) )
          {
            if ( m_pNodes[oSide]->empty() )
            {
              delete m_pNodes[oSide];
              m_pNodes[oSide] = NULL;
              m_iUsedChildNodes--;
            }
            return true;
          }
        }
        return false;
      }

      tListOctreeObjects::iterator    it( m_ListObjects.begin() );
      while ( it != m_ListObjects.end() )
      {
        T&  ListObj = *it;

        if ( ListObj == Object )
        {
          m_ListObjects.erase( it );
          return true;
        }

        ++it;
      }

    }

    void MoveObject( const T& Object, const tVector3& vectNewPos )
    {
      erase( Object );
      insert( Object, vectNewPos );
    }

    void MoveObject( const T& Object, const tVector3& vectOldPos, const tVector3& vectNewPos )
    {
      erase( Object, vectOldPos );
      insert( Object, vectNewPos );
    }

    void dump( const GR::String& strOffset = "" ) const
    {
      dh::Log( strOffset + "-node\n" );
      dh() << strOffset << "Bounds: " << m_BoundingRect.position() << "\n";
      dh() << strOffset << "        " << m_BoundingRect.position() + m_BoundingRect.size() << "\n";
      dh() << strOffset << "Radius: " << m_vectSideRadius << "\n";
      dh() << strOffset << "-" << m_ListObjects.size() << " Objects\n";

      tListOctreeObjects::const_iterator    it( m_ListObjects.begin() );
      while ( it != m_ListObjects.end() )
      {
        dh() << strOffset << ".." << CMisc::printf( "%x", it->second ) << "\n";

        ++it;
      }

      for ( int i = 0; i < 8; i++ )
      {
        if ( m_pNodes[i] )
        {
          m_pNodes[i]->dump( strOffset + "  " );
        }
      }
    }

    void EnumNeighbourObjects( const tRect3f& rectBB,
                               std::list<T>& listObjects )
    {

      if ( !m_iUsedChildNodes )
      {
        // es gibt keine Childnodes mehr, die eigenen Objekte addieren
        tListOctreeObjects::iterator    it( m_ListObjects.begin() );
        while ( it != m_ListObjects.end() )
        {
          if ( rectBB.contains( it->first ) )
          {
            listObjects.push_back( it->second );
          }
          ++it;
        }
        return;
      }
      for ( int i = 0; i < 8; i++ )
      {
        if ( m_pNodes[i] )
        {
          if ( m_pNodes[i]->intersects( rectBB ) )
          {
            m_pNodes[i]->EnumNeighbourObjects( rectBB, listObjects );
          }
        }
      }
    }

};

#endif // !defined(AFX_CULLBOX_H__FEFC0E5E_44C9_482C_A372_BD12DDFD40D1__INCLUDED_)
