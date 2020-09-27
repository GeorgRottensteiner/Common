#ifndef _STREAM_MANIP_H
#define _STREAM_MANIP_H

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <iostream>


namespace PJ
{

class Einrueckung
{
    int   filltimes;
    char  fillchar;
public:
    explicit Einrueckung( int i, char c = ' ' ) : filltimes(i), fillchar( c ) {} // Note it's 'explicit'!
    friend std::ostream & operator <<(std::ostream &os, const Einrueckung &ein);
};


inline std::ostream & operator <<(std::ostream &os, const Einrueckung &ein)
{
  for ( int i = 0; i < ein.filltimes; ++i )
    os << ein.fillchar << ein.fillchar;
  return os;
}

}
#endif//_STREAM_MANIP_H
