/*--------------------+-------------------------------------------------------+
 | Programmname       : Farb-Funktionen                                       |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 25.05.2001                                            |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Grafik/ContextDescriptor.h>

#include "PolygonFill.h"



void CPolygonFill::BuildFloatGET( const tVectEdges& listLines, EdgeState* NextFreeEdgeStruc )
{

  int         StartX, 
              StartY, 
              EndX, 
              EndY, 
              DeltaY, 
              DeltaX, 
              Width;

  EdgeState*  NewEdgePtr;
  EdgeState*  FollowingEdge;
  EdgeState** FollowingEdgeLink;


  // Scan through the vertex list and put all non-0-height edges into
  // the GET, sorted by increasing Y start coordinate
  GETPtr = NULL;
  for ( size_t i = 0; i < listLines.size(); i++ ) 
  {
    // Calculate the edge height and width
    StartX = (int)listLines[i].first.x;
    StartY = (int)listLines[i].first.y;
    EndX = (int)listLines[i].second.x;
    EndY = (int)listLines[i].second.y;
    // Make sure the edge runs top to bottom
    if ( StartY > EndY ) 
    {
      std::swap( StartX, EndX );
      std::swap( StartY, EndY );
    }
    // Skip if this can't ever be an active edge (has 0 height)
    if ( ( DeltaY = EndY - StartY ) != 0 ) 
    {
      // Allocate space for this edge's info, and fill in the structure
      NewEdgePtr = NextFreeEdgeStruc++;
      // direction in which X moves
      NewEdgePtr->XDirection = ( ( DeltaX = EndX - StartX ) > 0) ? 1 : -1;
      Width               = abs( DeltaX );
      NewEdgePtr->X       = StartX;
      NewEdgePtr->StartY  = StartY;
      NewEdgePtr->Count   = DeltaY;
      NewEdgePtr->ErrorTermAdjDown = DeltaY;
      if ( DeltaX >= 0 )    // initial error term going L->R
      {
        NewEdgePtr->ErrorTerm = 0;
      }
      else                  // initial error term going R->L
      {
        NewEdgePtr->ErrorTerm = -DeltaY + 1;
      }
      if ( DeltaY >= Width ) 
      { 
        // Y-major edge
        NewEdgePtr->WholePixelXMove = 0;
        NewEdgePtr->ErrorTermAdjUp = Width;
      } 
      else 
      {
        // X-major edge
        NewEdgePtr->WholePixelXMove = ( Width / DeltaY ) * NewEdgePtr->XDirection;
        NewEdgePtr->ErrorTermAdjUp  = Width % DeltaY;
      }
      // Link the new edge into the GET so that the edge list is still sorted by Y coordinate, 
      // and by X coordinate for all edges with the same Y coordinate
      FollowingEdgeLink = &GETPtr;
      while ( true )
      {
        FollowingEdge = *FollowingEdgeLink;
        if ( ( FollowingEdge == NULL ) 
        ||   ( FollowingEdge->StartY > StartY ) 
        ||   ( ( FollowingEdge->StartY == StartY ) 
        &&     ( FollowingEdge->X >= StartX ) ) ) 
        {
          NewEdgePtr->NextEdge = FollowingEdge;
          *FollowingEdgeLink = NewEdgePtr;
          break;
        }
        FollowingEdgeLink = &FollowingEdge->NextEdge;
      }
    }
  }

}



void CPolygonFill::MoveXSortedToAET( int YToMove ) 
{

  EdgeState*    AETEdge;
  EdgeState**   AETEdgePtr;
  EdgeState*    TempEdge;

  int CurrentX;


  // The GET is Y sorted. Any edges that start at the desired Y coordinate will be first in the GET, 
  // so we'll move edges from the GET to AET until the first edge left in the GET is no longer
  // at the desired Y coordinate. Also, the GET is X sorted within each Y coordinate, so each 
  // successive edge we add to the AET is guaranteed to belong later in the AET than the one just added

  AETEdgePtr = &AETPtr;
  while ( ( GETPtr != NULL ) 
  &&      ( GETPtr->StartY == YToMove ) ) 
  {
    CurrentX = GETPtr->X;

    // Link the new edge into the AET so that the AET is still sorted by X coordinate
    while ( true )
    {
      AETEdge = *AETEdgePtr;

      if ( ( AETEdge == NULL ) 
      ||   ( AETEdge->X >= CurrentX ) ) 
      {
        TempEdge    = GETPtr->NextEdge;
        *AETEdgePtr = GETPtr;   // link the edge into the AET
        GETPtr->NextEdge = AETEdge;
        AETEdgePtr  = &GETPtr->NextEdge;
        GETPtr = TempEdge;      // unlink the edge from the GET
        break;
      } 
      else 
      {
        AETEdgePtr = &AETEdge->NextEdge;
      }
    }
  }

}



void CPolygonFill::DrawScanLine( int YToScan, GR::Graphic::ContextDescriptor& cdTarget, GR::u32 dwColor ) 
{

  int           LeftX;

  EdgeState*    CurrentEdge;

  // Scan through the AET, drawing line segments as each pair of edge crossings is encountered. 
  // The nearest pixel on or to the right of left edges is drawn, and the nearest pixel to the left 
  // of but not on right edges is drawn

  CurrentEdge = AETPtr;

  while ( CurrentEdge != NULL ) 
  {
    LeftX       = CurrentEdge->X;
    CurrentEdge = CurrentEdge->NextEdge;

	  if ( CurrentEdge == NULL )
    {
      break;
    }
    cdTarget.HLine( LeftX, CurrentEdge->X - 1, YToScan, dwColor );
    CurrentEdge = CurrentEdge->NextEdge;
  }

}



void CPolygonFill::AdvanceAET() 
{

  EdgeState*    CurrentEdge;
  EdgeState**   CurrentEdgePtr;


  // Count down and remove or advance each edge in the AET
  CurrentEdgePtr = &AETPtr;
  while ( ( CurrentEdge = *CurrentEdgePtr ) != NULL ) 
  {
    // Count off one scan line for this edge
    if ( ( --CurrentEdge->Count ) == 0 ) 
    {
      // This edge is finished, so remove it from the AET
      *CurrentEdgePtr = CurrentEdge->NextEdge;
    } 
    else 
    {
      // Advance the edge's X coordinate by minimum move
      CurrentEdge->X += CurrentEdge->WholePixelXMove;
      // Determine whether it's time for X to advance one extra
      if ( ( CurrentEdge->ErrorTerm += CurrentEdge->ErrorTermAdjUp ) > 0 ) 
      {
        CurrentEdge->X += CurrentEdge->XDirection;
        CurrentEdge->ErrorTerm -= CurrentEdge->ErrorTermAdjDown;
      }
      CurrentEdgePtr = &CurrentEdge->NextEdge;
    }
  }

}



void CPolygonFill::XSortAET() 
{

  EdgeState*    CurrentEdge;
  EdgeState**   CurrentEdgePtr;
  EdgeState*    TempEdge;


  int           SwapOccurred;


  // Scan through the AET and swap any adjacent edges for which the
  // second edge is at a lower current X coord than the first edge.
  // Repeat until no further swapping is needed

  if ( AETPtr != NULL ) 
  {
    do 
    {
      SwapOccurred = 0;
      CurrentEdgePtr = &AETPtr;
      while ( ( CurrentEdge = *CurrentEdgePtr )->NextEdge != NULL ) 
      {
        if ( CurrentEdge->X > CurrentEdge->NextEdge->X ) 
        {
          // The second edge has a lower X than the first; swap them in the AET
          TempEdge        = CurrentEdge->NextEdge->NextEdge;
          *CurrentEdgePtr = CurrentEdge->NextEdge;
          CurrentEdge->NextEdge->NextEdge = CurrentEdge;
          CurrentEdge->NextEdge = TempEdge;
          SwapOccurred    = 1;
        }
        CurrentEdgePtr = &(*CurrentEdgePtr)->NextEdge;
      }
    } 
    while ( SwapOccurred != 0 );
  }

}



void CPolygonFill::FillPolygon( GR::Graphic::ContextDescriptor& cdTarget, const tVectEdges& vectEdges, GR::u32 dwColor )
{

  struct EdgeState*   EdgeTableBuffer;


  int CurrentY;

  // It takes a minimum of 3 vertices to cause any pixels to be drawn; 
  // reject polygons that are guaranteed to be invisible
  if ( vectEdges.size() < 3 )
  {
    return;
  }

  // Get enough memory to store the entire edge table
  if ( ( EdgeTableBuffer = (EdgeState*)( malloc( sizeof( EdgeState ) * vectEdges.size() ) ) ) == NULL )
  {
    return;
  }
  BuildFloatGET( vectEdges, EdgeTableBuffer );

  // Scan down through the polygon edges, one scan line at a time,
  // so long as at least one edge remains in either the GET or AET

  AETPtr = NULL;    // initialize the active edge table to empty

  if ( GETPtr != NULL )
  {
    // start at the top polygon vertex
    CurrentY = GETPtr->StartY; 
  }

  while ( ( GETPtr != NULL ) 
  ||      ( AETPtr != NULL ) ) 
  {
    MoveXSortedToAET( CurrentY );       // update AET for this scan line
    DrawScanLine( CurrentY, cdTarget, dwColor ); // draw this scan line from AET
    AdvanceAET();                       // advance AET edges 1 scan line
    XSortAET();                         // resort on X
    CurrentY++;                         // advance to the next scan line
  }

  free( EdgeTableBuffer );

}