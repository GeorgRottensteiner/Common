#ifndef _PJ_HELP_GRAPHICS_H_INCLUDED
#define _PJ_HELP_GRAPHICS_H_INCLUDED

namespace PJ
{
  namespace HELP
  {
    namespace GFX
    {
      //- Hilfsroutinen für Farbberechnungen Pixelformat 555

      //- Farbanteil aus unsigned long holen
      inline WORD RedComponent555(    const WORD ulSourceColor )
      { return ( ulSourceColor & COLOR_MASK_RED ) >> 10; }
      inline WORD GreenComponent555(  const WORD ulSourceColor )
      { return ( ulSourceColor & COLOR_MASK_GREEN ) >> 5; }
      inline WORD BlueComponent555(   const WORD ulSourceColor )
      { return ulSourceColor & COLOR_MASK_BLUE; }

      //- Komponente Rot, Grün oder Blau machen
      inline WORD ComponentToRed555(    const WORD ulSourceComponent )
      { return ( ulSourceComponent << 10 ) & COLOR_MASK_RED; }
      inline WORD ComponentToGreen555(    const WORD ulSourceComponent )
      { return ( ulSourceComponent << 5 ) & COLOR_MASK_GREEN; }
      inline WORD ComponentToBlue555(    const WORD ulSourceComponent )
      { return ulSourceComponent & COLOR_MASK_BLUE; }

      //- Farbanteil alpharisieren
      inline WORD AlphaComponent555( const WORD ulSourceComponent,
                                              const WORD ulDestComponent,
                                              const WORD ulSourceAlpha )
      {
         return ( ulDestComponent * ( 32 - ulSourceAlpha ) + ulSourceComponent * ulSourceAlpha ) >> 5;
         // return ( ulDestComponent * ( 31 - ulSourceAlpha ) + ulSourceComponent * ulSourceAlpha ) >> 5;
      }


      inline WORD AlphaRed555( const WORD ulSourceColor,
                                        const WORD ulDestColor,
                                        const WORD ulSourceAlpha )
      {
         return ComponentToRed555( AlphaComponent555( RedComponent555( ulSourceColor ),
                                                      RedComponent555( ulDestColor   ),
                                                      ulSourceAlpha                     ) );
      }

      inline WORD AlphaGreen555( const WORD ulSourceColor,
                                        const WORD ulDestColor,
                                        const WORD ulSourceAlpha )
      {
         return ComponentToGreen555( AlphaComponent555( GreenComponent555( ulSourceColor ),
                                                      GreenComponent555( ulDestColor   ),
                                                      ulSourceAlpha                     ) );
      }

      inline WORD AlphaBlue555( const WORD ulSourceColor,
                                        const WORD ulDestColor,
                                        const WORD ulSourceAlpha )
      {
         return ComponentToBlue555( AlphaComponent555( BlueComponent555( ulSourceColor ),
                                                      BlueComponent555( ulDestColor   ),
                                                      ulSourceAlpha                     ) );
      }

      inline WORD Alpha555( const WORD ulSourceColor,
                                     const WORD ulDestColor,
                                     const WORD ulSourceAlpha )
      {
        return    AlphaRed555(    ulSourceColor, ulDestColor, ulSourceAlpha )
               |  AlphaGreen555(  ulSourceColor, ulDestColor, ulSourceAlpha )
               |  AlphaBlue555(   ulSourceColor, ulDestColor, ulSourceAlpha );
      }












    }; // namespace GFX
  }; // namespace HELP
}; // namespace PJ
#endif//_PJ_HELP_GRAPHICS_H_INCLUDED
