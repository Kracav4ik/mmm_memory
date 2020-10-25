#include "MemoryViewPanel.h"

#include <sstream>
#include <cmath>

static const uint64_t ZOOM_STEP = 2;
static const std::map<DWORD, Color> REGION_COLORS {
        {PAGE_EXECUTE, Color::White},
        {PAGE_EXECUTE_READ, Color::Blue},
        {PAGE_EXECUTE_READWRITE, Color::Green},
        {PAGE_EXECUTE_WRITECOPY, Color::Cyan},
        {PAGE_NOACCESS, Color::DarkGrey},
        {PAGE_READONLY, Color::DarkBlue},
        {PAGE_READWRITE, Color::DarkGreen},
        {PAGE_WRITECOPY, Color::DarkCyan},
};
static const Color BG_REGION = Color::Black;
static const Color MIXED_REGION = Color::DarkMagenta;
static const Color ERROR_REGION = Color::DarkRed;

MemoryViewPanel::MemoryViewPanel(Rect rect, uint64_t minAddr, uint64_t maxAddr, uint64_t pageSize)
    : rect(rect)
    , beginAddr(minAddr)
    , endAddr(maxAddr + 1)
    , pageSize(pageSize)
    , curOffset(minAddr)
    , cellSize(pageSize)
{
    while (zoomOut()) {
        // do nothing
    }
}

bool MemoryViewPanel::zoomIn() {
    if (cellSize == pageSize) {
        return false;
    }
    cellSize = std::max(pageSize, cellSize / ZOOM_STEP);
    return true;
}

bool MemoryViewPanel::zoomOut() {
    if (getAreaBytes() >= endAddr - beginAddr) {
        return false;
    }
    cellSize *= ZOOM_STEP;
    if (curOffset + getAreaBytes() >= endAddr) {
        toEnd();
    }
    return true;
}

void MemoryViewPanel::upLine() {
    if (curOffset < beginAddr + getLineBytes()) {
        curOffset = beginAddr;
    } else {
        curOffset -= getLineBytes();
    }
}

void MemoryViewPanel::downLine() {
    if (curOffset + getAreaBytes() < endAddr) {
        curOffset += getLineBytes();
    }
}

void MemoryViewPanel::upPage() {
    for (int i = 0; i < pixelRect().h; ++i) {
        upLine();
    }
}

void MemoryViewPanel::downPage() {
    for (int i = 0; i < pixelRect().h; ++i) {
        downLine();
    }
}

void MemoryViewPanel::toBegin() {
    curOffset = 0;
}

void MemoryViewPanel::toEnd() {
    uint64_t linesCount = (uint64_t)ceil((endAddr - beginAddr) / (double)getLineBytes());
    uint64_t line = 0;
    if (linesCount > pixelRect().h) {
        line = linesCount - pixelRect().h;
    }
    curOffset = line * getLineBytes();
}

void MemoryViewPanel::drawOn(Screen& s) {
    s.paintRect(rect, FG::WHITE | BG::GREY);
    s.frame(rect, false);
    s.boundedLine(rect.moved(1, 0).getLeftTop(), rect.w - 2, getTitle(), true);
    Rect pixels = pixelRect();
    s.pixelMap(pixels, getPixels(), Color::Grey);
    s.pixelMap(pixels.moved(pixels.w, 0).withW(1), getScrollbar(), Color::Grey);
}

void MemoryViewPanel::setRegions(const std::vector<Region>& regions) {
    regionOffsets.clear();
    for (const auto& region : regions) {
        regionOffsets.emplace(region.offset, region);
    }
}

Rect MemoryViewPanel::pixelRect() const {
    return rect.withPadding(1, 1);
}

int MemoryViewPanel::getPixelArea() const {
    return pixelRect().getArea() * 2;
}

uint64_t MemoryViewPanel::getLineBytes() const {
    return pixelRect().w * 2 * cellSize;
}

uint64_t MemoryViewPanel::getAreaBytes() const {
    return getPixelArea() * cellSize;
}

std::wstring MemoryViewPanel::getTitle() const {
    std::wstringstream result;
    result << std::hex << L" Смещение 0x" << curOffset << L", клетка 0x" << cellSize << L" ";
    return result.str();
}

Color MemoryViewPanel::getCellColor(uint64_t startAddress) const {
    auto beginIter = regionOffsets.lower_bound(startAddress);
    if (beginIter != regionOffsets.begin()) {
        auto prev = beginIter;
        --prev;
        if (prev->second.offset + prev->second.size > startAddress) {
            beginIter = prev;
        }
    }
    uint64_t endAddress = startAddress + cellSize;
    auto endIter = regionOffsets.lower_bound(endAddress);
    DWORD allModes = 0;
    for (auto iter = beginIter; iter != endIter; ++iter) {
        DWORD mode = iter->second.mode;
        if (mode != 0 && REGION_COLORS.count(mode) == 0) {
            return ERROR_REGION;
        }
        allModes |= mode;
    }
    if (allModes == 0) {
        return BG_REGION;
    }
    auto iter = REGION_COLORS.find(allModes);
    if (iter == REGION_COLORS.end()) {
        return MIXED_REGION;
    }
    return iter->second;
}

std::vector<Color> MemoryViewPanel::getPixels() const {
    std::vector<Color> result;
    uint64_t offset = curOffset;
    for (int i = 0; i < getPixelArea() && offset < endAddr; ++i, offset += cellSize) {
        result.push_back(getCellColor(offset));
    }
    return result;
}

std::vector<Color> MemoryViewPanel::getScrollbar() const {
    std::vector<Color> result(pixelRect().h * 2, Color::DarkGrey);
    double addrDist = endAddr - beginAddr;
    int scrollBegin = (int)std::round(result.size() * (curOffset - beginAddr) / addrDist);
    int scrollSize = std::min((int)result.size(), (int)(result.size() * getAreaBytes() / addrDist));
    scrollBegin = std::min(scrollBegin, (int)result.size() - scrollSize);
    for (int i = scrollBegin; i < scrollBegin + scrollSize; ++i) {
        result[i] = Color::White;
    }
    return result;
}
