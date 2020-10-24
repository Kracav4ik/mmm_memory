#include "AttrChangePopup.h"

#include "Screen.h"
#include "colors.h"
#include "MessagePopup.h"
#include "utils.h"

#include <memory>

static const DWORD ATTRS[] = {
        FILE_ATTRIBUTE_READONLY,
        FILE_ATTRIBUTE_ARCHIVE,
        FILE_ATTRIBUTE_HIDDEN,
        FILE_ATTRIBUTE_SYSTEM,
};
static const std::wstring NAMES[] = {
        L"Только для чтения",
        L"Архивный",
        L"Скрытый",
        L"Системный",
};
static const std::wstring OFFSET(18, L' ');
static const std::wstring TIME_MASK = L"dd.MM.yyyy HH:mm:ss.fff";

AttrChangePopup::AttrChangePopup(Screen& screen, SHORT w, SHORT h)
    : w(w), h(h), timeEdit(screen, this, TIME_MASK.size() + 1, TIME_MASK.size())
{
    labelsOver.setLines(styledText({
            L"Дата создания:",
            L"Дата доступа:",
            L"Дата изменения:",
    }, FG::BLACK | BG::GREY));
}

void AttrChangePopup::show(const std::wstring& fileRoot, const std::wstring& fileName) {
    visible = true;
    root = fileRoot;
    name = fileName;
    std::wstring path = root + L"\\" + name;

    DWORD attr = GetFileAttributesW(path.c_str());
    badAttr = attr == INVALID_FILE_ATTRIBUTES;
    if (badAttr) {
        oldAttr = 0;
    } else {
        oldAttr = attr;
    }
    newAttr = oldAttr;

    badTime = true;
    HANDLE file = CreateFileW(path.data(), FILE_READ_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        badTime = GetFileTime(file, &oldTime[CREATE], &oldTime[ACCESS], &oldTime[MODIFY]) == 0;
        memcpy(&newTime, &oldTime, sizeof(oldTime));
        CloseHandle(file);
    }

    updateAttr();
    fillTimeEdit();
}

