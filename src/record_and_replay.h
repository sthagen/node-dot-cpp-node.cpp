/* -------------------------------------------------------------------------------
* Copyright (c) 2020, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the OLogN Technologies AG nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL OLogN Technologies AG BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/

#ifndef RECORD_AND_REPLAY_H
#define RECORD_AND_REPLAY_H

#include "../include/nodecpp/common.h"
#include <page_allocator.h>

namespace nodecpp::record_and_replay {

class BinaryLogCommon
{
protected:
	struct LogHeader
	{
		uint8_t signature[16];
		uint64_t size;
		uint64_t exeCheckSum; // size TBD
		uint64_t hashAlgo;
	};
	struct FrameHeader
	{
		uint32_t type;
		uint32_t size;
	};

	uint8_t* mem = nullptr;
	size_t size = 0;
	FrameHeader* fh = nullptr;
};

class BinaryLogForRecording : public BinaryLogCommon
{
	uint8_t* writePos = nullptr;
	size_t currentFrameSize = 0;
	uint32_t currentFrameType = 0;
public:
	BinaryLogForRecording() {}
	~BinaryLogForRecording() { deinit(); }
	void init( size_t szExp ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, szExp >= 16, "szExp = {}", szExp ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, szExp <= 48, "szExp = {}", szExp ); 
		size = ((size_t)1) << szExp;
		mem = (uint8_t*)(nodecpp::VirtualMemory::allocate( size ) );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem != nullptr ); 
		// TODO: init header
		writePos = mem + sizeof( LogHeader );
	}
	void deinit() {
		if ( mem )
			nodecpp::VirtualMemory::deallocate( mem, size );
	}
	void addFrame( uint32_t frameType, void* data, uint32_t sz) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frameType != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data != nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem + size - writePos >= sz ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize == 0, "indeed: {}", currentFrameSize ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameType == 0, "indeed: {}", currentFrameType ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh == nullptr ); 
		FrameHeader* h = (FrameHeader*)writePos;
		h->type = 0;
		memcpy( h + 1, data, sz );
		h->size = sz;
		h->type = frameType;
		writePos = (uint8_t*)(h+1) + sz;
	}
	void startAddingFrame( uint32_t frameType, void* data, uint32_t sz) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frameType != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data != nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem + size - writePos >= sz ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize == 0, "indeed: {}", currentFrameSize ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameType == 0, "indeed: {}", currentFrameType ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh == nullptr ); 
		fh = (FrameHeader*)writePos;
		fh->type = 0;
		memcpy( fh + 1, data, sz );
		writePos = (uint8_t*)(fh+1) + sz;
		currentFrameSize = sz;
	}
	void continueAddingFrame( void* data, uint32_t sz) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, data != nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mem + size - writePos >= sz ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh != nullptr ); 
		memcpy( writePos, data, sz );
		writePos += sz;
		currentFrameSize += sz;
	}
	void addingFrameDone() {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, currentFrameSize != 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, fh != nullptr ); 
		fh->type = currentFrameType;
		fh->size = currentFrameSize;
		fh = nullptr;
		currentFrameSize = 0;
	}
};

class BinaryLogForReplaying : public BinaryLogCommon
{
public:
	struct FrameData
	{
		void* ptr = nullptr;
		size_t size = 0;
		uint32_t type = 0;
	};

public:
	BinaryLogForReplaying() {}
	~BinaryLogForReplaying() {}

	void init( void* buff, size_t sz ) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz > 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff != nullptr ); 
		size = sz;
		mem = (uint8_t*)(buff);
		// TODO: complete header validation
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff != nullptr ); 
		LogHeader* lh = (LogHeader*)(mem);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, lh->size <= sz, "indeed: {} vs. {}", lh->size, sz ); 
		fh = (FrameHeader*)(lh + 1);
	}

	 bool readNextFrame( FrameData& fd ) {
		if ( (uint8_t*)fh + sizeof( FrameHeader ) < mem + size && fh->type && (uint8_t*)fh + sizeof( FrameHeader ) + fh->size < mem + size) {
			fd.size = fh->size;
			fd.type = fh->type;
			fd.ptr = fh + 1;
			fh = (FrameHeader*)( (uint8_t*)(fh + 1) + fh->size );
			return true;
		}
		return false;
	}
};

}

#endif // RECORD_AND_REPLAY_H
