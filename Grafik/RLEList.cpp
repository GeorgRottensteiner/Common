#include "RLEList.h"

#include <debug/debugclient.h>



namespace GR
{
  namespace Graphic
  {
    RLEList::RLEList() :
      m_pData( NULL ),
      m_Size( 0 )
    {
    }



    RLEList::RLEList( const RLEList& rleRHS )
    {
      m_Size = rleRHS.m_Size;
      if ( rleRHS.m_Size )
      {
        m_pData = new GR::u8[rleRHS.m_Size];
        memcpy( m_pData, rleRHS.m_pData, m_Size );
      }
      else
      {
        m_pData = NULL;
      }
    }



    RLEList& RLEList::operator =( const RLEList& rleRHS )
    {
      if ( this == &rleRHS )
      {
        return *this;
      }

      Release();

      if ( rleRHS.m_Size )
      {
        m_pData = new GR::u8[rleRHS.m_Size];
        memcpy( m_pData, rleRHS.m_pData, rleRHS.m_Size );
      }
      m_Size = rleRHS.m_Size;

      return *this;
    }



    RLEList::~RLEList()
    {
      Release();
    }



    void RLEList::Release()
    {
      if ( m_pData != NULL )
      {
        delete[] m_pData;
        m_pData = NULL;
      }
    }



    unsigned long RLEList::Compress( GR::Graphic::ImageData& ImageData, int X, int Y, int Width, int Height,
                                     GR::u32 TransparentColor, bool WriteList )
    {
      GR::u8          typeCur = ST_OPAQUE;
      GR::u8          typeLast = ST_OPAQUE;

      unsigned char   ucValue = 0;
      unsigned char*  pList = NULL;
      unsigned char*  pLineCounter = NULL;
      unsigned long   ulCounter = 0;

      TransparentColor &= 0x00ffffff;

      if ( WriteList )
      {
        Release();

        m_Size = 0;
        m_pData = new GR::u8[Compress( ImageData, X, Y, Width, Height, TransparentColor, false )];
      }
      pList = (GR::u8*)m_pData;
      for ( int i = 0; i < Height; i++ )
      {
        //dh::Log( "Zeile %lu:", i );
        pLineCounter = pList;
        pList += 2;
        ulCounter = 0;
        for ( int j = 0; j < Width; j++ )
        {
          if ( ( ImageData.GetPixelRGB( X + j, Y + i ) & 0x00ffffff ) == TransparentColor )
          {
            // durchsichtiger Pixel
            typeCur = ST_TRANSPARENT;
          }
          else
          {
            // massiver Pixel
            typeCur = ST_OPAQUE;
          }
          if ( ( typeCur != typeLast )
          &&   ( j > 0 ) )
          {
            // Noch nicht gesetzte Änderungen merken
            WriteSector( ImageData, &pList, ulCounter, typeLast, WriteList );
            ulCounter = 0;
          }
          ulCounter++;
          typeLast = typeCur;
        }
        WriteSector( ImageData, &pList, ulCounter, typeLast, WriteList );
        if ( WriteList )
        {
          *(GR::u16*)pLineCounter = (GR::u16)( pList - pLineCounter - 2 );
        }
      }
      //dh::Log( "Liste hat %lu Bytes.", pList - pCompressList );
      if ( WriteList )
      {
        m_Size = (unsigned long)( pList - ( GR::u8* )m_pData );
      }
      return (unsigned long)( pList - ( GR::u8* )m_pData );
    }



