#include "UI/Screens/Flow/FinalRunRewardOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Events/FinalRunEvent.h"
#include "Facade/FinalRunSession.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"

namespace
{
FText FormatRewardOptionalName(const FName Name, const FText& Fallback)
{
	return Name != NAME_None ? FText::FromName(Name) : Fallback;
}

FText FormatBattleOutcomeText(const EFinalBattleOutcome Outcome)
{
	switch (Outcome)
	{
	case EFinalBattleOutcome::Victory:
		return NSLOCTEXT("FinalFlowUI", "BattleOutcomeVictory", "胜利");

	case EFinalBattleOutcome::Defeat:
		return NSLOCTEXT("FinalFlowUI", "BattleOutcomeDefeat", "失败");

	case EFinalBattleOutcome::Escape:
		return NSLOCTEXT("FinalFlowUI", "BattleOutcomeEscape", "撤离");

	default:
		return NSLOCTEXT("FinalFlowUI", "BattleOutcomeNone", "未结算");
	}
}

UTextBlock* CreateRewardOverlayLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

UFinalRunSession* ResolveRewardOverlayRunSession(UUserWidget* Widget)
{
	if (Widget == nullptr)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = Widget->GetGameInstance();
	if (GameInstance == nullptr)
	{
		return nullptr;
	}

	if (UFinalGameFlowSubsystem* GameFlowSubsystem = GameInstance->GetSubsystem<UFinalGameFlowSubsystem>())
	{
		return GameFlowSubsystem->GetRunSession();
	}

	return nullptr;
}

FText ResolveRewardOverlayLatestRunFeedback(UFinalRunSession* RunSession, const FText& Fallback)
{
	if (RunSession == nullptr)
	{
		return Fallback;
	}

	const TArray<FFinalRunEvent> RunEvents = RunSession->GetRunLogEntries();
	if (RunEvents.Num() > 0)
	{
		const FFinalRunEvent& LastEvent = RunEvents.Last();
		if (!LastEvent.Message.IsEmpty())
		{
			return LastEvent.Message;
		}
	}

	return Fallback;
}
}

void UFinalRunRewardOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	CachedSnapshot = InSnapshot;
	LastActionFeedback = FText::GetEmpty();
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardClicked()
{
	UFinalRunSession* RunSession = ResolveRewardOverlayRunSession(this);
	if (RunSession == nullptr)
	{
		LastActionFeedback = NSLOCTEXT("FinalFlowUI", "RewardNoRunSession", "当前无法访问 RunSession，无法领取待领奖励。");
		RebuildVisual();
		return;
	}

	const bool bClaimed = RunSession->ClaimPendingBattleReward();
	CachedSnapshot = RunSession->GetSnapshot();
	LastActionFeedback = ResolveRewardOverlayLatestRunFeedback(
		RunSession,
		bClaimed
			? NSLOCTEXT("FinalFlowUI", "RewardClaimSucceeded", "已转发 ClaimPendingBattleReward。")
			: NSLOCTEXT("FinalFlowUI", "RewardClaimFailed", "ClaimPendingBattleReward 执行失败。"));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->ShowNodeProgressOverlayPlaceholder();
		}
	}
}

void UFinalRunRewardOverlayScreen::HandleCloseClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->CloseOverlayScreen(this);
		}
	}
}

void UFinalRunRewardOverlayScreen::HandleOpenModalClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->ShowPlaceholderModal(
				NSLOCTEXT("FinalFlowUI", "RewardModalTitle", "奖励确认占位"),
				NSLOCTEXT("FinalFlowUI", "RewardModalBody", "当前 RootLayout 已支持在 Overlay 之上打开 Modal。后续奖励确认、放弃奖励、二次确认都应落在这一层。"));
		}
	}
}

void UFinalRunRewardOverlayScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RewardOverlayRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.06f, 0.08f, 0.11f, 0.96f));
	RootBorder->SetPadding(FMargin(24.0f));
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardOverlayContent"));
	RootBorder->SetContent(ContentBox);

	TitleText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayTitle"), 22);
	ContentBox->AddChildToVerticalBox(TitleText);

	SummaryText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlaySummary"), 14);
	ContentBox->AddChildToVerticalBox(SummaryText);

	GapText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayGap"), 12);
	ContentBox->AddChildToVerticalBox(GapText);

	FeedbackText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayFeedback"), 12);
	ContentBox->AddChildToVerticalBox(FeedbackText);

	ClaimRewardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RewardOverlayClaimButton"));
	ClaimRewardButtonText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayClaimButtonText"), 13);
	ClaimRewardButton->AddChild(ClaimRewardButtonText);
	ClaimRewardButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardClicked);
	ContentBox->AddChildToVerticalBox(ClaimRewardButton);

	OpenNodePageButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RewardOverlayNodeButton"));
	OpenNodePageButtonText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayNodeButtonText"), 13);
	OpenNodePageButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardNodeButton", "查看节点推进页"));
	OpenNodePageButton->AddChild(OpenNodePageButtonText);
	OpenNodePageButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked);
	ContentBox->AddChildToVerticalBox(OpenNodePageButton);

	OpenModalButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RewardOverlayModalButton"));
	OpenModalButtonText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayModalButtonText"), 13);
	OpenModalButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardModalButton", "打开奖励说明模态"));
	OpenModalButton->AddChild(OpenModalButtonText);
	OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenModalClicked);
	ContentBox->AddChildToVerticalBox(OpenModalButton);

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RewardOverlayCloseButton"));
	CloseButtonText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayCloseButtonText"), 13);
	CloseButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardCloseButton", "关闭奖励页"));
	CloseButton->AddChild(CloseButtonText);
	CloseButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleCloseClicked);
	ContentBox->AddChildToVerticalBox(CloseButton);
}

void UFinalRunRewardOverlayScreen::RebuildVisual()
{
	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RewardOverlayTitleText", "战后奖励页"));
	}

	if (SummaryText)
	{
		const FFinalRunPendingBattleRewardViewData& PendingReward = CachedSnapshot.PendingBattleReward;
		const FFinalRunProgressionViewData& Progression = CachedSnapshot.Progression;
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardOverlaySummaryText", "待领奖励: {0}\n来源节点: {1}\n来源遭遇: {2}\n战斗结果: {3}\n奖励金币: {4}\n允许领取: {5}\n当前金币: {6} | 遗物数: {7} | 牌库数: {8}"),
			PendingReward.bHasPendingReward
				? NSLOCTEXT("FinalFlowUI", "RewardHasPendingReward", "是")
				: NSLOCTEXT("FinalFlowUI", "RewardHasNoPendingReward", "否"),
			FormatRewardOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNode", "无")),
			PendingReward.SourceEncounterId.IsValid()
				? FText::FromName(PendingReward.SourceEncounterId.Value)
				: NSLOCTEXT("FinalFlowUI", "RewardNoEncounter", "无"),
			FormatBattleOutcomeText(PendingReward.SourceBattleOutcome),
			FText::AsNumber(PendingReward.RewardGold),
			Progression.bCanClaimPendingBattleReward
				? NSLOCTEXT("FinalFlowUI", "RewardCanClaim", "是")
				: NSLOCTEXT("FinalFlowUI", "RewardCannotClaim", "否"),
			FText::AsNumber(CachedSnapshot.Gold),
			FText::AsNumber(CachedSnapshot.RelicCount),
			FText::AsNumber(CachedSnapshot.DeckCount)));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardOverlayGapText",
			"当前页已真实消费 PendingBattleReward 与 Progression.bCanClaimPendingBattleReward。剩余缺口主要是结构化 RewardEntries、奖励项显示元数据、以及多奖励选择布局。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(!LastActionFeedback.IsEmpty()
			? LastActionFeedback
			: NSLOCTEXT("FinalFlowUI", "RewardOverlayFeedbackDefault", "当前页面会直接转发 RunSession::ClaimPendingBattleReward()。"));
	}

	if (ClaimRewardButton)
	{
		const bool bCanClaimReward = CachedSnapshot.PendingBattleReward.bHasPendingReward && CachedSnapshot.Progression.bCanClaimPendingBattleReward;
		ClaimRewardButton->SetIsEnabled(bCanClaimReward);
	}

	if (ClaimRewardButtonText)
	{
		ClaimRewardButtonText->SetText(CachedSnapshot.PendingBattleReward.bHasPendingReward
			? NSLOCTEXT("FinalFlowUI", "RewardClaimButton", "领取待领奖励")
			: NSLOCTEXT("FinalFlowUI", "RewardClaimButtonDisabled", "当前没有待领奖励"));
	}

	if (OpenNodePageButton)
	{
		OpenNodePageButton->SetIsEnabled(CachedSnapshot.Progression.bCanAdvanceToNextNode || CachedSnapshot.Progression.CurrentNodeId != NAME_None);
	}
}
