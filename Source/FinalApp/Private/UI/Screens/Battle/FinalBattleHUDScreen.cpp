#include "UI/Screens/Battle/FinalBattleHUDScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleLogEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleUltimateEntryWidget.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

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
	Segments.Reserve(Texts.Num());
	for (const FText& Entry : Texts)
	{
		Segments.Add(Entry.ToString());
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
}
}

void UFinalBattleHUDScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleHUDScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (BattleViewModel)
	{
		BattleViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleHUDScreen::HandleViewModelChanged);
	}

	RefreshFromViewModel();
}

void UFinalBattleHUDScreen::NativeDestruct()
{
	if (BattleViewModel)
	{
		BattleViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleHUDScreen::HandleViewModelChanged);
	}

	Super::NativeDestruct();
}

void UFinalBattleHUDScreen::InitializeScreen(UFinalBattleHUDViewModel* InViewModel, UFinalBattleWidgetController* InController)
{
	BattleViewModel = InViewModel;
	BattleController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleHUDScreen::HandleEnemySelected(const FName RuntimeUnitId)
{
	if (BattleController)
	{
		BattleController->SelectEnemyByUnitId(RuntimeUnitId);
	}
}

void UFinalBattleHUDScreen::HandlePlayCard(const int32 HandIndex)
{
	if (BattleController)
	{
		BattleController->PlayCardByHandIndex(HandIndex);
	}
}

void UFinalBattleHUDScreen::HandlePlayUltimate(const int32 CharacterIndex)
{
	if (BattleController)
	{
		BattleController->PlayUltimateByCharacterIndex(CharacterIndex);
	}
}

void UFinalBattleHUDScreen::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleHUDScreen::HandleEndTurnClicked()
{
	if (BattleController)
	{
		BattleController->EndTurn();
	}
}

void UFinalBattleHUDScreen::HandleOpenDebugClicked()
{
	if (UFinalUISubsystem* UISubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalUISubsystem>() : nullptr)
	{
		UISubsystem->OpenPrototypeRunDebugOverlay();
	}
}

void UFinalBattleHUDScreen::HandleOpenEventLedgerClicked()
{
	if (UFinalUISubsystem* UISubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalUISubsystem>() : nullptr)
	{
		UISubsystem->OpenBattleEventOverlay();
	}
}

void UFinalBattleHUDScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	ScreenLayer = EFinalUIScreenLayer::HUD;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BattleHUDRoot"));
	WidgetTree->RootWidget = RootBox;

	UBorder* TopBarBorder = CreateSection(WidgetTree, TEXT("TopBarBorder"), FLinearColor(0.06f, 0.08f, 0.13f, 0.95f));
	TopBarText = CreateLabel(WidgetTree, TEXT("TopBarText"), 18);
	TopBarBorder->SetContent(TopBarText);
	RootBox->AddChildToVerticalBox(TopBarBorder);

	UBorder* FeedbackBorder = CreateSection(WidgetTree, TEXT("FeedbackBorder"), FLinearColor(0.17f, 0.13f, 0.06f, 0.92f));
	FeedbackText = CreateLabel(WidgetTree, TEXT("FeedbackText"), 14);
	FeedbackBorder->SetContent(FeedbackText);
	RootBox->AddChildToVerticalBox(FeedbackBorder);

	UBorder* ContextBorder = CreateSection(WidgetTree, TEXT("ContextBorder"), FLinearColor(0.08f, 0.11f, 0.12f, 0.92f));
	AuxiliaryContextText = CreateLabel(WidgetTree, TEXT("AuxiliaryContextText"), 12);
	ContextBorder->SetContent(AuxiliaryContextText);
	RootBox->AddChildToVerticalBox(ContextBorder);

	GapBorder = CreateSection(WidgetTree, TEXT("GapBorder"), FLinearColor(0.14f, 0.08f, 0.08f, 0.92f));
	GapText = CreateLabel(WidgetTree, TEXT("GapText"), 12);
	GapBorder->SetContent(GapText);
	GapBorder->SetVisibility(ESlateVisibility::Collapsed);
	RootBox->AddChildToVerticalBox(GapBorder);

	UHorizontalBox* MiddleRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MiddleRow"));
	RootBox->AddChildToVerticalBox(MiddleRow);

	UBorder* CharacterBorder = CreateSection(WidgetTree, TEXT("CharacterBorder"), FLinearColor(0.08f, 0.12f, 0.18f, 0.92f));
	CharacterListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterListBox"));
	CharacterBorder->SetContent(CharacterListBox);
	if (UHorizontalBoxSlot* CharacterSlot = MiddleRow->AddChildToHorizontalBox(CharacterBorder))
	{
		CharacterSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CharacterSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	UBorder* EnemyBorder = CreateSection(WidgetTree, TEXT("EnemyBorder"), FLinearColor(0.18f, 0.09f, 0.11f, 0.92f));
	EnemyListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EnemyListBox"));
	EnemyBorder->SetContent(EnemyListBox);
	if (UHorizontalBoxSlot* EnemySlot = MiddleRow->AddChildToHorizontalBox(EnemyBorder))
	{
		EnemySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UHorizontalBox* BottomRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
	RootBox->AddChildToVerticalBox(BottomRow);

	UBorder* RecentEventBorder = CreateSection(WidgetTree, TEXT("RecentEventBorder"), FLinearColor(0.08f, 0.08f, 0.08f, 0.92f));
	RecentEventListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RecentEventListBox"));
	RecentEventBorder->SetContent(RecentEventListBox);
	if (UHorizontalBoxSlot* EventSlot = BottomRow->AddChildToHorizontalBox(RecentEventBorder))
	{
		EventSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		EventSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	UBorder* HandBorder = CreateSection(WidgetTree, TEXT("HandBorder"), FLinearColor(0.08f, 0.11f, 0.16f, 0.92f));
	HandCardBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HandCardBox"));
	HandBorder->SetContent(HandCardBox);
	if (UHorizontalBoxSlot* HandSlot = BottomRow->AddChildToHorizontalBox(HandBorder))
	{
		HandSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		HandSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	UBorder* ActionBorder = CreateSection(WidgetTree, TEXT("ActionBorder"), FLinearColor(0.09f, 0.13f, 0.08f, 0.92f));
	UVerticalBox* ActionColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ActionColumn"));
	ActionBorder->SetContent(ActionColumn);

	EndTurnButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EndTurnButton"));
	EndTurnButton->OnClicked.AddDynamic(this, &UFinalBattleHUDScreen::HandleEndTurnClicked);
	EndTurnLabel = CreateLabel(WidgetTree, TEXT("EndTurnLabel"), 14);
	EndTurnLabel->SetText(NSLOCTEXT("FinalBattleHUD", "EndTurnLabel", "结束回合"));
	EndTurnButton->AddChild(EndTurnLabel);
	ActionColumn->AddChildToVerticalBox(EndTurnButton);

	OpenDebugButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OpenDebugButton"));
	OpenDebugButton->OnClicked.AddDynamic(this, &UFinalBattleHUDScreen::HandleOpenDebugClicked);
	OpenDebugLabel = CreateLabel(WidgetTree, TEXT("OpenDebugLabel"), 12);
	OpenDebugLabel->SetText(NSLOCTEXT("FinalBattleHUD", "OpenDebugLabel", "Open Debug"));
	OpenDebugButton->AddChild(OpenDebugLabel);
	if (UVerticalBoxSlot* DebugSlot = ActionColumn->AddChildToVerticalBox(OpenDebugButton))
	{
		DebugSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	OpenEventLedgerButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OpenEventLedgerButton"));
	OpenEventLedgerButton->OnClicked.AddDynamic(this, &UFinalBattleHUDScreen::HandleOpenEventLedgerClicked);
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

	UltimateButtonBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("UltimateButtonBox"));
	ActionColumn->AddChildToVerticalBox(UltimateButtonBox);

	if (UHorizontalBoxSlot* ActionSlot = BottomRow->AddChildToHorizontalBox(ActionBorder))
	{
		ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
}

void UFinalBattleHUDScreen::RefreshFromViewModel()
{
	if (BattleViewModel == nullptr || TopBarText == nullptr)
	{
		return;
	}

	const FFinalBattleHUDPresentationData Presentation = BattleViewModel->GetPresentation();
	RefreshTopBarSection(Presentation);
	RefreshFeedbackSection(Presentation);
	RefreshContextSection(Presentation);
	RefreshCharacterPanel(Presentation);
	RefreshEnemyPanel(Presentation);
	RefreshHandPanel(Presentation);
	RefreshUltimatePanel(Presentation);
	RefreshRecentEventPanel(Presentation);
	RefreshActionSection(Presentation);
}

void UFinalBattleHUDScreen::RefreshTopBarSection(const FFinalBattleHUDPresentationData& Presentation)
{
	if (Presentation.bHasActiveBattle)
	{
		TopBarText->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "TopBarFormat", "{0} | Round {1} | AP {2} | EP {3}/{4} | Team HP {5}/{6} | Shield {7}"),
			Presentation.EncounterName,
			FText::AsNumber(Presentation.CurrentRound),
			FText::AsNumber(Presentation.CurrentAP),
			FText::AsNumber(Presentation.CurrentEP),
			FText::AsNumber(Presentation.MaxEP),
			FText::AsNumber(Presentation.TeamCurrentHP),
			FText::AsNumber(Presentation.TeamMaxHP),
			FText::AsNumber(Presentation.TeamShield)));
		return;
	}

	TopBarText->SetText(NSLOCTEXT("FinalBattleHUD", "NoBattleHeader", "Battle HUD ready. No active battle session."));
}

void UFinalBattleHUDScreen::RefreshFeedbackSection(const FFinalBattleHUDPresentationData& Presentation)
{
	if (!Presentation.bHasActiveBattle)
	{
		FeedbackText->SetText(NSLOCTEXT("FinalBattleHUD", "NoBattleFeedback", "通过控制台命令或地图按钮启动测试战斗后，这里会自动刷新。"));
		return;
	}

	const FText CombinedFeedbackText = !Presentation.FeedbackTitleText.IsEmpty()
		? (!Presentation.FeedbackText.IsEmpty()
			? FText::Format(
				NSLOCTEXT("FinalBattleHUD", "FeedbackWithTitleFormat", "{0}\n{1}"),
				Presentation.FeedbackTitleText,
				Presentation.FeedbackText)
			: Presentation.FeedbackTitleText)
		: Presentation.FeedbackText;
	FeedbackText->SetText(CombinedFeedbackText);
}

void UFinalBattleHUDScreen::RefreshContextSection(const FFinalBattleHUDPresentationData& Presentation)
{
	if (!Presentation.bHasActiveBattle)
	{
		AuxiliaryContextText->SetText(FText::GetEmpty());
		GapText->SetText(FText::GetEmpty());
		GapBorder->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	AuxiliaryContextText->SetText(FText::Format(
		NSLOCTEXT("FinalBattleHUD", "ContextFormat", "{0}\nDeck: Draw {1} | Hand {2} | Discard {3} | Ongoing {4} | Consume {5}\nRun: Gold {6} | Relics {7} | RunDeck {8}\nTeam Status: {9}\nActive Relics: {10}"),
		Presentation.CurrentTargetText,
		FText::AsNumber(Presentation.DrawPileCount),
		FText::AsNumber(Presentation.HandCount),
		FText::AsNumber(Presentation.DiscardPileCount),
		FText::AsNumber(Presentation.OngoingZoneCount),
		FText::AsNumber(Presentation.ConsumePileCount),
		FText::AsNumber(Presentation.Gold),
		FText::AsNumber(Presentation.RelicCount),
		FText::AsNumber(Presentation.RunDeckCount),
		JoinTextArray(Presentation.TeamStatusTexts, NSLOCTEXT("FinalBattleHUD", "NoTeamStatus", "无")),
		JoinTextArray(Presentation.ActiveRelicTexts, NSLOCTEXT("FinalBattleHUD", "NoActiveRelics", "无已激活遗物"))));

	if (Presentation.MissingFieldNotices.Num() > 0)
	{
		GapText->SetText(JoinTextArray(Presentation.MissingFieldNotices, FText::GetEmpty()));
		GapBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GapText->SetText(FText::GetEmpty());
		GapBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalBattleHUDScreen::RefreshCharacterPanel(const FFinalBattleHUDPresentationData& Presentation)
{
	if (CharacterListBox == nullptr)
	{
		return;
	}

	CharacterListBox->ClearChildren();
	for (const FFinalBattleHUDCharacterEntry& Entry : Presentation.Characters)
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

void UFinalBattleHUDScreen::RefreshEnemyPanel(const FFinalBattleHUDPresentationData& Presentation)
{
	if (EnemyListBox == nullptr)
	{
		return;
	}

	EnemyListBox->ClearChildren();
	for (const FFinalBattleHUDEnemyEntry& Entry : Presentation.Enemies)
	{
		UFinalBattleEnemyEntryWidget* EnemyWidget = CreateWidget<UFinalBattleEnemyEntryWidget>(GetOwningPlayer(), UFinalBattleEnemyEntryWidget::StaticClass());
		if (EnemyWidget == nullptr)
		{
			continue;
		}

		EnemyWidget->Configure(this, Entry);
		EnemyListBox->AddChildToVerticalBox(EnemyWidget);
	}
}

void UFinalBattleHUDScreen::RefreshHandPanel(const FFinalBattleHUDPresentationData& Presentation)
{
	if (HandCardBox == nullptr)
	{
		return;
	}

	HandCardBox->ClearChildren();
	for (int32 Index = 0; Index < Presentation.HandCards.Num(); ++Index)
	{
		UFinalBattleCardEntryWidget* CardWidget = CreateWidget<UFinalBattleCardEntryWidget>(GetOwningPlayer(), UFinalBattleCardEntryWidget::StaticClass());
		if (CardWidget == nullptr)
		{
			continue;
		}

		CardWidget->Configure(this, Index, Presentation.HandCards[Index]);
		if (UHorizontalBoxSlot* CardSlot = HandCardBox->AddChildToHorizontalBox(CardWidget))
		{
			CardSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		}
	}
}

void UFinalBattleHUDScreen::RefreshUltimatePanel(const FFinalBattleHUDPresentationData& Presentation)
{
	if (UltimateButtonBox == nullptr)
	{
		return;
	}

	UltimateButtonBox->ClearChildren();
	for (int32 Index = 0; Index < Presentation.Ultimates.Num(); ++Index)
	{
		UFinalBattleUltimateEntryWidget* UltimateWidget = CreateWidget<UFinalBattleUltimateEntryWidget>(GetOwningPlayer(), UFinalBattleUltimateEntryWidget::StaticClass());
		if (UltimateWidget == nullptr)
		{
			continue;
		}

		UltimateWidget->Configure(this, Index, Presentation.Ultimates[Index]);
		UltimateButtonBox->AddChildToVerticalBox(UltimateWidget);
	}
}

void UFinalBattleHUDScreen::RefreshRecentEventPanel(const FFinalBattleHUDPresentationData& Presentation)
{
	if (RecentEventListBox == nullptr)
	{
		return;
	}

	RecentEventListBox->ClearChildren();
	for (const FFinalBattleHUDLogEntry& Entry : Presentation.LogEntries)
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

void UFinalBattleHUDScreen::RefreshActionSection(const FFinalBattleHUDPresentationData& Presentation)
{
	const bool bHasActiveBattle = Presentation.bHasActiveBattle;
	if (EndTurnButton)
	{
		EndTurnButton->SetIsEnabled(bHasActiveBattle);
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
