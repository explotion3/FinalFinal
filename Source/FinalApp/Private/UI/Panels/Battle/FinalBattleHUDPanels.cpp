#include "UI/Panels/Battle/FinalBattleHUDPanels.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Styling/CoreStyle.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "Queries/FinalRunSnapshot.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
#include "UI/ViewModels/Battle/FinalBattleHUDPanelViewModels.h"
#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleCardZoneEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterDetailWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyDetailWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleLogEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleTeamCharacterEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleTeamStatusDetailLineWidget.h"
#include "UI/Widgets/Battle/FinalBattleTeamStatusIconWidget.h"
#include "UI/Widgets/Battle/FinalBattleUltimateEntryWidget.h"
#include "World/FinalBattlePresentationActor.h"
#include "World/FinalBattleTargetInteractorComponent.h"

namespace
{
UBorder* CreateSection(UWidgetTree* WidgetTree, const TCHAR* Name, const FLinearColor& Color)
{
	UBorder* Border = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
	Border->SetBrushColor(Color);
	Border->SetPadding(FMargin(8.0f));
	return Border;
}

UTextBlock* CreateLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize = 14)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

UButton* CreateTextButton(UWidgetTree* WidgetTree, const TCHAR* ButtonName, const TCHAR* TextName, const FText& Label, TObjectPtr<UTextBlock>& OutText)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), ButtonName);
	OutText = CreateLabel(WidgetTree, TextName, 11);
	if (OutText)
	{
		OutText->SetJustification(ETextJustify::Center);
		OutText->SetText(Label);
		Button->AddChild(OutText);
	}
	return Button;
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

FText BuildCompactTextArraySummary(const TArray<FText>& Texts, const FText& EmptyText, const int32 MaxEntries = 2)
{
	if (Texts.Num() == 0)
	{
		return EmptyText;
	}

	TArray<FString> Segments;
	const int32 EntryCount = FMath::Min(Texts.Num(), MaxEntries);
	for (int32 Index = 0; Index < EntryCount; ++Index)
	{
		Segments.Add(Texts[Index].ToString());
	}

	if (Texts.Num() > MaxEntries)
	{
		Segments.Add(FString::Printf(TEXT("+%d"), Texts.Num() - MaxEntries));
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
}

template <typename TWidget>
TWidget* CreateConfiguredEntryWidget(UUserWidget* OwnerWidget, TSubclassOf<TWidget> WidgetClass)
{
	return OwnerWidget ? CreateWidget<TWidget>(OwnerWidget->GetOwningPlayer(), WidgetClass) : nullptr;
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
	TopBarText = CreateLabel(WidgetTree, TEXT("TopBarText"), 15);
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
			NSLOCTEXT("FinalBattleHUD", "TopBarFormat", "{0} | Round {1} | AP {2} | EP {3}/{4} | Team HP {5}/{6} | Shield {7}\nDraw {8} | Hand {9} | Discard {10} | Consume {11}"),
			Data.EncounterName,
			FText::AsNumber(Data.CurrentRound),
			FText::AsNumber(Data.CurrentAP),
			FText::AsNumber(Data.CurrentEP),
			FText::AsNumber(Data.MaxEP),
			FText::AsNumber(Data.TeamCurrentHP),
			FText::AsNumber(Data.TeamMaxHP),
			FText::AsNumber(Data.TeamShield),
			FText::AsNumber(Data.DrawPileCount),
			FText::AsNumber(Data.HandCount),
			FText::AsNumber(Data.DiscardPileCount),
			FText::AsNumber(Data.ConsumePileCount)));
		return;
	}

	TopBarText->SetText(NSLOCTEXT("FinalBattleHUD", "NoBattleHeader", "Battle HUD ready. No active battle session."));
}

void UFinalBattleResourcePanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleResourcePanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleResourcePanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleResourcePanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleResourcePanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleResourcePanel::InitializePanel(UFinalBattleResourcePanelViewModel* InViewModel, UFinalBattleResourcePanelController* InController)
{
	PanelViewModel = InViewModel;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleResourcePanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleResourcePanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("ResourceBorder"), FLinearColor(0.0f, 0.0f, 0.0f, 0.78f));
	UHorizontalBox* ResourceRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ResourceRow"));
	Border->SetContent(ResourceRow);

	APLabelText = CreateLabel(WidgetTree, TEXT("APLabelText"), 16);
	APLabelText->SetText(NSLOCTEXT("FinalBattleHUD", "APLabel", "AP"));
	ResourceRow->AddChildToHorizontalBox(APLabelText);

	APText = CreateLabel(WidgetTree, TEXT("APText"), 26);
	APText->SetJustification(ETextJustify::Center);
	if (UHorizontalBoxSlot* APSlot = ResourceRow->AddChildToHorizontalBox(APText))
	{
		APSlot->SetPadding(FMargin(8.0f, 0.0f, 20.0f, 0.0f));
	}

	EPLabelText = CreateLabel(WidgetTree, TEXT("EPLabelText"), 16);
	EPLabelText->SetText(NSLOCTEXT("FinalBattleHUD", "EPLabel", "EP"));
	ResourceRow->AddChildToHorizontalBox(EPLabelText);

	EPText = CreateLabel(WidgetTree, TEXT("EPText"), 26);
	EPText->SetJustification(ETextJustify::Center);
	if (UHorizontalBoxSlot* EPSlot = ResourceRow->AddChildToHorizontalBox(EPText))
	{
		EPSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
	}

	WidgetTree->RootWidget = Border;
}

void UFinalBattleResourcePanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr)
	{
		return;
	}

	const FFinalBattleResourcePanelData& Data = PanelViewModel->GetData();
	const FText APValueText = FText::AsNumber(Data.CurrentAP);
	const FText EPValueText = FText::AsNumber(Data.CurrentEP);
	const FSlateColor EPColor = Data.bEPFull ? FullEPColor : NormalEPColor;
	UImage* QiPipBases[] =
	{
		QIPipBase0,
		QIPipBase1,
		QIPipBase2,
		QIPipBase3,
		QIPipBase4,
		QIPipBase5,
		QIPipBase6
	};

	if (APLabelText)
	{
		APLabelText->SetText(NSLOCTEXT("FinalBattleHUD", "APLabel", "AP"));
	}

	if (APText)
	{
		APText->SetText(APValueText);
	}

	if (EPLabelText)
	{
		EPLabelText->SetText(NSLOCTEXT("FinalBattleHUD", "EPLabel", "EP"));
		EPLabelText->SetColorAndOpacity(EPColor);
	}

	if (EPText)
	{
		EPText->SetText(EPValueText);
		EPText->SetColorAndOpacity(EPColor);
	}

	if (QiLabelText)
	{
		QiLabelText->SetColorAndOpacity(EPColor);
	}

	const int32 ActivePipCount = FMath::Clamp(Data.ActiveQiPipCount, 0, UE_ARRAY_COUNT(QiPipBases));
	for (int32 PipIndex = 0; PipIndex < UE_ARRAY_COUNT(QiPipBases); ++PipIndex)
	{
		if (QiPipBases[PipIndex])
		{
			QiPipBases[PipIndex]->SetColorAndOpacity(PipIndex < ActivePipCount ? ActiveQiPipBaseColor : InactiveQiPipBaseColor);
		}
	}

	if (ResourceText)
	{
		ResourceText->SetText(Data.bHasActiveBattle
			? FText::Format(NSLOCTEXT("FinalBattleHUD", "ResourceTextFormat", "AP {0} | EP {1}"), APValueText, EPValueText)
			: NSLOCTEXT("FinalBattleHUD", "ResourceTextNoBattle", "AP 0 | EP 0"));
		ResourceText->SetColorAndOpacity(EPColor);
	}
}

void UFinalRunFlowPromptPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalRunFlowPromptPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		RunFlowSubsystem->OnRunFlowStateChanged.AddUniqueDynamic(this, &UFinalRunFlowPromptPanel::HandleRunFlowStateChanged);
	}

	RefreshPrompt();
}

void UFinalRunFlowPromptPanel::NativeDestruct()
{
	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		RunFlowSubsystem->OnRunFlowStateChanged.RemoveDynamic(this, &UFinalRunFlowPromptPanel::HandleRunFlowStateChanged);
	}

	Super::NativeDestruct();
}

void UFinalRunFlowPromptPanel::RefreshPrompt()
{
	const bool bShouldShow = ShouldShowPrompt();
	SetVisibility(bShouldShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (OpenFlowButton)
	{
		OpenFlowButton->SetIsEnabled(bShouldShow);
	}

	if (OpenFlowLabel)
	{
		OpenFlowLabel->SetText(BuildPromptText());
	}
}

void UFinalRunFlowPromptPanel::HandleOpenFlowClicked()
{
	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		RunFlowSubsystem->RefreshRunFlow(true);
	}

	RefreshPrompt();
}

void UFinalRunFlowPromptPanel::HandleRunFlowStateChanged()
{
	RefreshPrompt();
}

void UFinalRunFlowPromptPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("RunFlowPromptBorder"), FLinearColor(0.04f, 0.04f, 0.035f, 0.88f));
	OpenFlowButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OpenFlowButton"));
	OpenFlowButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowPromptPanel::HandleOpenFlowClicked);
	OpenFlowLabel = CreateLabel(WidgetTree, TEXT("OpenFlowLabel"), 13);
	OpenFlowLabel->SetText(NSLOCTEXT("FinalBattleHUD", "RunFlowPromptDefault", "打开流程"));
	OpenFlowButton->AddChild(OpenFlowLabel);
	Border->SetContent(OpenFlowButton);

	WidgetTree->RootWidget = Border;
}

UFinalRunFlowSubsystem* UFinalRunFlowPromptPanel::ResolveRunFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr;
}

bool UFinalRunFlowPromptPanel::ShouldShowPrompt() const
{
	const UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		return false;
	}

	const FFinalRunSnapshot Snapshot = RunFlowSubsystem->GetCurrentRunSnapshot();
	if (Snapshot.PendingGrowthChoice.bHasPendingChoice)
	{
		return true;
	}

	if (Snapshot.PendingBattleReward.bHasPendingReward
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		return true;
	}

	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
	case EFinalRunFlowStage::PendingRewardNode:
	case EFinalRunFlowStage::PendingEventNode:
	case EFinalRunFlowStage::PendingShopNode:
	case EFinalRunFlowStage::RunEnded:
		return true;

	case EFinalRunFlowStage::PreparingBattle:
	case EFinalRunFlowStage::None:
	default:
		return false;
	}
}

