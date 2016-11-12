#include "../include/CursorData.h"

CursorData::CursorData(int _x, int _y, int _leftClick, int _rightClick , char _key ) : 
    x(_x), y(_y), leftClick(_leftClick), rightClick(_rightClick), key(_key)
{

}

CursorData::CursorData(const CursorData& ref) :
    x(ref.x), y(ref.y), leftClick(ref.leftClick), rightClick(ref.rightClick), key(ref.key) 
{

}

CursorData::~CursorData()
{

}

char* CursorData::ToMsg() const
{
    char* msg = new char[5];
    msg[0] = *((char*)&x + 1);
    msg[1] = *((char*)&x);
    msg[2] = *((char*)&y + 1);
    msg[3] = *((char*)&y);
    msg[4] = *((char*)&leftClick);
    return msg;
    
}

char* CursorData::ToMsgWin() const
{

}
