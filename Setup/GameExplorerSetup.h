#ifndef GR_SETUP_GAMEEXPLORERSETUP_H
#define GR_SETUP_GAMEEXPLORERSETUP_H

#include <guiddef.h>

#include <GR/GRTypes.h>

// handles the registering with Vista Game Explorer
// note: The main executable needs a .gdf and thumbnail embedded as resource!


namespace GR
{
  namespace Setup
  {
    namespace GameExplorerSetup
    {

      bool      RegisterGameWithGameExplorer( const GR::String& PathToExecutable,
                                              const GR::String& StartParameters,
                                              bool CurrentUserOnly,
                                              GUID& guidGame );

      bool      UnregisterGameWithGameExplorer( GUID& guidGame );

    };
  };
};



#endif // GR_SETUP_GAMEEXPLORERSETUP_H