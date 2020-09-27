#ifndef _PJ_GRAMMAR_H
#define _PJ_GRAMMAR_H

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <iostream>
#include <string>
#include <sstream>

namespace PJ
{
  enum Deklination //- die ca. 16 Deklinationen im Deutschen, benannt nach beispielhaften Substantiven
  {
    e_deklinationBegin = 0,
    e_deklinationBild = 0 ,
    e_deklinationGlas,
    e_deklinationArm,
    e_deklinationGreis,
    e_deklinationStrahl,
    e_deklinationLappen,              // auch Infinitive
    e_deklinationMaler,
    e_deklinationUntertan,
    e_deklinationStudium,
    e_deklinationAuge,
    e_deklinationGenie,
    e_deklinationBaer,
    e_deklinationKnabe,
    e_deklinationTruebsal,
    e_deklinationBlume,
    e_deklinationFrau,
    e_deklinationEigenname,
    e_deklinationZimperliche,
    e_deklinationEnd
  };

  //- Kasi/Fälle ( Nominativ (wer?), Genitiv (wessen?), Dativ (wem?) und Akkusativ (wen?) )
  enum Kasus { e_kasusBegin = 0, e_kasusNominativ = 0, e_kasusGenitiv, e_kasusDativ, e_kasusAkkusativ, e_kasusEnd };
  //- Geni/Geschlechter (maskulin=männlich, feminin=weiblich, neutrum=neutral)
  enum Genus { e_genusBegin = 0, e_genusMaskulin = 0 , e_genusFeminin, e_genusNeutrum, e_genusEnd };
  //- Numeri/Anzahl (Singular=Einzahl, Plural=Mehrzahl)
  enum Numerus { e_numerusBegin = 0, e_numerusSingular = 0, e_numerusPlural, e_numerusEnd };
  //- Artikeltypen für Adjektive (DefArtikel=der,die,das,  IndefArtikel=einer,eine,ein)
  enum Artikeltyp { e_artikelBegin = 0, e_artikelKein = 0, e_artikelDef, e_artikelIndef, e_artikelEnd };


  inline GR::String Artikel( Kasus kas, Numerus num, Genus gen, Artikeltyp art )
  {
    static const GR::String alleArtikel[e_artikelEnd-1][e_kasusEnd][e_genusEnd+1] =
    {
      { //- bestimmter Artikel
        //-mask, fem,    neutr, plural
        { "der", "die",  "das", "die"  },    //- Nominativ
        { "des", "der",  "des", "der"  },    //- Genitiv
        { "dem", "der",  "dem", "den"  },    //- Dativ
        { "den", "die",  "das", "die"  }     //- Akkusativ
      },
      { //- unbestimmter Artikel
        //-mask,   fem,     neutr,   plural
        { "ein",   "eine",  "ein",   ""  },  //- Nominativ
        { "eines", "einer", "eines", ""  },  //- Genitiv
        { "einem", "einer", "einem", ""  },  //- Dativ
        { "einen", "eine",  "ein",   ""  }   //- Akkusativ
      }
    };

    if ( art != e_artikelDef && art != e_artikelIndef ) return "";

    if (num == e_numerusPlural)	return alleArtikel[art - 1][ kas ][ e_genusEnd ];
    else                        return alleArtikel[art - 1][ kas ][ gen ];
  }

  //Vergleichsgrad fuer Adjektive

  // enum Vergleichgrad { keinVergleich, Komparativ, Superlativ,
  // 											alleVergleichsgrade };




  // //nur Verben

  // enum Person		{ keinePerson = 0, erste, zweite, dritte,
  // 							allePersonen };
  // 																											//
  // enum Tempus   { keinTempus = 0, Praesens, Praeteritum, Perfekt,
  // 							alleTempi };
																											  //Zeiten
  // enum Indik		{ indikativ, subjektiv };
  // 																											//fuer indirekte Rede

  // // Konjugationen fuer Verben

  // enum Konjugation
  // { keineKonjugation = 0,
  // 	loben,
  // 	reden,
  // 	reisen,
  // 	fassen,
  // 	handeln,
  // 	starkeKonjugation,

  // 	alleKonjugationen
  // };






  //-------------------------Nomen

  // class SubstantivDeklinationen
  // {
  // public:
    // static GR::String Suffix( Deklination, Kasus,  Numerus );
  // };

  // class AdjektivDeklinationen   //enthaelt alle Endungen
  // {
  // public:
  // 	static GR::String Suffix( Kasus, Numerus, Genus, Artikeltyp );
  // };
  //--------------------------------
  // class VerbKonjugation
  // {
  // };

  // class StarkeVerbKonjugation : public VerbKonjugation
  // {
  // };
  // //-------------------------------------------------------------

  // class Wort
  // {
  // public:
  // 	virtual int Load() = 0;
    // virtual int BeginLoad(const GR::String& filename) = 0;
  // };
  //-------------------------------------------------------------

