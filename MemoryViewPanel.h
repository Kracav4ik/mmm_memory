#pragma once

#include "Screen.h"
#include <vector>

struct Region {
    uint64_t offset;
    uint64_t size;
    DWORD mode;
};

class MemoryViewPanel {
public:
    MemoryViewPanel(Rect rect, uint64_t minAddr, uint64_t maxAddr, uint64_t pageSize);

    bool zoomIn();
    bool zoomOut();
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
