#define D_PROGRESSIVE_SUPPORTED
#include <IO/FileStream.h>

#include <Grafik\ImageFormate\FormatJPG.h>

#include <String/StringUtil.h>

#include "Jpeg\jinclude.h"
#include "Jpeg\jmorecfg.h"
#include "Jpeg\jpeglib.h"

#include "Jpeg\Jpeg.cpp"

/*
#include "Jpeg\jpegint.h"
#include "Jpeg\jerror.h"

#include "Jpeg\Jpeg.cpp"

#include "Jpeg\jdapimin.c"
#include "Jpeg\jdapistd.c"
#include "Jpeg\jcomapi.c"

#include "Jpeg\Jdmaster.c"
#include "Jpeg\jdinput.c"
#include "Jpeg\jdmainct.c"
#include "Jpeg\jdcoefct.c"
#include "Jpeg\jdpostct.c"
#include "Jpeg\jdmarker.c"
#include "Jpeg\jdhuff.c"
#include "Jpeg\jdphuff.c"
#include "Jpeg\jddctmgr.c"
#include "Jpeg\jidctint.c"
#include "Jpeg\jidctfst.c"
#include "Jpeg\jidctflt.c"
#include "Jpeg\jidctred.c"
#include "Jpeg\jdsample.c"
#include "Jpeg\jdcolor.c"
#include "Jpeg\jdmerge.c"
#include "Jpeg\jquant1.c"
#include "Jpeg\jquant2.c"
#include "Jpeg\jdatasrc.c"

#include "Jpeg\jerror.c"
#include "Jpeg\jmemmgr.c"
#include "Jpeg\jmemansi.c"
#include "Jpeg\jutils.c"
*/



