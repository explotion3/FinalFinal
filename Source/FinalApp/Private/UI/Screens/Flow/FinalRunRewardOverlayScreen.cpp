#include "UI/Screens/Flow/FinalRunRewardOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"

namespace
{
FText FormatRewardOptionalName(const FName Name, const FText& Fallback)
{
	return Name != NAME_None ? FText::FromName(Name) : Fallback;
}

FText FormatRewardOptionalText(const FText& Value, const FText& Fallback)
{
	return !Value.IsEmpty() ? Value : Fallback;
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

FText FormatRunNodeTypeText(const EFinalRunNodeType NodeType)
{
	switch (NodeType)
	{
	case EFinalRunNodeType::Battle:
		return NSLOCTEXT("FinalFlowUI", "RewardSourceNodeTypeBattle", "Battle");

	case EFinalRunNodeType::Event:
		return NSLOCTEXT("FinalFlowUI", "RewardSourceNodeTypeEvent", "Event");

	case EFinalRunNodeType::Shop:
		return NSLOCTEXT("FinalFlowUI", "RewardSourceNodeTypeShop", "Shop");

	case EFinalRunNodeType::Reward:
		return NSLOCTEXT("FinalFlowUI", "RewardSourceNodeTypeReward", "Reward");

	case EFinalRunNodeType::EliteBattle:
		return NSLOCTEXT("FinalFlowUI", "RewardSourceNodeTypeEliteBattle", "Elite Battle");

	case EFinalRunNodeType::BossBattle:
		return NSLOCTEXT("FinalFlowUI", "RewardSourceNodeTypeBossBattle", "Boss Battle");

	case EFinalRunNodeType::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RewardSourceNodeTypeNone", "None");
	}
}

FText FormatRewardTypeText(const EFinalRunRewardType RewardType)
{
	switch (RewardType)
	{
	case EFinalRunRewardType::Gold:
		return NSLOCTEXT("FinalFlowUI", "RunRewardTypeGold", "金币");

	case EFinalRunRewardType::CardGrant:
		return NSLOCTEXT("FinalFlowUI", "RunRewardTypeCardGrant", "获得卡牌");

	case EFinalRunRewardType::RelicGrant:
		return NSLOCTEXT("FinalFlowUI", "RunRewardTypeRelicGrant", "获得遗物");

	case EFinalRunRewardType::RemoveCard:
		return NSLOCTEXT("FinalFlowUI", "RunRewardTypeRemoveCard", "移除卡牌");

	case EFinalRunRewardType::UpgradeCard:
		return NSLOCTEXT("FinalFlowUI", "RunRewardTypeUpgradeCard", "强化卡牌");

	case EFinalRunRewardType::Growth:
		return NSLOCTEXT("FinalFlowUI", "RunRewardTypeGrowth", "成长");

	case EFinalRunRewardType::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunRewardTypeNone", "未定义");
	}
}

FText FormatRewardEntryName(const FFinalRunRewardEntry& RewardEntry)
{
	if (!RewardEntry.DisplayName.IsEmpty())
	{
		return RewardEntry.DisplayName;
	}

	if (RewardEntry.DisplayId != NAME_None)
	{
		return FText::FromName(RewardEntry.DisplayId);
	}

	if (RewardEntry.RewardId != NAME_None)
	{
		return FText::FromName(RewardEntry.RewardId);
	}

	return NSLOCTEXT("FinalFlowUI", "RewardEntryUnnamed", "未命名奖励");
}

FText FormatRewardClaimStateText(const FFinalRunRewardEntry& RewardEntry)
{
	if (RewardEntry.bClaimed)
	{
		return NSLOCTEXT("FinalFlowUI", "RewardEntryClaimed", "已领取");
	}

	if (RewardEntry.bCanClaim)
	{
		return NSLOCTEXT("FinalFlowUI", "RewardEntryCanClaim", "可领取");
	}

	return NSLOCTEXT("FinalFlowUI", "RewardEntryCannotClaim", "暂不可领取");
}

FString BuildRewardEntriesSummary(const TArray<FFinalRunRewardEntry>& RewardEntries)
{
	if (RewardEntries.Num() <= 0)
	{
		return NSLOCTEXT("FinalFlowUI", "RewardEntriesEmpty", "当前没有公开的结构化奖励条目。").ToString();
	}

	FString RewardEntrySummary;
	for (int32 Index = 0; Index < RewardEntries.Num(); ++Index)
	{
		const FFinalRunRewardEntry& RewardEntry = RewardEntries[Index];
		RewardEntrySummary += FString::Printf(
			TEXT("[%d] %s | 类型: %s | 数值: %d | 可领取: %s | 状态: %s"),
			Index + 1,
			*FormatRewardEntryName(RewardEntry).ToString(),
			*FormatRewardTypeText(RewardEntry.RewardType).ToString(),
			RewardEntry.Value,
			RewardEntry.bCanClaim ? TEXT("是") : TEXT("否"),
			*FormatRewardClaimStateText(RewardEntry).ToString());

		if (RewardEntry.RewardId != NAME_None)
		{
			RewardEntrySummary += FString::Printf(TEXT(" | RewardId: %s"), *RewardEntry.RewardId.ToString());
		}

		RewardEntrySummary += TEXT("\n");
	}

	RewardEntrySummary.TrimEndInline();
	return RewardEntrySummary;
}

UTextBlock* CreateRewardOverlayLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

UFinalRunFlowSubsystem* ResolveRewardOverlayRunFlowSubsystem(UUserWidget* Widget)
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

	return GameInstance->GetSubsystem<UFinalRunFlowSubsystem>();
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
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRewardOverlayRunFlowSubsystem(this);
	if (RunFlowSubsystem == nullptr)
	{
		LastActionFeedback = NSLOCTEXT("FinalFlowUI", "RewardNoRunFlowSubsystem", "当前无法访问 RunFlowSubsystem，无法领取待领奖励。");
		RebuildVisual();
		return;
	}

	const bool bClaimed = RunFlowSubsystem->ClaimPendingBattleReward();
	CachedSnapshot = RunFlowSubsystem->GetCurrentRunSnapshot();
	LastActionFeedback = !RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bClaimed
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

	RewardEntriesText = CreateRewardOverlayLabel(WidgetTree, TEXT("RewardOverlayEntries"), 13);
	ContentBox->AddChildToVerticalBox(RewardEntriesText);

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
			NSLOCTEXT("FinalFlowUI", "RewardOverlaySummaryText", "待领奖励: {0}\n来源节点: {1}\n节点标签: {2}\n节点类型: {3}\n来源节点Id: {4}\n来源遭遇: {5}\n战斗结果: {6}\n奖励条目数: {7}\n金币汇总: {8}\n奖励可领取: {9}\n流程允许领取: {10}\n当前金币: {11} | 遗物数: {12} | 牌库数: {13}"),
			PendingReward.bHasPendingReward
				? NSLOCTEXT("FinalFlowUI", "RewardHasPendingReward", "是")
				: NSLOCTEXT("FinalFlowUI", "RewardHasNoPendingReward", "否"),
			FormatRewardOptionalText(
				PendingReward.SourceNodeDisplayName,
				FormatRewardOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNodeDisplayName", "未公开显示名"))),
			FormatRewardOptionalName(PendingReward.SourceNodeDisplayLabel, NSLOCTEXT("FinalFlowUI", "RewardNoNodeLabel", "无")),
			FormatRunNodeTypeText(PendingReward.SourceNodeType),
			FormatRewardOptionalName(PendingReward.SourceNodeId, NSLOCTEXT("FinalFlowUI", "RewardNoNode", "无")),
			PendingReward.SourceEncounterId.IsValid()
				? FText::FromName(PendingReward.SourceEncounterId.Value)
				: NSLOCTEXT("FinalFlowUI", "RewardNoEncounter", "无"),
			FormatBattleOutcomeText(PendingReward.SourceBattleOutcome),
			FText::AsNumber(PendingReward.RewardEntries.Num()),
			FText::AsNumber(PendingReward.RewardGold),
			PendingReward.bCanClaim
				? NSLOCTEXT("FinalFlowUI", "RewardEntrySetCanClaim", "是")
				: NSLOCTEXT("FinalFlowUI", "RewardEntrySetCannotClaim", "否"),
			Progression.bCanClaimPendingBattleReward
				? NSLOCTEXT("FinalFlowUI", "RewardCanClaim", "是")
				: NSLOCTEXT("FinalFlowUI", "RewardCannotClaim", "否"),
			FText::AsNumber(CachedSnapshot.Gold),
			FText::AsNumber(CachedSnapshot.RelicCount),
			FText::AsNumber(CachedSnapshot.DeckCount)));
	}

	if (RewardEntriesText)
	{
		RewardEntriesText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RewardEntriesText", "奖励条目:\n{0}"),
			FText::FromString(BuildRewardEntriesSummary(CachedSnapshot.PendingBattleReward.RewardEntries))));
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"RewardOverlayGapText",
			"当前页已真实消费 PendingBattleReward.RewardEntries、来源节点显示字段与可领取状态。剩余缺口主要是奖励图标/稀有度/描述等 richer 呈现，以及多奖励选择、替换、跳过这类更复杂流程。"));
	}

	if (FeedbackText)
	{
		if (!LastActionFeedback.IsEmpty())
		{
			FeedbackText->SetText(LastActionFeedback);
		}
		else if (!CachedSnapshot.Progression.CurrentNodeStateMessage.IsEmpty())
		{
			FeedbackText->SetText(CachedSnapshot.Progression.CurrentNodeStateMessage);
		}
		else
		{
			FeedbackText->SetText(NSLOCTEXT("FinalFlowUI", "RewardOverlayFeedbackDefault", "当前页面会把领取奖励意图转发给 RunFlowSubsystem，由它统一决定刷新与切页。"));
		}
	}

	if (ClaimRewardButton)
	{
		const bool bCanClaimReward = CachedSnapshot.PendingBattleReward.bHasPendingReward
			&& CachedSnapshot.PendingBattleReward.bCanClaim
			&& CachedSnapshot.Progression.bCanClaimPendingBattleReward;
		ClaimRewardButton->SetIsEnabled(bCanClaimReward);
	}

	if (ClaimRewardButtonText)
	{
		if (!CachedSnapshot.PendingBattleReward.bHasPendingReward)
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButtonDisabled", "当前没有待领奖励"));
		}
		else if (!CachedSnapshot.PendingBattleReward.bCanClaim || !CachedSnapshot.Progression.bCanClaimPendingBattleReward)
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButtonBlocked", "当前待领奖励暂不可领取"));
		}
		else
		{
			ClaimRewardButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RewardClaimButton", "领取当前奖励条目"));
		}
	}

	if (OpenNodePageButton)
	{
		OpenNodePageButton->SetIsEnabled(CachedSnapshot.Progression.bCanAdvanceToNextNode || CachedSnapshot.Progression.CurrentNodeId != NAME_None);
	}
}
