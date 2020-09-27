#ifndef INCLUDE_XWINDOW_H
#define INCLUDE_XWINDOW_H


#include <Lang/Service.h>

#include <Xtreme/Environment/EnvironmentConfig.h>



namespace Xtreme
{
  struct IAppWindow : public GR::IService
  {
    public:

      virtual bool Create( const EnvironmentConfig& Config ) = 0;
      virtual void Invalidate() = 0;
      virtual void Close() = 0;
      virtual GR::up Handle() = 0;

  };
}

#endif //INCLUDE_XWINDOW_H



