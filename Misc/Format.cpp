#include <String/Convert.h>

#include "Format.h"

#include <OS/OS.h>

// disable warning in std::vector
#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn (1506)
#endif


namespace Misc
{
  const GR::String CFormat::Result() const
  {
    GR::String      result;

    size_t          pos = 0;

    bool            insideParameter = false;

    GR::String      currentParameter;


    while ( pos < m_Content.length() )
    {
      GR::Char    currentChar = m_Content[pos];
      switch ( currentChar )
      {
        case '%':
          if ( insideParameter )
          {
            if ( currentParameter.empty() )
            {
              result += '%';
            }
            else
            {
              size_t    paramPos = 0;
              int       value = 0;
              int       paramIndex = 0;
              size_t    paramNumber = 0;
              int       paramDigits = 0;
              size_t    paramAfterCommaDigits = 0;
              GR::Char  fillChar = 0;
              bool      forceHex = false;
              bool      valueNegative = false;

              while ( paramPos < currentParameter.length() )
              {
                GR::Char    currentParamChar = currentParameter[paramPos];

                if ( ( ( currentParamChar >= '1' )
                &&     ( currentParamChar <= '9' ) )
                ||   ( ( currentParamChar == '0' )
                &&     ( paramPos != 0 ) )
                ||   ( currentParamChar == '-' ) )
                {
                  if ( currentParamChar == '-' )
                  {
                    valueNegative = true;
                  }
                  else
                  {
                    value *= 10;
                    value += currentParamChar - '0';
                  }
                }
                else if ( currentParamChar == ':' )
                {
                  if ( valueNegative )
                  {
                    value = -value;
                  }
                  switch ( paramIndex )
                  {
                    case 0:
                      paramNumber = value;
                      break;
                    case 1:
                      paramDigits = value;
                      break;
                    case 2:
                      paramAfterCommaDigits = value;
                      break;
                  }
                  value = 0;
                  valueNegative = false;
                  ++paramIndex;
                }
                else if ( currentParamChar == 'x' )
                {
                  forceHex = true;
                }
                else if ( paramPos == 0 )
                {
                  fillChar = currentParamChar;
                }

                ++paramPos;
              }

              if ( valueNegative )
              {
                value = -value;
              }

              switch ( paramIndex )
              {
                case 0:
                  paramNumber = value;
                  break;
                case 1:
                  paramDigits = value;
                  break;
                case 2:
                  paramAfterCommaDigits = value;
                  break;
              }
              if ( ( paramNumber <= m_Parameters.size() )
              &&   ( paramNumber > 0 ) )
              {
                const tParam&     Param = m_Parameters[paramNumber - 1];
                GR::String        paramContent = Param.Text;

                if ( Param.Type == tParam::PT_DOUBLE )
                {
                  paramContent.erase();

                  GR::f64     doubleValue = Param.DoubleValue;
                  GR::i64     value = (GR::i64)doubleValue;

                  if ( value < 0 )
                  {
                    value = -value;
                    doubleValue = -doubleValue;
                    if ( paramDigits )
                    {
                      paramDigits--;
                    }
                  }
                  if ( value == 0 )
                  {
                    paramContent = "0";
                  }
                  while ( value > 0 )
                  {
                    paramContent = (GR::Char)( ( value % 10 ) + '0' ) + paramContent;
                    value /= 10;
                  }
                  bool    clipToRight = false;
                  if ( paramDigits < 0 )
                  {
                    // die letzten X Stellen
                    clipToRight = true;
                    paramDigits = -paramDigits;
                  }

                  if ( ( paramDigits > 0 )
                  &&   ( fillChar ) )
                  {
                    while ( paramContent.length() < (size_t)paramDigits )
                    {
                      paramContent = fillChar + paramContent;
                    }
                    if ( Param.DoubleValue < 0 )
                    {
                      paramContent = "-" + paramContent;
                    }
                  }
                  else if ( paramDigits > 0 )
                  {
                    // fill up
                    if ( Param.DoubleValue < 0 )
                    {
                      paramContent = "-" + paramContent;
                    }

                    while ( paramContent.length() < (size_t)paramDigits )
                    {
                      paramContent += " ";
                    }
                  }
                  else if ( Param.DoubleValue < 0 )
                  {
                    paramContent = "-" + paramContent;
                  }

                  if ( clipToRight )
                  {
                    // die letzten iParamStellen der Anzeige!
                    if ( paramContent.length() > (size_t)paramDigits )
                    {
                      paramContent = paramContent.substr( paramContent.length() - paramDigits );
                    }
                  }

                  if ( paramAfterCommaDigits != 0 )
                  {
                    paramContent += ".";
                    if ( doubleValue < 0.0 )
                    {
                      doubleValue = -doubleValue;
                    }
                    double intPart;
                    doubleValue = modf( doubleValue, &intPart );
                    while ( paramAfterCommaDigits )
                    {
                      doubleValue *= 10;
                      if ( paramAfterCommaDigits == 1 )
                      {
                        if ( (char)( (int)fmod( doubleValue * 10, 10 ) + '0' ) >= '5' )
                        {
                          doubleValue += 1;
                        }
                      }
                      paramContent += (char)( (int)fmod( doubleValue, 10 ) + '0' );
                      --paramAfterCommaDigits;
                    }
                  }
                }
                else
                {
                  bool    clipToRight = false;
                  if ( paramDigits < 0 )
                  {
                    // die letzten X Stellen
                    clipToRight = true;
                    paramDigits = -paramDigits;
                  }

                  if ( forceHex )
                  {
                    GR::u64 value = GR::Convert::ToU64( paramContent );
                    paramContent  = GR::Convert::ToHex( value, paramDigits );
                  }
                  if ( ( paramDigits != 0 )
                  &&   ( fillChar ) )
                  {
                    while ( (size_t)paramDigits > paramContent.length() )
                    {
                      if ( ( paramContent[0] == '-' )
                      &&   ( fillChar == '0' ) )
                      {
                        // bei Zahlen das Minus vorne lassen
                        paramContent = GR::String( "-" ) + fillChar + paramContent.substr( 1 );
                      }
                      else
                      {
                        paramContent = fillChar + paramContent;
                      }
                    }
                  }
                  else if ( paramDigits > 0 )
                  {
                    // fill up
                    while ( paramContent.length() < (size_t)paramDigits )
                    {
                      paramContent += " ";
                    }
                  }
                  if ( clipToRight )
                  {
                    // die letzten iParamStellen der Anzeige!
                    if ( paramContent.length() > (size_t)paramDigits )
                    {
                      paramContent = paramContent.substr( paramContent.length() - paramDigits );
                    }
                  }
                }
                result += paramContent;
              }
            }

            insideParameter = false;
          }
          else
          {
            insideParameter = true;
            currentParameter.erase();
          }
          break;
        default:
          if ( !insideParameter )
          {
            result += currentChar;
          }
          else
          {
            currentParameter += currentChar;
          }
          break;
      }
      ++pos;
    }
    return result;
  }

