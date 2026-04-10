#include "UI/Screens/Battle/FinalBattleHUDScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "Styling/CoreStyle.h"
#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"
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

UTextBlock* CreateLabel(UWidgetTree* WidgetTree, const TCHAR* Name, int32 FontSize = 14)
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

void UFinalBattleHUDScreen::HandleEnemySelected(FName RuntimeUnitId)
{
	if (BattleController)
	{
		BattleController->SelectEnemyByUnitId(RuntimeUnitId);
	}
}

void UFinalBattleHUDScreen::HandlePlayCard(int32 HandIndex)
{
	if (BattleController)
	{
		BattleController->PlayCardByHandIndex(HandIndex);
	}
}

void UFinalBattleHUDScreen::HandlePlayUltimate(int32 CharacterIndex)
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

void UFinalBattleHUDScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	ScreenLayer = EFinalUIScreenLayer::HUD;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BattleHUDRoot"));
	WidgetTree->RootWidget = RootBox;

	UBorder* HeaderBorder = CreateSection(WidgetTree, TEXT("HeaderBorder"), FLinearColor(0.06f, 0.08f, 0.13f, 0.95f));
	HeaderText = CreateLabel(WidgetTree, TEXT("HeaderText"), 18);
	HeaderBorder->SetContent(HeaderText);
	RootBox->AddChildToVerticalBox(HeaderBorder);

	UBorder* FeedbackBorder = CreateSection(WidgetTree, TEXT("FeedbackBorder"), FLinearColor(0.17f, 0.13f, 0.06f, 0.92f));
	FeedbackText = CreateLabel(WidgetTree, TEXT("FeedbackText"), 14);
	FeedbackBorder->SetContent(FeedbackText);
	RootBox->AddChildToVerticalBox(FeedbackBorder);

	UBorder* ContextBorder = CreateSection(WidgetTree, TEXT("ContextBorder"), FLinearColor(0.08f, 0.11f, 0.12f, 0.92f));
	ContextText = CreateLabel(WidgetTree, TEXT("ContextText"), 12);
	ContextBorder->SetContent(ContextText);
	RootBox->AddChildToVerticalBox(ContextBorder);

	UBorder* GapBorder = CreateSection(WidgetTree, TEXT("GapBorder"), FLinearColor(0.14f, 0.08f, 0.08f, 0.92f));
	GapText = CreateLabel(WidgetTree, TEXT("GapText"), 12);
	GapBorder->SetContent(GapText);
	RootBox->AddChildToVerticalBox(GapBorder);

	UHorizontalBox* MiddleRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MiddleRow"));
	RootBox->AddChildToVerticalBox(MiddleRow);

	UBorder* CharacterBorder = CreateSection(WidgetTree, TEXT("CharacterBorder"), FLinearColor(0.08f, 0.12f, 0.18f, 0.92f));
	CharacterListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterListBox"));
	CharacterBorder->SetContent(CharacterListBox);
	UHorizontalBoxSlot* CharacterSlot = MiddleRow->AddChildToHorizontalBox(CharacterBorder);
	CharacterSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	CharacterSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));

	UBorder* EnemyBorder = CreateSection(WidgetTree, TEXT("EnemyBorder"), FLinearColor(0.18f, 0.09f, 0.11f, 0.92f));
	EnemyListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EnemyListBox"));
	EnemyBorder->SetContent(EnemyListBox);
	UHorizontalBoxSlot* EnemySlot = MiddleRow->AddChildToHorizontalBox(EnemyBorder);
	EnemySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UHorizontalBox* BottomRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
	RootBox->AddChildToVerticalBox(BottomRow);

	UBorder* LogBorder = CreateSection(WidgetTree, TEXT("LogBorder"), FLinearColor(0.08f, 0.08f, 0.08f, 0.92f));
	LogScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("LogScrollBox"));
	LogBorder->SetContent(LogScrollBox);
	UHorizontalBoxSlot* LogSlot = BottomRow->AddChildToHorizontalBox(LogBorder);
	LogSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	LogSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));

	UBorder* HandBorder = CreateSection(WidgetTree, TEXT("HandBorder"), FLinearColor(0.08f, 0.11f, 0.16f, 0.92f));
	HandCardBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("HandCardBox"));
	HandBorder->SetContent(HandCardBox);
	UHorizontalBoxSlot* HandSlot = BottomRow->AddChildToHorizontalBox(HandBorder);
	HandSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	HandSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));

	UBorder* ActionBorder = CreateSection(WidgetTree, TEXT("ActionBorder"), FLinearColor(0.09f, 0.13f, 0.08f, 0.92f));
	UVerticalBox* ActionColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ActionColumn"));
	ActionBorder->SetContent(ActionColumn);

	EndTurnButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EndTurnButton"));
	EndTurnLabel = CreateLabel(WidgetTree, TEXT("EndTurnLabel"), 14);
	EndTurnLabel->SetText(NSLOCTEXT("FinalBattleHUD", "EndTurnLabel", "结束回合"));
	EndTurnButton->AddChild(EndTurnLabel);
	EndTurnButton->OnClicked.AddDynamic(this, &UFinalBattleHUDScreen::HandleEndTurnClicked);
	ActionColumn->AddChildToVerticalBox(EndTurnButton);

	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("ActionSpacer"));
	Spacer->SetSize(FVector2D(8.0f, 12.0f));
	ActionColumn->AddChildToVerticalBox(Spacer);

	UltimateButtonBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("UltimateButtonBox"));
	ActionColumn->AddChildToVerticalBox(UltimateButtonBox);

	UHorizontalBoxSlot* ActionSlot = BottomRow->AddChildToHorizontalBox(ActionBorder);
	ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
}