    unsigned long RLEList::CompressMask( GR::Graphic::ImageData& ImageData, int X, int Y, int Width, int Height, bool WriteList )
    {
      if ( ( ImageData.ImageFormat() != GR::Graphic::IF_A8 )
      &&   ( ImageData.ImageFormat() != GR::Graphic::IF_INDEX8 ) )
      {
        return 0;
      }

      GR::u8          typeCur = ST_OPAQUE;
      GR::u8          typeLast = ST_OPAQUE;

      unsigned char   ucValue = 0;
      unsigned char*  pList = NULL;
      unsigned char*  pLineCounter = NULL;

      unsigned long   ulCounter = 0;

      if ( WriteList )
      {
        Release();

        m_Size = 0;
        m_pData = new GR::u8[CompressMask( ImageData, X, Y, Width, Height, false )];
      }
      pList = ( GR::u8* )m_pData;
      for ( int i = 0; i < Height; i++ )
      {
        //dh::Log( "Zeile %lu:", i );
        pLineCounter = pList;
        pList += 2;
        ulCounter = 0;
        for ( int j = 0; j < Width; j++ )
        {
          GR::u8  ucAlpha = (GR::u8)ImageData.GetPixel( X + j, Y + i );

          if ( ucAlpha == 0 )
          {
            typeCur = ST_TRANSPARENT;
          }
          else if ( ucAlpha == 255 )
          {
            typeCur = ST_OPAQUE;
          }
          else
          {
            typeCur = ST_SMALL_ALPHA;
          }

          if ( ( typeCur != typeLast )
          &&   ( j > 0 ) )
          {
            // Noch nicht gesetzte Änderungen merken
            WriteSector( ImageData, &pList, ulCounter, typeLast, WriteList );
            ulCounter = 0;
          }
          ulCounter++;
          typeLast = typeCur;
        }
        WriteSector( ImageData, &pList, ulCounter, typeLast, WriteList );
        if ( WriteList )
        {
          *(GR::u16*)pLineCounter = (GR::u16)( pList - pLineCounter - 2 );
        }
      }
      //dh::Log( "Liste hat %lu Bytes.", pList - pCompressList );
      if ( WriteList )
      {
        m_Size = (unsigned long)( pList - ( GR::u8* )m_pData );
      }
      return (unsigned long)( pList - ( GR::u8* )m_pData );
    }



    unsigned long RLEList::CompressMaskA1( GR::Graphic::ImageData& ImageData, int X, int Y, int Width, int Height, bool WriteList )
    {
      if ( ImageData.ImageFormat() != GR::Graphic::IF_A1R5G5B5 )
      {
        return 0;
      }

      GR::u8          typeCur = ST_OPAQUE;
      GR::u8          typeLast = ST_OPAQUE;

      unsigned char   ucValue = 0;
      unsigned char*  pList = NULL;
      unsigned char*  pLineCounter = NULL;
      unsigned long   ulCounter = 0;


      if ( WriteList )
      {
        Release();

        m_Size = 0;
        m_pData = new GR::u8[CompressMaskA1( ImageData, X, Y, Width, Height, false )];
      }
      pList = ( GR::u8* )m_pData;
      for ( int i = 0; i < Height; i++ )
      {
        //dh::Log( "Zeile %lu:", i );
        pLineCounter = pList;
        pList += 2;
        ulCounter = 0;
        for ( int j = 0; j < Width; j++ )
        {
          if ( ImageData.GetPixel( X + j, Y + i ) & 0x8000 )
          {
            typeCur = ST_OPAQUE;
          }
          else
          {
            typeCur = ST_TRANSPARENT;
          }

          if ( ( typeCur != typeLast )
          &&   ( j > 0 ) )
          {
            // Noch nicht gesetzte Änderungen merken
            WriteSector( ImageData, &pList, ulCounter, typeLast, WriteList );
            ulCounter = 0;
          }
          ulCounter++;
          typeLast = typeCur;
        }
        WriteSector( ImageData, &pList, ulCounter, typeLast, WriteList );
        if ( WriteList )
        {
          *(GR::u16*)pLineCounter = (GR::u16)( pList - pLineCounter - 2 );
        }
      }
      //dh::Log( "Liste hat %lu Bytes.", pList - pCompressList );
      if ( WriteList )
      {
        m_Size = (unsigned long)( pList - ( GR::u8* )m_pData );
      }
      return (unsigned long)( pList - ( GR::u8* )m_pData );
    }



    void RLEList::WriteSector( GR::Graphic::ImageData& ImageData,
                               unsigned char** ppList,
                               const unsigned long Size,
                               const GR::u8 Flag,
                               const bool Write )
    {
      unsigned long size = Size;

      if ( size == ImageData.Width() )
      {
        //dh::Log( "Ne ganze Zeile." );
        if ( Write )
        {
          **ppList = Flag;
        }
        ( *ppList )++;
      }
      else
      {
        while ( size > RLE_MAX_COMPRESS_SECTOR )
        {
          //dh::Log( "127 Pixel." );
          if ( Write )
          {
            **ppList = Flag + RLE_MAX_COMPRESS_SECTOR;
          }
          ( *ppList )++;
          size -= RLE_MAX_COMPRESS_SECTOR;
        }
        //dh::Log( "%lu Pixel.", ulSize );
        if ( Write )
        {
          **ppList = Flag + (unsigned char)size;
        }
        ( *ppList )++;
      }
    }

  }

}