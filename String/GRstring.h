#ifndef GR_STRING_H
#define GR_STRING_H


#include <locale>
#include <iostream>

#include <OS/OS.h>

#include <string>

#include <string.h>

#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn( 262 )
#endif



namespace GR
{

  class string
  {

    private:

      char*     m_pBuffer;

      size_t    m_BufferLength;
      size_t    m_Length;


    public:


      const static size_t  npos = (size_t)-1;


      string() :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        ResizeBuffer( 0 );
      }



      ~string()
      {
        if ( m_pBuffer != NULL )
        {
          delete[] m_pBuffer;
          m_pBuffer = NULL;
        }
      }



      string( const GR::string& rhs ) :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        copy_from( rhs );
      }



      string( const GR::string& rhs, size_t Offset ) :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        operator=( rhs.substr( Offset ) );
      }



      string( const GR::string& rhs, size_t Offset, size_t Length ) :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        operator=( rhs.substr( Offset, Length ) );
      }



      string( const char* rhs ) :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        if ( rhs != NULL )
        {
          assign( rhs, strlen( rhs ) );
        }
        else
        {
          clear();
        }
      }



      string( const char* rhs, size_t Length ) :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        assign( rhs, Length );
      }



      string( const std::string& rhs ) :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        assign( rhs.c_str(), rhs.length() );
      }



      string( char rhs ) :
        m_pBuffer( NULL ),
        m_BufferLength( 0 ),
        m_Length( 0 )
      {
        assign( &rhs, 1 );
      }



      void resize( size_t NewSize )
      {
        if ( NewSize == m_Length )
        {
          return;
        }
        if ( NewSize < m_Length )
        {
          char* pNew = new char[NewSize + 1];
          memcpy( pNew, m_pBuffer, NewSize );
          pNew[NewSize] = 0;

          delete[] m_pBuffer;
          m_pBuffer       = pNew;
          m_Length        = NewSize;
          m_BufferLength  = NewSize;
          return;
        }

        // Grow and fill with zeroes
        char* pNew = new char[NewSize + 1];
        memcpy( pNew, m_pBuffer, m_Length );
        memset( pNew + m_Length, 0, NewSize - m_Length + 1 );

        delete[] m_pBuffer;
        m_pBuffer       = pNew;
        m_Length        = NewSize;
        m_BufferLength  = NewSize;
      }



      void ResizeBuffer( size_t NewSize )
      {
        /*
        if ( NewSize == m_BufferLength )
        {
          return;
        }*/

        char* pTemp = new char[NewSize + 1];
        *pTemp          = 0;

        size_t  copySize = m_Length;
        if ( copySize > NewSize )
        {
          copySize = NewSize;
        }

        if ( m_pBuffer != NULL )
        {
          memcpy( pTemp, m_pBuffer, copySize );
        }
        pTemp[copySize] = 0;
        if ( NewSize < m_Length )
        {
          m_Length = NewSize;
        }
        m_BufferLength  = NewSize;

        delete[] m_pBuffer;
        m_pBuffer = pTemp;
      }



      // Zuweisungsoperator.
      GR::string& operator=( const GR::string& rhs )
      {
        return copy_from( rhs );
      }



      // Zuweisungsoperator von const char*.
      GR::string& operator=( const char* rhs )
      {
        if ( rhs == m_pBuffer )
        {
          return *this;
        }
        assign( rhs, strlen( rhs ) );
        return *this;
      }



      // Zuweisungsoperator von einzelnem character.
      GR::string& operator=( const char rhs )
      {
        return copy_from( GR::string( rhs ) );
      }



#if OPERATING_SYSTEM != OS_TANDEM
      GR::string& operator=( GR::string&& rhs )
      {
        if ( m_pBuffer != NULL )
        {
          delete[] m_pBuffer;
        }
        m_pBuffer       = rhs.m_pBuffer;
        m_BufferLength  = rhs.m_BufferLength;
        m_Length        = rhs.m_Length;

        rhs.m_pBuffer       = NULL;
        rhs.m_Length        = 0;
        rhs.m_BufferLength  = 0;

        return *this;
      }
