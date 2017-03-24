//
// Created by beadle on 3/16/17.
//

#ifndef LIBSAF_EVENTLOOP_H
#define LIBSAF_EVENTLOOP_H

#include <assert.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "base/CurrentThread.h"


namespace saf
{
	class IOFd;
	class Poller;
	class TimerQueue;

	class EventLoop
	{
	public:
		typedef std::function<void()> Functor;

	public:  // exposed to outers
		EventLoop();
		~EventLoop();

		void start();
		void stop();

		void runInLoop(Functor&& functor);
		void queueInLoop(Functor&& functor);

		int addTimer(float delay, const Functor& callback, bool repeated=false);
		void cancelTimer(int fd);

	public:  // exposed to fd objects(Socket, Timer...)
		void updateFd(IOFd* fd);
		void removeFd(IOFd* fd);

		bool isInLoopThread() { return _threadId == CurrentThread::tid(); }
		void assertInLoopThread() { if (!isInLoopThread()) abortNotInLoopThread(); }

	protected:
		void wakeup();
		void handleWakeupRead();
		void abortNotInLoopThread();

		void runFunctors();
		void runTimers();

	private:
		const pid_t _threadId;

		std::atomic_bool _quit;
		std::atomic_bool _looping;
		std::atomic_bool _handling;

		IOFd* _currentFd;

		std::unique_ptr<IOFd> _wakeupFd;
		std::unique_ptr<Poller> _poller;
		std::unique_ptr<TimerQueue> _timerQueue;

		std::mutex _mutex;
		std::vector<Functor> _functors;
	};
}


#endif //LIBSAF_EVENTLOOP_H
