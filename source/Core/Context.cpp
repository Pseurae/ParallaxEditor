#include "Core/Context.h"
#include <filesystem>
#include "Context.h"

Context::Context(unsigned short width, unsigned short height): mTilemap(width, height)
{}

void Context::TrySave(void)
{}

void Context::SaveAs(const std::string &path)
{}

void Context::Import(const std::string &path)
{}

void Context::Export(const std::string &path)
{}

const std::string Context::GetName() const
{
    return mPath.empty() ? "Untitled" : std::filesystem::path(mPath).filename().string();
}
void Context::Load(const std::string &fname)
{
}