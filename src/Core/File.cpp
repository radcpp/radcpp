#include "rad/Core/File.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifndef PATH_MAX_LEN
#define PATH_MAX_LEN 1024
#endif

File::File()
{
}

File::~File()
{
    if (m_handle)
    {
        Close();
    }
}

bool File::Open(const Path& filePath, FileOpenFlags flags)
{
    const char* mode = "r";
    switch (flags)
    {
    case FileOpenRead:
        mode = "r";
        break;
    case FileOpenWrite:
        mode = "w";
        break;
    case FileOpenAppend:
        mode = "a";
        break;
    case (FileOpenRead | FileOpenAppend):
        mode = "a+";
        break;
    case (FileOpenRead | FileOpenWrite):
        mode = "w+";
        break;
    case (FileOpenRead | FileOpenBinary):
        mode = "rb";
        break;
    case (FileOpenWrite | FileOpenBinary):
        mode = "wb";
        break;
    case (FileOpenAppend | FileOpenBinary):
        mode = "ab";
        break;
    case (FileOpenRead | FileOpenAppend | FileOpenBinary):
        mode = "ab+";
        break;
    case (FileOpenRead | FileOpenWrite | FileOpenBinary):
        mode = "wb+";
        break;
    }

    errno_t error = fopen_s(&m_handle, (const char*)filePath.u8string().c_str(), mode);
    if ((error == 0) && (m_handle != nullptr))
    {
        m_path = filePath;
        return true;
    }
    else
    {
        return false;
    }
}

void File::Close()
{
    if (m_handle != nullptr)
    {
        fclose(m_handle);
    }
}

bool File::IsOpen()
{
    return (m_handle != nullptr);
}

void File::Flush()
{
    fflush(m_handle);
}

size_t File::Read(void* buffer, size_t sizeInBytes, size_t elementCount)
{
    return fread(buffer, sizeInBytes, elementCount, m_handle);
}

size_t File::ReadLine(void* buffer, size_t bufferSize)
{
    size_t bytesRead = 0;
    char* charBuffer = static_cast<char*>(buffer);
    while (bytesRead < bufferSize)
    {
        int32_t c = getc(m_handle);
        if ((c == '\n') || (c == EOF))
        {
            break;
        }
        charBuffer[bytesRead] = static_cast<char>(c);
        bytesRead++;
    }

    const size_t endIndex = ((bytesRead < bufferSize) ? bytesRead : (bufferSize - 1));
    charBuffer[endIndex] = '\0';

    return bytesRead;
}

size_t File::Write(const void* buffer, size_t sizeInBytes, size_t count)
{
    return fwrite(buffer, sizeInBytes, count, m_handle);
}

int32_t File::GetChar()
{
    return std::getc(m_handle);
}

int64_t File::Seek(int64_t offset, int origin)
{
    return fseek(m_handle, static_cast<long>(offset), origin);
}

int64_t File::Rseek(int64_t offset)
{
    return fseek(m_handle, static_cast<long>(offset), SEEK_END);
}

void File::Rewind()
{
    std::rewind(m_handle);
}

void File::FastForward()
{
    Rseek(0);
}

int64_t File::Size()
{
    return static_cast<int64_t>(std::filesystem::file_size(m_path));
}

int64_t File::Tell()
{
    return ftell(m_handle);
}

bool File::Exists(const Path& path)
{
    return std::filesystem::exists(path);
}

std::string File::ReadAll(const Path& path, FileOpenFlags flags)
{
    File file;
    if (file.Open(path, flags))
    {
        int64_t fileSize = file.Size();
        std::string data(fileSize, 0);
        file.Read(data.data(), fileSize);
        return data;
    }
    else
    {
        return std::string();
    }
}

std::vector<std::string> File::ReadLines(const Path& path)
{
    File file;
    if (file.Open(path, FileOpenRead))
    {
        std::vector<std::string> lines;
        std::string* line = &lines.emplace_back();
        while (true)
        {
            int32_t c = file.GetChar();
            if (c == EOF)
            {
                break;
            }
            if ((c == '\n'))
            {
                line = &lines.emplace_back();
                continue;
            }
            line->push_back(static_cast<char>(c));
        };
        return lines;
    }
    return {};
}

namespace io
{
    Path GetAbsolutePath(const Path& path)
    {
        return std::filesystem::absolute(path);
    }

    Path GetCanonicalPath(const Path& path)
    {
        return std::filesystem::canonical(path);
    }

    Path GetRelativePath(const Path& path, const Path& base)
    {
        return std::filesystem::relative(path, base);
    }

    void Copy(const Path& from, const Path& to, CopyOptions options)
    {
        return std::filesystem::copy(from, to, options);
    }

