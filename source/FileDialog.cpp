#include "FileDialog.h"
#include <nfd.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <nfd_glfw3.h>

nfdwindowhandle_t FileDialog::sWinHandle;

void FileDialog::Init(void *handle)
{
    NFD::Init();
    NFD_GetNativeWindowFromGLFWWindow((GLFWwindow *)handle, &sWinHandle);
}

void FileDialog::Quit()
{
    NFD::Quit();
}

bool FileDialog::Open(Mode mode, const Filter &filters, std::string &path, const std::string &defaultPath)
{
    nfdresult_t result;
    NFD::UniquePath p;

    switch (mode) 
    {
    case Mode::Open:
        result = NFD::OpenDialog(p, filters.data(), filters.size(), defaultPath.c_str(), sWinHandle);
        break;
    case Mode::Save:
        result = NFD::SaveDialog(p, filters.data(), filters.size(), defaultPath.c_str(), NULL, sWinHandle);
        break;
    case Mode::Folder:
        result = NFD::PickFolder(p, defaultPath.c_str(), sWinHandle);
        break;
    default:
        return false;
    }

    if (result == NFD_OKAY)
        path = p.get();

    return result == NFD_OKAY;
}
