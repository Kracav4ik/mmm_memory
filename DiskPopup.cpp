#include "DiskPopup.h"

#include "Screen.h"
#include "colors.h"
#include "utils.h"

#include <iostream>
#include <iomanip>

static const int TYPE_W = 20;
static const int TOTAL_W = 12;
static const int FREE_W = 12;

std::wstring getDriveType(const std::wstring& path) {
    switch (GetDriveTypeW(path.c_str())) {
        case DRIVE_NO_ROOT_DIR: return L"Путь не является диском";
        case DRIVE_REMOVABLE: return L"Сменный диск";
        case DRIVE_FIXED: return L"Жесткий диск";
        case DRIVE_REMOTE: return L"Сетевой диск";
        case DRIVE_CDROM: return L"CD-ROM";
        case DRIVE_RAMDISK: return L"RAM-диск";
        default: return L"Неизвестный";
    }
}

std::wstring getByteSizeText(uint64_t num) {
    static const uint64_t K = 1024;
    static const uint64_t M = K*1024;
    static const uint64_t G = M*1024;
    static const uint64_t T = G*1024;
    std::wstringstream result;
    result << std::setprecision(2) << std::fixed;
    if (num < K) {
        result << num << L" Б";
    } else  if (num < M) {
        result << num / (double)K << L" К";
    } else if (num < G) {
        result << num / (double)M << L" М";
    } else if (num < T) {
        result << num / (double)G << L" Г";
    } else {
        result << num / (double)T << L" Т";
    }
    return result.str();
}

