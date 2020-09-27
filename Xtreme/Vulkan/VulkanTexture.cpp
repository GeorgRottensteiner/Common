#include "VulkanRenderer.h"
#include "VulkanTexture.h"



VulkanTexture::VulkanTexture() :
  m_MipMapLevels( 1 ),
  AllowUsageAsRenderTarget( false ),
  m_TransparentColor( 0 ),
  m_TextureImage( VK_NULL_HANDLE ),
  m_TextureImageMemory( VK_NULL_HANDLE ),
  m_Device( VK_NULL_HANDLE )
{
}



VulkanTexture::~VulkanTexture()
{
  Release();
}



bool VulkanTexture::Release()
{
  if ( m_TextureImage != VK_NULL_HANDLE )
  {
    vkDestroyImage( m_Device, m_TextureImage, nullptr );
    m_TextureImage = VK_NULL_HANDLE;
  }
  if ( m_TextureImageMemory = VK_NULL_HANDLE )
  {
    vkFreeMemory( m_Device, m_TextureImageMemory, nullptr );
    m_TextureImageMemory = VK_NULL_HANDLE;
  }
  return true;
}



bool VulkanTexture::RequiresRebuild()
{
  if ( !AllowUsageAsRenderTarget )
  {
    return false;
  }
  return false;
}