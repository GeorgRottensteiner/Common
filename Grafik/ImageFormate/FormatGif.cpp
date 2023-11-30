#include <IO/FileStream.h>
#include <IO\FileStream.h>

#include <Misc/Misc.h>

#include <String/StringUtil.h>

#include <debug/debugclient.h>

#include "FormatGIF.h"

#include <Grafik/ContextDescriptor.h>



GR::Graphic::FormatGIF    globalGIFPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatGIF::FormatGIF()
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );
    }



    bool FormatGIF::IsFileOfType( const GR::String& FileName )
    {
      // OPEN FILE
      GR::IO::FileStream    ioIn;

      if ( !ioIn.Open( FileName ) )
      {
        return NULL;
      }

      // *1* READ HEADER (SIGNATURE + VERSION)
      char szSignature[6];				// First 6 bytes (GIF87a or GIF89a)

      ioIn.ReadBlock( szSignature, 6 );
      if ( memcmp( szSignature, "GIF", 2 ) != 0 )
      {
        return false;
      }
      return true;
    }



    int FormatGIF::GetNextByte( IIOStream& IOIn )
    {
      GR::u8  dummy;

      if ( IOIn.ReadBlock( &dummy, 1 ) )
      {
        return dummy;
      }
      return -1;
    }



    size_t FormatGIF::ReadBlobBlock( IIOStream& IOIn, GR::u8* pData )
    {
      int     count = 0;

      count = GetNextByte( IOIn );
      if ( count <= 0 )
      {
        return 0;
      }
      for ( int i = 0; i < count; ++i )
      {
        *pData++ = GetNextByte( IOIn );
      }
      return (size_t)count;
    }



    short int FormatGIF::DecodeImage( IIOStream& IOIn, char* pTarget, const long opacity )
    {
      const int MAX_STACK_SIZE = 4096;
      const int NULL_CODE = -1;

      int               bits = 0,
                        offset = 0,
                        pass = 0;

      long              code,
                        in_code,
                        y;

      long              x;

      GR::u8*           pTempData = NULL;

      unsigned long     datum = 0;

      size_t            count = 0;

      unsigned char     first = 0,
                        index;



      // Initialize GIF data stream decoder
      GR::u8 dataSize = (GR::u8)GetNextByte( IOIn );
      if ( dataSize > 8 )
      {
        return false;
      }

      // Allocate decoder tables
      ByteBuffer  packet( 256 );
      ByteBuffer  prefix( MAX_STACK_SIZE * sizeof( GR::i16 ) );
      ByteBuffer  suffix( MAX_STACK_SIZE );
      ByteBuffer  pixel_stack( MAX_STACK_SIZE + 1 );

      long clear            = 1 << dataSize;
      long endOfInformation = clear + 1;
      long available        = clear + 2;
      long old_code         = NULL_CODE;
      int code_size         = dataSize + 1;
      long code_mask        = ( 1 << code_size ) - 1;
      for ( code = 0; code < clear; code++ )
      {
        ( (GR::i16*)prefix.Data() )[code] = 0;
        suffix[code] = (GR::u8)code;
      }

      // Decode GIF pixel stream
      GR::u8* pStackTop = (GR::u8*)pixel_stack.Data();

      char* pTargetOrig = pTarget;

      for ( y = 0; y < (long)nheight; y++ )
      {
        pTarget = pTargetOrig + m_FrameXOffset + ( offset + m_FrameYOffset ) * m_FullWidth;

        for ( x = 0; x < (long)nwidth; )
        {
          if ( pStackTop == (GR::u8*)pixel_stack.Data() )
          {
            if ( bits < code_size )
            {
              // Load bytes until there is enough bits for a code
              if ( count == 0 )
              {
                // Read a new data block
                count = ReadBlobBlock( IOIn, (GR::u8*)packet.Data() );
                if ( count == 0 )
                {
                  break;
                }
                pTempData = (GR::u8*)packet.Data();
              }
              datum += (unsigned long)( *pTempData ) << bits;
              bits  += 8;
              ++pTempData;
              --count;
              continue;
            }

            // Get the next code
            code    = (long)( datum & code_mask );
            datum   >>= code_size;
            bits    -= code_size;

            // Interpret the code
            if ( ( code > available )
            ||   ( code == endOfInformation ) )
            {
              break;
            }
            if ( code == clear )
            {
              // Reset decoder
              code_size = dataSize + 1;
              code_mask = ( 1 << code_size ) - 1;
              available = clear + 2;
              old_code  = NULL_CODE;
              continue;
            }
            if ( old_code == NULL_CODE )
            {
              *pStackTop++  = suffix[code];
              old_code      = code;
              first         = (unsigned char)code;
              continue;
            }
            in_code = code;
            if ( code >= available )
            {
              *pStackTop++  = first;
              code          = old_code;
            }
            while ( code >= clear )
            {
              *pStackTop++  = suffix[code];
              code          = ( (GR::i16*)prefix.Data() )[code];
            }
            first = suffix[code];

            // Add a new string to the string table
            if ( available >= MAX_STACK_SIZE )
            {
              break;
            }
            *pStackTop++      = first;
            ( (GR::i16*)prefix.Data() )[available] = (short)old_code;
            suffix[available] = first;
            ++available;
            if ( ( ( available & code_mask ) == 0 )
            &&   ( available < MAX_STACK_SIZE ) )
            {
              ++code_size;
              code_mask += available;
            }
            old_code = in_code;
          }

          // Pop a pixel off the pixel stack
          --pStackTop;
          index = *pStackTop;

          if ( ( ( m_TempTransparentIndex != -1 )
          &&     ( index != m_TempTransparentIndex ) )
          ||   ( m_TempTransparentIndex == -1 ) )
          {
            *pTarget++ = index;
          }
          else
          {
            pTarget++;
          }

          ++x;
        }
        if ( !m_Interlaced )
        {
          ++offset;
        }
        else
        {
          switch ( m_InterlacedPass )
          {
            case 0:
              offset += 8;
              if ( offset >= nheight )
              {
                offset = 4 - 8;
                ++m_InterlacedPass;
              }
              break;
            case 1:
              offset += 8;
              if ( offset >= nheight )
              {
                offset = 2 - 4;
                ++m_InterlacedPass;
              }
              break;
            case 2:
              offset += 4;
              if ( offset >= nheight )
              {
                offset = 1 - 2;
                ++m_InterlacedPass;
              }
              break;
            case 3:
              offset += 2;
              break;
          }
        }

        pTarget = pTargetOrig + offset * m_FullWidth;
        if ( x < (long)nwidth )
        {
          break;
        }
      }
      return true;
    }



    ImageSet* FormatGIF::LoadSet( const GR::String& FileName )
    {
      m_TempTransparentIndex = -1;
      m_InterlacedPass = 0;

      GR::IO::FileStream    ioIn;

	    int       n;

	    // Global GIF variables:
	    int       GlobalBPP = 0;						// Bits per Pixel.
      GR::u8    GlobalColorMap[768];				// Global colormap (allocate)

      #pragma pack(1)
      // GRAPHIC CONTROL EXTENSION
	    struct GIFGCEtag
      {
		    unsigned char   BlockSize;		// Block Size: 4 bytes
		    unsigned char   PackedFields;	// Packed Fields. Bits detail:
										                  //    0: Transparent Color Flag
										                  //    1: User Input Flag
										                  //  2-4: Disposal Method
		    unsigned short Delay;			    // Delay Time (1/100 seconds)
		    unsigned char Transparent;		// Transparent Color Index
	    } gifgce;
      #pragma pack()

	    int GraphicExtensionFound = 0;

	    // OPEN FILE
      if ( !ioIn.Open( FileName ) )
      {
        return NULL;
      }

	    // *1* READ HEADER (SIGNATURE + VERSION)
	    char szSignature[6];				// First 6 bytes (GIF87a or GIF89a)

      ioIn.ReadBlock( szSignature, 6 );
	    if ( memcmp( szSignature, "GIF", 2 ) != 0 )
	    {
        return NULL;
      }

      ImageSet*         pSet = new ImageSet();

      tImageSetFrame*   pCurrentFrame = NULL;


	    // *2* READ LOGICAL SCREEN DESCRIPTOR
    #pragma pack(1)
	    struct GIFLSDtag
      {
		    unsigned short  ScreenWidth;		// Logical Screen Width
		    unsigned short  ScreenHeight;	  // Logical Screen Height
		    unsigned char   PackedFields;		// Packed Fields. Bits detail:
										                    //  0-2: Size of Global Color Table
										                    //    3: Sort Flag
										                    //  4-6: Color Resolution
										                    //    7: Global Color Table Flag
		    unsigned char Background;		    // Background Color Index
		    unsigned char PixelAspectRatio;	// Pixel Aspect Ratio
	    } giflsd;
    #pragma pack()

      ioIn.ReadBlock( &giflsd, sizeof( giflsd ) );

	    GlobalBPP = ( giflsd.PackedFields & 0x07 ) + 1;

	    // fill some animation data:
	    int FrameWidth  = giflsd.ScreenWidth;
	    int FrameHeight = giflsd.ScreenHeight;
	    int nLoops      = 0;

      m_FullWidth     = FrameWidth;
      m_FullHeight    = FrameHeight;

      GR::tRect       previousFrameRect;

	    // *3* READ/GENERATE GLOBAL COLOR MAP
      if ( giflsd.PackedFields & 0x80 )
      {
        // File has global color map?
        for ( n = 0; n < 1 << GlobalBPP; n++ )
        {
          GlobalColorMap[n * 3 + 0] = ioIn.ReadU8();
          GlobalColorMap[n * 3 + 1] = ioIn.ReadU8();
          GlobalColorMap[n * 3 + 2] = ioIn.ReadU8();
        }
      }
      else
      {
        // GIF standard says to provide an internal default Palette:
        for ( n = 0; n < 256; n++ )
        {
          GlobalColorMap[n * 3 + 0] = GlobalColorMap[n * 3 + 1] = GlobalColorMap[n * 3 + 2] = n;
        }
      }

	    // *4* NOW WE HAVE 3 POSSIBILITIES:
	    //  4a) Get and Extension Block (Blocks with additional information)
	    //  4b) Get an Image Separator (Introductor to an image)
	    //  4c) Get the trailer Char (End of GIF File)
      int previousDisposalMethod = 0;
	    do
	    {
		    int charGot = ioIn.ReadU8();

		    if ( charGot == 0x21 )		// *A* EXTENSION BLOCK
		    {
			    switch ( ioIn.ReadU8() )
			    {
			      case 0xF9:
              // Graphic Control Extension
              ioIn.ReadBlock( &gifgce, sizeof( gifgce ) );
				      ++GraphicExtensionFound;

              // Block Terminator (always 0)
              ioIn.ReadU8();
				      break;
			      case 0xFF:			
              // Application Extension: Ignored
              {
                int nBlockLength = ioIn.ReadU8();

                ioIn.SetPosition( nBlockLength, IIOStream::PT_CURRENT );

                GR::u8    lengthSubBlock = -1;
                do
                {
                  lengthSubBlock = ioIn.ReadU8();
                  ioIn.SetPosition( lengthSubBlock, IIOStream::PT_CURRENT );
                }
                while ( lengthSubBlock != 0 );

                /*
                byte   1       : 33 (hex 0x21) GIF Extension code
                byte   2       : 255 (hex 0xFF) Application Extension Label
                byte   3       : 11 (hex (0x0B) Length of Application Block
                                (eleven bytes of data to follow)
                bytes  4 to 11 : "NETSCAPE"
                bytes 12 to 14 : "2.0"

                repeat until length = 0!
                  byte  15       : 3 (hex 0x03) Length of Data Sub-Block
                                  (three bytes of data to follow)
                  byte  16       : 1 (hex 0x01)
                  bytes 17 to 18 : 0 to 65535, an unsigned integer in
                                  lo-hi byte format. This indicate the
                                  number of iterations the loop should
                                  be executed.
                //bytes 19       : 0 (hex 0x00) a Data Sub-block Terminator.
                */
              }
              break;
			      case 0xFE:			// Comment Extension: Ignored
			      case 0x01:			// PlainText Extension: Ignored
			      default:			// Unknown Extension: Ignored
				      // read (and ignore) data sub-blocks
              while ( int nBlockLength = ioIn.ReadU8() )
              {
                for ( n = 0; n < nBlockLength; n++ )
                {
                  ioIn.ReadU8();
                }
              }
				      break;
			    }
		    }
		    else if ( charGot == 0x2c )
        {
          // *B* IMAGE (0x2c Image Separator)

			    // Create a new Image Object:
          auto pImageData  = new GR::Graphic::ImageData();
          pCurrentFrame = pSet->AddFrame( pImageData );

			    // Read Image Descriptor
          tGIFImageDescriptor   gifID;

          ioIn.ReadBlock( &gifID, sizeof( gifID ) );

          int LocalColorMap         = ( gifID.PackedFields & 0x80 ) ? 1 : 0;
          int iLocalColorTableSize  = 1 << ( ( gifID.PackedFields & 0x07 ) + 1 );

          pImageData->CreateData( FrameWidth, FrameHeight, GR::Graphic::IF_PALETTED );

          if ( ( gifID.PackedFields & 0x40 ) == 0x40 )
          {
            m_Interlaced = true;
          }
          else
          {
            m_Interlaced = false;
          }

			    // Fill NextImage Data
			    pCurrentFrame->XOffset = gifID.XPos;
			    pCurrentFrame->YOffset = gifID.YPos;

          m_FrameXOffset = gifID.XPos;
          m_FrameYOffset = gifID.YPos;

          m_TempTransparentIndex = -1;

			    if ( GraphicExtensionFound )
			    {
            // bei GIF-Anis Transparenz berücksichtigen
            m_TempTransparentIndex = ( gifgce.PackedFields & 0x01 ) ? gifgce.Transparent : -1;

            pCurrentFrame->DelayMS = gifgce.Delay * 10;

            pImageData->TransparentColorUsed( true );
            pImageData->TransparentColor( m_TempTransparentIndex );

            if ( ( previousDisposalMethod == 0 )
            &&   ( pImageData->TransparentColorUsed() ) )
            {
              memset( pImageData->Data(), m_TempTransparentIndex, pImageData->DataSize() );
            }

            // für optimierte GIF-Anis den alten Frame einsetzen
            int   disposalMethod = ( ( gifgce.PackedFields & 0x1c ) >> 2 );

            switch ( previousDisposalMethod )
            {
              case 0:
                //dh::Log( "no disposal required" );
                break;
              case 1:
                //dh::Log( "using last frame" );
                if ( pSet->Frames.size() >= 2 )
                {
                  tImageSetFrame*   pPrevFrame = pSet->Frames[pSet->Frames.size() - 2];

                  memcpy( pCurrentFrame->Layers[0]->pImageData->Data(),
                          pPrevFrame->Layers[0]->pImageData->Data(),
                          pCurrentFrame->Layers[0]->pImageData->DataSize() );
                }
                break;
              case 2:
                //dh::Log( "restore to background color" );
                if ( pSet->Frames.size() >= 2 )
                {
                  tImageSetFrame* pPrevFrame = pSet->Frames[pSet->Frames.size() - 2];

                  memcpy( pCurrentFrame->Layers[0]->pImageData->Data(),
                    pPrevFrame->Layers[0]->pImageData->Data(),
                    pCurrentFrame->Layers[0]->pImageData->DataSize() );

                  for ( int y = 0; y < previousFrameRect.height(); ++y )
                  {
                    GR::u8* pData = (GR::u8*)pCurrentFrame->Layers[0]->pImageData->GetRowColumnData( previousFrameRect.Left, previousFrameRect.Top + y );
                    memset( pData, m_TempTransparentIndex, previousFrameRect.width() );
                  }
                }
                break;
              case 3:
                //dh::Log( "using 2nd last frame" );
                if ( pSet->Frames.size() >= 3 )
                {
                  tImageSetFrame*   pPrevFrame = pSet->Frames[pSet->Frames.size() - 3];

                  memcpy( pCurrentFrame->Layers[0]->pImageData->Data(),
                          pPrevFrame->Layers[0]->pImageData->Data(),
                          pCurrentFrame->Layers[0]->pImageData->DataSize() );
                }
                break;
              default:
                dh::Log( "GIF unsupported disposal mode %d", disposalMethod );
                break;
            }
            /*
            0 -   No disposal specified. The decoder is
                  not required to take any action.
            1 -   Do not dispose. The graphic is to be left
                  in place.
            2 -   Restore to background color. The area used by the
                  graphic must be restored to the background color.
            3 -   Restore to previous. The decoder is required to
                  restore the area overwritten by the graphic with
                  what was there prior to rendering the graphic.
                  */

            previousDisposalMethod = disposalMethod;
			    }

          // Read Color Map (if descriptor says so)
			    if ( LocalColorMap )		
          {
            memset( pCurrentFrame->Layers[0]->pImageData->Palette().Data(), 0, pCurrentFrame->Layers[0]->pImageData->Palette().Entries() * 3 );
            for ( int i = 0; i < iLocalColorTableSize; ++i )
            {
              int   iR = ioIn.ReadU8();
              int   iG = ioIn.ReadU8();
              int   iB = ioIn.ReadU8();
              pCurrentFrame->Layers[0]->pImageData->Palette().SetColor( i, iR, iG, iB );
            }
          }
			    else					
          {
            // Otherwise copy Global
            memset( pCurrentFrame->Layers[0]->pImageData->Palette().Data(), 0, pCurrentFrame->Layers[0]->pImageData->Palette().Entries() * 3 );
				    memcpy( pCurrentFrame->Layers[0]->pImageData->Palette().Data(), GlobalColorMap,
					          3 * ( 1 << GlobalBPP ) );
          }

			    // Call LZW/GIF decompressor
          nwidth  = gifID.Width;
          nheight = gifID.Height;

          n = DecodeImage( ioIn, (char*)pCurrentFrame->Layers[0]->pImageData->Data(), m_TempTransparentIndex );
          previousFrameRect.set( m_FrameXOffset, m_FrameYOffset, nwidth, nheight );

			    if ( n )
          {
				    //AddImage(NextImage);
          }
			    else
			    {
            dh::Log( "GIF File Corrupt\n");
				    //delete NextImage;
				    //ERRORMSG("GIF File Corrupt");
			    }

			    // Some cleanup
			    GraphicExtensionFound = 0;
		    }
		    else if ( charGot == 0x3b )
        {
          // *C* TRAILER: End of GIF Info
          // Ok. Standard End.
			    break;
		    }
        if ( ioIn.GetPosition() == ioIn.GetSize() )
        {
          break;
        }
	    }
      while ( ioIn.IsGood() );

      return pSet;
    }



    bool FormatGIF::CanSave( ImageSet* pSet )
    {
      if ( pSet == NULL )
      {
        return false;
      }
      if ( pSet->Frames.empty() )
      {
        return false;
      }
      if ( pSet->Frames[0]->Layers.empty() )
      {
        return false;
      }
      if ( pSet->Frames[0]->Layers[0]->pImageData == NULL )
      {
        return false;
      }
      if ( pSet->Frames[0]->Layers[0]->pImageData->ImageFormat() == GR::Graphic::IF_PALETTED )
      {
        return true;
      }
      return false;
    }



    bool FormatGIF::CanSave( GR::Graphic::ImageData* pData )
    {
      if ( pData == NULL )
      {
        return false;
      }
      if ( pData->ImageFormat() == GR::Graphic::IF_PALETTED )
      {
        return true;
      }
      return false;
    }




    int FormatGIF::WriteImageDescriptor( GR::IO::FileStream& MyFile, ImageDescriptor *id)
    {
      GR::u8  tmp;

      MyFile.WriteU8( id->Separator );
      MyFile.WriteU16( id->LeftPosition );
      MyFile.WriteU16( id->TopPosition );
      MyFile.WriteU16( id->Width );
      MyFile.WriteU16( id->Height );

      tmp = ( id->LocalColorTableFlag << 7 )
          | ( id->InterlaceFlag << 6 )
          | ( id->SortFlag << 5 )
          | ( id->Reserved << 3 )
          | id->LocalColorTableSize;
      MyFile.WriteU8( tmp );

      return 0;
    }




    int FormatGIF::BitsNeeded( GR::u16 n )
    {
      int   ret = 1;

      if ( !n-- )
      {
        return 0;
      }

      while ( n >>= 1 )
      {
        ++ret;
      }

      return ret;
    }



    /*-------------------------------------------------------------------------
     *
     *  NAME:           InputBYTE()
     *
     *  DESCRIPTION:    Get next pixel from image. Called by the
     *                  LZW_Compress()-function
     *
     *  PARAMETERS:     None
     *
     *  RETURNS:        Next pixelvalue, or -1 if no more pixels
     *
     */
    int FormatGIF::InputBYTE()
    {
      if ( m_FrameYOffset >= ImageHeight )
      {
        return -1;
      }

      unsigned char ret;
      memcpy( &ret, (GR::u8*)m_pSavingData->Data() + m_FrameXOffset + m_FrameYOffset * m_pSavingData->Width(), 1 );

      if ( ++m_FrameXOffset >= ImageWidth )
      {
        m_FrameXOffset = 0;
        ++m_FrameYOffset;
      }

      return ret;
    }



    void FormatGIF::InitBitFile()
    {
      Buffer[Index = 0] = 0;
      BitsLeft          = 8;
    }



    int FormatGIF::ResetOutBitFile()
    {
      GR::u8      numBYTEs = 0;

      // Find out how much is in the buffer
      numBYTEs = Index + (BitsLeft == 8 ? 0 : 1);

      // Write whatever is in the buffer to the file
      if ( numBYTEs )
      {
        m_pSavingFile->WriteU8( numBYTEs );

        GR::u32 bytesWritten = m_pSavingFile->WriteBlock( Buffer, numBYTEs );
        if ( bytesWritten < numBYTEs )
        {
          return -1;
        }
        Buffer[Index = 0] = 0;
        BitsLeft = 8;
      }

      return 0;
    }



    int FormatGIF::WriteBits( int bits, int numbits )
    {
      int     bitswritten = 0;
      GR::u8  numBYTEs = 255;


      do
      {
        // If the buffer is full, write it
        if ( ( ( Index == 254 )
        &&     ( !BitsLeft ) )
        ||   ( Index > 254 ) )
        {
          m_pSavingFile->WriteU8( numBYTEs );

          m_pSavingFile->WriteBlock( Buffer, numBYTEs );

          Buffer[Index = 0] = 0;
          BitsLeft          = 8;
        }

        // Now take care of the two specialcases
        if ( numbits <= BitsLeft )
        {
          Buffer[Index] |= ( bits & ( ( 1 << numbits ) - 1 ) ) << ( 8 - BitsLeft );
          bitswritten   += numbits;
          BitsLeft      -= numbits;
          numbits       = 0;
        }
        else
        {
          Buffer[Index] |= ( bits & ( ( 1 << BitsLeft ) - 1 ) ) << ( 8 - BitsLeft );
          bitswritten   += BitsLeft;
          bits          >>= BitsLeft;
          numbits       -= BitsLeft;

          Buffer[++Index] = 0;
          BitsLeft        = 8;
        }
      }
      while ( numbits );

      return bitswritten;
    }



    GR::u16 FormatGIF::AddCharString( GR::u16 index, GR::u8 b )
    {
      GR::u16 hshidx;


      // Check if there is more room
      if ( NumStrings >= ( 1 << 12 ) )
      {
        return 0xFFFF;
      }

      // Search the string table until a free position is found
      #define HASH( index, lastBYTE )   ( ( ( lastBYTE << 8 ) ^ index ) % 9973 )

      hshidx = HASH( index, b );
      while ( StringHash[hshidx] != 0xFFFF )
      {
        hshidx = ( hshidx + 2039 ) % 9973;
      }

      // Insert new string
      StringHash[hshidx]        = NumStrings;
      StringBuffer[NumStrings]  = b;
      StringNext[NumStrings]    = ( index != 0xFFFF) ? index : 0xffff;

      return NumStrings++;
    }



    GR::u16 FormatGIF::FindCharString( GR::u16 index, GR::u8 b )
    {
      GR::u16 hshidx,
              nxtidx;

      // Check if index is 0xFFFF. In that case we need only return b, since all one-character strings has their BYTEvalue as their index
      if ( index == 0xFFFF )
      {
        return b;
      }

      // Search the string table until the string is found, or we find HASH_FREE. In that case the string does not exist.
      hshidx = HASH( index, b );
      while ( ( nxtidx = StringHash[hshidx] ) != 0xFFFF )
      {
        if ( ( StringNext[nxtidx] == index )
        &&   ( StringBuffer[nxtidx] == b ) )
        {
          return nxtidx;
        }
        hshidx = ( hshidx + 2039 ) % 9973;
      }

      // No match is found
      return 0xFFFF;
    }



    void FormatGIF::ClearStrtab( int codesize )
    {
      int       q,
                w;
      GR::u16*  wp;


      // No strings currently in the table
      NumStrings = 0;

      // Mark entire hashtable as free
      wp = StringHash;
      for ( q = 0; q < 9973; q++ )
      {
        *wp++ = 0xffff;
      }

      // Insert 2**codesize one-character strings, and reserved codes
      w = ( 1 << codesize ) + 2;
      for ( q = 0; q < w; q++ )
      {
        AddCharString( 0xFFFF, q );
      }
    }



    int FormatGIF::LZW_Compress( int codesize )
    {
      int     c;
      GR::u16 index;
      GR::u16 prefix = 0xFFFF;


      // Set up the given outfile
      InitBitFile();

      // Set up variables and tables
      int clearcode = 1 << codesize;
      int endofinfo = clearcode + 1;
      int numbits   = codesize + 1;
      int limit     = ( 1 << numbits ) - 1;

      ClearStrtab( codesize );

      // First send a code telling the unpacker to clear the stringtable.
      WriteBits( clearcode, numbits );

      // Pack image
      while ( ( c = InputBYTE() ) != -1 )
      {
        // Now perform the packing
        // Check if the prefix + the new character is a string that exists in the table
        if ( ( index = FindCharString( prefix, c ) ) != 0xFFFF )
        {
          // The string exists in the table. Make this string the new prefix
          prefix = index;
        }
        else
        {
          // The string does not exist in the table
          // First write code of the old prefix to the file
          WriteBits( prefix, numbits );

          // Add the new string (the prefix + the new character) to the stringtable.
          if ( AddCharString( prefix, c ) > limit )
          {
            if ( ++numbits > 12 )
            {
              WriteBits( clearcode, numbits - 1 );
              ClearStrtab( codesize );
              numbits = codesize + 1;
            }
            limit = ( 1 << numbits ) - 1;
          }

          // Set prefix to a string containing only the character read. Since all possible one-character strings exists
          // in the table, there's no need to check if it is found
          prefix = c;
        }
      }

      // End of info is reached. Write last prefix.
      if ( prefix != 0xFFFF )
      {
        WriteBits( prefix, numbits );
      }

      WriteBits( endofinfo, numbits );

      ResetOutBitFile();

      return 0;
    }



    bool FormatGIF::Save( const GR::String& FileName, GR::Graphic::ImageData* pData, GR::Graphic::ImageData* pMask )
    {
      if ( !CanSave( pData ) )
      {
        return false;
      }

      typedef struct
      {
        GR::u16         LocalScreenWidth,
                        LocalScreenHeight;
        GR::u8          GlobalColorTableSize : 3,
                        SortFlag             : 1,
                        ColorResolution      : 3,
                        GlobalColorTableFlag : 1;
        GR::u8          BackgroundColorIndex;
        GR::u8          PixelAspectRatio;
      } ScreenDescriptor;


      int                   q,
                            tabsize,
                            codesize,
                            errcode,
                            iWidth,
                            iHeight;

      ImageDescriptor       ID;

      GR::u8*               bp = NULL;
      GR::u8                tmp,
                            bDummy;

      ScreenDescriptor      SD;

      long                  maxcolor;

      GR::IO::FileStream    MyFile;

      BYTE                  *p;

      m_pSavingData = pData;
      m_pSavingFile = &MyFile;

      iWidth  = pData->Width();
      iHeight = pData->Height();

      // Initiate variables for new GIF-file
      NumColors         = 256;                // Anzahl Farben //numcolors ? ( 1 << BitsNeeded( numcolors ) ) : 0;
      ScreenHeight      = iWidth;          // Bildschirm-Breite
      ScreenWidth       = iHeight;          // Bildschirm-Höhe

      // Create file specified
      if ( !MyFile.Open( FileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      // Write GIF signature
      if ( !MyFile.WriteBlock( "GIF89a", 6 ) )
      {
        MyFile.Close();
        return false;
      }

      // Initiate and write screen descriptor
      SD.LocalScreenWidth   = iWidth;
      SD.LocalScreenHeight  = iHeight;
      if ( NumColors )
      {
        SD.GlobalColorTableSize = BitsNeeded( NumColors ) - 1;
        SD.GlobalColorTableFlag = 1;
      }
      else
      {
        SD.GlobalColorTableSize = 0;
        SD.GlobalColorTableFlag = 0;
      }
      SD.SortFlag             = 0;
      SD.ColorResolution      = 7;   // Bits pro Pixel - 1?
      SD.BackgroundColorIndex = 0;
      SD.PixelAspectRatio     = 0;

      MyFile.WriteU16( SD.LocalScreenWidth );
      MyFile.WriteU16( SD.LocalScreenHeight );

      tmp = ( SD.GlobalColorTableFlag << 7 )
          | ( SD.ColorResolution << 4 )
          | ( SD.SortFlag << 3 )
          | ( SD.GlobalColorTableSize );
      MyFile.WriteU8( tmp );
      MyFile.WriteU8( SD.BackgroundColorIndex );
      MyFile.WriteU8( SD.PixelAspectRatio );

      // Allocate color table
      if ( NumColors )
      {
        tabsize = NumColors * 3;
        bp = ColorTable;
        for ( q = 0; q < tabsize; q++ )
        {
          *bp++ = 0;
        }
      }


      maxcolor = ( 1L << 8 ) - 1L;
      for ( q = 0; q < 256; q++ )
      {
        // GEORG 10.7.2002
        // Paletten haben jetzt volle 256-Weite
        p = ColorTable + q * 3;
        *p++ = (GR::u8) (( ( pData->Palette().Red( q ) ) * 255L ) / maxcolor );
        *p++ = (GR::u8) (( ( pData->Palette().Green( q ) ) * 255L ) / maxcolor );
        *p++ = (GR::u8) (( ( pData->Palette().Blue( q ) ) * 255L ) / maxcolor );
      }

      if ( iWidth < 0 )
      {
        iWidth = ScreenWidth;
        //iX1 = 0;
      }
      if ( iHeight < 0 )
      {
        iHeight = ScreenHeight;
        //iY1 = 0;
      }

      // Write global colortable if any
      if ( NumColors )
      {
        if ( !MyFile.WriteBlock( ColorTable, NumColors * 3 ) )
        {
          return false;
        }
      }


      if ( pData->TransparentColorUsed() )
      {
        // graphic control extension
        MyFile.WriteU8( 0x21 );

        MyFile.WriteU8( 0xf9 );
        // block size
        MyFile.WriteU8( 4 );
        // Packed field
        if ( pData->TransparentColorUsed() )
        {
          bDummy = 5;
        }
        else
        {
          bDummy = 4;
        }
        MyFile.WriteU8( bDummy );
        // delay time
        bDummy = 0;
        MyFile.WriteU8( 0 );
        MyFile.WriteU8( 0 );
        // transparent index
        if ( pData->TransparentColorUsed() )
        {
          bDummy = (BYTE)pData->TransparentColor();
        }
        MyFile.WriteU8( bDummy );
        // block terminator
        bDummy = 0;
        MyFile.WriteU8( 0 );
      }

      // Initiate and write image descriptor
      ImageLeft               = 0;
      ImageTop                = 0;
      ID.Separator            = ',';
      ID.LeftPosition         = 0;
      ID.TopPosition          = 0;
      ID.Width                = ImageWidth = iWidth;
      ID.Height               = ImageHeight = iHeight;
      ID.LocalColorTableSize  = 0;
      ID.Reserved             = 0;
      ID.SortFlag             = 0;
      ID.InterlaceFlag        = 0;
      ID.LocalColorTableFlag  = 0;

      WriteImageDescriptor( MyFile, &ID );

      // Write code size
      codesize = BitsNeeded( NumColors );
      if ( codesize == 1 )
      {
        ++codesize;
      }
      MyFile.WriteU8( codesize );

      // Perform compression
      m_FrameXOffset = m_FrameYOffset = 0;
      if ( ( errcode = LZW_Compress( codesize ) ) != 0 )
      {
        return FALSE;
      }

      // Write terminating 0-BYTE
      MyFile.WriteU8( 0 );

      // Initiate and write ending image descriptor
      ID.Separator = ';';
      WriteImageDescriptor( MyFile, &ID );

      // Close file
      MyFile.Close();

      return true;
    }



    bool FormatGIF::Save( const GR::String& FileName, ImageSet* pSet )
    {
      if ( !CanSave( pSet ) )
      {
        return false;
      }

    typedef struct
    {
      GR::u16         LocalScreenWidth,
                      LocalScreenHeight;
      GR::u8          GlobalColorTableSize : 3,
                      SortFlag             : 1,
                      ColorResolution      : 3,
                      GlobalColorTableFlag : 1;
      GR::u8          BackgroundColorIndex;
      GR::u8          PixelAspectRatio;
    } ScreenDescriptor;


      int                   q,
                            tabsize,
                            codesize,
                            errcode,
                            iWidth,
                            iHeight;

      ImageDescriptor       ID;

      BYTE                  *bp,
                            tmp,
                            bDummy;

      ScreenDescriptor      SD;

      long                  maxcolor;

      GR::IO::FileStream             MyFile;

      BYTE                  *p;

      m_pSavingFile = &MyFile;

      iWidth  = pSet->Image( 0, 0 )->Width();
      iHeight = pSet->Image( 0, 0 )->Height();

      // Initiate variables for new GIF-file
      NumColors = 256;                // Anzahl Farben //numcolors ? ( 1 << BitsNeeded( numcolors ) ) : 0;
      ScreenHeight = iWidth;          // Bildschirm-Breite
      ScreenWidth = iHeight;          // Bildschirm-Höhe

      // Create file specified
      if ( !MyFile.Open( FileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      // Write GIF signature
      //if (fwrite( buf, sizeof(BYTE), len, OutFile) < len)
      if ( !MyFile.WriteBlock( "GIF89a", 6 ) )
      {
        MyFile.Close();
        return false;
      }

      // Initiate and write screen descriptor
      SD.LocalScreenWidth   = iWidth;
      SD.LocalScreenHeight  = iHeight;
      if ( NumColors )
      {
        SD.GlobalColorTableSize = BitsNeeded( NumColors ) - 1;
        SD.GlobalColorTableFlag = 1;
      }
      else
      {
        SD.GlobalColorTableSize = 0;
        SD.GlobalColorTableFlag = 0;
      }
      SD.SortFlag             = 0;
      SD.ColorResolution      = 7;   // Bits pro Pixel - 1
      SD.BackgroundColorIndex = 0;
      SD.PixelAspectRatio     = 0;

      MyFile.WriteU16( SD.LocalScreenWidth );
      MyFile.WriteU16( SD.LocalScreenHeight );

      tmp = ( SD.GlobalColorTableFlag << 7 )
          | ( SD.ColorResolution << 4 )
          | ( SD.SortFlag << 3 )
          | ( SD.GlobalColorTableSize );
      MyFile.WriteU8( tmp );
      MyFile.WriteU8( SD.BackgroundColorIndex );
      MyFile.WriteU8( SD.PixelAspectRatio );

      // Allocate color table
      if ( NumColors )
      {
        tabsize = NumColors * 3;
        bp = ColorTable;
        for ( q = 0; q < tabsize; q++ )
        {
          *bp++ = 0;
        }
      }

      GR::Graphic::Palette*   pPal = pSet->Palette( 0, 0 );
      maxcolor = ( 1L << 8 ) - 1L;
      for ( q = 0; q < 256; q++ )
      {
        // GEORG 10.7.2002
        // Paletten haben jetzt volle 256-Weite
        p = ColorTable + q * 3;
        *p++ = (GR::u8) ( ( ( pPal->Red( q ) ) * 255L ) / maxcolor );
        *p++ = (GR::u8) ( ( ( pPal->Green( q ) ) * 255L ) / maxcolor );
        *p++ = (GR::u8) ( ( ( pPal->Blue( q ) ) * 255L ) / maxcolor );
      }

      if ( iWidth < 0 )
      {
        iWidth = ScreenWidth;
        //iX1 = 0;
      }
      if ( iHeight < 0 )
      {
        iHeight = ScreenHeight;
        //iY1 = 0;
      }


      // Write global colortable if any
      if ( NumColors )
      {
        if ( !MyFile.WriteBlock( ColorTable, NumColors * 3 ) )
        {
          return false;
        }
      }

      // Loop-Block
      GR::u8    bPuffer[19];

      bPuffer[0]  = 0x21;       // GIF Extension code
      bPuffer[1]  = 0xff;       // Application Extension Label
      bPuffer[2]  = 11;         // Length of Application Block (eleven bytes of data to follow)
      bPuffer[3]  = 'N';
      bPuffer[4]  = 'E';
      bPuffer[5]  = 'T';
      bPuffer[6]  = 'S';
      bPuffer[7]  = 'C';
      bPuffer[8]  = 'A';
      bPuffer[9]  = 'P';
      bPuffer[10] = 'E';
      bPuffer[11] = '2';
      bPuffer[12] = '.';
      bPuffer[13] = '0';
      bPuffer[14] = 0x03;       // Length of Data Sub-Block (three bytes of data to follow)
      bPuffer[15] = 1;          // ?
      bPuffer[16] = 0;
      bPuffer[17] = 0;          // bytes 17 to 18 : 0 to 65535, an unsigned integer in lo-hi byte format. This indicate the
                                // number of iterations the loop should be executed.
      bPuffer[18] = 0;          // a Data Sub-block Terminator.

      MyFile.WriteBlock( bPuffer, 19 );


      for ( size_t iFrames = 0; iFrames < pSet->FrameCount(); ++iFrames )
      {
        m_pSavingData = pSet->Image( iFrames, 0 );

        //if ( m_pSavingData->TransparentColorUsed() )
        {
          // graphic control extension
          MyFile.WriteU8( 0x21 );

          MyFile.WriteU8( 0xf9 );
          // block size
          MyFile.WriteU8( 4 );
          // Packed field
          if ( m_pSavingData->TransparentColorUsed() )
          {
            bDummy = 5;
          }
          else
          {
            bDummy = 4;
          }
          MyFile.WriteU8( bDummy + ( 1 << 2 ) );
          // delay time
          bDummy = 0;
          MyFile.WriteU16( (GR::u16)pSet->Frame( iFrames )->DelayMS / 10 );
          // transparent index
          if ( m_pSavingData->TransparentColorUsed() )
          {
            bDummy = (GR::u8)m_pSavingData->TransparentColor();
          }
          MyFile.WriteU8( bDummy );
          // block terminator
          bDummy = 0;
          MyFile.WriteU8( 0 );
        }

        // Initiate and write image descriptor
        ImageLeft               = 0;
        ImageTop                = 0;
        ID.Separator            = ',';
        ID.LeftPosition         = 0;
        ID.TopPosition          = 0;
        ID.Width  = ImageWidth = iWidth;
        ID.Height = ImageHeight = iHeight;
        ID.LocalColorTableSize  = 7;
        ID.Reserved             = 0;
        ID.SortFlag             = 0;
        ID.InterlaceFlag        = 0;
        ID.LocalColorTableFlag  = 1;

        WriteImageDescriptor( MyFile, &ID );

        // Local Color Table schreiben
        for ( int i = 0; i < 256; ++i )
        {
          MyFile.WriteU8( m_pSavingData->Palette().Red( i ) );
          MyFile.WriteU8( m_pSavingData->Palette().Green( i ) );
          MyFile.WriteU8( m_pSavingData->Palette().Blue( i ) );
        }

        // Write code size
        codesize = BitsNeeded( NumColors );
        if ( codesize == 1 )
        {
          ++codesize;
        }
        MyFile.WriteU8( codesize );

        // Perform compression
        m_FrameXOffset = m_FrameYOffset = 0;
        if ( ( errcode = LZW_Compress( codesize ) ) != 0 )
        {
          return FALSE;
        }
        // Write terminating 0-BYTE
        MyFile.WriteU8( 0 );

      }

      // Initiate and write ending image descriptor
      ID.Separator = ';';
      WriteImageDescriptor( MyFile, &ID );

      // Close file
      MyFile.Close();

      return true;
    }



    bool FormatGIF::SaveAsGIF87( const GR::String& FileName, ImageSet* pSet )
    {
      if ( !CanSave( pSet ) )
      {
        return false;
      }

      struct ScreenDescriptor
      {
        GR::u16             LocalScreenWidth,
                            LocalScreenHeight;
        GR::u8              GlobalColorTableSize : 3,
                            SortFlag : 1,
                            ColorResolution : 3,
                            GlobalColorTableFlag : 1;
        GR::u8              BackgroundColorIndex;
        GR::u8              PixelAspectRatio;
      };


      int                   q,
                            tabsize,
                            codesize,
                            errcode;

      ImageDescriptor       ID;

      GR::u8*               bp,
                            tmp;

      ScreenDescriptor      SD;

      long                  maxcolor;

      GR::IO::FileStream    MyFile;

      GR::u8*               p;



      m_pSavingFile = &MyFile;

      int width  = pSet->Image( 0, 0 )->Width();
      int height = pSet->Image( 0, 0 )->Height();

      // Initiate variables for new GIF-file
      NumColors     = 256;                // Anzahl Farben //numcolors ? ( 1 << BitsNeeded( numcolors ) ) : 0;
      ScreenHeight  = width;          // Bildschirm-Breite
      ScreenWidth   = height;          // Bildschirm-Höhe

      // Create file specified
      if ( !MyFile.Open( FileName, GR::IO::FileStream::OT_WRITE_ONLY ) )
      {
        return false;
      }

      // Write GIF signature
      if ( !MyFile.WriteBlock( "GIF87a", 6 ) )
      {
        MyFile.Close();
        return false;
      }

      // Initiate and write screen descriptor
      SD.LocalScreenWidth   = width;
      SD.LocalScreenHeight  = height;
      if ( NumColors )
      {
        SD.GlobalColorTableSize = BitsNeeded( NumColors ) - 1;
        SD.GlobalColorTableFlag = 1;
      }
      else
      {
        SD.GlobalColorTableSize = 0;
        SD.GlobalColorTableFlag = 0;
      }
      SD.SortFlag             = 0;
      SD.ColorResolution      = 7;   // Bits pro Pixel - 1
      SD.BackgroundColorIndex = 0;
      SD.PixelAspectRatio     = 0;

      MyFile.WriteU16( SD.LocalScreenWidth );
      MyFile.WriteU16( SD.LocalScreenHeight );

      tmp = ( SD.GlobalColorTableFlag << 7 )
          | ( SD.ColorResolution << 4 )
          | ( SD.SortFlag << 3 )
          | ( SD.GlobalColorTableSize );
      MyFile.WriteU8( tmp );
      MyFile.WriteU8( SD.BackgroundColorIndex );
      MyFile.WriteU8( SD.PixelAspectRatio );

      // Allocate color table
      if ( NumColors )
      {
        tabsize = NumColors * 3;
        bp = ColorTable;
        for ( q = 0; q < tabsize; q++ )
        {
          *bp++ = 0;
        }
      }

      GR::Graphic::Palette* pPal = pSet->Palette( 0, 0 );
      maxcolor = ( 1L << 8 ) - 1L;
      for ( q = 0; q < 256; q++ )
      {
        p = ColorTable + q * 3;
        *p++ = ( GR::u8 ) ( ( ( pPal->Red( q ) ) * 255L ) / maxcolor );
        *p++ = ( GR::u8 ) ( ( ( pPal->Green( q ) ) * 255L ) / maxcolor );
        *p++ = ( GR::u8 ) ( ( ( pPal->Blue( q ) ) * 255L ) / maxcolor );
      }

      if ( width < 0 )
      {
        width = ScreenWidth;
      }
      if ( height < 0 )
      {
        height = ScreenHeight;
      }

      // Write global colortable if any
      if ( NumColors )
      {
        if ( !MyFile.WriteBlock( ColorTable, NumColors * 3 ) )
        {
          return false;
        }
      }

      for ( size_t iFrames = 0; iFrames < pSet->FrameCount(); ++iFrames )
      {
        m_pSavingData = pSet->Image( iFrames, 0 );

        // image descriptor
        ImageLeft               = 0;
        ImageTop                = 0;
        ID.Separator            = ',';
        ID.LeftPosition         = 0;
        ID.TopPosition          = 0;
        ID.Width                = ImageWidth  = width;
        ID.Height               = ImageHeight = height;
        ID.LocalColorTableSize  = 7;
        ID.Reserved             = 0;
        ID.SortFlag             = 0;
        ID.InterlaceFlag        = 0;
        ID.LocalColorTableFlag  = 1;

        WriteImageDescriptor( MyFile, &ID );

        // Local Color Table schreiben
        for ( int i = 0; i < 256; ++i )
        {
          MyFile.WriteU8( m_pSavingData->Palette().Red( i ) );
          MyFile.WriteU8( m_pSavingData->Palette().Green( i ) );
          MyFile.WriteU8( m_pSavingData->Palette().Blue( i ) );
        }

        // Write code size
        codesize = BitsNeeded( NumColors );
        if ( codesize == 1 )
        {
          ++codesize;
        }
        MyFile.WriteU8( codesize );

        // Perform compression
        m_FrameXOffset = m_FrameYOffset = 0;
        if ( ( errcode = LZW_Compress( codesize ) ) != 0 )
        {
          return FALSE;
        }
        // Write terminating 0-BYTE
        MyFile.WriteU8( 0 );

      }

      // ending image descriptor
      ID.Separator = ';';
      WriteImageDescriptor( MyFile, &ID );

      MyFile.Close();

      return true;
    }



    GR::String FormatGIF::GetDescription()
    {
      return "GIF Image";
    }



    GR::String FormatGIF::GetFilterString()
    {
      return "*.gif";
    }

  }
}