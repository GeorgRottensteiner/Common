#ifndef _PJ_ministring_h
#define _PJ_ministring_h

#include <string.h>

namespace mini
{
#pragma pack(1)

/////////////////////////////////////////////////////////////////////
//@{ 
// eine platzsparende stringklasse.
// angelehnt an basic_string.
// benötigt nur 6 byte, wenn sie weniger als 5 Zeichen enthält.
// @author peter jerz
// @version 1.0
// @todo wie bei basic_string auch unicode ermöglichen. 
// @todo weitere operationen ermöglichen. 
//@} 
/////////////////////////////////////////////////////////////////////
  class string
{
private:
  union str_u_t      
  {
    char* pointer_;
    char  buf_[5];
  }; 
  str_u_t str_;       //@- 5 bytes, entweder buffer oder pointer 
  bool    use_i_buf_; //@- 1 byte, um zu klären, ob buffer oder pointer verwendet werden

public:

  //- structors

  string() : use_i_buf_( true )
  {
    *str_.buf_ = '\0';
  }

  ~string()
  {
    clear();
  }

  string( const string& rhs ) : use_i_buf_( true )
  {
    copy_from( rhs );
  }

  //@{ 
  // Konstruktion aus const char*.
  // @param rhs Zeiger auf eine Zeichenkette.
  // @author peter jerz
  //@} 
  explicit string( const char* rhs ) : use_i_buf_( true )
  {
    assign_( rhs, rhs + strlen( rhs ) );
  }

  //@{ 
  // Konstruktion aus einzelnem character.
  // @param rhs ein character.
  // @author peter jerz
  //@} 
  explicit string( char rhs ) : use_i_buf_( true )
  {
    assign_( &rhs, &rhs + 1 );
  }

  //@{ 
  // Zuweisungsoperator.
  // @author peter jerz
  //@} 
  string& operator=( const string& rhs )
  {
    return copy_from( rhs );;
  }
  
  //@{ 
  // Zuweisungsoperator von const char*.
  // @param rhs Zeiger auf eine Zeichenkette.
  // @author peter jerz
  //@} 
  string& operator=( const char* rhs )
  {
    return copy_from( string( rhs ) );
  }
  
  //@{ 
  // Zuweisungsoperator von einzelnem character.
  // @param rhs ein character.
  // @author peter jerz
  //@} 
  string& operator=( char rhs )
  {
    return copy_from( string( rhs ) );
  }
  
  //@{ 
  // Bereichszuweisung.
  // @param first Beginn des Quellbereichs.
  // @param last Ende (+1) des Quellbereichs.
  // @author peter jerz
  //@} 
  template<typename IT>
  void assign( IT first, IT last )
  {
    clear();
    assign_( first, last );
  }

  //@{ 
  // Löschen der Zeichenkette.
  // @author peter jerz
  //@} 
  void clear()
  {
    if ( !use_i_buf_ )
      delete[] str_.pointer_;

    use_i_buf_ = true;
    *str_.buf_ = '\0';
  }

  //@{ 
  // Umwandeln in const char*.
  // @returns Beginn der Zeichenkette als const char*.
  // @author peter jerz
  //@} 
  const char* c_str() const
  {
    return use_i_buf_? str_.buf_ : str_.pointer_;
  }

  //@{ 
  // Länge der Zeichenkette.
  // @returns Länge der Zeichenkette ohne Endnull
  // @author peter jerz
  //@} 
  unsigned int length() const
  {
    return strlen( use_i_buf_? str_.buf_ : str_.pointer_ );
  }

  //@{ 
  // Test, ob String leer ist.
  // @returns true, falls Stringlänge 0 ist.
  // @author peter jerz
  //@} 
  bool empty() const
  {
    return *( use_i_buf_? str_.buf_ : str_.pointer_ ) == '\0';
  }

  //@{ 
  // Test auf Ungleichheit.
  // @returns true, falls Strings verschieden.
  // @author peter jerz
  //@} 
  bool operator!=( const string& rhs ) const
  {
    return 0 != strcmp( c_str(), rhs.c_str() );
  }

  //@{ 
  // Test auf Gleichheit.
  // @returns true, falls Strings gleich.
  // @author peter jerz
  //@} 
  bool operator==( const string& rhs ) const
  {
    return !operator!=( rhs );
  }

  //@{ 
  // Kleiner-Test.
  // @param rhs String, mit dem verglichen wird.
  // @returns true, falls String kleiner rhs
  // @author peter jerz
  //@} 
  bool operator<( const string& rhs ) const
  {
    return 0 > strcmp( c_str(), rhs.c_str() );
  }

protected:

  //@{ 
  // Interne Kopierfunktion.
  // @param rhs String, von dem kopiert wird.
  // @returns *this
  // @author peter jerz
  //@} 
  string& copy_from( const string& rhs )
  {
    if ( this != &rhs )
    {
      clear();  

      if ( !rhs.empty() )
      {
        if ( rhs.use_i_buf_ )
        {
          //- internen buffer kopieren
          use_i_buf_ = true;
          memcpy( &str_, &rhs.str_, sizeof( str_u_t ) );
        }
        else
        {
          //- externen buffer kopieren
          assign_( rhs.str_.pointer_, rhs.str_.pointer_ + strlen( rhs.str_.pointer_ ) );
        }
      }
    }
    return *this;
  }

  //@{ 
  // Interne Bereichszuweisung.
  // @param first Beginn des Quellbereichs.
  // @param last Ende (+1) des Quellbereichs.
  // @author peter jerz
  //@} 
  template<typename IT>
  void assign_( IT first, IT last )
  {
    char* it;
    int   len = last - first ;

    if ( len < 5 )
    {
      //- internen buffer verwenden
      use_i_buf_ = true;
      it = str_.buf_;
    }
    else
    {
      //- externen buffer verwenden
      use_i_buf_ = false;
      it = str_.pointer_ = new char[ len + 1 ];;
    }

    for ( ; first != last; ++first, ++it )
      *it = *first;
    *it = '\0';
  }

};



#pragma pack()
}



#endif//_PJ_ministring_h
