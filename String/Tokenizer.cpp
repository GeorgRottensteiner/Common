#include <algorithm>
#include <string>

#include <String/StringUtil.h>
#include <String\Tokenizer.h>


#include <debug/debugclient.h>



namespace GR
{
  namespace Tokenizer
  {
    Tokenizer::Symbol::Symbol( const GR::String& Text ) 
      : m_String( Text ),
        m_Type( TokenType::NIL ),
        m_NumRegistrations( 0 )
    {}

    Tokenizer::Symbol::Symbol( const GR::String& Text, const Token::tTokenType type )
      : m_String( Text ),
        m_Type( type ),
        m_NumRegistrations( 0 )
    {}

    Tokenizer::Symbol::Symbol( const Tokenizer::Symbol& rhs ) 
    :	m_String( rhs.m_String ), 
      m_Type( rhs.m_Type ), 
      m_NumRegistrations( rhs.m_NumRegistrations )
    {}

    Tokenizer::Symbol& Tokenizer::Symbol::operator=( const Tokenizer::Symbol& rhs ) 
    {
      if ( this == &rhs ) return *this;
      m_String   = rhs.m_String;
      m_Type       = rhs.m_Type;
      m_NumRegistrations  = rhs.m_NumRegistrations;
	    return *this;
    }

    Tokenizer::Symbol::~Symbol()
    {}

    inline bool Tokenizer::Symbol::operator<(  const Tokenizer::Symbol &rhs ) const 
    { return m_String < rhs.m_String; }

    inline bool Tokenizer::Symbol::operator<=( const Tokenizer::Symbol &rhs ) const 
    { return m_String <=  rhs.m_String; }

    inline bool Tokenizer::Symbol::operator>(  const Tokenizer::Symbol &rhs ) const
    { return m_String > rhs.m_String; } 

    inline bool Tokenizer::Symbol::operator>=( const Tokenizer::Symbol &rhs ) const
    { return m_String >=  rhs.m_String; }

    inline bool Tokenizer::Symbol::operator==( const Tokenizer::Symbol &rhs ) const
    { return m_String ==  rhs.m_String; }

    inline bool Tokenizer::Symbol::operator!=( const Tokenizer::Symbol &rhs ) const
    { return m_String !=  rhs.m_String;; }




    void Tokenizer::InitPredefinedSymbols()
    {
      if ( !m_setSymbol.empty() ) return;

      SetSymbol( TokenType::OPEN_PARENTHESES,       Symbol( "(" ) );
      SetSymbol( TokenType::CLOSE_PARENTHESES,      Symbol( ")" ) );
      SetSymbol( TokenType::OPEN_SQUARE_BRACKETS,   Symbol( "[" ) );
      SetSymbol( TokenType::CLOSE_SQUARE_BRACKETS,  Symbol( "]" ) );
      SetSymbol( TokenType::OPEN_BRACES,            Symbol( "{" ) );
      SetSymbol( TokenType::CLOSE_BRACES,           Symbol( "}" ) );
      SetSymbol( TokenType::COLON,                  Symbol( ":" ) );
      SetSymbol( TokenType::DOT,                    Symbol( "." ) );
      SetSymbol( TokenType::PLUS,                   Symbol( "+" ) );
      SetSymbol( TokenType::MINUS,                  Symbol( "-" ) );
      SetSymbol( TokenType::SEMICOLON,              Symbol( ";" ) );
      SetSymbol( TokenType::COMMA,                  Symbol( "," ) );
      SetSymbol( TokenType::STAR,                   Symbol( "*" ) );
      SetSymbol( TokenType::SLASH,                  Symbol( "/" ) );
      SetSymbol( TokenType::PIPE,                   Symbol( "|" ) );
      SetSymbol( TokenType::HASH,                   Symbol( "#" ) );
      SetSymbol( TokenType::EQUAL,                  Symbol( "=" ) );
      SetSymbol( TokenType::LESS,                   Symbol( "<" ) );
      SetSymbol( TokenType::GREATER,                Symbol( ">" ) );
      SetSymbol( TokenType::NOT,                    Symbol( "!" ) );
    }



    Tokenizer::Tokenizer()
    {
      m_bAllowEscape = true;
      InitPredefinedSymbols();
    }



    Tokenizer::Tokenizer( const Tokenizer& rhs ) 
      : m_setSymbol( rhs.m_setSymbol ),
        m_bAllowEscape( rhs.m_bAllowEscape )
    {
    }



    Tokenizer& Tokenizer::operator=( const Tokenizer& rhs ) 
    {
      if ( this == &rhs ) return *this;

      m_setSymbol = rhs.m_setSymbol;
      return *this;
    }

    Tokenizer::~Tokenizer()
    {
    }

    const Tokenizer::SymbolSet& Tokenizer::Symbols() const
    { 
      return m_setSymbol; 
    }



    Token::tTokenType Tokenizer::SetSymbol( const Token::tTokenType& t, const Tokenizer::Symbol& symbol )  
    { 
      SymbolSet::iterator it = m_setSymbol.find( symbol );
      if ( it != m_setSymbol.end() )
      {
        ++it->m_NumRegistrations;
        return it->Type();
      }

      Symbol symTemp( symbol );
      symTemp.m_Type = t;
      symTemp.m_NumRegistrations = 1;
      m_setSymbol.insert( symTemp ); 
      return symTemp.m_Type;
    }



