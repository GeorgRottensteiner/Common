#include <IO/FileStream.h>
#include <IO\FileStream.h>

#include <Misc/Misc.h>

#include <String/StringUtil.h>

#include <debug/debugclient.h>

#include "FormatGIF.h"



GR::Graphic::FormatGIF    globalGIFPlugin;



namespace GR
{
  namespace Graphic
  {
    FormatGIF::FormatGIF() :
      m_pImageData( NULL ),
      GIFStrChr( NULL ),
      StrNxt( NULL ),
      StrHsh( NULL )
    {
      ImageFormatManager::Instance().RegisterFormat( GetDescription(), this );

      ncodemask[0]  = 0;
      ncodemask[1]  = 0x0001;
      ncodemask[2]  = 0x0003;
      ncodemask[3]  = 0x0007;
      ncodemask[4]  = 0x000f;
      ncodemask[5]  = 0x001f;
      ncodemask[6]  = 0x003f;
      ncodemask[7]  = 0x007f;
      ncodemask[8]  = 0x00ff;
      ncodemask[9]  = 0x01ff;
      ncodemask[10] = 0x03ff;
      ncodemask[11] = 0x07ff;
      ncodemask[12] = 0x0fff;
    }



    bool FormatGIF::IsFileOfType( const GR::String& FileName )
    {
      size_t   length = FileName.length();
      if ( length >= 3 )
      {
        if ( ( toupper( FileName[length - 3] ) != 'G' )
        ||   ( toupper( FileName[length - 2] ) != 'I' )
        ||   ( toupper( FileName[length - 1] ) != 'F' ) )
        {
          return false;
        }
      }
      else
      {
        return false;
      }
      return true;
    }



    int FormatGIF::GetNextByte()
    {
      GR::u8  ucDummy;

      if ( m_pFile->ReadBlock( &ucDummy, 1 ) )
      {
        return ucDummy;
      }
      return -1;
    }



    size_t FormatGIF::ReadBlobBlock( GR::u8* pData )
    {
      size_t          count = 0;
      unsigned char   block_count = 0;

      count = GetNextByte();
      if ( count == 0 )
      {
        return( 0 );
      }
      block_count = (unsigned char)count;

      for ( int i = 0; i < block_count; ++i )
      {
        *pData++ = GetNextByte();
      }
      return block_count;
    }



