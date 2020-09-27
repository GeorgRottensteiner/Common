#ifndef XTREME_Vulkan_TEXTURE_H
#define XTREME_Vulkan_TEXTURE_H



#include <Xtreme/XTexture.h>

#include <vulkan/vulkan.h>

#include <list>


class VulkanTexture : public XTexture
{

  public:

    GR::u32                           m_TransparentColor;

    GR::u32                           m_MipMapLevels;

    std::list<GR::String>             m_listFileNames;

    std::list<GR::Graphic::ImageData> m_StoredImageData;

    bool                              AllowUsageAsRenderTarget;

    VkImage                           m_TextureImage;
    VkDeviceMemory                    m_TextureImageMemory;
    VkDevice                          m_Device;



    VulkanTexture();
    virtual ~VulkanTexture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME_Vulkan_TEXTURE_H
