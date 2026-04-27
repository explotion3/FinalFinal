#include "UI/Screens/Flow/FinalRunRewardOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

using namespace FinalRunFlowScreenUtils;

void UFinalRunRewardOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardClicked()
{
	const FFinalRunPendingBattleRewardViewData& PendingReward = GetCachedSnapshot().PendingBattleReward;
	if (PendingReward.RewardEntries.Num() == 1)
	{
		HandleClaimRewardOptionClicked(0);
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystem", "当前无法访问 RunFlowSubsystem，无法领取待领奖励。"));
		RebuildVisual();
		return;
	}

	const bool bClaimed = RunFlowSubsystem->ClaimPendingBattleReward();
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bClaimed
			? NSLOCTEXT("FinalFlowUI", "RewardClaimSucceeded", "已转发 ClaimPendingBattleReward。")
			: NSLOCTEXT("FinalFlowUI", "RewardClaimFailed", "ClaimPendingBattleReward 执行失败。")));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOption0Clicked()
{
	HandleClaimRewardOptionClicked(0);
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOption1Clicked()
{
	HandleClaimRewardOptionClicked(1);
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOption2Clicked()
{
	HandleClaimRewardOptionClicked(2);
}

void UFinalRunRewardOverlayScreen::HandleSkipRewardClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystemForSkip", "当前无法访问 RunFlowSubsystem，无法跳过待领奖励。"));
		RebuildVisual();
		return;
	}

	const bool bSkipped = RunFlowSubsystem->SkipPendingBattleReward();
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bSkipped
			? NSLOCTEXT("FinalFlowUI", "RewardSkipSucceeded", "已跳过战后卡牌奖励。")
			: NSLOCTEXT("FinalFlowUI", "RewardSkipFailed", "跳过战后卡牌奖励失败。")));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowNodeSelectOverlayPlaceholder();
	}
}

void UFinalRunRewardOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunRewardOverlayScreen::HandleOpenModalClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->ShowPlaceholderModal(
			NSLOCTEXT("FinalFlowUI", "RewardModalTitle", "奖励确认占位"),
			NSLOCTEXT("FinalFlowUI", "RewardModalBody", "当前 RootLayout 已支持在 Overlay 之上打开 Modal。后续奖励确认、放弃奖励、二次确认都应落在这一层。"));
	}
}

void UFinalRunRewardOverlayScreen::HandleClaimRewardOptionClicked(const int32 RewardIndex)
{
	const FFinalRunPendingBattleRewardViewData& PendingReward = GetCachedSnapshot().PendingBattleReward;
	if (!PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardOptionMissing", "当前奖励候选不存在，无法领取。"));
		RebuildVisual();
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystemForOption", "当前无法访问 RunFlowSubsystem，无法领取待领奖励。"));
		RebuildVisual();
		return;
	}

	const FName RewardId = PendingReward.RewardEntries[RewardIndex].RewardId;
	const bool bClaimed = RunFlowSubsystem->ClaimPendingBattleRewardById(RewardId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bClaimed
			? NSLOCTEXT("FinalFlowUI", "RewardOptionClaimSucceeded", "已领取战后卡牌奖励。")
			: NSLOCTEXT("FinalFlowUI", "RewardOptionClaimFailed", "领取战后卡牌奖励失败。")));
	RebuildVisual();
}

void UFinalRunRewardOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.06f, 0.08f, 0.11f, 0.96f), TEXT("RewardOverlayRoot"), TEXT("RewardOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (RewardEntriesText == nullptr)
	{
		RewardEntriesText = CreateStageLabel(TEXT("RewardOverlayEntries"), 13);
		ContentBox->InsertChildAt(2, RewardEntriesText);
	}

	if (ClaimRewardButton == nullptr)
	{
		ClaimRewardButton = CreateStageButton(
			TEXT("RewardOverlayClaimButton"),
			TEXT("RewardOverlayClaimButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardClaimButton", "领取当前奖励条目"),
			ClaimRewardButtonText);
		ClaimRewardButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardClicked);
		ContentBox->AddChildToVerticalBox(ClaimRewardButton);
	}

	if (ClaimRewardOption0Button == nullptr)
	{
		ClaimRewardOption0Button = CreateStageButton(
			TEXT("RewardOverlayClaimOption0Button"),
			TEXT("RewardOverlayClaimOption0ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardClaimOption0Button", "选择奖励 1"),
			ClaimRewardOption0ButtonText);
		ClaimRewardOption0Button->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardOption0Clicked);
		ContentBox->AddChildToVerticalBox(ClaimRewardOption0Button);
	}

	if (ClaimRewardOption1Button == nullptr)
	{
		ClaimRewardOption1Button = CreateStageButton(
			TEXT("RewardOverlayClaimOption1Button"),
			TEXT("RewardOverlayClaimOption1ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardClaimOption1Button", "选择奖励 2"),
			ClaimRewardOption1ButtonText);
		ClaimRewardOption1Button->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardOption1Clicked);
		ContentBox->AddChildToVerticalBox(ClaimRewardOption1Button);
	}

	if (ClaimRewardOption2Button == nullptr)
	{
		ClaimRewardOption2Button = CreateStageButton(
			TEXT("RewardOverlayClaimOption2Button"),
			TEXT("RewardOverlayClaimOption2ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardClaimOption2Button", "选择奖励 3"),
			ClaimRewardOption2ButtonText);
		ClaimRewardOption2Button->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleClaimRewardOption2Clicked);
		ContentBox->AddChildToVerticalBox(ClaimRewardOption2Button);
	}

	if (SkipRewardButton == nullptr)
	{
		SkipRewardButton = CreateStageButton(
			TEXT("RewardOverlaySkipButton"),
			TEXT("RewardOverlaySkipButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardSkipButton", "跳过卡牌奖励"),
			SkipRewardButtonText);
		SkipRewardButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleSkipRewardClicked);
		ContentBox->AddChildToVerticalBox(SkipRewardButton);
	}

	if (OpenNodePageButton == nullptr)
	{
		OpenNodePageButton = CreateStageButton(
			TEXT("RewardOverlayNodeButton"),
			TEXT("RewardOverlayNodeButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardNodeButton", "查看节点选择页"),
			OpenNodePageButtonText);
		OpenNodePageButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenNodePageClicked);
		ContentBox->AddChildToVerticalBox(OpenNodePageButton);
	}

	if (OpenModalButton == nullptr)
	{
		OpenModalButton = CreateStageButton(
			TEXT("RewardOverlayModalButton"),
			TEXT("RewardOverlayModalButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardModalButton", "打开奖励说明模态"),
			OpenModalButtonText);
		OpenModalButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleOpenModalClicked);
		ContentBox->AddChildToVerticalBox(OpenModalButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("RewardOverlayCloseButton"),
			TEXT("RewardOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "RewardCloseButton", "关闭奖励页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunRewardOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunRewardOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunPendingBattleRewardViewData& PendingReward = Snapshot.PendingBattleReward;
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RewardOverlayTitleText", "战后奖励页"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardOverlaySummaryText", "待领奖励: {0}\n来源节点: {1}\n节点标签: {2}\n节点类型: {3}\n来源节点Id: {4}\n来源遭遇: {5}\n战斗结果: {6}\n奖励条目数: {7}\n金币汇总: {8}\n奖励可领取: {9}\n流程允许领取: {10}\n当前金币: {11} | 遗物数: {12} | 牌库数: {13}"),
			PendingReward.bHasPendingReward
				? NSLOCTEXT("FinalFlowUI", "RewardHasPendingReward", "是")
				: NSLOCTEXT("FinalFlowUI", "RewardHasNoPendingReward", "否"),
			FormatOptionalText(
				PendingReward.SourceNodeDisplayName,
				FormatOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNodeDisplayName", "未公开显示名"))),
			FormatOptionalName(PendingReward.SourceNodeDisplayLabel, NSLOCTEXT("FinalFlowUI", "RewardNoNodeLabel", "无")),
			FormatNodeTypeText(PendingReward.SourceNodeType),
			FormatOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNode", "无")),
			PendingReward.SourceEncounterId.IsValid()
				? FText::FromName(PendingReward.SourceEncounterId.Value)
				: NSLOCTEXT("FinalFlowUI", "RewardNoEncounter", "无"),
			FormatBattleOutcomeText(PendingReward.SourceBattleOutcome),
			FText::AsNumber(PendingReward.RewardEntryViews.Num() > 0 ? PendingReward.RewardEntryViews.Num() : PendingReward.RewardEntries.Num()),
			FText::AsNumber(PendingReward.RewardGold),
			FormatBool(PendingReward.bCanClaim),
			FormatBool(Progression.bCanClaimPendingBattleReward),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount)));
	}

	if (RewardEntriesText)
	{
		RewardEntriesText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardEntriesText", "奖励条目:\n{0}"),
			FText::FromString(BuildRewardPresentationSummaryString(PendingReward.RewardEntryViews, PendingReward.RewardEntries))));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardOverlayGapText",
			"当前页已优先消费 PendingBattleReward.RewardEntryViews 的 PresentationKind / VisualTier / DetailText / IconId，并在缺失时回退到 raw RewardEntries。当前战后奖励为金币自动入账，卡牌奖励三选一或跳过。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "RewardOverlayFeedbackDefault", "当前页面会把领取奖励意图转发给 RunFlowSubsystem，由它统一决定刷新与切页。")));
	}

	if (ClaimRewardButton)
	{
		const bool bCanClaimReward = PendingReward.bHasPendingReward
			&& PendingReward.bCanClaim
			&& Progression.bCanClaimPendingBattleReward
			&& PendingReward.RewardEntries.Num() == 1;
		ClaimRewardButton->SetIsEnabled(bCanClaimReward);
	}

	if (ClaimRewardButtonText)
	{
		if (!PendingReward.bHasPendingReward)
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButtonDisabled", "当前没有待领奖励"));
		}
		else if (!PendingReward.bCanClaim || !Progression.bCanClaimPendingBattleReward)
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButtonBlocked", "当前待领奖励暂不可领取"));
		}
		else
		{
			ClaimRewardButtonText->SetText(PendingReward.RewardEntries.Num() == 1
				? NSLOCTEXT("FinalFlowUI", "RewardClaimButton", "领取当前奖励条目")
				: NSLOCTEXT("FinalFlowUI", "RewardClaimButtonSelectBelow", "请选择下方一个卡牌奖励"));
		}
	}

	auto ConfigureOptionButton = [&PendingReward, &Progression](const int32 OptionIndex, UButton* Button, UTextBlock* ButtonText)
	{
		const bool bHasOption = PendingReward.RewardEntryViews.IsValidIndex(OptionIndex) || PendingReward.RewardEntries.IsValidIndex(OptionIndex);
		const bool bCanChoose = PendingReward.bHasPendingReward
			&& PendingReward.bCanClaim
			&& Progression.bCanClaimPendingBattleReward
			&& bHasOption;

		if (Button)
		{
			Button->SetIsEnabled(bCanChoose);
		}

		if (ButtonText)
		{
			if (PendingReward.RewardEntryViews.IsValidIndex(OptionIndex))
			{
				const FFinalRunRewardEntryViewData& EntryView = PendingReward.RewardEntryViews[OptionIndex];
				ButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "RewardClaimOptionButtonWithView", "选择 {0}: {1} - {2}"),
					FText::AsNumber(OptionIndex + 1),
					FormatRewardEntryViewPrimaryText(EntryView),
					EntryView.SecondaryText.IsEmpty() ? EntryView.DetailText : EntryView.SecondaryText));
			}
			else if (PendingReward.RewardEntries.IsValidIndex(OptionIndex))
			{
				const FFinalRunRewardEntry& Entry = PendingReward.RewardEntries[OptionIndex];
				ButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "RewardClaimOptionButtonWithEntry", "选择 {0}: {1}"),
					FText::AsNumber(OptionIndex + 1),
					FormatRewardEntryName(Entry)));
			}
			else
			{
				ButtonText->SetText(FText::Format(
					NSLOCTEXT("FinalFlowUI", "RewardClaimOptionButtonMissing", "奖励 {0}: 无候选"),
					FText::AsNumber(OptionIndex + 1)));
			}
		}
	};

	ConfigureOptionButton(0, ClaimRewardOption0Button, ClaimRewardOption0ButtonText);
	ConfigureOptionButton(1, ClaimRewardOption1Button, ClaimRewardOption1ButtonText);
	ConfigureOptionButton(2, ClaimRewardOption2Button, ClaimRewardOption2ButtonText);

	if (SkipRewardButton)
	{
		SkipRewardButton->SetIsEnabled(PendingReward.bHasPendingReward && PendingReward.bCanClaim && Progression.bCanClaimPendingBattleReward);
	}

	if (SkipRewardButtonText)
	{
		SkipRewardButtonText->SetText(PendingReward.bHasPendingReward
			? NSLOCTEXT("FinalFlowUI", "RewardSkipButton", "跳过卡牌奖励")
			: NSLOCTEXT("FinalFlowUI", "RewardSkipButtonDisabled", "当前没有可跳过的卡牌奖励"));
	}

	if (OpenNodePageButton)
	{
		OpenNodePageButton->SetIsEnabled(Progression.bCanAdvanceToNextNode || Progression.CurrentNodeId != NAME_None);
	}
}