void UFinalBattleHUDScreen::RefreshFromViewModel()
{
	if (BattleViewModel == nullptr || HeaderText == nullptr)
	{
		return;
	}

	const FFinalBattleHUDPresentationData Presentation = BattleViewModel->GetPresentation();

	if (!Presentation.bHasActiveBattle)
	{
		HeaderText->SetText(NSLOCTEXT("FinalBattleHUD", "NoBattleHeader", "Battle HUD ready. No active battle session."));
		FeedbackText->SetText(NSLOCTEXT("FinalBattleHUD", "NoBattleFeedback", "通过控制台命令或地图按钮启动测试战斗后，这里会自动刷新。"));
		ContextText->SetText(FText::GetEmpty());
		GapText->SetText(FText::GetEmpty());
	}
	else
	{
		HeaderText->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "HeaderFormat", "{0} | Round {1} | AP {2} | EP {3}/{4} | Team HP {5}/{6} | Shield {7} | Gold {8} | Relics {9}"),
			Presentation.EncounterName,
			FText::AsNumber(Presentation.CurrentRound),
			FText::AsNumber(Presentation.CurrentAP),
			FText::AsNumber(Presentation.CurrentEP),
			FText::AsNumber(Presentation.MaxEP),
			FText::AsNumber(Presentation.TeamCurrentHP),
			FText::AsNumber(Presentation.TeamMaxHP),
			FText::AsNumber(Presentation.TeamShield),
			FText::AsNumber(Presentation.Gold),
			FText::AsNumber(Presentation.RelicCount)));
		FeedbackText->SetText(Presentation.FeedbackText);
		ContextText->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "ContextFormat", "{0}\nDeck: Draw {1} | Hand {2} | Discard {3} | Ongoing {4} | Consume {5} | RunDeck {6}\nTeam Status: {7}"),
			Presentation.CurrentTargetText,
			FText::AsNumber(Presentation.DrawPileCount),
			FText::AsNumber(Presentation.HandCount),
			FText::AsNumber(Presentation.DiscardPileCount),
			FText::AsNumber(Presentation.OngoingZoneCount),
			FText::AsNumber(Presentation.ConsumePileCount),
			FText::AsNumber(Presentation.RunDeckCount),
			JoinTextArray(Presentation.TeamStatusTexts, NSLOCTEXT("FinalBattleHUD", "NoTeamStatus", "无"))));

		if (Presentation.MissingFieldNotices.Num() > 0)
		{
			FString Joined;
			for (int32 Index = 0; Index < Presentation.MissingFieldNotices.Num(); ++Index)
			{
				if (Index > 0)
				{
					Joined += TEXT(" | ");
				}

				Joined += Presentation.MissingFieldNotices[Index].ToString();
			}

			GapText->SetText(FText::FromString(Joined));
		}
		else
		{
			GapText->SetText(FText::GetEmpty());
		}
	}

	RebuildCharacterPanel();
	RebuildEnemyPanel();
	RebuildHandPanel();
	RebuildUltimatePanel();
	RebuildLogPanel();
}

