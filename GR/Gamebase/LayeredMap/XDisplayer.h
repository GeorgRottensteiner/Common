#ifndef GR_GAMEBASE_LAYEREDMAP_DISPLAYER_X_H
#define GR_GAMEBASE_LAYEREDMAP_DISPLAYER_X_H

#include "LayeredMap.h"
#include "JREngine.h"

#include <Xtreme/XRenderer.h>


class Framework;

namespace GR
{
  namespace Gamebase
  {
    template <class APP,class OBJ> void DisplayMap( APP& theApp, XRenderer& Renderer, GR::Gamebase::JREngine& JREngine, const GR::tRect& DisplayRect, bool DisplayBounds = false )
    {
      Renderer.SetState( XRenderer::RS_LIGHTING, XRenderer::RSV_DISABLE );

      int     referenceLayerWidth = DisplayRect.width();
      int     referenceLayerHeight = DisplayRect.height();
      if ( JREngine.m_pGameLayer )
      {
        referenceLayerWidth = JREngine.m_pGameLayer->DisplayWidth() - DisplayRect.width();
        referenceLayerHeight = JREngine.m_pGameLayer->DisplayHeight() - DisplayRect.height();
      }
      bool    firstLayer = true;

      std::vector<GR::Gamebase::Layer*>::const_iterator   itL( JREngine.m_Map.Layers.begin() );
      while ( itL != JREngine.m_Map.Layers.end() )
      {
        const GR::Gamebase::Layer*    pLayer( *itL );

        switch ( pLayer->Type )
        {
          case GR::Gamebase::Layer::LT_TILE_LAYER:
            {
              GR::u32     tintColor = 0xffffffff;

              if ( pLayer->Name == "Water" )
              {
                Renderer.SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );
                tintColor = 0x80ffffff;
              }
              else if ( ( pLayer->Name == "Foreground" )
              ||        ( !firstLayer ) )
              {
                Renderer.SetShader( XRenderer::ST_ALPHA_TEST );
              }
              else
              {
                Renderer.SetShader( XRenderer::ST_FLAT );
              }
              firstLayer = false;
              GR::Gamebase::TileLayer*    pTileLayer = (GR::Gamebase::TileLayer*)pLayer;
              //int     displayOffsetX = JREngine.m_Map.DisplayOffset.x + DisplayRect.Left - pLayer->Offset.x;
              //int     displayOffsetY = JREngine.m_Map.DisplayOffset.y + DisplayRect.Top - pLayer->Offset.y;
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
              int     X2 = ( displayOffsetX + pTileLayer->TileSpacingX - 1 + DisplayRect.width() ) / pTileLayer->TileSpacingX;
              int     Y2 = ( displayOffsetY + pTileLayer->TileSpacingY - 1 + DisplayRect.height() ) / pTileLayer->TileSpacingY;

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
                    Renderer.RenderTextureSection2d( DisplayRect.Left + ( x - X1 - 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                     DisplayRect.Top + ( y - Y1 - 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                     theApp.Tile( pTileLayer->Tileset, field ),
                                                     tintColor );
                  }
                }
              }

              // only display game objects in game layer
              if ( JREngine.m_pGameLayer == pLayer )
              {
                Renderer.SetShader( XRenderer::ST_ALPHA_TEST );

                const std::list<GR::Gamebase::GameObject*>& listObj( JREngine.m_AwakeObjects );

                std::list<GR::Gamebase::GameObject*>::const_iterator   itO( listObj.begin() );
                while ( itO != listObj.end() )
                {
                  GR::Gamebase::GameObject*   pObj = (GR::Gamebase::GameObject*)*itO;

                  int   tilePosX = pObj->Position.x / pTileLayer->TileSpacingX;
                  int   tilePosY = pObj->Position.y / pTileLayer->TileSpacingY;

                  if ( ( tilePosX < X1 - 2 )
                  &&   ( tilePosX >= X2 + 2 )
                  &&   ( tilePosY < Y1 - 2 )
                  &&   ( tilePosY >= Y2 + 2 ) )
                  {
                    // object is outside screen
                    ++itO;
                    continue;
                  }

                  if ( pObj->m_CustomRender )
                  {
                    JREngine.RaiseObjectEvent( pObj, 
                                               GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::CUSTOM_DRAW, 
                                                                          GR::tPoint( DisplayRect.Left - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                                                      DisplayRect.Top - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY ) ) );
                  }
                  else
                  {
                    if ( !pObj->Animation.empty() )
                    {
                      Renderer.RenderTextureSection2d( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                       DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                       theApp.AnimationFrame( pObj->AnimPos ) );
                    }
                    else
                    {
                      Renderer.RenderTextureSection2d( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                       DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY, 
                                                       theApp.Section( pObj->Section ) );
                    }
                  }
                  if ( DisplayBounds )
                  {
                    Renderer.SetTexture( 0, NULL );
                    Renderer.RenderRect2d( DisplayRect.position() + pObj->Bounds().position() - GR::tPoint( displayOffsetX, displayOffsetY ), pObj->Bounds().size(), 0xff00ff00 );
                  }

                  ++itO;
                }
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



    template <class APP, class OBJ> void DisplayMapWithRotatingObjects( APP& theApp, XRenderer& Renderer, GR::Gamebase::JREngine& JREngine, const GR::tRect& DisplayRect, bool DisplayBounds = false )
    {
      Renderer.SetState( XRenderer::RS_LIGHTING, XRenderer::RSV_DISABLE );

      int     referenceLayerWidth = DisplayRect.width();
      int     referenceLayerHeight = DisplayRect.height();
      if ( JREngine.m_pGameLayer )
      {
        referenceLayerWidth = JREngine.m_pGameLayer->DisplayWidth() - DisplayRect.width();
        referenceLayerHeight = JREngine.m_pGameLayer->DisplayHeight() - DisplayRect.height();
      }
      bool    firstLayer = true;

      std::vector<GR::Gamebase::Layer*>::const_iterator   itL( JREngine.m_Map.Layers.begin() );
      while ( itL != JREngine.m_Map.Layers.end() )
      {
        const GR::Gamebase::Layer*    pLayer( *itL );

        switch ( pLayer->Type )
        {
          case GR::Gamebase::Layer::LT_TILE_LAYER:
            {
              GR::u32     tintColor = 0xffffffff;

              if ( pLayer->Name == "Water" )
              {
                Renderer.SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );
                tintColor = 0x80ffffff;
              }
              else if ( ( pLayer->Name == "Foreground" )
                        || ( !firstLayer ) )
              {
                Renderer.SetShader( XRenderer::ST_ALPHA_TEST );
              }
              else
              {
                Renderer.SetShader( XRenderer::ST_FLAT );
              }
              firstLayer = false;
              GR::Gamebase::TileLayer*    pTileLayer = ( GR::Gamebase::TileLayer* )pLayer;
              //int     displayOffsetX = JREngine.m_Map.DisplayOffset.x + DisplayRect.Left - pLayer->Offset.x;
              //int     displayOffsetY = JREngine.m_Map.DisplayOffset.y + DisplayRect.Top - pLayer->Offset.y;
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
              int     X2 = ( displayOffsetX + pTileLayer->TileSpacingX - 1 + DisplayRect.width() ) / pTileLayer->TileSpacingX;
              int     Y2 = ( displayOffsetY + pTileLayer->TileSpacingY - 1 + DisplayRect.height() ) / pTileLayer->TileSpacingY;

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
                    Renderer.RenderTextureSection2d( DisplayRect.Left + ( x - X1 - 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                     DisplayRect.Top + ( y - Y1 - 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                     theApp.Tile( pTileLayer->Tileset, field ),
                                                     tintColor );
                  }
                }
              }

              // only display game objects in game layer
              if ( JREngine.m_pGameLayer == pLayer )
              {
                Renderer.SetShader( XRenderer::ST_ALPHA_TEST );

                const std::list<GR::Gamebase::GameObject*>& listObj( JREngine.m_AwakeObjects );

                std::list<GR::Gamebase::GameObject*>::const_iterator   itO( listObj.begin() );
                while ( itO != listObj.end() )
                {
                  GR::Gamebase::GameObject*   pObj = ( GR::Gamebase::GameObject* )*itO;

                  int   tilePosX = pObj->Position.x / pTileLayer->TileSpacingX;
                  int   tilePosY = pObj->Position.y / pTileLayer->TileSpacingY;

                  if ( ( tilePosX < X1 - 2 )
                       && ( tilePosX >= X2 + 2 )
                       && ( tilePosY < Y1 - 2 )
                       && ( tilePosY >= Y2 + 2 ) )
                  {
                    // object is outside screen
                    ++itO;
                    continue;
                  }

                  if ( pObj->m_CustomRender )
                  {
                    JREngine.RaiseObjectEvent( pObj,
                                               GR::Gamebase::ObjectEvent( GR::Gamebase::ObjectEventType::CUSTOM_DRAW,
                                                                          GR::tPoint( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                                                      DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY ) ) );
                  }
                  else
                  {
                    if ( !pObj->Animation.empty() )
                    {
                      Renderer.RenderTextureSectionRotated2d( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                       DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                       theApp.AnimationFrame( pObj->AnimPos ),
                                                              pObj->Angle );
                    }
                    else
                    {
                      Renderer.RenderTextureSectionRotated2d( DisplayRect.Left + pObj->Position.x + pObj->DisplayOffset.x - ( X1 + 1 ) * pTileLayer->TileSpacingX - displayOffsetX % pTileLayer->TileSpacingX,
                                                       DisplayRect.Top + pObj->Position.y + pObj->DisplayOffset.y - ( Y1 + 1 ) * pTileLayer->TileSpacingY - displayOffsetY % pTileLayer->TileSpacingY,
                                                       theApp.Section( pObj->Section ),
                                                              pObj->Angle );
                    }
                  }
                  if ( DisplayBounds )
                  {
                    Renderer.SetTexture( 0, NULL );
                    Renderer.RenderRect2d( DisplayRect.position() + pObj->Bounds().position() - GR::tPoint( displayOffsetX, displayOffsetY ), pObj->Bounds().size(), 0xff00ff00 );
                  }

                  ++itO;
                }
              }
            }
            break;
        }

        ++itL;
      }
    }

  }
}

#endif // GR_GAMEBASE_LAYEREDMAP_DISPLAYER_X_H
