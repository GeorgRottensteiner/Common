#ifndef Vulkan_FONT_H
#define Vulkan_FONT_H


#include <Xtreme\XBasicFont.h>



class VulkanRenderer;

class VulkanFont : public XBasicFont
{
  public:

    VulkanFont( VulkanRenderer* pRenderer, GR::IEnvironment* pEnvironment );

};


#endif // Vulkan_FONT_H