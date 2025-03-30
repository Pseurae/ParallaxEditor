#pragma once

#include <string>
#include <list>
#include <memory>

class Popup
{
public:
    Popup(const std::string &internalName) : mInternalName(internalName) 
    {}

    virtual ~Popup() = default;

    void Draw();
    virtual void PreDrawContent() {}
    virtual void DrawContent() = 0;

    const auto &GetInternalName() const { return mInternalName; }
    bool ShouldClose() { return mShouldClose; }

    void Close() { mShouldClose = true; }

private:
    std::string mInternalName;
    bool mShouldClose = false;
};

class PopupManager final
{
public:
    template<typename T, typename... Args>
    requires (std::derived_from<T, Popup>)
    void Open(Args&& ...args)
    {
        mPopups.push_back(std::move(std::make_unique<T>(std::forward<Args>(args)...)));
    }

    void DrawAndUpdate(void);
private:
    std::list<std::unique_ptr<Popup>> mPopups;
};