void AttrChangePopup::registerKeys(Screen& screen) {
    screen.appendOwner(this);
    registerClosing(screen);
    screen.handleKey(this, VK_RETURN, 0, [this]() {
        updateTime();
        std::wstring path = root + L"\\" + name;
        if (anyAttrChanged()) {
            if (SetFileAttributesW(path.c_str(), newAttr) == 0) {
                MessagePopup::show({L"Ошибка выставления атрибутов:", getLastErrorText()});
                return;
            }
        }
        if (anyTimeChanged()) {
            HANDLE file = CreateFileW(path.data(), FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
            if (file != INVALID_HANDLE_VALUE) {
                if (SetFileTime(file, &newTime[CREATE], &newTime[ACCESS], &newTime[MODIFY]) == 0) {
                    MessagePopup::show({L"Ошибка выставления времени:", getLastErrorText()});
                    return;
                }
                CloseHandle(file);
            } else {
                MessagePopup::show({L"Ошибка открытия файла для выставления времени:", getLastErrorText()});
                return;
            }
        }

        visible = false;
    });
    screen.handleKey(this, VK_SPACE, 0, [this]() {
        int selected = attrText.getSelectedIdx();
        if (!badAttr && selected < 4) {
            newAttr ^= ATTRS[selected];
            updateAttr();
        }
    });
    screen.handleKey(this, VK_UP, 0, [this]() {
        updateTime();
        attrText.selectPrev();
        fillTimeEdit();
    });
    screen.handleKey(this, VK_DOWN, 0, [this]() {
        updateTime();
        attrText.selectNext();
        fillTimeEdit();
    });
}

void AttrChangePopup::drawOn(Screen& screen) {
    if (!visible) {
        return;
    }
    auto center = screen.center();
    Rect rect{(SHORT)(center.X - w/2), (SHORT)(center.Y - h/2), w, h};
    Rect shadow(rect.moved(2, 1));
    screen.paintRect(shadow, FG::DARK_GREY | BG::BLACK, false);
    screen.paintRect(rect, FG::BLACK | BG::GREY);

    Rect frameRect = rect.withPadding(2, 1);
    screen.frame(frameRect);

    Rect inner = frameRect.withPadding(2, 2);
    std::wstring type = (oldAttr & FILE_ATTRIBUTE_DIRECTORY) ? L"папки" : L"файла";
    screen.boundedLine(inner.getLeftTop(), inner.w, L"Атрибуты " + type + L" " + name);
    attrText.drawOn(screen, inner.moved(0, 1));

    Rect over = inner.moved(0, 5).withW(OFFSET.size());
    labelsOver.drawOn(screen, over, false);

    if (timeSelected()) {
        int timeIdx = attrText.getSelectedIdx() - 4;
        Rect line = over.moved(OFFSET.size(), timeIdx).withH(1).withW(inner.w - OFFSET.size());
        COORD pos = line.getLeftTop();
        screen.textOut(pos, TIME_MASK);
        std::wstring timeText = timeEdit.getText();
        int parsedSize = parseTime(timeText);
        timeEdit.drawOn(screen, pos, FG::BLACK | BG::DARK_CYAN, false);
        int textSize = std::min(line.w - 1, (int)timeText.size());
        if (textSize > parsedSize) {
            screen.paintRect(line.withW(textSize - parsedSize).moved(parsedSize, 0), FG::RED | BG::DARK_CYAN, false);
        }
        screen.paintRect(line.withW(line.w - textSize).moved(textSize, 0), FG::GREY | BG::DARK_CYAN, false);
    }

    if (!badTime) {
        for (int i = 0; i < 3; ++i) {
            if (sameTime(oldTime[i], newTime[i])) {
                continue;
            }
            screen.textOut(over.moved(OFFSET.size() - 2, i).getLeftTop(), L"*");
        }
    }

    Rect sep = frameRect.moved(0, frameRect.h - 3).withH(1);
    screen.separator(sep);

    screen.labels(sep.moved(0, 1), {L" <Enter> OK ", L" <Esc> Отмена "}, FG::BLACK | BG::DARK_CYAN);
}

bool AttrChangePopup::anyAttrChanged() const {
    return newAttr != oldAttr;
}

bool AttrChangePopup::anyTimeChanged() const {
    for (int i = 0; i < 3; ++i) {
        if (!sameTime(oldTime[i], newTime[i])) {
            return true;
        }
    }
    return false;
}

bool AttrChangePopup::timeSelected() const {
    return attrText.hasSelection() && attrText.getSelectedIdx() >= 4;
}

void AttrChangePopup::updateAttr() {
    std::vector<std::wstring> lines;
    for (int i = 0; i < 4; ++i) {
        std::wstring line;
        if (badAttr) {
            line = L"  - ошибка при получении атрибутов -";
        } else {
            if ((newAttr & ATTRS[i]) != (oldAttr & ATTRS[i])) {
                line += L" *";
            } else {
                line += L"  ";
            }
            if (newAttr & ATTRS[i]) {
                line += L"[x] ";
            } else {
                line += L"[ ] ";
            }
            line += NAMES[i];
        }
        lines.push_back(std::move(line));
    }
    for (const auto& time : newTime) {
        if (badTime) {
            lines.push_back(OFFSET + L"- не удалось получить -");
        } else {
            lines.push_back(OFFSET + getTimeStr(time));
        }
    }
    attrText.setLines(styledText(std::move(lines), FG::BLACK | BG::GREY, FG::BLACK | BG::DARK_CYAN));
    if (!attrText.hasSelection()) {
        attrText.setSelectedIdx(0);
    }
    if (badAttr || badTime) {
        attrText.unselect();
    }
}

void AttrChangePopup::updateTime() {
    if (timeSelected()) {
        int timeIdx = attrText.getSelectedIdx() - 4;
        parseTime(timeEdit.getText(), &newTime[timeIdx]);
        updateAttr();
    }
}

void AttrChangePopup::fillTimeEdit() {
    if (timeSelected()) {
        int timeIdx = attrText.getSelectedIdx() - 4;
        timeEdit.setText(getTimeStr(newTime[timeIdx]), [this]() {
            return timeSelected();
        });
    }
}

int AttrChangePopup::parseTime(const std::wstring& s, FILETIME* time) {
    WORD day = 0, month = 0, year = 0, hour = 0, minute = 0, second = 0, millisecond = 0;
    int readSize = 0;
    std::swscanf(
            s.c_str(),
            L"%2hd%n.%n%2hd%n.%n%4hd%n%*1[ ]%2hd%n:%n%2hd%n:%n%2hd%n.%n%3hd%n",
            &day, &readSize, &readSize,
            &month, &readSize, &readSize,
            &year, &readSize,
            &hour, &readSize, &readSize,
            &minute, &readSize, &readSize,
            &second, &readSize, &readSize,
            &millisecond, &readSize
    );
    if (time) {
        SYSTEMTIME tl{year, month, 0, day, hour, minute, second, millisecond};
        SYSTEMTIME ts;
        if (TzSpecificLocalTimeToSystemTime(nullptr, &tl, &ts) == 0) {
            MessagePopup::show({L"Некорректная дата " + getTimeStr(tl)});
        } else {
            SystemTimeToFileTime(&ts, time);
        }
    }

    return readSize;
}
