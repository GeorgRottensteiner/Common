#pragma once

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>
#include <Memory/MemoryStream.h>



namespace GR
{


#pragma pack( push, 1 )

struct tMessageHeader
{
  enum eMReceiver
  {
    R_SERVER  = 0,
    R_ALL     = 0xffffffff,
  };

  enum eMType
  {
    M_INVALID,
    M_UNKNOWN,
    M_BINARYDATA,
    M_TEXT,
    M_CONNECT_ID,
    M_DISCONNECT,
    M_ENTER_GROUP,
    M_LEAVE_GROUP,
    M_CREATE_GROUP,
    M_DESTROY_GROUP,
    M_USER_INFO,
    M_USER_VALUE,
    M_USER_VALUE_EX,

    M_USER_MESSAGE,
  };

  eMType      m_Type;
  GR::u32     m_dwSender,
              m_dwReceiver;

};

struct tMessage : public tMessageHeader
{

  tMessage( eMType Type, GR::u32 dwSender = R_SERVER, GR::u32 dwReceiver = R_ALL )
  {
    m_Type        = Type;
    m_dwSender    = dwSender;
    m_dwReceiver  = dwReceiver;
  }

  tMessage( MemoryStream& MemBuffer )
  {
    m_Type        = (GR::tMessage::eMType)MemBuffer.ReadU32();
    m_dwSender    = MemBuffer.ReadU32();
    m_dwReceiver  = MemBuffer.ReadU32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    Buffer.Grow( 12 );
    Buffer.AppendU32( m_Type );
    Buffer.AppendU32( m_dwSender );
    Buffer.AppendU32( m_dwReceiver );
  }

  operator ByteBuffer()
  {
    ByteBuffer    temp;

    ToByteBuffer( temp );

    return temp;
  }

};

struct tMsgBinaryData : tMessage
{
  GR::u8*     m_pData;
  GR::u32     m_iSize;
  GR::u32     m_dwType;

  tMsgBinaryData( void* pData, size_t iSize, GR::u32 dwType = 0 ) :
    tMessage( M_BINARYDATA )
  {
    m_pData   = (GR::u8*)pData;
    m_iSize   = (GR::u32)iSize;
    m_dwType  = dwType;
  }

  tMsgBinaryData( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_iSize = MemBuffer.ReadU32();
    m_pData = (GR::u8*)MemBuffer.CurrentData();
    MemBuffer.SetPosition( m_iSize, IIOStream::PT_CURRENT );
    m_dwType = MemBuffer.ReadU32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 8 + m_iSize );
    Buffer.AppendU32( (GR::u32)m_iSize );
    Buffer.AppendData( m_pData, m_iSize );
    Buffer.AppendU32( m_dwType );
  }

};

struct tMsgText : tMessage
{
  GR::String     m_strText;

  tMsgText( const GR::String& strText ) :
    tMessage( M_TEXT ),
    m_strText( strText )
  {
  }

  tMsgText( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    MemBuffer.ReadString( m_strText );
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( m_strText.length() + 1 + 4 );
    Buffer.AppendU32( (GR::u32)m_strText.length() );
    Buffer.AppendData( m_strText.c_str(), m_strText.length() + 1 );
  }
};

struct tMsgConnectId : tMessage
{
  GR::u32     m_dwClientID,
            m_dwReserved;

  tMsgConnectId( GR::u32 dwClientId ) :
    tMessage( M_CONNECT_ID ),
    m_dwClientID( dwClientId ),
    m_dwReserved( 0 )
  {
  }

  tMsgConnectId( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID  = MemBuffer.ReadU32();
    m_dwReserved  = MemBuffer.ReadU32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 8 );
    Buffer.AppendU32( m_dwClientID );
    Buffer.AppendU32( m_dwReserved );
  }
};

struct tMsgDisconnect : tMessage
{
  GR::u32     m_dwClientID;

  tMsgDisconnect( GR::u32 dwClientId ) :
    tMessage( M_DISCONNECT ),
    m_dwClientID( dwClientId )
  {
  }

  tMsgDisconnect( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID  = MemBuffer.ReadU32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 4 );
    Buffer.AppendU32( m_dwClientID );
  }
};

struct tMsgUserInfo : tMessage
{
  GR::u32     m_dwClientID;
  GR::String m_strName;

  tMsgUserInfo( GR::u32 dwClientId, const GR::String& strName ) :
    tMessage( M_USER_INFO ),
    m_dwClientID( dwClientId ),
    m_strName( strName )
  {
  }

  tMsgUserInfo( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID = MemBuffer.ReadU32();
    MemBuffer.ReadString( m_strName );
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( m_strName.length() + 1 + 8 );
    Buffer.AppendU32( m_dwClientID );
    Buffer.AppendU32( (GR::u32)m_strName.length() );
    Buffer.AppendData( m_strName.c_str(), m_strName.length() + 1 );
  }
};

struct tMsgCreateGroup : tMessage
{
  GR::u32       m_dwGroupID;
  GR::u32       m_dwFlags;
  GR::u32       m_dwMaxClients;
  GR::String   m_strName;

  tMsgCreateGroup( GR::u32 dwGroupID, GR::u32 dwFlags, GR::u32 dwMaxClients, const GR::String& strName ) :
    tMessage( M_CREATE_GROUP )
  {
    m_dwGroupID     = dwGroupID;
    m_dwFlags       = dwFlags;
    m_dwMaxClients  = dwMaxClients;
    m_strName       = strName;
  }

