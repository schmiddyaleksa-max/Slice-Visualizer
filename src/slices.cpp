#include "main.hpp"
#include "slices.hpp"
#include "sprites.hpp"
#include "config.hpp"
#include "GlobalNamespace/ComboUIController.hpp"
#include "GlobalNamespace/NoteCutDirectionExtensions.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "UnityEngine/UI/Mask.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Resources.hpp"
#include "questui/shared/BeatSaberUI.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace QuestUI;

namespace SliceVisualizer {

    struct SliceVisual {
        GameObject* parent;
        UI::Image* backgroundImage;
        UI::Image* foregroundImage;
        UI::Image* centerLine;
        float opacity;
    };

    static std::vector<SliceVisual> activeSlices;
    static float nextNoteTime = 0.0f;
    static Transform* rootTransform = nullptr;

    static Sprite* arrowSprite = nullptr;
    static Sprite* dotSprite = nullptr;
    static Sprite* arrowBackgroundSprite = nullptr;
    static Sprite* dotBackgroundSprite = nullptr;
    static Material* uiMaterial = nullptr;
    static BeatmapObjectSpawnController* spawnController = nullptr;

    static Color leftSaberColor = Color::get_red();
    static Color rightSaberColor = Color::get_blue();

    UI::Image* CreateImageComponent(Transform* parent, Sprite* sprite, const std::string& name) {
        auto gameObject = GameObject::New_ctor(name);
        auto image = gameObject->AddComponent<UI::Image*>();
        image->set_sprite(sprite);
        if (uiMaterial) {
            image->set_material(uiMaterial);
        }
        auto transform = gameObject->get_transform();
        transform->SetParent(parent, false);
        return image;
    }

    bool Initialize() {
        LOG_INFO("Initializing SliceVisualizer with custom position and scale");
        
        activeSlices.clear();
        
        auto comboController = Object::FindObjectOfType<ComboUIController*>();
        if (!comboController) {
            LOG_ERROR("Failed to find ComboUIController");
            return false;
        }

        auto gameObject = GameObject::New_ctor("SliceVisualizerRoot");
        rootTransform = gameObject->get_transform();
        
        // Apply user-configured position
        float posX = getModConfig().PositionX.GetValue();
        float posY = getModConfig().PositionY.GetValue();
        float posZ = getModConfig().PositionZ.GetValue();
        
        rootTransform->set_position(Vector3(posX, posY, posZ));
        rootTransform->set_localScale(Vector3(0.01f, 0.01f, 0.01f));
        rootTransform->SetParent(comboController->get_transform(), true);
        
        LOG_INFO("SliceVisualizer initialized at position (%.1f, %.1f, %.1f)", posX, posY, posZ);
        return true;
    }

    bool CreateSprites() {
        LOG_INFO("Loading sprite assets");
        
        arrowSprite = BeatSaberUI::Base64ToSprite(arrowBase64);
        dotSprite = BeatSaberUI::Base64ToSprite(dotBase64);
        arrowBackgroundSprite = BeatSaberUI::Base64ToSprite(arrowBackgroundBase64);
        dotBackgroundSprite = BeatSaberUI::Base64ToSprite(dotBackgroundBase64);
        
        if (!arrowSprite || !dotSprite || !arrowBackgroundSprite || !dotBackgroundSprite) {
            LOG_ERROR("Failed to load one or more sprites from base64");
            return false;
        }

        auto materials = Resources::FindObjectsOfTypeAll<Material*>();
        uiMaterial = materials.First([](auto x) { return x && x->get_name() == "UINoGlow"; });
        if (!uiMaterial) {
            LOG_ERROR("Failed to find UINoGlow material");
            return false;
        }

        spawnController = Resources::FindObjectsOfTypeAll<BeatmapObjectSpawnController*>().FirstOrDefault();
        if (!spawnController) {
            LOG_ERROR("Failed to find BeatmapObjectSpawnController");
            return false;
        }

        LOG_INFO("Sprites loaded successfully");
        return true;
    }

    void SetSaberColors(Color leftColor, Color rightColor) {
        leftSaberColor = leftColor;
        rightSaberColor = rightColor;
    }

