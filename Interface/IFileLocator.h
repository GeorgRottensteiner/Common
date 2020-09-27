#ifndef IFILE_LOCATOR_H
#define IFILE_LOCATOR_H

#include <Interface/IIOStream.h>

struct IFileLocator
{

  public:

    virtual bool          OpenFile( const char* pFilename, IIOStream& Stream ) = 0;
    virtual const char*   LocateFile( const char* pFilename ) = 0;
    virtual bool          FileExists( const char* pFilename ) = 0;

    virtual void          AddSource( const char* pFolder ) = 0;
    virtual void          RemoveSource( const char* pFolder ) = 0;

};

#endif // IFILE_LOCATOR_H



