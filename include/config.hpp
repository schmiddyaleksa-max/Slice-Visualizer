#pragma once

#include "HMUI/ViewController.hpp"
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(Enabled, bool, "Mod Enabled", true)
    CONFIG_VALUE(Dynamic, bool, "Dynamic Fade Speed", true, "Fade faster during fast sections")
    CONFIG_VALUE(FadeSpeed, float, "Fade Speed", 1.0f, "Fade speed multiplier")
    CONFIG_VALUE(SliceScale, float, "Slice Scale", 0.6f, "Size of slice visuals")
    CONFIG_VALUE(PositionZ, float, "Position Z", 15.0f, "Visual depth positioning")
)

void SetupModSettings(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
