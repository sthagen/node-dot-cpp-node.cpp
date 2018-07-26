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


#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "../../include/nodecpp/common.h"
#include "../../include/nodecpp/net.h"

using namespace nodecpp;

#ifdef _MSC_VER
#include <winsock2.h>
using socklen_t = int;
#else
using SOCKET = int;
const SOCKET INVALID_SOCKET = -1;
struct pollfd;
#endif

class EvQueue
{
	std::list<std::function<void()>> evQueue;
public:
	template<class M, class T, class... Args>
	void add(M T::* pm, T* inst, Args... args)
	{
		//auto b = std::bind(pm, inst, args...);
		//evQueue.push_back(std::move(b));
		(inst->*pm)(args...);
	}

	void emit()
	{
		//TODO: verify if exceptions may reach here from user code
		for (auto& current : evQueue)
		{
			current();
		}
		evQueue.clear();
	}
};


class Ip4
{
	uint32_t ip = -1;
	Ip4(uint32_t ip) :ip(ip) {}
public:
	Ip4() {}
	Ip4(const Ip4&) = default;
	Ip4(Ip4&&) = default;
	Ip4& operator=(const Ip4&) = default;
	Ip4& operator=(Ip4&&) = default;


	uint32_t getNetwork() const { return ip; }
	static Ip4 parse(const char* ip);
	static Ip4 fromNetwork(uint32_t ip);
	std::string toStr() const { return std::string("TODO"); }
};

class Port
{
	uint16_t port = -1;
	Port(uint16_t port) :port(port) {}
public:
	Port() {}
	Port(const Port&) = default;
	Port(Port&&) = default;
	Port& operator=(const Port&) = default;
	Port& operator=(Port&&) = default;

	uint16_t getNetwork() const { return port; }
	static Port fromHost(uint16_t port);
	static Port fromNetwork(uint16_t port);
	std::string toStr() const { return std::string("TODO"); }

};

bool isNetInitialized();

class NetSocketEntry {
public:
	size_t index;

	net::Socket* ptr = nullptr;

	bool connecting = false;
	bool remoteEnded = false;
	bool localEnded = false;
	bool pendingLocalEnd = false;
	bool paused = false;
	bool allowHalfOpen = true;

	bool refed = true;

	Buffer writeBuffer = Buffer(64 * 1024);
	Buffer recvBuffer = Buffer(64 * 1024);

	SOCKET osSocket = INVALID_SOCKET;


	NetSocketEntry(size_t index) :index(index) {}
	NetSocketEntry(size_t index, net::Socket* ptr) :index(index), ptr(ptr) {}

	NetSocketEntry(const NetSocketEntry& other) = delete;
	NetSocketEntry& operator=(const NetSocketEntry& other) = delete;

	NetSocketEntry(NetSocketEntry&& other) = default;
	NetSocketEntry& operator=(NetSocketEntry&& other) = default;

	bool isValid() const { return ptr != nullptr; }

	net::Socket* getPtr() const {
		return ptr;
	}
};


class NetSocketManager {
	//mb: ioSockets[0] is always reserved and invalid.
	std::vector<NetSocketEntry> ioSockets; // TODO: improve
	std::vector<std::pair<size_t, bool>> pendingCloseEvents;
	std::vector<Buffer> bufferStore; // TODO: improve
	std::vector<Error> errorStore;

	std::string family = "IPv4";
	
public:
	static const size_t MAX_SOCKETS = 100; //arbitrary limit
	NetSocketManager();
	
	size_t connect(net::Socket* ptr, const char* ip, uint16_t port);
	void destroy(size_t id);
	void end(size_t id);
	void pause(size_t id) { getEntry(id).paused = true; }
	void resume(size_t id) { getEntry(id).paused = false; }

	size_t bufferSize(size_t id) { return getEntry(id).writeBuffer.size(); }

