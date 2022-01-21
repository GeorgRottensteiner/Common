#ifndef GR_IDEBUG_SERVICE_H
#define GR_IDEBUG_SERVICE_H


#include <GR/GRTypes.h>

#include <Lang/Service.h>




class IDebugService : public GR::Service::Service
{

  public:

    virtual void LogEnable( const GR::String& System, bool Enable = true ) = 0;
    virtual void LogToFile( const GR::String& System, bool Enable = true ) = 0;

    virtual bool LogEnabled( const GR::String& System ) = 0;
    virtual bool LogToFileEnabled( const GR::String& System ) = 0;

    virtual void LogDirect( const GR::String& System, const GR::String& Text ) = 0;
    virtual void Log( const GR::String& System, const char* Format, ... ) = 0;

};

#endif // GR_IDEBUG_SERVICE_H