    short int FormatGIF::DecodeImage( char* pTarget, const long opacity )
    {
    #define MaxStackSize  4096
    #define NullCode  (-1)

      int               bits = 0,
                        code_size,
                        offset = 0,
                        pass = 0;

      long              available,
                        clear,
                        code,
                        code_mask,
                        end_of_information,
                        in_code,
                        old_code,
                        y;

      long              x;

      unsigned char*    c = NULL;

      unsigned long     datum = 0;

      size_t            count = 0;

      short*            prefix;

      unsigned char     data_size,
                        first = 0,
                        index;

      GR::u8*           packet;
      GR::u8*           pixel_stack;
      GR::u8*           suffix;
      GR::u8*           top_stack;



      // Initialize GIF data stream decoder
      data_size = GetNextByte();
      if ( data_size > 8 )
      {
        return false;
      }

      // Allocate decoder tables
      packet      = (GR::u8*)malloc( 256 );
      prefix      = (GR::i16*)malloc( MaxStackSize * sizeof( short ) );
      suffix      = (GR::u8*) malloc( MaxStackSize );
      pixel_stack = (GR::u8*) malloc( MaxStackSize + 1 );

      clear               = 1 << data_size;
      end_of_information  = clear + 1;
      available           = clear + 2;
      old_code            = NullCode;
      code_size           = data_size + 1;
      code_mask           = ( 1 << code_size ) - 1;
      for ( code = 0; code < clear; code++ )
      {
        prefix[code] = 0;
        suffix[code] = (GR::u8)code;
      }

      // Decode GIF pixel stream
      top_stack = pixel_stack;

      char* pTargetOrig = pTarget;

      for ( y = 0; y < (long)nheight; y++ )
      {
        pTarget = pTargetOrig + m_FrameXOffset + ( offset + m_FrameYOffset ) * m_FullWidth;

        for ( x = 0; x < (long)nwidth; )
        {
          if ( top_stack == pixel_stack )
          {
            if ( bits < code_size )
            {
              // Load bytes until there is enough bits for a code
              if ( count == 0 )
              {
                // Read a new data block
                count = ReadBlobBlock( packet );
                if ( count == 0 )
                {
                  break;
                }
                c = packet;
              }
              datum += (unsigned long)( *c ) << bits;
              bits  += 8;
              ++c;
              --count;
              continue;
            }

            // Get the next code
            code    = (long)( datum & code_mask );
            datum   >>= code_size;
            bits    -= code_size;

            // Interpret the code
            if ( ( code > available )
            ||   ( code == end_of_information ) )
            {
              break;
            }
            if ( code == clear )
            {
              // Reset decoder
              code_size = data_size + 1;
              code_mask = ( 1 << code_size ) - 1;
              available = clear + 2;
              old_code  = NullCode;
              continue;
            }
            if ( old_code == NullCode )
            {
              *top_stack++  = suffix[code];
              old_code      = code;
              first         = (unsigned char)code;
              continue;
            }
            in_code = code;
            if ( code >= available )
            {
              *top_stack++  = first;
              code          = old_code;
            }
            while ( code >= clear )
            {
              *top_stack++  = suffix[code];
              code          = prefix[code];
            }
            first = suffix[code];

            // Add a new string to the string table
            if ( available >= MaxStackSize )
            {
              break;
            }
            *top_stack++      = first;
            prefix[available] = (short)old_code;
            suffix[available] = first;
            ++available;
            if ( ( ( available & code_mask ) == 0 )
            &&   ( available < MaxStackSize ) )
            {
              ++code_size;
              code_mask += available;
            }
            old_code = in_code;
          }

          // Pop a pixel off the pixel stack
          --top_stack;
          index = *top_stack;

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

          //ConstrainColormapIndex(image,index);
          //indexes[x]=index;
          //*q=image->colormap[index];
          /*
          q->opacity=(Quantum)
            (index == opacity ? TransparentOpacity : OpaqueOpacity);
            */
          ++x;
          //q++;
        }
        if ( !m_Interlaced )
        {
          ++offset;
        }
        else
        {
          if ( GIFPass == 0 )
          {
            offset += 8;
            if ( offset >= nheight )
            {
              offset = 4 - 8;
              GIFPass = 1;
            }
          }
          if ( GIFPass == 1 )
          {
            offset += 8;
            if ( offset >= nheight )
            {
              offset = 2 - 4;
              GIFPass = 2;
            }
          }
          if ( GIFPass == 2 )
          {
            offset += 4;
            if ( offset >= nheight )
            {
              offset = 1 - 2;
              GIFPass = 3;
            }
          }
          if ( GIFPass == 3 )
          {
            offset += 2;
          }
        }

        pTarget = pTargetOrig + offset * m_FullWidth;
        if ( x < (long)nwidth )
        {
          break;
        }
      }
      free( pixel_stack );
      free( suffix );
      free( prefix );
      free( packet );

      return true;
    }