    void CreateSliceVisual(NoteCutInfo& cutInfo) {
        if (!rootTransform || !spawnController) {
            LOG_ERROR("Cannot create slice: root or spawn controller is null");
            return;
        }

        nextNoteTime = cutInfo.noteData->timeToNextColorNote;
        const float spriteSize = getModConfig().SliceScale.GetValue();
        const float initialOpacity = getModConfig().Opacity.GetValue();

        // Determine sprite type based on cut direction
        Sprite* bgSprite = (cutInfo.noteData->cutDirection == NoteCutDirection::Any) 
            ? dotBackgroundSprite 
            : arrowBackgroundSprite;
        Sprite* fgSprite = (cutInfo.noteData->cutDirection == NoteCutDirection::Any) 
            ? dotSprite 
            : arrowSprite;

        // Create parent for this slice visual
        auto sliceParent = GameObject::New_ctor("SliceGraphics")->get_transform();
        sliceParent->SetParent(rootTransform, false);
        
        // Rotation based on cut direction
        float rotationAngle = NoteCutDirectionExtensions::RotationAngle(cutInfo.noteData->cutDirection) 
                            + cutInfo.noteData->cutDirectionAngleOffset;
        sliceParent->set_localEulerAngles(Vector3(0.0f, 0.0f, rotationAngle));

        // Position based on note lane
        auto posOffset = spawnController->beatmapObjectSpawnMovementData->Get2DNoteOffset(
            cutInfo.noteData->lineIndex, 
            cutInfo.noteData->noteLineLayer
        );
        auto currentPos = sliceParent->get_position();
        sliceParent->set_position(Vector3(currentPos.x + posOffset.x, currentPos.y + posOffset.y, currentPos.z));
        sliceParent->set_localScale(Vector3(spriteSize, spriteSize, spriteSize));

        // Create background image
        auto background = CreateImageComponent(sliceParent, bgSprite, "Background");
        Color saberColor = (cutInfo.noteData->colorType == ColorType::ColorA) ? leftSaberColor : rightSaberColor;
        background->set_color(saberColor);
        background->get_gameObject()->AddComponent<UI::Mask*>()->set_showMaskGraphic(true);

        // Create foreground image
        auto foreground = CreateImageComponent(background->get_transform(), fgSprite, "Foreground");

        // Create cut direction line with configurable thickness
        auto line = CreateImageComponent(background->get_transform(), nullptr, "CutLine");
        line->set_color(Color::get_black());
        auto lineRect = line->GetComponent<RectTransform*>();
        float lineThickness = getModConfig().LineThickness.GetValue();
        lineRect->set_sizeDelta(Vector2(5.0f * lineThickness, 100.0f));
        
        auto lineTransform = line->get_transform();
        lineTransform->set_localScale(Vector3(1.0f / spriteSize, 1.0f / spriteSize, 1.0f / spriteSize));
        lineTransform->set_localEulerAngles(Vector3(0.0f, 0.0f, cutInfo.cutDirDeviation));
        lineTransform->set_localPosition(Vector3(
            (cutInfo.cutDistanceToCenter * 120.0f / spriteSize) - (2.3f / spriteSize),
            -2.3f / spriteSize,
            0.0f
        ));

        // Add to active slices with configured initial opacity
        activeSlices.push_back(SliceVisual{
            .parent = sliceParent->get_gameObject(),
            .backgroundImage = background,
            .foregroundImage = foreground,
            .centerLine = line,
            .opacity = initialOpacity
        });
    }

    void UpdateSlices() {
        for (auto iter = activeSlices.begin(); iter != activeSlices.end(); ) {
            auto& slice = *iter;
            
            float dynamicFade = 1.0f;
            if (getModConfig().Dynamic.GetValue()) {
                dynamicFade = std::clamp(2.0f - (nextNoteTime * 1.5f), 0.4f, 2.0f);
            }

            float fadeAmount = std::min(0.4f, 1.01f - slice.opacity);
            slice.opacity -= fadeAmount * Time::get_deltaTime() * getModConfig().FadeSpeed.GetValue() * 8.0f * dynamicFade;

            if (slice.opacity < 0.0f) {
                Object::Destroy(slice.parent);
                iter = activeSlices.erase(iter);
            } else {
                // Update colors with new opacity
                auto bgColor = slice.backgroundImage->get_color();
                bgColor.a = slice.opacity;
                slice.backgroundImage->set_color(bgColor);
                
                slice.foregroundImage->set_color(Color(1.0f, 1.0f, 1.0f, slice.opacity));
                
                auto lineColor = Color::get_black();
                lineColor.a = slice.opacity * 2.2f;
                slice.centerLine->set_color(lineColor);
                
                ++iter;
            }
        }
    }

    void Cleanup() {
        LOG_INFO("Cleaning up SliceVisualizer");
        for (auto& slice : activeSlices) {
            Object::Destroy(slice.parent);
        }
        activeSlices.clear();
        
        if (rootTransform) {
            Object::Destroy(rootTransform->get_gameObject());
            rootTransform = nullptr;
        }
    }

}
