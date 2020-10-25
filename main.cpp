#include <iostream>
#include "Lines.h"
#include "Screen.h"

#include "colors.h"
#include "MessagePopup.h"
#include "MemoryViewPanel.h"

#include <io.h>
#include <fcntl.h>

void _fixwcout() {
    constexpr char cp_utf16le[] = ".1200";
    setlocale( LC_ALL, cp_utf16le );
    _setmode( _fileno(stdout), _O_WTEXT );
    std::wcout << L"\r"; // need to output something for this to work
}

std::wstring getFullPath(const std::wstring& path) {
    DWORD size = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
    std::wstring result(size - 1, L' ');
    GetFullPathNameW(path.c_str(), size, result.data(), nullptr);
    return result;
}

int main() {
    _fixwcout();

    Screen s(80, 25);
    s.setTitle(L"Look at you, hacker");

    std::wstring appDir = getFullPath(L".");

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

    std::vector<Region> regions = {
            {0x0000, 0x004, PAGE_EXECUTE},
            {0x0004, 0x004, PAGE_EXECUTE_READ},
            {0x0008, 0x004, PAGE_EXECUTE_READWRITE},
            {0x000c, 0x004, PAGE_EXECUTE_WRITECOPY},
            {0x0010, 0x004, PAGE_NOACCESS},
            {0x0014, 0x004, PAGE_READONLY},
            {0x0018, 0x004, PAGE_READWRITE},
            {0x001c, 0x004, PAGE_WRITECOPY},

            {0x0100, 0x100, PAGE_EXECUTE},
            {0x1200, 0x200, PAGE_EXECUTE_READ},
            {0x2300, 0x300, PAGE_EXECUTE_READWRITE},
            {0x3400, 0x400, PAGE_EXECUTE_WRITECOPY},
            {0x4500, 0x500, PAGE_NOACCESS},
            {0x5600, 0x600, PAGE_READONLY},
            {0x6700, 0x700, PAGE_READWRITE},
            {0x7800, 0x800, PAGE_WRITECOPY},

            {0x9e00, 0x190, 0xff},
    };

    MemoryViewPanel memoryPanel({30, 0, 50, 25}, 0, 0xffff, 0x04);

    memoryPanel.setRegions(regions);

    // Drawing
    auto repaint = [&]() {
        s.clear(FG::GREY | BG::BLACK);

        memoryPanel.drawOn(s);
        s.pixelMap({10, 8, 16, 8}, pix, Color::DarkBlue);
        MessagePopup::drawOn(s);

        s.flip();
    };

    // Global exit
    s.handlePriorityKey(VK_F10, 0, [&]() {
        running = false;
    });

    // Global message popup
    MessagePopup::registerKeys(s);

    s.handleKey(VK_UP, 0, [&]() {
        memoryPanel.upLine();
    });
    s.handleKey(VK_DOWN, 0, [&]() {
        memoryPanel.downLine();
    });
    s.handleKey(VK_PRIOR, 0, [&]() {
        memoryPanel.upPage();
    });
    s.handleKey(VK_NEXT, 0, [&]() {
        memoryPanel.downPage();
    });
    s.handleKey(VK_HOME, 0, [&]() {
        memoryPanel.toBegin();
    });
    s.handleKey(VK_END, 0, [&]() {
        memoryPanel.toEnd();
    });
    s.handleKey(VK_ADD, 0, [&]() {
        memoryPanel.zoomIn();
    });
    s.handleKey(VK_SUBTRACT, 0, [&]() {
        memoryPanel.zoomOut();
    });

    // Initial state
    repaint();

    // Main loop
    while (running) {
        s.processEvent();
        repaint();
    }
}