  // class Verb : public Wort
  // {
  // private:
  // 	VerbKonjugation *konjugation;
  // public:
  // 	void SetKonjugation( VerbKonjugation * );
  // 	VerbKonjugation *GetKonjugation() const;
  // 	virtual int Load() = 0;
  // 	virtual int BeginLoad(const GR::String& filename) = 0;
  // };
  //-------------------------------------------------------------

  // class Nomen : public Wort
  // {
  // 	virtual int Load() = 0;
  // 	virtual int BeginLoad(const GR::String& filename) = 0;
  // 	virtual GR::String Beuge( 	 Kasus,
  // 															 Numerus,
  // 															 Genus,
  // 															 Artikeltyp ) const = 0;
  // };
  //-----------------------------------------------------------

  class CSubstantiv /* : public Nomen */
  {
  private:
    Deklination m_eDeklination;
    Genus       m_eGenus;
    GR::String m_strStamm[ e_numerusEnd ];

  public:

    CSubstantiv() : m_eDeklination( e_deklinationBegin ), m_eGenus( e_genusBegin ) {}
    CSubstantiv( const CSubstantiv& rhs )
      : m_eDeklination( rhs.m_eDeklination ),
        m_eGenus(       rhs.m_eGenus )
    {
      for ( int i = e_numerusBegin; i < e_numerusEnd; ++i )
        m_strStamm[ i ] = rhs.m_strStamm[ i ];
    }

    CSubstantiv& operator=( const CSubstantiv& rhs )
    {
      if ( this != &rhs )
      {
        m_eDeklination  = rhs.m_eDeklination;
        m_eGenus        = rhs.m_eGenus;
        for ( int i = e_numerusBegin; i < e_numerusEnd; ++i )
          m_strStamm[ i ] = rhs.m_strStamm[ i ];
      }
      return *this;
    }

    CSubstantiv( const GR::String& stammSingular, //- substantiv aus stämmen, deklination u. genus bilden
                 const GR::String& stammPlural,
                 const Deklination  deklination,
                 const Genus        genus )
    : m_eDeklination( deklination ), m_eGenus( genus )
    {
      m_strStamm[ e_numerusSingular ] = stammSingular;
      m_strStamm[ e_numerusPlural   ] = stammPlural;
    }

    bool operator==( const CSubstantiv& rhs ) const
    {
      return ( m_strStamm[ e_numerusBegin ] != rhs.m_strStamm[ e_numerusBegin ] );
    }

    bool operator<( const CSubstantiv& rhs ) const
    {
      return ( m_strStamm[ e_numerusBegin ] < rhs.m_strStamm[ e_numerusBegin ] );
    }

    bool operator!=( const CSubstantiv& rhs ) const { return !operator==( rhs ); }


    inline void         stamm( const Numerus num, const GR::String& nom )  { m_strStamm[ num ] = nom; }
    inline GR::String  stamm( const Numerus num ) const                    { return m_strStamm[ num ]; }
    inline void         deklination( const Deklination dek )                { m_eDeklination = dek; }
    inline Deklination  deklination() const                                 { return m_eDeklination; }
    inline void         genus( const Genus gen )                            { m_eGenus = gen; }
    inline Genus        genus() const                                       { return m_eGenus; }

    inline GR::String Beuge( Kasus kas, Numerus num, Genus gen, Artikeltyp art ) const
    { return m_strStamm[ num ] + Suffix( kas, num ); }

    inline GR::String Beuge( Kasus kas, Numerus num ) const //- für substantive sind genus und artikeltyp schnurz, nur wegen adjektiv-kompatibilität
    { return m_strStamm[ num ] + Suffix( kas, num ); }

    GR::String Suffix( Kasus kas, Numerus num ) const
    {
      static const GR::String Suffixe[e_deklinationEnd][e_numerusEnd][e_kasusEnd] =
      { //- Singular                    Plural
        //- Nom Gen   Dat   Akk         Nom Gen Dat   Akk       Deklination:
        { { "", "s",  "",   ""    },  { "", "", "n",  ""  } },  //-  0  Bild ,
        { { "", "es", "",   ""    },  { "", "", "n",  ""  } },  //-  1  Glas,
        { { "", "s",  "",   ""    },  { "", "", "n",  ""  } },  //-  2  Arm,
        { { "", "es", "",   ""    },  { "", "", "n",  ""  } },  //-  3  Greis,
        { { "", "s",  "",   ""    },  { "", "", "",   ""  } },  //-  4  Strahl,
        { { "", "s",  "",   ""    },  { "", "", "",   ""  } },  //-  5  Lappen, (auch Infinitive)
        { { "", "s",  "",   ""    },  { "", "", "n",  ""  } },  //-  6  Maler,
        { { "", "s",  "",   ""    },  { "", "", "",   ""  } },  //-  7  Untertan,
        { { "", "s",  "",   ""    },  { "", "", "",   ""  } },  //-  8  Studium,
        { { "", "s",  "",   ""    },  { "", "", "",   ""  } },  //-  9  Auge,
        { { "", "s",  "",   ""    },  { "", "", "",   ""  } },  //- 10  Genie,
        { { "", "en", "en", "en"  },  { "", "", "",   ""  } },  //- 11  Baer
        { { "", "n",  "n",  "n"   },  { "", "", "",   ""  } },  //- 12  Knabe, (auch substantivierte Adjektive? z.B. das Gute)
        { { "", "",   "",   ""    },  { "", "", "n",  ""  } },  //- 13  Truebsal,
        { { "", "",   "",   ""    },  { "", "", "",   ""  } },  //- 14  Blume,
        { { "", "",   "",   ""    },  { "", "", "",   ""  } },  //- 15  Frau,
        { { "", "s",  "",   ""    },  { "", "", "",   ""  } },  //- 16  Eigenname,
        { { "", "n",  "n",  ""    },  { "", "", "",   ""  } }   //- 17  die Zimperliche, (auch substantivierte Adjektive(wenn feminin)? z.B. das Gute)
      };
      return Suffixe[m_eDeklination][num][kas];
    }