FText UFinalRunFlowPromptPanel::BuildPromptText() const
{
	const UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptUnavailable", "流程不可用");
	}

	const FFinalRunSnapshot Snapshot = RunFlowSubsystem->GetCurrentRunSnapshot();
	if (Snapshot.PendingGrowthChoice.bHasPendingChoice)
	{
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptGrowthChoice", "选择成长");
	}

	if (Snapshot.PendingBattleReward.bHasPendingReward
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptBattleReward", "选择战利品");
	}

	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptAdvanceNode", "继续旅程");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptRewardNode", "领取节点奖励");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptEventNode", "处理事件");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptShopNode", "进入商店");

	case EFinalRunFlowStage::RunEnded:
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptRunEnded", "查看结算");

	case EFinalRunFlowStage::PreparingBattle:
	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalBattleHUD", "RunFlowPromptNoAction", "暂无流程操作");
	}
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
	FeedbackText = CreateLabel(WidgetTree, TEXT("FeedbackText"), 12);
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
	PanelController = InController;
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
	ContextText = CreateLabel(WidgetTree, TEXT("ContextText"), 11);
	ContextBorder->SetContent(ContextText);
	RootBox->AddChildToVerticalBox(ContextBorder);

	UHorizontalBox* ZoneButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CardZoneButtonRow"));
	if (UVerticalBoxSlot* ZoneRowSlot = RootBox->AddChildToVerticalBox(ZoneButtonRow))
	{
		ZoneRowSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	DrawPileButton = CreateTextButton(WidgetTree, TEXT("DrawPileButton"), TEXT("DrawPileButtonText"), NSLOCTEXT("FinalBattleHUD", "DrawPileButtonLabel", "抽牌"), DrawPileButtonText);
	HandButton = CreateTextButton(WidgetTree, TEXT("HandButton"), TEXT("HandButtonText"), NSLOCTEXT("FinalBattleHUD", "HandButtonLabel", "手牌"), HandButtonText);
	DiscardPileButton = CreateTextButton(WidgetTree, TEXT("DiscardPileButton"), TEXT("DiscardPileButtonText"), NSLOCTEXT("FinalBattleHUD", "DiscardPileButtonLabel", "弃牌"), DiscardPileButtonText);
	OngoingZoneButton = CreateTextButton(WidgetTree, TEXT("OngoingZoneButton"), TEXT("OngoingZoneButtonText"), NSLOCTEXT("FinalBattleHUD", "OngoingZoneButtonLabel", "持续"), OngoingZoneButtonText);
	ConsumePileButton = CreateTextButton(WidgetTree, TEXT("ConsumePileButton"), TEXT("ConsumePileButtonText"), NSLOCTEXT("FinalBattleHUD", "ConsumePileButtonLabel", "消耗"), ConsumePileButtonText);

	if (DrawPileButton)
	{
		DrawPileButton->OnClicked.AddDynamic(this, &UFinalBattleContextPanel::HandleDrawPileClicked);
	}
	if (HandButton)
	{
		HandButton->OnClicked.AddDynamic(this, &UFinalBattleContextPanel::HandleHandClicked);
	}
	if (DiscardPileButton)
	{
		DiscardPileButton->OnClicked.AddDynamic(this, &UFinalBattleContextPanel::HandleDiscardPileClicked);
	}
	if (OngoingZoneButton)
	{
		OngoingZoneButton->OnClicked.AddDynamic(this, &UFinalBattleContextPanel::HandleOngoingZoneClicked);
	}
	if (ConsumePileButton)
	{
		ConsumePileButton->OnClicked.AddDynamic(this, &UFinalBattleContextPanel::HandleConsumePileClicked);
	}

	UButton* ZoneButtons[] = { DrawPileButton, HandButton, DiscardPileButton, OngoingZoneButton, ConsumePileButton };
	for (UButton* ZoneButton : ZoneButtons)
	{
		if (ZoneButton == nullptr)
		{
			continue;
		}
		if (UHorizontalBoxSlot* ButtonSlot = ZoneButtonRow->AddChildToHorizontalBox(ZoneButton))
		{
			ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
			ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
	}

	GapBorder = CreateSection(WidgetTree, TEXT("GapBorder"), FLinearColor(0.14f, 0.08f, 0.08f, 0.92f));
	GapText = CreateLabel(WidgetTree, TEXT("GapText"), 11);
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
		UButton* ZoneButtons[] = { DrawPileButton, HandButton, DiscardPileButton, OngoingZoneButton, ConsumePileButton };
		for (UButton* ZoneButton : ZoneButtons)
		{
			if (ZoneButton)
			{
				ZoneButton->SetIsEnabled(false);
			}
		}
		return;
	}

	if (DrawPileButton) { DrawPileButton->SetIsEnabled(true); }
	if (HandButton) { HandButton->SetIsEnabled(true); }
	if (DiscardPileButton) { DiscardPileButton->SetIsEnabled(true); }
	if (OngoingZoneButton) { OngoingZoneButton->SetIsEnabled(true); }
	if (ConsumePileButton) { ConsumePileButton->SetIsEnabled(true); }

	if (DrawPileButtonText)
	{
		DrawPileButtonText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "DrawPileButtonCount", "抽 {0}"), FText::AsNumber(Data.DrawPileCount)));
	}
	if (HandButtonText)
	{
		HandButtonText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "HandButtonCount", "手 {0}"), FText::AsNumber(Data.HandCount)));
	}
	if (DiscardPileButtonText)
	{
		DiscardPileButtonText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "DiscardPileButtonCount", "弃 {0}"), FText::AsNumber(Data.DiscardPileCount)));
	}
	if (OngoingZoneButtonText)
	{
		OngoingZoneButtonText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "OngoingZoneButtonCount", "持 {0}"), FText::AsNumber(Data.OngoingZoneCount)));
	}
	if (ConsumePileButtonText)
	{
		ConsumePileButtonText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "ConsumePileButtonCount", "耗 {0}"), FText::AsNumber(Data.ConsumePileCount)));
	}

	ContextText->SetText(FText::Format(
		NSLOCTEXT("FinalBattleHUD", "ContextFormat", "{0}\n牌堆 D{1} H{2} X{3} O{4} C{5}\nRun G{6} R{7} Deck {8}\n队伍状态: {9}\n激活遗物: {10}"),
		Data.CurrentTargetText,
		FText::AsNumber(Data.DrawPileCount),
		FText::AsNumber(Data.HandCount),
		FText::AsNumber(Data.DiscardPileCount),
		FText::AsNumber(Data.OngoingZoneCount),
		FText::AsNumber(Data.ConsumePileCount),
		FText::AsNumber(Data.Gold),
		FText::AsNumber(Data.RelicCount),
		FText::AsNumber(Data.RunDeckCount),
		BuildCompactTextArraySummary(Data.TeamStatusTexts, NSLOCTEXT("FinalBattleHUD", "NoTeamStatus", "无")),
		BuildCompactTextArraySummary(Data.ActiveRelicTexts, NSLOCTEXT("FinalBattleHUD", "NoActiveRelics", "无"))));

	if (Data.MissingFieldNotices.Num() > 0)
	{
		GapText->SetText(BuildCompactTextArraySummary(Data.MissingFieldNotices, FText::GetEmpty()));
		GapBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GapText->SetText(FText::GetEmpty());
		GapBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalBattleContextPanel::HandleDrawPileClicked()
{
	InspectCardZone(EFinalBattleCardZone::DrawPile);
}

void UFinalBattleContextPanel::HandleHandClicked()
{
	InspectCardZone(EFinalBattleCardZone::Hand);
}

void UFinalBattleContextPanel::HandleDiscardPileClicked()
{
	InspectCardZone(EFinalBattleCardZone::DiscardPile);
}

void UFinalBattleContextPanel::HandleOngoingZoneClicked()
{
	InspectCardZone(EFinalBattleCardZone::OngoingZone);
}

void UFinalBattleContextPanel::HandleConsumePileClicked()
{
	InspectCardZone(EFinalBattleCardZone::ConsumePile);
}

void UFinalBattleContextPanel::InspectCardZone(const EFinalBattleCardZone Zone)
{
	if (PanelController)
	{
		PanelController->InspectCardZone(Zone);
	}
}

void UFinalBattleTeamPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	if (StatusDetailButton)
	{
		StatusDetailButton->OnClicked.AddUniqueDynamic(this, &UFinalBattleTeamPanel::HandleStatusDetailClicked);
	}
}

void UFinalBattleTeamPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleTeamPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleTeamPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleTeamPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleTeamPanel::InitializePanel(UFinalBattleTeamPanelViewModel* InViewModel, UFinalBattleTeamPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleTeamPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleTeamPanel::HandleStatusDetailClicked()
{
	if (PanelController)
	{
		PanelController->OpenTeamStatusDetail();
	}
}

void UFinalBattleTeamPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("TeamPanelBorder"), FLinearColor(0.08f, 0.12f, 0.16f, 0.94f));
	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TeamPanelRoot"));
	HealthText = CreateLabel(WidgetTree, TEXT("HealthText"), 14);
	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.78f, 0.12f, 0.10f, 1.0f));
	ShieldText = CreateLabel(WidgetTree, TEXT("ShieldText"), 12);
	ShieldFrameBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ShieldFrameBar"));
	ShieldFrameBar->SetFillColorAndOpacity(FLinearColor(0.18f, 0.55f, 0.95f, 0.85f));
	CharacterBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CharacterBox"));
	StatusDetailButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StatusDetailButton"));
	UHorizontalBox* StatusRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StatusPreviewBox"));
	StatusPreviewBox = StatusRow;
	StatusOverflowText = CreateLabel(WidgetTree, TEXT("StatusOverflowText"), 11);
	StatusDetailButton->AddChild(StatusRow);

	RootBox->AddChildToVerticalBox(HealthText);
	RootBox->AddChildToVerticalBox(HealthBar);
	RootBox->AddChildToVerticalBox(ShieldText);
	RootBox->AddChildToVerticalBox(ShieldFrameBar);
	RootBox->AddChildToVerticalBox(CharacterBox);
	RootBox->AddChildToVerticalBox(StatusDetailButton);
	RootBox->AddChildToVerticalBox(StatusOverflowText);
	Border->SetContent(RootBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleTeamPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr)
	{
		return;
	}

	EnsureWidgetTree();
	const FFinalBattleHUDTeamPanelData& Data = PanelViewModel->GetData();
	if (HealthText)
	{
		HealthText->SetText(BuildHealthText(Data));
	}
	if (HealthBar)
	{
		HealthBar->SetPercent(Data.TeamHealthPercent);
	}
	if (ShieldText)
	{
		ShieldText->SetText(BuildShieldText(Data));
	}
	if (ShieldFrameBar)
	{
		ShieldFrameBar->SetPercent(Data.TeamShieldFramePercent);
		ShieldFrameBar->SetVisibility(Data.TeamShield > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	RefreshCharacters(Data);
	RefreshStatusPreview(Data);
}

void UFinalBattleTeamPanel::RefreshCharacters(const FFinalBattleHUDTeamPanelData& Data)
{
	if (CharacterBox == nullptr)
	{
		return;
	}

	CharacterBox->ClearChildren();
	TSubclassOf<UFinalBattleTeamCharacterEntryWidget> EntryClass = TeamCharacterEntryWidgetClass
		? TeamCharacterEntryWidgetClass
		: UFinalUIWidgetClassSettings::GetBattleTeamCharacterEntryWidgetClass();
	for (const FFinalBattleHUDTeamCharacterEntry& Entry : Data.Characters)
	{
		UFinalBattleTeamCharacterEntryWidget* EntryWidget = CreateConfiguredEntryWidget(this, EntryClass);
		if (EntryWidget == nullptr)
		{
			continue;
		}
		EntryWidget->SetPresentationContext(PanelController, PanelViewModel);
		EntryWidget->ApplyTeamCharacterEntryView(Entry);
		CharacterBox->AddChild(EntryWidget);
	}
}

void UFinalBattleTeamPanel::RefreshStatusPreview(const FFinalBattleHUDTeamPanelData& Data)
{
	if (StatusPreviewBox)
	{
		StatusPreviewBox->ClearChildren();
		TSubclassOf<UFinalBattleTeamStatusIconWidget> IconClass = TeamStatusIconWidgetClass
			? TeamStatusIconWidgetClass
			: UFinalUIWidgetClassSettings::GetBattleTeamStatusIconWidgetClass();
		for (const FFinalBattleHUDTeamStatusEntry& Entry : Data.StatusPreviewEntries)
		{
			UFinalBattleTeamStatusIconWidget* StatusWidget = CreateConfiguredEntryWidget(this, IconClass);
			if (StatusWidget == nullptr)
			{
				continue;
			}
			StatusWidget->SetPresentationContext(PanelController, PanelViewModel);
			StatusWidget->ApplyTeamStatusIconView(Entry);
			StatusPreviewBox->AddChild(StatusWidget);
		}
	}

	if (StatusOverflowText)
	{
		StatusOverflowText->SetVisibility(Data.HiddenStatusCount > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		StatusOverflowText->SetText(FText::FromString(FString::Printf(TEXT("+%d"), Data.HiddenStatusCount)));
	}
}

FText UFinalBattleTeamPanel::BuildHealthText(const FFinalBattleHUDTeamPanelData& Data) const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleTeamPanel", "HealthText", "共享生命 {0}/{1}"),
		FText::AsNumber(Data.TeamCurrentHP),
		FText::AsNumber(Data.TeamMaxHP));
}

FText UFinalBattleTeamPanel::BuildShieldText(const FFinalBattleHUDTeamPanelData& Data) const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleTeamPanel", "ShieldText", "护盾 {0}"),
		FText::AsNumber(Data.TeamShield));
}

void UFinalBattleTeamStatusDetailPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UFinalBattleTeamStatusDetailPanel::HandleCloseClicked);
	}
}

void UFinalBattleTeamStatusDetailPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleTeamStatusDetailPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleTeamStatusDetailPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleTeamStatusDetailPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleTeamStatusDetailPanel::InitializePanel(
	UFinalBattleTeamStatusDetailPanelViewModel* InViewModel,
	UFinalBattleTeamStatusDetailPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleTeamStatusDetailPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleTeamStatusDetailPanel::HandleCloseClicked()
{
	if (PanelController)
	{
		PanelController->ClearTeamStatusDetail();
	}
}

void UFinalBattleTeamStatusDetailPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("TeamStatusDetailBorder"), FLinearColor(0.06f, 0.08f, 0.12f, 0.96f));
	ContentRoot = Border;
	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TeamStatusDetailRoot"));
	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	UTextBlock* CloseText = CreateLabel(WidgetTree, TEXT("CloseButtonText"), 12);
	CloseText->SetText(NSLOCTEXT("FinalBattleTeamStatusDetail", "Close", "关闭"));
	CloseButton->AddChild(CloseText);
	EmptyText = CreateLabel(WidgetTree, TEXT("EmptyText"), 12);
	StatusListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusListBox"));
	RootBox->AddChildToVerticalBox(CloseButton);
	RootBox->AddChildToVerticalBox(EmptyText);
	RootBox->AddChildToVerticalBox(StatusListBox);
	Border->SetContent(RootBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleTeamStatusDetailPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr)
	{
		return;
	}

	EnsureWidgetTree();
	const FFinalBattleHUDTeamStatusDetailData& Data = PanelViewModel->GetData();
	SetVisibility(Data.bOpen ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (ContentRoot)
	{
		ContentRoot->SetVisibility(Data.bOpen ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (EmptyText)
	{
		EmptyText->SetVisibility(Data.Statuses.Num() == 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		EmptyText->SetText(NSLOCTEXT("FinalBattleTeamStatusDetail", "NoStatuses", "无状态"));
	}

	if (StatusListBox == nullptr)
	{
		return;
	}

	StatusListBox->ClearChildren();
	TSubclassOf<UFinalBattleTeamStatusDetailLineWidget> LineClass = StatusLineWidgetClass
		? StatusLineWidgetClass
		: UFinalUIWidgetClassSettings::GetBattleTeamStatusDetailLineWidgetClass();
	for (const FFinalBattleHUDTeamStatusEntry& Entry : Data.Statuses)
	{
		UFinalBattleTeamStatusDetailLineWidget* LineWidget = CreateConfiguredEntryWidget(this, LineClass);
		if (LineWidget == nullptr)
		{
			continue;
		}
		LineWidget->SetPresentationContext(PanelController, PanelViewModel);
		LineWidget->ApplyTeamStatusDetailLineView(Entry);
		StatusListBox->AddChild(LineWidget);
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
	PanelController = InController;
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
	UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("CharacterScrollBox"));
	CharacterListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterListBox"));
	ScrollBox->AddChild(CharacterListBox);
	Border->SetContent(ScrollBox);
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
		UFinalBattleCharacterEntryWidget* CharacterWidget = CreateConfiguredEntryWidget(this, UFinalUIWidgetClassSettings::GetBattleCharacterEntryWidgetClass());
		if (CharacterWidget == nullptr)
		{
			continue;
		}

		CharacterWidget->SetPresentationContext(PanelController, PanelViewModel);
		CharacterWidget->Configure(Entry);
		if (UVerticalBoxSlot* CharacterSlot = CharacterListBox->AddChildToVerticalBox(CharacterWidget))
		{
			CharacterSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
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
	UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("EnemyScrollBox"));
	EnemyListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EnemyListBox"));
	ScrollBox->AddChild(EnemyListBox);
	Border->SetContent(ScrollBox);
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
		UFinalBattleEnemyEntryWidget* EnemyWidget = CreateConfiguredEntryWidget(this, UFinalUIWidgetClassSettings::GetBattleEnemyEntryWidgetClass());
		if (EnemyWidget == nullptr)
		{
			continue;
		}

		EnemyWidget->Configure(PanelController, Entry);
		if (UVerticalBoxSlot* EnemySlot = EnemyListBox->AddChildToVerticalBox(EnemyWidget))
		{
			EnemySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
	}
}

void UFinalBattleEnemyDetailPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleEnemyDetailPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleEnemyDetailPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleEnemyDetailPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleEnemyDetailPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleEnemyDetailPanel::InitializePanel(UFinalBattleEnemyDetailPanelViewModel* InViewModel, UFinalBattleEnemyDetailPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	if (EnemyDetailWidget)
	{
		EnemyDetailWidget->SetPresentationContext(InController, InViewModel);
	}
	RefreshFromViewModel();
}

void UFinalBattleEnemyDetailPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleEnemyDetailPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	const TSubclassOf<UFinalBattleEnemyDetailWidget> DetailWidgetClass = UFinalUIWidgetClassSettings::GetBattleEnemyDetailWidgetClass();
	if (DetailWidgetClass && DetailWidgetClass != UFinalBattleEnemyDetailWidget::StaticClass())
	{
		EnemyDetailWidget = WidgetTree->ConstructWidget<UFinalBattleEnemyDetailWidget>(DetailWidgetClass, TEXT("EnemyDetailWidget"));
		WidgetTree->RootWidget = EnemyDetailWidget;
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("EnemyDetailBorder"), FLinearColor(0.09f, 0.08f, 0.08f, 0.94f));
	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EnemyDetailFallbackContent"));
	Border->SetContent(ContentBox);

	UHorizontalBox* HeaderBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("EnemyDetailFallbackHeader"));
	if (UVerticalBoxSlot* HeaderSlot = ContentBox->AddChildToVerticalBox(HeaderBox))
	{
		HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	UTextBlock* HeaderText = CreateLabel(WidgetTree, TEXT("EnemyDetailFallbackHeaderText"), 16);
	HeaderText->SetText(NSLOCTEXT("FinalBattleHUD", "EnemyDetailFallbackTitle", "敌人详情"));
	if (UHorizontalBoxSlot* HeaderTextSlot = HeaderBox->AddChildToHorizontalBox(HeaderText))
	{
		HeaderTextSlot->SetHorizontalAlignment(HAlign_Left);
		HeaderTextSlot->SetVerticalAlignment(VAlign_Center);
		HeaderTextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	DetailFallbackCloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EnemyDetailFallbackCloseButton"));
	UTextBlock* CloseLabel = CreateLabel(WidgetTree, TEXT("EnemyDetailFallbackCloseLabel"), 14);
	CloseLabel->SetText(NSLOCTEXT("FinalBattleHUD", "EnemyDetailFallbackClose", "关闭"));
	DetailFallbackCloseButton->AddChild(CloseLabel);
	DetailFallbackCloseButton->OnClicked.AddDynamic(this, &UFinalBattleEnemyDetailPanel::HandleFallbackCloseClicked);
	if (UHorizontalBoxSlot* CloseSlot = HeaderBox->AddChildToHorizontalBox(DetailFallbackCloseButton))
	{
		CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		CloseSlot->SetHorizontalAlignment(HAlign_Right);
		CloseSlot->SetVerticalAlignment(VAlign_Center);
	}

	DetailFallbackText = CreateLabel(WidgetTree, TEXT("EnemyDetailFallbackText"), 14);
	DetailFallbackText->SetAutoWrapText(true);
	if (UVerticalBoxSlot* TextSlot = ContentBox->AddChildToVerticalBox(DetailFallbackText))
	{
		TextSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	WidgetTree->RootWidget = Border;
}

void UFinalBattleEnemyDetailPanel::HandleFallbackCloseClicked()
{
	if (PanelController)
	{
		PanelController->ClearInspectedEnemy();
	}
}

void UFinalBattleEnemyDetailPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FFinalBattleHUDEnemyDetailData& Data = PanelViewModel->GetData();
	SetVisibility(Data.bHasEnemy ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (EnemyDetailWidget)
	{
		EnemyDetailWidget->SetPresentationContext(PanelController, PanelViewModel);
		EnemyDetailWidget->ApplyEnemyDetailView(Data);
	}

	if (DetailFallbackText)
	{
		DetailFallbackText->SetText(BuildFallbackText(Data));
	}
}

FText UFinalBattleEnemyDetailPanel::BuildFallbackText(const FFinalBattleHUDEnemyDetailData& Data) const
{
	if (!Data.bHasEnemy)
	{
		return FText::GetEmpty();
	}

	TArray<FString> StatusSegments;
	for (const FFinalBattleHUDEnemyDetailStatusEntry& Status : Data.Statuses)
	{
		StatusSegments.Add(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "EnemyDetailFallbackStatus", "{0} x{1}"),
			Status.DisplayName,
			FText::AsNumber(Status.CurrentStacks)).ToString());
	}

	const FString StatusText = StatusSegments.Num() > 0 ? FString::Join(StatusSegments, TEXT(" | ")) : TEXT("无");
	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "EnemyDetailFallbackFormat", "{0}{1}\nHP {2}/{3} | Shield {4}\nBreak {5}/{6} | Init {7}\n意图: {8}\n阶段: {9}\n状态: {10}"),
		Data.bIsCurrentBattleTarget ? NSLOCTEXT("FinalBattleHUD", "EnemyDetailTargetPrefix", "[当前目标] ") : FText::GetEmpty(),
		Data.DisplayName,
		FText::AsNumber(Data.CurrentHP),
		FText::AsNumber(Data.MaxHP),
		FText::AsNumber(Data.CurrentShield),
		FText::AsNumber(Data.CurrentBreakValue),
		FText::AsNumber(Data.MaxBreakValue),
		FText::AsNumber(Data.CurrentInitiative),
		Data.IntentText,
		Data.PhaseProgressText,
		FText::FromString(StatusText));
}

void UFinalBattleCharacterDetailPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleCharacterDetailPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleCharacterDetailPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleCharacterDetailPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleCharacterDetailPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleCharacterDetailPanel::InitializePanel(UFinalBattleCharacterDetailPanelViewModel* InViewModel, UFinalBattleCharacterDetailPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	if (CharacterDetailWidget)
	{
		CharacterDetailWidget->SetPresentationContext(InController, InViewModel);
	}
	RefreshFromViewModel();
}

void UFinalBattleCharacterDetailPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleCharacterDetailPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	const TSubclassOf<UFinalBattleCharacterDetailWidget> DetailWidgetClass = UFinalUIWidgetClassSettings::GetBattleCharacterDetailWidgetClass();
	if (DetailWidgetClass && DetailWidgetClass != UFinalBattleCharacterDetailWidget::StaticClass())
	{
		CharacterDetailWidget = WidgetTree->ConstructWidget<UFinalBattleCharacterDetailWidget>(DetailWidgetClass, TEXT("CharacterDetailWidget"));
		WidgetTree->RootWidget = CharacterDetailWidget;
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("CharacterDetailBorder"), FLinearColor(0.08f, 0.09f, 0.12f, 0.94f));
	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterDetailFallbackContent"));
	Border->SetContent(ContentBox);

	UHorizontalBox* HeaderBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CharacterDetailFallbackHeader"));
	ContentBox->AddChildToVerticalBox(HeaderBox);

	UTextBlock* HeaderText = CreateLabel(WidgetTree, TEXT("CharacterDetailFallbackHeaderText"), 16);
	HeaderText->SetText(NSLOCTEXT("FinalBattleHUD", "CharacterDetailFallbackTitle", "角色详情"));
	if (UHorizontalBoxSlot* HeaderTextSlot = HeaderBox->AddChildToHorizontalBox(HeaderText))
	{
		HeaderTextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	DetailFallbackCloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CharacterDetailFallbackCloseButton"));
	UTextBlock* CloseLabel = CreateLabel(WidgetTree, TEXT("CharacterDetailFallbackCloseLabel"), 14);
	CloseLabel->SetText(NSLOCTEXT("FinalBattleHUD", "CharacterDetailFallbackClose", "关闭"));
	DetailFallbackCloseButton->AddChild(CloseLabel);
	DetailFallbackCloseButton->OnClicked.AddDynamic(this, &UFinalBattleCharacterDetailPanel::HandleFallbackCloseClicked);
	HeaderBox->AddChildToHorizontalBox(DetailFallbackCloseButton);

	DetailFallbackText = CreateLabel(WidgetTree, TEXT("CharacterDetailFallbackText"), 14);
	DetailFallbackText->SetAutoWrapText(true);
	if (UVerticalBoxSlot* TextSlot = ContentBox->AddChildToVerticalBox(DetailFallbackText))
	{
		TextSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	WidgetTree->RootWidget = Border;
}

void UFinalBattleCharacterDetailPanel::HandleFallbackCloseClicked()
{
	if (PanelController)
	{
		PanelController->ClearInspectedCharacter();
	}
}

void UFinalBattleCharacterDetailPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FFinalBattleHUDCharacterDetailData& Data = PanelViewModel->GetData();
	SetVisibility(Data.bHasCharacter ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (CharacterDetailWidget)
	{
		CharacterDetailWidget->SetPresentationContext(PanelController, PanelViewModel);
		CharacterDetailWidget->ApplyCharacterDetailView(Data);
	}

	if (DetailFallbackText)
	{
		DetailFallbackText->SetText(BuildFallbackText(Data));
	}
}

FText UFinalBattleCharacterDetailPanel::BuildFallbackText(const FFinalBattleHUDCharacterDetailData& Data) const
{
	if (!Data.bHasCharacter)
	{
		return FText::GetEmpty();
	}

	TArray<FString> StatusSegments;
	for (const FFinalBattleHUDCharacterDetailStatusEntry& Status : Data.Statuses)
	{
		StatusSegments.Add(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "CharacterDetailFallbackStatus", "{0} x{1}"),
			Status.DisplayName,
			FText::AsNumber(Status.CurrentStacks)).ToString());
	}

	TArray<FString> PassiveSegments;
	for (const FFinalBattleHUDCharacterDetailPassiveEntry& Passive : Data.Passives)
	{
		PassiveSegments.Add(Passive.DisplayName.ToString());
	}

	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterDetailFallbackFormat", "{0} Lv.{1}\n压力 {2}/{3} | Vital {4}\n突破 {5}/{6} | 苏醒 {7}/{8} | 崩溃 {9}\n根骨 {10} | 悟性 {11} | 杀意 {12}\n攻击 {13} | 防御 {14} | 削韧率 {15}%\n暴击 {16}% | 暴伤 {17}%\n奥义: {18} EP {19}\n状态: {20}\n被动: {21}"),
		Data.DisplayName,
		FText::AsNumber(Data.Level),
		FText::AsNumber(Data.CurrentStress),
		FText::AsNumber(Data.StressCap),
		FText::AsNumber(Data.VitalShare),
		FText::AsNumber(Data.BreakthroughValue),
		FText::AsNumber(Data.BreakthroughRequiredValue),
		FText::AsNumber(Data.CurrentAwakenCount),
		FText::AsNumber(Data.CurrentAwakenThreshold),
		FText::AsNumber(Data.CollapseCount),
		FText::AsNumber(Data.RootBone),
		FText::AsNumber(Data.Insight),
		FText::AsNumber(Data.KillingIntent),
		FText::AsNumber(Data.RuntimeAttack),
		FText::AsNumber(Data.RuntimeDefense),
		FText::AsNumber(FMath::RoundToInt(Data.RuntimeBreakRate * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(Data.RuntimeCritChance * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(Data.RuntimeCritDamage * 100.0f)),
		Data.UltimateNameText,
		FText::AsNumber(Data.UltimateCostEP),
		FText::FromString(StatusSegments.Num() > 0 ? FString::Join(StatusSegments, TEXT(" | ")) : TEXT("无")),
		FText::FromString(PassiveSegments.Num() > 0 ? FString::Join(PassiveSegments, TEXT(" | ")) : TEXT("无")));
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
	CancelActiveCardDrag();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleHandPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleHandPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateDragState(MyGeometry, InDeltaTime);

	const bool bHoverLayoutDirty = UpdateHoverAlphas(InDeltaTime);
	if (bHandLayoutDirty)
	{
		const FVector2D LocalSize = MyGeometry.GetLocalSize();
		if (LocalSize.X > 0.0f && LocalSize.Y > 0.0f)
		{
			ArrangeHandCards();
		}
	}

	const bool bVisualsChanged = UpdateCardVisuals(InDeltaTime);
	if (bHoverLayoutDirty && !bVisualsChanged)
	{
		for (const TPair<FGuid, FFinalBattleHandCardVisualState>& VisualPair : CardVisuals)
		{
			ApplyCardVisualState(VisualPair.Value);
		}
	}
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

void UFinalBattleHandPanel::HandleCardHoverChanged(const FGuid CardInstanceId, const int32 HandIndex, const bool bHovered)
{
	if (ActiveDragCardInstanceId.IsValid() && ActiveDragCardInstanceId == CardInstanceId)
	{
		return;
	}

	if (bHovered)
	{
		HoveredCardInstanceId = CardInstanceId;
	}
	else if (HoveredCardInstanceId == CardInstanceId)
	{
		HoveredCardInstanceId.Invalidate();
	}
	bHandLayoutDirty = true;
}

void UFinalBattleHandPanel::HandleCardPointerPressed(const FGuid CardInstanceId, const int32 HandIndex, const FVector2D ScreenPosition)
{
	const FFinalBattleHandCardVisualState* Visual = CardVisuals.Find(CardInstanceId);
	if (Visual == nullptr || !Visual->bCanPlayHint || Visual->bLeaving || Visual->bEntering)
	{
		return;
	}

	DragCandidateCardInstanceId = CardInstanceId;
	DragCandidateHandIndex = HandIndex;
	DragStartScreenPosition = ScreenPosition;
	bHasDragCandidate = true;
}

void UFinalBattleHandPanel::HandleCardPointerReleased(const FGuid CardInstanceId, const int32 HandIndex, const FVector2D ScreenPosition)
{
	if (ActiveDragCardInstanceId.IsValid() && ActiveDragCardInstanceId == CardInstanceId)
	{
		FinishActiveCardDrag(GetCachedGeometry(), ScreenPosition);
		return;
	}

	if (bHasDragCandidate && DragCandidateCardInstanceId == CardInstanceId)
	{
		bHasDragCandidate = false;
		DragCandidateCardInstanceId.Invalidate();
		DragCandidateHandIndex = INDEX_NONE;
	}
}

void UFinalBattleHandPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("HandBorder"), FLinearColor(0.08f, 0.11f, 0.16f, 0.92f));
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HandSizeBox"));
	SizeBox->SetHeightOverride(PanelHeightOverride);
	HandCardCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HandCardCanvas"));
	SizeBox->SetContent(HandCardCanvas);
	Border->SetContent(SizeBox);
	WidgetTree->RootWidget = Border;
}

void UFinalBattleHandPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr || HandCardCanvas == nullptr)
	{
		return;
	}

	const TArray<FFinalBattleHUDCardEntry>& Entries = PanelViewModel->GetEntries();
	TSet<FGuid> IncomingCardIds;
	OrderedCardInstanceIds.Reset(Entries.Num());

	const FVector2D PanelSize = GetCachedGeometry().GetLocalSize();
	const float PanelWidth = PanelSize.X;
	const float PanelHeight = PanelSize.Y > 0.0f ? PanelSize.Y : PanelHeightOverride;
	const FVector2D SafeCardSize(
		FMath::Max(1.0f, CardSize.X),
		FMath::Max(1.0f, CardSize.Y));
	const float BaseY = PanelHeight - BottomPadding;
	const FVector2D EnterStartPosition(
		-(SafeCardSize.X + FMath::Abs(EnterStartOffset.X)),
		BaseY + EnterStartOffset.Y);
	const FVector2D ExitTargetPosition(
		PanelWidth + SafeCardSize.X + FMath::Abs(ExitTargetOffset.X),
		BaseY + ExitTargetOffset.Y);
	const float SafeCardScale = FMath::Max(0.01f, CardScale);

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		FFinalBattleHUDCardEntry Entry = Entries[Index];
		FGuid CardInstanceId = Entry.CardInstanceId;
		if (!CardInstanceId.IsValid() || IncomingCardIds.Contains(CardInstanceId))
		{
			CardInstanceId = FGuid::NewGuid();
			Entry.CardInstanceId = CardInstanceId;
		}

		IncomingCardIds.Add(CardInstanceId);
		OrderedCardInstanceIds.Add(CardInstanceId);

		FFinalBattleHandCardVisualState* ExistingVisual = CardVisuals.Find(CardInstanceId);
		if (ExistingVisual != nullptr && ExistingVisual->Widget.IsValid())
		{
			ExistingVisual->HandIndex = Index;
			ExistingVisual->bCanPlayHint = Entry.bCanPlayHint;
			ExistingVisual->TargetRequirement = Entry.TargetRequirement;
			ExistingVisual->bLeaving = false;
			ExistingVisual->Widget->Configure(PanelController, Index, Entry);
			continue;
		}

		UFinalBattleCardEntryWidget* CardWidget = CreateConfiguredEntryWidget(this, UFinalUIWidgetClassSettings::GetBattleCardEntryWidgetClass());
		if (CardWidget == nullptr)
		{
			continue;
		}

		CardWidget->Configure(PanelController, Index, Entry);
		CardWidget->OnCardHoverChanged.AddDynamic(this, &UFinalBattleHandPanel::HandleCardHoverChanged);
		CardWidget->OnCardPointerPressed.AddDynamic(this, &UFinalBattleHandPanel::HandleCardPointerPressed);
		CardWidget->OnCardPointerReleased.AddDynamic(this, &UFinalBattleHandPanel::HandleCardPointerReleased);

		FFinalBattleHandCardVisualState NewVisual;
		NewVisual.CardInstanceId = CardInstanceId;
		NewVisual.Widget = CardWidget;
		NewVisual.HandIndex = Index;
		NewVisual.bCanPlayHint = Entry.bCanPlayHint;
		NewVisual.TargetRequirement = Entry.TargetRequirement;
		const bool bShouldAnimateEntry = bHasReceivedHandSnapshot || bAnimateInitialHand;
		NewVisual.CurrentPosition = bShouldAnimateEntry ? EnterStartPosition : FVector2D::ZeroVector;
		NewVisual.TargetPosition = NewVisual.CurrentPosition;
		NewVisual.CurrentScale = SafeCardScale;
		NewVisual.TargetScale = SafeCardScale;
		NewVisual.bEntering = bShouldAnimateEntry;
		NewVisual.bSnapToTargetOnNextArrange = !bShouldAnimateEntry;

		if (UCanvasPanelSlot* CardSlot = HandCardCanvas->AddChildToCanvas(CardWidget))
		{
			CardSlot->SetAutoSize(false);
			CardSlot->SetSize(SafeCardSize);
			CardSlot->SetPosition(NewVisual.CurrentPosition);
			CardSlot->SetAlignment(FVector2D(0.5f, 1.0f));
			CardSlot->SetZOrder(Index);
		}

		CardVisuals.Add(CardInstanceId, NewVisual);
	}

	TArray<FGuid> CardIdsToMarkLeaving;
	for (const TPair<FGuid, FFinalBattleHandCardVisualState>& VisualPair : CardVisuals)
	{
		if (!IncomingCardIds.Contains(VisualPair.Key) && !VisualPair.Value.bLeaving)
		{
			CardIdsToMarkLeaving.Add(VisualPair.Key);
		}
	}

	for (const FGuid& CardInstanceId : CardIdsToMarkLeaving)
	{
		FFinalBattleHandCardVisualState* Visual = CardVisuals.Find(CardInstanceId);
		if (Visual == nullptr)
		{
			continue;
		}

		Visual->HandIndex = INDEX_NONE;
		Visual->bEntering = false;
		Visual->bLeaving = true;
		Visual->HoverAlpha = 0.0f;
		Visual->TargetPosition = ExitTargetPosition;
		Visual->TargetAngle = 0.0f;
		Visual->TargetScale = SafeCardScale;
		if (HoveredCardInstanceId == CardInstanceId)
		{
			HoveredCardInstanceId.Invalidate();
		}
		if (DragCandidateCardInstanceId == CardInstanceId)
		{
			bHasDragCandidate = false;
			DragCandidateCardInstanceId.Invalidate();
			DragCandidateHandIndex = INDEX_NONE;
		}
		if (ActiveDragCardInstanceId == CardInstanceId)
		{
			CancelActiveCardDrag();
		}
	}

	bHasReceivedHandSnapshot = true;
	bHandLayoutDirty = true;
	ArrangeHandCards();
}

void UFinalBattleHandPanel::ArrangeHandCards()
{
	if (HandCardCanvas == nullptr)
	{
		bHandLayoutDirty = false;
		return;
	}

	const int32 NumCards = OrderedCardInstanceIds.Num();
	if (NumCards == 0)
	{
		for (TPair<FGuid, FFinalBattleHandCardVisualState>& VisualPair : CardVisuals)
		{
			if (VisualPair.Value.bLeaving)
			{
				const FVector2D SafeCardSize(
					FMath::Max(1.0f, CardSize.X),
					FMath::Max(1.0f, CardSize.Y));
				const FVector2D PanelSize = GetCachedGeometry().GetLocalSize();
				const float PanelHeight = PanelSize.Y > 0.0f ? PanelSize.Y : PanelHeightOverride;
				const float BaseY = PanelHeight - BottomPadding;
				VisualPair.Value.TargetPosition = FVector2D(
					PanelSize.X + SafeCardSize.X + FMath::Abs(ExitTargetOffset.X),
					BaseY + ExitTargetOffset.Y);
			}
		}
		bHandLayoutDirty = false;
		return;
	}

	const FVector2D PanelSize = GetCachedGeometry().GetLocalSize();
	const float PanelWidth = PanelSize.X;
	const float PanelHeight = PanelSize.Y > 0.0f ? PanelSize.Y : PanelHeightOverride;
	if (PanelWidth <= 0.0f || PanelHeight <= 0.0f)
	{
		bHandLayoutDirty = true;
		return;
	}

	const FVector2D SafeCardSize(
		FMath::Max(1.0f, CardSize.X),
		FMath::Max(1.0f, CardSize.Y));
	const float BaseY = PanelHeight - BottomPadding;
	const float CenterX = PanelWidth * 0.5f;
	const float MidIndex = static_cast<float>(NumCards - 1) * 0.5f;
	const float RawSpacing = NumCards > 1 ? (PanelWidth - SafeCardSize.X) / static_cast<float>(NumCards - 1) : 0.0f;
	const float LowerSpacing = FMath::Min(MinSpacing, MaxSpacing);
	const float UpperSpacing = FMath::Max(MinSpacing, MaxSpacing);
	const float Spacing = bAllowOverlap
		? FMath::Clamp(RawSpacing, LowerSpacing, UpperSpacing)
		: FMath::Max(RawSpacing, SafeCardSize.X);
	const float Normalizer = FMath::Max(1.0f, MidIndex);
	const float SafeCardScale = FMath::Max(0.01f, CardScale);

	for (int32 Index = 0; Index < NumCards; ++Index)
	{
		const FGuid& CardInstanceId = OrderedCardInstanceIds[Index];
		FFinalBattleHandCardVisualState* Visual = CardVisuals.Find(CardInstanceId);
		if (Visual == nullptr || !Visual->Widget.IsValid())
		{
			continue;
		}

		const float OffsetFromCenter = static_cast<float>(Index) - MidIndex;
		const float Norm = OffsetFromCenter / Normalizer;
		const float LiftAlpha = 1.0f - FMath::Abs(Norm);
		const float CenterWeight = 1.0f - FMath::Clamp(FMath::Abs(Norm), 0.0f, 1.0f);
		const float LowerUnplayableDrop = FMath::Min(UnplayableDropMin, UnplayableDropMax);
		const float UpperUnplayableDrop = FMath::Max(UnplayableDropMin, UnplayableDropMax);
		const float UnplayableDropOffset = Visual->bCanPlayHint
			? 0.0f
			: FMath::Lerp(LowerUnplayableDrop, UpperUnplayableDrop, CenterWeight);
		Visual->TargetPosition = FVector2D(
			CenterX + OffsetFromCenter * Spacing,
			BaseY - LiftAlpha * CenterLift + UnplayableDropOffset);
		Visual->TargetAngle = Norm * MaxFanAngle;
		Visual->TargetScale = SafeCardScale;
		Visual->BaseZOrder = Index;
		Visual->HandIndex = Index;
		if (Visual->bSnapToTargetOnNextArrange)
		{
			Visual->CurrentPosition = Visual->TargetPosition;
			Visual->CurrentAngle = Visual->TargetAngle;
			Visual->CurrentScale = Visual->TargetScale;
			Visual->bSnapToTargetOnNextArrange = false;
		}
		if (!Visual->bEntering && !Visual->bLeaving)
		{
			ApplyCardVisualState(*Visual);
		}
	}

	for (TPair<FGuid, FFinalBattleHandCardVisualState>& VisualPair : CardVisuals)
	{
		FFinalBattleHandCardVisualState& Visual = VisualPair.Value;
		if (!Visual.bLeaving)
		{
			continue;
		}

		Visual.TargetPosition = FVector2D(
			PanelWidth + SafeCardSize.X + FMath::Abs(ExitTargetOffset.X),
			BaseY + ExitTargetOffset.Y);
		Visual.TargetAngle = 0.0f;
		Visual.TargetScale = SafeCardScale;
		ApplyCardVisualState(Visual);
	}

	bHandLayoutDirty = false;
}

