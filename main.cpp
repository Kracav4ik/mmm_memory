#include <iostream>
#include "Lines.h"
#include "Screen.h"

#include "colors.h"
#include "MessagePopup.h"
#include "MemoryViewPanel.h"
#include "LeftPanel.h"
#include "LegendPanel.h"
#include "MemAllocPanel.h"
#include "MemFreePanel.h"
#include "MemProtectPanel.h"
#include "MemWritePanel.h"
#include "SystemInfoPanel.h"
#include "utils.h"

#include <memory>
#include <cstdio>
#include <io.h>
#include <fcntl.h>

void _fixwcout() {
    constexpr char cp_utf16le[] = ".1200";
    setlocale( LC_ALL, cp_utf16le );
    _setmode( _fileno(stdout), _O_WTEXT );
    std::wcout << L"\r"; // need to output something for this to work
}

int main() {
    _fixwcout();

    Screen s(80, 25);
    s.setTitle(L"Mmm... memory~");

    bool running = true;

    std::vector<Color> pix = {
            Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White,
            Color::White, Color::White, Color::White, Color::White, Color::White, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::White, Color::White, Color::Yellow, Color::Yellow, Color::Yellow, Color::White,
            Color::White, Color::White, Color::White, Color::White, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::Yellow, Color::Yellow, Color::White,
            Color::White, Color::White, Color::White, Color::White, Color::DarkYellow, Color::DarkYellow, Color::DarkYellow, Color::Yellow, Color::Yellow, Color::Black, Color::Yellow, Color::White, Color::Red, Color::Red, Color::Red, Color::White,
            Color::White, Color::White, Color::White, Color::DarkYellow, Color::Yellow, Color::DarkYellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Black, Color::Yellow, Color::Yellow, Color::Yellow, Color::Red, Color::Red, Color::White,
            Color::White, Color::White, Color::White, Color::DarkYellow, Color::Yellow, Color::DarkYellow, Color::DarkYellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Black, Color::Yellow, Color::Yellow, Color::Yellow, Color::Red, Color::White,
            Color::White, Color::White, Color::White, Color::DarkYellow, Color::DarkYellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Black, Color::Black, Color::Black, Color::Black, Color::Red, Color::White, Color::White,
            Color::White, Color::White, Color::White, Color::White, Color::White, Color::Yellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Yellow, Color::Red, Color::Red, Color::White, Color::White,
            Color::White, Color::White, Color::Red, Color::Red, Color::Red, Color::Red, Color::Blue, Color::Red, Color::Red, Color::Red, Color::Blue, Color::Red, Color::Red, Color::White, Color::White, Color::DarkYellow,
            Color::Yellow, Color::Yellow, Color::Red, Color::Red, Color::Red, Color::Red, Color::Red, Color::Blue, Color::Red, Color::Red, Color::Red, Color::Blue, Color::White, Color::White, Color::DarkYellow, Color::DarkYellow,
            Color::Yellow, Color::Yellow, Color::Yellow, Color::Red, Color::Red, Color::Red, Color::Red, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Cyan, Color::Blue, Color::Blue, Color::DarkYellow, Color::DarkYellow,
            Color::White, Color::Yellow, Color::White, Color::White, Color::Blue, Color::Red, Color::Blue, Color::Blue, Color::Cyan, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::DarkYellow, Color::DarkYellow,
            Color::White, Color::White, Color::DarkYellow, Color::DarkYellow, Color::DarkYellow, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::DarkYellow, Color::DarkYellow,
            Color::White, Color::DarkYellow, Color::DarkYellow, Color::DarkYellow, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::Blue, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White,
            Color::White, Color::DarkYellow, Color::DarkYellow, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White,
            Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White,
    };

    SYSTEM_INFO info;
    GetSystemInfo(&info);

    MemoryViewPanel memoryPanel({42, 0, 38, 23}, info);

    LegendPanel legendPanel;
    MemAllocPanel allocPanel(s, info);
    MemFreePanel freePanel(s, info);
    MemProtectPanel protectPanel(s, info);
    MemWritePanel writePanel(s, info);
    SystemInfoPanel infoPanel;

    // Drawing
    auto repaint = [&]() {
        s.clear(FG::GREY | BG::BLACK);

        memoryPanel.drawOn(s);
        LeftPanel::drawVisibleOn(s);
        s.labelsFill({0, 24, 80, 1}, {
                L"F1 Легенда",
                L"F2 Инфо",
                L"F4 Резерв.",
                L"F5 Запись",
                L"F6 Защита",
                L"F8 Возврат",
                L"F10 Выход",
        }, FG::BLACK | BG::DARK_CYAN);
//        s.pixelMap({10, 8, 16, 8}, pix, Color::DarkBlue);
        MessagePopup::drawOn(s);

        s.flip();
    };

    // Global exit
    s.handlePriorityKey(VK_F10, 0, [&]() {
        running = false;
    });

    // Global message popup
    MessagePopup::registerKeys(s);

    s.handleKey(VK_F1, 0, [&]() {
        legendPanel.setVisible();
    });

    s.handleKey(VK_F2, 0, [&]() {
        infoPanel.setVisible();
    });

    s.handleKey(VK_F4, 0, [&]() {
        allocPanel.setVisible();
    });

    s.handleKey(VK_F5, 0, [&]() {
        writePanel.setVisible();
    });

    s.handleKey(VK_F6, 0, [&]() {
        protectPanel.setVisible();
    });

    s.handleKey(VK_F8, 0, [&]() {
        freePanel.setVisible();
    });

    memoryPanel.registerKeys(s);
    allocPanel.registerKeys(s);
    freePanel.registerKeys(s);
    protectPanel.registerKeys(s);
    writePanel.registerKeys(s);

    // Initial state
    infoPanel.setVisible();
    repaint();

    // Main loop
    while (running) {
        s.processEvent();
        repaint();
    }
}
