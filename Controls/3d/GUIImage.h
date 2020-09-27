#ifndef __GUI_IMAGE_H__
#define __GUI_IMAGE_H__
/*--------------------+-------------------------------------------------------+
 | Programmname       : TIB-Controls                                          |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 11.10.99                                              |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <string>
#include <vector>

#include "GUIComponent.h"



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CD3DViewer;



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CGUIImage : public CGUIComponent
{

  protected:

    tTextureSection     m_TexSection;


  public:


    CGUIImage( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId = 0 );


    virtual void        DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY );

    void                SetImage( const tTextureSection& TexSec );

    virtual ICloneAble* Clone();

};


/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#endif // __GUI_IMAGE_H__



