#ifndef SCRIPT_HANDLER_H
#define SCRIPT_HANDLER_H

#include <Lua/LuaInstance.h>
#include <Lua/lua/luna.h>
#include <Lua/Loona.h>

#include "LayeredMap.h"
#include "GameObject.h"



namespace GR
{
  namespace Gamebase
  {
    class JREngine;
  }
}



class ScriptEngine
{
  public:

    GR::Gamebase::JREngine*           m_pJREngine;

    static const char                 s_ClassName[];

    static Loona<ScriptEngine>::tRegType  s_Methods[];



    ScriptEngine( GR::Gamebase::JREngine* pEngine = NULL );

    int FindObjectByID( LuaInstance& Lua );
    int FindObjectByType( LuaInstance& Lua );
    int GetControlledObject( LuaInstance& Lua );
    int SetControlledObject( LuaInstance& Lua );
    int ChangeMap( LuaInstance& Lua );
    int CurrentMap( LuaInstance& Lua );
    int CenterCameraOnObject( LuaInstance& Lua );
    int RemoveObject( LuaInstance& Lua );
};



class ScriptMap
{
  public:

    GR::Gamebase::LayeredMap*         m_pMap;
    GR::Gamebase::JREngine*           m_pJREngine;

    static const char                 s_ClassName[];

    static Loona<ScriptMap>::tRegType  s_Methods[];



    ScriptMap( GR::Gamebase::JREngine* pEngine = NULL, GR::Gamebase::LayeredMap* pMap = NULL );

    int GetLayer( LuaInstance& Lua );
    int FindObjectByID( LuaInstance& Lua );

};



class ScriptLayer
{
  public:

    GR::Gamebase::Layer*              m_pLayer;
    GR::Gamebase::JREngine*           m_pJREngine;

    static const char                 s_ClassName[];

    static Loona<ScriptLayer>::tRegType s_Methods[];



    ScriptLayer( GR::Gamebase::JREngine* pEngine = NULL, GR::Gamebase::Layer* pLayer = NULL );


    int SetField( LuaInstance& Lua );
    int Field( LuaInstance& Lua );
    int Width( LuaInstance& Lua );
    int Height( LuaInstance& Lua );

    int DetachObject( LuaInstance& Lua );
    int AttachObject( LuaInstance& Lua );
    int AwakenObject( LuaInstance& Lua );

    int FindObjectByType( LuaInstance& Lua );

};



class ScriptGameObject
{
  public:

    GR::Gamebase::GameObject*         m_pObject;

    GR::Gamebase::JREngine*           m_pJREngine;

    static const char                 s_ClassName[];

    static Loona<ScriptGameObject>::tRegType  s_Methods[];



    ScriptGameObject( GR::Gamebase::JREngine* pEngine = NULL, GR::Gamebase::GameObject* pObject = NULL );


    int SetPosition( LuaInstance& Lua );
    int SetSection( LuaInstance& Lua );

    int GetX( LuaInstance& Lua );
    int GetY( LuaInstance& Lua );

    int Show( LuaInstance& Lua );
    int Hide( LuaInstance& Lua );
    int SetOnTop( LuaInstance& Lua );
    int RemoveObject( LuaInstance& Lua );

};



class ScriptHandler : public LuaInstance
{

  public:

    ScriptHandler();
    ~ScriptHandler();

};


#endif // SCRIPT_HANDLER_H
