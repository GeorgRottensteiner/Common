#ifndef _PJ_FILEPATH_H
#define _PJ_FILEPATH_H

// #include <utility>

#pragma warning ( disable : 4786 ) //- disable stupid warning
#include <sstream>
#include <list>
#include <iostream>
#include <fstream>

// #include <boost/tokenizer.hpp>
// #include <boost/utility.hpp>
#include <utility>



//- net paths like "\\spiderman\abc\def\ghi.txt" don't work (basic_path thinks that they are relative)

namespace PJ
{
  template < typename C_ > // C_ ist a char type (only tested with 'char')
  class basic_path
  {
  public:
    typedef C_ char_type;
    typedef std::basic_string< char > string_type;

  //- operations
    // basic_path& StripPath();
    // bool IsDirectory() const;

    basic_path& rename_extension( const string_type& ext );   //- replaces or appends extension
    basic_path& append( const basic_path& rhs );              //- appends another basic_path
    basic_path& make_relative_to( const basic_path& rhs );    //- makes relative to another basic_path
    basic_path& make_absolute_with( const basic_path& rhs );  //- makes absolute with another basic_path
    basic_path& remove_filespec();
    basic_path& strip_path();

    basic_path& normalize(); //- removes unnecessary "." and ".."
    basic_path& canonicalize() { return normalize(); }

    //- tests
    bool relative() const; //- returns false if head of basic_path contains a drive name (':')
    bool same_root( const basic_path& rhs ) const;
    bool file_exists() const;

    //- getters
    const char_type*   c_str() const;  //- return basic_path as char*
    const string_type& str()   const;  //- return basic_path as string

    string_type root()      const;  //- returns head of basic_path
    string_type filespec()  const;  //- returns tail of basic_path
    string_type extension() const;  //- returns extension part of filespec(), if any

  public:
    //- contructors
    basic_path();
    basic_path( const basic_path& rhs );               //- copy constructor
    explicit basic_path( const char_type*   p ); //- construct from char*
    explicit basic_path( const string_type& p ); //- construct from string

    ~basic_path() {} //- destructor is not virtual

  public:
    //- assignment
    basic_path& operator=( const basic_path& rhs );
    basic_path& operator=( const string_type& s );
    basic_path& operator=( const char_type* p );

    string_type separator() const { return separator_; }  //- default is "\"'
    void        separator( const string_type& s ) {  if ( separator_ != s ) { strrep_invalid_(); separator_ = s; } }

    //- todo: should be replaced by use of not_casesensitive char_traits
    bool ignore_case() const { return ignore_case_; }
    void ignore_case( bool s ) {  ignore_case_ = s; }

  //- internals
  private:
    typedef std::list< string_type > strlist;

    void construct_( const string_type& p   );  //- clear and create new
    void parse_(     const string_type& p   );  //- create all internals from a string
    void copy_(      const basic_path& rhs  );  //- helper for assignment and copy construction

    string_type separator_; //- default is "\"'
    strlist     path_;      //- contains the seperated directory levels

    mutable bool        ignore_case_;

    //- for 'casts' to string and char*
    mutable GR::String string_rep_;
    mutable bool        string_is_valid_; //- if false, the string representaion has to be rebuilt
    void update_string_rep_() const;
    void strrep_invalid_( bool v = true ) const { string_is_valid_ = !v; }

    /////////////////////////////////////////////////////////////////////
    //-    struct equal, not_equal
    /////////////////////////////////////////////////////////////////////
    struct equal /* : public std::not_equal_to<string_type>  */
    {
      bool ignore_case_;
      equal( bool ignore_case = true ) : ignore_case_( ignore_case ) {}
      bool operator()( const string_type& lhs, const string_type& rhs ) const
      {
        if ( !ignore_case_ )            return lhs != rhs;
        if ( lhs.size() != rhs.size() ) return false;
        string_type::const_iterator it( lhs.begin() ), itrhs( rhs.begin() );
        for ( ; it != lhs.end() && itrhs != rhs.end(); ++it, ++itrhs )
          if ( tolower( *it ) != tolower( *itrhs ) )
            return false;
        return true;
      }
    };

    struct not_equal /* : public std::not_equal_to<string_type>  */
    {
      bool ignore_case_;
      not_equal( bool ignore_case = true ) : ignore_case_( ignore_case ) {}
      bool operator()( const string_type& lhs, const string_type& rhs ) const
      {
        return !equal( ignore_case_ )( lhs, rhs );
      }
    };
  };



