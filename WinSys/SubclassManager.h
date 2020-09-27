#ifndef SUBCLASS_MANAGER_H
#define SUBCLASS_MANAGER_H



#include <windows.h>

#include <Lang/FastDelegate.h>

#include <list>

#include <Lang/Service.h>

#include <GR/GRTypes.h>



class ISubclassManager : public GR::Service::Service
{
  public:

    typedef fastdelegate::FastDelegate4<HWND,UINT,WPARAM,LPARAM,BOOL>   tSubclassFunction;


    virtual bool                    AddHandler( const GR::Char* Name, tSubclassFunction Function ) = 0;
    virtual bool                    RemoveHandler( const GR::Char* Name ) = 0;

    virtual bool                    Empty() const = 0;

    virtual BOOL                    CallChain( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) = 0;
    virtual BOOL                    CallNext( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) = 0;

};




class SubclassManager : public ISubclassManager
{

  public:

    struct tSubclassInfo
    {
      tSubclassFunction         m_Function;
      GR::String                m_Name;
    };


  private:

    std::list<tSubclassInfo>    m_SubclassFunctions;

    std::list<GR::String>       m_CurrentHandlers;

    std::list<GR::String>       m_HandlersToRemove;

    int                         m_InsideChainLevel;


  public:


    SubclassManager();

    bool                        AddHandler( const GR::Char* Name, tSubclassFunction Function );
    bool                        RemoveHandler( const GR::Char* Name );

    bool                        Empty() const;

    BOOL                        CallChain( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL                        CallNext( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

};


#endif // SUBCLASS_MANAGER_H