    ImageSet* FormatGIF::LoadSet( const GR::String& FileName )
    {
      m_TempTransparentIndex = -1;
      GIFPass = 0;

      m_pFile = new GR::IO::FileStream();

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
      if ( !m_pFile->Open( FileName ) )
      {
        delete m_pFile;
        return NULL;
      }

	    // *1* READ HEADER (SIGNATURE + VERSION)
	    char szSignature[6];				// First 6 bytes (GIF87a or GIF89a)

      m_pFile->ReadBlock( szSignature, 6 );
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

	    m_pFile->ReadBlock( &giflsd, sizeof( giflsd ) );

	    GlobalBPP = ( giflsd.PackedFields & 0x07 ) + 1;

	    // fill some animation data:
	    int FrameWidth  = giflsd.ScreenWidth;
	    int FrameHeight = giflsd.ScreenHeight;
	    int nLoops      = 0;

      m_FullWidth     = FrameWidth;
      m_FullHeight    = FrameHeight;

	    // *3* READ/GENERATE GLOBAL COLOR MAP
      if ( giflsd.PackedFields & 0x80 )
      {
        // File has global color map?
        for ( n = 0; n < 1 << GlobalBPP; n++ )
        {
          GlobalColorMap[n * 3 + 0] = m_pFile->ReadU8();
          GlobalColorMap[n * 3 + 1] = m_pFile->ReadU8();
          GlobalColorMap[n * 3 + 2] = m_pFile->ReadU8();
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
	    do
	    {
		    int charGot = m_pFile->ReadU8();

		    if ( charGot == 0x21 )		// *A* EXTENSION BLOCK
		    {
			    switch ( m_pFile->ReadU8() )
			    {
			      case 0xF9:
              // Graphic Control Extension
				      m_pFile->ReadBlock( &gifgce, sizeof( gifgce ) );

				      ++GraphicExtensionFound;

              // Block Terminator (always 0)
				      m_pFile->ReadU8();
				      break;
			      case 0xFF:			
              // Application Extension: Ignored
              {
                int nBlockLength = m_pFile->ReadU8();

                m_pFile->SetPosition( nBlockLength, IIOStream::PT_CURRENT );

                GR::u8    lengthSubBlock = -1;
                do
                {
                  lengthSubBlock = m_pFile->ReadU8();
                  m_pFile->SetPosition( lengthSubBlock, IIOStream::PT_CURRENT );
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
              while ( int nBlockLength = m_pFile->ReadU8() )
              {
                for ( n = 0; n < nBlockLength; n++ )
                {
                  m_pFile->ReadU8();
                }
              }
				      break;
			    }
		    }
		    else if ( charGot == 0x2c )
        {
          // *B* IMAGE (0x2c Image Separator)

			    // Create a new Image Object:
          m_pImageData  = new GR::Graphic::ImageData();
          pCurrentFrame = pSet->AddFrame( m_pImageData );

			    // Read Image Descriptor
          tGIFImageDescriptor   gifID;

			    m_pFile->ReadBlock( &gifID, sizeof( gifID ) );

          int LocalColorMap         = ( gifID.PackedFields & 0x80 ) ? 1 : 0;
          int iLocalColorTableSize  = 1 << ( ( gifID.PackedFields & 0x07 ) + 1 );

          m_pImageData->CreateData( FrameWidth, FrameHeight, GR::Graphic::IF_PALETTED );

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
            m_TempTransparentIndex = ( gifgce.PackedFields&0x01 ) ? gifgce.Transparent : -1;

            pCurrentFrame->DelayMS = gifgce.Delay * 10;

            m_pImageData->TransparentColorUsed( true );
            m_pImageData->TransparentColor( m_TempTransparentIndex );

            if ( m_pImageData->TransparentColorUsed() )
            {
              memset( m_pImageData->Data(), m_TempTransparentIndex, m_pImageData->DataSize() );
            }

            // für optimierte GIF-Anis den alten Frame einsetzen
            int   disposalMethod = ( ( gifgce.PackedFields & 0x1c ) >> 2 );

            switch ( disposalMethod )
            {
              case 0:
                //dh::Log( "no disposal required\n" );
                // kein Entfernen, letzen Frame einblenden
                if ( pSet->Frames.size() >= 2 )
                {
                  tImageSetFrame*   pPrevFrame = pSet->Frames[pSet->Frames.size() - 2];

                  memcpy( pCurrentFrame->Layers[0]->pImageData->Data(),
                          pPrevFrame->Layers[0]->pImageData->Data(),
                          pCurrentFrame->Layers[0]->pImageData->DataSize() );
                }
                break;
              case 1:
                //dh::Log( "don't dispose\n" );
                if ( pSet->Frames.size() >= 2 )
                {
                  tImageSetFrame*   pPrevFrame = pSet->Frames[pSet->Frames.size() - 2];

                  memcpy( pCurrentFrame->Layers[0]->pImageData->Data(),
                          pPrevFrame->Layers[0]->pImageData->Data(),
                          pCurrentFrame->Layers[0]->pImageData->DataSize() );
                }
                break;
              case 2:
                //dh::Log( "restore to background color\n" );
                memset( pCurrentFrame->Layers[0]->pImageData->Data(), giflsd.Background, pCurrentFrame->Layers[0]->pImageData->DataSize() );
                break;
              case 3:
                //dh::Log( "restore to previous\n" );
                if ( pSet->Frames.size() >= 3 )
                {
                  tImageSetFrame*   pPrevFrame = pSet->Frames[pSet->Frames.size() - 3];

                  memcpy( pCurrentFrame->Layers[0]->pImageData->Data(),
                          pPrevFrame->Layers[0]->pImageData->Data(),
                          pCurrentFrame->Layers[0]->pImageData->DataSize() );
                }
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
			    }

			    if ( LocalColorMap )		// Read Color Map (if descriptor says so)
          {
            memset( pCurrentFrame->Layers[0]->pImageData->Palette().Data(), 0, pCurrentFrame->Layers[0]->pImageData->Palette().Entries() * 3 );
            for ( int i = 0; i < iLocalColorTableSize; ++i )
            {
              int   iR = m_pFile->ReadU8();
              int   iG = m_pFile->ReadU8();
              int   iB = m_pFile->ReadU8();
              pCurrentFrame->Layers[0]->pImageData->Palette().SetColor( i, iR, iG, iB );
            }
          }
			    else					// Otherwise copy Global
          {
            memset( pCurrentFrame->Layers[0]->pImageData->Palette().Data(), 0, pCurrentFrame->Layers[0]->pImageData->Palette().Entries() * 3 );
				    memcpy( pCurrentFrame->Layers[0]->pImageData->Palette().Data(), GlobalColorMap,
					          3 * ( 1 << GlobalBPP ) );
          }

			    // Call LZW/GIF decompressor
          nwidth  = gifID.Width;
          nheight = gifID.Height;

          n = DecodeImage( (char*)pCurrentFrame->Layers[0]->pImageData->Data(), m_TempTransparentIndex );

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

        if ( m_pFile->GetPosition() == m_pFile->GetSize() )
        {
          break;
        }
	    }
      while ( m_pFile->IsGood() );

      m_pFile->Close();
      delete m_pFile;
      m_pFile = NULL;

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
      if ( RelPixY >= ImageHeight )
      {
        return -1;
      }

      unsigned char ret;
      memcpy( &ret, (GR::u8*)m_pSavingData->Data() + RelPixX + RelPixY * m_pSavingData->Width(), 1 );

      if ( ++RelPixX >= ImageWidth )
      {
        RelPixX = 0;
        ++RelPixY;
      }

      return ret;
    }



    /*========================================================================*
     =                                                                        =
     =                      Routines to write a bit-file                      =
     =                                                                        =
     *========================================================================*/

    /*-------------------------------------------------------------------------
     *
     *  NAME:           InitBitFile()
     *
     *  DESCRIPTION:    Initiate for using a bitfile. All output is sent to
     *                  the current OutFile using the I/O-routines above.
     *
     *  PARAMETERS:     None
     *
     *  RETURNS:        Nothing
     *
     */
    void FormatGIF::InitBitFile()
    {
      Buffer[Index = 0] = 0;
      BitsLeft          = 8;
    }





    /*-------------------------------------------------------------------------
     *
     *  NAME:           ResetOutBitFile()
     *
     *  DESCRIPTION:    Tidy up after using a bitfile
     *
     *  PARAMETERS:     None
     *
     *  RETURNS:        0 - OK, -1 - error
     *
     */
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





    /*-------------------------------------------------------------------------
     *
     *  NAME:           WriteBits()
     *
     *  DESCRIPTION:    Put the given number of bits to the outfile.
     *
     *  PARAMETERS:     bits    - bits to write from (right justified)
     *                  numbits - number of bits to write
     *
     *  RETURNS:        bits written, or -1 on error.
     *
     */
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



    /*-------------------------------------------------------------------------
     *
     *  NAME:           FreeStrtab()
     *
     *  DESCRIPTION:    Free arrays used in string table routines
     *
     *  PARAMETERS:     None
     *
     *  RETURNS:        Nothing
     *
     */
    void FormatGIF::FreeStrtab()
    {
      if ( StrHsh )
      {
        free(StrHsh);
        StrHsh = NULL;
      }

      if ( StrNxt )
      {
        free( StrNxt );
        StrNxt = NULL;
      }

      if ( GIFStrChr )
      {
        free( GIFStrChr );
        GIFStrChr = NULL;
      }
    }



    /*-------------------------------------------------------------------------
     *
     *  NAME:           AllocStrtab()
     *
     *  DESCRIPTION:    Allocate arrays used in string table routines
     *
     *  PARAMETERS:     None
     *
     *  RETURNS:        GIF_OK     - OK
     *                  GIF_OUTMEM - Out of memory
     *
     */
    int FormatGIF::AllocStrtab()
    {
      // Just in case . . .
      FreeStrtab();

      if ( ( GIFStrChr = (GR::u8*)malloc( ( 1 << 12 ) * sizeof( GR::u8 ) ) ) == NULL )
      {
        FreeStrtab();
        return 3;
      }

      if ( ( StrNxt = (GR::u16*)malloc( ( 1 << 12 ) * sizeof( GR::u16 ) ) ) == NULL )
      {
        FreeStrtab();
        return 3;
      }

      if ( ( StrHsh = (GR::u16*)malloc( 9973 * sizeof( GR::u16 ) ) ) == 0 )
      {
        FreeStrtab();
        return 3;
      }

      return 0;
    }



    /*-------------------------------------------------------------------------
     *
     *  NAME:           AddCharString()
     *
     *  DESCRIPTION:    Add a string consisting of the string of index plus
     *                  the BYTE b.
     *
     *                  If a string of length 1 is wanted, the index should
     *                  be 0xFFFF.
     *
     *  PARAMETERS:     index - Index to first part of string, or 0xFFFF is
     *                          only 1 BYTE is wanted
     *                  b     - Last BYTE in new string
     *
     *  RETURNS:        Index to new string, or 0xFFFF if no more room
     *
     */
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
      while ( StrHsh[hshidx] != 0xFFFF )
      {
        hshidx = ( hshidx + 2039 ) % 9973;
      }

      // Insert new string
      StrHsh[hshidx]        = NumStrings;
      GIFStrChr[NumStrings] = b;
      StrNxt[NumStrings]    = ( index != 0xFFFF) ? index : 0xffff;

      return NumStrings++;
    }





    /*-------------------------------------------------------------------------
     *
     *  NAME:           FindCharString()
     *
     *  DESCRIPTION:    Find index of string consisting of the string of index
     *                  plus the BYTE b.
     *
     *                  If a string of length 1 is wanted, the index should
     *                  be 0xFFFF.
     *
     *  PARAMETERS:     index - Index to first part of string, or 0xFFFF is
     *                          only 1 BYTE is wanted
     *                  b     - Last BYTE in string
     *
     *  RETURNS:        Index to string, or 0xFFFF if not found
     *
     */
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
      while ( ( nxtidx = StrHsh[hshidx] ) != 0xFFFF )
      {
        if ( ( StrNxt[nxtidx] == index )
        &&   ( GIFStrChr[nxtidx] == b ) )
        {
            return nxtidx;
        }
        hshidx = ( hshidx + 2039 ) % 9973;
      }

      // No match is found
      return 0xFFFF;
    }



    /*-------------------------------------------------------------------------
     *
     *  NAME:           ClearStrtab()
     *
     *  DESCRIPTION:    Mark the entire table as free, enter the 2**codesize
     *                  one-BYTE strings, and reserve the RES_CODES reserved
     *                  codes.
     *
     *  PARAMETERS:     codesize - Number of bits to encode one pixel
     *
     *  RETURNS:        Nothing
     *
     */
    void FormatGIF::ClearStrtab( int codesize )
    {
      int       q,
                w;
      GR::u16*  wp;


      // No strings currently in the table
      NumStrings = 0;

      // Mark entire hashtable as free
      wp = StrHsh;
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



    /*========================================================================*
     =                                                                        =
     =                        LZW compression routine                         =
     =                                                                        =
     *========================================================================*/

    /*-------------------------------------------------------------------------
     *
     *  NAME:           LZW_Compress()
     *
     *  DESCRIPTION:    Perform LZW compression as specified in the
     *                  GIF-standard.
     *
     *  PARAMETERS:     codesize  - Number of bits needed to represent
     *                              one pixelvalue.
     *                  inputBYTE - Function that fetches each BYTE to compress.
     *                              Must return -1 when no more BYTEs.
     *
     *  RETURNS:        GIF_OK     - OK
     *                  GIF_OUTMEM - Out of memory
     *
     */
    int FormatGIF::LZW_Compress(int codesize)
    {
      int     c;
      GR::u16 index;
      int     clearcode,
              endofinfo,
              numbits,
              limit,
              errcode;
      GR::u16 prefix = 0xFFFF;


      // Set up the given outfile
      InitBitFile();

      // Set up variables and tables
      clearcode = 1 << codesize;
      endofinfo = clearcode + 1;
      numbits   = codesize + 1;
      limit     = ( 1 << numbits ) - 1;

      if ( ( errcode = AllocStrtab() ) != 0 )
      {
        return errcode;
      }
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

      // Write end of info -mark.
      WriteBits( endofinfo, numbits );

      // Flush the buffer
      ResetOutBitFile();

      // Tidy up
      FreeStrtab();

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
      BitsPrPrimColor   = 8;            // Bits pro Pixel
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
      RelPixX = RelPixY = 0;
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
      BitsPrPrimColor = 8;            // Bits pro Pixel
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
        RelPixX = RelPixY = 0;
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
        GR::u16         LocalScreenWidth,
                        LocalScreenHeight;
        GR::u8          GlobalColorTableSize : 3,
                        SortFlag : 1,
                        ColorResolution : 3,
                        GlobalColorTableFlag : 1;
        GR::u8          BackgroundColorIndex;
        GR::u8          PixelAspectRatio;
      };


      int                   q,
        tabsize,
        codesize,
        errcode,
        iWidth,
        iHeight;

      ImageDescriptor       ID;

      BYTE* bp,
            tmp;

      ScreenDescriptor      SD;

      long                  maxcolor;

      GR::IO::FileStream             MyFile;

      BYTE* p;

      m_pSavingFile = &MyFile;

      iWidth = pSet->Image( 0, 0 )->Width();
      iHeight = pSet->Image( 0, 0 )->Height();

      // Initiate variables for new GIF-file
      NumColors = 256;                // Anzahl Farben //numcolors ? ( 1 << BitsNeeded( numcolors ) ) : 0;
      BitsPrPrimColor = 8;            // Bits pro Pixel
      ScreenHeight = iWidth;          // Bildschirm-Breite
      ScreenWidth = iHeight;          // Bildschirm-Höhe

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
      SD.LocalScreenWidth = iWidth;
      SD.LocalScreenHeight = iHeight;
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
      SD.SortFlag = 0;
      SD.ColorResolution = 7;   // Bits pro Pixel - 1
      SD.BackgroundColorIndex = 0;
      SD.PixelAspectRatio = 0;

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
        // GEORG 10.7.2002
        // Paletten haben jetzt volle 256-Weite
        p = ColorTable + q * 3;
        *p++ = ( GR::u8 ) ( ( ( pPal->Red( q ) ) * 255L ) / maxcolor );
        *p++ = ( GR::u8 ) ( ( ( pPal->Green( q ) ) * 255L ) / maxcolor );
        *p++ = ( GR::u8 ) ( ( ( pPal->Blue( q ) ) * 255L ) / maxcolor );
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

      for ( size_t iFrames = 0; iFrames < pSet->FrameCount(); ++iFrames )
      {
        m_pSavingData = pSet->Image( iFrames, 0 );

        // Initiate and write image descriptor
        ImageLeft = 0;
        ImageTop = 0;
        ID.Separator = ',';
        ID.LeftPosition = 0;
        ID.TopPosition = 0;
        ID.Width = ImageWidth = iWidth;
        ID.Height = ImageHeight = iHeight;
        ID.LocalColorTableSize = 7;
        ID.Reserved = 0;
        ID.SortFlag = 0;
        ID.InterlaceFlag = 0;
        ID.LocalColorTableFlag = 1;

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
        RelPixX = RelPixY = 0;
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