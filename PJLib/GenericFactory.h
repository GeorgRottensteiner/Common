#ifndef _GENERICFACTORY_H
#define _GENERICFACTORY_H

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <memory>
#include <map>
#include <string>
#include <iostream>


/////////////////////////////////////////////////////////////////////
//-
//-    template class GenericFactory
//-
//-    die Fabrik
//-
/////////////////////////////////////////////////////////////////////
template <class ManufacturedType, typename ClassIDKey=GR::String>
class GenericFactory
{
  typedef std::auto_ptr<ManufacturedType> (*BaseCreateFn)();
  typedef std::map<ClassIDKey, BaseCreateFn> FnRegistry;
  FnRegistry registry;


  GenericFactory(const GenericFactory&); //- Not implemented
  GenericFactory &operator=(const GenericFactory&); //- Not implemented
  GenericFactory() {      std::cout << typeid( *this ).name() << " " << this << " created\r\n";  }

public:
// #pragma auto_inline( off )
  // static GenericFactory& instance()
  // {
    // static GenericFactory inst;
    // return inst;
  // }
// #pragma auto_inline()
  static GenericFactory& instance();

  ~GenericFactory() {      std::cout << typeid( *this ).name() << " " << this << " destroyed\r\n";  }


  void RegCreateFn(const ClassIDKey &id, BaseCreateFn fn);

  std::auto_ptr<ManufacturedType> Create(const ClassIDKey &className) const;


  void Dump( std::ostream& o )
  {
    o << "{\r\n";

    FnRegistry::iterator it( registry.begin() ), itend( registry.end() );
    for ( ; it != itend; ++it )
    {
      o << "  " << it->first.c_str() << "\r\n";
    }

    o << "}\r\n";
  }

};

// #pragma auto_inline( off )
#pragma auto_inline ( off )
template <class ManufacturedType, typename ClassIDKey>
GenericFactory<ManufacturedType, ClassIDKey>& GenericFactory<ManufacturedType, ClassIDKey>::instance()
{
  static GenericFactory<ManufacturedType, ClassIDKey> inst;
  std::cout << "instance" << " " << &inst << "\r\n";
  return inst;
}
#pragma auto_inline ( on )

// #pragma auto_inline()


/////////////////////////////////////////////////////////////////////
//-    GenericFactory::RegCreateFn
/////////////////////////////////////////////////////////////////////
template <class ManufacturedType, typename ClassIDKey>
void GenericFactory <ManufacturedType, ClassIDKey>::RegCreateFn(const ClassIDKey &id, BaseCreateFn fn)
{
  registry[ id ] = fn;
}


/////////////////////////////////////////////////////////////////////
//-    GenericFactory::Create
/////////////////////////////////////////////////////////////////////
template <class ManufacturedType, typename ClassIDKey>
std::auto_ptr<ManufacturedType> GenericFactory <ManufacturedType, ClassIDKey>::Create(const ClassIDKey &className) const
{
  std::auto_ptr<ManufacturedType> theObject(0);
  FnRegistry::const_iterator regEntry = registry.find(className);
  if (regEntry != registry.end())
    theObject = regEntry->second();
  return theObject;
}



/////////////////////////////////////////////////////////////////////
//-
//-    template class RegisterInFactory
//-
//-    Registrierungs-Helfer-Klasse
//-
/////////////////////////////////////////////////////////////////////
template <class AncestorType, class ManufacturedType, typename ClassIDKey=GR::String>
class RegisterInFactory
{
public:
  static std::auto_ptr<AncestorType> CreateInstance()
  {
    return std::auto_ptr<AncestorType>(new ManufacturedType);
  }

  RegisterInFactory(const ClassIDKey &id)
  {
    GenericFactory<AncestorType>::instance().RegCreateFn(id, CreateInstance);
    std::cout << typeid( *this ).name() << " created\r\n";
  }

  ~RegisterInFactory() {     std::cout << typeid( *this ).name() << " destroyed\r\n";  };
};


// RegisterInFactory<Base, Derived> registerMe("Derived");

#endif// _GENERICFACTORY_H
