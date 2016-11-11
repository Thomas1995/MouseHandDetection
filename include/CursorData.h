#pragma once
class CursorData
{
public:

    CursorData(int _x = 0, int _y = 0, int _state = 0);
    CursorData(const CursorData& ref);
    ~CursorData();
    char* ToMsg() const;
    int x;
    int y;
    int state;
};
