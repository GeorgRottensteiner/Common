#ifndef _PJ_CLIPLIST_H
#define _PJ_CLIPLIST_H

#pragma once
#pragma warning ( disable : 4786 )

// #include <math.h>
// #include <iostream.h>
// #include <assert.h>
// #include <fstream>
// #include <iomanip>

#include <list>
#include <PJLib\Math\PJ_tupel.h>
#include <PJLib\PJ_rect.h>



namespace PJ
{
  typedef PJ::TCTupel< long > ScreenVector;
  typedef TCRect< ScreenVector > ClipRect;
  // typedef std:.list< Cliprect >     ClipList;




  
  class ClipList : public std::list< ClipRect >
  {
  public:

    void AddClipRect( int x1, int y1, int x2, int y2 )
    {
      int       x1_,
                y1_,
                x2_,
                y2_,
                sx1,
                sy1,
                sx2,
                sy2;

      if ( ( x2 < x1)
      ||   ( y2 < y1 ) )
      {
        return; // -> illegal
      }

      ClipList& List = *this;

	    // erst mal so speichern
      ClipRect crNew( ScreenVector( x1, y1 ),  ScreenVector( x2, y2 ) );
      ClipList clNew;
      clNew.push_back( crNew );

      ClipList::iterator cliOld(   List.begin() );
      ClipList::iterator cliendOld( List.end() );
      // for ( ; cliOld != cliendOld; ++cliOld )
      for ( ; cliOld != List.end(); )
	    {
        ClipRect& crOld = *cliOld;
        x1_       = crOld.E1.x;             // Existierende Box laden
        y1_       = crOld.E1.y;
        x2_       = crOld.E2.x;
        y2_       = crOld.E2.y;



        ClipList::iterator cliNew(   clNew.begin() );
        ClipList::iterator clendNew( clNew.end() );
        // for ( ; cliNew != clendNew; ++cliNew )
        for ( ; cliNew != clNew.end(); )
		    {
          ClipRect& crNew = *cliNew;
          x1 = crNew.E1.x;                // Neue Box laden
				  y1 = crNew.E1.y;
				  x2 = crNew.E2.x;
				  y2 = crNew.E2.y;

          if ( ( x1 > x2_ )
          ||   ( y1 > y2_ )
          ||   ( x2 < x1_ )
          ||   ( y2 < y1_ ) ) 
          {
            ++cliNew;
            continue;  // keine Ueberlagerung ?
          }
				  
          {
					  // Neue Box komplett weg ?
            if ( ( x1 >= x1_ )
            &&   ( y1 >= y1_ )
            &&   ( x2 <= x2_ )
            &&   ( y2 <= y2_ ) )
					  {
              cliNew = clNew.erase( cliNew );
              continue;
					  }
					  else
					  {
						  // Alte Box komplett weg ?
              if ( ( x1_ >= x1 )
              &&   ( y1_ >= y1 )
              &&   ( x2_ <= x2 )
              &&   ( y2_ <= y2 ) )
						  {
                cliOld = List.erase( cliOld );
                break;
						  }

						  // Schnittbox erzeugen!
              if ( x1 > x1_ ) sx1 = x1;
						  else            sx1 = x1_;

              if ( y1 > y1_ ) sy1 = y1;
						  else            sy1 = y1_;

              if ( x2 < x2_ ) sx2 = x2;
						  else            sx2 = x2_;

              if ( y2 < y2_ ) sy2 = y2;
						  else            sy2 = y2_;

              // oben Box erzeugen
              if ( y1 != sy1 )
                clNew.push_back( ClipRect( ScreenVector( x1, y1 ), ScreenVector( x2, sy1 - 1 ) ) );
						  
              // unten Box erzeugen
              if ( y2 != sy2 )
                clNew.push_back( ClipRect( ScreenVector( x1, sy2 + 1 ), ScreenVector( x2, y2 ) ) );
						  
              // Mittig Box links erzeugen
              if ( x1 != sx1 )
                clNew.push_back( ClipRect( ScreenVector( x1, sy1  ), ScreenVector( sx1 - 1, sy2 ) ) );
						  
              // Mittig Box rechts erzeugen
              if ( x2 != sx2 )
                clNew.push_back( ClipRect( ScreenVector( sx2 + 1, sy1  ), ScreenVector( x2, sy2 ) ) );
            
              // existierende immer wegwerfen
              cliNew = clNew.erase( cliNew );
              continue;
					  }
				  }
          ++cliNew;
        }
	      ++cliOld;
      }

      List.insert( List.end(), clNew.begin(), clNew.end() );
    }



