// TileSet.h: interface for the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTRADATA_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
#define AFX_EXTRADATA_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_

#include <Interface/IIOStream.h>

struct tExtraData
{
  enum    eExtraDataType
  {
    EX_INVALID,
    EX_WARP_TARGET,
    EX_SCRIPT,
    EX_PATH,
    EX_SCRIPT_CALL_FUNCTION,
  };

  eExtraDataType        m_Type;
  int                   m_iParam[4];
  GR::String           m_strParam;


  tExtraData() :
    m_strParam( "" ),
    m_Type( EX_INVALID )
  {
    ZeroMemory( &m_iParam, sizeof( m_iParam ) );
  }


  void            Load( IIOStream& Stream )
  {
    m_Type    = (eExtraDataType)Stream.ReadLong();
    Stream.ReadBlock( &m_iParam[0], sizeof( m_iParam ) );
    Stream.ReadString( m_strParam );
  }

  void            Save( IIOStream& Stream )
  {
    Stream.WriteLong( m_Type );
    Stream.WriteBlock( &m_iParam[0], sizeof( m_iParam ) );
    Stream.WriteString( m_strParam );
  }

};

#endif // !defined(AFX_EXTRADATA_H__555CDE8B_4E8E_4E3E_BEDD_4F1D2CF08A5B__INCLUDED_)
