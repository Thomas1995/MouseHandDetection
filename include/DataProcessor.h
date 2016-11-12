#pragma once
#include <deque>
#include "CursorData.h"

class DataProcessor

{
public:
  static DataProcessor* instance();

  void SendCursorData(int x, int y, int state, int serverSocket);
  void SendInputDataWin(int x, int y, char keyCode, int leftClick, int rightClick, int serverSocket);
  long int GetLastTime();
  static  long int getTime();
private:
  DataProcessor();
  CursorData Interpolate();
  void ProcessComboAndSend(int serverSocket);
  void BuildWinMsg(char msg[], const CursorData& cdata);

  static DataProcessor* m_instance;
  std::deque<CursorData> m_queuedData;
  long int m_lastTimeSent;
  double m_tmpx; 
  double m_tmpy;
  int m_ctrlPressed;
  int m_shiftPressed;

};
