#include "ArgumentParser.h"

#include <OS/OS.h>

#include <String/StringUtil.h>



namespace GR
{


  ArgumentParser::ArgumentParser( const GR::String& SwitchCharacters ) :
    m_AllowedSwitchCharacters( SwitchCharacters )
  {
  }



  bool ArgumentParser::IsSwitchCharacter( GR::Char Char )
  {
    return ( m_AllowedSwitchCharacters.find( Char ) != GR::String::npos );
  }



  bool ArgumentParser::ParameterNameMatches( const ParameterInfo& Info, const GR::String& Parameter )
  {
    if ( ( Info.CaseSensitive )
    &&   ( Info.Name == Parameter ) )
    {
      return true;
    }
    if ( ( !Info.CaseSensitive )
    &&   ( GR::Strings::ToUpper( Info.Name ) == GR::Strings::ToUpper( Parameter ) ) )
    {
      return true;
    }
    return false;
  }



  bool ArgumentParser::CheckParameters()
  {
    std::list<ParameterInfo>::iterator    itPInfo( AllowedParameters.begin() );
    while ( itPInfo != AllowedParameters.end() )
    {
      ParameterInfo& pInfo( *itPInfo );
      
      pInfo.Result = PR_NOT_SET;
      
      ++itPInfo;
    }
    UnknownParameters.clear();
    Arguments.clear();
    
    
    GR::String    currentParameter;
    
    ParameterInfo*    pCurrentInfo = NULL;
    bool              foundError = false;
    
    std::vector<GR::String>::iterator    itP( CommandLineParams.begin() );
    while ( itP != CommandLineParams.end() )
    {
      const GR::String& param( *itP );
      
      if ( param.empty() )
      {
        ++itP;
        continue;
      }
      if ( ( IsSwitchCharacter( param[0] ) )
      &&   ( ( pCurrentInfo == NULL )
      ||     ( pCurrentInfo->Type != PT_SINGLE_VALUE ) ) )
      {
        // a switch
        if ( pCurrentInfo )
        {
          pCurrentInfo->Result = PR_VALUE_MISSING;
          currentParameter.clear();
          pCurrentInfo = NULL;
          foundError = true;
        }
        
        GR::String newParameter = param.substr( 1 );
        
        bool isKnownParameter = false;
        std::list<ParameterInfo>::iterator    itPInfo( AllowedParameters.begin() );
        while ( itPInfo != AllowedParameters.end() )
        {
          ParameterInfo& paramInfo( *itPInfo );
          
          if ( ParameterNameMatches( paramInfo, newParameter ) )
          {
            isKnownParameter = true;
            if ( paramInfo.Type == PT_SINGLE_VALUE )
            {
              if ( paramInfo.Value.empty() )
              {
                currentParameter = paramInfo.Name;
              }
              else
              {
                paramInfo.Result = PR_SET_MULTIPLE_TIMES;
                foundError = true;
              }
              pCurrentInfo = &paramInfo;
            }
            else if ( paramInfo.Type == PT_SWITCH )
            {
              paramInfo.Result = PR_SET;
            }
            break;
          }
          ++itPInfo;
        }
        if ( !isKnownParameter )
        {
          UnknownParameters.push_back( param );
        }
      }
      else
      {
        // a value
        if ( pCurrentInfo == NULL )
        {
          UnknownParameters.push_back( param );
          Arguments.push_back( param );
        }
        else
        {
          if ( pCurrentInfo->Value.empty() )
          {
            if ( pCurrentInfo->ValidValues.empty() )
            {
              pCurrentInfo->Value = param;
              pCurrentInfo->Result = PR_SET;
            }
            else
            {
              GR::String newParameter = GR::Strings::ToUpper( param );
              
              bool    validValueFound = false;
              std::list<GR::String>::iterator    itVV( pCurrentInfo->ValidValues.begin() );
              while ( itVV != pCurrentInfo->ValidValues.end() )
              {
                GR::String     value = GR::Strings::ToUpper( *itVV );
                if ( value == newParameter )
                {
                  pCurrentInfo->Value = newParameter;
                  pCurrentInfo->Result = PR_SET;
                  validValueFound = true;
                  break;
                }
                ++itVV;
              }
              if ( !validValueFound )
              {
                pCurrentInfo->Result = PR_INVALID_VALUE;
                foundError = true;
              }
            }
          }
          else
          {
            pCurrentInfo->Result = PR_SET_MULTIPLE_TIMES;
            foundError = true;
          }
          pCurrentInfo = NULL;
          currentParameter.erase();
        }
      }
      ++itP;
    }
    
    if ( pCurrentInfo )
    {
      pCurrentInfo->Result = PR_VALUE_MISSING;
      pCurrentInfo = NULL;
      foundError = true;
    }
  
    
    itPInfo = AllowedParameters.begin();
    while ( itPInfo != AllowedParameters.end() )
    {
      ParameterInfo& pInfo( *itPInfo );
      
      if ( ( pInfo.Result == PR_NOT_SET )
      &&   ( !pInfo.Optional ) )
      {
        foundError = true;
      }
      
      ++itPInfo;
    }
    return !foundError;
  }
  
  
  
  void ArgumentParser::SetCommandLine( int argc, const GR::Char* argv[] )
  {
    CommandLineParams.clear();
    for ( int i = 1; i < argc; ++i )
    {
      CommandLineParams.push_back( argv[i] );
    }
  }
  
  
  
