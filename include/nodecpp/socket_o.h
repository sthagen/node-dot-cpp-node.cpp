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

#ifndef SOCKET_O_H
#define SOCKET_O_H

#include "net_common.h"
#include "socket_t_base.h"

namespace nodecpp {

	namespace net {

		class [[nodecpp::owning_only]] SocketO : public SocketBase {

		private:
			void registerMeAndAcquireSocket();
			void registerMeAndAssignSocket(OpaqueSocketData& sdata);

		public:
			SocketO(NodeBase* node) : SocketBase( node ) {registerMeAndAcquireSocket();}
			SocketO(NodeBase* node, OpaqueSocketData& sdata) : SocketBase( node ) {registerMeAndAssignSocket(sdata);}

			SocketO(const SocketO&) = delete;
			SocketO& operator=(const SocketO&) = delete;

			SocketO(SocketO&&) = default;
			SocketO& operator=(SocketO&&) = default;

			virtual ~SocketO() { if (state == CONNECTING || state == CONNECTED) destroy(); }

			virtual void onClose(bool hadError) {}
			virtual void onConnect() {}
			virtual void onData(Buffer& buffer) {}
			virtual void onDrain() {}
			virtual void onEnd() {}
			virtual void onAccepted() {}
			virtual void onError(Error& err) {}

			void connect(uint16_t port, const char* ip);
			SocketO& setNoDelay(bool noDelay = true) { OSLayer::appSetNoDelay(dataForCommandProcessing, noDelay); return *this; }
			SocketO& setKeepAlive(bool enable = false) { OSLayer::appSetKeepAlive(dataForCommandProcessing, enable); return *this; }
		};

		
		template<auto x>
		struct OnClose {};

		template<auto x>
		struct OnConnect {};

		template<auto x>
		struct OnData {};

		template<auto x>
		struct OnDrain {};

		template<auto x>
		struct OnError {};

		template<auto x>
		struct OnAccepted {};

		template<auto x>
		struct OnEnd {};

		template<typename ... args>
		struct SocketOInitializer2;

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnClose<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onClose = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnConnect<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onConnect = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnData<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onData = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnDrain<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onDrain = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnError<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onError = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnEnd<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onEnd = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnAccepted<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onAccepted = F;
		};

		//partial template specialiazation to end recursion
		template<>
		struct SocketOInitializer2<> {
			static constexpr auto onConnect = nullptr;
			static constexpr auto onClose = nullptr;
			static constexpr auto onData = nullptr;
			static constexpr auto onDrain = nullptr;
			static constexpr auto onError = nullptr;
			static constexpr auto onEnd = nullptr;
			static constexpr auto onAccepted = nullptr;
		};

		template<class Node, class Extra>
		class SocketOUserBase : public SocketO
		{
	#if 0
			static constexpr bool is_void_extra = std::is_same< Extra, void >::value;
			static_assert( Initializer::onConnect != nullptr );
			static_assert( !is_void_extra );
			static_assert( std::is_same< decltype(Initializer::onConnect), void (Node::*)() >::value );
			static_assert( Initializer::onConnect == nullptr || (is_void_extra && std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value) || ( (!is_void_extra) && std::is_same< decltype(Initializer::onConnect), void (Node::*)(const Extra*) >::value ) );
	#endif
			//Node* node;
			Extra extra;
		public:
//			SocketN2(Node* node_) : SocketO( node_ ) { node = node_;}
//			SocketN2(Node* node_, OpaqueSocketData& sdata) : SocketO( sdata ) { node = node_;}
			SocketOUserBase(Node* node) : SocketO( node ) {}
			SocketOUserBase(Node* node, OpaqueSocketData& sdata) : SocketO( node, sdata ) {}
			Extra* getExtra() { return &extra; }
		};

		template<class Node>
		class SocketOUserBase<Node, void> : public SocketO
		{
	//		Node* node;
	//		static constexpr auto x = void (Node::*onConnect)(const void*);
//			typename std::remove_reference<decltype((Initializer::onConnect))>::type x;
	//		typename void (Node::*)(const void*) y;
	#if 1
	//		static_assert( Initializer::onConnect != nullptr );
	//		static_assert( std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value );
	//		static_assert( std::is_same< typename std::remove_cv<decltype((Initializer::onConnect))>::type, typename std::remove_cv<void (Node::*)(const void*)>::type >::value );
	//		static_assert( std::is_same< typename std::remove_reference<typename std::remove_cv<decltype((Initializer::onConnect))>::type>::type, typename std::remove_reference<typename std::remove_cv<void (Node::*)(const void*)>::type>::type >::value );
	//		static_assert( std::is_same< typename std::remove_cv<decltype(x)>::type, typename std::remove_reference<decltype((Initializer::onConnect))>::type >::value );
	//		static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value );
	//		static_assert( Initializer::onConnect == nullptr || typeid(Initializer::onConnect).hash_code() == typeid(void (Node::*)(const void*)).hash_code() );
	#endif
		public:
	//		SocketN2(Node* node) : SocketO( node ) {}
	//		SocketN2(Node* node_, OpaqueSocketData& sdata) : SocketO( sdata ) { node = node_;}
			SocketOUserBase(Node* node) : SocketO( node ) {}
			SocketOUserBase(Node* node, OpaqueSocketData& sdata) : SocketO( node, sdata ) {}
			void* getExtra() { return nullptr; }
		};

		template<class Node, class Initializer, class Extra>
		class SocketN2 : public SocketOUserBase<Node, Extra>
		{
	#if 0
			static constexpr bool is_void_extra = std::is_same< Extra, void >::value;
			static_assert( Initializer::onConnect != nullptr );
			static_assert( !is_void_extra );
			static_assert( std::is_same< decltype(Initializer::onConnect), void (Node::*)() >::value );
			static_assert( Initializer::onConnect == nullptr || (is_void_extra && std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value) || ( (!is_void_extra) && std::is_same< decltype(Initializer::onConnect), void (Node::*)(const Extra*) >::value ) );
	#endif

