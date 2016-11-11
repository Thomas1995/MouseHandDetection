#pragma once
#include <mutex>
template <typename T>
class SafeData
{
public:
	SafeData();
	SafeData(const T& val);
	
	void Set(const T& val);
	T Get();
private:
	std::mutex m_mutex;
	T m_val;

	
};