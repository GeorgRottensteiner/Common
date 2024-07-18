#pragma once

#include <string>
#include <map>

#include <Memory/ByteBuffer.h>



namespace GR
{
  namespace Codec
  {
    class IAuthenticatedCipher
    {
      public:

        virtual ~IAuthenticatedCipher()
        {
        }

        // single call to calculate
        virtual bool Encrypt( const ByteBuffer& Key, 
                              const ByteBuffer& InitialisationVector, 
                              const ByteBuffer& Data, 
                              const ByteBuffer& AuthenticationData, 
                              int AuthenticatedTagLength,
                              ByteBuffer& EncryptedData,
                              ByteBuffer& AuthenticatedTag ) = 0;
        virtual bool Decrypt( const ByteBuffer& Key, 
                              const ByteBuffer& InitialisationVector, 
                              const ByteBuffer& Data,
                              const ByteBuffer& AuthenticationData,
                              int AuthenticatedTagLength,
                              ByteBuffer& DecryptedData,
                              ByteBuffer& AuthenticatedTag ) = 0;

        // reset state
        virtual bool        Initialise( bool Encrypt, const ByteBuffer& Key ) = 0;

        virtual bool        Start( const ByteBuffer& InitialisationVector, const ByteBuffer& AuthenticationData ) = 0;

        // update with new data
        virtual bool        TransformBlock( const ByteBuffer& Data, 
                                            ByteBuffer& Target ) = 0;

        // final call
        virtual bool        TransformFinalBlock( ByteBuffer& AuthenticationTag ) = 0;

        virtual int         BlockSize() const = 0;

    };

  }
}


