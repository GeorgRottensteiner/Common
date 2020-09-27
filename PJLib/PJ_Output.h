#ifndef _PJ_OUTPUT_H_INCLUDED
#define _PJ_OUTPUT_H_INCLUDED

#pragma warning ( disable : 4786 )
#pragma once
#include <PJLib\PJ_Log.h>

namespace PJ
{
  // /////////////////////////////////////////////////////////////////////
  // //-
  // //-    COutput
  // //-
  // /////////////////////////////////////////////////////////////////////

  // class COutput
  // {
  // protected:
    // PJ::CLog Log;
    // HWND                  m_hwndOutput;

    // GR::String   m_strToLog;

  // public:
    // std::ostringstream    m_Messages;

    // COutput( const HWND& hwnd = 0 )
      // : m_hwndOutput( hwnd )
    // {}

    // void Hwnd( const HWND& hwnd ) { m_hwndOutput = hwnd; }
    // HWND Hwnd() const { return m_hwndOutput; }

    // void Update()
    // {
      // if ( !m_hwndOutput ) return;

      // if ( m_Messages.str().length() > 500 )
      // {
        // long pos = m_Messages.str().length() - 500;
        // GR::String strTemp( m_Messages.str().substr( pos, 500 ) );
        // m_Messages.str( strTemp );
      // }

      // std::ostringstream oss;
      // oss << "\r\n" << m_Messages.str().size() << "\r\n";

      // SetWindowText( m_hwndOutput, (m_Messages.str() + oss.str()).c_str() );


      // // std::ostringstream oss;
      // // oss << "\r\n" << m_Messages.str().size() << "\r\n";

      // // SetWindowText( m_hwndOutput, (m_Messages.str() + oss.str()).c_str() );
      // // SendMessage( m_hwndOutput, EM_LINESCROLL, 0, 1000000 );
    // }


    // COutput& operator>>( const HWND& hwnd )
    // {
      // Hwnd( hwnd );
      // Update();
      // return *this;
    // }

    // COutput& operator<<( const char *lpszBuffer )
    // {
      // GR::String strTemp( lpszBuffer );
      // m_strToLog += strTemp;
      // m_Messages << strTemp.c_str();

      // if ( strTemp.find( GR::String( "\r\n" ) ) != GR::String::npos )
      // {
        // // LogText( (char*)m_strToLog.c_str() );
        // Log.Log( (char*)m_strToLog.c_str() );
        // m_strToLog = "";
      // }

      // Update();
      // return *this;
    // }

    // void Print( const char *lpszBuffer )
    // {
      // operator<<( lpszBuffer ) << "\r\n";
    // }

  // };


  // template <class T>
  // COutput& operator<<( COutput& o, const T& t )
  // {
    // std::ostringstream oss;
    // oss << t;
    // o << oss.str().c_str();
    // return o;
  // }

  /////////////////////////////////////////////////////////////////////
  //-
  //-    COutput
  //-
  /////////////////////////////////////////////////////////////////////

  class COutput
  {
  protected:
    PJ::CLog      Log;
    HWND          m_hwndOutput;
    GR::String   m_strToLog;
    GR::String   m_Messages;
    long          m_iBuffersize;

  public:

    COutput(  const GR::String& strlogfile = GR::String( ".\\log.txt" ), const HWND& hwnd = 0, const long buffersize = 5000 )
      : Log( strlogfile ),
        m_hwndOutput( hwnd ),
        m_iBuffersize( buffersize )
    {}

    void Hwnd( const HWND& hwnd ) { m_hwndOutput = hwnd; }
    HWND Hwnd() const { return m_hwndOutput; }

    void Update()
    {
      if ( !m_hwndOutput ) return;

      if ( m_Messages.length() > m_iBuffersize )
      {
        long pos = m_Messages.length() - m_iBuffersize;
        GR::String strTemp( m_Messages.substr( pos, m_iBuffersize ) );
        m_Messages = strTemp;
      }

      // std::ostringstream oss;
      // oss << "\r\n" << m_Messages.length() << "\r\n";
      // SetWindowText( m_hwndOutput, (m_Messages + oss.str()).c_str() );

      SetWindowText( m_hwndOutput, m_Messages.c_str() );
      SendMessage( m_hwndOutput, EM_LINESCROLL, 0, 1000000 );
    }


    COutput& operator>>( const HWND& hwnd )
    {
      Hwnd( hwnd );
      Update();
      return *this;
    }

    COutput& operator<<( const char *lpszBuffer )
    {
      GR::String strTemp( lpszBuffer );
      m_strToLog += strTemp;
      m_Messages += strTemp;

      if ( strTemp.find( GR::String( "\r\n" ) ) != GR::String::npos )
      {
        // LogText( (char*)m_strToLog.c_str() );
        Log.Log( (char*)m_strToLog.c_str() );
        m_strToLog = "";
      }

      Update();
      return *this;
    }

    void Print( const char *lpszBuffer )
    {
      operator<<( lpszBuffer ) << "\r\n";
    }

  };


  template <class T>
  COutput& operator<<( COutput& o, const T& t )
  {
    std::ostringstream oss;
    oss << t;
    o << oss.str().c_str();
    return o;
  }


};
#endif//_PJ_OUTPUT_H_INCLUDED