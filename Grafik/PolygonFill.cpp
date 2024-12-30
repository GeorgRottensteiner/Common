#include <Grafik/ContextDescriptor.h>

#include "PolygonFill.h"



namespace GR
{
  namespace Graphic
  {

    void PolygonFill::BuildFloatGET( const tEdges& Lines, EdgeState* pNextFreeEdgeStruc )
    {

      int   startX,
            startY,
            endX,
            endY,
            deltaX,
            deltaY,
            width;

      EdgeState*  pNewEdgePtr = NULL;
      EdgeState*  pFollowingEdge = NULL;
      EdgeState** pFollowingEdgeLink = NULL;


      // Scan through the vertex list and put all non-0-height edges into
      // the GET, sorted by increasing Y start coordinate
      pGETPtr = NULL;
      for ( size_t i = 0; i < Lines.size(); i++ )
      {
        // Calculate the edge height and width
        startX  = (int)Lines[i].first.x;
        startY  = (int)Lines[i].first.y;
        endX    = (int)Lines[i].second.x;
        endY    = (int)Lines[i].second.y;

        // Make sure the edge runs top to bottom
        if ( startY > endY )
        {
          std::swap( startX, endX );
          std::swap( startY, endY );
        }

        // Skip if this can't ever be an active edge (has 0 height)
        if ( ( deltaY = endY - startY ) != 0 )
        {
          // Allocate space for this edge's info, and fill in the structure
          pNewEdgePtr = pNextFreeEdgeStruc++;

          // direction in which X moves
          pNewEdgePtr->XDirection       = ( ( deltaX = endX - startX ) > 0 ) ? 1 : -1;
          width                         = abs( deltaX );
          pNewEdgePtr->X                = startX;
          pNewEdgePtr->StartY           = startY;
          pNewEdgePtr->Count            = deltaY;
          pNewEdgePtr->ErrorTermAdjDown = deltaY;
          if ( deltaX >= 0 )    
          {
            // initial error term going L->R
            pNewEdgePtr->ErrorTerm = 0;
          }
          else                  
          {
            // initial error term going R->L
            pNewEdgePtr->ErrorTerm = -deltaY + 1;
          }
          if ( deltaY >= width )
          {
            // Y-major edge
            pNewEdgePtr->WholePixelXMove  = 0;
            pNewEdgePtr->ErrorTermAdjUp   = width;
          }
          else
          {
            // X-major edge
            pNewEdgePtr->WholePixelXMove  = ( width / deltaY ) * pNewEdgePtr->XDirection;
            pNewEdgePtr->ErrorTermAdjUp   = width % deltaY;
          }

          // Link the new edge into the GET so that the edge list is still sorted by Y coordinate, 
          // and by X coordinate for all edges with the same Y coordinate
          pFollowingEdgeLink = &pGETPtr;
          while ( true )
          {
            pFollowingEdge = *pFollowingEdgeLink;
            if ( ( pFollowingEdge == NULL )
            ||   ( pFollowingEdge->StartY > startY )
            ||   ( ( pFollowingEdge->StartY == startY )
            &&     ( pFollowingEdge->X >= startX ) ) )
            {
              pNewEdgePtr->pNextEdge = pFollowingEdge;
              *pFollowingEdgeLink = pNewEdgePtr;
              break;
            }
            pFollowingEdgeLink = &pFollowingEdge->pNextEdge;
          }
        }
      }
    }



    void PolygonFill::MoveXSortedToAET( int YToMove )
    {
      EdgeState*  pAETEdge = NULL;
      EdgeState** pAETEdgePtr = &_pActiveEdgeTable;
      EdgeState*  pTempEdge = NULL;
      int         currentX = 0;


      // The GET is Y sorted. Any edges that start at the desired Y coordinate will be first in the GET, 
      // so we'll move edges from the GET to AET until the first edge left in the GET is no longer
      // at the desired Y coordinate. Also, the GET is X sorted within each Y coordinate, so each 
      // successive edge we add to the AET is guaranteed to belong later in the AET than the one just added

      pAETEdgePtr = &_pActiveEdgeTable;
      while ( ( pGETPtr != NULL )
      &&      ( pGETPtr->StartY == YToMove ) )
      {
        currentX = pGETPtr->X;

        // Link the new edge into the AET so that the AET is still sorted by X coordinate
        while ( true )
        {
          pAETEdge = *pAETEdgePtr;

          if ( ( pAETEdge == NULL )
          ||   ( pAETEdge->X >= currentX ) )
          {
            pTempEdge           = pGETPtr->pNextEdge;
            *pAETEdgePtr        = pGETPtr;   

            // link the edge into the AET
            pGETPtr->pNextEdge  = pAETEdge;
            pAETEdgePtr         = &pGETPtr->pNextEdge;

            // unlink the edge from the GET
            pGETPtr = pTempEdge;      
            break;
          }
          else
          {
            pAETEdgePtr = &pAETEdge->pNextEdge;
          }
        }
      }
    }



