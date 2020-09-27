#include <IO/FileStream.h>

#include <Grafik/Image.h>

#include <Grafik\ImageFormate\FormatBTN.h>

#include <String/StringUtil.h>



GR::Graphic::FormatBTN    globalBTNPlugin;


namespace GR
{
  namespace Graphic
  {

    FormatBTN::FormatBTN()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatBTN::IsFileOfType( const GR::String& FileName )
    {
      size_t   iLength = FileName.length();
      if ( iLength >= 3 )
      {
        if ( ( toupper( FileName[iLength - 3] ) != 'B' )
             || ( toupper( FileName[iLength - 2] ) != 'T' )
             || ( toupper( FileName[iLength - 1] ) != 'N' ) )
        {
          return false;
        }
        return true;
      }
      else
      {
        return false;
      }

    }



    ImageSet* FormatBTN::LoadSet( const GR::String& FileName )
    {
      size_t   iLength = FileName.length();
      if ( iLength >= 3 )
      {
        if ( ( toupper( FileName[iLength - 3] ) != 'B' )
             || ( toupper( FileName[iLength - 2] ) != 'T' )
             || ( toupper( FileName[iLength - 1] ) != 'N' ) )
        {
          return NULL;
        }
      }
      else
      {
        return NULL;
      }

      GR::IO::FileStream    aFile;
      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_READ_ONLY ) )
      {
        return NULL;
      }
      GR::Graphic::ImageData* pData = new GR::Graphic::ImageData();

      WORD    wWidth = aFile.ReadU16();
      WORD    wHeight = aFile.ReadU16();

      pData->CreateData( wWidth, wHeight, GR::Graphic::IF_PALETTED );

      aFile.ReadBlock( pData->Data(), pData->DataSize() );

      aFile.Close();

      // BTNs haben keine Palette, Graustufen reinsetzen
      unsigned char         ucPalColors[768] = { 0,0,0,0,0,42,0,42,0,0,42,42,42,0,0,42,0,42,42,21,0,42,42,42,21,21,21,21,21,63,21,
                                                63,21,21,63,63,63,21,21,63,21,63,63,63,21,63,63,63,59,59,59,55,55,55,52,52,52,48,
                                                48,48,45,45,45,42,42,42,38,38,38,35,35,35,31,31,31,28,28,28,25,25,25,21,21,21,18,
                                                18,18,14,14,14,11,11,11,8,8,8,63,0,0,59,0,0,56,0,0,53,0,0,50,0,0,47,0,0,44,0,0,41,
                                                0,0,38,0,0,34,0,0,31,0,0,28,0,0,25,0,0,22,0,0,19,0,0,16,0,0,63,54,54,63,46,46,63,
                                                39,39,63,31,31,63,23,23,63,16,16,63,8,8,63,0,0,63,42,23,63,38,16,63,34,8,63,30,0,
                                                57,27,0,51,24,0,45,21,0,39,19,0,63,63,54,63,63,46,63,63,39,63,63,31,63,62,23,63,61,
                                                16,63,61,8,63,61,0,57,54,0,51,49,0,45,43,0,39,39,0,33,33,0,28,27,0,22,21,0,16,16,
                                                0,52,63,23,49,63,16,45,63,8,40,63,0,36,57,0,32,51,0,29,45,0,24,39,0,54,63,54,47,63,
                                                46,39,63,39,32,63,31,24,63,23,16,63,16,8,63,8,0,63,0,0,63,0,0,59,0,0,56,0,0,53,0,
                                                1,50,0,1,47,0,1,44,0,1,41,0,1,38,0,1,34,0,1,31,0,1,28,0,1,25,0,1,22,0,1,19,0,1,16,
                                                0,54,63,63,46,63,63,39,63,63,31,63,62,23,63,63,16,63,63,8,63,63,0,63,63,0,57,57,0,
                                                51,51,0,45,45,0,39,39,0,33,33,0,28,28,0,22,22,0,16,16,23,47,63,16,44,63,8,42,63,0,
                                                39,63,0,35,57,0,31,51,0,27,45,0,23,39,54,54,63,46,47,63,39,39,63,31,32,63,23,24,63,
                                                16,16,63,8,9,63,0,1,63,0,0,63,0,0,59,0,0,56,0,0,53,0,0,50,0,0,47,0,0,44,0,0,41,0,
                                                0,38,0,0,34,0,0,31,0,0,28,0,0,25,0,0,22,0,0,19,0,0,16,60,54,63,57,46,63,54,39,63,
                                                52,31,63,50,23,63,47,16,63,45,8,63,42,0,63,38,0,57,32,0,51,29,0,45,24,0,39,20,0,33,
                                                17,0,28,13,0,22,10,0,16,63,54,63,63,46,63,63,39,63,63,31,63,63,23,63,63,16,63,63,
                                                8,63,63,0,63,56,0,57,50,0,51,45,0,45,39,0,39,33,0,33,27,0,28,22,0,22,16,0,16,63,58,
                                                55,63,56,52,63,54,49,63,53,47,63,51,44,63,49,41,63,47,39,63,46,36,63,44,32,63,41,
                                                28,63,39,24,60,37,23,58,35,22,55,34,21,52,32,20,50,31,19,47,30,18,45,28,17,42,26,
                                                16,40,25,15,39,24,14,36,23,13,34,22,12,32,20,11,29,19,10,27,18,9,23,16,8,21,15,7,
                                                18,14,6,16,12,6,14,11,5,10,8,3,38,38,47,33,33,41,28,28,36,23,23,31,19,19,25,14,14,
                                                20,9,9,15,5,5,10,63,59,9,63,55,7,63,52,6,63,48,5,63,45,3,63,41,2,63,38,1,63,35,0,
                                                57,31,0,52,27,0,46,23,0,41,20,0,36,16,0,30,12,0,25,8,0,20,5,0,63,35,0,63,29,0,63,
                                                23,0,63,17,0,63,11,0,63,5,0,63,0,0,0,0,0 };

      for ( int i = 0; i < 256; i++ )
      {
        pData->Palette().SetColor( i, ucPalColors[i * 3] << 2, ucPalColors[i * 3 + 1] << 2, ucPalColors[i * 3 + 2] << 2 );
      }

      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pData );

      return pSet;
    }



    bool FormatBTN::CanSave( GR::Graphic::ImageData* pData )
    {

      if ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      {
        return true;
      }
      return false;

    }



    bool FormatBTN::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      GR::IO::FileStream      aFile;

      if ( !aFile.Open( FileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      aFile.WriteU16( pData->Width() );
      aFile.WriteU16( pData->Height() );

      aFile.WriteBlock( pData->Data(), pData->DataSize() );

      aFile.Close();

      return true;
    }



    GR::String  FormatBTN::GetFilterString()
    {
      return "*.btn";
    }



    GR::String  FormatBTN::GetDescription()
    {
      return "BTN Image";
    }

  }
}