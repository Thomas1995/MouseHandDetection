#include "../include/CursorData.h"

CursorData::CursorData(int _x, int _y, int _state) : 
    x(_x), y(_y), state(_state)
{

}

CursorData::CursorData(const CursorData& ref) :
    x(ref.x), y(ref.y), state(ref.state) 
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
    msg[4] = *((char*)&state);
    return msg;
    
}
