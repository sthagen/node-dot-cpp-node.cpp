/* -------------------------------------------------------------------------------
* Copyright (c) 2019, OLogN Technologies AG
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

#ifndef CLUSTER_H
#define CLUSTER_H

#include "net_common.h"
#include "server_common.h"

namespace nodecpp
{
	class Cluster; // forward declaration
	class Worker
	{
		static constexpr size_t invalidID = (size_t)(-1);
		friend class Cluster;
		size_t id_ = invalidID;
		Worker() {}
		void setID( size_t id ) { 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, id_ == invalidID, "indeed: id_ = {}", id_ ); 
			id_ = id; 
		}

		nodecpp::safememory::owning_ptr<nodecpp::net::SocketBase> ctrlServer; // TODO: this might be a temporary solution

	public:
		Worker(const Worker&) = delete;
		Worker& operator = (const Worker&) = delete;
		Worker(Worker&& other) {
			id_ = other.id_;
			other.id_ = invalidID;
			ctrlServer = std::move( other.ctrlServer );
		}
		size_t id() const { return id_; }
		void disconnect();
	};

	extern void preinitMasterThreadClusterObject();
	extern void preinitSlaveThreadClusterObject( size_t id);
	extern void postinitThreadClusterObject();
	class Cluster
	{
		class MasterSocket : public nodecpp::net::SocketBase, public ::nodecpp::DataParent<Cluster>
		{
		public:
			nodecpp::handler_ret_type onAccepted()
			{
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
				CO_RETURN;
			}
		};

		class SlaveSocket : public nodecpp::net::SocketBase
		{
		public:
			nodecpp::handler_ret_type onConnect()
			{
				CO_RETURN;
			}
			nodecpp::handler_ret_type onData( nodecpp::Buffer& b)
			{
				CO_RETURN;
			}
		};

		class MasterServer : public nodecpp::net::ServerSocket<Cluster>
		{
			nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<MasterSocket> socket) { 
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
				//srv.unref();
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 

				/*nodecpp::Buffer r_buff(0x200);
				for (;;)
				{
					co_await socket->a_read( r_buff, 2 );
					co_await onDataServerSocket_(socket, r_buff);
				}*/
				CO_RETURN;
			}

		};

		Worker thisThreadWorker;
		std::vector<Worker> workers_;
		uint64_t coreCtr = 0; // for: thread ID generation at master; requestID generation at Slave

		using CtrlServerT = MasterServer;
		nodecpp::safememory::owning_ptr<CtrlServerT> ctrlServer; // TODO: this might be a temporary solution

		friend void preinitMasterThreadClusterObject();
		friend void preinitSlaveThreadClusterObject(size_t);
		friend void postinitThreadClusterObject();

		void preinitMaster() { 
			thisThreadWorker.id_ = 0; 
		}
		void preinitSlave(size_t threadID) { 
			thisThreadWorker.id_ = threadID; 
		}
		void postinit() { 
			if ( isMaster() )
			{
				nodecpp::net::SocketBase::addHandler<MasterSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Accepted, &MasterSocket::onAccepted>();
				nodecpp::net::SocketBase::addHandler<MasterSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MasterSocket::onData>();
				ctrlServer = nodecpp::net::createServer<CtrlServerT, MasterSocket>();
				ctrlServer->listen(21000, "127.0.0.1", 500);
			}
			else
			{
				nodecpp::net::SocketBase::addHandler<SlaveSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &SlaveSocket::onConnect>();
				nodecpp::net::SocketBase::addHandler<SlaveSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &SlaveSocket::onData>();
			}
		}
	public:
		Cluster() {}
		Cluster(const Cluster&) = delete;
		Cluster& operator = (const Cluster&) = delete;
		Cluster(Cluster&&) = delete;
		Cluster& operator = (Cluster&&) = delete;

		bool isMaster() const { return thisThreadWorker.id() == 0; }
		bool isWorker() const { return thisThreadWorker.id() != 0; }
		const std::vector<Worker>& workers() const { return workers_; }
		const Worker& worker() const { return thisThreadWorker; }

		Worker& fork();
		void disconnect() { for ( auto& w : workers_ ) w.disconnect(); }

		// event handling (awaitable)
	};
	extern thread_local Cluster cluster;

	inline
	Cluster& getCluster() { return cluster; }
}
#endif //CLUSTER_H


