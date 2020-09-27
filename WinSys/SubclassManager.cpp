#include "SubclassManager.h"

#include <debug/debugclient.h>



SubclassManager::SubclassManager() :
  m_InsideChainLevel( 0 )
{
}



bool SubclassManager::AddHandler( const GR::Char* Name, tSubclassFunction Function )
{
  std::list<tSubclassInfo>::iterator    it( m_SubclassFunctions.begin() );
  while ( it != m_SubclassFunctions.end() )
  {
    tSubclassInfo&    Info( *it );

    if ( Info.m_Name == Name )
    {
      Info.m_Function = Function;
      return true;
    }

    ++it;
  }
  tSubclassInfo     Info;

  Info.m_Name = Name;
  Info.m_Function = Function;
  m_SubclassFunctions.push_front( Info );

  return true;
}



bool SubclassManager::RemoveHandler( const GR::Char* Name )
{
  std::list<tSubclassInfo>::iterator    it( m_SubclassFunctions.begin() );
  while ( it != m_SubclassFunctions.end() )
  {
    tSubclassInfo&    Info( *it );

    if ( Info.m_Name == Name )
    {
      if ( m_InsideChainLevel > 0 )
      {
        m_HandlersToRemove.push_back( Name );
      }
      else
      {
        m_SubclassFunctions.erase( it );
      }
      return true;
    }

    ++it;
  }
  return true;
}



BOOL SubclassManager::CallNext( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  std::list<tSubclassInfo>::iterator    it( m_SubclassFunctions.begin() );
  while ( it != m_SubclassFunctions.end() )
  {
    tSubclassInfo&    Info( *it );

    if ( m_CurrentHandlers.empty() )
    {
      dh::Error( "SubclassManager: Ran out of current handlers! This should never happen!" );
      return FALSE;
    }

    if ( Info.m_Name == m_CurrentHandlers.back() )
    {
      ++it;
      if ( it == m_SubclassFunctions.end() )
      {
        return (BOOL)::DefWindowProc( hWnd, uMsg, wParam, lParam );
        /*
        dh::Error( "SubclassManager: Arrived at lowest CallNext level! This should never happen!" );
        return FALSE;
        */
      }
      tSubclassInfo    NextInfo( *it );

      m_CurrentHandlers.pop_back();
      m_CurrentHandlers.push_back( NextInfo.m_Name );

      ++m_InsideChainLevel;
      BOOL    bResult = NextInfo.m_Function( hWnd, uMsg, wParam, lParam );
      --m_InsideChainLevel;

      if ( m_InsideChainLevel == 0 )
      {
        while ( !m_HandlersToRemove.empty() )
        {
          RemoveHandler( m_HandlersToRemove.front().c_str() );
          m_HandlersToRemove.pop_front();
        }
      }

      return bResult;
    }

    ++it;
  }

  dh::Error( "SubclassManager: Callnext - List is empty! This should never happen!" );
  return FALSE;
}



BOOL SubclassManager::CallChain( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  if ( m_SubclassFunctions.empty() )
  {
    return (BOOL)::DefWindowProc( hWnd, uMsg, wParam, lParam );
  }

  tSubclassInfo&    Info( m_SubclassFunctions.front() );

  size_t    iHandlersBefore = m_CurrentHandlers.size();

  m_CurrentHandlers.push_back( Info.m_Name );

  ++m_InsideChainLevel;
  BOOL    bResult = Info.m_Function( hWnd, uMsg, wParam, lParam );
  --m_InsideChainLevel;

  // Durchgang ist fertig
  m_CurrentHandlers.pop_back();

  while ( !m_HandlersToRemove.empty() )
  {
    RemoveHandler( m_HandlersToRemove.front().c_str() );
    m_HandlersToRemove.pop_front();
  }

  if ( m_CurrentHandlers.size() != iHandlersBefore )
  {
    dh::Error( "SubclassManager: Handler list size corrupted!" );
  }
  return bResult;
}



bool SubclassManager::Empty() const
{
  return m_SubclassFunctions.empty();
}