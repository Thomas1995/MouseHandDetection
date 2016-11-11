#include "../include/SafeData.h"
#include "../include/CursorData.h"
template <typename T>
SafeData<T>::SafeData()
{

}

template <typename T>
SafeData<T>::SafeData(const T& x) : m_val(x)
{

}


template <typename T>
void SafeData<T>::Set(const T& val)
{
	m_mutex.lock();
	m_val = val;
	m_mutex.unlock();
}

template <typename T>
T SafeData<T>::Get()
{
	T tmp;
	m_mutex.lock();
	tmp = m_val;
	m_mutex.unlock();
	return tmp;
}	
template class SafeData<int>;
template class SafeData<long int>;
template class SafeData<CursorData>;