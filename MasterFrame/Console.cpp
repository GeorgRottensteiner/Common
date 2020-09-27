#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten

#include <MasterFrame/Console.h>

#include <debug\debugclient.h>



Console::Console()
{

  // Tokenizer anwerfen
  m_setT      = RegisterSymbol( "set" );
  m_incT      = RegisterSymbol( "inc" );
  m_decT      = RegisterSymbol( "dec" );
  m_eraseT    = RegisterSymbol( "erase" );
  m_toggleT   = RegisterSymbol( "toggle" );
  m_quitT     = RegisterSymbol( "quit" );
  m_helpT     = RegisterSymbol( "help" );
  m_soundT    = RegisterSymbol( "sound" );
  m_playT     = RegisterSymbol( "play" );
  m_stopT     = RegisterSymbol( "stop" );
  m_loopT     = RegisterSymbol( "loop" );
  m_infoT     = RegisterSymbol( "info" );

}



Console::~Console()
{

}



void Console::ConsolePrint( const char* szText, ... )
{
  char          szDummy[3000];

#ifdef vsprintf_s
  vsprintf_s( szDummy, 3000, szText, (char *)( &szText + 1 ) );
#elif ( ( OPERATING_SYSTEM == OS_ANDROID ) || ( OPERATING_SYSTEM == OS_WEB ) )
  va_list args;
  va_start( args, szText );
  vsprintf( szDummy, szText, args );
  va_end( args ); 
#else
  vsprintf_s( szDummy, 3000, szText, (char *)( &szText + 1 ) );
#endif
  m_listConsoleLines.push_back( szDummy );

  if ( m_listConsoleLines.size() >= 50 )
  {
    m_listConsoleLines.pop_front();
  }
}



int Console::RegisterSymbol( const char *szToken )
{
  return m_Tokenizer.registerSymbol( GR::Tokenizer::Tokenizer::Symbol( szToken ) );
}



void Console::ProcessCommand( const char *szCommand )
{
  if ( !szCommand )
  {
    return;
  }
  GR::Tokenizer::TokenSequence                 m_TokenSequence;

  m_TokenSequence = m_Tokenizer.Tokenize( szCommand );

  GR::String     command( szCommand );

  ProcessCommandTokens( m_TokenSequence, command );
}



void Console::ProcessCommandTokens( GR::Tokenizer::TokenSequence& TokenSequence, GR::String& Command )
{
}



