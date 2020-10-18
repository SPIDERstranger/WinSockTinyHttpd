#include "pch.h"
#include "ThreadPool.h"

namespace spider {
	void *ThreadPool::worker(void * arg)
	{
		ThreadPool* pool = (ThreadPool*)arg;
		pool->run();
		return pool;
	}

	void ThreadPool::run()
	{
		while (stop != false) {
			std::unique_lock<std::mutex> lock(taskMtx);

			taskCond.wait(lock, [this] {return ((!this->taskQueue.empty()) || (this->stop)); });
			if (stop)
			{
				return;
			}
			if (taskQueue.empty())
			{
				continue;
			}
			else {
				Task task = taskQueue.front();
				taskQueue.pop();
				task();
			}

		}
	}

	ThreadPool::ThreadPool() :ThreadPool(5) {}


	ThreadPool::ThreadPool(int num) : stop(false)
	{
		while (threadPool.size() < num)
		{
			threadPool.emplace_back(ThreadPool::worker);
		}
	}

	ThreadPool::~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(taskMtx);
			stop = true;
		}
		taskCond.notify_all();
		for (auto &m_thread : threadPool)
			m_thread.join();
	}

	void ThreadPool::append(Task task) {
		{
			std::unique_lock<std::mutex> lock(taskMtx);
			taskQueue.emplace(task);
			lock.unlock();
		}
		taskCond.notify_all();
	}

}
