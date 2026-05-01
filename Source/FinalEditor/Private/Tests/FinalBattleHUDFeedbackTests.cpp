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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleHUDCardAPPlayHintTest,
	"Final.Editor.BattleHUD.CardEntry.APPlayHint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleHUDCardAPPlayHintTest::RunTest(const FString& Parameters)
{
	FFinalBattleHUDCardEntry AffordableEntry;
	AffordableEntry.RuntimeCostAP = 2;
	ApplyBattleHUDCardAPPlayHint(AffordableEntry, 2);
	TestTrue(TEXT("Card should be playable when AP equals runtime cost."), AffordableEntry.bCanPlayHint);
	TestTrue(TEXT("Playable card should not carry an unplayable hint."), AffordableEntry.UnplayableHintText.IsEmpty());

	FFinalBattleHUDCardEntry ExpensiveEntry;
	ExpensiveEntry.RuntimeCostAP = 3;
	ApplyBattleHUDCardAPPlayHint(ExpensiveEntry, 2);
	TestFalse(TEXT("Card should be visually marked unplayable when AP is insufficient."), ExpensiveEntry.bCanPlayHint);
	TestEqual(TEXT("AP hint text should be AP不足."), ExpensiveEntry.UnplayableHintText.ToString(), FString(TEXT("AP不足")));

	return true;
}

#endif
