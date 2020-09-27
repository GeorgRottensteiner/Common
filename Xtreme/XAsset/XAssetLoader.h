#ifndef XASSET_LOADER_H
#define XASSET_LOADER_H

#include <map>
#include <string>

#include <Interface/IAssetLoader.h>

#include "XAsset.h"



namespace GR
{
  namespace Strings
  {
    class XMLElement;
  };
};

namespace Xtreme
{

  namespace Asset
  {

    class XAssetLoader : public GR::Service::ServiceImpl<IAssetLoader>
    {

      private:

        typedef std::map<GR::String, std::pair<GR::Strings::XMLElement*, XAsset*> >  tResourceMap;

        struct AssetTypeData
        {
          tResourceMap              Resources;
          GR::Strings::XMLElement*  pXMLAssetGroup;

          AssetTypeData();
          AssetTypeData( const AssetTypeData& RHS );
          AssetTypeData& operator=( const AssetTypeData& RHS );

          ~AssetTypeData();
        };

      public:

        
        typedef std::map<eXAssetType,AssetTypeData>                      tMapAssets;


        tMapAssets                        m_mapAssets;


        static XAssetLoader&              Instance();

        bool                              LoadAssets( const GR::Char* Filename );
        void                              ReleaseAllAssets();

        XAsset*                           Asset( const eXAssetType AssetType, const GR::Char* Name );

        virtual GR::up                    AssetTypeCount( const eXAssetType AssetType );
        virtual XAsset*                   Asset( const eXAssetType AssetType, GR::up Index );
        virtual GR::Strings::XMLElement*  AssetInfo( const eXAssetType AssetType, GR::up Index );
        virtual void                      SetAsset( const eXAssetType AssetType, GR::up Index, Xtreme::Asset::XAsset* pAsset );
        virtual const GR::Char*           AssetAttribute( const eXAssetType AssetType, GR::up Index, const GR::Char* Name );
        const GR::Char*                   AssetName( const eXAssetType AssetType, GR::up Index );



      private:

        XAssetLoader();
        virtual ~XAssetLoader();

    };

  };

};

#endif //XTEXTURE_MANAGER_H_