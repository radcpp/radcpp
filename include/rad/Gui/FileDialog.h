#pragma once

#include "rad/Core/Common.h"
#include "rad/Core/File.h"
#include "rad/Core/Span.h"

class FileDialog
{
public:
    struct FilterItem
    {
        std::string name; // e.g. C/C++ Source
        std::string spec; // e.g. c,cpp
    };

    FileDialog();
    ~FileDialog();

    Path OpenFileDialog(ArrayRef<FilterItem> filters, const Path* defaultPath = nullptr);
    std::vector<Path> OpenFileDialogMultiSelect(ArrayRef<FilterItem> filters, const Path* defaultPath = nullptr);
    Path SaveFileDialog(ArrayRef<FilterItem> filters, const Path* defaultPath = nullptr, const Path* defaultName = nullptr);
    Path PickFolder(const Path* defaultPath = nullptr);

private:
    bool Init();
    void Quit();
    const char* GetError() const;

}; // class FileDialog
