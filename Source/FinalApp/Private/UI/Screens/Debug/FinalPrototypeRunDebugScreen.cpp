#include "UI/Screens/Debug/FinalPrototypeRunDebugScreen.h"

#include "App/FinalGameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"

namespace
{
UTextBlock* CreatePrototypeLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize, const FLinearColor& Color = FLinearColor::White)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

FText GetFlowStageText(const EFinalRunFlowStage FlowStage)
{
	switch (FlowStage)
	{
	case EFinalRunFlowStage::PreparingBattle:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPreparingBattle", "PreparingBattle");

	case EFinalRunFlowStage::PendingBattleReward:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingBattleReward", "PendingBattleReward");

	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowAwaitingNodeAdvance", "AwaitingNodeAdvance");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingRewardNode", "PendingRewardNode");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingEventNode", "PendingEventNode");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowPendingShopNode", "PendingShopNode");

	case EFinalRunFlowStage::RunEnded:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowRunEnded", "RunEnded");

	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalPrototypeRunDebug", "FlowNone", "None");
	}
}

FText GetNodeSummaryText(const FFinalRunProgressionViewData& Progression)
{
	const FText NodeName = !Progression.CurrentNodeDisplayName.IsEmpty()
		? Progression.CurrentNodeDisplayName
		: NSLOCTEXT("FinalPrototypeRunDebug", "NoCurrentNodeName", "No current node");

	return FText::Format(
		NSLOCTEXT("FinalPrototypeRunDebug", "NodeSummaryFormat", "{0} | Chapter {1} Floor {2}"),
		NodeName,
		FText::AsNumber(Progression.CurrentChapter),
		FText::AsNumber(Progression.CurrentFloor));
}

FText GetLatestDebugMessage(
	const UFinalRunFlowSubsystem* RunFlowSubsystem,
	const UFinalGameFlowSubsystem* GameFlowSubsystem,
	const UFinalGameInstance* FinalGameInstance)
{
	if (RunFlowSubsystem != nullptr)
	{
		const FText LastFlowMessage = RunFlowSubsystem->GetLastFlowMessage();
		if (!LastFlowMessage.IsEmpty())
		{
			return LastFlowMessage;
		}

		const FFinalRunEvent LastRunEvent = RunFlowSubsystem->GetLastProcessedRunEvent();
		if (!LastRunEvent.Message.IsEmpty())
		{
			return LastRunEvent.Message;
		}
	}

	if (GameFlowSubsystem != nullptr)
	{
		const FText LastBattleFailure = GameFlowSubsystem->GetLastBattleFailureReason();
		if (!LastBattleFailure.IsEmpty())
		{
			return LastBattleFailure;
		}
	}

	if (FinalGameInstance != nullptr)
	{
		return FinalGameInstance->GetLastTestFailureReason();
	}

	return FText::GetEmpty();
}
}

void UFinalPrototypeRunDebugScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalPrototypeRunDebugScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		RunFlowSubsystem->OnRunFlowStateChanged.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleRunFlowStateChanged);
	}

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleBattleSnapshotChanged);
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::NativeDestruct()
{
	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		RunFlowSubsystem->OnRunFlowStateChanged.RemoveDynamic(this, &UFinalPrototypeRunDebugScreen::HandleRunFlowStateChanged);
	}

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &UFinalPrototypeRunDebugScreen::HandleBattleSnapshotChanged);
	}

	Super::NativeDestruct();
}

void UFinalPrototypeRunDebugScreen::RefreshFromSubsystems()
{
	if (SummaryText == nullptr || MessageText == nullptr)
	{
		return;
	}

	const UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	const UFinalGameFlowSubsystem* GameFlowSubsystem = ResolveGameFlowSubsystem();
	const UFinalGameInstance* FinalGameInstance = ResolveFinalGameInstance();

	const FFinalRunSnapshot RunSnapshot = RunFlowSubsystem ? RunFlowSubsystem->GetCurrentRunSnapshot() : FFinalRunSnapshot{};
	const FFinalBattleSnapshot BattleSnapshot = GameFlowSubsystem ? GameFlowSubsystem->GetCurrentBattleSnapshot() : FFinalBattleSnapshot{};
	const bool bHasActiveBattleSession = GameFlowSubsystem && GameFlowSubsystem->GetActiveBattleSession() != nullptr;

	SummaryText->SetText(FText::Format(
		NSLOCTEXT("FinalPrototypeRunDebug", "SummaryFormat", "FlowStage: {0}\nNode: {1}\nGold {2} | Deck {3} | Relics {4}\nActiveBattleSession: {5}"),
		GetFlowStageText(RunSnapshot.Progression.FlowStage),
		GetNodeSummaryText(RunSnapshot.Progression),
		FText::AsNumber(RunSnapshot.Gold),
		FText::AsNumber(RunSnapshot.DeckCount),
		FText::AsNumber(RunSnapshot.RelicCount),
		bHasActiveBattleSession
			? (BattleSnapshot.bBattleEnded
				? NSLOCTEXT("FinalPrototypeRunDebug", "BattleEnded", "Yes (Resolved)")
				: NSLOCTEXT("FinalPrototypeRunDebug", "BattleActive", "Yes"))
			: NSLOCTEXT("FinalPrototypeRunDebug", "BattleInactive", "No")));

	const FText LatestMessage = GetLatestDebugMessage(RunFlowSubsystem, GameFlowSubsystem, FinalGameInstance);
	MessageText->SetText(!LatestMessage.IsEmpty()
		? LatestMessage
		: NSLOCTEXT("FinalPrototypeRunDebug", "NoLatestMessage", "No recent flow feedback."));

	if (RestartRunButton)
	{
		RestartRunButton->SetIsEnabled(true);
	}

	if (CompleteResolvedBattleButton)
	{
		CompleteResolvedBattleButton->SetIsEnabled(bHasActiveBattleSession && BattleSnapshot.bBattleEnded);
	}
}

