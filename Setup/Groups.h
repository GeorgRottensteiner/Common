#ifndef PROGRAMGROUPS_H
#define PROGRAMGROUPS_H

#include <GR/GRTypes.h>



class CProgramGroups
{

  public:

    static GR::String         GetShellFolder( const GR::Char* FolderName, bool bCurrentUser = false );
    static GR::String         GetSystemFolder( const GR::Char* FolderName );
    static GR::String         GetStartMenuProgramPath( bool bCurrentUser = false );

    static bool                   CreateProgramGroup( const GR::Char* Description, bool bCurrentUser = false );
    static bool                   CreateLink( const GR::Char* Path, 
                                              const GR::Char* Description, 
                                              const GR::Char* File, 
                                              const GR::Char* IconPath = NULL, 
                                              GR::u32 IconIndex = 0, 
                                              const GR::Char* Parameter = NULL );   // Verknüpfung mit Icon anlegen
    static bool                   CreateProgramLink( const GR::Char* Description, 
                                                     const GR::Char* File, 
                                                     const GR::Char* Parameter = NULL,
                                                     const GR::Char* IconPath = NULL, 
                                                     GR::u32 IconIndex = 0,
                                                     bool bCurrentUser = false );          // Link in Programmgruppe anlegen
    static bool                   CreateDesktopLink( const GR::Char* Description, 
                                                     const GR::Char* File, 
                                                     const GR::Char* IconPath = NULL, 
                                                     GR::u32 IconIndex = 0,
                                                     bool bCurrentUser = false );   // Link auf Desktop anlegen

};



#endif // __PROGRAMGROUPS_H__



