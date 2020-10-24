#include <iostream>
#include "Lines.h"
#include "Screen.h"

#include "colors.h"
#include "FilePanel.h"
#include "DiskPopup.h"
#include "MessagePopup.h"
#include "AttrChangePopup.h"
#include "CopyMovePopup.h"
#include "MakeDirPopup.h"
#include "RemoveDirPopup.h"
#include "MakeFilePopup.h"

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
    s.setTitle(L"Not too far");

    std::wstring appDir = getFullPath(L".");

    bool running = true;
    FilePanel leftPanel({0, 0, 40, 23}, appDir);
    FilePanel rightPanel({40, 0, 40, 23}, appDir);
    DiskPopup diskPopup(3, 27, 3, 55, 18);
    AttrChangePopup attrChangePopup(s, 50, 16);
    CopyMovePopup copyMovePopup(s, 70, 10);
    MakeDirPopup makeDirPopup(s, 70, 10);
    MakeFilePopup makeFilePopup(s, 70, 10);
    RemoveDirPopup removeDirPopup(70, 10);
    Lines bottom;

    // Drawing
    auto repaint = [&]() {
        s.clear(FG::GREY | BG::BLACK);

        leftPanel.drawOn(s);
        rightPanel.drawOn(s);
        bottom.drawOn(s, {0, 23, 80, 1});
        s.labelsFill({0, (SHORT)(s.h() - 1), s.w(), 1}, {
            L"Alt-F1/F2 Диск",
            L"F2 Новый",
            L"F4 Атр.",
            L"F5 Копир.",
            L"F6 Перен.",
            L"F7 Папка",
            L"F8 Удал.",
            L"F10 Выход",
        }, FG::BLACK | BG::DARK_CYAN);

        diskPopup.drawOn(s);
        attrChangePopup.drawOn(s);
        copyMovePopup.drawOn(s);
        makeDirPopup.drawOn(s);
        makeFilePopup.drawOn(s);
        removeDirPopup.drawOn(s);
        MessagePopup::drawOn(s);

        s.flip();
    };

    // Bottom panel
    auto getCurrentPanel = [&]() -> FilePanel& {
        return rightPanel.hasSelection() ? rightPanel : leftPanel;
    };
    auto getOtherPanel = [&]() -> FilePanel& {
        return rightPanel.hasSelection() ? leftPanel : rightPanel;
    };
    auto updateBottom = [&]() {
        std::wstring path = getCurrentPanel().getPath();
        if (path[path.size() - 1] == L':') {
            path += L'\\';
        }
        path += L'>';
        bottom.setLines(styledText({ std::move(path) }, FG::GREY | BG::BLACK));
    };

    // Global exit
    s.handlePriorityKey(VK_F10, 0, [&]() {
        running = false;
    });

    // Global message popup
    MessagePopup::registerKeys(s);

    // Popup controls
    diskPopup.registerKeys(s);
    attrChangePopup.registerKeys(s);
    copyMovePopup.registerKeys(s);
    makeDirPopup.registerKeys(s);
    makeFilePopup.registerKeys(s);
    removeDirPopup.registerKeys(s);

    diskPopup.setOnSelectFunc([&]() {
        auto& panel = diskPopup.isLeftPopup() ? leftPanel : rightPanel;
        std::wstring selectedDisk = diskPopup.selectedDisk();
        if (panel.getPath()[0] == selectedDisk[0]) {
            return;
        }
        panel.setPath(selectedDisk);
        updateBottom();
    });
    s.handleKey(VK_F1, ANY_ALT_PRESSED, [&]() {
        diskPopup.show(true);
    });
    s.handleKey(VK_F2, ANY_ALT_PRESSED, [&]() {
        diskPopup.show(false);
    });

    auto updateDirs = [&]() {
        leftPanel.updateLines();
        rightPanel.updateLines();
    };
    makeDirPopup.setOnUpdateDirs(updateDirs);
    makeFilePopup.setOnUpdateDirs(updateDirs);
    removeDirPopup.setOnUpdateDirs(updateDirs);
    copyMovePopup.setOnUpdateDirs(updateDirs);

    s.handleKey(VK_F2, 0, [&]() {
        makeFilePopup.show(getCurrentPanel().getPath());
    });

    s.handleKey(VK_F4, 0, [&]() {
        auto& current = getCurrentPanel();
        auto name = current.getSelectedText();
        if (name != L"..") {
            attrChangePopup.show(current.getPath(), name);
        }
    });
    s.handleKey(VK_F5, 0, [&]() {
        auto& current = getCurrentPanel();
        auto& other = getOtherPanel();
        auto name = current.getSelectedText();
        if (name != L"..") {
            copyMovePopup.show(true, current.getPath(), name, other.getPath());
        }
    });
    s.handleKey(VK_F6, 0, [&]() {
        auto& current = getCurrentPanel();
        auto& other = getOtherPanel();
        auto name = current.getSelectedText();
        if (name != L"..") {
            copyMovePopup.show(false, current.getPath(), name, other.getPath());
        }
    });
    s.handleKey(VK_F7, 0, [&]() {
        makeDirPopup.show(getCurrentPanel().getPath());
    });
    s.handleKey(VK_F8, 0, [&]() {
        auto& current = getCurrentPanel();
        auto& other = getOtherPanel();
        auto name = current.getSelectedText();
        if (name != L"..") {
            removeDirPopup.show(current.getPath(), name);
        }
    });

    // Panel controls
    s.handleKey(VK_TAB, 0, [&]() {
        leftPanel.hasSelection() ? leftPanel.unselect() : leftPanel.select();
        rightPanel.hasSelection() ? rightPanel.unselect() : rightPanel.select();
        updateBottom();
    });
    s.handleKey(VK_UP, 0, [&]() {
        getCurrentPanel().selectPrev();
    });
    s.handleKey(VK_DOWN, 0, [&]() {
        getCurrentPanel().selectNext();
    });
    s.handleKey(VK_PRIOR, 0, [&]() {
        getCurrentPanel().selectPageUp();
    });
    s.handleKey(VK_NEXT, 0, [&]() {
        getCurrentPanel().selectPageDown();
    });
    s.handleKey(VK_HOME, 0, [&]() {
        getCurrentPanel().selectFirst();
    });
    s.handleKey(VK_END, 0, [&]() {
        getCurrentPanel().selectLast();
    });

    s.handleKey(VK_RETURN, 0, [&]() {
        getCurrentPanel().enter();
        updateBottom();
    });

    s.handleKey(VK_SPACE, 0, [&]() {
        MessagePopup::show({L"Ш     Т     О     Ш"});
    });

    // Initial state
    leftPanel.select();
    updateBottom();
    repaint();

    // Main loop
    while (running) {
        s.processEvent();
        repaint();
    }
}
