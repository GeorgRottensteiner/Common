#ifndef DX11_FONT_H
#define DX11_FONT_H


#include <Xtreme\XBasicFont.h>



class DX11Renderer;

class DX11Font : public XBasicFont
{
  public:

    DX11Font( DX11Renderer* pRenderer, GR::IEnvironment* pEnvironment );

};


#endif // DX11_FONT_H