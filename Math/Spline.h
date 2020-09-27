#ifndef BEZIER_SPLINE_H
#define BEZIER_SPLINE_H



#include <GR/GRTypes.h>

#include <String/XML.h>
#include <String/Convert.h>
#include <Misc/Format.h>

#include <vector>



class CSpline
{

  public:


    struct tSplinePoint
    {
      GR::tVector     m_ptPos, 
                      m_ptControl1, 
                      m_ptControl2;

      bool            m_bSmooth, 
                      m_bSymmetric;



      tSplinePoint( const GR::tVector& p, const GR::tVector& c1, const GR::tVector& c2, bool sm = true, bool sy = true )
        : m_ptPos( p ), 
          m_ptControl1( c1 ), 
          m_ptControl2( c2 ), 
          m_bSmooth( sm ), 
          m_bSymmetric( sy )  
      { 
      }

      tSplinePoint()
        : m_ptPos(), 
          m_ptControl1(), 
          m_ptControl2(), 
          m_bSmooth( true ), 
          m_bSymmetric( true )  
      {  
      }

      void SetPt( const GR::tVector& p )
      {
        GR::tVector delta( p - m_ptPos );

        m_ptPos    = p;
        m_ptControl1 += delta;
        m_ptControl2 += delta;
      }
    
      void SetCtrl1( const GR::tVector& p )
      {
        m_ptControl1 = p;
        if ( m_bSmooth )
        {
          m_ptControl2 = m_ptPos + ( m_ptControl2 - m_ptPos ).length() * ( m_ptPos - m_ptControl1 ).unit();
        }
        if ( m_bSymmetric )
        {
          m_ptControl2 = m_ptPos + ( m_ptControl2 - m_ptPos ).unit()   * ( m_ptPos - m_ptControl1 ).length();
        }
      }

      void SetCtrl2( const GR::tVector& p )
      {
        m_ptControl2 = p;
        if ( m_bSmooth )
        {
          m_ptControl1 = m_ptPos + ( m_ptControl1 - m_ptPos ).length() * ( m_ptPos - m_ptControl2 ).unit();
        }
        if ( m_bSymmetric )
        {
          m_ptControl1 = m_ptPos + ( m_ptControl1 - m_ptPos ).unit()   * ( m_ptPos - m_ptControl2 ).length();
        }
      }

    };

    typedef std::vector<tSplinePoint>  tVectSplinePoints; 
    typedef std::vector<GR::tVector>   tVectVector; 

    tVectSplinePoints      m_SplinePoints;

    tVectVector            m_InterpolatedPoints;

    bool                   m_bClosedSpline;



    CSpline() :
      m_bClosedSpline( true )
    {
    }

    bool CloseSpline() const
    {
      return m_bClosedSpline;
    }

    void CloseSpline( bool bClose )
    {
      m_bClosedSpline = bClose;
    }



    GR::u32 AddPoint( const tSplinePoint& pt, int iBeforethis = -1 )
    {

      tSplinePoint spoint = pt;
      if ( pt.m_bSmooth )
      {
        spoint.m_ptControl2 = spoint.m_ptPos + ( spoint.m_ptControl2 - spoint.m_ptPos ).length() * ( spoint.m_ptPos - spoint.m_ptControl1 ).unit();
      }
      if ( pt.m_bSymmetric )
      {
        spoint.m_ptControl2 = spoint.m_ptPos + ( spoint.m_ptControl2 - spoint.m_ptPos ).unit()   * ( spoint.m_ptPos - spoint.m_ptControl1 ).length();
      }
      
      if ( (size_t)iBeforethis < m_SplinePoints.size() )
      {
        m_SplinePoints.insert( m_SplinePoints.begin() + iBeforethis, spoint );
        return iBeforethis;
      }
      m_SplinePoints.push_back( spoint );

      return (GR::u32)m_SplinePoints.size() - 1;

    }



    void RemovePoint( int iNr )
    {

      m_SplinePoints.erase( m_SplinePoints.begin() + iNr );

    }



    void CalculateSpline( GR::u32 dwSteps )
    {

      m_InterpolatedPoints.clear();
      if ( ( dwSteps == 0 ) 
      ||   ( m_SplinePoints.empty() ) )
      {
        return;
      }

      m_InterpolatedPoints.push_back( m_SplinePoints[0].m_ptPos );
      
      size_t s = m_SplinePoints.size() - 1;
      if ( m_bClosedSpline )
      {
        ++s;
      }
      for ( size_t i = 0; i < s; ++i )
      {
        InterpolateSegment( m_SplinePoints[( i     + m_SplinePoints.size() ) % m_SplinePoints.size()],
                            m_SplinePoints[( i + 1 + m_SplinePoints.size() ) % m_SplinePoints.size()],
                            dwSteps );
      }

    } 



