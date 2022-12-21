#include "rad/Gui/FileDialog.h"

#include "nfd.h"

FileDialog::FileDialog()
{
}

FileDialog::~FileDialog()
{
}

bool FileDialog::Init()
{
    if (NFD_Init() == NFD_OKAY)
    {
        return true;
    }
    return false;
}

void FileDialog::Quit()
{
    NFD_Quit();
}

Path FileDialog::OpenFileDialog(ArrayRef<FilterItem> filters, const Path* defaultPath)
{
    Path selected;
    nfdu8char_t* outPath = nullptr;
    std::vector<nfdfilteritem_t> filterItems(filters.size());
    for (uint32_t i = 0; i < filters.size(); ++i)
    {
        filterItems[i].name = filters[i].name.c_str();
        filterItems[i].spec = filters[i].spec.c_str();
    }
    nfdresult_t result = NFD_OpenDialogU8(&outPath,
        filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()),
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        selected = (char8_t*)outPath;
        NFD_FreePathU8(outPath);
    }
    return selected;
}

std::vector<Path> FileDialog::OpenFileDialogMultiSelect(ArrayRef<FilterItem> filters, const Path* defaultPath)
{
    std::vector<Path> selected;
    const nfdpathset_t* outPaths = nullptr;
    std::vector<nfdfilteritem_t> filterItems(filters.size());
    for (uint32_t i = 0; i < filters.size(); ++i)
    {
        filterItems[i].name = filters[i].name.c_str();
        filterItems[i].spec = filters[i].spec.c_str();
    }
    nfdresult_t result = NFD_OpenDialogMultipleU8(&outPaths,
        filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()),
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        nfdpathsetsize_t pathCount = 0;
        if (NFD_PathSet_GetCount(outPaths, &pathCount) == NFD_OKAY)
        {
            selected.reserve(pathCount);
            for (nfdpathsetsize_t i = 0; i < pathCount; ++i)
            {
                nfdu8char_t* outPath = nullptr;
                if (NFD_PathSet_GetPathU8(outPaths, i, &outPath) == NFD_OKAY)
                {
                    selected.push_back((char8_t*)outPath);
                    NFD_FreePathU8(outPath);
                }
            }
        }
        NFD_PathSet_Free(outPaths);
    }
    return selected;
}

Path FileDialog::SaveFileDialog(ArrayRef<FilterItem> filters, const Path* defaultPath, const Path* defaultName)
{
    Path selected;
    nfdu8char_t* outPath = nullptr;
    std::vector<nfdfilteritem_t> filterItems(filters.size());
    for (uint32_t i = 0; i < filters.size(); ++i)
    {
        filterItems[i].name = filters[i].name.c_str();
        filterItems[i].spec = filters[i].spec.c_str();
    }
    nfdresult_t result = NFD_SaveDialogU8(&outPath,
        filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()),
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr,
        defaultName ? (nfdu8char_t*)defaultName->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        selected = (char8_t*)outPath;
        NFD_FreePathU8(outPath);
    }
    return selected;
}

Path FileDialog::PickFolder(const Path* defaultPath)
{
    Path selected;
    nfdu8char_t* outPath = nullptr;
    nfdresult_t result = NFD_PickFolderU8(&outPath,
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        selected = (char8_t*)outPath;
        NFD_FreePathU8(outPath);
    }
    return selected;
}

const char* FileDialog::GetError() const
{
    return NFD_GetError();
}
