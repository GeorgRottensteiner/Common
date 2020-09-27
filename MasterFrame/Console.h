#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED



#include <list>
#include <GR/GRTypes.h>

#include <String\Tokenizer.h>



class Console
{

  protected:

    std::list<GR::String>             m_listConsoleLines;
                                      
    int                               m_iLastToken,
                                      m_setT,
                                      m_incT,
                                      m_decT,
                                      m_eraseT,
                                      m_toggleT,
                                      m_quitT,
                                      m_helpT,
                                      m_soundT,
                                      m_playT,
                                      m_stopT,
                                      m_loopT,
                                      m_infoT;

    GR::Tokenizer::Tokenizer          m_Tokenizer;


  public:


    Console();
    virtual ~Console();

    void                              ConsolePrint( const char *szText, ... );
    int                               RegisterSymbol( const char *szToken );

    virtual void                      ProcessCommand( const char *szCommand );

    virtual void                      ProcessCommandTokens( GR::Tokenizer::TokenSequence& m_TokenSequence, GR::String& strCommand );

};



#endif // CONSOLE_H_INCLUDED