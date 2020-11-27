#include <iostream>
#include "Lines.h"
#include "Screen.h"

#include "colors.h"
#include "MessagePopup.h"
#include "MemoryViewPanel.h"
#include "utils.h"

#include <memory>
#include <cstdio>
#include <io.h>
#include <fcntl.h>

uintptr_t startAddr;
uintptr_t addrToFree = 0;
int protTimes = 0;
std::vector<DWORD> prots = {PAGE_READWRITE, PAGE_READONLY};

void updateRegions(std::vector<Region>& regions, const SYSTEM_INFO& info) {
    regions.clear();
    startAddr = (uintptr_t)info.lpMinimumApplicationAddress;
    uintptr_t endAddr = (uintptr_t)info.lpMaximumApplicationAddress;
    auto mid = (uintptr_t) info.lpMinimumApplicationAddress / 2 + (uintptr_t) info.lpMaximumApplicationAddress / 2 + 1;
    MEMORY_BASIC_INFORMATION basInf;
    while (startAddr <= endAddr) {
        SIZE_T size = VirtualQuery((LPCVOID)startAddr, &basInf, sizeof(MEMORY_BASIC_INFORMATION));

        auto string = getLastErrorText();
//        if (size == 0) {
//            startAddr += info.dwPageSize;
//            totalSize -= info.dwPageSize;
//        } else {
        if (startAddr <= mid && startAddr + basInf.RegionSize > mid) {
//            if (basInf.AllocationBase != nullptr) {
//                mid = (uintptr_t)basInf.BaseAddress;
//            }
            MessagePopup::show({
                to_hex((uintptr_t)addrToFree),
                to_hex((uintptr_t)mid),
                to_hex((uintptr_t)basInf.BaseAddress),
                to_hex((uintptr_t)basInf.AllocationBase),
                std::to_wstring(basInf.AllocationProtect),
                std::to_wstring(basInf.RegionSize),
                std::to_wstring(basInf.State),
                std::to_wstring(basInf.Protect),
                std::to_wstring(basInf.Type),
            }, true);
        }

        RegionMode mode;
        if (basInf.State == MEM_FREE) {
            mode = RegionMode::Free;
        } else if (basInf.State == MEM_RESERVE) {
            mode = RegionMode::Uncommitted;
        } else {
            mode = getModeValue(basInf.Protect);
        }
        regions.push_back({startAddr, basInf.RegionSize, mode});

        startAddr += basInf.RegionSize;
//        }
    }
    startAddr = mid;
}

