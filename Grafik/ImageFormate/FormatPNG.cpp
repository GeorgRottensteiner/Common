#include <Grafik/Image.h>

#include <Grafik\ImageFormate\FormatPNG.h>

extern "C"
{
  #include <Grafik/ImageFormate/PNG/png.h>
}

#include <String/StringUtil.h>



namespace GR
{
  namespace Graphic
  {
    FormatPNG    s_GlobalPNGPlugin;



    FormatPNG::FormatPNG()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatPNG::IsFileOfType( const GR::String& FileName )
    {
      size_t   iLength = FileName.length();
      if ( iLength >= 3 )
      {
        if ( ( toupper( FileName[iLength - 3] ) != 'P' )
          || ( toupper( FileName[iLength - 2] ) != 'N' )
          || ( toupper( FileName[iLength - 1] ) != 'G' ) )
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



    ImageSet* FormatPNG::LoadSet( const GR::String& FileName )
    {
      size_t   length = FileName.length();
      if ( length >= 3 )
      {
        if ( ( toupper( FileName[length - 3] ) != 'P' )
          || ( toupper( FileName[length - 2] ) != 'N' )
          || ( toupper( FileName[length - 1] ) != 'G' ) )
        {
          return NULL;
        }
      }
      else
      {
        return NULL;
      }

      static FILE*        pfFile;
      png_byte            pbSig[8];
      int                 bitDepth = 0;
      int                 colorType = 0;
      double              dGamma = 0;
      png_color_16*       pBackground = NULL;
      png_uint_32         ulChannels;
      png_uint_32         ulRowBytes;
      png_byte*           pbImageData = NULL;
      static png_byte*    *ppbRowPointers = NULL;
      int                 i;
      png_structp         png_ptr = NULL;
      png_infop           info_ptr = NULL;
      png_uint_32         width = 0,
        height = 0;

      png_color           BkgColor;
      png_color*          pBkgColor = &BkgColor;

      // open the PNG input file

#if OPERATING_SYSTEM == OS_WINDOWS
      if ( _wfopen_s( &pfFile, GR::Convert::ToUTF16( FileName ).c_str(), L"rb" ) )
#else
      if ( ( pfFile = fopen( FileName.c_str(), "rb" ) ) == NULL )
#endif
      {
        pbImageData = NULL;
        return NULL;
      }

      // first check the eight GR::u8 PNG signature
      fread( pbSig, 1, 8, pfFile );
      if ( !png_check_sig( pbSig, 8 ) )
      {
        pbImageData = NULL;
        fclose( pfFile );
        pfFile = NULL;
        return NULL;
      }

      // create the two png(-info) structures
      png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL );
      if ( png_ptr == NULL )
      {
        pbImageData = NULL;
        fclose( pfFile );
        pfFile = NULL;
        return NULL;
      }

      info_ptr = png_create_info_struct( png_ptr );
      if ( info_ptr == NULL )
      {
        png_destroy_read_struct( &png_ptr, NULL, NULL );
        pbImageData = NULL;
        fclose( pfFile );
        pfFile = NULL;
        return NULL;
      }

      //try
      {

        // initialize the png structure

#if !defined(PNG_NO_STDIO)
        png_init_io( png_ptr, pfFile );
#else
        png_set_read_fn( png_ptr, (png_voidp)pfFile, png_read_data );
#endif

        png_set_sig_bytes( png_ptr, 8 );

        // read all PNG info up to image data

        png_read_info( png_ptr, info_ptr );

        // get width, height, bit-depth and color-type

        png_get_IHDR( png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL );

        /*
        dh::Log( "PNG %dx%dx%d  - colortype %x\n", iWidth, iHeight, iBitDepth, iColorType );

        dh::Log( "Channels per pixel %d\n", info_ptr->channels );
        dh::Log( "pixel_depth (bpp) %d\n", info_ptr->pixel_depth );
        */

        // expand images of all color-type and bit-depth to 3x8 bit RGB images
        // let the library process things like alpha, transparency, background

        if ( bitDepth == 16 )
        {
          png_set_strip_16( png_ptr );
        }
        /*
        if (iColorType == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
            */
            /*
            if (iBitDepth < 8)
                png_set_expand(png_ptr);
                */
        if ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
        {
          png_set_expand( png_ptr );
        }
        /*
        if (iColorType == PNG_COLOR_TYPE_GRAY ||
            iColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);
            */

            /*
            // set the background color to draw transparent and alpha images over.
            if (png_get_bKGD(png_ptr, info_ptr, &pBackground))
            {
                png_set_background(png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
                pBkgColor->red   = (GR::u8) pBackground->red;
                pBkgColor->green = (GR::u8) pBackground->green;
                pBkgColor->blue  = (GR::u8) pBackground->blue;
            }
            else
            {
                pBkgColor = NULL;
            }
            */

            // if required set gamma conversion
            /*
            if (png_get_gAMA(png_ptr, info_ptr, &dGamma))
                png_set_gamma(png_ptr, (double) 2.2, dGamma);
                */

                // after the transformations have been registered update info_ptr data

        png_read_update_info( png_ptr, info_ptr );

        // get again width, height and the new bit-depth and color-type

        png_get_IHDR( png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL );


        // row_bytes is the width x number of channels
        ulRowBytes = (png_uint_32)png_get_rowbytes( png_ptr, info_ptr );
        ulChannels = png_get_channels( png_ptr, info_ptr );

        // now we can allocate memory to store the image
        if ( pbImageData )
        {
          free( pbImageData );
          pbImageData = NULL;
        }
        if ( ( pbImageData = (png_byte*)malloc( ulRowBytes * ( height ) * sizeof( png_byte ) ) ) == NULL )
        {
          png_error( png_ptr, "Visual PNG: out of memory" );
        }

        // and allocate memory for an array of row-pointers
        if ( ( ppbRowPointers = (png_bytepp)malloc( height * sizeof( png_bytep ) ) ) == NULL )
        {
          png_error( png_ptr, "Visual PNG: out of memory" );
        }

        // set the individual row-pointers to point at the correct offsets

        for ( i = 0; i < (int)height; i++ )
        {
          ppbRowPointers[i] = pbImageData + i * ulRowBytes;
        }

        // now we can go ahead and just read the whole image
        png_read_image( png_ptr, ppbRowPointers );

        // read the additional chunks in the PNG file (not really needed)
        png_read_end( png_ptr, NULL );

        // and we're done
        free( ppbRowPointers );
        ppbRowPointers = NULL;
      }

      /*
      catch (msg)
      {
          png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

          *ppbImageData = pbImageData = NULL;

          if(ppbRowPointers)
              free (ppbRowPointers);

          fclose(pfFile);

          return FALSE;
      }
      */

      fclose( pfFile );

      GR::Graphic::ImageData*     pData = new GR::Graphic::ImageData();
      GR::Graphic::ImageData*     pDataMask = NULL;

      if ( ( colorType == PNG_COLOR_TYPE_PALETTE )
        || ( colorType == PNG_COLOR_TYPE_GRAY ) )
      {
        if ( bitDepth == 1 )
        {
          pData->CreateData( width, height, GR::Graphic::IF_MONOCHROME );
        }
        else if ( bitDepth == 2 )
        {
          pData->CreateData( width, height, GR::Graphic::IF_INDEX2 );
        }
        else if ( bitDepth == 4 )
        {
          pData->CreateData( width, height, GR::Graphic::IF_INDEX4 );
        }
        else if ( bitDepth == 8 )
        {
          pData->CreateData( width, height, GR::Graphic::IF_PALETTED );
        }
        memcpy( pData->Data(), pbImageData, pData->DataSize() );

        if ( colorType == PNG_COLOR_TYPE_GRAY )
        {
          for ( int i = 0; i < ( 1 << bitDepth ); ++i )
          {
            pData->Palette().SetColor( i,
              ( 255 * i ) / ( ( 1 << bitDepth ) - 1 ),
              ( 255 * i ) / ( ( 1 << bitDepth ) - 1 ),
              ( 255 * i ) / ( ( 1 << bitDepth ) - 1 ) );
          }
        }
        else
        {
          png_color*      pTempColors = NULL;
          int             numColors = 0;

          if ( png_get_PLTE( png_ptr, info_ptr, &pTempColors, &numColors ) == PNG_INFO_PLTE )
          {
            for ( int i = 0; i < numColors; ++i )
            {
              pData->Palette().SetColor( i, pTempColors[i].red, pTempColors[i].green, pTempColors[i].blue );
            }
          }
        }
      }
      else if ( colorType == PNG_COLOR_MASK_ALPHA )
      {
        if ( bitDepth == 8 )
        {
          pData->CreateData( width, height, GR::Graphic::IF_PALETTED );
          pDataMask = new GR::Graphic::ImageData();
          pDataMask->CreateData( width, height, GR::Graphic::IF_PALETTED );
          GR::u8*   pSource = ( GR::u8* )pbImageData;
          GR::u8*   pTarget = ( GR::u8* )pData->Data();
          GR::u8*   pTargetMask = ( GR::u8* )pDataMask->Data();
          for ( unsigned j = 0; j < height; ++j )
          {
            for ( unsigned i = 0; i < width; ++i )
            {
              *pTarget++ = *pSource++;
              *pTargetMask++ = *pSource++;
            }
          }
        }
        for ( int i = 0; i < ( 1 << bitDepth ); ++i )
        {
          pData->Palette().SetColor( i,
            ( 255 * i ) / ( ( 1 << bitDepth ) - 1 ),
            ( 255 * i ) / ( ( 1 << bitDepth ) - 1 ),
            ( 255 * i ) / ( ( 1 << bitDepth ) - 1 ) );
        }
      }
      else if ( colorType == PNG_COLOR_TYPE_RGB_ALPHA )
      {
        if ( bitDepth == 8 )
        {
          if ( ulChannels == 4 )
          {
            // alpha
            pData->CreateData( width, height, GR::Graphic::IF_A8R8G8B8 );
          }
          else
          {
            pData->CreateData( width, height, GR::Graphic::IF_R8G8B8 );
          }
          pDataMask = new GR::Graphic::ImageData();
          pDataMask->CreateData( width, height, GR::Graphic::IF_PALETTED );
          GR::u8*   pSource = ( GR::u8* )pbImageData;
          GR::u8*   pTarget = ( GR::u8* )pData->Data();
          GR::u8*   pTargetMask = ( GR::u8* )pDataMask->Data();
          for ( unsigned j = 0; j < height; ++j )
          {
            for ( unsigned i = 0; i < width; ++i )
            {
              if ( ulChannels == 4 )
              {
                *( pTarget + 2 ) = *pSource++;
                *( pTarget + 1 ) = *pSource++;
                *pTarget = *pSource++;
                *( pTarget + 3 ) = *pSource;
                *pTargetMask++ = *pSource++;
                pTarget += 4;
              }
              else
              {
                *( pTarget + 2 ) = *pSource++;
                *( pTarget + 1 ) = *pSource++;
                *pTarget = *pSource++;
                pTarget += 3;
                *pTargetMask++ = *pSource++;
              }
            }
          }
        }
      }
      else if ( colorType == PNG_COLOR_TYPE_RGB )
      {
        if ( bitDepth == 8 )
        {
          pData->CreateData( width, height, GR::Graphic::IF_R8G8B8 );
          GR::u8*   pSource = ( GR::u8* )pbImageData;
          GR::u8*   pTarget = ( GR::u8* )pData->Data();
          for ( unsigned j = 0; j < height; ++j )
          {
            for ( unsigned i = 0; i < width; ++i )
            {
              *( pTarget + 2 ) = *pSource++;
              *( pTarget + 1 ) = *pSource++;
              *pTarget = *pSource++;
              pTarget += 3;
            }
          }
        }
      }

      png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
      free( pbImageData );

      ImageSet*    pSet = new ImageSet();

      pSet->AddFrame( pData, pDataMask );

      return pSet;
    }



    bool FormatPNG::CanSave( GR::Graphic::ImageData* pData )
    {
      return false;
    }



    bool FormatPNG::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      /*
      GR::IO::FileStream      aFile;

      if ( aFile.Open( szFileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      aFile.WriteU16( pData->m_iWidth );
      aFile.WriteU16( pData->m_iHeight );

      aFile.WriteBlock( pData->m_pData, pData->DataSize() );

      aFile.Close();

      return true;
      */
      return false;
    }



    GR::String FormatPNG::GetDescription()
    {
      return "PNG Image";
    }



    GR::String FormatPNG::GetFilterString()
    {
      return "*.png";
    }
  }
}