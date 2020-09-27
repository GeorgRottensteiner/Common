#pragma once


namespace Xtreme
{
  namespace EnvironmentType
  {
    enum Value
    {
      UNKNOWN,
      DESKTOP,
      TABLET,
      PHONE,
      WEB
    };
  }

  struct EnvironmentInfo
  {
    EnvironmentType::Value      Type;


    EnvironmentInfo() :
      Type( EnvironmentType::DESKTOP )
    {
    }
  };
}