void allocateMem() {
    addrToFree = (uintptr_t)(VirtualAlloc((LPVOID) startAddr, 32 * 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
//    addrToFree = (uintptr_t)(VirtualAlloc(nullptr, 32 * 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!addrToFree) {
        MessagePopup::show({L"Ошибка вы деления мемов:", getLastErrorText()});
    }
}

void freeMem() {
    if (!addrToFree) return;
    auto addr1 = VirtualFree((LPVOID)addrToFree, 0, MEM_DECOMMIT);
    if (!addr1) {
        MessagePopup::show({L"Ошибка освобождение мемов1:", getLastErrorText()});
        return;
    }
    auto addr2 = VirtualFree((LPVOID)addrToFree, 0, MEM_RELEASE);
    if (!addr2) {
        MessagePopup::show({L"Ошибка освобождение мемов2:", getLastErrorText()});
        return;
    }
    addrToFree = 0;
}

void protectMem() {
    if (!addrToFree) return;
    auto retVal = VirtualProtect((LPVOID)addrToFree, 32 * 1024 * 1024, prots[1-protTimes], &prots[protTimes]);
    if (!retVal) {
        MessagePopup::show({L"Ошибка смены прокта мемов:", getLastErrorText()});
        return;
    }
    protTimes = (protTimes + 1) % 2;
}

void writeToMem() {
    memset((void*)addrToFree, 1, 32 * 1024 * 1024);
}

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
//            {0x0000, 0x004, PAGE_EXECUTE},
//            {0x0004, 0x004, PAGE_EXECUTE_READ},
//            {0x0008, 0x004, PAGE_EXECUTE_READWRITE},
//            {0x000c, 0x004, PAGE_EXECUTE_WRITECOPY},
//            {0x0010, 0x004, PAGE_NOACCESS},
//            {0x0014, 0x004, PAGE_READONLY},
//            {0x0018, 0x004, PAGE_READWRITE},
//            {0x001c, 0x004, PAGE_WRITECOPY},
//
//            {0x0100, 0x100, PAGE_EXECUTE},
//            {0x1200, 0x200, PAGE_EXECUTE_READ},
//            {0x2300, 0x300, PAGE_EXECUTE_READWRITE},
//            {0x3400, 0x400, PAGE_EXECUTE_WRITECOPY},
//            {0x4500, 0x500, PAGE_NOACCESS},
//            {0x5600, 0x600, PAGE_READONLY},
//            {0x6700, 0x700, PAGE_READWRITE},
//            {0x7800, 0x800, PAGE_WRITECOPY},
//
//            {0x9e00, 0x190, 0xff},
    };

    Lines lines;
    SYSTEM_INFO info;
    MEMORYSTATUSEX memstat;
    memstat.dwLength = sizeof(MEMORYSTATUSEX);
    GetSystemInfo(&info);
    GlobalMemoryStatusEx(&memstat);

    MemoryViewPanel memoryPanel({42, 0, 38, 23}, (uintptr_t) info.lpMinimumApplicationAddress, (uintptr_t)info.lpMaximumApplicationAddress, info.dwPageSize);

    std::vector<std::wstring> rows = {
            {std::wstring(L"PageSize: ") + std::to_wstring(info.dwPageSize)},
            {std::wstring(L"MinAddr: ") + to_hex((uintptr_t)info.lpMinimumApplicationAddress)},
            {std::wstring(L"MaxAddr: ") + to_hex((uintptr_t)info.lpMaximumApplicationAddress)},
            {std::wstring(L"ActiveProcessorMask: ") + std::to_wstring(info.dwActiveProcessorMask)},
            {std::wstring(L"NumberOfProcessors: ") + std::to_wstring(info.dwNumberOfProcessors)},
            {std::wstring(L"ProcessorType: ") + std::to_wstring(info.dwProcessorType)},
            {std::wstring(L"AllocationGranularity: ") + std::to_wstring(info.dwAllocationGranularity)},
            {std::wstring(L"ProcessorLevel: ") + std::to_wstring(info.wProcessorLevel)},
            {std::wstring(L"ProcessorRevision: ") + std::to_wstring(info.wProcessorRevision)},
            {std::wstring(L"MemoryLoad: ") + std::to_wstring(memstat.dwMemoryLoad)},
            {std::wstring(L"TotalPhys: ") + std::to_wstring(memstat.ullTotalPhys)},
            {std::wstring(L"AvailPhys: ") + std::to_wstring(memstat.ullAvailPhys)},
            {std::wstring(L"TotalPageFile: ") + std::to_wstring(memstat.ullTotalPageFile)},
            {std::wstring(L"AvailPageFile: ") + std::to_wstring(memstat.ullAvailPageFile)},
            {std::wstring(L"TotalVirtual: ") + std::to_wstring(memstat.ullTotalVirtual)},
            {std::wstring(L"AvailVirtual: ") + std::to_wstring(memstat.ullAvailVirtual)},
            {std::wstring(L"AvailExtendedVirtual: ") + std::to_wstring(memstat.ullAvailExtendedVirtual)},
            };

    lines.setLines(styledText(rows, FG::WHITE | BG::BLACK));

    // Drawing
    auto repaint = [&]() {
        s.clear(FG::GREY | BG::BLACK);

        memoryPanel.drawOn(s);
        s.labelsFill({0, 24, 80, 1}, {
                L"Alt-F1/F2 Диск",
                L"F2 Новый",
                L"F4 Атр.",
                L"F5 Копир.",
                L"F6 Перен.",
                L"F7 Папка",
                L"F8 Удал.",
                L"F10 Выход",
        }, FG::BLACK | BG::DARK_CYAN);
        s.pixelMap({10, 8, 16, 8}, pix, Color::DarkBlue);
        lines.drawOn(s, {0, 0, 30, 28});
        MessagePopup::drawOn(s);

        s.flip();
    };

    // Refresh memory regions
    auto refreshRegions = [&]() {
        updateRegions(regions, info);
        memoryPanel.setRegions(regions);
    };

    // Global exit
    s.handlePriorityKey(VK_F10, 0, [&]() {
        running = false;
    });

    // Global message popup
    MessagePopup::registerKeys(s);

    s.handleKey(VK_F1, 0, [&]() {
        allocateMem();
    });

    s.handleKey(VK_F2, 0, [&]() {
        refreshRegions();
    });

    s.handleKey(VK_F3, 0, [&]() {
        freeMem();
    });

    s.handleKey(VK_F4, 0, [&]() {
        protectMem();
    });

    s.handleKey(VK_F5, 0, [&]() {
        writeToMem();
    });

    s.handleKey(VK_LEFT, 0, [&]() {
        memoryPanel.upCell();
    });
    s.handleKey(VK_RIGHT, 0, [&]() {
        memoryPanel.downCell();
    });
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
    refreshRegions();
    repaint();

    // Main loop
    while (running) {
        s.processEvent();
        repaint();
    }
}
