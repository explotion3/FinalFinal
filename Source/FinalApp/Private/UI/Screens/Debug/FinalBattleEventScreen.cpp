#include "UI/Screens/Debug/FinalBattleEventScreen.h"

#include "BattleBridge/FinalBattleEventPresentationUtils.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Queries/FinalDataRegistry.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"

namespace
{
UTextBlock* CreateLedgerLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize, const FLinearColor& Color = FLinearColor::White)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}
}

void UFinalBattleEventScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleEventScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &UFinalBattleEventScreen::HandleBattleSnapshotChanged);
		BattleFlowSubsystem->OnBattleEventBroadcast.AddDynamic(this, &UFinalBattleEventScreen::HandleBattleEventBroadcast);
	}

	RefreshFromSubsystems();
}

void UFinalBattleEventScreen::NativeDestruct()
{
	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &UFinalBattleEventScreen::HandleBattleSnapshotChanged);
		BattleFlowSubsystem->OnBattleEventBroadcast.RemoveDynamic(this, &UFinalBattleEventScreen::HandleBattleEventBroadcast);
	}

	Super::NativeDestruct();
}

void UFinalBattleEventScreen::RefreshFromSubsystems()
{
	ConsumeIncrementalEvents();
	RebuildLedgerEntries();
}

void UFinalBattleEventScreen::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	if (!Snapshot.BattleId.IsValid())
	{
		CachedBattleId.Invalidate();
		CachedLedgerEvents.Reset();
		LastSeenEventSequence = 0;
	}

	RefreshFromSubsystems();
}

void UFinalBattleEventScreen::HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent)
{
	RefreshFromSubsystems();
}

void UFinalBattleEventScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("BattleEventLedgerRoot"));
	WidgetTree->RootWidget = RootOverlay;

	PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BattleEventLedgerPanel"));
	PanelBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.90f));
	PanelBorder->SetPadding(FMargin(12.0f));
	if (UOverlaySlot* PanelSlot = RootOverlay->AddChildToOverlay(PanelBorder))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Left);
		PanelSlot->SetVerticalAlignment(VAlign_Top);
		PanelSlot->SetPadding(FMargin(16.0f, 290.0f, 0.0f, 16.0f));
	}

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BattleEventLedgerContent"));
	PanelBorder->SetContent(ContentBox);

	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BattleEventLedgerHeaderRow"));
	ContentBox->AddChildToVerticalBox(HeaderRow);

	TitleText = CreateLedgerLabel(WidgetTree, TEXT("BattleEventLedgerTitle"), 15, FLinearColor(0.92f, 0.96f, 1.0f, 1.0f));
	TitleText->SetText(NSLOCTEXT("FinalBattleEventLedger", "Title", "Battle Event Ledger"));
	if (UHorizontalBoxSlot* TitleSlot = HeaderRow->AddChildToHorizontalBox(TitleText))
	{
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BattleEventLedgerCloseButton"));
	CloseButton->OnClicked.AddDynamic(this, &UFinalBattleEventScreen::HandleCloseClicked);
	CloseButtonLabel = CreateLedgerLabel(WidgetTree, TEXT("BattleEventLedgerCloseLabel"), 11, FLinearColor(0.95f, 0.95f, 0.95f, 1.0f));
	CloseButtonLabel->SetText(NSLOCTEXT("FinalBattleEventLedger", "CloseButton", "Close"));
	CloseButton->AddChild(CloseButtonLabel);
	HeaderRow->AddChildToHorizontalBox(CloseButton);

	StatusText = CreateLedgerLabel(WidgetTree, TEXT("BattleEventLedgerStatus"), 11, FLinearColor(0.82f, 0.87f, 0.96f, 1.0f));
	ContentBox->AddChildToVerticalBox(StatusText);

	EventScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("BattleEventLedgerScroll"));
	ContentBox->AddChildToVerticalBox(EventScrollBox);
}

void UFinalBattleEventScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalUISubsystem>() : nullptr)
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalBattleEventScreen::ResetLedgerFromBattleFlow()
{
	CachedLedgerEvents.Reset();
	LastSeenEventSequence = 0;

	if (const UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem())
	{
		CachedLedgerEvents = BattleFlowSubsystem->GetBattleLogEntries();
		LastSeenEventSequence = BattleFlowSubsystem->GetLatestBattleEventSequence();
		if (const UFinalBattleSession* Session = BattleFlowSubsystem->GetActiveBattleSession())
		{
			CachedBattleId = Session->GetSnapshot().BattleId;
		}
	}
}

void UFinalBattleEventScreen::ConsumeIncrementalEvents()
{
	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem();
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		CachedBattleId.Invalidate();
		CachedLedgerEvents.Reset();
		LastSeenEventSequence = 0;
		return;
	}

	const FFinalBattleSnapshot Snapshot = BattleFlowSubsystem->GetCurrentSnapshot();
	if (!CachedBattleId.IsValid() || CachedBattleId != Snapshot.BattleId || LastSeenEventSequence <= 0)
	{
		ResetLedgerFromBattleFlow();
		return;
	}

	const int32 LatestSequence = BattleFlowSubsystem->GetLatestBattleEventSequence();
	if (LatestSequence < LastSeenEventSequence)
	{
		ResetLedgerFromBattleFlow();
		return;
	}

	const TArray<FFinalBattleEvent> NewEvents = BattleFlowSubsystem->GetBattleEventsSince(LastSeenEventSequence);
	for (const FFinalBattleEvent& Event : NewEvents)
	{
		CachedLedgerEvents.Add(Event);
	}

	LastSeenEventSequence = LatestSequence;
}

void UFinalBattleEventScreen::RebuildLedgerEntries()
{
	if (StatusText == nullptr || EventScrollBox == nullptr)
	{
		return;
	}

	EventScrollBox->ClearChildren();

	const UFinalBattleFlowSubsystem* BattleFlowSubsystem = ResolveBattleFlowSubsystem();
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr || CachedLedgerEvents.IsEmpty())
	{
		StatusText->SetText(NSLOCTEXT("FinalBattleEventLedger", "NoBattleStatus", "当前没有活动战斗事件账本。"));
		UTextBlock* EmptyText = CreateLedgerLabel(WidgetTree, TEXT("BattleEventLedgerEmpty"), 11, FLinearColor(0.78f, 0.82f, 0.88f, 1.0f));
		EmptyText->SetText(NSLOCTEXT("FinalBattleEventLedger", "NoBattleEntries", "启动一场战斗后，这里会按事件序号显示最近 BattleEvent。"));
		EventScrollBox->AddChild(EmptyText);
		return;
	}

	StatusText->SetText(FText::Format(
		NSLOCTEXT("FinalBattleEventLedger", "LedgerStatusFormat", "Latest Seq {0} | Showing {1} recent entries"),
		FText::AsNumber(LastSeenEventSequence),
		FText::AsNumber(FMath::Min(CachedLedgerEvents.Num(), 12))));

	const UFinalDataRegistry* DataRegistry = ResolveDataRegistry();
	const FFinalBattleSnapshot Snapshot = BattleFlowSubsystem->GetCurrentSnapshot();
	const int32 StartIndex = FMath::Max(CachedLedgerEvents.Num() - 12, 0);
	for (int32 Index = CachedLedgerEvents.Num() - 1; Index >= StartIndex; --Index)
	{
		const FinalBattleEventPresentation::FEventPresentation EventPresentation =
			FinalBattleEventPresentation::BuildPresentation(CachedLedgerEvents[Index], Snapshot, DataRegistry);

		UTextBlock* EntryText = CreateLedgerLabel(
			WidgetTree,
			*FString::Printf(TEXT("BattleEventLedgerEntry_%d"), CachedLedgerEvents[Index].EventSequence),
			11,
			FLinearColor(0.92f, 0.92f, 0.94f, 1.0f));
		EntryText->SetText(EventPresentation.LedgerText);
		EventScrollBox->AddChild(EntryText);
	}
}

UFinalBattleFlowSubsystem* UFinalBattleEventScreen::ResolveBattleFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
}

UFinalDataRegistry* UFinalBattleEventScreen::ResolveDataRegistry() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
}
