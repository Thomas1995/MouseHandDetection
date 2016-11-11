#include "../include/SafeQueue.h"
#include "../include/CursorData.h"
template <typename T>
SafeQueue<T>::SafeQueue()
{

}

template <typename T>
SafeQueue<T>::~SafeQueue()
{

}

template <typename T>
void SafeQueue<T>::push(const T& x)
{
	m_mutex.lock();
	m_queue.push(x);
	m_mutex.unlock();
}

template <typename T>
T SafeQueue<T>::front()
{
	T tmp;
	m_mutex.lock();
	tmp = m_queue.front();
	m_mutex.unlock();
	return tmp;
}

template <typename T>
void SafeQueue<T>::pop()
{
	m_mutex.lock();
	m_queue.pop();
	m_mutex.unlock();
}

template <typename T>
size_t SafeQueue<T>::size()
{	
	size_t tmp;
	m_mutex.lock();
	tmp = m_queue.size();
	m_mutex.unlock();
	return tmp;
}
template class SafeQueue<CursorData>;