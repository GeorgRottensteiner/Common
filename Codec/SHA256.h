/*
 * FILE:	sha2.h
 * AUTHOR:	Aaron D. Gifford - http://www.aarongifford.com/
 * 
 * Copyright (c) 2000-2001, Aaron D. Gifford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: sha2.h,v 1.1 2001/11/08 00:02:01 adg Exp adg $
 */

#pragma once

#include <Interface/IHash.h>

#include <Memory/ByteBuffer.h>



namespace GR
{
	namespace Codec
	{
		class SHA256 : public IHash
		{
			public:


				virtual ByteBuffer      Calculate( const ByteBuffer& Data );
				static bool             Calculate( const ByteBuffer& Data, ByteBuffer& Hash );

				virtual void            Initialise();
				virtual bool            Update( const ByteBuffer& Data, GR::u32 NumOfBytes = 0 );
				virtual ByteBuffer      Finalize();

				static bool             CalcHashKeyed( const ByteBuffer& Data, const ByteBuffer& Key, ByteBuffer& Hash );

				virtual ByteBuffer      Hash() const;

				virtual int							HashSize() const;



			private:

				enum Constants
				{
					SHA256_BLOCK_LENGTH					= 64,
					SHA256_DIGEST_LENGTH				= 32,
					SHA256_DIGEST_STRING_LENGTH	= ( SHA256_DIGEST_LENGTH * 2 + 1 )
				};


				struct SHA256_CTX
				{
					GR::u32	  state[8];
					GR::u64   bitcount;
					GR::u8    buffer[SHA256_BLOCK_LENGTH];
				};

				SHA256_CTX							context;

				ByteBuffer							m_Hash;


				void SHA256_Transform( const GR::u32* data );
				void SHA256_Update( const GR::u8* data, size_t len );
				void SHA256_Final( GR::u8 digest[] );

		};

	}
}
