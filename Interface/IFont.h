#ifndef IFONT_INCLUDED
#define IFONT_INCLUDED

#include <GR/GRTypes.h>

#include <Interface/IString.h>


namespace Interface
{

  struct IFont
  {

    public:

      virtual int TextLength( const GR::String& ) const
      {
        return 0;
      }

      virtual int TextHeight( const GR::String& = GR::String() ) const   // mit NULL gibt's die Standardhöhe
      {
        return 0;
      }

      virtual int FontSpacing()                             // Abstand zwischen Zeichen
      {
        return 0;
      }

      virtual ~IFont()
      {
      }
  };

}

#endif// IFONT_INCLUDED

