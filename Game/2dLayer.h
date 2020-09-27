#ifndef ZWEIDLAYER_H
#define ZWEIDLAYER_H



#include <list>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Gamebase
  {

    template <typename T> class TileLayer2D
    {

      protected:

        T               m_InvalidValue;

        mutable T       m_TempInvalid;

        int             m_Width,
                        m_Height;

      public:

        bool            m_WrappingX,
                        m_WrappingY,
                        m_ClampTop,
                        m_ClampLeft,
                        m_ClampRight,
                        m_ClampBottom;

        T*              m_pData;


        TileLayer2D() :
          m_pData( NULL ),
          m_Width( 0 ),
          m_Height( 0 ),
          m_InvalidValue( T() ),
          m_TempInvalid( T() ),
          m_WrappingX( false ),
          m_WrappingY( false ),
          m_ClampTop( false ),
          m_ClampLeft( false ),
          m_ClampBottom( false ),
          m_ClampRight( false )
        {
        }

        TileLayer2D( GR::u32 Width, GR::u32 Height ) :
          m_pData( NULL ),
          m_Width( Width ),
          m_Height( Height ),
          m_InvalidValue( T() ),
          m_TempInvalid( T() ),
          m_WrappingX( false ),
          m_WrappingY( false ),
          m_ClampTop( false ),
          m_ClampLeft( false ),
          m_ClampBottom( false ),
          m_ClampRight( false )
        {
          InitFeld( Width, Height );
          ResetField( m_InvalidValue );
        }

        //- Copy-Construktor
        TileLayer2D( const TileLayer2D& t2lCopyFrom )
        {
          if ( &t2lCopyFrom == this )
          {
            return;
          }

          m_Width           = t2lCopyFrom.m_Width;
          m_Height          = t2lCopyFrom.m_Height;
          m_InvalidValue    = t2lCopyFrom.m_InvalidValue;
          m_TempInvalid     = t2lCopyFrom.m_TempInvalid;
          m_WrappingX       = t2lCopyFrom.m_WrappingX;
          m_WrappingY       = t2lCopyFrom.m_WrappingY;
          m_ClampTop        = t2lCopyFrom.m_ClampTop;
          m_ClampLeft       = t2lCopyFrom.m_ClampLeft;
          m_ClampBottom     = t2lCopyFrom.m_ClampBottom;
          m_ClampRight      = t2lCopyFrom.m_ClampRight;

          m_pData           = new T[m_Width * m_Height];
          for ( int i = 0; i < m_Width * m_Height; ++i )
          {
            m_pData[i] = t2lCopyFrom.m_pData[i];
          }
        }

        virtual ~TileLayer2D()
        {
          if ( m_pData != NULL )
          {
            delete[] m_pData;
            m_pData = NULL;
          }
        }

        TileLayer2D& operator= ( const TileLayer2D& t2lCopyFrom )
        {
          if ( this == &t2lCopyFrom )
          {
            return *this;
          }

          if ( m_pData != NULL )
          {
            delete[] m_pData;
            m_pData = NULL;
          }

          m_Width           = t2lCopyFrom.m_Width;
          m_Height          = t2lCopyFrom.m_Height;
          m_InvalidValue    = t2lCopyFrom.m_InvalidValue;
          m_TempInvalid     = t2lCopyFrom.m_TempInvalid;
          m_WrappingX       = t2lCopyFrom.m_WrappingX;
          m_WrappingY       = t2lCopyFrom.m_WrappingY;
          m_ClampTop        = t2lCopyFrom.m_ClampTop;
          m_ClampLeft       = t2lCopyFrom.m_ClampLeft;
          m_ClampBottom     = t2lCopyFrom.m_ClampBottom;
          m_ClampRight      = t2lCopyFrom.m_ClampRight;

          m_pData           = new T[m_Width * m_Height];
          for ( int i = 0; i < m_Width * m_Height; ++i )
          {
            m_pData[i] = t2lCopyFrom.m_pData[i];
          }

          return *this;
        }



        void Clear()
        {
          if ( m_pData != NULL )
          {
            delete[] m_pData;
            m_pData = NULL;
          }

          m_Width  = 0;
          m_Height = 0;
        }



        virtual void  InitFeld( GR::u32 Width, GR::u32 Height )
        {
          if ( m_pData != NULL )
          {
            delete[] m_pData;
            m_pData = NULL;
          }

          m_Width            = Width;
          m_Height           = Height;

          m_pData = new T[m_Width * m_Height];
          ResetField( m_InvalidValue );
        }



        virtual void  ResetField( const T& FillValue )
        {
          if ( m_pData == NULL )
          {
            return;
          }
          for ( int i = 0; i < m_Width * m_Height; i++ )
          {
            m_pData[i] = FillValue;
          }
        }



        void CalcTrueCoordinates( int& X, int& Y ) const
        {
          if ( ( m_ClampLeft )
          &&   ( X < 0 ) )
          {
            X = 0;
          }
          if ( ( m_ClampRight )
          &&   ( X >= m_Width ) )
          {
            X = m_Width - 1;
          }
          if ( ( m_ClampTop )
          &&   ( Y < 0 ) )
          {
            Y = 0;
          }
          if ( ( m_ClampBottom )
          &&   ( Y >= m_Height ) )
          {
            Y = m_Height - 1;
          }
          if ( m_WrappingX )
          {
            while ( X < 0 )
            {
              X += m_Width;
            }
            while ( X >= m_Width )
            {
              X -= m_Width;
            }
          }
          if ( m_WrappingY )
          {
            while ( Y < 0 )
            {
              Y += m_Height;
            }
            while ( Y >= m_Height )
            {
              Y -= m_Height;
            }
          }
        }



        virtual void  SetField( int X, int Y, const T& FieldValue )
        {
          if ( m_pData == NULL )
          {
            return;
          }
          CalcTrueCoordinates( X, Y );
          if ( ( X < 0 )
          ||   ( Y < 0 )
          ||   ( X >= m_Width )
          ||   ( Y >= m_Height ) )
          {
            // auﬂerhalb
            return;
          }
          m_pData[X + Y * m_Width] = FieldValue;
        }



        bool IsOutside( int X, int Y )
        {
          CalcTrueCoordinates( X, Y );
          if ( ( m_WrappingX )
          &&   ( m_WrappingY ) )
          {
            return false;
          }
          if ( ( ( X < 0 )
          ||     ( X >= m_Width ) )
          &&   ( !m_WrappingX ) )
          {
            return true;
          }
          if ( ( ( Y < 0 )
          ||     ( Y >= m_Height ) )
          &&   ( !m_WrappingY ) )
          {
            return true;
          }
          return false;
        }



        virtual T& Field( int X, int Y )
        {
          if ( m_pData == NULL )
          {
            // das originale Invalid soll nicht ver‰ndert werden
            m_TempInvalid = m_InvalidValue;
            return m_TempInvalid;
          }
          if ( ( m_Width == 0 )
          ||   ( m_Height == 0 ) )
          {
            m_TempInvalid = m_InvalidValue;
            return m_TempInvalid;
          }
          CalcTrueCoordinates( X, Y );
          if ( ( X < 0 )
          ||   ( Y < 0 )
          ||   ( X >= m_Width )
          ||   ( Y >= m_Height ) )
          {
            // auﬂerhalb
            // das originale Invalid soll nicht ver‰ndert werden
            m_TempInvalid = m_InvalidValue;
            return m_TempInvalid;
          }
          return m_pData[X + Y * m_Width];
        }



        virtual const T& Field( int X, int Y ) const
        {
          if ( m_pData == NULL )
          {
            // das originale Invalid soll nicht ver‰ndert werden
            m_TempInvalid = m_InvalidValue;
            return m_TempInvalid;
          }
          if ( ( m_Width == 0 )
          ||   ( m_Height == 0 ) )
          {
            m_TempInvalid = m_InvalidValue;
            return m_TempInvalid;
          }
          CalcTrueCoordinates( X, Y );
          if ( ( X < 0 )
          ||   ( Y < 0 )
          ||   ( X >= m_Width )
          ||   ( Y >= m_Height ) )
          {
            // auﬂerhalb
            // das originale Invalid soll nicht ver‰ndert werden
            m_TempInvalid = m_InvalidValue;
            return m_TempInvalid;
          }
          return m_pData[X + Y * m_Width];
        }



        virtual void  Resize( int Width, int Height )
        {
          if ( ( Width <= 0 )
          ||   ( Height <= 0 )
          ||   ( ( Width == m_Width )
          &&     ( Height == m_Height ) ) )
          {
            return;
          }

          T*   pOldData = m_pData;

          m_pData = new T[Width * Height];

          int   oldWidth = m_Width,
                oldHeight = m_Height;

          m_Width    = Width;
          m_Height   = Height;
          ResetField( m_InvalidValue );

          int   copyWidth = oldWidth,
                copyHeight = oldHeight;

          if ( copyWidth > m_Width )
          {
            copyWidth = m_Width;
          }
          if ( copyHeight > m_Height )
          {
            copyHeight = m_Height;
          }

          for ( int i = 0; i < copyWidth; i++ )
          {
            for ( int j = 0; j < copyHeight; j++ )
            {
              m_pData[i + j * m_Width] = pOldData[i + j * oldWidth];
            }
          }
          delete[] pOldData;
        }



        int Width() const
        {
          return m_Width;
        }
        
        
        
        int Height() const
        {
          return m_Height;
        }



        void Wrapping( bool Wrap = true )
        {
          m_WrappingX = Wrap;
          m_WrappingY = Wrap;
        }



        void Wrapping( bool WrapX, bool WrapY )
        {
          m_WrappingX = WrapX;
          m_WrappingY = WrapY;
        }



        bool WrappingX() const
        {
          return m_WrappingX;
        }



        bool WrappingY() const
        {
          return m_WrappingY;
        }



        void Clamp( bool ClampLeft = true, bool ClampTop = true, bool ClampRight = true, bool ClampBottom = true )
        {
          m_ClampTop    = ClampTop;
          m_ClampLeft   = ClampLeft;
          m_ClampBottom = ClampBottom;
          m_ClampRight  = ClampRight;
        }



        bool ClampTop() const
        {
          return m_ClampTop;
        }



        bool ClampLeft() const
        {
          return m_ClampLeft;
        }



        bool ClampRight() const
        {
          return m_ClampRight;
        }



        bool ClampBottom() const
        {
          return m_ClampBottom;
        }



        virtual void InvalidFeldValue( const T& FieldValue )
        {
          m_InvalidValue = FieldValue;
          m_TempInvalid  = m_InvalidValue;
        }



        const T& InvalidFeldValue()
        {
          return m_InvalidValue;
        }



        size_t DataSize() const
        {
          return m_Width * m_Height * sizeof( T );
        }



        void* Data() const
        {
          return m_pData;
        }



        void ShiftY( int X1, int X2, int DY, const T& fillValue = T() )
        {
          for ( int X = X1; X <= X2; ++X )
          {
            if ( DY > 0 )
            {
              for ( int Y = Height() - 1; Y >= 0; --Y )
              {
                if ( Y >= DY )
                {
                  SetField( X, Y, Field( X, Y - DY ) );
                }
                else
                {
                  SetField( X, Y, fillValue );
                }
              }
            }
            else if ( DY < 0 )
            {
              for ( int Y = 0; Y < Height(); ++Y )
              {
                if ( Y < Height() + DY )
                {
                  SetField( X, Y, Field( X, Y - DY ) );
                }
                else
                {
                  SetField( X, Y, fillValue );
                }
              }
            }
          }
        }



        void ShiftX( int Y1, int Y2, int DX, const T& fillValue = T() )
        {
          for ( int Y = Y1; Y <= Y2; ++Y )
          {
            if ( DX > 0 )
            {
              for ( int X = Width() - 1; X >= 0; --X )
              {
                if ( X >= DX )
                {
                  SetField( X, Y, Field( X - DX, Y ) );
                }
                else
                {
                  SetField( X, Y, fillValue );
                }
              }
            }
            else if ( DX < 0 )
            {
              for ( int X = 0; X < Width(); ++X )
              {
                if ( X < Width() + DX )
                {
                  SetField( X, Y, Field( X - DX, Y ) );
                }
                else
                {
                  SetField( X, Y, fillValue );
                }
              }
            }
          }
        }



        GR::u32 ReplaceTile( const T& OldValue, const T& NewValue )
        {
          GR::u32     ReplacedTileCount = 0;
          for ( int i = 0; i < Width(); i++ )
          {
            for ( int j = 0; j < Height(); j++ )
            {
              if ( Field( i, j ) == OldValue )
              {
                SetField( i, j, NewValue );
                ++ReplacedTileCount;
              }
            }
          }
          return ReplacedTileCount;
        }

    };

  }
}

#endif // ZWEIDLAYER_H