  /////////////////////////////////////////////////////////////////////
  //-    basic_path - inplementation
  /////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  //-    constructors
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >::basic_path()
  : separator_( string_type( "\\" ) ),
    string_is_valid_( true ),
    ignore_case_( true )
  {
  }

  //- copy constructor
  template < typename C_ >
  basic_path< C_ >::basic_path( const basic_path< C_ >& rhs )
  {
    copy_( rhs );
  }

  //- construct from char*
  template < typename C_ >
  basic_path< C_ >::basic_path( const char_type* p )
  : separator_( string_type( "\\" ) ),
    ignore_case_( true )
  {
    if ( p ) construct_( string_type( p  ) );
    else     construct_( string_type( "" ) );
  }

  //- construct from string
  template < typename C_ >
  basic_path< C_ >::basic_path( const string_type& p )
  : separator_( string_type( "\\" ) ),
    ignore_case_( true )
  {
    construct_( p );
  }

  /////////////////////////////////////////////////////////////////////
  //-    assignment operators
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::operator=( const basic_path< C_ >& rhs )
  {
    if ( this != &rhs ) copy_( rhs );
    return *this;
  }

  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::operator=( const string_type& p )
  {
    construct_( p );
    return *this;
  }

  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::operator=( const char_type* p )
  {
    if ( p ) construct_( string_type( p  ) );
    else     construct_( string_type( "" ) );
    return *this;
  }

  /////////////////////////////////////////////////////////////////////
  //-    helper functions for constuction and assignment
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  void basic_path< C_ >::copy_( const basic_path& rhs )
  {
    //- is it myself?
    if ( this == &rhs ) return;
    ignore_case_  = rhs.ignore_case_;
    separator_    = rhs.separator_;
    path_         = rhs.path_;
    string_rep_   = "";
    strrep_invalid_();
  }

  template < typename C_ >
  void basic_path< C_ >::construct_( const string_type& p )
  {
    ignore_case_ = true;
    parse_( p );
  }

  template < typename C_ >
  void basic_path< C_ >::parse_( const string_type& p )
  {
    //- parse basic_path from string
    path_.clear();

    // using namespace boost;
    // using namespace std;
    // char_delimiters_separator< char_type > cds( false, "", "\\/" );
    // tokenizer<> tok( p, cds );
    // for( tokenizer<>::iterator beg = tok.begin(); beg != tok.end(); ++beg )
      // path_.push_back( *beg );
    // using namespace std;
    // string cds( false, "", "\\/" );
    char_type cDiv = '\\';
    string_type::const_iterator itCurrent = p.begin();
    string_type::const_iterator itDivider = p.begin();
    do
    {
      itDivider = std::find( itCurrent, p.end(), cDiv );

      path_.push_back( string_type( itCurrent, itDivider ) );

      if ( itDivider != p.end() )
        itCurrent = ++itDivider;
    }
    while ( itDivider != p.end() );

    // for( tokenizer<>::iterator beg = tok.begin(); beg != tok.end(); ++beg )
      // path_.push_back( *beg );

    strrep_invalid_();
  }

  template < typename C_ >
  void basic_path< C_ >::update_string_rep_() const
  {
    //- build string representation for output and so on
    string_rep_ = string_type();
    strlist::const_iterator it( path_.begin() ), itend( path_.end() );
    if ( it != itend )
      string_rep_ += *it++;
    for ( ; it != itend; ++it )
      string_rep_ += separator_ + *it;
    strrep_invalid_( false );
  }

  /////////////////////////////////////////////////////////////////////
  //-    c_str - 'cast' to char*
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  const typename basic_path<C_ >::char_type* basic_path<C_>::c_str() const
  {
    return str().c_str();
  }

  /////////////////////////////////////////////////////////////////////
  //-    str
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  const typename basic_path< C_ >::string_type& basic_path< C_ >::str() const
  {
    if ( !string_is_valid_ ) update_string_rep_();
    return string_rep_;
  }

  /////////////////////////////////////////////////////////////////////
  //-    root
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  typename basic_path<C_>::string_type basic_path<C_>::root() const
  {
    return path_.empty() ? string_type( "" ) : path_.front();
  }

  /////////////////////////////////////////////////////////////////////
  //-    filespec
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  typename basic_path< C_ >::string_type basic_path< C_ >::filespec() const
  {
    return path_.empty() ? string_type( "" ) : path_.back();
  }

