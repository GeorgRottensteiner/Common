#ifndef SOUND_MANAGER_H_
#define SOUND_MANAGER_H_


#include <Interface/IResourceManager.h>
#include <Lang/Service.h>



class CSoundManager : public IResourceManager<GR::String,GR::u32>,
                      public GR::Service::Service
{

  protected:


  public:


    virtual void OnSet()
    {
    }

};



#endif //SOUND_MANAGER_H_