    // void AddClipRect( int x1, int y1, int x2, int y2 )
    // {
      // int       x1_,
                // y1_,
                // x2_,
                // y2_,
                // sx1,
                // sy1,
                // sx2,
                // sy2;

      // if ( ( x2 < x1)
      // ||   ( y2 < y1 ) )
      // {
        // return; // -> illegal
      // }

      // ClipList& List = *this;

// 	    // erst mal so speichern
      // ClipRect crNew( ScreenVector( x1, y1 ),  ScreenVector( x2, y2 ) );
      // ClipList clNew;
      // clNew.push_back( crNew );

      // ClipList::iterator cliOld(   List.begin() );
      // ClipList::iterator cliendOld( List.end() );
      // // for ( ; cliOld != cliendOld; ++cliOld )
      // for ( ; cliOld != List.end(); )
// 	    {
        // ClipRect& crOld = *cliOld;
        // x1_       = crOld.E1.x;             // Existierende Box laden
        // y1_       = crOld.E1.y;
        // x2_       = crOld.E2.x;
        // y2_       = crOld.E2.y;



        // ClipList::iterator cliNew(   clNew.begin() );
        // ClipList::iterator clendNew( clNew.end() );
        // // for ( ; cliNew != clendNew; ++cliNew )
        // for ( ; cliNew != clNew.end(); )
// 		    {
          // ClipRect& crNew = *cliNew;
          // x1 = crNew.E1.x;                // Neue Box laden
// 				  y1 = crNew.E1.y;
// 				  x2 = crNew.E2.x;
// 				  y2 = crNew.E2.y;

          // if ( ( x1 > x2_ )
          // ||   ( y1 > y2_ )
          // ||   ( x2 < x1_ )
          // ||   ( y2 < y1_ ) ) 
          // {
            // ++cliNew;
            // continue;  // keine Ueberlagerung ?
          // }
// 				  
          // {
// 					  // Neue Box komplett weg ?
            // if ( ( x1 >= x1_ )
            // &&   ( y1 >= y1_ )
            // &&   ( x2 <= x2_ )
            // &&   ( y2 <= y2_ ) )
// 					  {
              // cliNew = clNew.erase( cliNew );
              // continue;
// 					  }
// 					  else
// 					  {
// 						  // Alte Box komplett weg ?
              // if ( ( x1_ >= x1 )
              // &&   ( y1_ >= y1 )
              // &&   ( x2_ <= x2 )
              // &&   ( y2_ <= y2 ) )
// 						  {
                // cliOld = List.erase( cliOld );
                // break;
// 						  }

// 						  // Schnittbox erzeugen!
              // if ( x1 > x1_ ) sx1 = x1;
// 						  else            sx1 = x1_;

              // if ( y1 > y1_ ) sy1 = y1;
// 						  else            sy1 = y1_;

              // if ( x2 < x2_ ) sx2 = x2;
// 						  else            sx2 = x2_;

              // if ( y2 < y2_ ) sy2 = y2;
// 						  else            sy2 = y2_;

              // // oben Box erzeugen
              // if ( y1 != sy1 )
                // clNew.push_back( ClipRect( ScreenVector( x1, y1 ), ScreenVector( x2, sy1 - 1 ) ) );
// 						  
              // // unten Box erzeugen
              // if ( y2 != sy2 )
                // clNew.push_back( ClipRect( ScreenVector( x1, sy2 + 1 ), ScreenVector( x2, y2 ) ) );
// 						  
              // // Mittig Box links erzeugen
              // if ( x1 != sx1 )
                // clNew.push_back( ClipRect( ScreenVector( x1, sy1  ), ScreenVector( sx1 - 1, sy2 ) ) );
// 						  
              // // Mittig Box rechts erzeugen
              // if ( x2 != sx2 )
                // clNew.push_back( ClipRect( ScreenVector( sx2 + 1, sy1  ), ScreenVector( x2, sy2 ) ) );
            
              // // existierende immer wegwerfen
              // cliNew = clNew.erase( cliNew );
              // continue;
// 					  }
// 				  }
          // ++cliNew;
        // }
// 	      ++cliOld;
      // }

      // List.insert( List.end(), clNew.begin(), clNew.end() );
    // }


  };