std::vector<std::wstring> getFlagsStrings(DWORD sysFlags) {
    std::vector<std::wstring> words;
    if (sysFlags & FILE_CASE_PRESERVED_NAMES) words.emplace_back(L"CASE_NAMES");
    if (sysFlags & FILE_CASE_SENSITIVE_SEARCH) words.emplace_back(L"CASE_SEARCH");
    if (sysFlags & FILE_FILE_COMPRESSION) words.emplace_back(L"FILE_COMPR");
    if (sysFlags & FILE_NAMED_STREAMS) words.emplace_back(L"NAMED_STREAMS");
    if (sysFlags & FILE_PERSISTENT_ACLS) words.emplace_back(L"PERSIST_ACLS");
    if (sysFlags & FILE_READ_ONLY_VOLUME) words.emplace_back(L"RO_VOLUME");
    if (sysFlags & FILE_SEQUENTIAL_WRITE_ONCE) words.emplace_back(L"SEQ_WRITE_ONCE");
    if (sysFlags & FILE_SUPPORTS_ENCRYPTION) words.emplace_back(L"ENCRYPT");
    if (sysFlags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) words.emplace_back(L"EXT_ATTR");
    if (sysFlags & FILE_SUPPORTS_HARD_LINKS) words.emplace_back(L"HARD_LINKS");
    if (sysFlags & FILE_SUPPORTS_OBJECT_IDS) words.emplace_back(L"OBJECT_IDS");
    if (sysFlags & FILE_SUPPORTS_OPEN_BY_FILE_ID) words.emplace_back(L"OPEN_BY_FILE_ID");
    if (sysFlags & FILE_SUPPORTS_REPARSE_POINTS) words.emplace_back(L"REPARSE_POINTS");
    if (sysFlags & FILE_SUPPORTS_SPARSE_FILES) words.emplace_back(L"SPARSE_FILES");
    if (sysFlags & FILE_SUPPORTS_TRANSACTIONS) words.emplace_back(L"TRANSACTIONS");
    if (sysFlags & FILE_SUPPORTS_USN_JOURNAL) words.emplace_back(L"USN_JOURNAL");
    if (sysFlags & FILE_UNICODE_ON_DISK) words.emplace_back(L"UNICODE");
    if (sysFlags & FILE_VOLUME_IS_COMPRESSED) words.emplace_back(L"VOLUME_COMPR");
    if (sysFlags & FILE_VOLUME_QUOTAS) words.emplace_back(L"VOLUME_QUOTAS");

    std::vector<std::wstring> lines;

    std::wstring currentString = L"Флаги ФС:";
    for (const auto& word : words) {
        if ((currentString + word).size() > 47) {
            lines.emplace_back(currentString);
            currentString = word;
        } else {
            if (!currentString.empty()) {
                currentString += L" ";
            }
            currentString += word;
        }
    }
    lines.emplace_back(currentString);
    return lines;
}
std::vector<std::wstring> getDriveString() {
    DWORD size = GetLogicalDriveStringsW(0, nullptr);
    std::vector<wchar_t> chars(size);
    GetLogicalDriveStringsW(size, chars.data());

    std::vector<std::wstring> result;

    int start = 0;
    for (int i = start; i < size - 1; ++i) {
        if (chars[i] == L'\0') {
            std::wstring rootName = chars.data() + start;

            DWORD sectorsPerCluster;
            DWORD bytesPerSector;
            DWORD numberOfFreeClusters;
            DWORD totalNumberOfClusters;

            GetDiskFreeSpaceW(rootName.data(), &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters);

            /*
            std::wcout << L"rootName: " << rootName << std::endl;
            std::wcout << L"  sectorsPerCluster: " << sectorsPerCluster << std::endl;
            std::wcout << L"  bytesPerSector: " << bytesPerSector << std::endl;
            std::wcout << L"  numberOfFreeClusters: " << numberOfFreeClusters << std::endl;
            std::wcout << L"  totalNumberOfClusters: " << totalNumberOfClusters << std::endl;

            std::vector<wchar_t> volumeInfo(MAX_PATH + 1);
            std::vector<wchar_t> fileSysName(MAX_PATH + 1);
            DWORD serialNum;
            DWORD maxCompLen;
            DWORD sysFlags;

            GetVolumeInformationW(rootName.data(), volumeInfo.data(), volumeInfo.size(), &serialNum, &maxCompLen, &sysFlags, fileSysName.data(), fileSysName.size());

            std::wcout << L"rootName: " << rootName << std::endl;
            std::wcout << L"  volumeInfo: " << volumeInfo.data() << std::endl;
            std::wcout << L"  fileSysName: " << fileSysName.data() << std::endl;
            std::wcout << L"  serialNum: " << serialNum << std::endl;
            std::wcout << L"  maxCompLen: " << maxCompLen << std::endl;
            std::wcout << L"  sysFlags: " << sysFlags << std::endl;
            

            if (sysFlags & FILE_CASE_PRESERVED_NAMES) std::wcout << L"    FILE_CASE_PRESERVED_NAMES" << std::endl;
            if (sysFlags & FILE_CASE_SENSITIVE_SEARCH) std::wcout << L"    FILE_CASE_SENSITIVE_SEARCH" << std::endl;
            if (sysFlags & FILE_FILE_COMPRESSION) std::wcout << L"    FILE_FILE_COMPRESSION" << std::endl;
            if (sysFlags & FILE_NAMED_STREAMS) std::wcout << L"    FILE_NAMED_STREAMS" << std::endl;
            if (sysFlags & FILE_PERSISTENT_ACLS) std::wcout << L"    FILE_PERSISTENT_ACLS" << std::endl;
            if (sysFlags & FILE_READ_ONLY_VOLUME) std::wcout << L"    FILE_READ_ONLY_VOLUME" << std::endl;
            if (sysFlags & FILE_SEQUENTIAL_WRITE_ONCE) std::wcout << L"    FILE_SEQUENTIAL_WRITE_ONCE" << std::endl;
            if (sysFlags & FILE_SUPPORTS_ENCRYPTION) std::wcout << L"    FILE_SUPPORTS_ENCRYPTION" << std::endl;
            if (sysFlags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) std::wcout << L"    FILE_SUPPORTS_EXTENDED_ATTRIBUTES" << std::endl;
            if (sysFlags & FILE_SUPPORTS_HARD_LINKS) std::wcout << L"    FILE_SUPPORTS_HARD_LINKS" << std::endl;
            if (sysFlags & FILE_SUPPORTS_OBJECT_IDS) std::wcout << L"    FILE_SUPPORTS_OBJECT_IDS" << std::endl;
            if (sysFlags & FILE_SUPPORTS_OPEN_BY_FILE_ID) std::wcout << L"    FILE_SUPPORTS_OPEN_BY_FILE_ID" << std::endl;
            if (sysFlags & FILE_SUPPORTS_REPARSE_POINTS) std::wcout << L"    FILE_SUPPORTS_REPARSE_POINTS" << std::endl;
            if (sysFlags & FILE_SUPPORTS_SPARSE_FILES) std::wcout << L"    FILE_SUPPORTS_SPARSE_FILES" << std::endl;
            if (sysFlags & FILE_SUPPORTS_TRANSACTIONS) std::wcout << L"    FILE_SUPPORTS_TRANSACTIONS" << std::endl;
            if (sysFlags & FILE_SUPPORTS_USN_JOURNAL) std::wcout << L"    FILE_SUPPORTS_USN_JOURNAL" << std::endl;
            if (sysFlags & FILE_UNICODE_ON_DISK) std::wcout << L"    FILE_UNICODE_ON_DISK" << std::endl;
            if (sysFlags & FILE_VOLUME_IS_COMPRESSED) std::wcout << L"    FILE_VOLUME_IS_COMPRESSED" << std::endl;
            if (sysFlags & FILE_VOLUME_QUOTAS) std::wcout << L"    FILE_VOLUME_QUOTAS" << std::endl;
            //*/

            std::wstring disk = align(rootName.substr(0, 2) + L" " + getDriveType(rootName), TYPE_W);
            std::wstring totalSize = getByteSizeText(bytesPerSector * sectorsPerCluster * (uint64_t)totalNumberOfClusters);
            totalSize = align(totalSize, TOTAL_W);
            std::wstring freeSize = getByteSizeText(bytesPerSector * sectorsPerCluster * (uint64_t)numberOfFreeClusters);
            freeSize = align(freeSize, FREE_W);

            result.emplace_back(disk + totalSize + freeSize);
            start = i + 1;
        }
    }

    return result;
}

