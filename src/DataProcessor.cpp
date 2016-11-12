#include "../include/DataProcessor.h"
#include <sys/socket.h>
#include "../include/CursorData.h"
#include <sys/time.h>
#include <cstdio>
#include <math.h>
#define MAX_QUEUE_SIZE 5
#define DELTA_TIME_SEND 5



DataProcessor* DataProcessor::m_instance = nullptr;
DataProcessor* DataProcessor::instance()
{
    if (m_instance == nullptr) {
        m_instance = new DataProcessor();


    }
    return m_instance;
}
DataProcessor::DataProcessor() : m_lastTimeSent(0), m_tmpx(0), m_tmpy(0), m_ctrlPressed(0), m_shiftPressed(0)
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
    m_queuedData.push_back(CursorData(x,y,state));
    m_tmpx += x;
    m_tmpy += y;

    while (m_queuedData.size() > MAX_QUEUE_SIZE) {
	m_tmpx -= m_queuedData.front().x;
	m_tmpy -= m_queuedData.front().y;
	m_queuedData.pop_front();

    }

    if (getTime() - m_lastTimeSent > DELTA_TIME_SEND) {
	m_lastTimeSent = getTime();
	char* msg = DataProcessor::instance()->Interpolate().ToMsg();
	send(socket, msg, 5, 0);
	delete msg;
    }

}
void DataProcessor::SendInputDataWin(int x, int y, char keyCode, int leftClick, int rightClick, int serverSocket)
{

    CursorData cdata = CursorData(x,y,leftClick,rightClick,keyCode);
    m_queuedData.push_back(cdata);
    m_tmpx += x;
    m_tmpy += y;

    while (m_queuedData.size() > MAX_QUEUE_SIZE) {
	m_tmpx -= m_queuedData.front().x;
	m_tmpy -= m_queuedData.front().y;
	m_queuedData.pop_front();

    }

    if (getTime() - m_lastTimeSent > DELTA_TIME_SEND) {
	m_lastTimeSent = getTime();
	ProcessComboAndSend(serverSocket);
    }
}
void DataProcessor::BuildWinMsg(char msg[], const CursorData& cdata)
{

    msg[0] = *((char*)&cdata.x + 1);
    msg[1] = *((char*)&cdata.x);
    msg[2] = *((char*)&cdata.y + 1);
    msg[3] = *((char*)&cdata.y);
    msg[4] = *((char*)&cdata.leftClick);
    msg[5] = *((char*)&cdata.rightClick);
    msg[6] = *((char*)&m_ctrlPressed);
    msg[7] = *((char*)&m_shiftPressed);
    msg[8] = cdata.key;
}
void DataProcessor::ProcessComboAndSend(int serverSocket)
{
    CursorData cdata = DataProcessor::instance()->Interpolate();
    char msg[9];
    if (cdata.key == 1) { //ctrl
	m_ctrlPressed = 1;
    } else {
	if (cdata.key == 2) {
	    m_shiftPressed = 1;
	} else {
	    if (cdata.key == 3) {
		m_ctrlPressed = 1;
		m_shiftPressed = 1;
	    }
	    else {
		if (m_ctrlPressed || m_shiftPressed) {
		    BuildWinMsg(msg, cdata);
		    m_ctrlPressed = 0;
		    m_shiftPressed = 0;

		}
		else {
		    BuildWinMsg(msg, cdata);
		}
	    }
	}
    }
    send(serverSocket, msg, 9, 0);

}
CursorData DataProcessor::Interpolate()
{
    return CursorData(round(m_tmpx / m_queuedData.size()), 
	    round(m_tmpy / m_queuedData.size()), 
	    m_queuedData.back().leftClick, 
	    m_queuedData.back().rightClick,
	    m_queuedData.back().key);
}
