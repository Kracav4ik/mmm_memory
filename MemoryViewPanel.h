#pragma once

#include "Screen.h"
#include <vector>

enum class RegionMode {
    PageNoAccess,
    PageReadonly,
    PageReadWrite,
    PageCopyOnWrite,
    PageExecuteNoAccess,
    PageExecuteReadonly,
    PageExecuteReadWrite,
    PageExecuteCopyOnWrite,
    PageGuard,              // MEMORY_BASIC_INFORMATION::Protect & PAGE_GUARD != 0
    ForbiddenRegion,        // MEMORY_BASIC_INFORMATION::Protect == 0
    Free,
    Uncommitted,
    Selection,
    UnknownValue,
};

RegionMode getModeValue(DWORD mode);

struct Region {
    uint64_t offset;
    uint64_t size;
    RegionMode mode;
};

Region selectionRegion(uint64_t offset, uint64_t size);

class MemoryViewPanel {
public:
    MemoryViewPanel(Rect rect, uint64_t minAddr, uint64_t maxAddr, uint64_t pageSize);

    bool zoomIn();
    bool zoomOut();
    void upCell();
    void downCell();
    void upLine();
    void downLine();
    void upPage();
    void downPage();
    void toBegin();
    void toEnd();

    void drawOn(Screen& s);
    void setRegions(const std::vector<Region>& regions);

private:
    Rect pixelRect() const;
    int getPixelArea() const;
    int getLinesCount() const;
    uint64_t getLineBytes() const;
    uint64_t getAreaBytes() const;
    std::wstring getTitle() const;
    Color getCellColor(uint64_t startAddress) const;
    std::vector<Color> getPixels() const;
    std::vector<Color> getScrollbar() const;

    Rect rect;
    uint64_t beginAddr;
    uint64_t endAddr;
    uint64_t pageSize;
    uint64_t curOffset;
    uint64_t cellSize;
    std::map<uint64_t, Region> regionOffsets;
};