void UFinalBattleHandPanel::UpdateDragState(const FGeometry& MyGeometry, const float InDeltaTime)
{
	FVector2D ScreenPosition = DragStartScreenPosition;
	if (FSlateApplication::IsInitialized())
	{
		ScreenPosition = FSlateApplication::Get().GetCursorPos();
	}

	if (!ActiveDragCardInstanceId.IsValid())
	{
		if (!bHasDragCandidate)
		{
			return;
		}

		if (FVector2D::Distance(ScreenPosition, DragStartScreenPosition) < FMath::Max(1.0f, DragStartDistance))
		{
			return;
		}

		FFinalBattleHandCardVisualState* Visual = CardVisuals.Find(DragCandidateCardInstanceId);
		if (Visual == nullptr || !Visual->bCanPlayHint || Visual->bLeaving || Visual->bEntering)
		{
			bHasDragCandidate = false;
			DragCandidateCardInstanceId.Invalidate();
			DragCandidateHandIndex = INDEX_NONE;
			return;
		}

		BeginActiveCardDrag(*Visual, ScreenPosition);
	}

	FFinalBattleHandCardVisualState* ActiveVisual = CardVisuals.Find(ActiveDragCardInstanceId);
	if (ActiveVisual == nullptr)
	{
		CancelActiveCardDrag();
		return;
	}

	const FVector2D LocalPointer = MyGeometry.AbsoluteToLocal(ScreenPosition);
	ActiveVisual->DragFollowPosition = LocalPointer + ActiveVisual->DragPointerOffset;

	if (ActiveDragTargetRequirement == EFinalBattleCardTargetRequirement::Enemy)
	{
		AFinalBattlePresentationActor* TargetActor = nullptr;
		if (UFinalBattleTargetInteractorComponent* TargetInteractor = ResolveTargetInteractor())
		{
			FVector2D PixelPosition = FVector2D::ZeroVector;
			FVector2D ViewportPosition = FVector2D::ZeroVector;
			USlateBlueprintLibrary::AbsoluteToViewport(this, ScreenPosition, PixelPosition, ViewportPosition);
			TargetActor = TargetInteractor->TraceEnemyTargetAtScreenPosition(PixelPosition);
		}

		if (ActiveDragTargetActor.Get() != TargetActor)
		{
			ClearDragPreviewTarget();
			ActiveDragTargetActor = TargetActor;
			if (TargetActor)
			{
				TargetActor->SetDropPreviewTarget(true);
			}
		}

		ActiveVisual->bDragLockedToTarget = TargetActor != nullptr;
	}
	else
	{
		ClearDragPreviewTarget();
		ActiveVisual->bDragLockedToTarget = false;
	}

	const float TargetLockAlpha = ActiveVisual->bDragLockedToTarget ? 1.0f : 0.0f;
	const float SafeLockInterpSpeed = FMath::Max(0.0f, DragTargetLockInterpSpeed);
	ActiveVisual->DragTargetLockAlpha = SafeLockInterpSpeed > 0.0f
		? FMath::FInterpTo(ActiveVisual->DragTargetLockAlpha, TargetLockAlpha, InDeltaTime, SafeLockInterpSpeed)
		: TargetLockAlpha;
	if (FMath::IsNearlyEqual(ActiveVisual->DragTargetLockAlpha, TargetLockAlpha, 0.001f))
	{
		ActiveVisual->DragTargetLockAlpha = TargetLockAlpha;
	}

	const float SafeCardScale = FMath::Max(0.01f, CardScale);
	const float SafeDragScaleInterpSpeed = FMath::Max(0.0f, DragScaleInterpSpeed);
	ActiveVisual->DragVisualScale = SafeDragScaleInterpSpeed > 0.0f
		? FMath::FInterpTo(ActiveVisual->DragVisualScale, SafeCardScale, InDeltaTime, SafeDragScaleInterpSpeed)
		: SafeCardScale;
	if (FMath::IsNearlyEqual(ActiveVisual->DragVisualScale, SafeCardScale, 0.001f))
	{
		ActiveVisual->DragVisualScale = SafeCardScale;
	}

	ApplyCardVisualState(*ActiveVisual);
}

void UFinalBattleHandPanel::BeginActiveCardDrag(FFinalBattleHandCardVisualState& VisualState, const FVector2D& ScreenPosition)
{
	UFinalBattleCardEntryWidget* CardWidget = VisualState.Widget.Get();
	if (CardWidget == nullptr)
	{
		return;
	}

	UCanvasPanelSlot* CardSlot = Cast<UCanvasPanelSlot>(CardWidget->Slot);
	const FVector2D LocalPointer = GetCachedGeometry().AbsoluteToLocal(ScreenPosition);
	VisualState.DragPointerOffset = CardSlot ? CardSlot->GetPosition() - LocalPointer : FVector2D::ZeroVector;
	VisualState.DragFollowPosition = LocalPointer + VisualState.DragPointerOffset;
	VisualState.bDragging = true;
	VisualState.bDragLockedToTarget = false;
	VisualState.DragTargetLockAlpha = 0.0f;
	VisualState.DragVisualScale = FMath::Lerp(
		VisualState.CurrentScale,
		VisualState.CurrentScale * FMath::Max(0.01f, HoverScale),
		FMath::Clamp(VisualState.HoverAlpha, 0.0f, 1.0f));
	VisualState.bEntering = false;
	VisualState.bLeaving = false;

	ActiveDragCardInstanceId = VisualState.CardInstanceId;
	ActiveDragHandIndex = VisualState.HandIndex;
	ActiveDragTargetRequirement = VisualState.TargetRequirement;
	HoveredCardInstanceId = VisualState.CardInstanceId;
	bHasDragCandidate = false;
	DragCandidateCardInstanceId.Invalidate();
	DragCandidateHandIndex = INDEX_NONE;
	CardWidget->SuppressNextClick();
	ApplyCardVisualState(VisualState);
}

void UFinalBattleHandPanel::FinishActiveCardDrag(const FGeometry& MyGeometry, const FVector2D& ScreenPosition)
{
	const FGuid FinishedCardInstanceId = ActiveDragCardInstanceId;
	FFinalBattleHandCardVisualState* Visual = CardVisuals.Find(FinishedCardInstanceId);
	const int32 FinishedHandIndex = ActiveDragHandIndex;
	const EFinalBattleCardTargetRequirement FinishedRequirement = ActiveDragTargetRequirement;
	AFinalBattlePresentationActor* TargetActor = ActiveDragTargetActor.Get();

	bool bSubmitted = false;
	if (PanelController)
	{
		if (FinishedRequirement == EFinalBattleCardTargetRequirement::Enemy)
		{
			if (TargetActor != nullptr)
			{
				bSubmitted = PanelController->PlayCardByHandIndexWithTarget(FinishedHandIndex, TargetActor->GetRuntimeUnitId());
			}
		}
		else if (FinishedRequirement == EFinalBattleCardTargetRequirement::None && IsOutsideHandPlayArea(MyGeometry, ScreenPosition))
		{
			bSubmitted = PanelController->PlayCardByHandIndex(FinishedHandIndex);
		}
	}

	if (Visual != nullptr)
	{
		Visual->bDragging = false;
		Visual->bDragLockedToTarget = false;
		Visual->DragTargetLockAlpha = 0.0f;
		Visual->DragVisualScale = FMath::Max(0.01f, CardScale);
	}

	ClearDragPreviewTarget();
	ActiveDragCardInstanceId.Invalidate();
	ActiveDragHandIndex = INDEX_NONE;
	ActiveDragTargetRequirement = EFinalBattleCardTargetRequirement::None;
	bHandLayoutDirty = true;

	if (!bSubmitted && Visual != nullptr)
	{
		ApplyCardVisualState(*Visual);
	}
}

void UFinalBattleHandPanel::CancelActiveCardDrag()
{
	if (FFinalBattleHandCardVisualState* Visual = CardVisuals.Find(ActiveDragCardInstanceId))
	{
		Visual->bDragging = false;
		Visual->bDragLockedToTarget = false;
		Visual->DragTargetLockAlpha = 0.0f;
		Visual->DragVisualScale = FMath::Max(0.01f, CardScale);
	}

	ClearDragPreviewTarget();
	ActiveDragCardInstanceId.Invalidate();
	ActiveDragHandIndex = INDEX_NONE;
	ActiveDragTargetRequirement = EFinalBattleCardTargetRequirement::None;
	bHasDragCandidate = false;
	DragCandidateCardInstanceId.Invalidate();
	DragCandidateHandIndex = INDEX_NONE;
	bHandLayoutDirty = true;
}

void UFinalBattleHandPanel::ClearDragPreviewTarget()
{
	if (AFinalBattlePresentationActor* PreviewActor = ActiveDragTargetActor.Get())
	{
		PreviewActor->SetDropPreviewTarget(false);
	}
	ActiveDragTargetActor.Reset();
}

bool UFinalBattleHandPanel::IsOutsideHandPlayArea(const FGeometry& MyGeometry, const FVector2D& ScreenPosition) const
{
	const FVector2D LocalPointer = MyGeometry.AbsoluteToLocal(ScreenPosition);
	const FVector2D LocalSize = MyGeometry.GetLocalSize();
	const float HandAreaPadding = DragPlayLeaveHandPadding;
	const bool bInside =
		LocalPointer.X >= -HandAreaPadding
		&& LocalPointer.Y >= -HandAreaPadding
		&& LocalPointer.X <= LocalSize.X + HandAreaPadding
		&& LocalPointer.Y <= LocalSize.Y + HandAreaPadding;
	return !bInside;
}

