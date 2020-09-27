#ifndef XTEXTURE_MANAGER_H_
#define XTEXTURE_MANAGER_H_



#include <map>

#include <Xtreme\XTexture.h>


class XRenderer;

class XTextureManager
{

  protected:

    typedef std::map<GR::String,XTexture*>   tResourceMap;


    tResourceMap      m_mapResourcen;


  public:


    static XTextureManager&   Instance();

    XTexture*                 Create( XRenderer& Renderer, 
                                      const GR::String& strName, 
                                      GR::u32 dwTransparentColor = 0xff000000 );

    void                      RecreateTextures();
    void                      ReleaseTextures();

    XTexture*                 Request( const GR::String& strName );
    void                      Insert( const GR::String& strName, XTexture* pResource );
    void                      Remove( XTexture* pResource );

    bool                      empty() const
    {
      return m_mapResourcen.empty();
    }
    size_t                    size() const
    {
      return m_mapResourcen.size();
    }
  
  private:

    XTextureManager();

};



#endif //XTEXTURE_MANAGER_H_