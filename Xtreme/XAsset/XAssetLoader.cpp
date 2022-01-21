#include <String/XML.h>

#include <Xtreme/XRenderer.h>

#include "XAssetLoader.h"
#include "XAssetImage.h"
#include "XAssetImageSection.h"



namespace Xtreme
{

  namespace Asset
  {
    XAssetLoader::AssetTypeData::AssetTypeData() :
      pXMLAssetGroup( NULL )
    {
    }



    XAssetLoader::AssetTypeData::AssetTypeData( const AssetTypeData& RHS ) :
      Resources( RHS.Resources ),
      pXMLAssetGroup( NULL )
    {
      if ( RHS.pXMLAssetGroup )
      {
        pXMLAssetGroup = RHS.pXMLAssetGroup->Clone();
      }
    }



    XAssetLoader::AssetTypeData& XAssetLoader::AssetTypeData::operator=( const XAssetLoader::AssetTypeData& RHS )
    {
      if ( this != &RHS )
      {
        Resources = RHS.Resources;
        pXMLAssetGroup = NULL;
        if ( RHS.pXMLAssetGroup )
        {
          pXMLAssetGroup = RHS.pXMLAssetGroup->Clone();
        }
      }
      return *this;
    }


    XAssetLoader::AssetTypeData::~AssetTypeData()
    {
      delete pXMLAssetGroup;
    }



    XAssetLoader::XAssetLoader()
    {
    }



    XAssetLoader::~XAssetLoader()
    {
      ReleaseAllAssets();
    }



    XAssetLoader& XAssetLoader::Instance()
    {

      static XAssetLoader   theInstance;

      return theInstance;
    }