    //evaluate a segment on the B spline
    void InterpolateSegment( const tSplinePoint& sp1, const tSplinePoint& sp2, GR::u32 dwSteps ) 
    {
      for ( GR::u32 j = 1; j <= dwSteps; ++j )
      {
        m_InterpolatedPoints.push_back( InterpolatePoint( sp1, sp2, j / (float)dwSteps ) );
      }
    }



    //evaluate a point on the B spline
    GR::tVector InterpolatePoint( const tSplinePoint& sp1, const tSplinePoint& sp2, float t ) 
    {
      return    ( 1 - t ) * ( 1 - t ) * ( 1 - t )  * sp1.m_ptPos
              + 3 * t * ( 1 - t ) * ( 1 - t )      * sp1.m_ptControl1
              + 3 * t * t * ( 1 - t )              * sp2.m_ptControl2
              + t * t * t                          * sp2.m_ptPos;
    }

    GR::tVector Point( const GR::f32 fPathPos )
    {
      if ( m_SplinePoints.empty() )
      {
        return GR::tVector();
      }

      GR::tVector   vectPos;

      if ( (size_t)fPathPos + 1 >= m_SplinePoints.size() )
      {
        return m_SplinePoints.back().m_ptPos;
      }

      if ( fPathPos - (int)fPathPos == 0.0f )
      {
        // genau auf einem Punkt
        vectPos = m_SplinePoints[(int)fPathPos].m_ptPos;
      }
      else
      {
        vectPos = InterpolatePoint( m_SplinePoints[(int)fPathPos],
                                    m_SplinePoints[( (int)fPathPos ) + 1],
                                    fPathPos - (int)fPathPos );
      }
      return vectPos;
    }

    GR::Strings::XMLElement* ToXML()
    {
      GR::Strings::XMLElement*    pXMLSpline = new GR::Strings::XMLElement( "Spline" );

      for ( size_t i = 0; i < m_SplinePoints.size(); ++i )
      {
        tSplinePoint& Point( m_SplinePoints[i] );

        GR::Strings::XMLElement*    pXMLPoint = new GR::Strings::XMLElement( "Point" );

        pXMLPoint->AddAttribute( "X", Misc::Format() << Point.m_ptPos.x );
        pXMLPoint->AddAttribute( "Y", Misc::Format() << Point.m_ptPos.y );
        pXMLPoint->AddAttribute( "ControlX1", Misc::Format() << Point.m_ptControl1.x );
        pXMLPoint->AddAttribute( "ControlY1", Misc::Format() << Point.m_ptControl1.y );
        pXMLPoint->AddAttribute( "ControlX2", Misc::Format() << Point.m_ptControl2.x );
        pXMLPoint->AddAttribute( "ControlY2", Misc::Format() << Point.m_ptControl2.y );

        pXMLSpline->InsertChild( pXMLPoint );
      }

      return pXMLSpline;
    }

    bool FromXML( GR::Strings::XMLElement* pXMLSpline )
    {

      m_SplinePoints.clear();

      if ( ( pXMLSpline == NULL )
      ||   ( pXMLSpline->Type() != "Spline" ) )
      {
        return false;
      }

      GR::Strings::XML::iterator    it( pXMLSpline->FirstChild() );
      while ( it != GR::Strings::XML::iterator() )
      {
        GR::Strings::XMLElement*    pXMLPoint( *it );

        if ( pXMLPoint->Type() == "Point" )
        {
          CSpline::tSplinePoint    Point;

          Point.m_ptPos.x = GR::Convert::ToF32( pXMLPoint->Attribute( "X" ) );
          Point.m_ptPos.y = GR::Convert::ToF32( pXMLPoint->Attribute( "Y" ) );
          Point.m_ptControl1.x = GR::Convert::ToF32( pXMLPoint->Attribute( "ControlX1" ) );
          Point.m_ptControl1.y = GR::Convert::ToF32( pXMLPoint->Attribute( "ControlY1" ) );
          Point.m_ptControl2.x = GR::Convert::ToF32( pXMLPoint->Attribute( "ControlX2" ) );
          Point.m_ptControl2.y = GR::Convert::ToF32( pXMLPoint->Attribute( "ControlY2" ) );

          AddPoint( Point );
        }

        it = it.next_sibling();
      }

      return true;
    }


};



#endif // BEZIER_SPLINE_H