	void ref(size_t id) { getEntry(id).refed = true; }
	void setKeepAlive(size_t id, bool enable);
	void setNoDelay(size_t id, bool noDelay);
	void unref(size_t id) { getEntry(id).refed = false; }
	bool write(size_t id, const uint8_t* data, uint32_t size);

	//TODO quick workaround until definitive life managment is in place
	Buffer& storeBuffer(Buffer buff) {
		bufferStore.push_back(std::move(buff));
		return bufferStore.back();
	}

	Error& storeError(Error err) {
		errorStore.push_back(std::move(err));
		return errorStore.back();
	}

	void clearStores() {
		bufferStore.clear();
		errorStore.clear();
	}
	
	
public:
	// to help with 'poll'
	size_t getPollFdSetSize() const;
	bool setPollFdSet(pollfd* begin, const pollfd* end) const;
	void getPendingEvent(EvQueue& evs);
	void checkPollFdSet(const pollfd* begin, const pollfd* end, EvQueue& evs);

private:
	void processReadEvent(NetSocketEntry& current, EvQueue& evs);
	void processRemoteEnded(NetSocketEntry& current, EvQueue& evs);
	void processWriteEvent(NetSocketEntry& current, EvQueue& evs);

	std::pair<bool, Buffer> getPacketBytes(Buffer& buff, SOCKET sock);
public:
	size_t addEntry(net::Socket* ptr);
	bool addAccepted(net::Socket* ptr, SOCKET sock);

private:
	NetSocketEntry& getEntry(size_t id);
	const NetSocketEntry& getEntry(size_t id) const;

	void makeErrorEventAndClose(NetSocketEntry& entry, EvQueue& evs);
	void makeAsyncErrorEventAndClose(NetSocketEntry& entry);
};



class NetServerEntry {
public:
	size_t index;
	net::Server* ptr = nullptr;
	bool refed = true;
	SOCKET osSocket = INVALID_SOCKET;
	short fdEvents = 0;

	NetServerEntry(size_t index) :index(index) {}
	NetServerEntry(size_t index, net::Server* ptr) :index(index), ptr(ptr) {}
	
	NetServerEntry(const NetServerEntry& other) = delete;
	NetServerEntry& operator=(const NetServerEntry& other) = delete;

	NetServerEntry(NetServerEntry&& other) = default;
	NetServerEntry& operator=(NetServerEntry&& other) = default;

	bool isValid() const { return ptr != nullptr; }

	net::Server* getPtr() const { return ptr; }
};


class NetServerManager
{
	//mb: ioSockets[0] is always reserved and invalid.
	std::vector<NetServerEntry> ioSockets; // TODO: improve
	std::vector<std::pair<size_t, bool>> pendingCloseEvents;
	std::vector<Error> errorStore;

	std::string family = "IPv4";

public:
	static const size_t MAX_SOCKETS = 100; //arbitrary limit
	NetServerManager();

	void close(size_t id);
	void listen(net::Server* ptr, uint16_t port, const char* ip, int backlog);

	void ref(size_t id) { getEntry(id).refed = true; }
	void unref(size_t id) { getEntry(id).refed = false; }


	//TODO quick workaround until definitive life managment is in place
	Error& storeError(Error err) {
		errorStore.push_back(std::move(err));
		return errorStore.back();
	}

	void clearStores() {
		errorStore.clear();
	}

	// to help with 'poll'
	size_t getPollFdSetSize() const;
	bool setPollFdSet(pollfd* begin, const pollfd* end) const;
	void getPendingEvent(EvQueue& evs);
	void checkPollFdSet(const pollfd* begin, const pollfd* end, EvQueue& evs);

private:
	void processAcceptEvent(NetServerEntry& entry, EvQueue& evs);
public:
	size_t addEntry(net::Server* ptr);
private:
	NetServerEntry& getEntry(size_t id);
	const NetServerEntry& getEntry(size_t id) const;

	void makeErrorEventAndClose(NetServerEntry& entry, EvQueue& evs);
};


#endif // TCP_SOCKET_H
