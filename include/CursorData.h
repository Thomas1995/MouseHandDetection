#pragma once
class CursorData
{
public:

    CursorData(int _x = 0, int _y = 0, int leftClick = 0, int _rightClick = 0, int key = 0);
    CursorData(const CursorData& ref);
    ~CursorData();
    char* ToMsg() const;
    char* ToMsgWin() const;
    int x;
    int y;
    int leftClick;
    int rightClick;
    int key;
};
