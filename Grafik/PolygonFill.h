#ifndef POLYGON_FILL_H
#define POLYGON_FILL_H



#include <GR/GRTypes.h>

#include <vector>



namespace GR
{
  namespace Graphic
  {
    class ContextDescriptor;
  }
}

class CPolygonFill
{

  protected:

    struct EdgeState 
    {
      EdgeState*    NextEdge;
      int X;
      int StartY;
      int WholePixelXMove;
      int XDirection;
      int ErrorTerm;
      int ErrorTermAdjUp;
      int ErrorTermAdjDown;
      int Count;
    };


    EdgeState*              GETPtr;
    EdgeState*              AETPtr;


  public:

    typedef std::pair<GR::tFPoint,GR::tFPoint>    tLine;

    typedef std::vector<tLine>                    tVectEdges;


    void                    FillPolygon( GR::Graphic::ContextDescriptor& cdTarget, const tVectEdges& vectEdges, GR::u32 dwColor );


  protected:

    void                    BuildFloatGET( const tVectEdges& listLines, EdgeState* NextFreeEdgeStruc );
    void                    MoveXSortedToAET( int YToMove );
    virtual void            DrawScanLine( int YToScan, GR::Graphic::ContextDescriptor& cdTarget, GR::u32 dwColor );
    void                    AdvanceAET();
    void                    XSortAET();

};



#endif // POLYGON_FILL_H