  /////////////////////////////////////////////////////////////////////
  //-    extension
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  typename basic_path< C_ >::string_type basic_path< C_ >::extension() const
  {
    if ( path_.empty() )               return string_type( "" );
    const string_type& file = path_.back();
    if ( file == "." || file == ".." ) return string_type( "" );
    string_type::allocator_type::size_type dotpos = file.find_last_of( '.' );
    if ( dotpos == string_type::npos ) return  string_type( "" );
    return string_type( file.begin() + dotpos, file.end() );
  }

/////////////////////////////////////////////////////////////////////
//-
//-    tests
//-
/////////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////////
  //-    relative
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  bool basic_path< C_ >::relative() const
  {
    if ( path_.empty() )                             return true;
    const string_type& root = path_.front();
    if ( root.empty() )                              return true;
    if ( root[root.size() - 1] != char_type( ':' ) ) return true;
    return false;
  }

  /////////////////////////////////////////////////////////////////////
  //-    same_root
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  bool basic_path< C_ >::same_root( const basic_path< C_ >& rhs ) const
  {
    //- is it myself?
    if ( this == &rhs )                       return true;
    if ( path_.empty() || rhs.path_.empty() ) return false;
    return equal( ignore_case_ )( path_.front(), rhs.path_.front() );
  }

  template < typename C_ >
  bool basic_path< C_ >::file_exists() const
  {
    return std::ifstream( c_str() )? true : false;
  }

/////////////////////////////////////////////////////////////////////
//-
//-    mutating
//-
/////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
  //-    remove_filespec
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::remove_filespec()
  {
    if ( !path_.empty() ) path_.pop_back();
    strrep_invalid_();
    return *this;
  }

  /////////////////////////////////////////////////////////////////////
  //-    strip_path
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::strip_path()
  {
    if ( path_.size() > 1 ) path_.pop_front();
    strrep_invalid_();
    return *this;
  }

  /////////////////////////////////////////////////////////////////////
  //-    make_relative_to
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::make_relative_to( const basic_path< C_ >& rhs )
  {
    if ( this == &rhs )                     return *this; //- making me relative to myself makes no sense
    if ( rhs.relative() )                   return *this; //- rhs must be absolute
    if ( !relative() && !same_root( rhs ) ) return *this; //- if I am absolute, then my root has to be same like rhs's root

    std::pair <strlist::const_iterator, strlist::const_iterator> res
      = std::mismatch( path_.begin(), path_.end(), rhs.path_.begin(), equal( ignore_case_ ) );

    strlist temp( (strlist::const_iterator)res.first, (strlist::const_iterator)path_.end() );

    if ( res.second == rhs.path_.end() )
      temp.push_front( "." );
    else
      for ( ; res.second != rhs.path_.end(); ++res.second )
        temp.push_front( ".." );

    path_.swap( temp );

    strrep_invalid_();
    return *this;
  }

  /////////////////////////////////////////////////////////////////////
  //-    make_absolute_with
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::make_absolute_with( const basic_path< C_ >& rhs )
  {
    if ( this == &rhs ) return *this;
    if ( !relative() ) return *this;
    operator=( basic_path( rhs ).append( *this ) );
    normalize();
    return *this;
  }

  /////////////////////////////////////////////////////////////////////
  //-    rename_extension
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  typename basic_path< typename C_ >& basic_path< typename C_ >::rename_extension( const typename basic_path< typename C_ >::string_type& ext )
  {
    if ( path_.empty() )               return *this;
    string_type& file = path_.back();
    if ( file == "." || file == ".." ) return *this;
    string_type::allocator_type::size_type dotpos = file.find_last_of( '.' );
    if ( dotpos == string_type::npos )
      file += ext; //- append an extension
    else
      file = string_type( file.begin(), file.begin() + dotpos ) + ext; //- replace an extension
    strrep_invalid_();
    return *this;
  }

  /////////////////////////////////////////////////////////////////////
  //-    append
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::append( const basic_path< C_ >& rhs )
  {
    path_.insert( path_.end(), rhs.path_.begin(), rhs.path_.end() );
    strrep_invalid_();
    return *this;
  }

  /////////////////////////////////////////////////////////////////////
  //-    normalize
  /////////////////////////////////////////////////////////////////////
  template < typename C_ >
  basic_path< C_ >& basic_path< C_ >::normalize()
  {
    strlist::iterator it( path_.begin() );
    while( it != path_.end() )
    {
      if ( *it == "." )  //- handle "."
        it = path_.erase( it );
      else if ( *it == ".." )   //- handle ".."
      {
        if ( it != path_.begin() )
          // path_.erase( boost::prior( it ) );
          path_.erase( prior( it ) );
        it = path_.erase( it );
      }
      else
        ++it;
    }

    strrep_invalid_();
    return *this;
  }

  typedef basic_path< char > path;

};//end of namespace PJ

#endif//_PJ_FILEPATH_H