DiskPopup::DiskPopup(SHORT xLeft, SHORT xRight, SHORT y, SHORT w, SHORT h)
    : xLeft(xLeft)
    , xRight(xRight)
    , y(y)
    , w(w)
    , h(h)
{
    fillDrivesList();
    diskList.setSelectedIdx(0);

    updateDiskInfo();
}

void DiskPopup::fillDrivesList() {
    diskList.setLines(styledText(getDriveString(), FG::WHITE | BG::DARK_CYAN, FG::WHITE | BG::BLACK));
}

void DiskPopup::updateDiskInfo() {
    std::wstring rootName = selectedDisk() + L"\\";  // needed for FS name

    std::vector<wchar_t> volumeInfo(MAX_PATH + 1);
    std::vector<wchar_t> fileSysName(MAX_PATH + 1);
    DWORD serialNum;
    DWORD maxCompLen;
    DWORD sysFlags;

    GetVolumeInformationW(rootName.data(), volumeInfo.data(), volumeInfo.size(), &serialNum, &maxCompLen, &sysFlags, fileSysName.data(), fileSysName.size());

    std::wstring volumeStr = volumeInfo.data();
    if (volumeStr.empty()) {
        volumeStr = getDriveType(rootName) + L" " + rootName.substr(0, 2);
    }
    auto flags = getFlagsStrings(sysFlags);
    std::wstring fileSysStr = fileSysName.data();

    auto lines = std::vector<std::wstring>{
            L"Название тома: " + volumeStr,
            L"Файловая система: " + fileSysStr,
            L"Серийный номер: " + to_hex(serialNum)
    };

    lines.insert(lines.end(), flags.begin(), flags.end());

    diskInfo.setLines(styledText(lines, FG::WHITE | BG::DARK_CYAN, FG::WHITE | BG::BLACK));
}

bool DiskPopup::isLeftPopup() const {
    return isLeft;
}

std::wstring DiskPopup::selectedDisk() const {
    return diskList.getSelectedText().substr(0, 2);
}

void DiskPopup::registerKeys(Screen& screen) {
    screen.appendOwner(this);
    registerClosing(screen);
    screen.handleKey(this, VK_UP, 0, [this]() {
        diskList.selectPrev();
        updateDiskInfo();
        diskList.scrollToSelection(diskListRect().h);
    });
    screen.handleKey(this, VK_DOWN, 0, [this]() {
        diskList.selectNext();
        updateDiskInfo();
        diskList.scrollToSelection(diskListRect().h);
    });
    screen.handleKey(this, VK_PRIOR, 0, [this]() {
        diskList.moveSelection(-diskListRect().h + 1);
        updateDiskInfo();
        diskList.scrollToSelection(diskListRect().h);
    });
    screen.handleKey(this, VK_NEXT, 0, [this]() {
        diskList.moveSelection(diskListRect().h - 1);
        updateDiskInfo();
        diskList.scrollToSelection(diskListRect().h);
    });
    screen.handleKey(this, VK_HOME, 0, [this]() {
        diskList.selectFirst();
        updateDiskInfo();
        diskList.scrollToSelection(diskListRect().h);
    });
    screen.handleKey(this, VK_END, 0, [this]() {
        diskList.selectLast();
        updateDiskInfo();
        diskList.scrollToSelection(diskListRect().h);
    });
    screen.handleKey(this, VK_RETURN, 0, [this]() {
        if (selectDisk) {
            selectDisk();
            visible = false;
        }
    });
}

void DiskPopup::setOnSelectFunc(std::function<void()> func) {
    selectDisk = std::move(func);
}

void DiskPopup::drawOn(Screen& screen) {
    if (!visible) {
        return;
    }
    Rect rect = popupRect();
    Rect shadow(rect.moved(2, 1));
    screen.paintRect(shadow, FG::DARK_GREY | BG::BLACK, false);
    screen.paintRect(rect, FG::WHITE | BG::DARK_CYAN);

    screen.frame(frameRect());
    screen.textOut(innerRect().getLeftTop(), align(L"   Тип", TYPE_W) + align(L"Всего", TOTAL_W) + align(L"Свободно", FREE_W));

    diskList.drawOn(screen, diskListRect());
    screen.separator(diskInfoRect().moved(0, -1).withPadX(-1).withH(1));
    diskInfo.drawOn(screen, diskInfoRect());
}

Rect DiskPopup::popupRect() const {
    SHORT x = isLeft ? xLeft : xRight;
    return {x, y, w, h};
}

Rect DiskPopup::frameRect() const {
    return popupRect().withPadding(2, 1);
}

Rect DiskPopup::innerRect() const {
    return frameRect().withPadding(1, 1);
}

Rect DiskPopup::diskListRect() const {
    Rect inner = innerRect();
    return inner.moved(0, 1).withH(inner.h - diskInfo.getLinesCount() - 2);
}

Rect DiskPopup::diskInfoRect() const {
    Rect listRect = diskListRect();
    return listRect.moved(0, listRect.h + 1).withH(diskInfo.getLinesCount());
}

void DiskPopup::show(bool left) {
    fillDrivesList();
    updateDiskInfo();
    isLeft = left;
    visible = true;
}