    Token::tTokenType Tokenizer::registerSymbol( const Tokenizer::Symbol& symbol )  
    { 
      SymbolSet::iterator it( m_setSymbol.find( symbol ) ), itend( m_setSymbol.end() );
      if ( it != itend )
      {
        ++it->m_NumRegistrations;
        return it->Type();
      }

      //- freie Tokennummer finden (BAUSTELLE, da ineffizient)
      Token::tTokenType t = (Token::tTokenType)0;
      it = m_setSymbol.begin();
      for ( ; it != itend; ++it )
        if ( t < it->Type() ) 
          t = it->Type(); 

      Symbol symTemp( symbol );
      t = ( Token::tTokenType)( (int)( t + 1 ) );
      symTemp.m_Type = t;
      symTemp.m_NumRegistrations = 1;
      m_setSymbol.insert( symTemp ); 
      return symTemp.m_Type;
    }



    void Tokenizer::RemoveComments( TokenSequence& tokens ) const
    {
      TokenSequence strippedTokens;
      TokenSequence::iterator tsi( tokens.begin() );

      while ( tsi != tokens.end() )
      {
        switch ( tsi->Type() )
        {
          case TokenType::COMMENT:
            break;
          default:
            strippedTokens.push_back( *tsi );
            break;
        }

        ++tsi;
      }

      strippedTokens.swap( tokens );
    }



    void Tokenizer::RemoveCR( TokenSequence& tokens ) const
    {
      TokenSequence strippedTokens;
      TokenSequence::iterator tsi( tokens.begin() ), tsiend( tokens.end() );
      for ( ; tsi != tsiend; ++tsi )
      {
        if ( tsi->Type() != TokenType::LINE_BREAK )
        {
          strippedTokens.push_back( *tsi );
        }
      }
      strippedTokens.swap( tokens );
    }



    bool Tokenizer::IsIdentifier( const char Character ) const 
    {
      if ( ( GR::Strings::IsAlphaNumeric( Character ) )
      ||   ( Character == '_' ) )
      {
        return true;
      }
      return false;
    }



