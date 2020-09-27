#include "DDrawRenderClass.h"

#include "DDrawTexture.h"



CDDrawTexture::CDDrawTexture( CDDrawRenderClass* pRenderer ) :
  m_pRenderer( pRenderer )
{
}



CDDrawTexture::~CDDrawTexture()
{

}



bool CDDrawTexture::Release()
{
  m_cdImage.Clear();
  return true;
}



bool CDDrawTexture::RequiresRebuild()
{
  return false;
}