UFinalBattleTargetInteractorComponent* UFinalBattleHandPanel::ResolveTargetInteractor() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	return PlayerController ? PlayerController->FindComponentByClass<UFinalBattleTargetInteractorComponent>() : nullptr;
}

bool UFinalBattleHandPanel::UpdateHoverAlphas(const float InDeltaTime)
{
	bool bAnyAlphaChanged = false;
	const float SafeInterpSpeed = FMath::Max(0.0f, HoverInterpSpeed);
	for (TPair<FGuid, FFinalBattleHandCardVisualState>& VisualPair : CardVisuals)
	{
		FFinalBattleHandCardVisualState& Visual = VisualPair.Value;
		const bool bLockedDragCard = Visual.bDragging && Visual.DragTargetLockAlpha > 0.01f;
		const float TargetAlpha = (bLockedDragCard || (VisualPair.Key == HoveredCardInstanceId && Visual.bCanPlayHint && !Visual.bLeaving)) ? 1.0f : 0.0f;
		const float CurrentAlpha = Visual.HoverAlpha;
		float NewAlpha = SafeInterpSpeed > 0.0f
			? FMath::FInterpTo(CurrentAlpha, TargetAlpha, InDeltaTime, SafeInterpSpeed)
			: TargetAlpha;

		if (FMath::IsNearlyEqual(NewAlpha, TargetAlpha, 0.001f))
		{
			NewAlpha = TargetAlpha;
		}

		if (!FMath::IsNearlyEqual(CurrentAlpha, NewAlpha, KINDA_SMALL_NUMBER))
		{
			Visual.HoverAlpha = NewAlpha;
			bAnyAlphaChanged = true;
		}
	}

	return bAnyAlphaChanged;
}

bool UFinalBattleHandPanel::UpdateCardVisuals(const float InDeltaTime)
{
	bool bAnyVisualChanged = false;
	TArray<FGuid> CardIdsToRemove;
	const float SafeRemoveDistanceTolerance = FMath::Max(0.0f, RemoveDistanceTolerance);

	for (TPair<FGuid, FFinalBattleHandCardVisualState>& VisualPair : CardVisuals)
	{
		FFinalBattleHandCardVisualState& Visual = VisualPair.Value;
		UFinalBattleCardEntryWidget* CardWidget = Visual.Widget.Get();
		if (CardWidget == nullptr)
		{
			CardIdsToRemove.Add(VisualPair.Key);
			bAnyVisualChanged = true;
			continue;
		}

		if (Visual.bDragging)
		{
			ApplyCardVisualState(Visual);
			bAnyVisualChanged = true;
			continue;
		}

		const float InterpSpeed = Visual.bLeaving
			? FMath::Max(0.0f, ExitInterpSpeed)
			: (Visual.bEntering ? FMath::Max(0.0f, EnterInterpSpeed) : FMath::Max(0.0f, MoveInterpSpeed));

		const FVector2D PreviousPosition = Visual.CurrentPosition;
		const float PreviousAngle = Visual.CurrentAngle;
		const float PreviousScale = Visual.CurrentScale;

		if (InterpSpeed > 0.0f)
		{
			Visual.CurrentPosition = FMath::Vector2DInterpTo(Visual.CurrentPosition, Visual.TargetPosition, InDeltaTime, InterpSpeed);
			Visual.CurrentAngle = FMath::FInterpTo(Visual.CurrentAngle, Visual.TargetAngle, InDeltaTime, InterpSpeed);
			Visual.CurrentScale = FMath::FInterpTo(Visual.CurrentScale, Visual.TargetScale, InDeltaTime, InterpSpeed);
		}
		else
		{
			Visual.CurrentPosition = Visual.TargetPosition;
			Visual.CurrentAngle = Visual.TargetAngle;
			Visual.CurrentScale = Visual.TargetScale;
		}

		if (FVector2D::Distance(Visual.CurrentPosition, Visual.TargetPosition) <= 0.5f)
		{
			Visual.CurrentPosition = Visual.TargetPosition;
		}
		if (FMath::IsNearlyEqual(Visual.CurrentAngle, Visual.TargetAngle, 0.01f))
		{
			Visual.CurrentAngle = Visual.TargetAngle;
		}
		if (FMath::IsNearlyEqual(Visual.CurrentScale, Visual.TargetScale, 0.001f))
		{
			Visual.CurrentScale = Visual.TargetScale;
		}

		ApplyCardVisualState(Visual);

		const bool bMoved = FVector2D::Distance(PreviousPosition, Visual.CurrentPosition) > KINDA_SMALL_NUMBER
			|| !FMath::IsNearlyEqual(PreviousAngle, Visual.CurrentAngle, KINDA_SMALL_NUMBER)
			|| !FMath::IsNearlyEqual(PreviousScale, Visual.CurrentScale, KINDA_SMALL_NUMBER);
		bAnyVisualChanged = bAnyVisualChanged || bMoved;

		if (Visual.bEntering && Visual.CurrentPosition == Visual.TargetPosition)
		{
			Visual.bEntering = false;
		}

		if (Visual.bLeaving && FVector2D::Distance(Visual.CurrentPosition, Visual.TargetPosition) <= SafeRemoveDistanceTolerance)
		{
			CardWidget->RemoveFromParent();
			CardIdsToRemove.Add(VisualPair.Key);
			bAnyVisualChanged = true;
		}
	}

	for (const FGuid& CardInstanceId : CardIdsToRemove)
	{
		CardVisuals.Remove(CardInstanceId);
	}

	return bAnyVisualChanged;
}

void UFinalBattleHandPanel::ApplyCardVisualState(const FFinalBattleHandCardVisualState& VisualState)
{
	UFinalBattleCardEntryWidget* CardWidget = VisualState.Widget.Get();
	if (CardWidget == nullptr)
	{
		return;
	}

	UCanvasPanelSlot* CardSlot = Cast<UCanvasPanelSlot>(CardWidget->Slot);
	if (CardSlot == nullptr)
	{
		return;
	}

	const FVector2D SafeCardSize(
		FMath::Max(1.0f, CardSize.X),
		FMath::Max(1.0f, CardSize.Y));
	const FVector2D PanelSize = GetCachedGeometry().GetLocalSize();
	const float PanelHeight = PanelSize.Y > 0.0f ? PanelSize.Y : PanelHeightOverride;
	const float BaseY = PanelHeight - BottomPadding;
	const float SafeHoverScale = FMath::Max(0.01f, HoverScale);
	const float DragLockAlpha = VisualState.bDragging
		? FMath::Clamp(VisualState.DragTargetLockAlpha, 0.0f, 1.0f)
		: 0.0f;
	const float HoverAlpha = VisualState.bDragging
		? DragLockAlpha
		: (VisualState.bLeaving ? 0.0f : VisualState.HoverAlpha);
	const float BaseOpacity = VisualState.bCanPlayHint
		? 1.0f
		: FMath::Clamp(UnplayableOpacity, 0.0f, 1.0f);
	const FVector2D BasePosition = VisualState.bDragging
		? VisualState.DragFollowPosition
		: VisualState.CurrentPosition;
	const float BaseAngle = VisualState.bDragging
		? 0.0f
		: VisualState.CurrentAngle;
	const float BaseScale = VisualState.bDragging
		? FMath::Max(0.01f, VisualState.DragVisualScale)
		: VisualState.CurrentScale;
	const FVector2D HoverPosition(
		VisualState.bDragging ? VisualState.CurrentPosition.X : BasePosition.X,
		FMath::RoundToFloat(BaseY - HoverLift));
	const FVector2D BlendedPosition = FMath::Lerp(BasePosition, HoverPosition, HoverAlpha);
	const float BlendedAngle = FMath::Lerp(BaseAngle, HoverAngle, HoverAlpha);
	const float BlendedScale = VisualState.bDragging
		? BaseScale
		: FMath::Lerp(BaseScale, BaseScale * SafeHoverScale, HoverAlpha);
	const bool bShouldRaiseCard = (VisualState.CardInstanceId == HoveredCardInstanceId || VisualState.bDragging) && VisualState.bCanPlayHint && !VisualState.bLeaving;

	CardSlot->SetAutoSize(false);
	CardSlot->SetSize(SafeCardSize);
	CardSlot->SetPosition(FVector2D(
		FMath::RoundToFloat(BlendedPosition.X),
		FMath::RoundToFloat(BlendedPosition.Y)));
	CardSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	CardSlot->SetZOrder(VisualState.bDragging
		? DraggingZOrder + VisualState.BaseZOrder
		: (bShouldRaiseCard ? HoverZOrder + VisualState.BaseZOrder : VisualState.BaseZOrder));

	FWidgetTransform Transform = CardWidget->GetRenderTransform();
	Transform.Angle = BlendedAngle;
	Transform.Scale = FVector2D(BlendedScale, BlendedScale);
	Transform.Translation = FVector2D::ZeroVector;
	CardWidget->SetRenderTransform(Transform);
	CardWidget->SetRenderTransformPivot(FVector2D(0.5f, 1.0f));
	const float DragLockedOpacityAlpha = VisualState.bDragging ? DragLockAlpha : 0.0f;
	CardWidget->SetRenderOpacity(FMath::Lerp(
		BaseOpacity,
		FMath::Clamp(DragTargetLockedOpacity, 0.0f, 1.0f),
		DragLockedOpacityAlpha));
}

void UFinalBattleCardZoneDetailPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleCardZoneDetailPanel::NativeConstruct()
{
	Super::NativeConstruct();
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.AddDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleViewModelChanged);
	}
	RefreshFromViewModel();
}

