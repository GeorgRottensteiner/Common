#ifndef XASSET_TILESET_H
#define XASSET_TILESET_H

#include "XAsset.h"

#include <Interface/IAnimationManager.h>

#include <vector>
#include <string>


namespace Xtreme
{

namespace Asset
{

  class XAssetTileset : public XAsset
  {

    public:

      struct TileInfo
      {
        GR::String       Animation;
        tAnimationPos     AnimPos;
        GR::String       Section;
        GR::u32           Flags;

        TileInfo() :
          Flags( 0 )
        {
        }
      };

      std::vector<TileInfo>        m_Tiles;



      XAssetTileset()
      {
      }

      const GR::String& Section( int TileIndex )
      {
        if ( TileIndex >= (int)m_Tiles.size() )
        {
          static GR::String    empty;
          return empty;
        }
        return m_Tiles[TileIndex].Section;
      }



      const GR::String& Anim( int TileIndex )
      {
        if ( TileIndex >= (int)m_Tiles.size() )
        {
          static GR::String    empty;
          return empty;
        }
        return m_Tiles[TileIndex].Animation;
      }



      const tAnimationPos& AnimationPos( int TileIndex )
      {
        if ( TileIndex >= (int)m_Tiles.size() )
        {
          static tAnimationPos  empty;
          return empty;
        }
        return m_Tiles[TileIndex].AnimPos;
      }



      GR::u32 Flags( int TileIndex )
      {
        if ( TileIndex >= (int)m_Tiles.size() )
        {
          return 0;
        }
        return m_Tiles[TileIndex].Flags;
      }



      void AddTile( const GR::String& Tilename, GR::u32 Flags = 0 )
      {
        TileInfo    info;
        info.Section = Tilename;
        info.Flags   = Flags;
        m_Tiles.push_back( info );
      }



      void AddTileAnimated( const GR::String& Anim, GR::u32 Flags = 0 )
      {
        TileInfo    info;
        info.Animation = Anim;
        info.Flags     = Flags;
        m_Tiles.push_back( info );
      }



      virtual bool                Release()
      {
        m_Tiles.clear();
        return false;
      }



      virtual GR::up Handle( const char* Name )
      {
        return 0;
      }

  };

}

}


#endif // XASSET_TILESET_H