g++  ../../../../src/infra_main.cpp ../user_code/NetSocket.cpp ../../../../src/loop.cpp ../../../../src/net.cpp ../../../../src/infrastructure.cpp ../../../../src/tcp_socket/tcp_socket.cpp ../../../../safe_memory/gcc_lto_workaround/gcc_lto_workaround.cpp ../../../../safe_memory/src/iibmalloc/src/iibmalloc_linux.cpp  ../../../../safe_memory/src/iibmalloc/src/page_allocator_linux.cpp ../../../../safe_memory/src/iibmalloc/src/foundation/src/log.cpp ../../../../safe_memory/src/iibmalloc/src/foundation/3rdparty/fmt/src/format.cc -I../../../../safe_memory/src/iibmalloc/src/foundation/include -I../../../../safe_memory/src/iibmalloc/src/foundation/3rdparty/fmt/include -I../../../../safe_memory/src/iibmalloc/src -I../../../../safe_memory/src -I../../../../include -I../../../../src -std=c++17 -DUSING_T_SOCKETS -g -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-empty-body -DNDEBUG -O2 -flto -lpthread -o xclient.bin