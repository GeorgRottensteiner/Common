#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>
#include <vector>
#include <list>

#include <GR/GRTypes.h>



namespace GR
{
  class ArgumentParser
  {
    private:

      GR::String            m_AllowedSwitchCharacters;


      bool IsSwitchCharacter( GR::Char Char ) const;



    public:
      
      enum ParameterResult
      {
        PR_NOT_SET = 0,
        PR_SET,
        PR_INVALID_VALUE,
        PR_SET_MULTIPLE_TIMES,
        PR_VALUE_MISSING
      };
      
      enum ParameterType
      {
        PT_SINGLE_VALUE,
        PT_SWITCH
      };
      
      struct ParameterInfo
      {
        GR::String                Name;
        GR::String                Value;
        std::list<GR::String>     ValidValues;
        bool                      Optional;
        ParameterResult           Result;
        ParameterType             Type;
        bool                      CaseSensitive;


        
        ParameterInfo( const GR::String& Name = GR::String(), bool Optional = false, bool CaseSensitive = false, ParameterType Type = PT_SINGLE_VALUE ) :
          Name( Name ),
          Optional( Optional ),
          Result( PR_NOT_SET ),
          Type( Type ),
          CaseSensitive( CaseSensitive )
        {
        }
      };

    private:

      bool                                  ParameterNameMatches( const ParameterInfo& Info, const GR::String& Parameter ) const;


    public:

      std::vector<GR::String>               CommandLineParams;
      std::list<ParameterInfo>              AllowedParameters;
      std::list<GR::String>                 UnknownParameters;
      std::vector<GR::String>               Arguments;



      ArgumentParser( const GR::String& SwitchCharacters = "-" );
      

      
      void                                  AddParameter( const ParameterInfo& ParamInfo );
      void                                  AddParameter( const GR::String& Param, bool CaseSensitive = false );
      void                                  AddSwitch( const GR::String& Switch, bool Optional = true, bool CaseSensitive = false );
      void                                  AddOptionalParameter( const GR::String& Param, bool CaseSensitive = false );
      
      bool                                  CheckParameters();
      GR::String                            Parameter( const GR::String& Name ) const;
      bool                                  IsParameterSet( const GR::String& Name ) const;

      void                                  SetCommandLine( int argc, const GR::Char* argv[] );
      
      GR::String                            ErrorInfo() const;
      GR::String                            CallInfo() const;

      int                                   NumArguments() const;
      GR::String                            Argument( int Index ) const;

  };
      
}

#endif // ARGUMENT_PARSER_H
