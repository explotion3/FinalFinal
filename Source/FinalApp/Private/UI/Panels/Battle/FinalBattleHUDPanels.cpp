#include "UI/Panels/Battle/FinalBattleHUDPanels.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "Queries/FinalRunSnapshot.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
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
		return;
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

void UFinalBattleHandPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

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

		FFinalBattleHandCardVisualState NewVisual;
		NewVisual.CardInstanceId = CardInstanceId;
		NewVisual.Widget = CardWidget;
		NewVisual.HandIndex = Index;
		NewVisual.bCanPlayHint = Entry.bCanPlayHint;
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

bool UFinalBattleHandPanel::UpdateHoverAlphas(const float InDeltaTime)
{
	bool bAnyAlphaChanged = false;
	const float SafeInterpSpeed = FMath::Max(0.0f, HoverInterpSpeed);
	for (TPair<FGuid, FFinalBattleHandCardVisualState>& VisualPair : CardVisuals)
	{
		FFinalBattleHandCardVisualState& Visual = VisualPair.Value;
		const float TargetAlpha = VisualPair.Key == HoveredCardInstanceId && Visual.bCanPlayHint && !Visual.bLeaving ? 1.0f : 0.0f;
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
	const float HoverAlpha = VisualState.bLeaving ? 0.0f : VisualState.HoverAlpha;
	const float BaseOpacity = VisualState.bCanPlayHint
		? 1.0f
		: FMath::Clamp(UnplayableOpacity, 0.0f, 1.0f);
	const FVector2D HoverPosition(
		VisualState.CurrentPosition.X,
		FMath::RoundToFloat(BaseY - HoverLift));
	const FVector2D BlendedPosition = FMath::Lerp(VisualState.CurrentPosition, HoverPosition, HoverAlpha);
	const float BlendedAngle = FMath::Lerp(VisualState.CurrentAngle, HoverAngle, HoverAlpha);
	const float BlendedScale = FMath::Lerp(VisualState.CurrentScale, VisualState.CurrentScale * SafeHoverScale, HoverAlpha);
	const bool bShouldRaiseCard = VisualState.CardInstanceId == HoveredCardInstanceId && VisualState.bCanPlayHint && !VisualState.bLeaving;

	CardSlot->SetAutoSize(false);
	CardSlot->SetSize(SafeCardSize);
	CardSlot->SetPosition(FVector2D(
		FMath::RoundToFloat(BlendedPosition.X),
		FMath::RoundToFloat(BlendedPosition.Y)));
	CardSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	CardSlot->SetZOrder(bShouldRaiseCard ? HoverZOrder + VisualState.BaseZOrder : VisualState.BaseZOrder);

	FWidgetTransform Transform = CardWidget->GetRenderTransform();
	Transform.Angle = BlendedAngle;
	Transform.Scale = FVector2D(BlendedScale, BlendedScale);
	Transform.Translation = FVector2D::ZeroVector;
	CardWidget->SetRenderTransform(Transform);
	CardWidget->SetRenderTransformPivot(FVector2D(0.5f, 1.0f));
	CardWidget->SetRenderOpacity(BaseOpacity);
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
