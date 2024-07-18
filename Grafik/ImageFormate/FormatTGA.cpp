#include <IO/FileStream.h>

#include <String/StringUtil.h>

#include "FormatTGA.h"



GR::Graphic::FormatTGA    globalTGAPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatTGA::FormatTGA()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatTGA::IsFileOfType( const GR::String& FileName )
    {
      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        return false;
      }

      char   ucHeader[26];

      aFile.SetPosition( aFile.GetSize() - 26, IIOStream::PT_SET );

      aFile.ReadBlock( ucHeader, 26 );

      if ( strncmp( ucHeader + 8, "TRUEVISION-XFILE", 16 ) == 0 )
      {
        aFile.Close();
        return true;
      }

      aFile.Close();
      size_t   iLength = FileName.length();
      if ( iLength >= 3 )
      {
        if ( ( toupper( FileName[iLength - 3] ) != 'T' )
             || ( toupper( FileName[iLength - 2] ) != 'G' )
             || ( toupper( FileName[iLength - 1] ) != 'A' ) )
        {
          return false;
        }
        return true;
      }
      else
      {
        return false;
      }

      aFile.SetPosition( 0, IIOStream::PT_SET );
      aFile.ReadBlock( ucHeader, 18 );
      aFile.Close();

      // wir versuchen, anhand der Image-Typen zu identifizieren
      if ( ( ucHeader[2] == 9 )
           || ( ucHeader[2] == 10 )
           || ( ucHeader[2] == 11 )
           || ( ucHeader[2] == 1 )
           || ( ucHeader[2] == 2 )
           || ( ucHeader[2] == 3 ) )
      {
        return true;
      }
      return false;
    }



    ImageSet* FormatTGA::LoadSet( const GR::String& FileName )
    {
      int             i,
        j;

      BOOL            bRLE;

      unsigned char   ucHeader[18],
        ucDummy,
        ucPixel;

      WORD            wWidth,
        wHeight;

      GR::u32           dwPixel;

      BYTE            bPixel[4];

      BYTE* pPointer;

      GR::IO::FileStream     aFile;


      bRLE = FALSE;

      if ( !aFile.Open( FileName, IIOStream::OT_READ_ONLY ) )
      {
        return NULL;
      }
      aFile.ReadBlock( ucHeader, 18 );
      if ( ucHeader[16] == 15 )
      {
        ucHeader[16] = 16;
      }
      wWidth = ucHeader[12] + 256 * ucHeader[13];
      wHeight = ucHeader[14] + 256 * ucHeader[15];

      GR::Graphic::ImageData* pData = NULL;

      if ( ucHeader[16] == 8 )
      {
        pData = new GR::Graphic::ImageData();
        pData->CreateData( wWidth, wHeight, GR::Graphic::IF_PALETTED );
      }
      else if ( ucHeader[16] == 16 )
      {
        pData = new GR::Graphic::ImageData();
        pData->CreateData( wWidth, wHeight, GR::Graphic::IF_X1R5G5B5 );
      }
      else if ( ucHeader[16] == 24 )
      {
        pData = new GR::Graphic::ImageData();
        pData->CreateData( wWidth, wHeight, GR::Graphic::IF_R8G8B8 );
      }
      else if ( ucHeader[16] == 32 )
      {
        pData = new GR::Graphic::ImageData();
        pData->CreateData( wWidth, wHeight, GR::Graphic::IF_A8R8G8B8 );
      }

      if ( pData == NULL )
      {
        aFile.Close();
        return NULL;
      }

      // ID-Feld überlesen
      aFile.SetPosition( ucHeader[0], IIOStream::PT_CURRENT );

      bool  bUpsideDown = false;
      if ( ucHeader[17] & 0x20 )
      {
        bUpsideDown = true;
      }

      // Palette
      if ( ucHeader[16] == 8 )
      {
        if ( ucHeader[1] == 1 )
        {
          int   iEntries = ucHeader[6] * 256 + ucHeader[5];
          int   iFirstIndex = ucHeader[4] * 256 + ucHeader[3];

          //ucHeader[7] = 24;
          for ( i = 0; i < iEntries; i++ )
          {
            aFile.ReadBlock( &ucHeader, 3 );
            pData->Palette().SetColor( iFirstIndex + i, ucHeader[2], ucHeader[1], ucHeader[0] );
          }
        }
        else
        {
          // Graustufen
          for ( i = 0; i < 256; i++ )
          {
            pData->Palette().SetColor( i, i, i, i );
          }
        }
      }

      int   iCurrentLine = wHeight - 1,
        iCurrentLineStep = -1;
      if ( bUpsideDown )
      {
        iCurrentLine = 0;
        iCurrentLineStep = 1;
      }

      if ( ( ucHeader[2] == 9 )
      ||   ( ucHeader[2] == 10 )
      ||   ( ucHeader[2] == 11 ) )
      {
        // dieses Bild ist RL-Encoded
        bRLE = TRUE;
        pPointer = (BYTE*)pData->Data() + iCurrentLine * wWidth * ucHeader[16] / 8;
        i = 0;
        do
        {
          ucDummy = aFile.ReadU8();
          if ( ucDummy & 0x80 )
          {
            // ein RLE-Paket
            switch ( ucHeader[16] )
            {
              case 8:
                ucDummy &= 0x7f;
                ucPixel = aFile.ReadU8();
                ucDummy++;
                do
                {
                  ucDummy--;

                  *pPointer++ = ucPixel;
                  i++;
                }
                while ( ucDummy > 0 );
                break;
              case 16:
                break;
              case 24:
                ucDummy &= 0x7f;
                bPixel[0] = aFile.ReadU8();
                bPixel[1] = aFile.ReadU8();
                bPixel[2] = aFile.ReadU8();
                ucDummy++;
                do
                {
                  ucDummy--;

                  *pPointer++ = bPixel[0];
                  *pPointer++ = bPixel[1];
                  *pPointer++ = bPixel[2];
                  i++;

                  if ( ( i % wWidth ) == 0 )
                  {
                    // ein Zeilenende!
                    iCurrentLine += iCurrentLineStep;
                    pPointer = (BYTE*)pData->Data() + iCurrentLine * wWidth * ucHeader[16] / 8;
                  }

                }
                while ( ucDummy > 0 );
                break;
              case 32:
                ucDummy &= 0x7f;
                dwPixel = ( GR::u32 )aFile.ReadU32();
                ucDummy++;
                do
                {
                  ucDummy--;

                  *( GR::u32* )pPointer = dwPixel;
                  pPointer += 4;
                  i++;

                  if ( ( i % wWidth ) == 0 )
                  {
                    // ein Zeilenende!
                    iCurrentLine += iCurrentLineStep;
                    pPointer = (BYTE*)pData->Data() + iCurrentLine * wWidth * ucHeader[16] / 8;
                  }

                }
                while ( ucDummy > 0 );
                break;
            }
          }
          else
          {
            // ein Raw-Data-Paket
            switch ( ucHeader[16] )
            {
              case 8:
                ucDummy &= 0x7f;
                ucDummy++;
                do
                {
                  ucDummy--;

                  *pPointer++ = aFile.ReadU8();
                  i++;

                  if ( ( i % wWidth ) == 0 )
                  {
                    // ein Zeilenende!
                    iCurrentLine += iCurrentLineStep;
                    pPointer = (BYTE*)pData->Data() + iCurrentLine * wWidth * ucHeader[16] / 8;
                  }
                }
                while ( ucDummy > 0 );
                break;
              case 16:
                break;
              case 24:
                ucDummy &= 0x7f;
                ucDummy++;
                do
                {
                  ucDummy--;

                  *pPointer++ = aFile.ReadU8();
                  *pPointer++ = aFile.ReadU8();
                  *pPointer++ = aFile.ReadU8();
                  i++;

                  if ( ( i % wWidth ) == 0 )
                  {
                    // ein Zeilenende!
                    iCurrentLine += iCurrentLineStep;
                    pPointer = (BYTE*)pData->Data() + iCurrentLine * wWidth * ucHeader[16] / 8;
                  }

                }
                while ( ucDummy > 0 );
                break;
              case 32:
                ucDummy &= 0x7f;
                ucDummy++;
                do
                {
                  ucDummy--;

                  *( GR::u32* )pPointer = ( GR::u32 )aFile.ReadU32();
                  pPointer += 4;
                  i++;

                  if ( ( i % wWidth ) == 0 )
                  {
                    // ein Zeilenende!
                    iCurrentLine += iCurrentLineStep;
                    pPointer = (BYTE*)pData->Data() + iCurrentLine * wWidth * ucHeader[16] / 8;
                  }

                }
                while ( ucDummy > 0 );
                break;
            }
          }
        }
        while ( i < wWidth * wHeight );
      }
      else
      {
        // ein umkomprimiertes Bild
        for ( j = wHeight - 1; j >= 0; j-- )
        {
          switch ( ucHeader[16] )
          {
            case 8:
              aFile.ReadBlock( (BYTE*)( (BYTE*)pData->Data() + iCurrentLine * wWidth ), wWidth );
              break;
            case 16:
              aFile.ReadBlock( (BYTE*)( (BYTE*)pData->Data() + iCurrentLine * 2 * wWidth ), 2 * wWidth );
              break;
            case 24:
              aFile.ReadBlock( (BYTE*)( (BYTE*)pData->Data() + iCurrentLine * 3 * wWidth ), 3 * wWidth );
              break;
            case 32:
              // das oberste Byte kann Alpha enthalten!
              aFile.ReadBlock( (BYTE*)( (BYTE*)pData->Data() + iCurrentLine * 4 * wWidth ), 4 * wWidth );
              break;
          }
          iCurrentLine += iCurrentLineStep;
        }
      }
      aFile.Close();

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pData );

      return pSet;
    }



    bool FormatTGA::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( pData == NULL )
      {
        return false;
      }
      switch ( pData->ImageFormat() )
      {
        case GR::Graphic::IF_A8R8G8B8:
        case GR::Graphic::IF_X8R8G8B8:
        case GR::Graphic::IF_R8G8B8:
        case GR::Graphic::IF_X1R5G5B5:
        case GR::Graphic::IF_PALETTED:
          return true;
      }
      return false;

    }



    bool FormatTGA::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      if ( !CanSave( pData ) )
      {
        return false;
      }

      int             j;

      BOOL            bRLE;

      unsigned char   ucHeader[18];

      WORD            wWidth,
        wHeight;

      GR::IO::FileStream       MyFile;


      bRLE = FALSE;
      if ( !MyFile.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      // Header zusammenbauen
      memset( ucHeader, 0, 18 );

      ucHeader[0] = 0;
      ucHeader[1] = 0;
      ucHeader[2] = 2;
      ucHeader[9] = 1;

      ucHeader[12] = pData->Width() % 256;
      ucHeader[13] = pData->Width() / 256;
      ucHeader[14] = pData->Height() % 256;
      ucHeader[15] = pData->Height() / 256;

      switch ( pData->ImageFormat() )
      {
        case GR::Graphic::IF_PALETTED:
          ucHeader[16] = 8;
          ucHeader[2] = 1;
          // eine Palette ist vorhanden
          if ( pData->Palette().Data() )
          {
            ucHeader[1] = 1;
            ucHeader[3] = 0;    // index of first color map entry
            ucHeader[4] = 0;    // index of first color map entry
            ucHeader[5] = 256 % 256; // total number of entries
            ucHeader[6] = 256 / 256;
            ucHeader[7] = 24;
          }
          break;
        case GR::Graphic::IF_X1R5G5B5:
          ucHeader[16] = 16;
          break;
        case GR::Graphic::IF_R8G8B8:
          ucHeader[16] = 24;
          if ( pMask )
          {
            ucHeader[16] = 32;
          }
          break;
        case GR::Graphic::IF_X8R8G8B8:
        case GR::Graphic::IF_A8R8G8B8:
          ucHeader[16] = 32;
          break;
      }

      MyFile.WriteBlock( &ucHeader, 18 );


      // ID-Feld überlesen
      MyFile.SetPosition( ucHeader[0], IIOStream::PT_CURRENT );

      if ( ucHeader[16] == 8 )
      {
        if ( ( ucHeader[1] == 1 )
             && ( pData->Palette().Data() ) )
        {
          for ( int i = 0; i < 256; ++i )
          {
            MyFile.WriteU8( pData->Palette().Blue( i ) );
            MyFile.WriteU8( pData->Palette().Green( i ) );
            MyFile.WriteU8( pData->Palette().Red( i ) );
          }
        }
      }

      // ein umkomprimiertes Bild
      GR::Graphic::ImageData* pTempData = NULL;
      if ( ( pMask != NULL )
      &&   ( pData->BitsProPixel() == 24 ) )
      {
        pTempData = new GR::Graphic::ImageData( *pData );

        pTempData->ConvertSelfTo( GR::Graphic::IF_X8R8G8B8 );
      }

      wHeight = pData->Height();
      wWidth = pData->Width();
      for ( j = wHeight - 1; j >= 0; j-- )
      {
        switch ( ucHeader[16] )
        {
          case 8:
            MyFile.WriteBlock( (BYTE*)( (BYTE*)pData->Data() + j * wWidth ), wWidth );
            break;
          case 16:
            MyFile.WriteBlock( (BYTE*)( (BYTE*)pData->Data() + j * 2 * wWidth ), 2 * wWidth );
            break;
          case 24:
            MyFile.WriteBlock( (BYTE*)( (BYTE*)pData->Data() + j * 3 * wWidth ), 3 * wWidth );
            break;
          case 32:
            {
              // die Alpha-Werte von der Maske eintragen
              if ( pTempData )
              {
                for ( int i = 0; i < wWidth; i++ )
                {
                  *( (BYTE*)pTempData->Data() + j * 4 * wWidth + i * 4 + 3 ) =
                    *( (BYTE*)pMask->Data() + j * wWidth + i );
                }
                MyFile.WriteBlock( (BYTE*)( (BYTE*)pTempData->Data() + j * 4 * wWidth ), 4 * wWidth );
                break;
              }
              MyFile.WriteBlock( (BYTE*)( (BYTE*)pData->Data() + j * 4 * wWidth ), 4 * wWidth );
            }
            break;
        }
      }

      MyFile.Close();

      if ( pTempData )
      {
        delete pTempData;
      }

      return true;
    }



    GR::String FormatTGA::GetDescription()
    {
      return "TGA Image";
    }



    GR::String FormatTGA::GetFilterString()
    {
      return "*.tga";
    }

  }
}