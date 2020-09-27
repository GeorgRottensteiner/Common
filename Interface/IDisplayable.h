#ifndef IDISPLAYABLE_H
#define IDISPLAYABLE_H


template <typename T> class IDisplayable
{

  public:


    typedef T display_type;

    virtual ~IDisplayable() 
    {
    };

    virtual void Display( T& view ) = 0;

};

#endif // IDISPLAYABLE_H

