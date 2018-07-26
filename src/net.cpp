/* -------------------------------------------------------------------------------
* Copyright (c) 2018, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/

#include "../include/nodecpp/net.h"

#include "infrastructure.h"

using namespace std;
using namespace nodecpp;
using namespace nodecpp::net;




void Socket::connect(uint16_t port, const char* ip)
{
	state = CONNECTING;
	id = getInfra().getNetSocket().connect(this, ip, port);
}

void Socket::destroy() { getInfra().getNetSocket().destroy(id); }
void Socket::end() { getInfra().getNetSocket().end(id); }

void Socket::pause() { getInfra().getNetSocket().pause(id); }
void Socket::ref() { getInfra().getNetSocket().ref(id); }

void Socket::resume() { getInfra().getNetSocket().resume(id); }
void Socket::unref() { getInfra().getNetSocket().unref(id); }

size_t Socket::bufferSize() const { return getInfra().getNetSocket().bufferSize(id); }


Socket& Socket::setNoDelay(bool noDelay)
{ 
	getInfra().getNetSocket().setNoDelay(id, noDelay);
	return *this;
}

Socket& Socket::setKeepAlive(bool enable)
{
	getInfra().getNetSocket().setKeepAlive(id, enable);
	return *this;
}

bool Socket::write(const uint8_t* data, uint32_t size)
{
	_bytesWritten += size;
	return getInfra().getNetSocket().write(id, data, size);
}

void Server::ref() { getInfra().getNetServer().ref(id); }
void Server::unref() { getInfra().getNetServer().unref(id); }

void Server::close()
{
	getInfra().getNetServer().close(id);
}


void Server::listen(uint16_t port, const char* ip, int backlog)
{
	getInfra().getNetServer().listen(this, port, ip, backlog);
}




