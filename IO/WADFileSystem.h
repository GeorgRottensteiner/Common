#ifndef WAD_FILE_SYSTEM_H
#define WAD_FILE_SYSTEM_H



#include <list>
#include <string>

#include <IO/WADFile.h>
#include <IO/FileStream.h>



class CWADFileSystem
{

  protected:

    struct tWADFileEntry
    {
      GR::WADFile  WadFile;
      GR::String   strFileName;
    };

    typedef std::list<tWADFileEntry>      tListWADFiles;
    typedef std::list<GR::String>        tListSearchDirs;

    tListWADFiles           m_listWADFiles;

    tListSearchDirs         m_listSearchDir;

    CWADFileSystem();


  public:

    ~CWADFileSystem();

    static CWADFileSystem& CWADFileSystem::Instance();

    GR::IO::FileStream      OpenFile( const char *szFileName );


    bool                    AddWADFileToList( const char *szFileName );
    void                    RemoveWADFileFromList( const char *szFileName );

    void                    AddSearchDir( const char *szFileName );

};



#endif // WAD_FILE_SYSTEM_H



