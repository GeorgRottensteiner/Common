#include "SecureRandom.h"



namespace math
{

  SecureRandom::SecureRandom( GR::Codec::IHash& Hash, GR::Codec::ICipher& Cipher ) :
    m_Pool( MAX_BLOCK_SIZE ),
    m_Hash( Hash ),
    m_Cipher( Cipher ),
    m_CipherInitialised( false )
  {
  }



  void SecureRandom::AddEntropy( const ByteBuffer& Data )
  {
    m_Hash.Initialise();
    m_Hash.Update( m_Pool );
    m_Hash.Update( Data );

    m_Pool = m_Hash.Finalize();
  }



  bool SecureRandom::GetRandom( ByteBuffer& Target, int NumBytes )
  {
    //if ( !m_CipherInitialised )
    {
      int keySize = m_Hash.HashSize();

      ByteBuffer    key = m_Pool.SubBuffer( 0, keySize );
      if ( !m_Cipher.Initialise( true, key, key ) )
      {
        return false;
      }
      m_CipherInitialised = true;
    }

    Target.Clear();

    while ( Target.Size() < (size_t)m_Cipher.BlockSize() )
    {
      Target.Resize( NumBytes + m_Cipher.BlockSize() );
    }
    if ( !m_Cipher.TransformBlock( Target, Target ) )
    {
      return false;
    }
    Target.Resize( NumBytes );
    AddEntropy( Target );
    return true;
  }



}