void UFinalPrototypeRunDebugScreen::HandleRunFlowStateChanged()
{
	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleRestartPrototypeRunClicked()
{
	if (UFinalGameInstance* FinalGameInstance = ResolveFinalGameInstance())
	{
		FinalGameInstance->StartTestBattle();
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::HandleCompleteResolvedBattleClicked()
{
	if (UFinalGameFlowSubsystem* GameFlowSubsystem = ResolveGameFlowSubsystem())
	{
		GameFlowSubsystem->CompleteResolvedBattle();
	}

	RefreshFromSubsystems();
}

void UFinalPrototypeRunDebugScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	ScreenLayer = EFinalUIScreenLayer::HUD;

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PrototypeRunDebugRoot"));
	WidgetTree->RootWidget = RootOverlay;

	PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PrototypeRunDebugPanel"));
	PanelBorder->SetBrushColor(FLinearColor(0.05f, 0.06f, 0.09f, 0.88f));
	PanelBorder->SetPadding(FMargin(12.0f));
	if (UOverlaySlot* PanelSlot = RootOverlay->AddChildToOverlay(PanelBorder))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Right);
		PanelSlot->SetVerticalAlignment(VAlign_Top);
		PanelSlot->SetPadding(FMargin(16.0f));
	}

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PrototypeRunDebugContent"));
	PanelBorder->SetContent(ContentBox);

	TitleText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugTitle"), 15, FLinearColor(0.92f, 0.96f, 1.0f, 1.0f));
	TitleText->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "Title", "Prototype Run Summary"));
	ContentBox->AddChildToVerticalBox(TitleText);

	SummaryText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugSummary"), 12);
	if (UVerticalBoxSlot* SummarySlot = ContentBox->AddChildToVerticalBox(SummaryText))
	{
		SummarySlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	MessageText = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugMessage"), 11, FLinearColor(0.86f, 0.88f, 0.92f, 1.0f));
	if (UVerticalBoxSlot* MessageSlot = ContentBox->AddChildToVerticalBox(MessageText))
	{
		MessageSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	RestartRunButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PrototypeRunDebugRestartButton"));
	RestartRunButton->OnClicked.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleRestartPrototypeRunClicked);
	RestartRunLabel = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugRestartLabel"), 12);
	RestartRunLabel->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "RestartRunButton", "Restart Prototype Run"));
	RestartRunButton->AddChild(RestartRunLabel);
	if (UVerticalBoxSlot* RestartSlot = ContentBox->AddChildToVerticalBox(RestartRunButton))
	{
		RestartSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}

	CompleteResolvedBattleButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PrototypeRunDebugCompleteResolvedBattleButton"));
	CompleteResolvedBattleButton->OnClicked.AddDynamic(this, &UFinalPrototypeRunDebugScreen::HandleCompleteResolvedBattleClicked);
	CompleteResolvedBattleLabel = CreatePrototypeLabel(WidgetTree, TEXT("PrototypeRunDebugCompleteResolvedBattleLabel"), 12);
	CompleteResolvedBattleLabel->SetText(NSLOCTEXT("FinalPrototypeRunDebug", "CompleteResolvedBattleButton", "Complete Resolved Battle"));
	CompleteResolvedBattleButton->AddChild(CompleteResolvedBattleLabel);
	ContentBox->AddChildToVerticalBox(CompleteResolvedBattleButton);
}

UFinalBattleFlowSubsystem* UFinalPrototypeRunDebugScreen::ResolveBattleFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
}

UFinalGameFlowSubsystem* UFinalPrototypeRunDebugScreen::ResolveGameFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr;
}

UFinalRunFlowSubsystem* UFinalPrototypeRunDebugScreen::ResolveRunFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr;
}

UFinalGameInstance* UFinalPrototypeRunDebugScreen::ResolveFinalGameInstance() const
{
	return Cast<UFinalGameInstance>(GetGameInstance());
}
