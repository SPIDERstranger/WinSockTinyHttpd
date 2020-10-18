#ifndef THREADPOOL_H

#define THREADPOOL_H
#pragma once
#include<thread>
#include<queue>
#include<map>
#include<set>
#include<functional>
#include<mutex>
#include<condition_variable>
#include<atomic>
#include<iostream>
namespace spider {
	typedef std::function<void(void)> Task;
	class ThreadPool
	{
	private:
		/*线程池的主要容器*/
		std::vector<std::thread>  threadPool;
		/*任务队列*/
		std::queue<Task> taskQueue;

		std::atomic<bool>  stop;
		std::mutex taskMtx;
		std::condition_variable taskCond;

		static void *worker(void*arg);
		void run();
	public:
		ThreadPool();
		ThreadPool(int num);
		~ThreadPool();
		void append(Task task);

	};
	//void *ThreadPool::worker(void * arg)
	//{
	//	ThreadPool* pool = (ThreadPool*)arg;
	//	pool->run();
	//	return pool;
	//}

	void ThreadPool::run()
	{
		//std::cout << "create : thread"<< std::endl;
		while (stop == false) {
			std::unique_lock<std::mutex> lock(taskMtx);
			//std::cout << "thread in wait" << std::endl;
			taskCond.wait(lock, [this] {return ((!(this->taskQueue.empty())) || (this->stop)); });
			//std::cout << "thread wake" << std::endl;
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
				//std::cout << "run : task()" << std::endl;
				task();
			}

		}
	}

	ThreadPool::ThreadPool() :ThreadPool(5) {}


	ThreadPool::ThreadPool(int num) : stop(false)
	{
		int i = 0;
		while (i< num)
		{
			threadPool.emplace_back(std::bind(&ThreadPool::run,this));
			++i;
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
			//std::cout << " append a task" << std::endl;
			taskQueue.emplace(task);
			//std::cout << " messageQueue :"<<taskQueue.size()<< std::endl;
		}
		taskCond.notify_all();
	}

}

#endif