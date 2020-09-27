#ifndef _INTERFACE_IMAGEGENERATOR_H_
#define _INTERFACE_IMAGEGENERATOR_H_


namespace GR
{
  namespace Graphic
  {
    class ContextDescriptor;
  };
};

class IImageGenerator
{
  public:

    virtual bool      Generate( GR::Graphic::ContextDescriptor& cdTarget ) = 0;

};



#endif //_INTERFACE_IMAGEGENERATOR_H_