#pragma once

#include <nfd.hpp>
#include <vector>
#include <string>

class FileDialog
{
public:
    enum class Mode : char
    {
        Open,
        Save,
        Folder
    };

    using Filter = std::vector<nfdfilteritem_t>;
    
    static void Init(void *handle);
    static void Quit();
    static bool Open(Mode mode, const Filter &filters, std::string &path, const std::string &defaultPath = "");
private:
    static nfdwindowhandle_t sWinHandle;
};