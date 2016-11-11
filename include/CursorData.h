#pragma once
class CursorData
{
public:
    CursorData(int _x, int _y, int _state);
    CursorData(const CursorData& ref);
    ~CursorData();
    char* ToMsg() const;
    int x;
    int y;
    int state;
};
