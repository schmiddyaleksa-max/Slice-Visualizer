#include "main.hpp"
#include "config.hpp"
#include "slices.hpp"
#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "HMUI/ViewController.hpp"
#include "HMUI/SimpleTextDropdown.hpp"

using namespace GlobalNamespace;
using namespace SliceVisualizer;

static ModInfo modInfo;
bool initSuccessful = true;

Logger& getLogger() {
    static auto logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

MACE_HOOK_MATCH(AudioTimeSyncController_Start, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {
    AudioTimeSyncController_Start(self);

    if (getModConfig().Enabled.GetValue() && !initSuccessful) {
        LOG_INFO("Reinitializing SliceVisualizer");
        initSuccessful = Initialize();
    }
}

MACE_HOOK_MATCH(AudioTimeSyncController_StartSong, &AudioTimeSyncController::StartSong, void, AudioTimeSyncController* self, float startTimeOffset) {
    if (getModConfig().Enabled.GetValue() && initSuccessful) {
        LOG_INFO("Creating sprites for new song");
        if (!CreateSprites()) {
            LOG_ERROR("Failed to create sprites");
            initSuccessful = false;
        }
    }

    AudioTimeSyncController_StartSong(self, startTimeOffset);
}

MACE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {
    AudioTimeSyncController_Update(self);

    if (getModConfig().Enabled.GetValue() && initSuccessful) {
        UpdateSlices();
    }
}

MACE_HOOK_MATCH(NoteController_SendNoteWasCutEvent, &NoteController::SendNoteWasCutEvent, void, NoteController* self, ByRef<NoteCutInfo> noteCutInfo) {
    if (getModConfig().Enabled.GetValue() && initSuccessful && 
        noteCutInfo->get_allIsOK() && 
        self->noteData->gameplayType != NoteData::GameplayType::BurstSliderElement) {
        CreateSliceVisual(noteCutInfo.heldRef);
    }

    NoteController_SendNoteWasCutEvent(self, noteCutInfo);
}

MACE_HOOK_FIND_CLASS_UNSAFE_INSTANCE(GameplayCoreSceneSetupData_ctor, "", "GameplayCoreSceneSetupData", ".ctor", void, 
    GameplayCoreSceneSetupData* self, IDifficultyBeatmap* f1, IPreviewBeatmapLevel* f2, 
    GameplayModifiers* f3, PlayerSpecificSettings* f4, PracticeSettings* f5, bool f6, 
    bool f7, bool f8, ColorScheme* colorScheme, AudioClip* f9, float f10, float f11, 
    EnvironmentInfoData* f12, ConfigFileData* f13, Dictionary* f14) {
    
    SetSaberColors(colorScheme->get_saberAColor(), colorScheme->get_saberBColor());
    initSuccessful = true;

    GameplayCoreSceneSetupData_ctor(self, f1, f2, f3, f4, f5, f6, f7, f8, colorScheme, f9, f10, f11, f12, f13, f14);
}

void SetupModSettings(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        auto container = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(self);
        container->set_childControlHeight(false);
        container->set_childForceExpandHeight(false);
        container->set_spacing(0.5f);
        container->set_padding({2, 2, 2, 2});
        
        // Title
        auto title = QuestUI::BeatSaberUI::CreateText(container, "<size=80%><b>Slice Visualizer Settings</b></size>");
        title->set_alignment(TMPro::TextAlignmentOptions::Center);
        
        // Main toggles section
        auto mainToggleLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(container);
        mainToggleLayout->set_childControlHeight(false);
        mainToggleLayout->set_childForceExpandHeight(false);
        mainToggleLayout->set_spacing(1.0f);
        
        QuestUI::BeatSaberUI::AddConfigValueToggle(mainToggleLayout, getModConfig().Enabled);
        QuestUI::BeatSaberUI::AddConfigValueToggle(mainToggleLayout, getModConfig().Dynamic);
        
        // Visual Settings
        auto visualHeader = QuestUI::BeatSaberUI::CreateText(container, "<size=70%><b>Visual Settings</b></size>");
        visualHeader->set_alignment(TMPro::TextAlignmentOptions::Center);
        visualHeader->set_fontSize(4);
        
        auto visualLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(container);
        visualLayout->set_childControlHeight(false);
        visualLayout->set_childForceExpandHeight(false);
        visualLayout->set_spacing(0.5f);
        
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(visualLayout, getModConfig().FadeSpeed, 0.1f, 0.1f, 0.1f, 3.0f);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(visualLayout, getModConfig().SliceScale, 0.05f, 0.1f, 0.3f, 1.0f);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(visualLayout, getModConfig().LineThickness, 0.1f, 0.1f, 0.5f, 2.0f);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(visualLayout, getModConfig().Opacity, 0.05f, 0.1f, 0.3f, 1.0f);
        
        // Position Settings
        auto positionHeader = QuestUI::BeatSaberUI::CreateText(container, "<size=70%><b>Position Settings</b></size>");
        positionHeader->set_alignment(TMPro::TextAlignmentOptions::Center);
        positionHeader->set_fontSize(4);
        
        auto positionLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(container);
        positionLayout->set_childControlHeight(false);
        positionLayout->set_childForceExpandHeight(false);
        positionLayout->set_spacing(0.5f);
        
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(positionLayout, getModConfig().PositionX, 0.5f, 0.1f, -5.0f, 5.0f);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(positionLayout, getModConfig().PositionY, 0.5f, 0.1f, 0.0f, 6.0f);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(positionLayout, getModConfig().PositionZ, 1.0f, 0.5f, 5.0f, 25.0f);
        
        // Info text
        auto infoText = QuestUI::BeatSaberUI::CreateText(container, "<size=60%>Settings apply to next song</size>");
        infoText->set_alignment(TMPro::TextAlignmentOptions::Center);
        infoText->set_fontSize(2);
    }
}

extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
    getModConfig().Init(modInfo);
    LOG_INFO("SliceVisualizer setup complete");
}

extern "C" void load() {
    LOG_INFO("Loading SliceVisualizer v%s", VERSION);
    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, SetupModSettings);

    LOG_INFO("Installing hooks...");
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Start);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_StartSong);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Update);
    INSTALL_HOOK(getLogger(), NoteController_SendNoteWasCutEvent);
    INSTALL_HOOK(getLogger(), GameplayCoreSceneSetupData_ctor);
    LOG_INFO("All hooks installed successfully!");
}
