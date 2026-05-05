#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleHUDCardZoneInspectStateTest,
	"Final.Editor.BattleHUD.CardZoneDetail.InspectState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleHUDCardZoneInspectStateTest::RunTest(const FString& Parameters)
{
	UFinalBattleHUDViewModel* ViewModel = NewObject<UFinalBattleHUDViewModel>();
	UFinalBattleWidgetController* Controller = NewObject<UFinalBattleWidgetController>();
	Controller->Initialize(ViewModel);

	TestFalse(TEXT("Card zone detail should start closed."), Controller->IsCardZoneDetailOpen());
	TestEqual(TEXT("Default selected card zone should be DrawPile."), Controller->GetSelectedCardZone(), EFinalBattleCardZone::DrawPile);

	Controller->InspectCardZone(EFinalBattleCardZone::OngoingZone);
	TestTrue(TEXT("InspectCardZone should open the detail panel."), Controller->IsCardZoneDetailOpen());
	TestEqual(TEXT("InspectCardZone should select the requested zone."), Controller->GetSelectedCardZone(), EFinalBattleCardZone::OngoingZone);

	Controller->SetSelectedCardZone(EFinalBattleCardZone::ConsumePile);
	TestTrue(TEXT("Tab selection should keep the detail panel open."), Controller->IsCardZoneDetailOpen());
	TestEqual(TEXT("SetSelectedCardZone should change only the inspected zone."), Controller->GetSelectedCardZone(), EFinalBattleCardZone::ConsumePile);

	Controller->ClearCardZoneDetail();
	TestFalse(TEXT("ClearCardZoneDetail should close the detail panel."), Controller->IsCardZoneDetailOpen());
	TestEqual(TEXT("ClearCardZoneDetail should preserve the last selected tab for next open."), Controller->GetSelectedCardZone(), EFinalBattleCardZone::ConsumePile);

	return true;
}

#endif