		public:
			using StorableType = SocketOUserBase<Node, Extra>;

		public:
//			SocketN2(Node* node_) : SocketO( node_ ) { node = node_;}
//			SocketN2(Node* node_, OpaqueSocketData& sdata) : SocketO( sdata ) { node = node_;}
			SocketN2(Node* node) : SocketOUserBase<Node, Extra>( node ) {}
			SocketN2(Node* node, OpaqueSocketData& sdata) : SocketOUserBase<Node, Extra>( node, sdata ) {}

			void onConnect() override
			{ 
				if constexpr ( Initializer::onConnect != nullptr )
				{
	//				static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)(const Extra*) >::value );
					((static_cast<Node*>(this->node))->*(Initializer::onConnect))(nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>>(this)); 
				}
				else
				{
	//		static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)() >::value );
					SocketO::onConnect();
				}
			}
			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onClose))(nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>>(this),b); 
				else
					SocketO::onClose(b);
			}
			void onData(nodecpp::Buffer& b) override
			{ 
				if constexpr ( Initializer::onData != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onData))(nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>>(this),b); 
				else
					SocketO::onData(b);
			}
			void onDrain() override
			{
				if constexpr ( Initializer::onDrain != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onDrain))(nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>>(this));
				else
					SocketO::onDrain();
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onError))(nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>>(this),e);
				else
					SocketO::onError(e);
			}
			void onEnd() override
			{
				if constexpr ( Initializer::onEnd != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onEnd))(nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>>(this));
				else
					SocketO::onEnd();
			}
			void onAccepted() override
			{
				if constexpr ( Initializer::onAccepted != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onAccepted))(nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>>(this));
				else
					SocketO::onAccepted();
			}
		};

#if 0
		template<class Node, class Initializer>
		class SocketN2<Node, Initializer, void> : public SocketO
		{
	//		Node* node;
	//		static constexpr auto x = void (Node::*onConnect)(const void*);
//			typename std::remove_reference<decltype((Initializer::onConnect))>::type x;
	//		typename void (Node::*)(const void*) y;
	#if 1
			static_assert( Initializer::onConnect != nullptr );
	//		static_assert( std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value );
	//		static_assert( std::is_same< typename std::remove_cv<decltype((Initializer::onConnect))>::type, typename std::remove_cv<void (Node::*)(const void*)>::type >::value );
	//		static_assert( std::is_same< typename std::remove_reference<typename std::remove_cv<decltype((Initializer::onConnect))>::type>::type, typename std::remove_reference<typename std::remove_cv<void (Node::*)(const void*)>::type>::type >::value );
	//		static_assert( std::is_same< typename std::remove_cv<decltype(x)>::type, typename std::remove_reference<decltype((Initializer::onConnect))>::type >::value );
	//		static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value );
	//		static_assert( Initializer::onConnect == nullptr || typeid(Initializer::onConnect).hash_code() == typeid(void (Node::*)(const void*)).hash_code() );
	#endif
		public:
	//		SocketN2(Node* node) : SocketO( node ) {}
	//		SocketN2(Node* node_, OpaqueSocketData& sdata) : SocketO( sdata ) { node = node_;}
			SocketN2(Node* node) : SocketO( node ) {}
			SocketN2(Node* node, OpaqueSocketData& sdata) : SocketO( node, sdata ) {}
			void* getExtra() { return nullptr; }

			void onConnect() override
			{ 
	//assert( Initializer::onConnect == nullptr || typeid(Initializer::onConnect).hash_code() == typeid(void (Node::*)(const void*)).hash_code() );
				//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("type 1 = \'{}\', hash = 0x{:x}", typeid(x).name(), typeid(x).hash_code() );
				//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("type 2 = \'{}\', hash = 0x{:x}", typeid(void (Node::*)(const void*)).name(), typeid(x).hash_code() );
				//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("type 3 = \'{}\', hash = 0x{:x}", typeid(Initializer::onConnect).name(), typeid(x).hash_code() );
				if constexpr ( Initializer::onConnect != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onConnect))(this->getExtra()); 
				else
					SocketO::onConnect();
			}
			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onClose))(this->getExtra(),b); 
				else
					SocketO::onClose(b);
			}
			void onData(nodecpp::Buffer& b) override
			{ 
				if constexpr ( Initializer::onData != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onData))(this->getExtra(),b); 
				else
					SocketO::onData(b);
			}
			void onDrain() override
			{
				if constexpr ( Initializer::onDrain != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onDrain))(this->getExtra());
				else
					SocketO::onDrain();
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onError))(this->getExtra(),e);
				else
					SocketO::onError(e);
			}
			void onEnd() override
			{
				if constexpr ( Initializer::onEnd != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onEnd))(this->getExtra());
				else
					SocketO::onEnd();
			}
			void onAccepted() override
			{
				if constexpr ( Initializer::onAccepted != nullptr )
					((static_cast<Node*>(this->node))->*(Initializer::onAccepted))(this->getExtra());
				else
					SocketO::onAccepted();
			}
		};
#endif // 0

		template<class Node, class Extra, class ... Handlers>
		class SocketN : public SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>
		{
		public:
			SocketN(Node* node) : SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>( node ) {}
			SocketN(Node* node, OpaqueSocketData& sdata) : SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>( node, sdata ) {}
		
		};


	} // namespace net

} // namespace nodecpp

#endif // SOCKET_O_H