  CFormat& CFormat::Format( const GR::String& FormatString )
  {
    m_Content = FormatString;

    m_Parameters.clear();

    return *this;
  }



  CFormat::operator GR::String() const
  {
    return Result();
  }



  void CFormat::AddParam( const tParam& Param )
  {
    m_Parameters.push_back( Param );
  }

  

  CFormat FormatA( const GR::String& FormatString )
  {
    CFormat     Formatter;

    Formatter.Format( FormatString );

    return Formatter;
  }



  CFormat Format( const GR::String& FormatString )
  {
    CFormat     Formatter;

    Formatter.Format( FormatString );

    return Formatter;
  }



  // zweite Generation
  CFormat operator<< ( CFormat Format, const GR::String& Param )
  {
    Format.AddParam( CFormat::tParam( Param ) );

    return Format;
  }



  CFormat operator<< ( CFormat Format, const GR::Char* Param )
  {
    Format.AddParam( CFormat::tParam( Param ) );

    return Format;
  }



  CFormat operator<< ( CFormat Format, const GR::Char Param )
  {
    GR::String    temp;

    temp.append( Param );
    Format.AddParam( CFormat::tParam( temp ) );

    return Format;
  }



  CFormat operator<< ( CFormat Format, const GR::ip Param )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( (GR::i64)Param ) ) );

    return Format;
  }



#if OPERATING_SYSTEM == OS_TANDEM
  // Tandem cross compiler fails to see that int = long
  CFormat operator<< ( CFormat Format, const long iParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( iParam ) ) );

    return Format;
  }
#endif



#if OS_ENVIRONMENT == OS_ENVIRONMENT_64
  CFormat operator<< ( CFormat Format, const GR::i32 iParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( iParam ) ) );

    return Format;
  }



  CFormat operator<< ( CFormat Format, const GR::u32 iParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( (GR::i64)iParam ) ) );

    return Format;
  }

#elif OS_ENVIRONMENT == OS_ENVIRONMENT_32

  CFormat operator<< ( CFormat Format, const GR::i64 iParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( iParam ) ) );

    return Format;
  }



  CFormat operator<< ( CFormat Format, const GR::u64 iParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( (GR::i64)iParam ) ) );

    return Format;
  }
#endif



  CFormat operator<< ( CFormat Format, const GR::up dwParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( (GR::u64)dwParam ) ) );

    return Format;
  }

#if OPERATING_SYSTEM == OS_WEB
  CFormat operator<< ( CFormat Format, const size_t dwParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( ( GR::u64 )dwParam ) ) );

    return Format;
  }
#endif

  CFormat operator<< ( CFormat Format, const GR::f64 dParam )
  {
    Format.AddParam( CFormat::tParam( dParam ) );

    return Format;
  }

  CFormat operator<< ( CFormat Format, const bool bParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( bParam ? (GR::u64)1 : (GR::u64)0 ) ) );

    return Format;
  }


}



