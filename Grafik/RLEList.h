#ifndef RLE_LIST_H
#define RLE_LIST_H



#include <GR/GRTypes.h>

#include <Grafik/ImageData.h>



namespace GR
{
  namespace Graphic
  {
    class RLEList
    {

      public:

        enum eConstants
        {
          RLE_MAX_COMPRESS_SECTOR       = 0x3f,
        };

        enum eSectorType
        {
          ST_OPAQUE       = 0,
          ST_TRANSPARENT  = 0x80,
          ST_SMALL_ALPHA  = 0x40,
        };

        void*                     m_pData;

        size_t                    m_Size;



        RLEList();
        RLEList( const RLEList& rleRHS );
        ~RLEList();

        RLEList& operator=( const RLEList& image );


        unsigned long             Compress( GR::Graphic::ImageData& ImageData, int X, int Y, int Width, int Height, GR::u32 TransparentColor, bool WriteList );
        unsigned long             CompressMask( GR::Graphic::ImageData& ImageData, int X, int Y, int Width, int Height, bool WriteList );
        unsigned long             CompressMaskA1( GR::Graphic::ImageData& ImageData, int X, int Y, int Width, int Height, bool WriteList );

        void                      WriteSector( GR::Graphic::ImageData& ImageData, 
                                               unsigned char** ppList,
                                               const unsigned long size, 
                                               const GR::u8 Flag, 
                                               const bool Write );

        void                      Release();

    };

  }
}

#endif // RLE_LIST_H



