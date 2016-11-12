#include "../include/DataProcessor.h"
#include <sys/socket.h>
#include "../include/CursorData.h"
#include <sys/time.h>
#include <cstdio>
#include <math.h>
#define MAX_QUEUE_SIZE 5
#define DELTA_DATA_SEND 8


DataProcessor* DataProcessor::m_instance = nullptr;
DataProcessor* DataProcessor::instance()
{
    if (m_instance == nullptr) {
        m_instance = new DataProcessor();
        

    }
    return m_instance;
}
DataProcessor::DataProcessor() : m_lastTimeSent(0)
{

	
}
long int DataProcessor::GetLastTime()
{
	return m_lastTimeSent;
}
long int DataProcessor::getTime()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}
void DataProcessor::SendCursorData(int x, int y, int state, int socket)
{
    printf("%d\n", getTime());
    if (m_queuedData.size() > MAX_QUEUE_SIZE) {
		m_queuedData.pop_front();
    }

    m_queuedData.push_back(CursorData(x,y,state));


    if (getTime() - m_lastTimeSent > DELTA_DATA_SEND) {
		m_lastTimeSent = getTime();
		char* msg = m_queuedData.back().ToMsg();
		m_queuedData.pop_front();
		send(socket, msg, 5, 0);
		delete msg;
    }
}

