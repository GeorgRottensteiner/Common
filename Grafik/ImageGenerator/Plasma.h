#ifndef IMAGE_GENERATOR_PLASMA_H
#define IMAGE_GENERATOR_PLASMA_H

#include <interface/IImageGenerator.h>



class PlasmaGenerator : public IImageGenerator
{

  protected:

    unsigned long     m_VirtualPalette[256];

    int               m_SinusTabelle[4096];

    int               m_X1,
                      m_Y1,
                      m_X2,
                      m_Y2,

                      m_Speed;


  public:

    PlasmaGenerator();

    virtual bool      Generate( GR::Graphic::ContextDescriptor& Target );

    virtual void      SetSpeed( int Speed );
              
};



#endif // IMAGE_GENERATOR_PLASMA_H