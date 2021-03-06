//
// Created by beadle on 3/23/17.
//

#ifndef EXAMPLE_TCP_CONNECTION_H
#define EXAMPLE_TCP_CONNECTION_H

#include <atomic>
#include <memory>

#include "net/Types.h"
#include "net/Buffer.h"
#include "net/Connection.h"


namespace saf
{
	class IOFd;
	class Socket;
	class EventLoop;

	class TcpConnection : public Connection, public IOFdObserver
	{
	public:
		enum {
			kDisconnected,
			kConnecting,
			kConnected,
			kDisconnecting,
		};

	public: /// Thread-Safed Methods
		TcpConnection(EventLoop* loop,
					  Socket* socket,
					  const std::string& index,
					  const InetAddress& addr);
		~TcpConnection();

		bool isConnected() const { return _status == kConnected; }
		void setTcpNoDelay(bool on);

		void shutdown();
		void close();

	public:  /// IOFdObserber Methods
		void onReadInIOFd(IOFd*);
		void onWriteInIOFd(IOFd*);
		void onErrorInIOFd(IOFd*);
		void onCloseInIOFd(IOFd*);

	protected:  /// Friend Methods
		void changeStatus(int status);

		friend class TcpServer;
		friend class TcpClient;

	protected:  /// Looper Thread Methods
		void sendInLoop(const char* data, size_t len);
		void closeInLoop();

		ssize_t writeInLoop(const char* buffer, size_t length);

		void handleReadInLoop();
		void handleWriteInLoop();
		void handleErrorInLoop();
		void handleCloseInLoop();

		void onConnectEstablishedInLoop();

	protected:
		Socket* _socket;
		std::atomic_int _status;
	};


}

#endif //EXAMPLE_CONNECTION_H
