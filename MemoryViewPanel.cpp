#include "MemoryViewPanel.h"

#include "utils.h"

#include <sstream>
#include <cmath>

static const uint64_t ZOOM_STEP = 2;

static const Color MIXED_REGION = Color::DarkMagenta;
static const Color UNKNOWN_REGION = Color::Magenta;

static const std::map<RegionMode, Color> REGION_COLORS {
        {RegionMode::PageNoAccess, Color::DarkGrey},
        {RegionMode::PageReadonly, Color::DarkBlue},
        {RegionMode::PageReadWrite, Color::DarkGreen},
        {RegionMode::PageCopyOnWrite, Color::DarkCyan},

        {RegionMode::PageExecuteNoAccess, Color::White},
        {RegionMode::PageExecuteReadonly, Color::Blue},
        {RegionMode::PageExecuteReadWrite, Color::Green},
        {RegionMode::PageExecuteCopyOnWrite, Color::Cyan},

        {RegionMode::PageGuard, Color::DarkRed},
        {RegionMode::ForbiddenRegion, Color::Red},

        {RegionMode::Free, Color::Black},
        {RegionMode::Uncommitted, Color::DarkYellow},
        {RegionMode::Selection, Color::Yellow},
        {RegionMode::UnknownValue, UNKNOWN_REGION},
};

static std::wstring getCellStr(uint64_t cell) {
    static const uint64_t K = 1024;
    static const uint64_t M = K*1024;
    static const uint64_t G = M*1024;
    std::wstringstream result;
    if (cell < K) {
        result << cell << L"Б";
    } else  if (cell < M) {
        result << cell / K << L"К";
    } else if (cell < G) {
        result << cell / M << L"М";
    } else {
        result << cell / G << L"Г";
    }
    return result.str();
}

RegionMode getModeValue(DWORD mode) {
    if (mode & PAGE_GUARD) {
        return RegionMode::PageGuard;
    }
    switch (mode) {
        case 0: return RegionMode::ForbiddenRegion;
        case PAGE_NOACCESS: return RegionMode::PageNoAccess;
        case PAGE_READONLY: return RegionMode::PageReadonly;
        case PAGE_READWRITE: return RegionMode::PageReadWrite;
        case PAGE_WRITECOPY: return RegionMode::PageCopyOnWrite;
        case PAGE_EXECUTE: return RegionMode::PageExecuteNoAccess;
        case PAGE_EXECUTE_READ: return RegionMode::PageExecuteReadonly;
        case PAGE_EXECUTE_READWRITE: return RegionMode::PageExecuteReadWrite;
        case PAGE_EXECUTE_WRITECOPY: return RegionMode::PageExecuteCopyOnWrite;
        default: return RegionMode::UnknownValue;
    }
}

Region selectionRegion(uint64_t offset, uint64_t size) {
    return {offset, size, RegionMode::Selection};
}

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

void MemoryViewPanel::upCell() {
    if (curOffset < beginAddr + cellSize) {
        curOffset = beginAddr;
    } else {
        curOffset -= cellSize;
    }
}

void MemoryViewPanel::downCell() {
    if (curOffset + getAreaBytes() + cellSize < endAddr + getLineBytes()) {
        curOffset += cellSize;
    }
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
    for (int i = 0; i < getLinesCount()/2; ++i) {
        upLine();
    }
}

void MemoryViewPanel::downPage() {
    for (int i = 0; i < getLinesCount()/2; ++i) {
        downLine();
    }
}

void MemoryViewPanel::toBegin() {
    curOffset = beginAddr;
}

void MemoryViewPanel::toEnd() {
    uint64_t linesCount = (uint64_t)ceil((endAddr - beginAddr) / (double)getLineBytes());
    uint64_t line = 0;
    if (linesCount > getLinesCount()) {
        line = linesCount - getLinesCount();
    }
    curOffset = beginAddr + line * getLineBytes();
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

int MemoryViewPanel::getLinesCount() const {
    return 2*pixelRect().h;
}

uint64_t MemoryViewPanel::getLineBytes() const {
    return pixelRect().w * cellSize;
}

uint64_t MemoryViewPanel::getAreaBytes() const {
    return getPixelArea() * cellSize;
}

std::wstring MemoryViewPanel::getTitle() const {
    std::wstringstream result;
    result << std::hex << L" Смещение 0x" << curOffset << L" клетка " << getCellStr(cellSize) << L" ";
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
    std::optional<RegionMode> commonMode;
    for (auto iter = beginIter; iter != endIter; ++iter) {
        RegionMode mode = iter->second.mode;
        if (commonMode && *commonMode != mode) {
            return MIXED_REGION;
        }
        commonMode = mode;
    }
    if (commonMode) {
        auto iter = REGION_COLORS.find(*commonMode);
        if (iter != REGION_COLORS.end()) {
            return iter->second;
        }
    }
    return UNKNOWN_REGION;
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
    int linesCount = getLinesCount();
    std::vector<Color> result(linesCount, Color::Grey);
    double addrDist = endAddr - beginAddr;
    int scrollBegin = roundI(linesCount * (curOffset - beginAddr) / addrDist);
    int scrollSize = clamp(1, roundI(linesCount * getAreaBytes() / addrDist), (int)result.size());
    scrollBegin = std::min(scrollBegin, linesCount - scrollSize);
    for (int i = scrollBegin; i < scrollBegin + scrollSize; ++i) {
        result[i] = Color::White;
    }
    return result;
}
