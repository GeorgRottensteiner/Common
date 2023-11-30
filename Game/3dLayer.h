#ifndef THREEDLAYER_H
#define THREEDLAYER_H



#include <string>
#include <list>

#include <GR/GRTypes.h>


namespace GR
{
  namespace Gamebase
  {

    template <typename T> class TileLayer3D
    {

      protected:

        T               m_InvalidValue;

        int             m_iWidth,
                        m_iHeight,
                        m_iDepth;

      public:

        GR::String     m_strDesc;

        bool            m_bWrapping;

        T*              m_pData;


        TileLayer3D() :
          m_pData( NULL ),
          m_iWidth( 0 ),
          m_iHeight( 0 ),
          m_iDepth( 0 ),
          m_InvalidValue( T() ),
          m_bWrapping( false )
        {
        }

        TileLayer3D( GR::u32 dwWidth, GR::u32 dwHeight, GR::u32 dwDepth, const GR::String& Desc = GR::String() ) :
          m_strDesc( Desc ),
          m_pData( NULL ),
          m_iWidth( dwWidth ),
          m_iHeight( dwHeight ),
          m_iDepth( dwDepth ),
          m_InvalidValue( T() ),
          m_bWrapping( false )
        {
          InitFeld( dwWidth, dwHeight, dwDepth, "" );
          ResetFeld( m_InvalidValue );
        }

        //- Copy-Construktor
        TileLayer3D( const TileLayer3D& t3lCopyFrom )
        {
          if ( &t3lCopyFrom == this )
          {
            return;
          }

          m_strDesc         = t3lCopyFrom.m_strDesc;
          m_iWidth          = t3lCopyFrom.m_iWidth;
          m_iHeight         = t3lCopyFrom.m_iHeight;
          m_iDepth          = t3lCopyFrom.m_iDepth;
          m_InvalidValue    = t3lCopyFrom.m_InvalidValue;
          m_bWrapping       = t3lCopyFrom.m_bWrapping;

          m_pData           = new T[m_iWidth * m_iHeight * m_iDepth];
          for ( int i = 0; i < m_iWidth * m_iHeight * m_iDepth; ++i )
          {
            m_pData[i] = t3lCopyFrom.m_pData[i];
          }
        }

        virtual ~TileLayer3D()
        {
          if ( m_pData != NULL )
          {
            delete[] m_pData;
            m_pData = NULL;
          }
        }

        TileLayer3D& operator= ( const TileLayer3D& t3lCopyFrom )
        {
          if ( this == &t3lCopyFrom )
          {
            return *this;
          }

          if ( m_pData != NULL )
          {
            delete[] m_pData;
            m_pData = NULL;
          }

          m_strDesc         = t3lCopyFrom.m_strDesc;
          m_iWidth          = t3lCopyFrom.m_iWidth;
          m_iHeight         = t3lCopyFrom.m_iHeight;
          m_iDepth          = t3lCopyFrom.m_iDepth;
          m_InvalidValue    = t3lCopyFrom.m_InvalidValue;
          m_bWrapping       = t3lCopyFrom.m_bWrapping;

          m_pData           = new T[m_iWidth * m_iHeight * m_iDepth];
          for ( int i = 0; i < m_iWidth * m_iHeight * m_iDepth; ++i )
          {
            m_pData[i] = t3lCopyFrom.m_pData[i];
          }

          return *this;
        }


        virtual void  InitFeld( GR::u32 dwWidth, GR::u32 dwHeight, GR::u32 dwDepth, const char *szDesc = "" )
        {
          if ( m_pData != NULL )
          {
            delete[] m_pData;
            m_pData = NULL;
          }

          m_strDesc           = szDesc;
          m_iWidth            = dwWidth;
          m_iHeight           = dwHeight;
          m_iDepth            = dwDepth;

          m_pData = new T[m_iWidth * m_iHeight * m_iDepth];
          ResetFeld( m_InvalidValue );
        }

        virtual void  ResetFeld( const T& FillValue )
        {
          if ( m_pData == NULL )
          {
            return;
          }
          for ( int i = 0; i < m_iWidth * m_iHeight * m_iDepth; i++ )
          {
            m_pData[i] = FillValue;
          }
        }

        virtual void  SetFeld( int iX, int iY, int iZ, const T& FieldValue )
        {
          if ( m_pData == NULL )
          {
            return;
          }
          if ( ( iX < 0 )
          ||   ( iY < 0 )
          ||   ( iZ < 0 )
          ||   ( iX >= m_iWidth )
          ||   ( iY >= m_iHeight )
          ||   ( iZ >= m_iDepth ) )
          {
            // auﬂerhalb
            return;
          }
          m_pData[iX + iY * m_iWidth + iZ * m_iWidth * m_iHeight] = FieldValue;
        }

        virtual T&    GetFeld( int iX, int iY, int iZ )
        {
          if ( m_pData == NULL )
          {
            return m_InvalidValue;
          }
          if ( m_bWrapping )
          {
            while ( iX < 0 )
            {
              iX += m_iWidth;
            }
            while ( iX >= m_iWidth )
            {
              iX -= m_iWidth;
            }
            while ( iY < 0 )
            {
              iY += m_iHeight;
            }
            while ( iY >= m_iHeight )
            {
              iY -= m_iHeight;
            }
            while ( iZ < 0 )
            {
              iZ += m_iDepth;
            }
            while ( iZ >= m_iDepth )
            {
              iZ -= m_iDepth;
            }
          }
          else if ( ( iX < 0 )
          ||        ( iY < 0 )
          ||        ( iZ < 0 )
          ||        ( iX >= m_iWidth )
          ||        ( iY >= m_iHeight )
          ||        ( iZ >= m_iDepth ) )
          {
            // auﬂerhalb
            return m_InvalidValue;
          }
          return m_pData[iX + iY * m_iWidth + iZ * m_iWidth * m_iHeight];
        }

        bool IsInside( int iX, int iY, int iZ ) const
        {
          if ( ( iX < 0 )
          ||   ( iX >= m_iWidth )
          ||   ( iY < 0 )
          ||   ( iY >= m_iHeight )
          ||   ( iZ < 0 )
          ||   ( iZ >= m_iDepth ) )
          {
            return false;
          }
          return true;
        }

        virtual void  Resize( int iWidth, int iHeight, int iDepth )
        {
          if ( ( iWidth <= 0 )
          ||   ( iHeight <= 0 )
          ||   ( iDepth <= 0 )
          ||   ( ( iWidth == m_iWidth )
          &&     ( iHeight == m_iHeight )
          &&     ( iDepth == m_iDepth ) ) )
          {
            return;
          }

          T*   pOldData = m_pData;

          m_pData = new T[iWidth * iHeight * iDepth];

          int   iOldWidth = m_iWidth,
                iOldHeight = m_iHeight,
                iOldDepth = m_iDepth;

          m_iWidth    = iWidth;
          m_iHeight   = iHeight;
          m_iDepth    = iDepth;
          ResetFeld( m_InvalidValue );

          int   iCopyWidth = iOldWidth,
                iCopyHeight = iOldHeight,
                iCopyDepth = iOldDepth;

          if ( iCopyWidth > m_iWidth )
          {
            iCopyWidth = m_iWidth;
          }
          if ( iCopyHeight > m_iHeight )
          {
            iCopyHeight = m_iHeight;
          }
          if ( iCopyDepth > iDepth )
          {
            iCopyDepth = iDepth;
          }

          for ( int i = 0; i < iCopyWidth; i++ )
          {
            for ( int j = 0; j < iCopyHeight; j++ )
            {
              for ( int k = 0; k < iCopyDepth; k++ )
              {
                m_pData[i + j * m_iWidth + k * m_iWidth * m_iHeight] = pOldData[i + j * iOldWidth + k * iOldWidth * iOldHeight];
              }
            }
          }
          delete[] pOldData;
        }


        int           Width() const
        {
          return m_iWidth;
        }
        int           Height() const
        {
          return m_iHeight;
        }
        int           Depth() const
        {
          return m_iDepth;
        }

        void*         Data()
        {
          return m_pData;
        }

        const void* Data() const
        {
          return m_pData;
        }

        size_t        DataSize() const
        {
          return m_iWidth * m_iHeight * m_iDepth * sizeof( T );
        }

        void          Wrapping( bool bWrap = true )
        {
          m_bWrapping = bWrap;
        }
        bool          Wrapping() const
        {
          return m_bWrapping;
        }

        virtual inline void SetInvalidFeldValue( const T& FieldValue )
        {
          m_InvalidValue = FieldValue;
        }

    };
  };
};


#endif // THREEDLAYER_H