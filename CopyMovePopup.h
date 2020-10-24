#pragma once

#include "Lines.h"
#include "Popup.h"
#include "LineEdit.h"

class Screen;

class CopyMovePopup : Popup {
public:
    CopyMovePopup(Screen& screen, SHORT w, SHORT h);

    void show(bool isCopy, const std::wstring& fromRoot, const std::wstring& fromName, const std::wstring& toRoot);
    void registerKeys(Screen& screen);
    void drawOn(Screen& screen);

    void setOnUpdateDirs(std::function<void()> func);

private:
    SHORT w;
    SHORT h;
    bool showCopy = false;
    std::wstring oldName;
    std::wstring oldRoot;
    LineEdit newName;
    std::wstring newRoot;
    std::function<void()> updateDirs;
};
