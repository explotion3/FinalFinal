#include "UI/Panels/Battle/FinalBattleHUDPanels.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/ViewModels/Battle/FinalBattleHUDPanelViewModels.h"
#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleLogEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleUltimateEntryWidget.h"

namespace
{
UBorder* CreateSection(UWidgetTree* WidgetTree, const TCHAR* Name, const FLinearColor& Color)
{
	UBorder* Border = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
	Border->SetBrushColor(Color);
	Border->SetPadding(FMargin(10.0f));
	return Border;
}

UTextBlock* CreateLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize = 14)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

FText JoinTextArray(const TArray<FText>& Texts, const FText& EmptyText)
{
	if (Texts.Num() == 0)
	{
		return EmptyText;
	}

	TArray<FString> Segments;
	for (const FText& Entry : Texts)
	{
		Segments.Add(Entry.ToString());
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
}
}

void UFinalBattleTopBarPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleTopBarPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleTopBarPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleTopBarPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleTopBarPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleTopBarPanel::InitializePanel(UFinalBattleTopBarPanelViewModel* InViewModel, UFinalBattleTopBarPanelController* InController)
{
	PanelViewModel = InViewModel;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleTopBarPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleTopBarPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("TopBarBorder"), FLinearColor(0.06f, 0.08f, 0.13f, 0.95f));
	TopBarText = CreateLabel(WidgetTree, TEXT("TopBarText"), 18);
	Border->SetContent(TopBarText);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleTopBarPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || TopBarText == nullptr)
	{
		return;
	}

	const FFinalBattleTopBarPanelData& Data = PanelViewModel->GetData();
	if (Data.bHasActiveBattle)
	{
		TopBarText->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "TopBarFormat", "{0} | Round {1} | AP {2} | EP {3}/{4} | Team HP {5}/{6} | Shield {7}"),
			Data.EncounterName,
			FText::AsNumber(Data.CurrentRound),
			FText::AsNumber(Data.CurrentAP),
			FText::AsNumber(Data.CurrentEP),
			FText::AsNumber(Data.MaxEP),
			FText::AsNumber(Data.TeamCurrentHP),
			FText::AsNumber(Data.TeamMaxHP),
			FText::AsNumber(Data.TeamShield)));
		return;
	}

	TopBarText->SetText(NSLOCTEXT("FinalBattleHUD", "NoBattleHeader", "Battle HUD ready. No active battle session."));
}

void UFinalBattleFeedbackPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleFeedbackPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleFeedbackPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleFeedbackPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleFeedbackPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleFeedbackPanel::InitializePanel(UFinalBattleFeedbackPanelViewModel* InViewModel, UFinalBattleFeedbackPanelController* InController)
{
	PanelViewModel = InViewModel;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleFeedbackPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleFeedbackPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("FeedbackBorder"), FLinearColor(0.17f, 0.13f, 0.06f, 0.92f));
	FeedbackText = CreateLabel(WidgetTree, TEXT("FeedbackText"), 14);
	Border->SetContent(FeedbackText);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleFeedbackPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || FeedbackText == nullptr)
	{
		return;
	}

	const FFinalBattleFeedbackPanelData& Data = PanelViewModel->GetData();
	const FText CombinedFeedbackText = !Data.FeedbackTitleText.IsEmpty()
		? (!Data.FeedbackText.IsEmpty()
			? FText::Format(NSLOCTEXT("FinalBattleHUD", "FeedbackWithTitleFormat", "{0}\n{1}"), Data.FeedbackTitleText, Data.FeedbackText)
			: Data.FeedbackTitleText)
		: Data.FeedbackText;
	FeedbackText->SetText(CombinedFeedbackText);
}

void UFinalBattleContextPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleContextPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleContextPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleContextPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleContextPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleContextPanel::InitializePanel(UFinalBattleContextPanelViewModel* InViewModel, UFinalBattleContextPanelController* InController)
{
	PanelViewModel = InViewModel;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleContextPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleContextPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContextRootBox"));

	UBorder* ContextBorder = CreateSection(WidgetTree, TEXT("ContextBorder"), FLinearColor(0.08f, 0.11f, 0.12f, 0.92f));
	ContextText = CreateLabel(WidgetTree, TEXT("ContextText"), 12);
	ContextBorder->SetContent(ContextText);
	RootBox->AddChildToVerticalBox(ContextBorder);

	GapBorder = CreateSection(WidgetTree, TEXT("GapBorder"), FLinearColor(0.14f, 0.08f, 0.08f, 0.92f));
	GapText = CreateLabel(WidgetTree, TEXT("GapText"), 12);
	GapBorder->SetContent(GapText);
	GapBorder->SetVisibility(ESlateVisibility::Collapsed);
	RootBox->AddChildToVerticalBox(GapBorder);

	WidgetTree->RootWidget = RootBox;
}

void UFinalBattleContextPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || ContextText == nullptr || GapBorder == nullptr || GapText == nullptr)
	{
		return;
	}

	const FFinalBattleContextPanelData& Data = PanelViewModel->GetData();
	if (!Data.bHasActiveBattle)
	{
		ContextText->SetText(FText::GetEmpty());
		GapText->SetText(FText::GetEmpty());
		GapBorder->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	ContextText->SetText(FText::Format(
		NSLOCTEXT("FinalBattleHUD", "ContextFormat", "{0}\nDeck: Draw {1} | Hand {2} | Discard {3} | Ongoing {4} | Consume {5}\nRun: Gold {6} | Relics {7} | RunDeck {8}\nTeam Status: {9}\nActive Relics: {10}"),
		Data.CurrentTargetText,
		FText::AsNumber(Data.DrawPileCount),
		FText::AsNumber(Data.HandCount),
		FText::AsNumber(Data.DiscardPileCount),
		FText::AsNumber(Data.OngoingZoneCount),
		FText::AsNumber(Data.ConsumePileCount),
		FText::AsNumber(Data.Gold),
		FText::AsNumber(Data.RelicCount),
		FText::AsNumber(Data.RunDeckCount),
		JoinTextArray(Data.TeamStatusTexts, NSLOCTEXT("FinalBattleHUD", "NoTeamStatus", "无")),
		JoinTextArray(Data.ActiveRelicTexts, NSLOCTEXT("FinalBattleHUD", "NoActiveRelics", "无已激活遗物"))));

	if (Data.MissingFieldNotices.Num() > 0)
	{
		GapText->SetText(JoinTextArray(Data.MissingFieldNotices, FText::GetEmpty()));
		GapBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GapText->SetText(FText::GetEmpty());
		GapBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalBattleCharacterPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleCharacterPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleCharacterPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleCharacterPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleCharacterPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleCharacterPanel::InitializePanel(UFinalBattleCharacterPanelViewModel* InViewModel, UFinalBattleCharacterPanelController* InController)
{
	PanelViewModel = InViewModel;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleCharacterPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleCharacterPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("CharacterBorder"), FLinearColor(0.08f, 0.12f, 0.18f, 0.92f));
	CharacterListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterListBox"));
	Border->SetContent(CharacterListBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleCharacterPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || CharacterListBox == nullptr)
	{
		return;
	}

	CharacterListBox->ClearChildren();
	for (const FFinalBattleHUDCharacterEntry& Entry : PanelViewModel->GetEntries())
	{
		UFinalBattleCharacterEntryWidget* CharacterWidget = CreateWidget<UFinalBattleCharacterEntryWidget>(GetOwningPlayer(), UFinalBattleCharacterEntryWidget::StaticClass());
		if (CharacterWidget == nullptr)
		{
			continue;
		}

		CharacterWidget->Configure(Entry);
		CharacterListBox->AddChildToVerticalBox(CharacterWidget);
	}
}

void UFinalBattleEnemyPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleEnemyPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleEnemyPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleEnemyPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleEnemyPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleEnemyPanel::InitializePanel(UFinalBattleEnemyPanelViewModel* InViewModel, UFinalBattleEnemyPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleEnemyPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleEnemyPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("EnemyBorder"), FLinearColor(0.18f, 0.09f, 0.11f, 0.92f));
	EnemyListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EnemyListBox"));
	Border->SetContent(EnemyListBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleEnemyPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || EnemyListBox == nullptr)
	{
		return;
	}

	EnemyListBox->ClearChildren();
	for (const FFinalBattleHUDEnemyEntry& Entry : PanelViewModel->GetEntries())
	{
		UFinalBattleEnemyEntryWidget* EnemyWidget = CreateWidget<UFinalBattleEnemyEntryWidget>(GetOwningPlayer(), UFinalBattleEnemyEntryWidget::StaticClass());
		if (EnemyWidget == nullptr)
		{
			continue;
		}

		EnemyWidget->Configure(PanelController, Entry);
		EnemyListBox->AddChildToVerticalBox(EnemyWidget);
	}
}

void UFinalBattleHandPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleHandPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleHandPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleHandPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleHandPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleHandPanel::InitializePanel(UFinalBattleHandPanelViewModel* InViewModel, UFinalBattleHandPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleHandPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleHandPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("HandBorder"), FLinearColor(0.08f, 0.11f, 0.16f, 0.92f));
	HandCardBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HandCardBox"));
	Border->SetContent(HandCardBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleHandPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || HandCardBox == nullptr)
	{
		return;
	}

	HandCardBox->ClearChildren();
	const TArray<FFinalBattleHUDCardEntry>& Entries = PanelViewModel->GetEntries();
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		UFinalBattleCardEntryWidget* CardWidget = CreateWidget<UFinalBattleCardEntryWidget>(GetOwningPlayer(), UFinalBattleCardEntryWidget::StaticClass());
		if (CardWidget == nullptr)
		{
			continue;
		}

		CardWidget->Configure(PanelController, Index, Entries[Index]);
		if (UHorizontalBoxSlot* CardSlot = HandCardBox->AddChildToHorizontalBox(CardWidget))
		{
			CardSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		}
	}
}

void UFinalBattleUltimatePanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleUltimatePanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleUltimatePanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleUltimatePanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleUltimatePanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleUltimatePanel::InitializePanel(UFinalBattleUltimatePanelViewModel* InViewModel, UFinalBattleUltimatePanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleUltimatePanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleUltimatePanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("UltimateBorder"), FLinearColor(0.09f, 0.13f, 0.08f, 0.92f));
	UltimateListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("UltimateListBox"));
	Border->SetContent(UltimateListBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleUltimatePanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || UltimateListBox == nullptr)
	{
		return;
	}

	UltimateListBox->ClearChildren();
	const TArray<FFinalBattleHUDUltimateEntry>& Entries = PanelViewModel->GetEntries();
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		UFinalBattleUltimateEntryWidget* UltimateWidget = CreateWidget<UFinalBattleUltimateEntryWidget>(GetOwningPlayer(), UFinalBattleUltimateEntryWidget::StaticClass());
		if (UltimateWidget == nullptr)
		{
			continue;
		}

		UltimateWidget->Configure(PanelController, Index, Entries[Index]);
		UltimateListBox->AddChildToVerticalBox(UltimateWidget);
	}
}

void UFinalBattleRecentEventPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleRecentEventPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleRecentEventPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleRecentEventPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleRecentEventPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleRecentEventPanel::InitializePanel(UFinalBattleRecentEventPanelViewModel* InViewModel, UFinalBattleRecentEventPanelController* InController)
{
	PanelViewModel = InViewModel;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleRecentEventPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleRecentEventPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("RecentEventBorder"), FLinearColor(0.08f, 0.08f, 0.08f, 0.92f));
	RecentEventListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RecentEventListBox"));
	Border->SetContent(RecentEventListBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleRecentEventPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || RecentEventListBox == nullptr)
	{
		return;
	}

	RecentEventListBox->ClearChildren();
	for (const FFinalBattleHUDLogEntry& Entry : PanelViewModel->GetEntries())
	{
		UFinalBattleLogEntryWidget* EntryWidget = CreateWidget<UFinalBattleLogEntryWidget>(GetOwningPlayer(), UFinalBattleLogEntryWidget::StaticClass());
		if (EntryWidget == nullptr)
		{
			continue;
		}

		EntryWidget->Configure(Entry);
		RecentEventListBox->AddChildToVerticalBox(EntryWidget);
	}
}

void UFinalBattleActionPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleActionPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleActionPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleActionPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleActionPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleActionPanel::InitializePanel(UFinalBattleActionPanelViewModel* InViewModel, UFinalBattleActionPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleActionPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleActionPanel::HandleEndTurnClicked()
{
	if (PanelController)
	{
		PanelController->EndTurn();
	}
}

void UFinalBattleActionPanel::HandleOpenDebugClicked()
{
	if (PanelController)
	{
		PanelController->OpenDebugOverlay();
	}
}

void UFinalBattleActionPanel::HandleOpenEventLedgerClicked()
{
	if (PanelController)
	{
		PanelController->OpenEventLedgerOverlay();
	}
}

void UFinalBattleActionPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("ActionBorder"), FLinearColor(0.09f, 0.13f, 0.08f, 0.92f));
	UVerticalBox* ActionColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ActionColumn"));
	Border->SetContent(ActionColumn);

	EndTurnButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EndTurnButton"));
	EndTurnButton->OnClicked.AddDynamic(this, &UFinalBattleActionPanel::HandleEndTurnClicked);
	EndTurnLabel = CreateLabel(WidgetTree, TEXT("EndTurnLabel"), 14);
	EndTurnLabel->SetText(NSLOCTEXT("FinalBattleHUD", "EndTurnLabel", "结束回合"));
	EndTurnButton->AddChild(EndTurnLabel);
	ActionColumn->AddChildToVerticalBox(EndTurnButton);

	OpenDebugButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OpenDebugButton"));
	OpenDebugButton->OnClicked.AddDynamic(this, &UFinalBattleActionPanel::HandleOpenDebugClicked);
	OpenDebugLabel = CreateLabel(WidgetTree, TEXT("OpenDebugLabel"), 12);
	OpenDebugLabel->SetText(NSLOCTEXT("FinalBattleHUD", "OpenDebugLabel", "Open Debug"));
	OpenDebugButton->AddChild(OpenDebugLabel);
	if (UVerticalBoxSlot* DebugSlot = ActionColumn->AddChildToVerticalBox(OpenDebugButton))
	{
		DebugSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	OpenEventLedgerButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OpenEventLedgerButton"));
	OpenEventLedgerButton->OnClicked.AddDynamic(this, &UFinalBattleActionPanel::HandleOpenEventLedgerClicked);
	OpenEventLedgerLabel = CreateLabel(WidgetTree, TEXT("OpenEventLedgerLabel"), 12);
	OpenEventLedgerLabel->SetText(NSLOCTEXT("FinalBattleHUD", "OpenEventLedgerLabel", "Open Event Ledger"));
	OpenEventLedgerButton->AddChild(OpenEventLedgerLabel);
	if (UVerticalBoxSlot* LedgerSlot = ActionColumn->AddChildToVerticalBox(OpenEventLedgerButton))
	{
		LedgerSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("ActionSpacer"));
	Spacer->SetSize(FVector2D(8.0f, 12.0f));
	ActionColumn->AddChildToVerticalBox(Spacer);

	WidgetTree->RootWidget = Border;
}

void UFinalBattleActionPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr)
	{
		return;
	}

	const FFinalBattleActionPanelData& Data = PanelViewModel->GetData();
	if (EndTurnButton)
	{
		EndTurnButton->SetIsEnabled(Data.bHasActiveBattle);
	}

	if (OpenDebugButton)
	{
		OpenDebugButton->SetIsEnabled(true);
	}

	if (OpenEventLedgerButton)
	{
		OpenEventLedgerButton->SetIsEnabled(true);
	}
}
