#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <vector>

#include <stdlib.h>

#include <GR/GRTypes.h>

#include <OS/OS.h>

// for _T
#include <Misc/Misc.h>

#include <Interface/IString.h>



namespace Misc
{
  class CFormat
  {
    protected:

      struct tParam
      {
        enum eType
        {
          PT_STRING,
          PT_DOUBLE,
          PT_HEX
        };

        GR::String      Text;
        GR::f64         DoubleValue;
        eType           Type;

        tParam( const GR::String& strType ) :
          Text( strType ),
          DoubleValue( 0 ),
          Type( PT_STRING )
        {
        }

        tParam( const double dblValue ) :
          DoubleValue( dblValue ),
          Type( PT_DOUBLE )
        {
          char    temp[309 + 40 + 2];

#if OPERATING_SYSTEM == OS_WINDOWS
          _gcvt_s( temp, 309 + 40 + 2, dblValue, 30 );
#else
          //sprintf_s( szTemp, 309 + 40 + 2, "%f", dblValue );
          sprintf( temp, "%f", dblValue );
#endif
          Text = temp;
        }

      };

      GR::String                m_Content;


    public:

      std::vector<tParam>       m_Parameters;


      const GR::String Result() const;

      CFormat& Format( const GR::String& FormatString );

      operator GR::String() const;

      friend CFormat operator<< ( CFormat Format, const GR::String& Param );
      friend CFormat operator<< ( CFormat Format, const GR::Char* Param );
      friend CFormat operator<< ( CFormat Format, const GR::Char Param );
      friend CFormat operator<< ( CFormat Format, const GR::ip Param );

#if OPERATING_SYSTEM == OS_TANDEM
      friend CFormat operator<< ( CFormat Format, const long Param );
#endif

#if OS_ENVIRONMENT == OS_ENVIRONMENT_32
      friend CFormat operator<< ( CFormat Format, const GR::i64 Param );
      friend CFormat operator<< ( CFormat Format, const GR::u64 Param );
#elif OS_ENVIRONMENT == OS_ENVIRONMENT_64
      friend CFormat operator<< ( CFormat Format, const GR::i32 Param );
      friend CFormat operator<< ( CFormat Format, const GR::u32 Param );
#endif
      friend CFormat operator<< ( CFormat Format, const GR::up Param );
#if OPERATING_SYSTEM == OS_WEB
      friend CFormat operator<< ( CFormat Format, const size_t Param );
#endif
      friend CFormat operator<< ( CFormat Format, const GR::f64 Param );
      friend CFormat operator<< ( CFormat Format, const bool Param );

      void AddParam( const tParam& Param );
  };


  CFormat   Format( const GR::String& FormatString = "%1%" );

  CFormat operator<< ( CFormat Format, const GR::String& Param );


  // zweite Generation
  CFormat operator<< ( CFormat Format, const GR::String& Param );
  CFormat operator<< ( CFormat Format, const GR::Char* Param );
  CFormat operator<< ( CFormat Format, const GR::Char Param );
  CFormat operator<< ( CFormat Format, const GR::up Param );
  CFormat operator<< ( CFormat Format, const size_t Param );
  CFormat operator<< ( CFormat Format, const GR::ip Param );
#if OS_ENVIRONMENT == OS_ENVIRONMENT_32
  CFormat operator<< ( CFormat Format, const GR::i64 Param );
  CFormat operator<< ( CFormat Format, const GR::u64 Param );
#elif OS_ENVIRONMENT == OS_ENVIRONMENT_64
  CFormat operator<< ( CFormat Format, const GR::i32 Param );
  CFormat operator<< ( CFormat Format, const GR::u32 Param );
#endif
  CFormat operator<< ( CFormat Format, const GR::f64 Param );
  CFormat operator<< ( CFormat Format, const bool Param );

}








#endif // FORMAT_H
