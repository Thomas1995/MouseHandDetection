#pragma once
#include <mutex>
#include <queue>
template <typename T>
class SafeQueue
{
public:
	SafeQueue();
	~SafeQueue();

	void push(const T& x);
	T front();
	void pop();
	size_t size();
private:
	std::queue<T> m_queue;
	std::mutex m_mutex;


};