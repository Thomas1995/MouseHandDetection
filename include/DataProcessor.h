#pragma once
#include <deque>
#include "CursorData.h"

class DataProcessor

{
public:
    static DataProcessor* instance();

    void SendCursorData(int x, int y, int state, int serverSocket);
    long int GetLastTime();
  static  long int getTime();
private:
    DataProcessor();
    CursorData Interpolate();

    static DataProcessor* m_instance;
    std::deque<CursorData> m_queuedData;
    long int m_lastTimeSent;

};
