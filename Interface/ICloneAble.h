#ifndef _ICLONEABLE_H
#define _ICLONEABLE_H

#include <GR/GRTypes.h>



#define DECLARE_CLONEABLE( xClass, strClassName )   virtual ICloneAble* Clone() \
                                     { \
                                       xClass*  pNewClass = new xClass( *this ); \
                                       pNewClass->m_ClassName = strClassName; \
                                       return pNewClass; \
                                     } \
                                     \
                                     static ICloneAble* CreateNewInstance() \
                                     {\
                                       xClass*  pNewClass = new xClass(); \
                                       pNewClass->m_ClassName = strClassName; \
                                       return pNewClass;\
                                     }

#define IMPLEMENT_CLONEABLE( xClass, strClassName ) class C##xClass##StaticRegistrar \
                                      { \
                                        public: \
                                          C##xClass##StaticRegistrar() \
                                          {\
                                          CloneFactory::Instance().RegisterObject( strClassName, xClass::CreateNewInstance ); \
                                          }\
                                      };\
                                      C##xClass##StaticRegistrar register##xClass;




class ICloneAble
{

  public:

    GR::String          m_ClassName;


    ICloneAble()
    {
    }
    virtual ~ICloneAble() 
    {
    }
    virtual ICloneAble* Clone() = 0;

    GR::String Class()
    { 
      return m_ClassName;
    }

};

#endif// _ICLONEABLE_H

