#ifndef RECT_REGION_H
#define RECT_REGION_H

#include <GR/GRTypes.h>
#include <math/tRect.h>


namespace math
{

class RectRegion
{

  public:

    typedef std::list<GR::tRect>    tListRects;

    tListRects                      m_ListRects;

  public:

    RectRegion()
    {
    }

    RectRegion( const GR::tRect& rhs )
    {
      m_ListRects.push_back( rhs );
    }

    bool empty() const
    {
      return m_ListRects.empty();
    }

    void clear()
    {
      m_ListRects.clear();
    }

    RectRegion&      combine( const RectRegion& rhs )
    {
      return *this;
    }

    // check if adjacent rects can be combined
    void Normalize()
    {
      if ( m_ListRects.size() <= 1 )
      {
        return;
      }

      restart:;
      tListRects::iterator  itR( m_ListRects.begin() );
      while ( itR != m_ListRects.end() )
      {
        GR::tRect&    rectInList( *itR );

        tListRects::iterator  itR2( itR );
        ++itR2;
        while ( itR2 != m_ListRects.end() )
        {
          GR::tRect&    rcTemp( *itR2 );

          // can be combined with existing rect?
          // combine right side
          if ( ( rectInList.Right == rcTemp.Left )
          &&   ( rectInList.Top == rcTemp.Top )
          &&   ( rectInList.Bottom == rcTemp.Bottom ) )
          {
            rectInList.Right = rcTemp.Right;
            m_ListRects.erase( itR2 );
            goto restart;
          }
          // combine left side
          if ( ( rectInList.Left == rcTemp.Right )
          &&   ( rectInList.Top == rcTemp.Top )
          &&   ( rectInList.Bottom == rcTemp.Bottom ) )
          {
            rectInList.Left = rcTemp.Left;
            m_ListRects.erase( itR2 );
            goto restart;
          }
          // combine bottom side
          if ( ( rectInList.Bottom == rcTemp.Top )
          &&   ( rectInList.Left == rcTemp.Left )
          &&   ( rectInList.Right == rcTemp.Right ) )
          {
            rectInList.Bottom = rcTemp.Bottom;
            m_ListRects.erase( itR2 );
            goto restart;
          }
          // combine top side
          if ( ( rectInList.Top == rcTemp.Bottom )
          &&   ( rectInList.Left == rcTemp.Left )
          &&   ( rectInList.Right == rcTemp.Right ) )
          {
            rectInList.Top = rcTemp.Top;
            m_ListRects.erase( itR2 );
            goto restart;
          }
          ++itR2;
        }

        ++itR;
      }
    }

    RectRegion&      combine( const GR::tRect& rhs )
    {
      RectRegion   rgnTemp( rhs );

      tListRects::iterator  itNew( rgnTemp.m_ListRects.begin() );
      while ( itNew != rgnTemp.m_ListRects.end() )
      {
        GR::tRect     rcTemp = *itNew;

        tListRects::iterator    itOld = itNew;
        itNew++;
        tListRects::iterator  it( m_ListRects.begin() );

        bool      bInserted = false;

        while ( it != m_ListRects.end() )
        {
          GR::tRect&    rectInList = *it;

          if ( rectInList.contains( rcTemp ) )
          {
            // voll enthalten
            itNew = rgnTemp.m_ListRects.erase( itOld );
            bInserted = true;
            break;
          }
          if ( rectInList.intersects( rcTemp ) )
          {
            // oha, Rechteck muß angeschnitten werden

            GR::tRect   rcSchnittMenge = rectInList.intersection( rcTemp );
            rgnTemp.remove( rcSchnittMenge );

            // nochmal neu beginnen
            itNew = rgnTemp.m_ListRects.begin();
            bInserted = true;
            break;
          }
          // can be combined with existing rect?
          // combine right side
          if ( ( rectInList.Right == rcTemp.Left )
          &&   ( rectInList.Top == rcTemp.Top )
          &&   ( rectInList.Bottom == rcTemp.Bottom ) )
          {
            rectInList.Right = rcTemp.Right;
            Normalize();
            return *this;
          }
          // combine left side
          if ( ( rectInList.Left == rcTemp.Right )
          &&   ( rectInList.Top == rcTemp.Top )
          &&   ( rectInList.Bottom == rcTemp.Bottom ) )
          {
            rectInList.Left = rcTemp.Left;
            Normalize();
            return *this;
          }
          // combine bottom side
          if ( ( rectInList.Bottom == rcTemp.Top )
          &&   ( rectInList.Left == rcTemp.Left )
          &&   ( rectInList.Right == rcTemp.Right ) )
          {
            rectInList.Bottom = rcTemp.Bottom;
            Normalize();
            return *this;
          }
          // combine top side
          if ( ( rectInList.Top == rcTemp.Bottom )
          &&   ( rectInList.Left == rcTemp.Left )
          &&   ( rectInList.Right == rcTemp.Right ) )
          {
            rectInList.Top = rcTemp.Top;
            Normalize();
            return *this;
          }

          ++it;
        }
        if ( !bInserted )
        {
          // es war kein Ausschnitt dabei
          m_ListRects.push_back( rcTemp );
        }
      }
      // nicht gefunden bzw. angeschnippelt
      return *this;
    }