void UFinalBattleHUDScreen::RebuildCharacterPanel()
{
	if (CharacterListBox == nullptr || BattleViewModel == nullptr)
	{
		return;
	}

	CharacterListBox->ClearChildren();
	const FFinalBattleHUDPresentationData Presentation = BattleViewModel->GetPresentation();

	for (const FFinalBattleHUDCharacterEntry& Entry : Presentation.Characters)
	{
		UTextBlock* Label = CreateLabel(WidgetTree, *FString::Printf(TEXT("Character_%s"), *Entry.RuntimeUnitId.ToString()));
		Label->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "CharacterEntryFormat", "{0}\nStress {1}/{2} | Vital {3}\nAwaken {4}/{5} | Collapse {6}\n{7}\nStatus: {8}"),
			Entry.DisplayName,
			FText::AsNumber(Entry.CurrentStress),
			FText::AsNumber(Entry.StressCap),
			FText::AsNumber(Entry.VitalShare),
			FText::AsNumber(Entry.CurrentAwakenCount),
			FText::AsNumber(Entry.CurrentAwakenThreshold),
			FText::AsNumber(Entry.CollapseCount),
			Entry.StateText,
			JoinTextArray(Entry.StatusTexts, NSLOCTEXT("FinalBattleHUD", "NoCharacterStatus", "无"))));
		CharacterListBox->AddChildToVerticalBox(Label);
	}
}

void UFinalBattleHUDScreen::RebuildEnemyPanel()
{
	if (EnemyListBox == nullptr || BattleViewModel == nullptr)
	{
		return;
	}

	EnemyListBox->ClearChildren();
	const FFinalBattleHUDPresentationData Presentation = BattleViewModel->GetPresentation();

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

void UFinalBattleHUDScreen::RebuildHandPanel()
{
	if (HandCardBox == nullptr || BattleViewModel == nullptr)
	{
		return;
	}

	HandCardBox->ClearChildren();
	const FFinalBattleHUDPresentationData Presentation = BattleViewModel->GetPresentation();

	for (int32 Index = 0; Index < Presentation.HandCards.Num(); ++Index)
	{
		UFinalBattleCardEntryWidget* CardWidget = CreateWidget<UFinalBattleCardEntryWidget>(GetOwningPlayer(), UFinalBattleCardEntryWidget::StaticClass());
		if (CardWidget == nullptr)
		{
			continue;
		}

		CardWidget->Configure(this, Index, Presentation.HandCards[Index]);
		UHorizontalBoxSlot* CardSlot = HandCardBox->AddChildToHorizontalBox(CardWidget);
		CardSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}
}

void UFinalBattleHUDScreen::RebuildUltimatePanel()
{
	if (UltimateButtonBox == nullptr || BattleViewModel == nullptr)
	{
		return;
	}

	UltimateButtonBox->ClearChildren();
	const FFinalBattleHUDPresentationData Presentation = BattleViewModel->GetPresentation();

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

void UFinalBattleHUDScreen::RebuildLogPanel()
{
	if (LogScrollBox == nullptr || BattleViewModel == nullptr)
	{
		return;
	}

	LogScrollBox->ClearChildren();
	const FFinalBattleHUDPresentationData Presentation = BattleViewModel->GetPresentation();

	for (const FFinalBattleHUDLogEntry& Entry : Presentation.LogEntries)
	{
		UTextBlock* Label = CreateLabel(WidgetTree, *FString::Printf(TEXT("Log_%d"), Entry.Round));
		Label->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "LogEntryFormat", "R{0} | {1}"),
			FText::AsNumber(Entry.Round),
			Entry.Message));
		LogScrollBox->AddChild(Label);
	}
}
