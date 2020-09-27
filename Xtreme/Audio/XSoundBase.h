#ifndef XSOUNDBASE_H
#define XSOUNDBASE_H



#include <Xtreme/XSound.h>



class IDebugService;

class XSoundBase : public XSound
{
  protected:

    int                     m_VolumeRange[101] = { -10000, -6644, -5644, -5059, -4644,
                                             -4322, -4059, -3837, -3644, -3474,
                                             -3322, -3184, -3059, -2943, -2837,
                                             -2737, -2644, -2556, -2474, -2396,
                                             -2322, -2252, -2184, -2120, -2059,
                                             -2000, -1943, -1889, -1837, -1786,
                                             -1737, -1690, -1644, -1599, -1556,
                                             -1515, -1474, -1434, -1396, -1358,
                                             -1322, -1286, -1252, -1218, -1184,
                                             -1152, -1120, -1089, -1059, -1029,
                                             -1000,  -971,  -943,  -916,  -889,
                                              -862,  -837,  -811,  -786,  -761,
                                              -737,  -713,  -690,  -667,  -644,
                                              -621,  -599,  -578,  -556,  -535,
                                              -515,  -494,  -474,  -454,  -434,
                                              -415,  -396,  -377,  -358,  -340,
                                              -322,  -304,  -286,  -269,  -252,
                                              -234,  -218,  -201,  -184,  -168,
                                              -152,  -136,  -120,  -105,   -89,
                                               -74,   -59,   -44,   -29,   -14,
                                               0 };

    IDebugService*          m_pDebugger;



    GR::String              AppPath( const GR::String& Path );
    void                    Log( const char* Format, ... );


  private:


  public:


    XSoundBase();

    virtual ~XSoundBase();


    void                    LoadAssets();
    void                    ReleaseAssets();


};


#endif // XSOUNDBASE_H