#include <iostream>

#include "FilePanel.h"
#include "colors.h"
#include "MessagePopup.h"
#include "utils.h"

static const SHORT BOTTOM = 3;

struct ListDirsResults {
    bool isRoot;
    std::vector<std::wstring> dirs;
    std::vector<std::wstring> files;
};

ListDirsResults getListDirs(const std::wstring& path) {
    bool isRoot = path.size() == 2 && path[1] == L':';
    std::vector<std::wstring> dirs;
    std::vector<std::wstring> files;

    std::wstring mask = path + L"\\*";
    WIN32_FIND_DATAW findData;
    HANDLE find = FindFirstFileW(mask.c_str(), &findData);

    do {
        std::wstring name = findData.cFileName;
        if (name == L"." || name == L"..") {  // ".." entry reported for S:\ the root of a network drive
            continue;
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            dirs.push_back(std::move(name));
        } else {
            files.push_back(std::move(name));
        }
    } while (FindNextFileW(find, &findData) != 0);

    FindClose(find);
    auto compareFunc = [](const std::wstring& s1, const std::wstring& s2) {
        int result = CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE | SORT_STRINGSORT, s1.c_str(), s1.size(), s2.c_str(), s2.size());
        return result == CSTR_LESS_THAN;
    };
    std::sort(files.begin(), files.end(), compareFunc);
    std::sort(dirs.begin(), dirs.end(), compareFunc);

    return {isRoot, dirs, files};
}

bool canChangeDir(const std::wstring& path) {
    std::wstring mask = path + L"\\*";
    WIN32_FIND_DATAW findData;
    HANDLE h = FindFirstFileW(mask.c_str(), &findData);
    if (h == INVALID_HANDLE_VALUE ) {
        return false;
    }
    FindClose(h);
    return true;
}

FilePanel::FilePanel(Rect rect, std::wstring initialPath): rect(rect) {
    setPath(std::move(initialPath));
}

std::wstring FilePanel::getSelectedText() const {
    if (!hasSelection()) {
        return L"";
    }
    return lines.getSelectedText();
}

void FilePanel::selectPrev() {
    lines.selectPrev();
    scrollToSelection();
}

void FilePanel::selectNext() {
    lines.selectNext();
    scrollToSelection();
}

void FilePanel::selectPageUp() {
    lines.moveSelection(-visibleHeight() + 1);
    scrollToSelection();
}

void FilePanel::selectPageDown() {
    lines.moveSelection(visibleHeight() - 1);
    scrollToSelection();
}

void FilePanel::selectFirst() {
    lines.selectFirst();
    scrollToSelection();
}

void FilePanel::selectLast() {
    lines.selectLast();
    scrollToSelection();
}

void FilePanel::enter() {
    if (lines.hasSelection() && lines.getSelectedIdx() < dirsCount) {
        std::wstring selectedName = lines.getSelectedText();
        std::wstring currentDir = path.substr(path.rfind(L'\\') + 1);
        std::wstring newPath;
        if (selectedName == L"..") {
            newPath = path.substr(0, path.rfind(L'\\'));
        } else {
            newPath = path + L'\\' + selectedName;
        }
        if (!canChangeDir(newPath)) {
            MessagePopup::show({L"Невозможно зайти в папку", newPath, getLastErrorText()});
            return;
        }
        path = std::move(newPath);
        updateLines();
        int newSelection = 0;
        if (selectedName == L"..") {
            int nameIdx = lines.findLine(currentDir);
            if (nameIdx != -1) {
                newSelection = nameIdx;
            }
        }
        lines.setSelectedIdx(newSelection);
        scrollToSelection();
    }
}

void FilePanel::drawOn(Screen& s) {
    s.paintRect(rect, FG::CYAN | BG::DARK_BLUE);
    Rect linesRect = rect.withPadding(1, 1);
    linesRect = linesRect.withH(linesRect.h - BOTTOM);
    lines.drawOn(s, linesRect);
    s.frame(rect);
    Rect separatorRect = linesRect.moved(0, linesRect.h).withH(1).withPadX(-1);
    s.separator(separatorRect);
    std::wstring title = L" " + path;

    if (path[path.size() - 1] == L':') {
       title += L"\\";
    }
    title += L" ";
    s.boundedLine(rect.moved(1, 0).getLeftTop(), rect.w - 2, title, true);
}

const std::wstring& FilePanel::getPath() const {
    return path;
}

void FilePanel::setPath(std::wstring newPath) {
    if (!canChangeDir(newPath)) {
        return;
    }
    path = std::move(newPath);
    updateLines();
    bool wasSelected = hasSelection();
    lines.setSelectedIdx(0);
    scrollToSelection();
    if (!wasSelected) {
        unselect();
    }
}

bool FilePanel::hasSelection() const {
    return lines.hasSelection();
}

void FilePanel::select() {
    lines.setSelectedIdx(lastSelectedIdx);
}

void FilePanel::unselect() {
    lastSelectedIdx = lines.getSelectedIdx();
    lines.unselect();
}

void FilePanel::updateLines() {
    auto listDirs = getListDirs(path);
    std::vector<StyledText> newLines;

    if (!listDirs.isRoot) {
        newLines.push_back({L"..", FG::WHITE | BG::DARK_BLUE, FG::WHITE | BG::DARK_CYAN});
    }

    for (auto&& dir : listDirs.dirs) {
        newLines.push_back({std::move(dir), FG::WHITE | BG::DARK_BLUE, FG::WHITE | BG::DARK_CYAN});
    }

    dirsCount = newLines.size();

    for (auto&& file : listDirs.files) {
        newLines.push_back({std::move(file), FG::CYAN | BG::DARK_BLUE, FG::BLACK | BG::DARK_CYAN});
    }

    lines.setLines(newLines);
}

int FilePanel::visibleHeight() const {
    return rect.h - 2 - BOTTOM;
}

void FilePanel::scrollToSelection() {
    lines.scrollToSelection(visibleHeight());
}