    void remove( const RectRegion& rgnRemove )
    {
      tListRects::const_iterator  it( rgnRemove.m_ListRects.begin() );
      while ( it != rgnRemove.m_ListRects.end() )
      {
        const GR::tRect&    rcOther = *it;

        remove( rcOther );

        ++it;
      }

    }

    void remove( const GR::tRect& rcRemove )
    {
      tListRects::iterator  it( m_ListRects.begin() );
      while ( it != m_ListRects.end() )
      {
        GR::tRect&    rcLocal = *it;

        if ( !rcLocal.intersects( rcRemove ) )
        {
          ++it;
          continue;
        }

        if ( rcLocal == rcRemove )
        {
          // komplett gleich
          it = m_ListRects.erase( it );
          return;
        }

        // Rechteck muß angeschnippelt werden
        GR::tRect   rcSchnittMenge = rcLocal.intersection( rcRemove );

        // schneide innerhalb
        if ( rcSchnittMenge.Top > rcLocal.Top )
        {
          // Quer-Rect oben
          m_ListRects.push_back( GR::tRect( rcLocal.Left, rcLocal.Top, rcLocal.Right - rcLocal.Left, rcSchnittMenge.Top - rcLocal.Top ) );
        }
        if ( rcSchnittMenge.Left > rcLocal.Left )
        {
          // Rect links
          m_ListRects.push_back( GR::tRect( rcLocal.Left, rcSchnittMenge.Top, rcSchnittMenge.Left - rcLocal.Left, rcSchnittMenge.Bottom - rcSchnittMenge.Top ) );
        }
        if ( rcSchnittMenge.Right < rcLocal.Right )
        {
          // Rect rechts
          m_ListRects.push_back( GR::tRect( rcSchnittMenge.Right, rcSchnittMenge.Top, rcLocal.Right - rcSchnittMenge.Right, rcSchnittMenge.Bottom - rcSchnittMenge.Top ) );
        }
        if ( rcSchnittMenge.Bottom < rcLocal.Bottom )
        {
          // Quer-Rect unten
          m_ListRects.push_back( GR::tRect( rcLocal.Left, rcSchnittMenge.Bottom, rcLocal.Right - rcLocal.Left, rcLocal.Bottom - rcSchnittMenge.Bottom ) );
        }
        it = m_ListRects.erase( it );
        continue;

        ++it;
      }

    }

    bool contains( GR::i32 iX, GR::i32 iY ) const
    {
      tListRects::const_iterator    it( m_ListRects.begin() );
      while ( it != m_ListRects.end() )
      {
        const GR::tRect&    rcLocal = *it;

        if ( rcLocal.contains( iX, iY ) )
        {
          return true;
        }

        ++it;
      }
      return false;
    }

    bool contains( const GR::tRect& rhs ) const
    { 
      RectRegion   rgnTemp( rhs );

      RectRegion   rgnTemp2( *this );

      rgnTemp2.remove( rgnTemp );

      return rgnTemp.empty();
    }

    //- prüfen, ob sich zwei rects schneiden
    bool intersects( const GR::tRect& rhs ) const
    { 
      tListRects::const_iterator    it( m_ListRects.begin() );
      while ( it != m_ListRects.end() )
      {
        const GR::tRect&    rcLocal = *it;

        if ( rcLocal.intersects( rhs ) )
        {
          return true;
        }

        ++it;
      }
      return false;
    }

    void intersect( const RectRegion& rhsRegion )
    { 
      RectRegion   rcIntersection;

      tListRects::iterator    it( m_ListRects.begin() );
      while ( it != m_ListRects.end() )
      {
        GR::tRect&    rcLocal = *it;

        tListRects::const_iterator    it2( rhsRegion.m_ListRects.begin() );
        while ( it2 != rhsRegion.m_ListRects.end() )
        {
          const GR::tRect&    rcOther = *it2;

          if ( rcLocal.intersects( rcOther ) )
          {
            rcIntersection.combine( rcLocal.intersection( rcOther ) );
          }

          ++it2;
        }

        ++it;
      }

      *this = rcIntersection;
    }

};

};

#endif //RECT_REGION_H