#pragma once

#include "HMUI/ViewController.hpp"
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(Enabled, bool, "Mod Enabled", true, "Enable or disable Slice Visualizer")
    CONFIG_VALUE(Dynamic, bool, "Dynamic Fade Speed", true, "Fade faster during fast sections of songs")
    CONFIG_VALUE(FadeSpeed, float, "Fade Speed", 1.0f, "How quickly slice visuals fade (0.1 - 3.0)")
    CONFIG_VALUE(SliceScale, float, "Slice Scale", 0.6f, "Size multiplier for slice visuals (0.3 - 1.0)")
    CONFIG_VALUE(PositionX, float, "Position X", 0.0f, "Horizontal offset of visuals (-5.0 - 5.0)")
    CONFIG_VALUE(PositionY, float, "Position Y", 3.0f, "Vertical offset of visuals (0.0 - 6.0)")
    CONFIG_VALUE(PositionZ, float, "Position Z", 15.0f, "Depth positioning of visuals (5.0 - 25.0)")
    CONFIG_VALUE(LineThickness, float, "Line Thickness", 1.0f, "Cut direction line thickness (0.5 - 2.0)")
    CONFIG_VALUE(Opacity, float, "Initial Opacity", 1.0f, "Starting opacity of slice visuals (0.3 - 1.0)")
)

void SetupModSettings(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
