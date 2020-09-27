#ifndef _TRIPLE_FLOAT_H_INCLUDED
#define _TRIPLE_FLOAT_H_INCLUDED

#pragma message ( __FILE__ ": Defining types for PJ::tCTriple<float> in global namespace" )

#pragma warning ( disable : 4786 )
#pragma once

#include <PJLib\Math\PJ_triple.h>

////////////////////////////////////////////////
//-    triple-typedefs für floats
////////////////////////////////////////////////
typedef float CScalar;                           
typedef PJ::tCTriple<     CScalar > CVector;         //- 3D-Vektor
typedef PJ::tCPlane<      CScalar > CPlane;          //- 3D-Ebene
typedef PJ::tCMatrix<     CScalar > CMatrix;         //- 3D-Matrix 
typedef PJ::tCBasis<      CScalar > CBasis;          //- 3D-Winkelausrichtung  
typedef PJ::tCCoordFrame< CScalar > CCoordFrame;     //- 3D-Bezugssystem 

//- Das gibt's zwar noch aber bitte nicht mehr benutzen!!!
//typedef PJ::tCBlock<      CScalar > CBox;          //- 3D-Rechteck (deprecated!!!) 
//- stattdessen:
#include <PJLib\Math\PJ_rect.h>
typedef PJ::tCRect< CVector, CScalar > C_Rect;       //- 3D-Rechteck (neu!!) 
//- C_Rect, damits kein Problem gibt mit MFC-CRect

#endif//_TRIPLE_FLOAT_H_INCLUDED

