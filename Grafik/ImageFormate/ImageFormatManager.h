#ifndef IMAGEFORMATMANAGER_H
#define IMAGEFORMATMANAGER_H



#include <map>
#include <string>

#include <Interface/IImageFormatManager.h>

#include <Grafik/Palette.h>

#include <Grafik\ImageData.h>
#include <Grafik\ImageFormate\ImageSet.h>

#include <Lang/GlobalAllocator.h>



class AbstractImageFileFormat;

#ifdef EMSCRIPTEN
typedef std::map<GR::String, AbstractImageFileFormat*>  tMapFormate;
#else
typedef std::map<GR::String, AbstractImageFileFormat*, std::less<const GR::String>,
        GR::STL::global_allocator<std::pair<const GR::String, AbstractImageFileFormat*> > >  tMapFormate;
#endif


class ImageFormatManager : public IImageFormatManager
{

  protected:

    tMapFormate         m_mapFormate;



  public:



    ImageFormatManager()
    {
    }

    ~ImageFormatManager()
    {
    }



    void RegisterFormat( const GR::String& Description, AbstractImageFileFormat* pFormatPlugin )
    {
      m_mapFormate[Description] = pFormatPlugin;
      //dh::Log( "Register Plugin: %s %x (%d registered now)\n", szDesc, formatPlugin, m_mapFormate.size() );
    }



    GR::String                    IdentifyType( const GR::String& FileName );
    GR::Graphic::ImageData*       LoadData( const GR::String& FileName, AbstractImageFileFormat** pPlugin = NULL );
    ImageSet*                     LoadSet( const GR::String& FileName, AbstractImageFileFormat** pPlugin = NULL );
    bool                          SaveData( const GR::String& FileName, GR::Graphic::ImageData* pData );

    const tMapFormate&            GetFormatMap() const;

    static ImageFormatManager&    Instance();

};



class IAbstractFormat
{
  public:

    virtual bool          IsFileOfType( const GR::String& FileName ) = 0;
    virtual bool          CanSave( GR::Graphic::ImageData* pData ) = 0;
    virtual bool          CanSave( ImageSet* pSet ) = 0;
    virtual GR::String    GetDescription() = 0;
    virtual GR::String    GetFilterString() = 0;
    virtual GR::Graphic::ImageData* Load( const GR::String& FileName ) = 0;
    virtual ImageSet*     LoadSet( const GR::String& FileName ) = 0;
    virtual bool          Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL ) = 0;
    virtual bool          Save( const GR::String& FileName, ImageSet* pSet ) = 0;
    virtual void          SetMetaData( const GR::String& Key, const GR::String& Value ) = 0;
    virtual GR::String    MetaData( const GR::String& Key ) = 0;

    virtual ~IAbstractFormat()
    {
    }
};



class AbstractImageFileFormat : public IAbstractFormat
{
  protected:

    std::map<GR::String, GR::String>      m_MetaData;



  public:

    AbstractImageFileFormat()
    {
      //ImageFormatManager::GetInstance()->RegisterFormat( GetDescription(), this );
    }
    virtual bool        IsFileOfType( const GR::String& FileName ) = 0;
    virtual bool        CanSave( GR::Graphic::ImageData* pData ) = 0;



    virtual bool        CanSave( ImageSet* pSet )
    {
      if ( pSet == NULL )
      {
        return false;
      }
      return false;
    }



    virtual GR::String  GetDescription()
    {
      return "Abstract Image File Format Base";
    }

    virtual GR::String  GetFilterString() = 0;



    GR::Graphic::ImageData*         Load( const GR::String& FileName )
    {
      ImageSet*    pSet = LoadSet( FileName );
      if ( pSet == NULL )
      {
        return NULL;
      }

      if ( pSet->Frames.empty() )
      {
        delete pSet;
        return NULL;
      }

      GR::Graphic::ImageData*   pData = pSet->DetachImage( 0, 0 );

      delete pSet;

      return pData;
    }



    virtual ImageSet*   LoadSet( const GR::String& FileName ) = 0;
    virtual bool        Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask = NULL ) = 0;
    virtual bool        Save( const GR::String&, ImageSet* )
    {
      return false;
    }



    virtual void SetMetaData( const GR::String& Key, const GR::String& Value )
    {
      m_MetaData[Key] = Value;
    }



    virtual GR::String MetaData( const GR::String& Key )
    {
      std::map<GR::String, GR::String>::iterator it = m_MetaData.find( Key );
      if ( it == m_MetaData.end() )
      {
        return GR::String();
      }
      return it->second;
    }

};


#endif //_IMAGEFORMATMANAGER_H_