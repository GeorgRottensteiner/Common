#ifndef _PJ_AI_H_INCLUDED
#define _PJ_AI_H_INCLUDED

#pragma once
#pragma warning ( disable:4786 )
#include <string>

#include <PJLib\DesignPatterns\Observer.h>

namespace PJ
{
  namespace AI
  {
    class CAI : public CObserver
    {
    public:
      //- constructor
      CAI() : CObserver() {}

      //- copy constructor
      CAI( const CAI& rhs ) : CObserver( rhs ) {}

      //- assignment operator
      CAI& operator=( const CAI& rhs )
      {
        if ( this == &rhs ) return *this;
        CObserver::operator=( rhs );
        return *this;
      }

      //- destructor (virtual)
      virtual ~CAI() {}




      /////////////////////////////////////////////////////////////////////
      //-
      //-    MessageIdle
      //-
      /////////////////////////////////////////////////////////////////////
      virtual GR::String StatementIdle( const long id )
      {
        string strTemp;
        switch( id )
        {
        case 0: strTemp   = "Wann geht's los?"; break;
        case 1: strTemp   = "Macht mal hin!"; break;
        case 2: strTemp   = "Dum di dum."; break;
        case 3: strTemp   = "Hallo?"; break;
        case 4: strTemp   = "Ihr könnt mich Rummenigge nennen."; break;
        case 5: strTemp   = "Ihr könnt mich Georg nennen."; break;
        case 6: strTemp   = "Ihr könnt mich Jochen nennen."; break;
        case 7: strTemp   = "Ihr könnt mich Al nennen."; break;
        case 8: strTemp   = "Hier ist ja gar nichts los..."; break;
        case 9: strTemp   = "Wer fehlt denn noch?"; break;
        case 10: strTemp  = "Wollt ihr mal meine Waden fühlen?"; break;
        case 11: strTemp  = "Genug gelabert!"; break;
        case 12: strTemp  = "Ich glaub ich geh wieder."; break;
        case 13: strTemp  = "Ich dachte, hier kann man Skat spielen."; break;
        case 14: strTemp  = "Hat mal jemand ein Snickers für mich?"; break;
        case 15: strTemp  = "Ich bin ein Mensch, keine künstliche Intelligenz."; break;
        case 16: strTemp  = "Ich bin eine künstliche Intelligenz, kein Mensch."; break;
        case 17: strTemp  = ":-)"; break;
        case 18: strTemp  = "Aaanfangen!"; break;
        case 19: strTemp  = "Und nun?"; break;
        case 20: strTemp  = "Komme gleich wieder. Habe mein Trikot vergessen."; break;
        case 21: strTemp  = "Ihr könnt mich " + name() + " nennen."; break;
        case 22: strTemp  = "Hallo, ich bin " + name() + "."; break;
        case 23: strTemp  = name() + " hören gefährliche Stille."; break;
        case 24: strTemp  = "Schönes Wetter heute."; break;
        case 25: strTemp  = "..."; break;
        case 26: strTemp  = "Hallo, ich bin " + RandomName() + "."; break;
        case 27: strTemp  = "Bots sind doof!"; break;
        case 28: strTemp  = "Christian ist doof!"; break;
        }
        return strTemp;
      }

      GR::String StatementBegruessung( const long id )
      {
        string strTemp;
        switch( id )
        {
        case 0: strTemp = "Hallo!";                   break;
        case 1: strTemp = "Hi!";                      break;
        case 2: strTemp = "Wie geht's?" ;             break;
        case 3: strTemp = "Tach auch!";               break;
        case 4: strTemp = "Na du!" ;                  break;
        case 5: strTemp = "Guten Tag!" ;              break;
        case 6: strTemp = "Schön, dich zu sehen!" ;   break;
        }
        return strTemp;
      }

      GR::String StatementFreu( const long id )
      {
        string strTemp;
        switch( id )
        {
        case 0: strTemp   = "*hurra!*"; break;
        case 1: strTemp   = "Bravo!"; break;
        case 2: strTemp   = "Lobet den Herrn!"; break;
        case 3: strTemp   = "He he he!"; break;
        case 4: strTemp   = "Halleluja!"; break;
        case 5: strTemp   = ":-)"; break;
        case 6: strTemp   = "Johl!"; break;
        case 7: strTemp   = "/me kichert."; break;
        }
        return strTemp;
      }

      GR::String StatementHeul( const long id )
      {
        string strTemp;
        switch( id )
        {
        case 0: strTemp   = "O nein!"; break;
        case 1: strTemp   = "Buuuuuh!"; break;
        case 2: strTemp   = "Rache!"; break;
        case 3: strTemp   = "*heul*"; break;
        case 4: strTemp   = name() + " sehr, sehr betrübt."; break;
        case 5: strTemp   = "Raaaaaaahhhh!"; break;
        case 6: strTemp   = ":-("; break;
        case 7: strTemp   = "/me schmollt."; break;
        }
        return strTemp;
      }

      string RandomName( const long id )
      {
        string strTemp;
        switch ( id )
        {
          case 0:   strTemp =  "Mr. Roboto";  break;
          case 1:   strTemp =  "Amigo";       break;
          case 2:   strTemp =  "Mc. Intusch"; break;
          case 3:   strTemp =  "Maik Rosoft"; break;
          case 4:   strTemp =  "HAL 9000";    break;
          case 5:   strTemp =  "MCP";         break;
          case 6:   strTemp =  "Jochi";       break;
          case 7:   strTemp =  "Hoshi";       break;
          case 8:   strTemp =  "Peer";        break;
          case 9:   strTemp =  "Okolonzo";    break;
          case 10:  strTemp =  "R2D2";        break;
          case 11:  strTemp =  "C3PO";        break;
          case 12:  strTemp =  "Franz";       break;
          case 13:  strTemp =  "Terminator";  break;
          case 14:  strTemp =  "Niemand";     break;
          case 15:  strTemp =  "Mutter";      break;
          case 16:  strTemp =  "Vatter";      break;
          case 17:  strTemp =  "VC20";         break;
          case 18:  strTemp =  "Sky-Net";     break;
          case 19:  strTemp =  "6502";        break;
          case 20:  strTemp =  "KI";          break;
          case 21:  strTemp =  "Botmaster";   break;
          case 22:  strTemp =  "Deep Thought";break;
          case 23:  strTemp =  "Conan";       break;
          case 24:  strTemp =  "Wadenbeisser";break;
          case 25:  strTemp =  "Geronimo";    break;
          case 26:  strTemp =  "ZX81";        break;
          case 27:  strTemp =  "Kachelator";  break;
          case 28:  strTemp =  "Ripley";       break;
          case 29:  strTemp =  "Chingachgook"; break;
          case 30:  strTemp =  "Freeman";       break;
          case 31:  strTemp =  "Rebit";       break;
          case 32:  strTemp =  "Shepard";       break;
          case 33:  strTemp =  "Roderick";       break;
          case 34:  strTemp =  "Star Billard";       break;
          case 35:  strTemp =  "Tonto";       break;
          case 36:  strTemp =  "AxMan";       break;
          case 37:  strTemp =  "MS-DOS 6.2";       break;
          case 38:  strTemp =  "chrissian";       break;
          case 39:  strTemp =  "Christian";       break;
        }
        return strTemp;
      }

      GR::String name() const { return m_strName; }
      virtual void name( const GR::String& Name ) { m_strName = Name; }


    protected:
    private:
      GR::String m_strName;
    };


  };
};

#endif//_PJ_AI_H_INCLUDED
