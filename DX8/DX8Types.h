#ifndef __DX8TYPES_H__
#define __DX8TYPES_H__
/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Math\vector2t.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#ifndef tSize
typedef math::vector2t<int>   tSize;
#endif

namespace DX8
{
  enum eTextureFlags
  {
    TF_DEFAULT           = 0x00000001,     // Standard - massive Textur
    TF_HOLES             = 0x00000002,     // Textur enthält Löcher (1-Bit-Alpha)
    TF_ALPHA_MASK        = 0x00000004,     // Textur enthält Alpha-Maske (mehr als 1-Bit-Alpha)
    TF_CONVERT_TO_ALPHA  = 0x00000008,     // 8bit-Textur wird zu Alpha-Maske
    TF_SQUARED           = 0x00000010,     // Quadr. Font-Texture a. NeHe (2 Fonts in 16x8-Anordnung)
    TF_SQUARED_ONE_FONT  = 0x00000020,     // Squared, aber nur ein Font drin
    TF_DONT_LOAD         = 0x40000000,     // Textur wird nur eingetragen, aber noch nicht geladen
    TF_FROM_RESOURCE     = 0x80000000,     // Textur aus Ressourcen geladen
  };
};


#endif // __DX8TYPES_H__