    bool CreateDirectories(const Path& path)
    {
        return std::filesystem::create_directories(path);
    }

    Path GetCurrentPath()
    {
        return std::filesystem::current_path();
    }

    void SetCurrentPath(const Path& path)
    {
        std::filesystem::current_path(path);
    }

    Path GetTempDirectory()
    {
        return std::filesystem::temp_directory_path();
    }

    bool Exists(const Path& path)
    {
        return std::filesystem::exists(path);
    }

    void Rename(const Path& oldPath, const Path& newPath)
    {
        std::filesystem::rename(oldPath, newPath);
    }

    void ResizeFile(const Path& path, uint64_t newSize)
    {
        std::filesystem::resize_file(path, newSize);
    }

    FileTime GetLastWriteTime(const Path& path)
    {
        return std::filesystem::last_write_time(path);
    }

    std::vector<Path> GetRootDirectories()
    {
        std::vector<Path> rootDirectories;
#ifdef _WIN32
        DWORD logicalDrivesBitmask = GetLogicalDrives();
        std::string rootDirectory = "A:\\";
        for (DWORD i = 0; i < sizeof(DWORD) * 8; i++)
        {
            if (logicalDrivesBitmask & (1 << i))
            {
                rootDirectory[0] = 'A' + char(i);
                rootDirectories.push_back(rootDirectory);
            }
        }
#else
        rootDirectories.push_back(Path("/"));
#endif
        return rootDirectories;
    }

    bool IsBlockFile(const Path& path)
    {
        return std::filesystem::is_block_file(path);
    }

    bool IsCharacterFile(const Path& path)
    {
        return std::filesystem::is_character_file(path);
    }

    bool IsDirectory(const Path& path)
    {
        return std::filesystem::is_directory(path);
    }

    bool IsEmpty(const Path& path)
    {
        return std::filesystem::is_empty(path);
    }

    bool IsFIFO(const Path& path)
    {
        return std::filesystem::is_fifo(path);
    }

    bool IsOther(const Path& path)
    {
        return std::filesystem::is_other(path);
    }

    bool IsRegularFile(const Path& path)
    {
        return std::filesystem::is_regular_file(path);
    }

    bool IsSocket(const Path& path)
    {
        return std::filesystem::is_socket(path);
    }

    bool IsSymbolLink(const Path& path)
    {
        return std::filesystem::is_symlink(path);
    }

    bool IsStatusKnown(FileStatus s)
    {
        return std::filesystem::status_known(s);
    }

    FileStatus GetStatus(const Path& path)
    {
        return std::filesystem::status(path);
    }

    uint32_t GetPathElementCount(const Path& path)
    {
        uint32_t elementCount = 0;
        for (auto iter = path.begin(); iter != path.end(); iter++)
        {
            elementCount++;
        }
        return elementCount;
    }

    std::string GetPathTimeString(const Path& path)
    {
        FileTime fileTime = io::GetLastWriteTime(path);
        time_t time = std::chrono::duration_cast<std::chrono::seconds>(fileTime.time_since_epoch()).count();
        std::tm dateTime = {};

#ifdef _WIN32
        struct __stat64 fileStatus = {};
        _wstat64(path.c_str(), &fileStatus);
#else
        struct stat fileStatus = {};
        stat(path.c_str(), &fileStatus);
#endif
        localtime_s(&dateTime, &fileStatus.st_mtime);

        std::string buffer(32, 0);
        strftime(buffer.data(), buffer.size(), "%F %T", &dateTime);
        return buffer;
    }

    std::string GetProcessName()
    {
#ifdef _WIN32
        std::wstring buffer;
        buffer.resize(PATH_MAX_LEN);
        do
        {
            unsigned int len = GetModuleFileNameW(NULL, &buffer[0], static_cast<DWORD>(buffer.size()));
            if (len < buffer.size())
            {
                buffer.resize(len);
                break;
            }

            buffer.resize(buffer.size() * 2);
        } while (buffer.size() < 65536);

        return (const char*)Path(buffer).filename().u8string().c_str();

#else
        std::error_code ec;
        if (std::filesystem::exists("/proc/self/exe", ec))
        {
            return std::filesystem::read_symlink("/proc/self/exe", ec).filename().string();
        }
        if (std::filesystem::exists("/proc/curproc/file", ec))
        {
            return std::filesystem::read_symlink("/proc/curproc/file", ec).filename().string();
        }
        if (std::filesystem::exists("/proc/curproc/exe", ec))
        {
            return std::filesystem::read_symlink("/proc/curproc/exe", ec).filename().string();
        }
        return std::to_string(getpid());
#endif
    }

} // namespace io