GR::Graphic::FormatJPG    globalJPGPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatJPG::FormatJPG()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatJPG::IsFileOfType( const GR::String& FileName )
    {
      size_t   iLength = FileName.length();
      if ( iLength >= 4 )
      {
        if ( ( toupper( FileName[iLength - 3] ) == 'J' )
             && ( toupper( FileName[iLength - 2] ) == 'P' )
             && ( toupper( FileName[iLength - 1] ) == 'G' ) )
        {
          return true;
        }
        if ( ( toupper( FileName[iLength - 4] ) == 'J' )
             && ( toupper( FileName[iLength - 3] ) == 'P' )
             && ( toupper( FileName[iLength - 2] ) == 'E' )
             && ( toupper( FileName[iLength - 1] ) == 'G' ) )
        {
          return true;
        }
      }
      return false;
    }



    ImageSet* FormatJPG::LoadSet( const GR::String& FileName )
    {

      int     iY,
        i;

      WORD    wWidth,
        wHeight;


      /* This struct contains the JPEG decompression parameters and pointers to
       * working space (which is allocated as needed by the JPEG library).
       */
      struct jpeg_decompress_struct cinfo;
      /* We use our private extension JPEG error handler.
       * Note that this struct must live as long as the main JPEG parameter
       * struct, to avoid dangling-pointer problems.
       */
      struct my_error_mgr jerr;
      /* More stuff */
      FILE* infile;		/* source file */
      JSAMPARRAY buffer;		/* Output row buffer */
      int row_stride;		/* physical row width in output buffer */

      /* In this example we want to open the input file before doing anything else,
       * so that the setjmp() error recovery below can assume the file is open.
       * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
       * requires it in order to read binary files.
       */

      if ( _wfopen_s( &infile, GR::Convert::ToUTF16( FileName ).c_str(), L"rb" ) )
      {
        return 0;
      }

      /* Step 1: allocate and initialize JPEG decompression object */

      /* We set up the normal JPEG error routines, then override error_exit. */
      cinfo.err = jpeg_std_error( &jerr.pub );
      jerr.pub.error_exit = my_error_exit;
      /* Establish the setjmp return context for my_error_exit to use. */
      if ( setjmp( jerr.setjmp_buffer ) )
      {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress( &cinfo );
        fclose( infile );
        return 0;
      }
      /* Now we can initialize the JPEG decompression object. */
      jpeg_create_decompress( &cinfo );

      /* Step 2: specify data source (eg, a file) */

      jpeg_stdio_src( &cinfo, infile );

      /* Step 3: read file parameters with jpeg_read_header() */

      (void)jpeg_read_header( &cinfo, TRUE );
      /* We can ignore the return value from jpeg_read_header since
       *   (a) suspension is not possible with the stdio data source, and
       *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
       * See libjpeg.doc for more info.
       */

       /* Step 4: set parameters for decompression */

       /* In this example, we don't need to change any of the defaults set by
        * jpeg_read_header(), so we do nothing here.
        */

        /* Step 5: Start decompressor */

      (void)jpeg_start_decompress( &cinfo );
      /* We can ignore the return value since suspension is not possible
       * with the stdio data source.
       */

       /* We may need to do some setup of our own at this point before reading
        * the data.  After jpeg_start_decompress() we have the correct scaled
        * output image dimensions available, as well as the output colormap
        * if we asked for color quantization.
        * In this example, we need to make an output work buffer of the right size.
        */
        /* JSAMPLEs per row in output buffer */
      row_stride = cinfo.output_width * cinfo.output_components;

      wWidth = cinfo.output_width;
      wHeight = cinfo.output_height;

      /* Make a one-row-high sample array that will go away when done with image */
      buffer = ( *cinfo.mem->alloc_sarray )
        ( (j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1 );


      // Image-Data erzeugen
      GR::Graphic::ImageData* pData = new GR::Graphic::ImageData();

      pData->CreateData( wWidth, wHeight, GR::Graphic::IF_R8G8B8 );

      BYTE* pImageData = (BYTE*)pData->Data();

      /* Step 6: while (scan lines remain to be read) */
      /*           jpeg_read_scanlines(...); */

      /* Here we use the library's state variable cinfo.output_scanline as the
       * loop counter, so that we don't have to keep track ourselves.
       */
      iY = 0;
      while ( cinfo.output_scanline < cinfo.output_height )
      {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        (void)jpeg_read_scanlines( &cinfo, buffer, 1 );
        /* Assume put_scanline_someplace wants a pointer and sample count. */

        pImageData = (BYTE*)pData->GetRowData( iY );

        if ( cinfo.out_color_space == JCS_GRAYSCALE )
        {
          for ( i = 0; i < row_stride; i++ )
          {
            *pImageData++ = buffer[0][i];
            *pImageData++ = buffer[0][i];
            *pImageData++ = buffer[0][i];
          }
        }
        else
        {
          for ( i = 0; i < row_stride / 3; i++ )
          {
            *pImageData++ = buffer[0][i * 3 + 2];
            *pImageData++ = buffer[0][i * 3 + 1];
            *pImageData++ = buffer[0][i * 3 + 0];
          }
        }

        iY++;
        //put_scanline_someplace(buffer[0], row_stride);
      }

      /* Step 7: Finish decompression */

      (void)jpeg_finish_decompress( &cinfo );
      /* We can ignore the return value since suspension is not possible
       * with the stdio data source.
       */

       /* Step 8: Release JPEG decompression object */

       /* This is an important step since it will release a good deal of memory. */
      jpeg_destroy_decompress( &cinfo );

      /* After finish_decompress, we can close the input file.
       * Here we postpone it until after no more JPEG errors are possible,
       * so as to simplify the setjmp error logic above.  (Actually, I don't
       * think that jpeg_destroy can do an error exit, but why assume anything...)
       */
      fclose( infile );

      /* At this point you may want to check to see whether any corrupt-data
       * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
       */

       /* And we're done! */
      ImageSet* pSet = new ImageSet();

      pSet->AddFrame( pData );

      return pSet;

    }



    bool FormatJPG::CanSave( GR::Graphic::ImageData* pData )
    {

      if ( pData->ImageFormat() == GR::Graphic::IF_R8G8B8 )
      {
        return true;
      }
      return false;

    }



    bool FormatJPG::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      if ( !CanSave( pData ) )
      {
        return false;
      }


      return false;
    }



    GR::String FormatJPG::GetDescription()
    {
      return "JPEG Image";
    }



    GR::String FormatJPG::GetFilterString()
    {
      return "*.jpg;*.jpeg";
    }

  }
}