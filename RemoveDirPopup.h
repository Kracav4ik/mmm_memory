#pragma once

#include "Lines.h"
#include "Popup.h"

#include <windows.h>
#include <functional>

class Screen;

class RemoveDirPopup : Popup {
public:
    RemoveDirPopup(SHORT w, SHORT h);

    void setOnUpdateDirs(std::function<void()> func);

    void show(const std::wstring& dir, const std::wstring& name);
    void registerKeys(Screen& screen);
    void drawOn(Screen& screen);

private:
    SHORT w;
    SHORT h;
    Lines text;
    std::wstring toDelete;
    std::function<void()> updateDirs;
};