  ////////////////////////////////////////////////
  //-    Schnittmenge
  ////////////////////////////////////////////////
  //- Cliprects mueesen normalisiert sein!
  inline ClipList operator&( const ClipRect& cr1, const ClipRect& cr2 )
  {
    ClipRect crResult( cr1 );
    ClipList clTemp;
    
    for ( long i = cr1.E1.dimensions() - 1; i >= 0; --i ) 
    {
      // LogText( "i %d,  E1 %d E2 %d", i, crResult.E1[i], crResult.E2[i] );
      //- Überhaupt was zu schneiden?
      if ( crResult.E2[i] <= cr2.E1[i] || crResult.E1[i] >= cr2.E2[i] ) return clTemp;

      if ( crResult.E1[i] < cr2.E1[i] ) crResult.E1[i] = cr2.E1[i]; 
      if ( crResult.E2[i] > cr2.E2[i] ) crResult.E2[i] = cr2.E2[i]; 
    }
    
    if ( !crResult.empty() ) 
    {
      clTemp.push_back( crResult );
    }

    // if ( clTemp.empty() )
    // {
      // LogText( "clTemp ist leer" );
    // }

    return clTemp;
  }


    inline ClipList Differenz( const ClipRect& cr1, const ClipRect& cr2 )
  {
    int       x1_, y1_, x2_, y2_,
              x1,  y1,  x2,  y2,
              sx1, sy1, sx2, sy2;

    ClipList clNew;

    if ( cr1.empty() && cr2.empty() ) 
    {
      return clNew;
    }
    else if ( cr1.empty() ) 
    {
      clNew.push_back( cr2 );
      return clNew;
    }
    else if ( cr2.empty() )
    {
      clNew.push_back( cr1 );
      return clNew;
    }
    else if ( cr1.contains( cr2 ) )
    {
      clNew.push_back( cr1 );
      return clNew;
    }
    else if ( cr2.contains( cr1 ) )
    {
      clNew.push_back( cr2 );
      return clNew;
    }


    x1_ = cr1.E1.x;             // Existierende Box laden
    y1_ = cr1.E1.y;
    x2_ = cr1.E2.x;
    y2_ = cr1.E2.y;

    x1  = cr2.E1.x;                // Neue Box laden
		y1  = cr2.E1.y;
		x2  = cr2.E2.x;
		y2  = cr2.E2.y;

    clNew = cr1 & cr2;

    if ( clNew.empty() )
    {
      // keine Ueberlagerung 
      clNew.push_back( cr1 );
      clNew.push_back( cr2 );
      return clNew;
    }

    ClipRect crSchnitt( clNew.front() );
    clNew.clear();


    sx1  = crSchnitt.E1.x;                // Neue Box laden
		sy1  = crSchnitt.E1.y;
		sx2  = crSchnitt.E2.x;
		sy2  = crSchnitt.E2.y;


    // oben Box erzeugen
    if ( y1 != sy1 )
      clNew.push_back( ClipRect( ScreenVector( x1, y1 ), ScreenVector( x2, sy1 - 1 ) ) );
		
    // unten Box erzeugen
    if ( y2 != sy2 )
      clNew.push_back( ClipRect( ScreenVector( x1, sy2 + 1 ), ScreenVector( x2, y2 ) ) );
		
    // Mittig Box links erzeugen
    if ( x1 != sx1 )
      clNew.push_back( ClipRect( ScreenVector( x1, sy1  ), ScreenVector( sx1 - 1, sy2 ) ) );
		
    // Mittig Box rechts erzeugen
    if ( x2 != sx2 )
      clNew.push_back( ClipRect( ScreenVector( sx2 + 1, sy1  ), ScreenVector( x2, sy2 ) ) );
  
    return clNew;
  }
  
  inline ClipList Differenz( const ClipRect& cr1, ClipList& cl )
  {
    ClipList clResult;
    ClipList::iterator cli(   cl.begin() );
    ClipList::iterator clend( cl.end() );
    for ( ; cli != clend; ++cli )
    {
      ClipList clNew( cr1 & *cli );
      clResult.insert( clResult.end(), clNew.begin(), clNew.end() );
    }  
    return clResult;
  }

