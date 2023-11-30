#ifndef LOCAL_REGISTRY_H
#define LOCAL_REGISTRY_H



#include <Interface/IIOStream.h>

#include <Lang/Service.h>
#include <Lang/FastDelegate.h>

#include <Misc/Misc.h>

#include <string>
#include <map>
#include <list>



class LocalRegistry;

class LocalRegistry : public GR::Service::Service
{

  public:

    class Variable
    {

      protected:

        GR::String        m_Name;

        LocalRegistry*    m_pRegistry;

        bool              m_Temporary;

        GR::String        m_TempValue;



      public:

        Variable( LocalRegistry& LR, const GR::String& Name = GR::String(), const GR::String& Value = GR::String() );

        Variable& operator =( const GR::String& Input );
        Variable& operator +=( const GR::String& Input );
        Variable& operator =( const float Input );
        Variable& operator +=( const float Input );
        Variable& operator -=( const float Input );
        Variable& operator *=( const float Input );
        Variable& operator /=( const float Input );
        Variable& operator =( const int& Input );
        Variable& operator +=( const int& Input );
        Variable& operator -=( const int& Input );
        Variable& operator *=( const int& Input );
        Variable& operator /=( const int& Input );
        Variable& operator ++(); //- pre-inc
        Variable operator ++(int); //- post-inc
        Variable& operator --(); //- pre-inc
        Variable operator --(int); //- post-inc

        operator int();
        operator float();
        operator GR::String();

        GR::String ToString() const;

    

    
};

    enum LocalRegistryEvent
    {
      LRE_VAR_ADDED,
      LRE_VAR_CHANGED,
      LRE_VAR_REMOVED
    };

    typedef fastdelegate::FastDelegate2<LocalRegistryEvent,const GR::String&>    tVarEventHandler;

    typedef std::list<tVarEventHandler>     tEventHandlers;



  protected:

    tEventHandlers              m_EventHandlers;
  
    typedef std::map<GR::String, GR::String>      tMapVars;

    tMapVars                    m_Vars;


    void                        _SetVar( const GR::String& Name, const GR::String& Value );


  public:

    LocalRegistry();

    void                        Clear();
    bool                        Empty() const;

    bool                        SaveVars( const GR::Char* szFileName ) const;
    bool                        SaveVars( const GR::String& FileName ) const;
    bool                        LoadVars( const GR::Char* szFileName );
    bool                        LoadVars( const GR::String& FileName );
    bool                        SaveVarsINI( const GR::Char* szFileName ) const;
    bool                        SaveVarsINI( const GR::String& FileName ) const;
    bool                        LoadVarsINI( const GR::Char* szFileName );
    bool                        LoadVarsINI( const GR::String& FileName );
    bool                        LoadVarsXML( const GR::Char* szFileName );
    bool                        LoadVarsXML( const GR::String& FileName );
    bool                        SaveVarsXML( const GR::Char* szFileName ) const;
    bool                        SaveVarsXML( const GR::String& FileName ) const;

    bool                        SaveVars( IIOStream& Stream ) const;
    bool                        LoadVars( IIOStream& Stream );
    bool                        SaveVarsINI( IIOStream& Stream ) const;
    bool                        LoadVarsINI( IIOStream& Stream );

    bool                        VarExists( const GR::String& Name ) const;

    void                        SetVar( const GR::String& Name, const GR::String& Value );
    void                        SetVarF( const GR::String& Name, const float fValue );
    void                        SetVarFDelta( const GR::String& Name, const float fValue );
    void                        SetVarI( const GR::String& Name, const GR::i32 iValue );
    void                        SetVarIDelta( const GR::String& Name, const GR::i32 iValue );
    void                        SetVarI64( const GR::String& Name, const GR::i64 iValue );
    void                        SetVarI64Delta( const GR::String& Name, const GR::i64 iValue );
    void                        SetVarUP( const GR::String& Name, const GR::up dwValue );
    void                        SetVarUPDelta( const GR::String& Name, const GR::ip iValue );

    float                       GetVarF( const GR::String& Name ) const;
    GR::i32                     GetVarI( const GR::String& Name ) const;
    GR::i64                     GetVarI64( const GR::String& Name ) const;
    GR::i64                     GetVarI64( const GR::String& Name, const GR::i64 max ) const;
    GR::String                  GetVar( const GR::String& Name ) const;
    GR::up                      GetVarUP( const GR::String& Name ) const;

    void                        DeleteVar( const GR::String& Name );

    void                        AddHandler( tVarEventHandler Handler );
    void                        RemoveHandler( tVarEventHandler Handler );
    void                        SendEvent( LocalRegistryEvent Event, const GR::String& Var );

    void                        Merge( const LocalRegistry& Other, bool OverwriteExisting = true );

    Variable                        operator[]( const GR::String& Name )
    {
      return Variable( *this, Name, m_Vars[Name] );
    }

};

#endif // LOCAL_REGISTRY_H