    bool XAssetLoader::LoadAssets( const GR::Char* szFilename )
    {
      GR::Strings::XML    xmlFile;

      if ( szFilename[0] == 0 )
      {
        return true;
      }

      if ( !xmlFile.Load( szFilename ) )
      {
        dh::Log( "Failed to LoadAssets from file %s", szFilename );
        return false;
      }

      GR::Strings::XMLElement*    pAssetProject = xmlFile.FindByType( "AssetProject" );
      if ( pAssetProject == NULL )
      {
        return false;
      }
      GR::Strings::XML::iterator    it( pAssetProject->FirstChild() );
      while ( it != GR::Strings::XML::iterator() )
      {
        GR::Strings::XMLElement*    pAssetGroup( *it );

        if ( pAssetGroup->Type() == "AssetGroup" )
        {
          GR::Strings::XML::iterator    itAsset( pAssetGroup->FirstChild() );
          while ( itAsset != GR::Strings::XML::iterator() )
          {
            GR::Strings::XMLElement*    pAsset( *itAsset );

            Xtreme::Asset::eXAssetType    assType = Xtreme::Asset::XA_INVALID;

            if ( pAssetGroup->Attribute( "Type" ) == "Image" )
            {
              assType = Xtreme::Asset::XA_IMAGE;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "ImageSection" )
            {
              assType = Xtreme::Asset::XA_IMAGE_SECTION;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Animation" )
            {
              assType = Xtreme::Asset::XA_ANIMATION;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Font" )
            {
              assType = Xtreme::Asset::XA_FONT;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Textbase" )
            {
              assType = Xtreme::Asset::XA_TEXT;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Sound" )
            {
              assType = Xtreme::Asset::XA_SOUND;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Script" )
            {
              assType = Xtreme::Asset::XA_SCRIPT;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Mesh" )
            {
              assType = Xtreme::Asset::XA_MESH;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Spline" )
            {
              assType = Xtreme::Asset::XA_SPLINE;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "GUIComponent" )
            {
              assType = Xtreme::Asset::XA_GUI;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Tileset" )
            {
              assType = Xtreme::Asset::XA_TILESET;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "Layered Map" )
            {
              assType = Xtreme::Asset::XA_LAYERED_MAP;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "ParameterTable" )
            {
              assType = Xtreme::Asset::XA_PARAMETER_TABLE;
            }
            if ( pAssetGroup->Attribute( "Type" ) == "ValueTable" )
            {
              assType = Xtreme::Asset::XA_VALUE_TABLE;
            }
            if ( assType != Xtreme::Asset::XA_INVALID )
            {
              if ( m_mapAssets[assType].pXMLAssetGroup == NULL )
              {
                m_mapAssets[assType].pXMLAssetGroup = pAssetGroup->Clone();
              }
              AssetTypeData&    assetData = m_mapAssets[assType];
              if ( assetData.Resources[pAsset->Attribute( "Name" ).c_str()].first != NULL )
              {
                dh::Log( "Asset Info was already allocated! (%x, type %d) for %s", assetData.Resources[pAsset->Attribute( "Name" ).c_str()].first, assType, pAsset->Attribute( "Name" ).c_str() );
                delete assetData.Resources[pAsset->Attribute( "Name" ).c_str()].first;
              }
              assetData.Resources[pAsset->Attribute( "Name" ).c_str()].first = pAsset->Clone();
              assetData.Resources[pAsset->Attribute( "Name" ).c_str()].second = NULL;
            }

            itAsset = itAsset.next_sibling();
          }
        }

        it = it.next_sibling();
      }

      return true;
    }



    void XAssetLoader::ReleaseAllAssets( const eXAssetType AssetType )
    {
      tMapAssets::iterator    it( m_mapAssets.find( AssetType ) );
      if ( it != m_mapAssets.end() )
      {
        AssetTypeData& data( it->second );

        delete data.pXMLAssetGroup;
        data.pXMLAssetGroup = NULL;

        tResourceMap& mapAssets( it->second.Resources );

        tResourceMap::iterator    itR( mapAssets.begin() );
        while ( itR != mapAssets.end() )
        {
          XAsset* pAsset( itR->second.second );

          if ( pAsset )
          {
            dh::Log( "Asset %s is still allocated!", itR->first.c_str() );
            //pAsset->Release();
            //delete pAsset;
          }
          delete itR->second.first;

          ++itR;
        }
        m_mapAssets.erase( it );
      }
    }



    void XAssetLoader::ReleaseAllAssets()
    {
      tMapAssets::iterator    it( m_mapAssets.begin() );
      while ( it != m_mapAssets.end() )
      {
        AssetTypeData&    data( it->second );

        delete data.pXMLAssetGroup;
        data.pXMLAssetGroup = NULL;

        tResourceMap& mapAssets( it->second.Resources );

        tResourceMap::iterator    itR( mapAssets.begin() );
        while ( itR != mapAssets.end() )
        {
          XAsset* pAsset( itR->second.second );

          if ( pAsset )
          {
            dh::Log( "Asset %s is still allocated!", itR->first.c_str() ); 
            //pAsset->Release();
            //delete pAsset;
          }
          delete itR->second.first;

          ++itR;
        }

        ++it;
      }
      m_mapAssets.clear();
    }



    XAsset* XAssetLoader::Asset( const eXAssetType AssetType, const GR::Char* Name )
    {
      tMapAssets::iterator    it( m_mapAssets.find ( AssetType ) );
      if ( it == m_mapAssets.end() )
      {
        return NULL;
      }
      tResourceMap& mapAssets( it->second.Resources );

      tResourceMap::iterator    itR( mapAssets.find( Name ) );
      if ( itR == mapAssets.end() )
      {
        return NULL;
      }
      return itR->second.second;
    }



    GR::up XAssetLoader::AssetTypeCount( const eXAssetType AssetType )
    {
      tMapAssets::iterator    it( m_mapAssets.find ( AssetType ) );
      if ( it == m_mapAssets.end() )
      {
        return 0;
      }
      return it->second.Resources.size();
    }



    XAsset* XAssetLoader::Asset( const eXAssetType AssetType, GR::up Index )
    {
      tMapAssets::iterator    it( m_mapAssets.find ( AssetType ) );
      if ( it == m_mapAssets.end() )
      {
        return NULL;
      }
      tResourceMap& mapAssets( it->second.Resources );
      if ( Index >= mapAssets.size() )
      {
        return NULL;
      }

      tResourceMap::iterator    itR( mapAssets.begin() );
      std::advance( itR, Index );
      return itR->second.second;
    }



    GR::Strings::XMLElement* XAssetLoader::AssetInfo( const eXAssetType AssetType, GR::up Index )
    {
      tMapAssets::iterator    it( m_mapAssets.find ( AssetType ) );
      if ( it == m_mapAssets.end() )
      {
        return NULL;
      }
      if ( Index == -1 )
      {
        return it->second.pXMLAssetGroup;
      }

      tResourceMap& mapAssets( it->second.Resources );
      if ( Index >= mapAssets.size() )
      {
        return NULL;
      }

      tResourceMap::iterator    itR( mapAssets.begin() );
      std::advance( itR, Index );
      return itR->second.first;
    }


    void XAssetLoader::SetAsset( const eXAssetType AssetType, GR::up Index, Xtreme::Asset::XAsset* pAsset )
    {
      tMapAssets::iterator    it( m_mapAssets.find ( AssetType ) );
      if ( it == m_mapAssets.end() )
      {
        return;
      }
      tResourceMap& mapAssets( it->second.Resources );
      if ( Index >= mapAssets.size() )
      {
        return;
      }

      tResourceMap::iterator    itR( mapAssets.begin() );
      std::advance( itR, Index );
      itR->second.second = pAsset;
    }



    const GR::Char* XAssetLoader::AssetAttribute( const eXAssetType AssetType, GR::up Index, const GR::Char* Name )
    {
      tMapAssets::iterator    it( m_mapAssets.find ( AssetType ) );
      if ( it == m_mapAssets.end() )
      {
        return "";
      }
      tResourceMap& mapAssets( it->second.Resources );
      if ( Index >= mapAssets.size() )
      {
        return "";
      }
      
      tResourceMap::iterator    itR( mapAssets.begin() );
      std::advance( itR, Index );

      static GR::String    attributeValue;
      
      attributeValue = itR->second.first->Attribute( Name ).c_str();

      return attributeValue.c_str();
    }



    const GR::Char* XAssetLoader::AssetName( const eXAssetType AssetType, GR::up Index )
    {
      tMapAssets::iterator    it( m_mapAssets.find ( AssetType ) );
      if ( it == m_mapAssets.end() )
      {
        return "";
      }
      tResourceMap& mapAssets( it->second.Resources );
      if ( Index >= mapAssets.size() )
      {
        return "";
      }
      
      tResourceMap::iterator    itR( mapAssets.begin() );
      std::advance( itR, Index );

      static GR::String    assetName;
      
      assetName = itR->first.c_str();
      return assetName.c_str();
    }


  };

};