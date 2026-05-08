#pragma once

#include "GlobalNamespace/NoteCutInfo.hpp"
#include "UnityEngine/Color.hpp"

namespace SliceVisualizer {
    bool Initialize();
    bool CreateSprites();
    void SetSaberColors(UnityEngine::Color leftColor, UnityEngine::Color rightColor);
    void CreateSliceVisual(GlobalNamespace::NoteCutInfo& cutInfo);
    void UpdateSlices();
    void Cleanup();
}
