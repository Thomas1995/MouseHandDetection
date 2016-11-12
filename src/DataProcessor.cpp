#include "../include/DataProcessor.h"
#include <sys/socket.h>
#include "../include/CursorData.h"
#include <sys/time.h>
#include <cstdio>
#include <math.h>
#define MAX_QUEUE_SIZE 25
#define PREDICTION_OFFSET 2.82


DataProcessor* DataProcessor::m_instance = nullptr;
DataProcessor* DataProcessor::instance()
{
    if (m_instance == nullptr) {
        m_instance = new DataProcessor();
        

    }
    return m_instance;
}
DataProcessor::DataProcessor() : m_lastTimeSent(getTime()), m_lastData(std::vector<SafeData<CursorData>>(2)), m_initDataNr(0)
{

	
}
SafeData<long int>& DataProcessor::GetLastTime()
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
    if (m_queuedData.size() > MAX_QUEUE_SIZE) {
		m_queuedData.pop();
    }

    if (m_initDataNr.Get() < 2) {
    	m_lastData[m_initDataNr.Get()].Set(CursorData(x,y,state));	
    } else {
    	m_lastData[0].Set(m_lastData[1].Get());
    	m_lastData[1].Set(CursorData(x,y,state));
    }

    m_queuedData.push(CursorData(x,y,state));


    if (getTime() - m_lastTimeSent.Get() > 5) {
		m_lastTimeSent.Set(getTime());
		char* msg = m_queuedData.front().ToMsg();
		m_queuedData.pop();
		send(socket, msg, 5, 0);
		delete msg;
    }
}

void DataProcessor::PredictNext(int socket)
{
	if (m_initDataNr.Get() < 2) {
		return;
	}
	int dy = abs(m_lastData[1].Get().y - m_lastData[0].Get().y);
	int dx = abs(m_lastData[1].Get().x - m_lastData[0].Get().x);
	if (dx == 0 && dy == 0) {
		return;
	}
	double iplen = sqrt(pow(dy, 2) + pow(dx, 2));
	double offset = PREDICTION_OFFSET;
	int newY = round(offset * (dy / iplen) + m_lastData[1].Get().y);
	int newX = round(offset * (dx / iplen) + m_lastData[1].Get().x);
	printf("Predicting next move.. %d %d %d\n", newX, newY, m_lastData[1].Get().state);
	SendCursorData(newX, newY, m_lastData[1].Get().state, socket);

}