    void PolygonFill::DrawScanLine( int YToScan, GR::Graphic::ContextDescriptor& cdTarget, GR::u32 Color )
    {
      int           leftX = 0;
      EdgeState*    pCurrentEdge = _pActiveEdgeTable;

      // Scan through the AET, drawing line segments as each pair of edge crossings is encountered. 
      // The nearest pixel on or to the right of left edges is drawn, and the nearest pixel to the left 
      // of but not on right edges is drawn

      while ( pCurrentEdge != NULL )
      {
        leftX         = pCurrentEdge->X;
        pCurrentEdge  = pCurrentEdge->pNextEdge;

        if ( pCurrentEdge == NULL )
        {
          break;
        }
        cdTarget.HLine( leftX, pCurrentEdge->X - 1, YToScan, Color );
        pCurrentEdge = pCurrentEdge->pNextEdge;
      }
    }



    void PolygonFill::AdvanceAET()
    {
      EdgeState*    pCurrentEdge    = NULL;
      EdgeState**   pCurrentEdgePtr = &_pActiveEdgeTable;


      // Count down and remove or advance each edge in the AET
      while ( ( pCurrentEdge = *pCurrentEdgePtr ) != NULL )
      {
        // Count off one scan line for this edge
        if ( ( --pCurrentEdge->Count ) == 0 )
        {
          // This edge is finished, so remove it from the AET
          *pCurrentEdgePtr = pCurrentEdge->pNextEdge;
        }
        else
        {
          // Advance the edge's X coordinate by minimum move
          pCurrentEdge->X += pCurrentEdge->WholePixelXMove;

          // Determine whether it's time for X to advance one extra
          if ( ( pCurrentEdge->ErrorTerm += pCurrentEdge->ErrorTermAdjUp ) > 0 )
          {
            pCurrentEdge->X         += pCurrentEdge->XDirection;
            pCurrentEdge->ErrorTerm -= pCurrentEdge->ErrorTermAdjDown;
          }
          pCurrentEdgePtr = &pCurrentEdge->pNextEdge;
        }
      }
    }



    void PolygonFill::XSortAET()
    {
      EdgeState*    pCurrentEdge = NULL;
      EdgeState**   pCurrentEdgePtr = NULL;
      EdgeState*    pTempEdge = NULL;
      int           swapOccurred = 0;


      // Scan through the AET and swap any adjacent edges for which the
      // second edge is at a lower current X coord than the first edge.
      // Repeat until no further swapping is needed

      if ( _pActiveEdgeTable != NULL )
      {
        do
        {
          swapOccurred    = 0;
          pCurrentEdgePtr = &_pActiveEdgeTable;

          while ( ( pCurrentEdge = *pCurrentEdgePtr )->pNextEdge != NULL )
          {
            if ( pCurrentEdge->X > pCurrentEdge->pNextEdge->X )
            {
              // The second edge has a lower X than the first; swap them in the AET
              pTempEdge                           = pCurrentEdge->pNextEdge->pNextEdge;
              *pCurrentEdgePtr                    = pCurrentEdge->pNextEdge;
              pCurrentEdge->pNextEdge->pNextEdge  = pCurrentEdge;
              pCurrentEdge->pNextEdge             = pTempEdge;
              swapOccurred                        = 1;
            }
            pCurrentEdgePtr = &( *pCurrentEdgePtr )->pNextEdge;
          }
        }
        while ( swapOccurred != 0 );
      }
    }



    void PolygonFill::FillPolygon( GR::Graphic::ContextDescriptor& cdTarget, const tEdges& Edges, GR::u32 Color )
    {
      EdgeState*  pEdgeTableBuffer = NULL;
      int         currentY = 0;

      // It takes a minimum of 3 vertices to cause any pixels to be drawn; 
      // reject polygons that are guaranteed to be invisible
      if ( Edges.size() < 3 )
      {
        return;
      }

      // Get enough memory to store the entire edge table
      if ( ( pEdgeTableBuffer = (EdgeState*)( malloc( sizeof( EdgeState ) * Edges.size() ) ) ) == NULL )
      {
        return;
      }
      BuildFloatGET( Edges, pEdgeTableBuffer );

      // Scan down through the polygon edges, one scan line at a time,
      // so long as at least one edge remains in either the GET or AET

      // initialize the active edge table to empty
      _pActiveEdgeTable = NULL;    
      if ( pGETPtr != NULL )
      {
        // start at the top polygon vertex
        currentY = pGETPtr->StartY;
      }

      while ( ( pGETPtr != NULL )
      ||      ( _pActiveEdgeTable != NULL ) )
      {
        // update AET for this scan line
        MoveXSortedToAET( currentY );  

        // draw this scan line from AET
        DrawScanLine( currentY, cdTarget, Color ); 

        // advance AET edges 1 scan line
        AdvanceAET();                       

        // resort on X
        XSortAET();                         

        // advance to the next scan line
        currentY++;                         
      }

      free( pEdgeTableBuffer );
    }



  }
}