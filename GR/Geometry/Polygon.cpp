#include ".\polygon.h"



namespace GR
{
  namespace Geometry
  {

    bool Polygon2d::Intersects( const Polygon2d& OtherPoly )
    {
      // geklauter SAT

	    GR::tFPoint axis;
      GR::f32      tmp, minA, maxA, minB, maxB;
	    size_t      side, i;

      Polygon2d*  a = this;
      const Polygon2d*  b = &OtherPoly;


	    // test polygon A's sides
	    for ( side = 0; side < a->m_vectPoints.size(); side++ )
	    {
		    // get the axis that we will project onto
		    if ( side == 0)
		    {
			    axis.x = a->m_vectPoints[a->m_vectPoints.size() - 1].y - a->m_vectPoints[0].y;
			    axis.y = a->m_vectPoints[0].x - a->m_vectPoints[a->m_vectPoints.size() - 1].x;
		    }
		    else
		    {
			    axis.x = a->m_vectPoints[side - 1].y - a->m_vectPoints[side].y;
			    axis.y = a->m_vectPoints[side].x - a->m_vectPoints[side - 1].x;
		    }

		    // normalize the axis
		    tmp = sqrt(axis.x * axis.x + axis.y * axis.y);
		    axis.x /= tmp;
		    axis.y /= tmp;

		    // project polygon A onto axis to determine the min/max
		    minA = maxA = a->m_vectPoints[0].x * axis.x + a->m_vectPoints[0].y * axis.y;
		    for (i = 1; i < a->m_vectPoints.size(); i++)
		    {
			    tmp = a->m_vectPoints[i].x * axis.x + a->m_vectPoints[i].y * axis.y;
			    if (tmp > maxA)
				    maxA = tmp;
			    else if (tmp < minA)
				    minA = tmp;
		    }
		    // correct for offset
        /*
		    tmp = a->Center.x * axis.x + a->Center.y * axis.y;
		    minA += tmp;
		    maxA += tmp;
        */

		    // project polygon B onto axis to determine the min/max
		    minB = maxB = b->m_vectPoints[0].x * axis.x + b->m_vectPoints[0].y * axis.y;
		    for (i = 1; i < b->m_vectPoints.size(); i++)
		    {
			    tmp = b->m_vectPoints[i].x * axis.x + b->m_vectPoints[i].y * axis.y;
			    if (tmp > maxB)
				    maxB = tmp;
			    else if (tmp < minB)
				    minB = tmp;
		    }
		    // correct for offset
        /*
		    tmp = b->Center.x * axis.x + b->Center.y * axis.y;
		    minB += tmp;
		    maxB += tmp;
        */

		    // test if lines intersect, if not, return false
		    if ( ( maxA < minB )
        ||   ( minA > maxB ) )
        {
			    return false;
        }
	    }

	    // test polygon B's sides
	    for (side = 0; side < b->m_vectPoints.size(); side++)
	    {
		    // get the axis that we will project onto
		    if (side == 0)
		    {
			    axis.x = b->m_vectPoints[b->m_vectPoints.size() - 1].y - b->m_vectPoints[0].y;
			    axis.y = b->m_vectPoints[0].x - b->m_vectPoints[b->m_vectPoints.size() - 1].x;
		    }
		    else
		    {
			    axis.x = b->m_vectPoints[side - 1].y - b->m_vectPoints[side].y;
			    axis.y = b->m_vectPoints[side].x - b->m_vectPoints[side - 1].x;
		    }

		    // normalize the axis
		    tmp = sqrt(axis.x * axis.x + axis.y * axis.y);
		    axis.x /= tmp;
		    axis.y /= tmp;

		    // project polygon A onto axis to determine the min/max
		    minA = maxA = a->m_vectPoints[0].x * axis.x + a->m_vectPoints[0].y * axis.y;
		    for (i = 1; i < a->m_vectPoints.size(); i++)
		    {
			    tmp = a->m_vectPoints[i].x * axis.x + a->m_vectPoints[i].y * axis.y;
			    if (tmp > maxA)
				    maxA = tmp;
			    else if (tmp < minA)
				    minA = tmp;
		    }
		    // correct for offset
        /*
		    tmp = a->Center.x * axis.x + a->Center.y * axis.y;
		    minA += tmp;
		    maxA += tmp;
        */

		    // project polygon B onto axis to determine the min/max
		    minB = maxB = b->m_vectPoints[0].x * axis.x + b->m_vectPoints[0].y * axis.y;
		    for (i = 1; i < b->m_vectPoints.size(); i++)
		    {
			    tmp = b->m_vectPoints[i].x * axis.x + b->m_vectPoints[i].y * axis.y;
			    if (tmp > maxB)
				    maxB = tmp;
			    else if (tmp < minB)
				    minB = tmp;
		    }
		    // correct for offset
        /*
		    tmp = b->Center.x * axis.x + b->Center.y * axis.y;
		    minB += tmp;
		    maxB += tmp;
        */

		    // test if lines intersect, if not, return false
		    if ( ( maxA < minB )
        ||   ( minA > maxB ) )
        {
			    return false;
        }
	    }
      return true;
    }

    bool Polygon2d::IsPointInside( const GR::tFPoint& ptPos ) const
    {
      // int pnpoly(int npol, float *xp, float *yp, float x, float y)
      size_t    i,j;
      bool      bResult = false;

      for ( i = 0, j = m_vectPoints.size() - 1; i < m_vectPoints.size(); j = i++ ) 
      {
        if ( ( ( ( m_vectPoints[i].y <= ptPos.y ) 
        &&       ( ptPos.y < m_vectPoints[j].y ) ) 
        ||     ( ( m_vectPoints[j].y <= ptPos.y ) 
        &&       ( ptPos.y < m_vectPoints[i].y ) ) ) 
        &&   ( ptPos.x < ( m_vectPoints[j].x - m_vectPoints[i].x ) * ( ptPos.y - m_vectPoints[i].y ) / ( m_vectPoints[j].y - m_vectPoints[i].y ) + m_vectPoints[i].x ) )
        {
          bResult = !bResult;
        }
      }
      return bResult;
    }

  };

};