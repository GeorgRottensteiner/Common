#ifndef GR_DEBUG_SERVICE_H
#define GR_DEBUG_SERVICE_H


#include <map>
#include <String/GRstring.h>

#include <Interface/IDebugService.h>




class DebugService : public IDebugService
{

  private:

    struct tDebugSettings
    {
      bool            m_Enabled;
      bool            m_ToFile;

      tDebugSettings() :
        m_Enabled( true ),
        m_ToFile( false )
      {
      }
    };

    typedef std::map<GR::String,tDebugSettings>    tMapLogSystems;

    tMapLogSystems                                  m_mapLogSystems;


  public:

    void LogEnable( const GR::String& System, bool Enable = true );
    void LogToFile( const GR::String& System, bool Enable = true );

    bool LogEnabled( const GR::String& System );
    bool LogToFileEnabled( const GR::String& System );

    void LogDirect( const GR::String& System, const GR::String& Text );
    void Log( const GR::String& System, const char* Format, ... );

};

#endif // GR_DEBUG_SERVICE_H