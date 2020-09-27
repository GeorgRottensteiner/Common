#ifndef IUPDATEABLE_H
#define IUPDATEABLE_H


template <typename T> class IUpdateable
{

  public:


    virtual ~IUpdateable() 
    {
    };

    virtual void Update( const T deltaT ) = 0;
};

#endif // IUPDATEABLE_H

