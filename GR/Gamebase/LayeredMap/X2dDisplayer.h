#ifndef GR_GAMEBASE_LAYEREDMAP_DISPLAYER_X2D_H
#define GR_GAMEBASE_LAYEREDMAP_DISPLAYER_X2D_H

#include "LayeredMap.h"
#include "JREngine.h"

#include <Xtreme/X2dRenderer.h>


class Framework;

namespace GR
{
  namespace Gamebase
  {
    template <class APP,class ITERATOR> void RenderObjects( APP& theApp, 
                                             X2dRenderer& Renderer, 
                                             GR::Gamebase::JREngine& JREngine, 
                                             ITERATOR Start,
                                             ITERATOR End,
                                             const GR::tRect& DisplayRect, 
                                             bool DisplayBounds )
    {
      ITERATOR  itO( Start );
      while ( itO != End )
      {
        GR::Gamebase::GameObject*   pObj = (GR::Gamebase::GameObject*)*itO;

        if ( pObj->Flags & GR::Gamebase::LayerObject::ObjectInfoFlags::OF_HIDDEN )
        {
          ++itO;
          continue;
        }

        if ( pObj->m_ProcessingFlags & GR::Gamebase::ProcessingFlags::CUSTOM_RENDER )
        {
          JREngine.RaiseObjectEvent( pObj, 
                                     GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::CUSTOM_DRAW,
                                                                GR::tPoint( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x, 
                                                                            DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y ) ) );
        }
        else
        {
          if ( !pObj->Animation.empty() )
          {
            Renderer.RenderTextureSectionColorKeyed( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x, 
                                                     DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y, 
                                                     theApp.AnimationFrame( pObj->AnimPos ),
                                                     0xffff00ff );
          }
          else
          {
            Renderer.RenderTextureSectionColorKeyed( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x, 
                                                     DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y, 
                                                     theApp.Section( pObj->Section ),
                                                     0xffff00ff );
          }
        }
        if ( DisplayBounds )
        {
          Renderer.RenderRect( pObj->Bounds().position() + GR::tPoint( DisplayRect.Left, DisplayRect.Top ), 
                               pObj->Bounds().size(), 0xff00ff00 );
        }

        ++itO;
      }
    }