  tMsgCreateGroup( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwGroupID     = MemBuffer.ReadU32();
    m_dwFlags       = MemBuffer.ReadU32();
    m_dwMaxClients  = MemBuffer.ReadU32();
    MemBuffer.ReadString( m_strName );
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( m_strName.length() + 1 + 16 );
    Buffer.AppendU32( m_dwGroupID );
    Buffer.AppendU32( m_dwFlags );
    Buffer.AppendU32( m_dwMaxClients );
    Buffer.AppendU32( (GR::u32)m_strName.length() );
    Buffer.AppendData( m_strName.c_str(), m_strName.length() + 1 );
  }
};

struct tMsgDestroyGroup : tMessage
{
  GR::u32   m_dwGroupID;

  tMsgDestroyGroup( const GR::u32 dwGroupID ) :
    tMessage( M_DESTROY_GROUP ),
    m_dwGroupID( dwGroupID )
  {
  }

  tMsgDestroyGroup( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwGroupID = MemBuffer.ReadU32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 4 );
    Buffer.AppendU32( m_dwGroupID );
  }
};

struct tMsgEnterGroup : tMessage
{
  GR::u32       m_dwClientID,
                m_dwGroupID;
  GR::String   m_strPassword;

  tMsgEnterGroup( GR::u32 dwClientID, GR::u32 dwGroupID, const GR::String& strPassword ) :
    tMessage( M_ENTER_GROUP ),
    m_dwClientID( dwClientID ),
    m_dwGroupID( dwGroupID ),
    m_strPassword( strPassword )
  {
  }

  tMsgEnterGroup( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID  = MemBuffer.ReadU32();
    m_dwGroupID   = MemBuffer.ReadU32();
    MemBuffer.ReadString( m_strPassword );
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( m_strPassword.length() + 1 + 12 );
    Buffer.AppendU32( m_dwClientID );
    Buffer.AppendU32( m_dwGroupID );
    Buffer.AppendU32( (GR::u32)m_strPassword.length() );
    Buffer.AppendData( m_strPassword.c_str(), m_strPassword.length() + 1 );
  }
};

struct tMsgLeaveGroup : tMessage
{
  GR::u32   m_dwClientID,
            m_dwGroupID;

  tMsgLeaveGroup( GR::u32 dwClientID, GR::u32 dwGroupID ) :
    tMessage( M_LEAVE_GROUP, R_SERVER, dwClientID ),
    m_dwClientID( dwClientID ),
    m_dwGroupID( dwGroupID )
  {
  }
  tMsgLeaveGroup( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID  = MemBuffer.ReadU32();
    m_dwGroupID   = MemBuffer.ReadU32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 8 );
    Buffer.AppendU32( m_dwClientID );
    Buffer.AppendU32( m_dwGroupID );
  }
};

struct tMsgUserValue : tMessage
{
  GR::u32   m_dwClientID,
            m_dwValueType;

  int       m_iValue,
            m_iValueParam;

  tMsgUserValue( GR::u32 dwClientID, GR::u32 dwValueType = 0, int iValue = 0, int iValueParam = 0 ) :
    tMessage( M_USER_VALUE ),
    m_dwClientID( dwClientID ),
    m_dwValueType( dwValueType ),
    m_iValue( iValue ),
    m_iValueParam( iValueParam )
  {
  }

  tMsgUserValue( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID  = MemBuffer.ReadU32();
    m_dwValueType = MemBuffer.ReadU32();
    m_iValue      = MemBuffer.ReadU32();
    m_iValueParam = MemBuffer.ReadU32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 16 );
    Buffer.AppendU32( m_dwClientID );
    Buffer.AppendU32( m_dwValueType );
    Buffer.AppendU32( m_iValue );
    Buffer.AppendU32( m_iValueParam );
  }
};

struct tMsgUserValueEx : tMessage
{
  GR::u32   m_dwClientID,
            m_dwValueType;

  float     m_fValue,
            m_fValueParam;

  tMsgUserValueEx( GR::u32 dwClientID, GR::u32 dwValueType = 0, float fValue = 0.0f, float fValueParam = 0.0f ) :
    tMessage( M_USER_VALUE_EX ),
    m_dwClientID( dwClientID ),
    m_dwValueType( dwValueType ),
    m_fValue( fValue ),
    m_fValueParam( fValueParam )
  {
  }

  tMsgUserValueEx( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID  = MemBuffer.ReadU32();
    m_dwValueType = MemBuffer.ReadU32();
    m_fValue      = MemBuffer.ReadF32();
    m_fValueParam = MemBuffer.ReadF32();
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 16 );
    Buffer.AppendU32( m_dwClientID );
    Buffer.AppendU32( m_dwValueType );
    Buffer.AppendF32( m_fValue );
    Buffer.AppendF32( m_fValueParam );
  }
};

struct tMsgUserMessage : tMessage
{
  GR::u32         m_dwClientID,
                  m_dwValueType;

  GR::String      m_strText;


  tMsgUserMessage( GR::u32 dwClientID, GR::u32 dwValueType, const GR::String& strText ) :
    tMessage( M_USER_MESSAGE ),
    m_dwClientID( dwClientID ),
    m_dwValueType( dwValueType ),
    m_strText( strText )
  {
  }

  tMsgUserMessage( MemoryStream& MemBuffer ) :
    tMessage( MemBuffer )
  {
    m_dwClientID  = MemBuffer.ReadU32();
    m_dwValueType = MemBuffer.ReadU32();
    MemBuffer.ReadString( m_strText );
  }

  virtual void ToByteBuffer( ByteBuffer& Buffer )
  {
    tMessage::ToByteBuffer( Buffer );

    Buffer.Grow( 12 + m_strText.length() + 1 );
    Buffer.AppendU32( m_dwClientID );
    Buffer.AppendU32( m_dwValueType );
    Buffer.AppendU32( (GR::u32)m_strText.length() );
    Buffer.AppendData( m_strText.c_str(), m_strText.length() + 1 );
  }
};

#pragma pack( pop )


} // namespace GR