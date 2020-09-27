#ifndef REGISTRYUTILS_H
#define REGISTRYUTILS_H


#include <GR/GRTypes.h>

#include <windows.h>



class Registry
{
  public:

    // Referenzzähler simulieren, zählt eins hoch (bzw. legt den Key neu an, falls nicht vorhanden)
    static int          IncreaseReference( const GR::Char* BranchName, const GR::Char* KeyName, bool bCurrentUser = true );

    // Referenzzähler runter, true bedeutet, Referenz ist 0
    static bool         DecreaseReference( const GR::Char* BranchName, const GR::Char* KeyName, bool bCurrentUser = true );

    // einen Key setzen
    static bool         SetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, const GR::Char* Value );
    static bool         SetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, void* pData, size_t iDataSize );

    // einen Key holen
    static bool         GetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, void* pData, int iDataSize );
    static bool         GetKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key, unsigned long& dwValue );

    static bool         GetKey( HKEY hKeyMain, const GR::String& Branch, const GR::String& Key, GR::String& Value );

    // einen Zweig löschen
    static bool         DeleteBranch( HKEY hKeyMain, const GR::Char* Key );

    // einen einzelnen Key löschen
    static bool         DeleteKey( HKEY hKeyMain, const GR::Char* Branch, const GR::Char* Key );

    // ist ein Zweig komplett ohne Keys?
    static bool         IsBranchEmpty( HKEY hKeyMain, const GR::Char* Branch );

    // einen File-Typ zu einer Applikation zuordnen
    static void         RegisterFileType( const GR::Char* Extension, const GR::Char* AppDescription, 
                                          const GR::Char* IconPath, const GR::Char* OpenCommand );

    // einen File-Typ wieder freigeben
    static void         UnregisterFileType( const GR::Char* Extension, const GR::Char* AppDescription );

    // ist ein File-Typ (Extension) zu einer bestimmten Applikation zugehörig?
    static bool         IsFileTypeRegisteredTo( const GR::Char* Extension, const GR::Char* AppDescription );

    // einen Uninstall-Eintrag in Software anlegen
    static bool         AddUnInstallEntry( const GR::Char* RegistryKey, 
                                           const GR::Char* Description, 
                                           const GR::Char* UninstallCommand );

};


#endif // REGISTRYUTILS_H