void UFinalBattleCardZoneDetailPanel::NativeDestruct()
{
	if (PanelViewModel)
	{
		PanelViewModel->OnViewModelChanged.RemoveDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UFinalBattleCardZoneDetailPanel::InitializePanel(UFinalBattleCardZoneDetailPanelViewModel* InViewModel, UFinalBattleCardZoneDetailPanelController* InController)
{
	PanelViewModel = InViewModel;
	PanelController = InController;
	SetPresentationContext(InController, InViewModel);
	RefreshFromViewModel();
}

void UFinalBattleCardZoneDetailPanel::HandleViewModelChanged()
{
	RefreshFromViewModel();
}

void UFinalBattleCardZoneDetailPanel::HandleCloseClicked()
{
	if (PanelController)
	{
		PanelController->ClearCardZoneDetail();
	}
}

void UFinalBattleCardZoneDetailPanel::HandleDrawPileClicked()
{
	SelectZone(EFinalBattleCardZone::DrawPile);
}

void UFinalBattleCardZoneDetailPanel::HandleHandClicked()
{
	SelectZone(EFinalBattleCardZone::Hand);
}

void UFinalBattleCardZoneDetailPanel::HandleDiscardPileClicked()
{
	SelectZone(EFinalBattleCardZone::DiscardPile);
}

void UFinalBattleCardZoneDetailPanel::HandleOngoingZoneClicked()
{
	SelectZone(EFinalBattleCardZone::OngoingZone);
}

void UFinalBattleCardZoneDetailPanel::HandleConsumePileClicked()
{
	SelectZone(EFinalBattleCardZone::ConsumePile);
}

void UFinalBattleCardZoneDetailPanel::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Border = CreateSection(WidgetTree, TEXT("CardZoneDetailBorder"), FLinearColor(0.06f, 0.06f, 0.07f, 0.96f));
	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardZoneDetailRoot"));
	Border->SetContent(RootBox);

	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CardZoneDetailHeader"));
	RootBox->AddChildToVerticalBox(HeaderRow);

	TitleText = CreateLabel(WidgetTree, TEXT("TitleText"), 16);
	TitleText->SetText(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailDefaultTitle", "牌区详情"));
	if (UHorizontalBoxSlot* TitleSlot = HeaderRow->AddChildToHorizontalBox(TitleText))
	{
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TitleSlot->SetVerticalAlignment(VAlign_Center);
	}

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	UTextBlock* CloseLabel = CreateLabel(WidgetTree, TEXT("CloseButtonText"), 13);
	CloseLabel->SetText(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailClose", "关闭"));
	CloseButton->AddChild(CloseLabel);
	CloseButton->OnClicked.AddDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleCloseClicked);
	if (UHorizontalBoxSlot* CloseSlot = HeaderRow->AddChildToHorizontalBox(CloseButton))
	{
		CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	UHorizontalBox* TabRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CardZoneDetailTabs"));
	if (UVerticalBoxSlot* TabRowSlot = RootBox->AddChildToVerticalBox(TabRow))
	{
		TabRowSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 6.0f));
	}

	DrawPileTabButton = CreateTextButton(WidgetTree, TEXT("DrawPileTabButton"), TEXT("DrawPileTabText"), NSLOCTEXT("FinalBattleHUD", "CardZoneDetailDrawPileTab", "抽牌"), DrawPileTabText);
	HandTabButton = CreateTextButton(WidgetTree, TEXT("HandTabButton"), TEXT("HandTabText"), NSLOCTEXT("FinalBattleHUD", "CardZoneDetailHandTab", "手牌"), HandTabText);
	DiscardPileTabButton = CreateTextButton(WidgetTree, TEXT("DiscardPileTabButton"), TEXT("DiscardPileTabText"), NSLOCTEXT("FinalBattleHUD", "CardZoneDetailDiscardPileTab", "弃牌"), DiscardPileTabText);
	OngoingZoneTabButton = CreateTextButton(WidgetTree, TEXT("OngoingZoneTabButton"), TEXT("OngoingZoneTabText"), NSLOCTEXT("FinalBattleHUD", "CardZoneDetailOngoingZoneTab", "持续"), OngoingZoneTabText);
	ConsumePileTabButton = CreateTextButton(WidgetTree, TEXT("ConsumePileTabButton"), TEXT("ConsumePileTabText"), NSLOCTEXT("FinalBattleHUD", "CardZoneDetailConsumePileTab", "消耗"), ConsumePileTabText);

	if (DrawPileTabButton) { DrawPileTabButton->OnClicked.AddDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleDrawPileClicked); }
	if (HandTabButton) { HandTabButton->OnClicked.AddDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleHandClicked); }
	if (DiscardPileTabButton) { DiscardPileTabButton->OnClicked.AddDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleDiscardPileClicked); }
	if (OngoingZoneTabButton) { OngoingZoneTabButton->OnClicked.AddDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleOngoingZoneClicked); }
	if (ConsumePileTabButton) { ConsumePileTabButton->OnClicked.AddDynamic(this, &UFinalBattleCardZoneDetailPanel::HandleConsumePileClicked); }

	UButton* TabButtons[] = { DrawPileTabButton, HandTabButton, DiscardPileTabButton, OngoingZoneTabButton, ConsumePileTabButton };
	for (UButton* TabButton : TabButtons)
	{
		if (TabButton == nullptr)
		{
			continue;
		}
		if (UHorizontalBoxSlot* TabSlot = TabRow->AddChildToHorizontalBox(TabButton))
		{
			TabSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TabSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
		}
	}

	USizeBox* ListSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CardZoneDetailListSizeBox"));
	ListSizeBox->SetHeightOverride(430.0f);
	UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("CardZoneDetailScrollBox"));
	CardListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardListBox"));
	ScrollBox->AddChild(CardListBox);
	ListSizeBox->SetContent(ScrollBox);
	if (UVerticalBoxSlot* ListSlot = RootBox->AddChildToVerticalBox(ListSizeBox))
	{
		ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	EmptyText = CreateLabel(WidgetTree, TEXT("EmptyText"), 13);
	EmptyText->SetText(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailEmpty", "这个牌区暂无卡牌。"));
	EmptyText->SetVisibility(ESlateVisibility::Collapsed);
	if (UVerticalBoxSlot* EmptySlot = RootBox->AddChildToVerticalBox(EmptyText))
	{
		EmptySlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	WidgetTree->RootWidget = Border;
}

void UFinalBattleCardZoneDetailPanel::RefreshFromViewModel()
{
	if (PanelViewModel == nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FFinalBattleHUDCardZoneDetailData& Data = PanelViewModel->GetData();
	const bool bShouldShow = Data.bIsOpen && Data.bHasActiveBattle;
	SetVisibility(bShouldShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (!bShouldShow)
	{
		return;
	}

	if (TitleText)
	{
		TitleText->SetText(Data.TitleText);
	}

	if (DrawPileTabText)
	{
		DrawPileTabText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailDrawPileTabCount", "抽牌 {0}"), FText::AsNumber(Data.DrawPileCount)));
	}
	if (HandTabText)
	{
		HandTabText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailHandTabCount", "手牌 {0}"), FText::AsNumber(Data.HandCount)));
	}
	if (DiscardPileTabText)
	{
		DiscardPileTabText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailDiscardPileTabCount", "弃牌 {0}"), FText::AsNumber(Data.DiscardPileCount)));
	}
	if (OngoingZoneTabText)
	{
		OngoingZoneTabText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailOngoingZoneTabCount", "持续 {0}"), FText::AsNumber(Data.OngoingZoneCount)));
	}
	if (ConsumePileTabText)
	{
		ConsumePileTabText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "CardZoneDetailConsumePileTabCount", "消耗 {0}"), FText::AsNumber(Data.ConsumePileCount)));
	}

	if (CardListBox)
	{
		CardListBox->ClearChildren();
		for (const FFinalBattleHUDCardZoneEntry& Entry : Data.Entries)
		{
			UFinalBattleCardZoneEntryWidget* EntryWidget = CreateConfiguredEntryWidget(this, UFinalUIWidgetClassSettings::GetBattleCardZoneEntryWidgetClass());
			if (EntryWidget == nullptr)
			{
				continue;
			}
			EntryWidget->ApplyCardZoneEntryView(Entry);
			if (UVerticalBoxSlot* EntrySlot = CardListBox->AddChildToVerticalBox(EntryWidget))
			{
				EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
			}
		}
	}

	if (EmptyText)
	{
		EmptyText->SetVisibility(Data.Entries.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UFinalBattleCardZoneDetailPanel::SelectZone(const EFinalBattleCardZone Zone)
{
	if (PanelController)
	{
		PanelController->SetSelectedCardZone(Zone);
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
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("UltimateSizeBox"));
	SizeBox->SetHeightOverride(160.0f);
	UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("UltimateScrollBox"));
	UltimateListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("UltimateListBox"));
	ScrollBox->AddChild(UltimateListBox);
	SizeBox->SetContent(ScrollBox);
	Border->SetContent(SizeBox);
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
		UFinalBattleUltimateEntryWidget* UltimateWidget = CreateConfiguredEntryWidget(this, UFinalUIWidgetClassSettings::GetBattleUltimateEntryWidgetClass());
		if (UltimateWidget == nullptr)
		{
			continue;
		}

		UltimateWidget->Configure(PanelController, Index, Entries[Index]);
		if (UVerticalBoxSlot* UltimateSlot = UltimateListBox->AddChildToVerticalBox(UltimateWidget))
		{
			UltimateSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}
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
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RecentEventSizeBox"));
	SizeBox->SetHeightOverride(84.0f);
	UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("RecentEventScrollBox"));
	RecentEventListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RecentEventListBox"));
	ScrollBox->AddChild(RecentEventListBox);
	SizeBox->SetContent(ScrollBox);
	Border->SetContent(SizeBox);
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
		UFinalBattleLogEntryWidget* EntryWidget = CreateConfiguredEntryWidget(this, UFinalUIWidgetClassSettings::GetBattleLogEntryWidgetClass());
		if (EntryWidget == nullptr)
		{
			continue;
		}

		EntryWidget->Configure(Entry);
		if (UVerticalBoxSlot* EventSlot = RecentEventListBox->AddChildToVerticalBox(EntryWidget))
		{
			EventSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}
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
	OpenDebugLabel->SetText(NSLOCTEXT("FinalBattleHUD", "OpenDebugLabel", "调试"));
	OpenDebugButton->AddChild(OpenDebugLabel);
	if (UVerticalBoxSlot* DebugSlot = ActionColumn->AddChildToVerticalBox(OpenDebugButton))
	{
		DebugSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	OpenEventLedgerButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OpenEventLedgerButton"));
	OpenEventLedgerButton->OnClicked.AddDynamic(this, &UFinalBattleActionPanel::HandleOpenEventLedgerClicked);
	OpenEventLedgerLabel = CreateLabel(WidgetTree, TEXT("OpenEventLedgerLabel"), 12);
	OpenEventLedgerLabel->SetText(NSLOCTEXT("FinalBattleHUD", "OpenEventLedgerLabel", "账本"));
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

	if (EndTurnLabel)
	{
		EndTurnLabel->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "EndTurnWithPileCounts", "结束回合\n弃牌 {0} / 消耗 {1}"),
			FText::AsNumber(Data.DiscardPileCount),
			FText::AsNumber(Data.ConsumePileCount)));
	}
}
