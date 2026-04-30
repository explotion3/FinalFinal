#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleHUDFeedbackPassiveTitleMappingTest,
	"Final.Editor.BattleHUD.FeedbackTitles.MapPassiveEvents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleHUDFeedbackPassiveTitleMappingTest::RunTest(const FString& Parameters)
{
	TArray<FFinalBattleStartRelicInput> ActiveRelics;

	FFinalBattleEvent PassiveAppliedEvent;
	PassiveAppliedEvent.EventType = EFinalBattleEventType::PassiveApplied;
	TestEqual(
		TEXT("PassiveApplied should map to 被动获得."),
		ResolveBattleHUDEventFeedbackTitleText(PassiveAppliedEvent, FText::GetEmpty(), ActiveRelics).ToString(),
		FString(TEXT("被动获得")));

	FFinalBattleEvent PassiveTriggeredEvent;
	PassiveTriggeredEvent.EventType = EFinalBattleEventType::PassiveTriggered;
	TestEqual(
		TEXT("PassiveTriggered should map to 被动触发."),
		ResolveBattleHUDEventFeedbackTitleText(PassiveTriggeredEvent, FText::GetEmpty(), ActiveRelics).ToString(),
		FString(TEXT("被动触发")));

	FFinalBattleEvent PassiveRemovedEvent;
	PassiveRemovedEvent.EventType = EFinalBattleEventType::PassiveRemoved;
	TestEqual(
		TEXT("PassiveRemoved should map to 被动失效."),
		ResolveBattleHUDEventFeedbackTitleText(PassiveRemovedEvent, FText::GetEmpty(), ActiveRelics).ToString(),
		FString(TEXT("被动失效")));

	FFinalBattleStartRelicInput RelicInput;
	RelicInput.RelicId = FFinalRelicId(FName(TEXT("relic.test.feedback")));
	RelicInput.DisplayName = FText::FromString(TEXT("试作遗物"));
	ActiveRelics.Add(RelicInput);

	FFinalBattleEvent RelicTriggeredEvent;
	RelicTriggeredEvent.EventType = EFinalBattleEventType::RelicTriggered;
	RelicTriggeredEvent.RelicId = RelicInput.RelicId;
	TestEqual(
		TEXT("RelicTriggered title mapping should remain unchanged."),
		ResolveBattleHUDEventFeedbackTitleText(RelicTriggeredEvent, FText::GetEmpty(), ActiveRelics).ToString(),
		FString(TEXT("遗物触发 · 试作遗物")));

	return true;
}

#endif