  inline ClipList operator-( const ClipRect& cr2, const ClipRect& cr1 )
  {
    ClipList clNew;

    if ( cr1.empty() && cr2.empty() ) 
    {
      return clNew;
    }
    else if ( cr2.empty() ) 
    {
      return clNew;
    }
    else if ( cr1.empty() )
    {
      clNew.push_back( cr2 );
      return clNew;
    }
    else if ( cr1.contains( cr2 ) )
    {
      return clNew;
    }


    int       x1_, y1_, x2_, y2_,
              x1,  y1,  x2,  y2,
              sx1, sy1, sx2, sy2;

    x1_ = cr1.E1.x;             // Existierende Box laden
    y1_ = cr1.E1.y;
    x2_ = cr1.E2.x;
    y2_ = cr1.E2.y;

    x1  = cr2.E1.x;                // Neue Box laden
		y1  = cr2.E1.y;
		x2  = cr2.E2.x;
		y2  = cr2.E2.y;

    clNew = cr1 & cr2;

    if ( clNew.empty() )
    {
      // keine Ueberlagerung 
      clNew.push_back( cr2 );
      return clNew;
    }

    ClipRect crSchnitt( clNew.front() );
    clNew.clear();


    sx1  = crSchnitt.E1.x;                // Neue Box laden
		sy1  = crSchnitt.E1.y;
		sx2  = crSchnitt.E2.x;
		sy2  = crSchnitt.E2.y;


    // oben Box erzeugen
    if ( y1 != sy1 )
      clNew.push_back( ClipRect( ScreenVector( x1, y1 ), ScreenVector( x2, sy1 - 1 ) ) );
		
    // unten Box erzeugen
    if ( y2 != sy2 )
      clNew.push_back( ClipRect( ScreenVector( x1, sy2 + 1 ), ScreenVector( x2, y2 ) ) );
		
    // Mittig Box links erzeugen
    if ( x1 != sx1 )
      clNew.push_back( ClipRect( ScreenVector( x1, sy1  ), ScreenVector( sx1 - 1, sy2 ) ) );
		
    // Mittig Box rechts erzeugen
    if ( x2 != sx2 )
      clNew.push_back( ClipRect( ScreenVector( sx2 + 1, sy1  ), ScreenVector( x2, sy2 ) ) );
  
    return clNew;
  }

  
  inline ClipList operator-( ClipList& clSubtrahend, const ClipRect& crSubtraktor )
  {
    ClipList clResult( clSubtrahend );

    ClipList::iterator cliSubtrahend(   clSubtrahend.begin() );
    ClipList::iterator clendSubtrahend( clSubtrahend.end() );
    for ( ; cliSubtrahend != clendSubtrahend; ++cliSubtrahend )
    {
      ClipList clDifferenz( *cliSubtrahend - crSubtraktor );
      clResult.insert( cliSubtrahend = clResult.erase( cliSubtrahend ), clDifferenz.begin(), clDifferenz.end() );
    }  

    return clResult;
  }


  inline ClipList operator-( ClipList& clSubtrahend, ClipList& clSubtraktor )
  {
    ClipList clResult;

    ClipList::iterator cliSubtraktor(   clSubtraktor.begin() );
    ClipList::iterator clendSubtraktor( clSubtraktor.end() );
    for ( ; cliSubtraktor != clendSubtraktor; ++cliSubtraktor )
    {
      ClipList clDifferenz( clSubtrahend - *cliSubtraktor );
      clResult.insert( clResult.end(), clDifferenz.begin(), clDifferenz.end() );
    }  

    return clResult;
  }

  inline ClipList operator-( const ClipRect& crSubtrahend, ClipList& clSubtraktor )
  {
    ClipList clSubtrahend;
    clSubtrahend.push_back( crSubtrahend );
    ClipList clResult( clSubtrahend - clSubtraktor );
    return clResult;
  }


  // inline ClipList operator|( ClipList& clOld, ClipList& clNew )
  // {
    // ClipList clResult( clNew );

    // ClipList::iterator cliOld(   clOld.begin() );
    // ClipList::iterator clendOld( clOld.end() );
    // for ( ; cliOld != clendOld; ++cliOld )
    // {
      // ClipList::iterator cliResult(   clResult.begin() );
      // ClipList::iterator clendResult( clResult.end() );
      // for ( ; cliResult != clendResult; ++cliResult )
      // {
        // ClipList clDifferenz( *cliResult - *cliSubtraktor );
        // clResult.insert( clResult.end(), clDifferenz.begin(), clDifferenz.end() );
      // }  
    // }  
  // }

  inline ClipList operator|( ClipList& clOld, const ClipRect& crNew )
  {
    ClipList clResult( clOld );
    if ( clOld.empty() )
    {
      clResult.push_back( crNew );
      return clResult;
    }

    ClipList clDifferenz( crNew - clOld );
    clResult.insert( clResult.end(), clDifferenz.begin(), clDifferenz.end() );
    return clResult;
  }






  ////////////////////////////////////////////////
  //-    Vereingungsmenge
  ////////////////////////////////////////////////
  inline ClipList operator|( const ClipRect& cr1, const ClipRect& cr2 )
  {
    ClipList clNew;
    
    if ( cr2.contains( cr1 ) )
    {
      clNew.push_back( cr2 );
      return clNew;
    }
    
    clNew = cr2 - cr1;
    clNew.push_front( cr1 );
    return clNew;
  }















};



#endif //_PJ_CLIPLIST_H