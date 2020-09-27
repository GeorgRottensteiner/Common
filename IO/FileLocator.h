#ifndef FILE_LOCATOR_H
#define FILE_LOCATOR_H


#include <memory>
#include <list>

#include <Interface/IIOStream.h>
#include <Interface/IFileLocator.h>

#include <Lang/Service.h>



namespace GR
{
  namespace IO
  {

    class FileLocator : public GR::Service::Service, public IFileLocator
    {

      protected:

        typedef std::list<GR::String>        tSourceFolders;


        tSourceFolders          m_SourceFolders;


      public:

        static FileLocator& Instance();

        virtual bool            OpenFile( const char* pFilename, IIOStream& Stream );
        virtual const char*     LocateFile( const char* pFilename );
        virtual bool            FileExists( const char* pFilename );

        virtual void            AddSource( const char* pFolder );
        virtual void            RemoveSource( const char* pFolder );

    };

  }

}

#endif // FILE_LOCATOR_H



