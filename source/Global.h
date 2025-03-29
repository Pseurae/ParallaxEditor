#pragma once

#include <string>
#include <memory>
#include "Utils/Tilemap.h"
#include "Core/Context.h"
#include "Core/Popup.h"
#include "Core/Renderer.h"

using ContextList = std::list<std::unique_ptr<Context>>;

struct Global
{
    double dpiScale, zoomScale = 1.0f;
    bool drawScreenBounds = false;

    ContextList contexts;
    Context *currentContext = nullptr;

    PopupManager popupManager;
    Renderer renderer;
};

extern Global global;