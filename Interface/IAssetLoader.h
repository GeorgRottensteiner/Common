#ifndef IASSET_LOADER_H
#define IASSET_LOADER_H

#include <Lang/Service.h>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Strings
  {
    class XMLElement;
  }
}

namespace Xtreme
{

  namespace Asset
  {
    class XAsset;

    enum eXAssetType
    {
      XA_INVALID = 0,
      XA_IMAGE,
      XA_IMAGE_SECTION,
      XA_ANIMATION,
      XA_SOUND,
      XA_FONT,
      XA_TEXT,
      XA_MESH,
      XA_SPLINE,
      XA_GUI,
      XA_TILESET,
      XA_LAYERED_MAP,
      XA_PARAMETER_TABLE,
      XA_VALUE_TABLE,
      XA_SCRIPT
    };

    class IAssetLoader : public GR::IService
    {

      public:

        virtual bool                      LoadAssets( const GR::Char* Filename ) = 0;
        virtual void                      ReleaseAllAssets() = 0;

        virtual XAsset*                   Asset( const eXAssetType AssetType, const GR::Char* Name ) = 0;

        virtual GR::up                    AssetTypeCount( const eXAssetType AssetType ) = 0;
        virtual XAsset*                   Asset( const eXAssetType AssetType, GR::up Index ) = 0;
        virtual GR::Strings::XMLElement*  AssetInfo( const eXAssetType AssetType, GR::up Index ) = 0;
        virtual void                      SetAsset( const eXAssetType AssetType, GR::up Index, Xtreme::Asset::XAsset* pAsset ) = 0;

        virtual const GR::Char*           AssetAttribute( const eXAssetType AssetType, GR::up Index, const GR::Char* Name ) = 0;

    };

  }

}

#endif // IASSET_LOADER_H