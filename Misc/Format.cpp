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
    GR::String      strResult;

    size_t          iPos = 0;

    bool            bParam = false;

    GR::String      strParam;


    while ( iPos < m_Content.length() )
    {
      GR::Char    cDummy = m_Content[iPos];
      switch ( cDummy )
      {
        case '%':
          if ( bParam )
          {
            if ( strParam.empty() )
            {
              strResult += '%';
            }
            else
            {
              size_t    iParamPos = 0;

              int       iValue = 0;

              int       iParamIndex = 0;

              size_t    iParamNumber = 0;

              int       iParamStellen = 0;
              size_t    iParamNachkommaStellen = 0;

              GR::Char  cFillChar = 0;

              bool      bForceHex = false;

              bool      bValueNegative = false;

              while ( iParamPos < strParam.length() )
              {
                GR::Char    cParamDummy = strParam[iParamPos];

                if ( ( ( cParamDummy >= '1' )
                &&     ( cParamDummy <= '9' ) )
                ||   ( ( cParamDummy == '0' )
                &&     ( iParamPos != 0 ) )
                ||   ( cParamDummy == '-' ) )
                {
                  if ( cParamDummy == '-' )
                  {
                    bValueNegative = true;
                  }
                  else
                  {
                    iValue *= 10;
                    iValue += cParamDummy - '0';
                  }
                }
                else if ( cParamDummy == ':' )
                {
                  if ( bValueNegative )
                  {
                    iValue = -iValue;
                  }
                  switch ( iParamIndex )
                  {
                    case 0:
                      iParamNumber = iValue;
                      break;
                    case 1:
                      iParamStellen = iValue;
                      break;
                    case 2:
                      iParamNachkommaStellen = iValue;
                      break;
                  }
                  iValue = 0;
                  bValueNegative = false;
                  ++iParamIndex;
                }
                else if ( cParamDummy == 'x' )
                {
                  bForceHex = true;
                }
                else if ( iParamPos == 0 )
                {
                  cFillChar = cParamDummy;
                }

                ++iParamPos;
              }

              if ( bValueNegative )
              {
                iValue = -iValue;
              }

              switch ( iParamIndex )
              {
                case 0:
                  iParamNumber = iValue;
                  break;
                case 1:
                  iParamStellen = iValue;
                  break;
                case 2:
                  iParamNachkommaStellen = iValue;
                  break;
              }
              if ( ( iParamNumber <= m_vectParameters.size() )
              &&   ( iParamNumber > 0 ) )
              {
                const tParam&     Param = m_vectParameters[iParamNumber - 1];

                GR::String        strParamContent = Param.m_Text;

                if ( Param.m_Type == tParam::PT_DOUBLE )
                {
                  strParamContent.erase();

                  GR::f64     dblValue = Param.m_dblValue;

                  GR::i64     iValue = (GR::i64)dblValue;

                  if ( iValue < 0 )
                  {
                    iValue = -iValue;
                    dblValue = -dblValue;
                    if ( iParamStellen )
                    {
                      iParamStellen--;
                    }
                  }
                  if ( iValue == 0 )
                  {
                    strParamContent = "0";
                  }
                  while ( iValue > 0 )
                  {
                    strParamContent = (GR::Char)( ( iValue % 10 ) + '0' ) + strParamContent;
                    iValue /= 10;
                  }
                  if ( Param.m_dblValue < 0 )
                  {
                    strParamContent = "-" + strParamContent;
                  }

                  bool    bClipToRight = false;
                  if ( iParamStellen < 0 )
                  {
                    // die letzten X Stellen
                    bClipToRight = true;
                    iParamStellen = -iParamStellen;
                  }


                  if ( ( iParamStellen > 0 )
                  &&   ( cFillChar ) )
                  {
                    while ( strParamContent.length() < (size_t)iParamStellen )
                    {
                      strParamContent = cFillChar + strParamContent;
                    }
                  }
                  else if ( iParamStellen > 0 )
                  {
                    // nach hinten auffüllen
                    while ( strParamContent.length() < (size_t)iParamStellen )
                    {
                      strParamContent += " ";
                    }
                  }
                  if ( bClipToRight )
                  {
                    // die letzten iParamStellen der Anzeige!
                    if ( strParamContent.length() > (size_t)iParamStellen )
                    {
                      strParamContent = strParamContent.substr( strParamContent.length() - iParamStellen );
                    }
                  }

                  if ( iParamNachkommaStellen != 0 )
                  {
                    strParamContent += ".";

                    if ( dblValue < 0.0 )
                    {
                      dblValue = -dblValue;
                    }

                    while ( iParamNachkommaStellen )
                    {
                      dblValue *= 10;
                      strParamContent += (char)( (int)fmod( dblValue, 10 ) + '0' );
                      //strParamContent += (char)( ( ( (int)dblValue ) % 10 ) + '0' );
                      --iParamNachkommaStellen;
                    }
                  }
                }
                else
                {
                  bool    bClipToRight = false;
                  if ( iParamStellen < 0 )
                  {
                    // die letzten X Stellen
                    bClipToRight = true;
                    iParamStellen = -iParamStellen;
                  }

                  if ( bForceHex )
                  {
                    GR::u64 iValue = GR::Convert::ToU64( strParamContent );

                    strParamContent = GR::Convert::ToHexA( iValue, iParamStellen );
                  }
                  if ( ( iParamStellen != 0 )
                  &&   ( cFillChar ) )
                  {
                    while ( (size_t)iParamStellen > strParamContent.length() )
                    {
                      if ( ( strParamContent[0] == '-' )
                      &&   ( cFillChar == '0' ) )
                      {
                        // bei Zahlen das Minus vorne lassen
                        strParamContent = GR::String( "-" ) + cFillChar + strParamContent.substr( 1 );
                      }
                      else
                      {
                        strParamContent = cFillChar + strParamContent;
                      }
                    }
                  }
                  else if ( iParamStellen > 0 )
                  {
                    // nach hinten auffüllen
                    while ( strParamContent.length() < (size_t)iParamStellen )
                    {
                      strParamContent += " ";
                    }
                  }
                  if ( bClipToRight )
                  {
                    // die letzten iParamStellen der Anzeige!
                    if ( strParamContent.length() > (size_t)iParamStellen )
                    {
                      strParamContent = strParamContent.substr( strParamContent.length() - iParamStellen );
                    }
                  }
                }
                strResult += strParamContent;
              }
            }

            bParam = false;
          }
          else
          {
            bParam = true;
            strParam.erase();
          }
          break;
        default:
          if ( !bParam )
          {
            strResult += cDummy;
          }
          else
          {
            strParam += cDummy;
          }
          break;
      }
      ++iPos;
    }
    return strResult;
  }

  CFormat& CFormat::Format( const GR::String& strFormatString )
  {
    m_Content = strFormatString;

    m_vectParameters.clear();

    return *this;
  }



  CFormat::operator GR::String() const
  {
    return Result();
  }



  void CFormat::AddParam( const tParam& Param )
  {
    m_vectParameters.push_back( Param );
  }

  

  CFormat FormatA( const GR::String& strFormatString )
  {
    CFormat     Formatter;

    Formatter.Format( strFormatString );

    return Formatter;
  }



  CFormat Format( const GR::String& strFormatString )
  {
    CFormat     Formatter;

    Formatter.Format( strFormatString );

    return Formatter;
  }



  // zweite Generation
  CFormat operator<< ( CFormat Format, const GR::String& strParam )
  {
    Format.AddParam( CFormat::tParam( strParam ) );

    return Format;
  }



  CFormat operator<< ( CFormat Format, const GR::Char* Param )
  {
    Format.AddParam( CFormat::tParam( Param ) );

    return Format;
  }



  CFormat operator<< ( CFormat Format, const GR::ip iParam )
  {
    Format.AddParam( CFormat::tParam( GR::Convert::ToStringA( (GR::i64)iParam ) ) );

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



