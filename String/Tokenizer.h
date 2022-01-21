#ifndef TOKENIZER_H
#define TOKENIZER_H

#pragma once
#include <string>
#include <set>
#include <list>
#include <utility>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Tokenizer
  {
    namespace TokenType
    {
      enum Value
      {
        NIL                     =   0, //  nix
        LABEL                   =   1, //  Identifier
        INT                     =   2, //  Integer
        REAL                    =   3, //  Fliesskommazahl, double
        STRING                  =   4, //  Text in Anführungszeichen
        UNKNOWN                 =   5, //  ein Fehler: unbekannt, aber kein Identifier
        LINE_BREAK              =   6, //  Zeilenende
        COMMENT                 =   7, //  Kommentar
                                    
        OPEN_PARENTHESES        =   8,  //  (
        CLOSE_PARENTHESES       =   9,  //  )
        OPEN_SQUARE_BRACKETS    =   10, //  [
        CLOSE_SQUARE_BRACKETS   =   11, //  ]
        OPEN_BRACES             =   12, //  {
        CLOSE_BRACES            =   13, //  }
        COLON                   =   14, //  :
        DOT                     =   15, //  .
        PLUS                    =   16, //  +
        MINUS                   =   17, //  -
        SEMICOLON               =   18, //  ;
        COMMA                   =   19, //  ,
        STAR                    =   20, //  *
        SLASH                   =   21, //  /
        PIPE                    =   22, //  |
        HASH                    =   23, //  #
                                     
        EQUAL                   =   24, //  =
        LESS                    =   25, //  <
        GREATER                 =   26, //  >
        NOT                     =   27, //  !
        WHITE_SPACE             =   28  // blank or tab
      };
    }

    class Tokenizer;

    class Token  
    {
        friend class Tokenizer;

      public:
      
        typedef TokenType::Value tTokenType;


        Token() : 
          m_Type( TokenType::NIL ), 
          m_Integer( 0 ),
          m_PosInLine( 0 )
        {
        }



        Token( const Token& rhs ) 
          : m_Type( rhs.m_Type ), 
            m_String( rhs.m_String ), 
            m_Integer( 0 ),
            m_PosInLine( rhs.m_PosInLine )
        {
          if ( m_Type == TokenType::INT )
          {
            m_Integer = rhs.m_Integer;
          }
          else if ( m_Type == TokenType::REAL )
          {
            m_dbReal = rhs.m_dbReal;
          }
        }
    


        explicit Token( const GR::i64 lValue )           : m_Type( TokenType::INT ),    m_String( "" ),       m_Integer( lValue )   {}
        explicit Token( const double& dValue )        : m_Type( TokenType::REAL ),   m_String( "" ),       m_dbReal( dValue ) {}
        explicit Token( const GR::String& strValue )  : m_Type( TokenType::STRING ), m_String( strValue ), m_Integer( 0 )        {}
    
        //- Achtung, kann Unsinn erzeugen!!!!!!!!!!!
        Token( const GR::String& strValue, const tTokenType& type ) : m_Type( type ), m_String( strValue ), m_Integer( 0 ), m_PosInLine( 0 ) {}

        virtual ~Token() 
        {
        }



        Token& operator =( const Token& rhs ) 
        {
          if ( this == &rhs ) return *this;
          m_Type      = rhs.m_Type;
          m_String    = rhs.m_String;
          m_Integer      = 0;
          m_PosInLine = rhs.m_PosInLine;
          if ( m_Type == TokenType::INT )
          {
            m_Integer = rhs.m_Integer;
          }
          else if ( m_Type == TokenType::REAL )
          {
            m_dbReal = rhs.m_dbReal;
          }
          return *this;
        }



        bool operator ==( const Token& rhs ) const {
          if ( this == &rhs ) return true;
          if ( m_Type != rhs.m_Type ) return false;
          if ( m_String != rhs.m_String ) return false;
          return true;
        }

        bool operator !=( const Token& rhs ) const {
          return !operator==( rhs );
        }

        //- Kleiner-Operator - wichtig für Benutzung in Maps und zum Sortieren!
        bool operator <( const Token& rhs ) const {
          if ( m_Type < rhs.m_Type ) return true;
          if ( m_String < rhs.m_String ) return true;
          return false;
        }

	      inline tTokenType     Type()    const { return m_Type;     }
        inline GR::String     String()  const { return m_String; }
	      inline GR::i64        Int()     const { return m_Integer;      }
	      inline double         Real()    const { return m_dbReal;    }
	  
        //- Nur mit Vorsicht verwenden! (Zur Nachbearbeitung von Tokensequenzen)
        inline void Type( const tTokenType& type )   { m_Type = type;  }
        inline void String( const GR::String& s  )  { m_String = s; }
	      inline void Int( const GR::i64& i )            { m_Integer = i;      }
	      inline void Real( const double& d  )        { m_dbReal = d;    }
        inline int PosInLine() const { return m_PosInLine; }

        //- Au ha! Riskant!
        inline operator tTokenType() const { return m_Type; }



      protected:

        tTokenType    m_Type;
	      GR::String    m_String;
        int           m_PosInLine;
        union 
        { 
          GR::i64 m_Integer; 
          double  m_dbReal; 
        };
    };

    typedef std::list< Token > TokenSequence;
    typedef std::pair< TokenSequence::iterator, TokenSequence::iterator > TokenSequenceDelimiter;



    class Tokenizer
    {
      public:

        class Symbol  
        {
          friend class Tokenizer;

        public:
          Symbol( const GR::String& astring );
          Symbol( const GR::String& astring, const Token::tTokenType type );
          Symbol( const Symbol& rhs );
          Symbol& operator=( const Symbol& rhs );
          virtual ~Symbol();

	        inline Token::tTokenType Type()    const { return m_Type;     }
          inline GR::String       String()  const { return m_String; }
          inline bool operator<(  const Symbol &rhs ) const;
          inline bool operator<=( const Symbol &rhs ) const;
          inline bool operator>(  const Symbol &rhs ) const;
          inline bool operator>=( const Symbol &rhs ) const;
          inline bool operator==( const Symbol &rhs ) const;
          inline bool operator!=( const Symbol &rhs ) const;

        protected:
          GR::String         m_String;
          Token::tTokenType   m_Type;
          mutable long        m_NumRegistrations;
        };

        typedef std::set< Symbol > SymbolSet;

      public:

        Tokenizer();
        Tokenizer( const Tokenizer& rhs );
        Tokenizer& operator=( const Tokenizer& rhs );
        virtual ~Tokenizer();

        virtual Token::tTokenType registerSymbol( const Symbol& symbol );

        const SymbolSet& Symbols() const;

        virtual TokenSequence Tokenize( const GR::String& sequence ) const;

        virtual void RemoveCR( TokenSequence& tokens ) const;
        virtual void RemoveComments( TokenSequence& tokens ) const;

        void AllowEscape( bool bAllow = true )
        {
          m_bAllowEscape = bAllow;
        }


        //- einige Hilfsfunktionen
        static bool Replace(  TokenSequence::iterator first,
                              TokenSequence::iterator last,
                              const Token& oldToken,
                              const Token& newToken );
        //- und Tests
        bool IsTokenOfType( const GR::String &s, const Token::tTokenType& t ) const;



      protected:

        virtual Token::tTokenType   SetSymbol( const Token::tTokenType& t, const Symbol& symbol );
        virtual void                InitPredefinedSymbols();
        SymbolSet                   m_setSymbol;
        bool                        m_bAllowEscape;

        bool                        IsIdentifier( const char Character ) const;
  
    };

  }
}

#endif // TOKENIZER_H