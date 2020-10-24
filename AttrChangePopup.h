#pragma once

#include "Lines.h"
#include "Popup.h"
#include "LineEdit.h"

#include <windows.h>

#include <string>

class Screen;

class AttrChangePopup : Popup {
public:
    AttrChangePopup(Screen& screen, SHORT w, SHORT h);

    void show(const std::wstring& root, const std::wstring& name);
    void registerKeys(Screen& screen);
    void drawOn(Screen& screen);

private:
    enum {CREATE, ACCESS, MODIFY};
    bool anyAttrChanged() const;
    bool anyTimeChanged() const;
    bool timeSelected() const;
    void updateAttr();
    void updateTime();
    void fillTimeEdit();
    int parseTime(const std::wstring& s, FILETIME* time = nullptr);

    SHORT w;
    SHORT h;
    std::wstring root;
    std::wstring name;
    DWORD oldAttr = 0;
    DWORD newAttr = 0;
    Lines attrText;
    Lines labelsOver;
    LineEdit timeEdit;
    FILETIME oldTime[3];
    FILETIME newTime[3];
    bool badAttr = false;
    bool badTime = false;
};
