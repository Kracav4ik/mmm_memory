#pragma once

#include <windows.h>

#include <string>
#include <iomanip>
#include <cmath>

bool isDir(const std::wstring& path);
std::wstring getLastErrorText();
std::wstring getLastErrorText(DWORD lastError);

std::wstring getTimeStr(const SYSTEMTIME& tl);
std::wstring getTimeStr(const FILETIME& time);
bool sameTime(const FILETIME& t1, const FILETIME& t2);

std::wstring align(const std::wstring& s, int size);
std::wstring trim(const std::wstring& s);

template<typename T>
std::wstring to_hex(T num) {
    std::wstringstream s;
    s << std::setfill(L'0') << std::setw(2*sizeof(T)) << std::hex;
    s << num;
    return s.str();
}

template <typename T>
T clamp(const T& left, const T& value, const T& right) {
    return std::max(left, std::min(value, right));
}

template <typename T>
int roundI(const T& val) {
    return (int)std::round(val);
}
