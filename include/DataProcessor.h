#pragma once
#include <queue>
#include "CursorData.h"
#include "SafeData.h"
#include "SafeQueue.h"

class DataProcessor

{
public:
    static DataProcessor* instance();

    void SendCursorData(int x, int y, int state, int serverSocket);
    SafeData<long int>& GetLastTime();
    void PredictNext(int serverSock);
private:
    DataProcessor();

    long int getTime();
    static DataProcessor* m_instance;
    SafeQueue<CursorData> m_queuedData;
    SafeData<long int> m_lastTimeSent;
    std::vector<SafeData<CursorData>> m_lastData;
    SafeData<int> m_initDataNr;
   
};