    template <class APP,class OBJ> void DisplayMap( APP& theApp, X2dRenderer& Renderer, GR::Gamebase::JREngine& JREngine, const GR::tRect& DisplayRect, bool DisplayBounds = false )
    {
      int     referenceLayerWidth = DisplayRect.width();
      int     referenceLayerHeight = DisplayRect.height();
      if ( JREngine.m_pGameLayer )
      {
        referenceLayerWidth = JREngine.m_pGameLayer->DisplayWidth() - DisplayRect.width();
        referenceLayerHeight = JREngine.m_pGameLayer->DisplayHeight() - DisplayRect.height();
      }
      std::vector<GR::Gamebase::Layer*>::const_iterator   itL( JREngine.m_Map.Layers.begin() );
      while ( itL != JREngine.m_Map.Layers.end() )
      {
        const GR::Gamebase::Layer*    pLayer( *itL );

        switch ( pLayer->Type )
        {
          case GR::Gamebase::Layer::LT_TILE_LAYER:
            {
              bool        doTint = false;
              bool        doAlphaTest = false;
              GR::u32     tintColor = 0xffffffff;

              if ( pLayer->Tinted )
              {
                doTint = true;
                tintColor = pLayer->TintColor;
              }
              else if ( pLayer->Colorkeyed )
              {
                doAlphaTest = true;
              }
              GR::Gamebase::TileLayer*    pTileLayer = (GR::Gamebase::TileLayer*)pLayer;
              int     displayOffsetX = JREngine.m_Map.DisplayOffset.x - pLayer->Offset.x;
              int     displayOffsetY = JREngine.m_Map.DisplayOffset.y - pLayer->Offset.y;

              if ( pLayer->RelativeSpeed )
              {
                displayOffsetX = displayOffsetX * ( pTileLayer->DisplayWidth() - DisplayRect.width() ) / referenceLayerWidth;
                displayOffsetY = displayOffsetY * ( pTileLayer->DisplayHeight() - DisplayRect.height() ) / referenceLayerHeight;
                //dh::Log( "Relative Pos (from %d,%d to %d,%d)", m_Map.DisplayOffset.x, m_Map.DisplayOffset.y, displayOffsetX, displayOffsetY );
              }

              int     X1 = displayOffsetX / pTileLayer->TileSpacingX - 1;
              int     Y1 = displayOffsetY / pTileLayer->TileSpacingY - 1;
              int     X2 = ( displayOffsetX + DisplayRect.width() ) / pTileLayer->TileSpacingX;
              int     Y2 = ( displayOffsetY + DisplayRect.height() ) / pTileLayer->TileSpacingY;

              GR::tPoint      gridTopLeft( displayOffsetX / pTileLayer->SectorWidth(), displayOffsetY / pTileLayer->SectorHeight() );
              GR::tPoint      gridBottomRight( ( displayOffsetX + DisplayRect.width() ) / pTileLayer->SectorWidth(), 
                                               ( displayOffsetY + DisplayRect.height() ) / pTileLayer->SectorHeight() );

              for ( int x = X1; x <= X2; ++x )
              {
                for ( int y = Y1; y <= Y2; ++y )
                {
                  if ( pTileLayer->IsOutside( x, y ) )
                  {
                    continue;
                  }
                  /*
                  bool      isLiquidV1 = IsLiquid( x, y );
                  bool      isLiquidV2 = IsLiquid( x + 1, y );
                  bool      isLiquidV3 = IsLiquid( x, y + 1 );
                  bool      isLiquidV4 = IsLiquid( x + 1, y + 1 );
                  */
                  GR::u16     field = pTileLayer->Field( x, y );
                  if ( field != 65535 )
                  {
                    if ( doTint )
                    {
                      Renderer.RenderTextureSectionColorized( DisplayRect.Left + ( x - X1 - 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                              DisplayRect.Top + ( y - Y1 - 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                              theApp.Tile( pTileLayer->Tileset, field ),
                                                              tintColor );
                    }
                    else if ( doAlphaTest )
                    {
                      Renderer.RenderTextureSectionColorKeyed( DisplayRect.Left + ( x - X1 - 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                               DisplayRect.Top + ( y - Y1 - 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                               theApp.Tile( pTileLayer->Tileset, field ),
                                                               pLayer->ColorKey );
                    }
                    else
                    {
                      Renderer.RenderTextureSection( DisplayRect.Left + ( x - X1 - 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                     DisplayRect.Top + ( y - Y1 - 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                     theApp.Tile( pTileLayer->Tileset, field ) );
                    }
                  }
                }
              }

              // only display game objects in game layer
              if ( JREngine.m_pGameLayer == pLayer )
              {
                // sleeping objects too
                for ( int x = gridTopLeft.x; x <= gridBottomRight.x; ++x )
                {
                  for ( int y = gridTopLeft.y; y <= gridBottomRight.y; ++y )
                  {
                    GR::tPoint    gridPos( x, y );

                    if ( pTileLayer->Objects.find( gridPos ) == pTileLayer->Objects.end() )
                    {
                      continue;
                    }
                    const std::list<GR::Gamebase::LayerObject*>&  listObj( pTileLayer->Objects[gridPos] );

                    GR::tRect     displayRectOffset( DisplayRect );

                    displayRectOffset.offset( - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                              - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY );
                    RenderObjects( theApp, Renderer, JREngine, listObj.begin(), listObj.end(), displayRectOffset, DisplayBounds );
                  }
                }

                // awake objects
                const std::list<GR::Gamebase::GameObject*>& listObj( JREngine.m_AwakeObjects );

                GR::tRect     displayRectOffset( DisplayRect );

                displayRectOffset.offset( - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                          - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY );
                RenderObjects( theApp, Renderer, JREngine, listObj.begin(), listObj.end(), displayRectOffset, DisplayBounds );
              }
              // TODO - awake objects for every layer
              /*
              for ( int x = gridTopLeft.x; x <= gridBottomRight.x; ++x )
              {
                for ( int y = gridTopLeft.y; y <= gridBottomRight.y; ++y )
                {
                  GR::tPoint    gridPos( x, y );

                  if ( pTileLayer->Objects.find( gridPos ) == pTileLayer->Objects.end() )
                  {
                    continue;
                  }
                  const std::list<GR::Gamebase::LayerObject*>&  listObj( pTileLayer->Objects[gridPos] );

                  std::list<GR::Gamebase::LayerObject*>::const_iterator   itO( listObj.begin() );
                  while ( itO != listObj.end() )
                  {
                    OBJ*   pObj = (OBJ*)*itO;

                    if ( !pObj->Animation.empty() )
                    {
                      Renderer.RenderTextureSection2d( pObj->Position.x - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX, 
                                                       pObj->Position.y - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY, 
                                                       theApp.AnimationFrame( pObj->AnimPos ) );
                    }
                    else
                    {
                      Renderer.RenderTextureSection2d( pObj->Position.x - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX, 
                                                       pObj->Position.y - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY, 
                                                       theApp.Section( pObj->Section ) );
                    }
                    if ( DisplayBounds )
                    {
                      Renderer.SetTexture( 0, NULL );
                      Renderer.RenderRect2d( pObj->Bounds().position() - GR::tPoint( displayOffsetX, displayOffsetY ), pObj->Bounds().size(), 0xff00ff00 );
                    }

                    ++itO;
                  }
                }
              }
              */
            }
            break;
        }

        ++itL;
      }
    }

  }
}

#endif // GR_GAMEBASE_LAYEREDMAP_DISPLAYER_X_H
