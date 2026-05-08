#include "main.hpp"
#include "config.hpp"
#include "slices.hpp"
#include "questui/shared/QuestUI.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/ColorScheme.hpp"

using namespace GlobalNamespace;
using namespace SliceVisualizer;

static ModInfo modInfo;
bool initSuccessful = true;

Logger& getLogger() {
    static auto logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

Make_HOOK_MATCH(AudioTimeSyncController_Start, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {
    AudioTimeSyncController_Start(self);

    if (getModConfig().Enabled.GetValue() && !initSuccessful) {
        LOG_INFO("Reinitializing SliceVisualizer");
        initSuccessful = Initialize();
    }
}

Make_HOOK_MATCH(AudioTimeSyncController_StartSong, &AudioTimeSyncController::StartSong, void, AudioTimeSyncController* self, float startTimeOffset) {
    if (getModConfig().Enabled.GetValue() && initSuccessful) {
        LOG_INFO("Creating sprites for new song");
        if (!CreateSprites()) {
            LOG_ERROR("Failed to create sprites");
            initSuccessful = false;
        }
    }

    AudioTimeSyncController_StartSong(self, startTimeOffset);
}

Make_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {
    AudioTimeSyncController_Update(self);

    if (getModConfig().Enabled.GetValue() && initSuccessful) {
        UpdateSlices();
    }
}

Make_HOOK_MATCH(NoteController_SendNoteWasCutEvent, &NoteController::SendNoteWasCutEvent, void, NoteController* self, ByRef<NoteCutInfo> noteCutInfo) {
    if (getModConfig().Enabled.GetValue() && initSuccessful && 
        noteCutInfo->get_allIsOK() && 
        self->noteData->gameplayType != NoteData::GameplayType::BurstSliderElement) {
        CreateSliceVisual(noteCutInfo.heldRef);
    }

    NoteController_SendNoteWasCutEvent(self, noteCutInfo);
}

Make_HOOK_FIND_CLASS_UNSAFE_INSTANCE(GameplayCoreSceneSetupData_ctor, "", "GameplayCoreSceneSetupData", ".ctor", void, 
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
        auto layout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(self);
        layout->set_childControlHeight(false);
        layout->set_childForceExpandHeight(false);
        layout->set_spacing(1.0f);

        QuestUI::BeatSaberUI::AddConfigValueToggle(layout, getModConfig().Enabled);
        QuestUI::BeatSaberUI::AddConfigValueToggle(layout, getModConfig().Dynamic);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(layout, getModConfig().FadeSpeed, 0.5f, 0.1f, 0.5f, 3.0f);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(layout, getModConfig().SliceScale, 0.1f, 0.1f, 0.3f, 1.0f);
        QuestUI::BeatSaberUI::AddConfigValueIncrementFloat(layout, getModConfig().PositionZ, 1.0f, 1.0f, 10.0f, 20.0f);
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
