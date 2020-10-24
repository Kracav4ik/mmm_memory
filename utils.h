#pragma once

#include <windows.h>

#include <string>

bool isDir(const std::wstring& path);
std::wstring getLastErrorText();
std::wstring getLastErrorText(DWORD lastError);

std::wstring getTimeStr(const SYSTEMTIME& tl);
std::wstring getTimeStr(const FILETIME& time);
bool sameTime(const FILETIME& t1, const FILETIME& t2);

std::wstring align(const std::wstring& s, int size);
std::wstring trim(const std::wstring& s);
std::wstring to_hex(DWORD num);

template <typename T>
T clamp(const T& left, const T& value, const T& right) {
    return std::max(left, std::min(value, right));
}
