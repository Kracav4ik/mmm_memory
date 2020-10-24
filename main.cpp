#include <iostream>
#include "Lines.h"
#include "Screen.h"

#include "colors.h"
#include "MessagePopup.h"

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

    // Drawing
    auto repaint = [&]() {
        s.clear(FG::GREY | BG::BLACK);

        s.pixelMap({30, 8, 16, 5}, pix, Color::DarkBlue);
        MessagePopup::drawOn(s);

        s.flip();
    };

    // Global exit
    s.handlePriorityKey(VK_F10, 0, [&]() {
        running = false;
    });

    // Global message popup
    MessagePopup::registerKeys(s);

    // Initial state
    repaint();

    // Main loop
    while (running) {
        s.processEvent();
        repaint();
    }
}
