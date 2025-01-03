#include <Misc/CloneFactory.h>

#include <debug/debugclient.h>



CloneFactory::~CloneFactory()
{
  Clear();
}



void CloneFactory::Clear()
{
  m_Clones.clear();
}



CloneFactory& CloneFactory::Instance()
{
  static CloneFactory    g_Instance;

  return g_Instance;
}



ICloneAble* CloneFactory::CreateObject( const GR::String& Name )
{
  tClones::iterator    it( m_Clones.find( Name ) );
  if ( it == m_Clones.end() )
  {
    return NULL;
  }
  ICloneAble*   pClone = it->second();

  pClone->m_ClassName = Name;

  return pClone;
}



bool CloneFactory::RegisterObject( const GR::String& Name, CloneFunc Func, bool ReplaceExisting )
{
  tClones::iterator    it( m_Clones.find( Name ) );
  if ( ( it != m_Clones.end() )
  &&   ( !ReplaceExisting ) )
  {
    dh::Log( "Trying to register object %s a second time!", Name.c_str() );
    return false;
  }
  m_Clones[Name] = Func;
  return true;
}



