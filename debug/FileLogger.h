#pragma once

#include <GR/GRTypes.h>

//#include "ResultCode.h"



namespace Risk
{
  
}

class FileLogger
{
  public:
    
    enum class LogLevel
    {
      LL_NONE             = 0,    // nothing
      LL_ERROR            = 1,    // error only
      LL_WARNING          = 2,    // error and warning only
      LL_INFO             = 3,    // all but debug (Info, Warning, Error)
      LL_DEBUG            = 4     // all
    };
    

    
  private:

    GR::String      m_Filename;
    int             m_NumFiles;
    GR::u64         m_MaxSize;
    
    // used to avoid checking log file size for every single call
    int             m_NumCallsSinceLastRotateCheck;

    int             m_ActiveFileIndex;
    GR::String      m_ActiveFilename;
    
    LogLevel        m_LogLevel;
    bool            m_Verbose;
    bool            m_IsSetup;



    void Log( const GR::String& Message );

    void DetermineFirstFile();

    
    
  public:

    FileLogger();
    FileLogger( const GR::String& Filename, int NumFiles, GR::u64 MaxSize, LogLevel LogLevel );
    
    static FileLogger& Instance();
    static LogLevel    LogLevelFromProperty( const GR::String& Property );
    
    void Setup( const GR::String& Filename, int NumFiles, GR::u64 MaxSize, LogLevel LogLevel );
    
    void LogDebug( const GR::String& Message );
    void LogInfo( const GR::String& Message );
    void LogWarning( const GR::String& Message );
    void LogError( const GR::String& Message );
    void LogRaw( const GR::String& Message );

    void RotateFiles();
    void ResetAllFiles();

    void SetVerbose( bool Verbose );

};

