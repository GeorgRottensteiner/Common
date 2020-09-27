#ifndef XSID_H
#define XSID_H


#include <GR/GRTypes.h>


class CXSID
{

  protected:

    GR::String           m_strID;

    GR::u32               m_dwID;


  public:


    CXSID( const GR::String& strID, const GR::u32 dwID = 0 ) :
      m_strID( strID ),
      m_dwID( dwID )
    {
    }

    CXSID( const GR::u32 dwID = 0 ) :
      m_strID( "" ),
      m_dwID( dwID )
    {
    }

    bool operator==( const CXSID& xsID ) const
    {
      if ( &xsID == this )
      {
        return true;
      }
      if ( m_dwID != xsID.m_dwID )
      {
        return false;
      }
      return ( m_strID == xsID.m_strID );
    }

    const char* ToString() const
    {
      return m_strID.c_str();
    }


};

#endif // XSID_H