    TokenSequence Tokenizer::Tokenize( const GR::String& sequence ) const
    {
	    Token           token;
      TokenSequence   tokens;

	    //- Jeden char untersuchen
	    for( unsigned int i = 0; i < sequence.length(); ++i )
	    {
		    token.m_Type        = TokenType::NIL;
		    token.m_String      = "";
        token.m_PosInLine   = i;

        ////////////////////////////////////////////////
        //- Tabulator und Space überspringen
        ////////////////////////////////////////////////
		    if ( ( sequence[i] == ' ' )
        ||   ( sequence[i] == '\t' ) )
        {
          token.m_Type    = TokenType::WHITE_SPACE;
          token.m_String  = sequence[i];
          tokens.push_back( token );
          continue;
        }
        ////////////////////////////////////////////////
		    //- Anführungsstriche behandeln (Stringkonstante)
        ////////////////////////////////////////////////
		    else if ( sequence[i] == '\"' ) 
		    {
			    ++i;
			    token.m_Type     = TokenType::STRING;
			    token.m_String = "";

          while ( ( i < sequence.length() )
          &&      ( sequence[i] != '\"' ) ) //- alles vor dem zweiten " in den Tokenstring
          {
		        if ( ( sequence[i] == 13 )
            ||   ( sequence[i] == 10 ) ) //- Newline
		        {
              //- Fehler! Zeilenumbruch innerhalb einer Konstanten!
              dh::Log( "Tokenizer: Fehler! Zeilenumbruch innerhalb einer Konstanten!" );
              dh::Log( "\"%s\"...", token.m_String.c_str() );

              //- versuchen, das Beste draus zu machen
              token.m_Type = TokenType::UNKNOWN;
              break;
            }
            else if ( sequence[i] != '\\' ) 
            {
              //- kein ESC
				      token.m_String += sequence[i++]; 
            }
            else
            {
              if ( m_bAllowEscape )
              {
                ++i;
                if ( i < sequence.length() )
                {
                  switch ( sequence[i] )
                  {
                    case 'n' : 
                      token.m_String += '\n';         
                      break;
                    case 'r' : 
                      token.m_String += '\r';         
                      break;
                    case 't' : 
                      token.m_String += '\t';         
                      break;
                    case '\\': 
                      token.m_String += '\\';         
                      break;
                    case '\"': 
                      token.m_String += '\"';         
                      break;
                    default:   
                      token.m_String +=  sequence[i]; 
                      break;
                  }
                  ++i;
                }
              }
              else
              {
                token.m_String += sequence[i++]; 
              }
            }
          }
          tokens.push_back( token );
		    }
		    else if ( ( sequence[i] == '/' )
        &&        ( sequence[i + 1] == '/' ) ) 
        { 
          // SingleLine-Comment
			    token.m_Type    = TokenType::COMMENT;
			    token.m_String  = "";

          while ( ( i < sequence.length() ) 
          &&      ( sequence[i] != 13 ) 
          &&      ( sequence[i] != 10 ) )
          {
            token.m_String += sequence[i++];
          }
      
          tokens.push_back( token );

          if ( ( i > 0 )
          &&   ( i < sequence.length() )
          &&   ( ( sequence[i] == 13 )
          ||     ( sequence[i] == 10 ) ) )
          {
            --i; //- echt?
          }
		    }
		    else if ( ( sequence[i] == '/' )
        &&        ( sequence[i + 1] == '*' ) )
        { 
          //- MultiLine-Comment
			    token.m_Type    = TokenType::COMMENT;
			    token.m_String  = "";
          while ( ( i < sequence.length() )
          &&      ( !( sequence[i] == '*' && sequence[i + 1] == '/' ) ) )
          {
            token.m_String += sequence[i++];
          }
          token.m_String += "*/";
			    tokens.push_back( token );
          ++i;
		    }
		    else if ( ( sequence[i] == 13 )
        &&        ( sequence[i + 1] == 10 ) )
		    {
			    token.m_Type    = TokenType::LINE_BREAK;
          token.m_String  = "\n"; 
			    tokens.push_back( token );
          ++i;
		    }
		    else if ( ( sequence[i] == '\n' )
        ||        ( sequence[i] == 0x0A ) )
		    {
			    token.m_Type = TokenType::LINE_BREAK;
          token.m_String = "\n"; 
			    tokens.push_back( token );
		    }
		    else if ( GR::Strings::IsNumeric( sequence[i] ) ) 
        { 
          //- beginnend nur mit dezimalpunkt ist z.zt. nicht mehr erlaubt!
			    token.m_Type    = TokenType::INT;
			    token.m_String  = "";
			    //while the character is within bounds and is either a 
			    //digit or decimal point...
			    while( i < sequence.length() && ( GR::Strings::IsNumeric( sequence[i] ) || sequence[i] == '.' ) )
			    {
				    if( sequence[i] == '.' ) token.m_Type = TokenType::REAL;
				    token.m_String += sequence[i];
				    ++i;
			    }
			    --i;   //Need to decrement it because in the for loop, i will be incremented
			    if     ( token.m_Type == TokenType::INT )   token.m_Integer   = atoi( token.m_String.c_str() );
			    else if( token.m_Type == TokenType::REAL )  token.m_dbReal = atof( token.m_String.c_str() );

	        //if invalid decimal number is given
    // 			if( token.m_Type == Tokens::realT && token.m_dbReal == 0.0 ) token.m_Type = Tokens::errorT;
  	      tokens.push_back( token );
		    }
		    else 
        {
          if ( ( i < sequence.length() )
          &&   ( IsIdentifier( sequence[i] ) ) )
			    {
            //- Identifier sammeln
            token.m_Type = TokenType::LABEL;
            while ( ( i < sequence.length() )
            &&      ( IsIdentifier( sequence[i] ) ) )
            {
              token.m_String += sequence[i++];
            }
            if ( ( i < sequence.length() )
            &&   ( sequence[i] != ' ' ) )
            {
              // what about TAB?
              --i;
            }

            SymbolSet::const_iterator sti(    m_setSymbol.find( Symbol( token.m_String ) ) ), 
                                      stiend( m_setSymbol.end() );
            if ( sti != stiend )
			      {
					    token.m_Type     = sti->Type();
					    token.m_String = sti->String();
			      }
            tokens.push_back( token );
			    }
          else
		      {
            //- Symbole behandeln
            SymbolSet::const_reverse_iterator sti( m_setSymbol.rbegin() ), stiend( m_setSymbol.rend() );
            for ( ; sti != stiend; ++sti )
			      {
              GR::String strTemp( sequence, i, sti->String().length() );
				      if( strTemp != sti->String() ) continue;
					    token.m_Type     = sti->Type();
					    token.m_String = sti->String();
					    i += (int)sti->String().length() - 1;
					    tokens.push_back( token );
					    break;
			      }
		      }
        }

        // handle error
		    if ( token.m_Type == TokenType::NIL )
		    {
          if ( i < sequence.length() )
          {
            token.m_String = sequence[i];
          }
          token.m_Type = TokenType::UNKNOWN;
			    tokens.push_back( token );
		    }
	    }
      return tokens;
    }



    bool Tokenizer::Replace( TokenSequence::iterator first,
                             TokenSequence::iterator last,
                             const Token& oldToken,
                             const Token& newToken )
    {
      bool bResult = false;
      for ( ; first != last; ++first )
      {
        if ( first->Type()   != oldToken.Type()   ) continue;
        if ( first->String() != oldToken.String() ) continue;
        *first = newToken;
        bResult = true;
      }
      return bResult;
    }



    bool Tokenizer::IsTokenOfType( const GR::String& s, const Token::tTokenType& t ) const
    {
      TokenSequence tokens( Tokenize( s ) );
      if ( tokens.size()          != 1  ) return false;
      if ( tokens.front().Type()  != t  ) return false;

      return true;
    }

  }

}

