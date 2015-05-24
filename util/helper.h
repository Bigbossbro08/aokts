#ifndef INC_HELPER_H
#define INC_HELPER_H

#include <string>

bool replaced( std::string &s, const std::string &search, const std::string &replace );
void replaceAll( std::string &s, const std::string &search, const std::string &replace );
std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);
std::string time_string(unsigned int seconds, bool shortened = false);
std::string wstringToString(std::wstring ws);
std::string wstringToString(const wchar_t * ws);

// inline functions must reside in the header (the compiler needs access
// to it)
inline int from_ecplayer(int player) {
    if (player == 0)
        return 8;
    else if (player > 0)
        return player - 1;
    else
        return -1;
}

inline int to_ecplayer(int player) {
    if (player == 8)
        return 0;
    else if (player >= 0)
        return player + 1;
    else
        return -1;
}

std::string playerPronoun(int p);

// Standard in C++11
// Gives pointers to the first and last elements of an array
template <typename T, size_t N>
T* begin(T(&arr)[N]) { return &arr[0]; }
template <typename T, size_t N>
T* end(T(&arr)[N]) { return &arr[0]+N; }

#endif // INC_HELPER_H