    inline friend std::ostream& operator<<( std::ostream& o, CSubstantiv& s )
    {
      o << s.stamm( e_numerusSingular ).c_str()  << " "
        << s.stamm( e_numerusPlural ).c_str()    << " "
        << s.deklination()              << " "
        << s.genus()                    << " ";
      return o;
    }

    inline friend std::istream& operator>>( std::istream& i, CSubstantiv& s )
    {
      long dekl, gen;
      i >> s.m_strStamm[ e_numerusSingular ] >> s.m_strStamm[ e_numerusPlural ] >> dekl >> gen;
      s.m_eDeklination = (Deklination)dekl;
      s.m_eGenus       = (Genus)gen;
      return i;
    }

  };
  //-------------------------------------------------------------


  class CAdjektiv /* : public Nomen */
  {
  private:
    GR::String m_strStamm;

  public:
    CAdjektiv() {}
    CAdjektiv( const CAdjektiv& rhs ) : m_strStamm( rhs.m_strStamm ) {}
    CAdjektiv& operator=( const CAdjektiv& rhs )
    {
      if ( this != &rhs ) m_strStamm = rhs.m_strStamm;
      return *this;
    }
    CAdjektiv( const GR::String& stamm ) : m_strStamm( stamm ) {} //- adjektiv aus string bilden

    inline void stamm( const GR::String& stamm ) {	m_strStamm = stamm;}
    inline GR::String stamm() const {	return m_strStamm; }

    inline GR::String Beuge( Kasus kas, Numerus num, Genus gen, Artikeltyp art ) const
    { return m_strStamm + Suffix( kas, num, gen, art ); }

    static inline GR::String Beuge( const GR::String& stamm, Kasus kas, Numerus num, Genus gen, Artikeltyp art )
    { return stamm + Suffix( kas, num, gen, art ); }

    static GR::String Suffix( Kasus kas, Numerus num, Genus gen, Artikeltyp art)
    {
      static const GR::String Suffixe[e_artikelEnd][e_kasusEnd][e_genusEnd+1] =
      {
        { //- ohne Artikel
          //-mask, fem, neutr, plural
          { "er", "e",  "es", "e"   },    //- Nominativ
          { "en", "er", "en", "er"  },    //- Genitiv
          { "em", "er", "em", "en"  },    //- Dativ
          { "en", "e",  "es", "e"   }     //- Akkusativ
        },
        { //- mit bestimmtem Artikel
          //-mask, fem, neutr, plural
          { "e",  "e",  "e",  "en"  },    //- Nominativ
          { "en", "en", "en", "en"  },    //- Genitiv
          { "en", "en", "en", "en"  },    //- Dativ
          { "en", "e",  "e",  "en"  }     //- Akkusativ
        },
        { //- mit unbestimmtem Artikel
          //-mask, fem, neutr, plural
          { "er", "e",  "es", "en"  },    //- Nominativ
          { "en", "en", "en", "en"  },    //- Genitiv
          { "en", "en", "en", "en"  },    //- Dativ
          { "en", "e",  "es", "en"  }     //- Akkusativ
        }
      };

      if (num == e_numerusPlural)	return Suffixe[art][ kas ][ e_genusEnd ];
      else                        return Suffixe[art][ kas ][ gen ];
    }

    inline friend std::ostream& operator<<( std::ostream& o, CAdjektiv& s )
    { o << s.m_strStamm.c_str() << " "; return o; }
    inline friend std::istream& operator>>( std::istream& i, CAdjektiv& s )
    { i >> s.m_strStamm; return i; }
  };

};


template < class ISTREAM >
ISTREAM& operator >>( ISTREAM& is, GR::String& str )
{
  char szTemp[5000];
  is >> szTemp;
  str = szTemp;
  return is;
}

template < class OSTREAM >
OSTREAM& operator <<( OSTREAM& os, GR::String& str )
{ os << str.c_str(); return os; }
#endif//_PJ_GRAMMAR_H
