#include "rad/Core/String.h"
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <boost/locale/encoding_utf.hpp>

std::vector<std::string> StrSplit(std::string_view str, std::string_view delimiters, bool skipEmptySubStr)
{
    std::vector<std::string> substrs;

    std::string::size_type pos = 0;
    std::string::size_type offset = 0;

    while (offset < str.length() + 1)
    {
        pos = str.find_first_of(delimiters, offset);
        if (pos == std::string::npos)
        {
            pos = str.length();
        }

        if (pos != offset || !skipEmptySubStr)
        {
            substrs.push_back(std::string(str.data() + offset, pos - offset));
        }

        offset = pos + 1;
    }

    return substrs;
}

std::string StrPrint(const char* format, ...)
{
    std::string str;

    va_list args;

    va_start(args, format);
    StrPrintInPlaceArgList(str, format, args);
    va_end(args);

    return str;
}

int StrPrintInPlace(std::string& buffer, const char* format, ...)
{
    va_list args;

    int charsPrinted = 0;
    va_start(args, format);
    charsPrinted = vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);

    if ((charsPrinted > 0) && (size_t(charsPrinted) + 1 > buffer.size()))
    {
        buffer.resize(size_t(charsPrinted) + 1);
        va_start(args, format);
        charsPrinted = vsnprintf(buffer.data(), buffer.size(), format, args);
        va_end(args);
    }
    buffer.pop_back();

    return charsPrinted;
}

int StrPrintInPlaceArgList(std::string& buffer, const char* format, va_list args)
{
    int charsPrinted = 0;
    va_list args1;
    va_copy(args1, args);
    charsPrinted = vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args1);

    if ((charsPrinted > 0) && (size_t(charsPrinted) + 1 > buffer.size()))
    {
        buffer.resize(size_t(charsPrinted) + 1);
        va_copy(args1, args);
        charsPrinted = vsnprintf(buffer.data(), buffer.size(), format, args);
        va_end(args1);
    }
    buffer.pop_back();

    return charsPrinted;
}

bool StrEqual(std::string_view str1, std::string_view str2)
{
    return (strcmp(str1.data(), str2.data()) == 0);
}

bool StrCaseEqual(std::string_view str1, std::string_view str2)
{
    return (_stricmp(str1.data(), str2.data()) == 0);
}

std::string StrWideToU8(std::wstring_view wstr)
{
#ifdef _WIN32
    std::string str;
    int charCount = ::WideCharToMultiByte(CP_UTF8,
        0, wstr.data(), static_cast<int>(wstr.length()), NULL, 0, NULL, NULL);
    if (charCount > 0)
    {
        str.resize(charCount, 0);
        ::WideCharToMultiByte(CP_UTF8,
            0, wstr.data(), static_cast<int>(wstr.length()), &str[0], charCount, NULL, NULL);
    }
    return str;
#else
    return boost::locale::conv::utf_to_utf<char>(wstr.data());
#endif
}

std::wstring StrU8ToWide(std::string_view str)
{
#ifdef _WIN32
    std::wstring wStr;
    int charCount = ::MultiByteToWideChar(CP_UTF8,
        0, str.data(), static_cast<int>(str.length()), NULL, 0);
    if (charCount > 0)
    {
        wStr.resize(charCount, 0);
        ::MultiByteToWideChar(CP_UTF8,
            0, str.data(), static_cast<int>(str.length()), &wStr[0], charCount);
    }
    return wStr;
#else
    return boost::locale::conv::utf_to_utf<wchar_t>(str.data());
#endif
}

std::string StrUpper(std::string_view s)
{
    std::string res(s);
    StrUpperInplace(res);
    return res;
}

std::string StrLower(std::string_view s)
{
    std::string res(s);
    StrLowerInplace(res);
    return res;
}

void StrUpperInplace(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::toupper(c); });
}

void StrLowerInplace(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
}

void StrReplaceAll(std::string& str, std::string_view subOld, std::string_view subNew)
{
    std::string::size_type pos = 0;
    while ((pos = str.find(subOld, pos)) != std::string::npos)
    {
        str.replace(pos, subOld.length(), subNew);
        pos += subNew.length();
    }
}

bool StrIsDecInteger(std::string_view str)
{
    if (str.empty())
    {
        return false;
    }
    auto beg = str.begin();
    if (str.size() >= 2)
    {
        if (str[0] == '+' || str[0] == '-')
        {
            beg++;
        }
    }
    return std::all_of(beg, str.end(), IsDigit);
}

bool StrIsHexNumber(std::string_view str)
{
    if (str.size() > 2 &&
        (str.starts_with("0x") || str.starts_with("0X")))
    {
        return std::all_of(str.begin() + 2, str.end(), IsHexDigit);
    }
    return false;
}

bool StrIsBinNumber(std::string_view str)
{
    if (str.size() > 2 &&
        (str.starts_with("0b") || str.starts_with("0B")))
    {
        return std::all_of(str.begin() + 2, str.end(),
            [](char c) { return (c == '0' || c == '1'); });
    }
    return false;
}

bool StrIsNumeric(std::string_view str)
{
    const char* p = str.data();
    if ((*p == '-') || (*p == '+'))
    {
        ++p;
    }

    bool hasDot = false;
    while (*p != '\0')
    {
        if (*p == '.')
        {
            if (hasDot)
            {
                return false;
            }
            hasDot = true;
        }
        else if (!IsDigit(*p))
        {
            return false;
        }

        ++p;
    }

    return true;
}

bool IsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

bool IsHexDigit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

void StrTrimLeading(std::string& str, std::string_view charlist)
{
    str.erase(0, str.find_first_not_of(charlist));
}

void StrTrimTrailing(std::string& str, std::string_view charlist)
{
    str.erase(str.find_last_not_of(charlist) + 1);
}

void StrTrim(std::string& str)
{
    StrTrimLeading(str);
    StrTrimTrailing(str);
}

std::string StrFindReplace(std::string_view str, std::string_view subOld, std::string_view subNew)
{
    std::string newStr;
    newStr.reserve(str.size());
    std::string::size_type offset = 0u;
    std::string::size_type pos = 0u;
    while ((pos = str.find(subOld, offset)) != std::string::npos)
    {
        newStr.append(str, offset, pos - offset);
        newStr.append(subNew);
        offset = pos + subOld.size();
    }
    if (offset < str.size() - 1)
    {
        pos = str.size();
        newStr.append(str, offset, pos - offset);
    }
    return newStr;
}

void StrFindReplaceInPlace(std::string& str, std::string_view subOld, std::string_view subNew)
{
    std::string::size_type pos = 0u;
    while ((pos = str.find(subOld, pos)) != std::string::npos)
    {
        str.replace(pos, subOld.length(), subNew);
        pos += subNew.length();
    }
}
