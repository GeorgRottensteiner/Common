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

        GR::String  m_Text;
        GR::f64         m_dblValue;
        eType           m_Type;

        tParam( const GR::String& strType ) :
          m_Text( strType ),
          m_dblValue( 0 ),
          m_Type( PT_STRING )
        {
        }

        tParam( const double dblValue ) :
          m_dblValue( dblValue ),
          m_Type( PT_DOUBLE )
        {
          char    szTemp[309 + 40 + 2];

#if OPERATING_SYSTEM == OS_WINDOWS
          _gcvt_s( szTemp, 309 + 40 + 2, dblValue, 30 );
#else
          //sprintf_s( szTemp, 309 + 40 + 2, "%f", dblValue );
          sprintf( szTemp, "%f", dblValue );
#endif
          m_Text = szTemp;
        }

      };

      GR::String                m_Content;


    public:

      std::vector<tParam>       m_vectParameters;


      const GR::String Result() const;

      CFormat& Format( const GR::String& strFormatString );

      operator GR::String() const;

      friend CFormat operator<< ( CFormat Format, const GR::String& strParam );
      friend CFormat operator<< ( CFormat Format, const GR::WString& strParam );
      friend CFormat operator<< ( CFormat Format, const char* szParam );
      friend CFormat operator<< ( CFormat Format, const GR::ip iParam );

#if OPERATING_SYSTEM == OS_TANDEM
      friend CFormat operator<< ( CFormat Format, const long iParam );
#endif

#if OS_ENVIRONMENT == OS_ENVIRONMENT_32
      friend CFormat operator<< ( CFormat Format, const GR::i64 iParam );
      friend CFormat operator<< ( CFormat Format, const GR::u64 iParam );
#elif OS_ENVIRONMENT == OS_ENVIRONMENT_64
      friend CFormat operator<< ( CFormat Format, const GR::i32 iParam );
      friend CFormat operator<< ( CFormat Format, const GR::u32 iParam );
#endif
      friend CFormat operator<< ( CFormat Format, const GR::up iParam );
#if OPERATING_SYSTEM == OS_WEB
      friend CFormat operator<< ( CFormat Format, const size_t iParam );
#endif
      friend CFormat operator<< ( CFormat Format, const GR::f64 iParam );
      friend CFormat operator<< ( CFormat Format, const bool bParam );

      void AddParam( const tParam& Param );
  };


  CFormat   Format( const GR::String& FormatString = "%1%" );

  CFormat operator<< ( CFormat Format, const GR::String& Param );


  // zweite Generation
  CFormat operator<< ( CFormat Format, const GR::String& strParam );
  CFormat operator<< ( CFormat Format, const GR::Char* Param );
  CFormat operator<< ( CFormat Format, const GR::up dwParam );
  CFormat operator<< ( CFormat Format, const size_t dwParam );
  CFormat operator<< ( CFormat Format, const GR::ip iParam );
#if OS_ENVIRONMENT == OS_ENVIRONMENT_32
  CFormat operator<< ( CFormat Format, const GR::i64 iParam );
  CFormat operator<< ( CFormat Format, const GR::u64 iParam );
#elif OS_ENVIRONMENT == OS_ENVIRONMENT_64
  CFormat operator<< ( CFormat Format, const GR::i32 iParam );
  CFormat operator<< ( CFormat Format, const GR::u32 iParam );
#endif
  CFormat operator<< ( CFormat Format, const GR::f64 dParam );
  CFormat operator<< ( CFormat Format, const bool bParam );

}








#endif // FORMAT_H
