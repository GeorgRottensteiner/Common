#ifndef PUNY_CODE_H
#define PUNY_CODE_H

#include <OS/OS.h>

// disable warning in std::vector
#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn (1506)
#endif


#include <list>

#include <GR/GRTypes.h>


#if OPERATING_SYSTEM == OS_TANDEM
#pragma warn (1506)
#endif


class PunyCode
{

  public:

    static bool             Encode( const GR::WString& Input, GR::String& Result );

    // takes a full URL and only Punifies the domain
    static bool             EncodeDomain( const GR::WString& FullURL, GR::String& Result );

    static bool             Decode( const GR::String& Input, GR::WString& Result );


  private:

    enum
    {
      base = 36,
      tmin = 1,
      tmax = 26,
      skew = 38,
      damp = 700,
      initial_bias = 72,
      initial_n = 0x80,
      delimiter = 0x2D
    };


    static char             PunyEncodeBasic( unsigned short bcp, bool Uppercase );
    static char             EncodeDigit( unsigned short d, bool Uppercase );
    static unsigned short   DecodeDigit( unsigned short cp );
    static unsigned short   Adapt( unsigned short delta, unsigned short numpoints, int firsttime );

    static void             Split( const GR::WString& strSource, const wchar_t cSeparator, std::list<GR::WString>& listParts );



};


#endif // PUNYCODE_H