  void ArgumentParser::AddSwitch( const GR::String& Switch, bool Optional, bool CaseSensitive )
  {
    AllowedParameters.push_back( ParameterInfo( Switch, Optional, CaseSensitive, PT_SWITCH ) );
  }
  
  
  
  void ArgumentParser::AddParameter( const GR::String& Param, bool CaseSensitive )
  {
    AllowedParameters.push_back( ParameterInfo( Param ) );
  }
  
  
  
  void ArgumentParser::AddParameter( const ParameterInfo& Param )
  {
    AllowedParameters.push_back( Param );
  }
  
  
  
  void ArgumentParser::AddOptionalParameter( const GR::String& Param, bool CaseSensitive )
  {
    AllowedParameters.push_back( ParameterInfo( Param, true, CaseSensitive ) );
  }
  
  
  
  bool ArgumentParser::IsParameterSet( const GR::String& Name )
  {
    std::list<ParameterInfo>::iterator    itPInfo( AllowedParameters.begin() );
    while ( itPInfo != AllowedParameters.end() )
    {
      ParameterInfo& pInfo( *itPInfo );
      
      if ( pInfo.Result == PR_SET )
      {
        if ( pInfo.CaseSensitive )
        {
          if ( Name == pInfo.Name )
          {
            return true;
          }
        }
        else if ( GR::Strings::ToUpper( pInfo.Name ) == GR::Strings::ToUpper( Name ) )
        {
          return true;
        }
      }
      ++itPInfo;
    }
    return false;
  }

  

  GR::String ArgumentParser::Parameter( const GR::String& Name )
  {
    std::list<ParameterInfo>::iterator    itPInfo( AllowedParameters.begin() );
    while ( itPInfo != AllowedParameters.end() )
    {
      ParameterInfo& pInfo( *itPInfo );
      
      if ( pInfo.Result == PR_SET )
      {
        if ( pInfo.CaseSensitive )
        {
          if ( Name == pInfo.Name )
          {
            return pInfo.Value;
          }
        }
        else if ( GR::Strings::ToUpper( pInfo.Name ) == GR::Strings::ToUpper( Name ) )
        {
          return pInfo.Value;
        }
      }
      ++itPInfo;
    }
    return "";
  }
  


  GR::String ArgumentParser::ErrorInfo()
  {
    GR::String     info;


    std::list<ParameterInfo>::iterator    itPInfo( AllowedParameters.begin() );
    while ( itPInfo != AllowedParameters.end() )
    {
      ParameterInfo& paramInfo( *itPInfo );

      if ( paramInfo.Result == PR_INVALID_VALUE )
      {
        info += "Invalid value for parameter " + paramInfo.Name + "\r\n";
      }
      else if ( paramInfo.Result == PR_VALUE_MISSING )
      {
        info += "Value missing for parameter " + paramInfo.Name + "\r\n";
      }
      else if ( paramInfo.Result == PR_SET_MULTIPLE_TIMES )
      {
        info += "Value set several times for parameter " + paramInfo.Name + "\r\n";
      }
      else if ( paramInfo.Result == PR_NOT_SET )
      {
        if ( !paramInfo.Optional )
        {
          info += "Mandatory parameter " + paramInfo.Name + " not set\r\n";
        }
      }
      ++itPInfo;
    }

    std::list<GR::String>::iterator    itUP( UnknownParameters.begin() );
    while ( itUP != UnknownParameters.end() )
    {
      info += "Unknown parameter " + *itUP + " passed\r\n";

      ++itUP;
    }
    return info;
  }



  GR::String ArgumentParser::CallInfo()
  {
    GR::String     info;


    std::list<ParameterInfo>::iterator    itPInfo( AllowedParameters.begin() );
    while ( itPInfo != AllowedParameters.end() )
    {
      ParameterInfo& paramInfo( *itPInfo );

      switch ( paramInfo.Type )
      {
        case PT_SINGLE_VALUE:
          if ( paramInfo.Optional )
          {
            info += "[";
          }
          info += "-" + paramInfo.Name;
          if ( paramInfo.Optional )
          {
            info += "]";
          }
          info += "\r\n";
          break;
        case PT_SWITCH:
          if ( paramInfo.Optional )
          {
            info += "[";
          }
          info += "-" + paramInfo.Name;
          if ( !paramInfo.ValidValues.empty() )
          {
            info += " <";
            int index = 0;
            std::list<GR::String>::iterator    itVV( paramInfo.ValidValues.begin() );
            while ( itVV != paramInfo.ValidValues.end() )
            {
              info += *itVV;
              if ( index + 1 < (int)paramInfo.ValidValues.size() )
              {
                info += ",";
              }
              ++index;

              ++itVV;
            }
            info += ">";
          }
          if ( paramInfo.Optional )
          {
            info += "]";
          }
          info += "\r\n";
          break;
      }
      ++itPInfo;
    }
    return info;
  }



  int ArgumentParser::NumArguments()
  {
    return (int)Arguments.size();
  }



  GR::String ArgumentParser::Argument( int Index )
  {
    if ( ( Index < 0 )
    ||   ( Index >= (int)Arguments.size() ) )
    {
      return "";
    }
    return Arguments[Index];
  }


}
