#ifndef GR_ISTRING_H
#define GR_ISTRING_H



namespace GR
{
  struct IString
  {
    virtual ~IString()
    {
    }

    virtual const char* Text() const
    {
      return "";
    }

  };

  struct StringBase : public IString
  {
    private:

      GR::String         m_Text;


    public:

      StringBase( const GR::String& Text ) :
        m_Text( Text )
      {
      }



      virtual const char* Text() const
      {
        return m_Text.c_str();
      }
  };

}    

#endif// GR_ISTRING_H