#endif



      // Löschen der Zeichenkette.
      void clear()
      {
        ResizeBuffer( 0 );
      }



      void erase( size_t Offset, size_t Count )
      {
        if ( ( Offset >= m_Length )
        ||   ( Offset + Count > m_Length ) )
        {
          // TODO!!
          return;
        }
        char*   pTemp = new char[m_Length - Count + 1];

        if ( Offset > 0 )
        {
          memcpy( pTemp, m_pBuffer, Offset );
        }
        if ( Offset + Count < m_Length )
        {
          memcpy( pTemp + Offset, m_pBuffer + Offset + Count, m_Length - Offset - Count );
        }
        pTemp[m_Length - Count] = 0;

        delete[] m_pBuffer;
        m_pBuffer      = pTemp;
        m_Length       = m_Length - Count;
        m_BufferLength = m_Length;
      }



      void erase()
      {
        clear();
      }



      void release()
      {
        delete[] m_pBuffer;
        m_pBuffer       = NULL;
        m_Length        = 0;
        m_BufferLength  = 0;
      }



      // Umwandeln in const char*.
      const char* c_str() const
      {
        return m_pBuffer;
      }



      const char* data() const
      {
        return m_pBuffer;
      }



      std::string str() const
      {
        return std::string( m_pBuffer, m_Length );
      }



      size_t length() const
      {
        return m_Length;
      }



      const GR::string& Trim( const char TrimChar = ' ' )
      {
        size_t    startPos = find_first_not_of( TrimChar );
        size_t    pos = find_last_not_of( TrimChar );

        if ( ( startPos == npos )
        &&   ( pos == npos ) )
        {
          if ( ( !empty() )
          &&   ( m_pBuffer[0] == TrimChar ) )
          {
            clear();
          }
          return *this;
        }
        *this = substr( startPos, pos - startPos + 1 );
        return *this;
      }



      GR::string& Replace( const char ToRemove, const char ReplaceWith )
      {
        for ( size_t i = 0; i < m_Length; ++i )
        {
          if ( m_pBuffer[i] == ToRemove )
          {
            m_pBuffer[i] = ReplaceWith;
          }
        }
        return *this;
      }



      GR::string& Replace( const GR::string& ToRemove, const GR::string& ReplaceWith )
      {
        GR::string      result;
        size_t          lastFindPos = 0;
        size_t          findPos = npos;


        while ( ( findPos = find( ToRemove, lastFindPos ) ) != npos )
        {
          if ( findPos > lastFindPos )
          {
            // Ein Schnipsel vor dem Fundort
            result.append( m_pBuffer + lastFindPos, findPos - lastFindPos );
          }
          result += ReplaceWith;
          lastFindPos = findPos + ToRemove.length();
        }
        if ( lastFindPos < length() )
        {
          result.append( m_pBuffer + lastFindPos, m_Length - lastFindPos );
        }

        *this = result;
        return *this;
      }




      GR::string& ToUpper()
      {
        char*   pDummy = m_pBuffer;
        for ( size_t i = 0; i < m_Length; ++i )
        {
          *pDummy = std::toupper( *pDummy, std::locale() );
          ++pDummy;
        }
        return *this;
      }



      GR::string& ToLower()
      {
        char*   pDummy = m_pBuffer;
        for ( size_t i = 0; i < m_Length; ++i )
        {
          *pDummy = std::tolower( *pDummy, std::locale() );
          ++pDummy;
        }
        return *this;
      }



      // Test, ob String leer ist.
      bool empty() const
      {
        return m_Length == 0;
      }



      bool operator!=( const string& rhs ) const
      {
        if ( m_Length != rhs.m_Length )
        {
          return true;
        }
        return memcmp( m_pBuffer, rhs.m_pBuffer, m_Length ) != 0;
      }



      bool operator==( const GR::string& rhs ) const
      {
        return !operator!=( rhs );
      }



      bool operator<( const GR::string& rhs ) const
      {
        return 0 > strcmp( c_str(), rhs.c_str() );
      }



      bool operator<=( const GR::string& rhs ) const
      {
        return 0 >= strcmp( c_str(), rhs.c_str() );
      }



      bool operator>=( const GR::string& rhs ) const
      {
        return 0 <= strcmp( c_str(), rhs.c_str() );
      }



      GR::string operator+( const GR::string& rhs ) const
      {
        GR::string  me( *this );

        me._append( rhs );
        return me;
      }



      GR::string operator+( char SingleChar ) const
      {
        GR::string  me( *this );

        me.append( SingleChar );
        return me;
      }



      GR::string& operator+=( const string& rhs )
      {
        _append( rhs );
        return *this;
      }



      GR::string& operator+=( const char* szBuffer )
      {
        if ( szBuffer == NULL )
        {
          return *this;
        }
        append( szBuffer, strlen( szBuffer ) );
        return *this;
      }



      void _append( const string& rhs )
      {
        if ( m_Length + rhs.length() + 1 > m_BufferLength )
        {
          // need to grow buffer
          size_t    newSize = m_BufferLength * 2;
          if ( newSize < m_Length + rhs.length() + 1 )
          {
            newSize = m_Length + rhs.length() + 1;
          }
          ResizeBuffer( newSize );
        }
        //if ( m_Length + rhs.length() + 1 <= m_BufferLength )
        {
          // can use existing buffer
          memcpy( m_pBuffer + m_Length, rhs.m_pBuffer, rhs.length() + 1 );
          m_Length += rhs.length();
          return;
        }
        /*
        dh::Log( "INTERNAL ERROR, GROWN BUFFER SIZE IS TOO SMALL!" );

        char*   pTemp = new char[m_Length + rhs.m_Length + 1];

        memcpy( pTemp, m_pBuffer, m_Length );
        memcpy( pTemp + m_Length, rhs.m_pBuffer, rhs.m_Length );
        pTemp[m_Length + rhs.m_Length] = '\0';

        delete[] m_pBuffer;
        m_pBuffer = pTemp;
        m_Length += rhs.m_Length;*/
      }



      void append( const string& rhs, size_t Offset, size_t Length )
      {
        if ( Offset >= rhs.length() )
        {
          return;
        }
        if ( Offset + Length > rhs.length() )
        {
          return;
        }
        append( rhs.m_pBuffer + Offset, Length );
      }



      void append( size_t Count, char SingleChar )
      {
        // TODO - Slot!
        for ( size_t i = 0; i < Count; ++i )
        {
          append( SingleChar );
        }
      }



      void append( char SingleChar )
      {
        append( &SingleChar, 1 );
      }



      void append( const char* pBuffer )
      {
        append( pBuffer, strlen( pBuffer ) );
      }



      void append( const char* szBuffer, size_t iLength )
      {
        if ( szBuffer == NULL )
        {
          return;
        }
        if ( iLength == 0 )
        {
          return;
        }

        if ( m_Length + iLength + 1 > m_BufferLength )
        {
          // need to grow buffer
          size_t    newSize = m_BufferLength * 2;
          if ( newSize < m_Length + iLength + 1 )
          {
            newSize = m_Length + iLength + 1;
          }
          ResizeBuffer( newSize );
        }
        // can use existing buffer
        memcpy( m_pBuffer + m_Length, szBuffer, iLength + 1 );
        m_Length += iLength;
        m_pBuffer[m_Length] = 0;
      }



      GR::string substr( size_t Offset ) const
      {
        if ( Offset >= m_Length )
        {
          return GR::string();
        }
        return GR::string( m_pBuffer + Offset, m_Length - Offset );
      }



      GR::string substr( size_t Offset, size_t Length ) const
      {
        if ( Length >= 100000000 )
        {
          // GR::String substr called with negative length?
          //dh::Log( "Length for substr suspiciously long!" );
        }

        if ( Offset >= m_Length )
        {
          return GR::string();
        }
        if ( Offset + Length > m_Length )
        {
          // TODO !! out of bounds!
          return GR::string( m_pBuffer, m_Length );
        }
        return GR::string( m_pBuffer + Offset, Length );
      }



      void reserve( size_t NewSize )
      {
        ResizeBuffer( NewSize );
      }



      char at( const size_t Index ) const
      {
        if ( Index >= m_Length )
        {
          // TODO! out of bounds!
          return 0;
        }
        return m_pBuffer[Index];
      }



      char& operator[] ( const size_t Index )
      {
        if ( Index >= m_Length )
        {
          // TODO! out of bounds!
          static char dummy;

          return dummy;
        }
        return m_pBuffer[Index];
      }



      const char& operator[] ( const size_t Index ) const
      {
        if ( Index >= m_Length )
        {
          // TODO! out of bounds!
          static char dummy;

          return dummy;
        }
        return m_pBuffer[Index];
      }



      size_t find_first_of( const GR::string& CharactersFind, size_t StartIndex = 0 ) const
      {
        char*     pPos = m_pBuffer + StartIndex;

        while ( *pPos != 0 )
        {
          if ( CharactersFind.find( *pPos ) != npos )
          {
            return (size_t)( pPos - m_pBuffer );
          }
          ++pPos;
        }
        return npos;
      }



      size_t find_first_not_of( char CharacterNotToFind ) const
      {
        char*     pPos = m_pBuffer;

        while ( *pPos != 0 )
        {
          if ( *pPos != CharacterNotToFind )
          {
            return (size_t)( pPos - m_pBuffer );
          }
          ++pPos;
        }
        return npos;
      }



      size_t find_first_not_of( const GR::string& CharactersNotToFind ) const
      {
        char*     pPos = m_pBuffer;

        while ( *pPos != 0 )
        {
          if ( CharactersNotToFind.find( *pPos ) == GR::string::npos )
          {
            return (size_t)( pPos - m_pBuffer );
          }
          ++pPos;
        }
        return npos;
      }



      size_t find_last_of( char CharacterNotToFind ) const
      {
        if ( m_Length == 0 )
        {
          return npos;
        }

        char*     pPos = m_pBuffer + m_Length - 1;

        while ( pPos != m_pBuffer )
        {
          if ( *pPos == CharacterNotToFind )
          {
            return (size_t)( pPos - m_pBuffer );
          }
          --pPos;
        }
        return npos;
      }



      size_t find_last_not_of( char CharacterNotToFind ) const
      {
        if ( m_Length == 0 )
        {
          return npos;
        }

        char*     pPos = m_pBuffer + m_Length - 1;

        while ( pPos >= m_pBuffer )
        {
          if ( *pPos != CharacterNotToFind )
          {
            return (size_t)( pPos - m_pBuffer );
          }
          --pPos;
        }
        return npos;
      }



      size_t find( char CharacterToFind ) const
      {
        return find( CharacterToFind, 0 );
      }



      size_t find( char CharacterToFind, size_t Offset ) const
      {
        if ( Offset >= m_Length )
        {
          return npos;
        }
        char*     pPos = m_pBuffer + Offset;

        while ( *pPos != 0 )
        {
          if ( *pPos == CharacterToFind )
          {
            return (size_t)( pPos - m_pBuffer );
          }
          ++pPos;
        }
        if ( *pPos == 0 )
        {
          return npos;
        }
        return (size_t)( pPos - m_pBuffer );
      }



      size_t find( const GR::string& SearchText ) const
      {
        return find( SearchText, 0 );
      }



      size_t find( const GR::string& SearchText, size_t Offset ) const
      {
        if ( Offset >= m_Length )
        {
          return npos;
        }
        if ( Offset + SearchText.m_Length > m_Length )
        {
          return npos;
        }

        for ( size_t i = 0; i <= m_Length - Offset - SearchText.m_Length; ++i )
        {
          if ( compare( Offset + i, SearchText.m_Length, SearchText ) == 0 )
          {
            return Offset + i;
          }
        }
        return npos;
      }



      size_t rfind( const GR::string& SearchText ) const
      {
        if ( SearchText.m_Length > m_Length )
        {
          return npos;
        }

        size_t    pos = m_Length - SearchText.m_Length;

        while ( true )
        {
          if ( compare( pos, SearchText.m_Length, SearchText ) == 0 )
          {
            return pos;
          }
          if ( pos == 0 )
          {
            break;
          }
          --pos;
        }
        return npos;
      }



      int compare( const GR::string& RHS ) const
      {
        return compare( 0, m_Length, RHS );
      }



      int compare( size_t Offset, size_t Length, const GR::string& RHS ) const
      {
        if ( Offset >= m_Length )
        {
          //dh::Log( "GR::string::compare - Offset out of bounds!" );
          return -2;
        }
        if ( Offset + Length > m_Length )
        {
          //dh::Log( "GR::string::compare - Offset + Length out of bounds!" );
          return -2;
        }

        if ( m_Length == 0 )
        {
          return 0;
        }
        size_t  compareLength = m_Length;
        if ( RHS.length() < compareLength )
        {
          compareLength = RHS.length();
        }

        for ( size_t i = 0; i < compareLength; ++i )
        {
          if ( m_pBuffer[Offset + i] < RHS.m_pBuffer[i] )
          {
            return -1;
          }
          else if ( m_pBuffer[Offset + i] > RHS.m_pBuffer[i] )
          {
            return 1;
          }
        }

        if ( Length < RHS.length() )
        {
          return -1;
        }
        if ( Length > RHS.length() )
        {
          return 1;
        }
        return 0;
      }



      bool StartsWith( const GR::string& Start ) const
      {
        if ( Start.length() > length() )
        {
          return false;
        }
        return ( compare( 0, Start.length(), Start ) == 0 );
      }



      bool EndsWith( const GR::string& End ) const
      {
        if ( End.length() > length() )
        {
          return false;
        }
        return ( compare( length() - End.length(), End.length(), End ) == 0 );
      }



      void assign( const char* szBuffer, size_t Length )
      {
        delete[] m_pBuffer;

        m_Length        = Length;
        m_pBuffer       = new char[m_Length + 1];
        m_BufferLength  = Length;
        if ( ( szBuffer != NULL )
        &&   ( m_Length > 0 ) )
        {
          memcpy( m_pBuffer, szBuffer, m_Length );
        }
        m_pBuffer[m_Length] = '\0';
      }



      void swap( GR::string& OtherString )
      {
        GR::string    temp( *this );
        OtherString = *this;
        *this = temp;
      }



      #if OPERATING_SYSTEM != OS_TANDEM
      explicit operator bool() const
      {
        return m_pBuffer != NULL;
      }
      #endif



    protected:

      string& copy_from( const string& rhs )
      {
        if ( this != &rhs )
        {
          release();

          m_Length        = rhs.m_Length;
          m_pBuffer       = new char[m_Length + 1];
          m_BufferLength  = m_Length;
          memcpy( m_pBuffer, rhs.m_pBuffer, m_Length + 1 );
        }
        return *this;
      }

  };

  static GR::string operator+( char SingleCharacter, const GR::string& rhs )
  {
    GR::string    singleChar;
    singleChar.append( SingleCharacter );
    singleChar._append( rhs );

    return singleChar;
  }



  static GR::string operator+( const char* pString, const GR::string& rhs )
  {
    GR::string    singleChar;
    singleChar.append( pString, strlen( pString ) );
    singleChar._append( rhs );

    return singleChar;
  }



  static bool operator> ( const GR::string& lhs, const GR::string& rhs )
  {
    return lhs.compare( rhs ) > 0;
  }

}



static std::ostream& operator<<( std::ostream& OutStream, const GR::string& Text )
{
  OutStream.rdbuf()->sputn( Text.c_str(), (int)Text.length() );

  return OutStream;
}



#endif//GR_STRING_H
