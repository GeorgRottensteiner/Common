#pragma once



#include <GR/GRTypes.h>

#include <vector>



namespace GR
{
  namespace Graphic
  {
    class ContextDescriptor;

    class PolygonFill
    {

      protected:

        struct EdgeState
        {
          EdgeState*    pNextEdge = NULL;
          int           X = 0;
          int           StartY = 0;
          int           WholePixelXMove = 0;
          int           XDirection = 0;
          int           ErrorTerm = 0;
          int           ErrorTermAdjUp = 0;
          int           ErrorTermAdjDown = 0;
          int           Count = 0;
        };

        EdgeState*      pGETPtr;
        EdgeState*      _pActiveEdgeTable;


      public:

        typedef std::pair<GR::tFPoint, GR::tFPoint>   tLine;

        typedef std::vector<tLine>                    tEdges;


        void                    FillPolygon( GR::Graphic::ContextDescriptor& cdTarget, const tEdges& Edges, GR::u32 Color );


      protected:

        void                    BuildFloatGET( const tEdges& Lines, EdgeState* pNextFreeEdgeStruc );
        void                    MoveXSortedToAET( int YToMove );
        virtual void            DrawScanLine( int YToScan, GR::Graphic::ContextDescriptor& cdTarget, GR::u32 Color );
        void                    AdvanceAET();
        void                    XSortAET();

    };

  }

}




