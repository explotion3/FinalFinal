#include "UI/Screens/Flow/FinalRunFlowOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"

using namespace FinalRunFlowScreenUtils;

namespace
{
FText FormatRewardOptionText(const FFinalRunPendingBattleRewardViewData& PendingReward, const int32 RewardIndex)
{
	if (PendingReward.RewardEntryViews.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntryViewData& RewardView = PendingReward.RewardEntryViews[RewardIndex];
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionView", "选择卡牌 {0}：{1}"),
			FText::AsNumber(RewardIndex + 1),
			FormatRewardEntryViewPrimaryText(RewardView));
	}

	if (PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntry& RewardEntry = PendingReward.RewardEntries[RewardIndex];
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionRaw", "选择卡牌 {0}：{1}"),
			FText::AsNumber(RewardIndex + 1),
			FormatRewardEntryName(RewardEntry));
	}

	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionMissing", "卡牌候选 {0}: 无"),
		FText::AsNumber(RewardIndex + 1));
}

FText BuildNextNodeSelectionText(const FFinalRunNodeOptionViewData& Node)
{
	const FText NodeName = FormatRunNodeDisplayName(Node.DisplayName, Node.NodeId, Node.NodeType);
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeSelection", "{0} · {1}\n第 {2} 章 / 第 {3} 层\n{4}"),
		NodeName,
		FormatNodeTypeText(Node.NodeType),
		FText::AsNumber(Node.ChapterIndex),
		FText::AsNumber(Node.FloorIndex),
		FormatOptionalText(Node.AvailabilityMessage, Node.bLocked
			? NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeLocked", "不可前往")
			: NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeAvailable", "可前往")));
}

FText BuildRewardOptionMetaText(const FFinalRunPendingBattleRewardViewData& PendingReward, const int32 RewardIndex)
{
	if (PendingReward.RewardEntryViews.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntryViewData& RewardView = PendingReward.RewardEntryViews[RewardIndex];
		if (!RewardView.DetailText.IsEmpty())
		{
			return RewardView.DetailText;
		}
		if (!RewardView.SecondaryText.IsEmpty())
		{
			return RewardView.SecondaryText;
		}
		if (RewardView.Value != 0)
		{
			return FText::Format(
				NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionValueMeta", "数值：{0}"),
				FText::AsNumber(RewardView.Value));
		}
	}

	if (PendingReward.RewardEntries.IsValidIndex(RewardIndex) && PendingReward.RewardEntries[RewardIndex].Value != 0)
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionRawValueMeta", "数值：{0}"),
			FText::AsNumber(PendingReward.RewardEntries[RewardIndex].Value));
	}

	return FText::GetEmpty();
}

FFinalRunFlowOptionButtonData BuildRewardOptionData(const FFinalRunPendingBattleRewardViewData& PendingReward, const int32 RewardIndex)
{
	FFinalRunFlowOptionButtonData Data;
	Data.Kind = EFinalRunFlowOptionKind::Reward;
	Data.PayloadIndex = RewardIndex;
	Data.bEnabled = PendingReward.bCanClaim;

	if (PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		Data.PayloadId = PendingReward.RewardEntries[RewardIndex].RewardId;
	}

	if (PendingReward.RewardEntryViews.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntryViewData& RewardView = PendingReward.RewardEntryViews[RewardIndex];
		Data.Title = FormatRewardEntryViewPrimaryText(RewardView);
		Data.Subtitle = FormatRewardTypeText(RewardView.RewardType);
	}
	else if (PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		const FFinalRunRewardEntry& RewardEntry = PendingReward.RewardEntries[RewardIndex];
		Data.Title = FormatRewardEntryName(RewardEntry);
		Data.Subtitle = FormatRewardTypeText(RewardEntry.RewardType);
	}
	else
	{
		Data.Title = FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionTitleMissing", "卡牌候选 {0}"),
			FText::AsNumber(RewardIndex + 1));
		Data.Subtitle = NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionSubtitleMissing", "战后卡牌候选");
	}

	Data.Meta = BuildRewardOptionMetaText(PendingReward, RewardIndex);
	Data.State = Data.bEnabled
		? NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionStateClaimable", "可领取")
		: NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionStateBlocked", "暂不可领取");
	return Data;
}

FFinalRunFlowOptionButtonData BuildNextNodeOptionData(const FFinalRunNodeOptionViewData& Node, const int32 NodeIndex, const bool bCanAdvance)
{
	FFinalRunFlowOptionButtonData Data;
	Data.Kind = EFinalRunFlowOptionKind::NextNode;
	Data.PayloadId = Node.NodeId;
	Data.PayloadIndex = NodeIndex;
	Data.Title = FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeOptionTitle", "前往：{0}"),
		FormatRunNodeDisplayName(Node.DisplayName, Node.NodeId, Node.NodeType));
	Data.Subtitle = FormatNodeTypeText(Node.NodeType);
	Data.Meta = FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeOptionMeta", "第 {0} 章 / 第 {1} 层"),
		FText::AsNumber(Node.ChapterIndex),
		FText::AsNumber(Node.FloorIndex));
	Data.bEnabled = bCanAdvance && !Node.bLocked;
	Data.State = Data.bEnabled
		? NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeOptionStateAvailable", "可前往")
		: FormatOptionalText(Node.AvailabilityMessage, NSLOCTEXT("FinalFlowUI", "RunFlowNextNodeOptionStateBlocked", "暂不可前往"));
	return Data;
}

FFinalRunFlowOptionButtonData BuildEventOptionData(const FFinalRunEventOptionViewData& Option, const int32 OptionIndex, const bool bCanResolve)
{
	FFinalRunFlowOptionButtonData Data;
	Data.Kind = EFinalRunFlowOptionKind::EventOption;
	Data.PayloadId = Option.OptionId;
	Data.PayloadIndex = OptionIndex;
	Data.Title = FormatOptionalText(Option.DisplayText, FormatOptionalName(Option.OptionId, NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionTitleFallback", "未命名选项")));
	Data.Subtitle = FormatOptionalText(Option.OutcomeSummary, NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionSubtitleFallback", "无额外结果说明。"));
	Data.Meta = FText::FromString(BuildRewardPresentationSummaryString(Option.RewardEntryViews, Option.RewardEntries));
	Data.bEnabled = bCanResolve && Option.bSelectable;
	Data.State = Data.bEnabled
		? NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionStateSelectable", "可选择")
		: FormatOptionalText(Option.AvailabilityMessage, NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionStateBlocked", "暂不可选择"));
	return Data;
}

FFinalRunFlowOptionButtonData BuildShopOfferOptionData(const FFinalRunShopOfferViewData& Offer, const int32 OfferIndex, const bool bCanResolve)
{
	FFinalRunFlowOptionButtonData Data;
	Data.Kind = EFinalRunFlowOptionKind::ShopOffer;
	Data.PayloadId = Offer.OfferId;
	Data.PayloadIndex = OfferIndex;
	Data.Title = FormatOptionalText(Offer.DisplayName, FormatOptionalName(Offer.OfferId, NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferTitleFallback", "未命名商品")));
	Data.Subtitle = FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferSubtitle", "价格：{0}"),
		FText::AsNumber(Offer.Price));
	Data.Meta = !Offer.Description.IsEmpty()
		? Offer.Description
		: FText::FromString(BuildRewardPresentationSummaryString(Offer.RewardEntryViews, Offer.RewardEntries));
	Data.bEnabled = bCanResolve && Offer.bPurchasable && !Offer.bPurchased;
	Data.State = Offer.bPurchased
		? NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferStatePurchased", "已购买")
		: (Data.bEnabled
			? NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferStatePurchasable", "可购买")
			: FormatOptionalText(Offer.AvailabilityMessage, NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferStateBlocked", "暂不可购买")));
	return Data;
}

FFinalRunFlowOptionButtonData BuildFlowActionOptionData(const FFinalRunFlowActionViewData& Action)
{
	FFinalRunFlowOptionButtonData Data;
	Data.Kind = EFinalRunFlowOptionKind::FlowAction;
	Data.CommandType = Action.CommandType;
	Data.PayloadId = Action.PayloadId;
	Data.Title = Action.DisplayText;
	Data.Subtitle = Action.Description;
	Data.bEnabled = Action.bEnabled;
	Data.State = Action.bEnabled
		? NSLOCTEXT("FinalFlowUI", "RunFlowActionStateEnabled", "可执行")
		: FormatOptionalText(Action.DisabledReason, NSLOCTEXT("FinalFlowUI", "RunFlowActionStateDisabled", "暂不可执行"));
	return Data;
}

FText BuildEventOptionSelectionText(const FFinalRunEventOptionViewData& Option)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionSelection", "{0}\n结果：{1}\n状态：{2}\n奖励：\n{3}"),
		FormatOptionalText(Option.DisplayText, FormatOptionalName(Option.OptionId, NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionUnnamed", "未命名选项"))),
		FormatOptionalText(Option.OutcomeSummary, NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionNoOutcome", "无额外结果说明。")),
		FormatOptionalText(Option.AvailabilityMessage, Option.bSelectable
			? NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionSelectable", "可选择")
			: NSLOCTEXT("FinalFlowUI", "RunFlowEventOptionBlocked", "不可选择")),
		FText::FromString(BuildRewardPresentationSummaryString(Option.RewardEntryViews, Option.RewardEntries)));
}

FText BuildShopOfferSelectionText(const FFinalRunShopOfferViewData& Offer)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferSelection", "{0}\n价格：{1}\n说明：{2}\n状态：{3}\n奖励：\n{4}"),
		FormatOptionalText(Offer.DisplayName, FormatOptionalName(Offer.OfferId, NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferUnnamed", "未命名商品"))),
		FText::AsNumber(Offer.Price),
		FormatOptionalText(Offer.Description, NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferNoDescription", "无额外说明。")),
		FormatOptionalText(Offer.AvailabilityMessage, Offer.bPurchasable
			? NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferPurchasable", "可购买")
			: NSLOCTEXT("FinalFlowUI", "RunFlowShopOfferBlocked", "不可购买")),
		FText::FromString(BuildRewardPresentationSummaryString(Offer.RewardEntryViews, Offer.RewardEntries)));
}

FText BuildCompactCurrentNodeText(const FFinalRunProgressionViewData& Progression)
{
	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowCompactCurrentNode", "当前节点：{0} · {1}\n第 {2} 章 / 第 {3} 层"),
		FormatRunNodeDisplayName(Progression.CurrentNodeDisplayName, Progression.CurrentNodeId, Progression.CurrentNodeType),
		FormatNodeTypeText(Progression.CurrentNodeType),
		FText::AsNumber(Progression.CurrentChapter),
		FText::AsNumber(Progression.CurrentFloor));
}

FText FormatRouteNodeStateText(const FFinalRunRouteNodeViewData& Node)
{
	if (Node.bCurrent)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeStateCurrent", "当前");
	}
	if (Node.bResolved)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeStateResolved", "已解决");
	}
	if (Node.bVisited)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeStateVisited", "已访问");
	}
	if (Node.bLocked)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeStateLocked", "锁定");
	}
	if (Node.bReachable)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeStateReachable", "可前往");
	}
	if (Node.bNeedsResolution)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeStateNeedsResolution", "待解决");
	}

	return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeStateFuture", "未到达");
}

FText BuildRouteNodeLabelText(const FFinalRunRouteNodeViewData& Node)
{
	const FText NodeName = FormatRunNodeDisplayName(Node.DisplayName, Node.NodeId, Node.NodeType);
	if (!Node.DisplayLabel.IsNone())
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeLabelWithDisplayLabel", "{0} · {1}"),
			FText::FromName(Node.DisplayLabel),
			NodeName);
	}

	return NodeName;
}

FText BuildRouteNodeAvailabilityText(const FFinalRunRouteNodeViewData& Node)
{
	if (!Node.AvailabilityMessage.IsEmpty())
	{
		return Node.AvailabilityMessage;
	}

	if (!Node.bHasImplementedResolver && Node.bNeedsResolution)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeResolverMissing", "当前节点解析器尚未实现。");
	}

	if (Node.bReachable)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeReachableMessage", "当前可达。");
	}

	return FText::GetEmpty();
}
}

void UFinalRunRouteNodeEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RefreshBoundWidgets();
}

void UFinalRunRouteNodeEntryWidget::ApplyRouteNodeView(const FFinalRunRouteNodeViewData& InViewData)
{
	CachedViewData = InViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnRouteNodeViewApplied(CachedViewData);
}

void UFinalRunRouteNodeEntryWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RouteNodeEntryRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.08f, 0.09f, 0.08f, 0.88f));
	RootBorder->SetPadding(FMargin(8.0f));
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* TextBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RouteNodeEntryTextBox"));
	RootBorder->SetContent(TextBox);

	NodeLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NodeLabelText"));
	NodeLabelText->SetAutoWrapText(true);
	NodeLabelText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 12));
	TextBox->AddChildToVerticalBox(NodeLabelText);

	NodeTypeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NodeTypeText"));
	NodeTypeText->SetAutoWrapText(true);
	NodeTypeText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(NodeTypeText);

	StateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
	StateText->SetAutoWrapText(true);
	StateText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(StateText);

	AvailabilityText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AvailabilityText"));
	AvailabilityText->SetAutoWrapText(true);
	AvailabilityText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(AvailabilityText);
}

void UFinalRunRouteNodeEntryWidget::RefreshBoundWidgets()
{
	if (NodeLabelText)
	{
		NodeLabelText->SetText(BuildRouteNodeLabelText(CachedViewData));
	}
	if (NodeTypeText)
	{
		NodeTypeText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRouteNodeTypeLine", "{0} | 第 {1} 章 / 第 {2} 层"),
			FormatNodeTypeText(CachedViewData.NodeType),
			FText::AsNumber(CachedViewData.ChapterIndex),
			FText::AsNumber(CachedViewData.FloorIndex)));
	}
	if (StateText)
	{
		StateText->SetText(FormatRouteNodeStateText(CachedViewData));
	}
	if (AvailabilityText)
	{
		const FText Availability = BuildRouteNodeAvailabilityText(CachedViewData);
		AvailabilityText->SetText(Availability);
		AvailabilityText->SetVisibility(Availability.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	if (CurrentVisual)
	{
		CurrentVisual->SetVisibility(CachedViewData.bCurrent ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (VisitedVisual)
	{
		VisitedVisual->SetVisibility(CachedViewData.bVisited ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (LockedVisual)
	{
		LockedVisual->SetVisibility(CachedViewData.bLocked ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (ResolvedVisual)
	{
		ResolvedVisual->SetVisibility(CachedViewData.bResolved ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UFinalRunFlowOptionButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	if (OptionButton)
	{
		OptionButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOptionButton::HandleClicked);
	}
}

void UFinalRunFlowOptionButton::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	if (SelectedVisual)
	{
		SelectedVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UFinalRunFlowOptionButton::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	if (SelectedVisual)
	{
		SelectedVisual->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalRunFlowOptionButton::ConfigureOption(
	const EFinalRunFlowOptionKind InKind,
	const FName InPayloadId,
	const int32 InPayloadIndex,
	const FText& InLabel,
	const bool bInEnabled)
{
	FFinalRunFlowOptionButtonData Data;
	Data.Kind = InKind;
	Data.PayloadId = InPayloadId;
	Data.PayloadIndex = InPayloadIndex;
	Data.Title = InLabel;
	Data.bEnabled = bInEnabled;
	Data.State = bInEnabled
		? NSLOCTEXT("FinalFlowUI", "RunFlowOptionCompatStateEnabled", "可选择")
		: NSLOCTEXT("FinalFlowUI", "RunFlowOptionCompatStateDisabled", "不可选择");
	ConfigureOption(Data);
}

void UFinalRunFlowOptionButton::ConfigureOption(const FFinalRunFlowOptionButtonData& InData)
{
	CachedData = InData;
	OptionKind = InData.Kind;
	PayloadId = InData.PayloadId;
	PayloadIndex = InData.PayloadIndex;

	EnsureWidgetTree();
	const FText CombinedFallbackText = !InData.Subtitle.IsEmpty() || !InData.Meta.IsEmpty() || !InData.State.IsEmpty()
		? FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowOptionCombinedFallback", "{0}\n{1}\n{2}\n{3}"),
			InData.Title,
			InData.Subtitle,
			InData.Meta,
			InData.State)
		: InData.Title;
	if (OptionLabel)
	{
		OptionLabel->SetText(CombinedFallbackText);
	}
	if (TitleText)
	{
		TitleText->SetText(InData.Title);
	}
	if (SubtitleText)
	{
		SubtitleText->SetText(InData.Subtitle);
		SubtitleText->SetVisibility(InData.Subtitle.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (MetaText)
	{
		MetaText->SetText(InData.Meta);
		MetaText->SetVisibility(InData.Meta.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (StateText)
	{
		StateText->SetText(InData.State);
		StateText->SetVisibility(InData.State.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (OptionButton)
	{
		OptionButton->SetIsEnabled(InData.bEnabled);
	}
	if (SelectedVisual)
	{
		SelectedVisual->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalRunFlowOptionButton::HandleClicked()
{
	OnOptionClicked.Broadcast(this);
}

UWidget* UFinalRunFlowOptionButton::GetFocusTarget() const
{
	return OptionButton;
}

void UFinalRunFlowOptionButton::EnsureWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}
	if (WidgetTree->RootWidget != nullptr)
	{
		if (OptionButton == nullptr)
		{
			OptionButton = Cast<UButton>(WidgetTree->RootWidget);
		}
		return;
	}

	OptionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OptionButton"));
	UVerticalBox* TextBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OptionTextBox"));

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetAutoWrapText(true);
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 13));
	TextBox->AddChildToVerticalBox(TitleText);

	SubtitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubtitleText"));
	SubtitleText->SetAutoWrapText(true);
	SubtitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(SubtitleText);

	MetaText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MetaText"));
	MetaText->SetAutoWrapText(true);
	MetaText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(MetaText);

	StateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
	StateText->SetAutoWrapText(true);
	StateText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(StateText);

	OptionLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OptionLabel"));
	OptionLabel->SetVisibility(ESlateVisibility::Collapsed);
	OptionLabel->SetAutoWrapText(true);
	OptionLabel->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12));
	TextBox->AddChildToVerticalBox(OptionLabel);

	OptionButton->AddChild(TextBox);
	WidgetTree->RootWidget = OptionButton;
}

void UFinalRunFlowOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	ClampSelectionIndices();
	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::HandleRewardOption0Clicked()
{
	HandleRewardOptionClicked(0);
}

void UFinalRunFlowOverlayScreen::HandleRewardOption1Clicked()
{
	HandleRewardOptionClicked(1);
}

void UFinalRunFlowOverlayScreen::HandleRewardOption2Clicked()
{
	HandleRewardOptionClicked(2);
}

void UFinalRunFlowOverlayScreen::HandlePreviousChoiceClicked()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (Snapshot.Progression.AvailableNextNodes.Num() > 0)
		{
			SelectedNextNodeIndex = SelectedNextNodeIndex == INDEX_NONE
				? 0
				: (SelectedNextNodeIndex - 1 + Snapshot.Progression.AvailableNextNodes.Num()) % Snapshot.Progression.AvailableNextNodes.Num();
		}
		break;

	case EFinalRunFlowStage::PendingEventNode:
		if (Snapshot.PendingEventNode.Options.Num() > 0)
		{
			SelectedEventOptionIndex = SelectedEventOptionIndex == INDEX_NONE
				? 0
				: (SelectedEventOptionIndex - 1 + Snapshot.PendingEventNode.Options.Num()) % Snapshot.PendingEventNode.Options.Num();
		}
		break;

	case EFinalRunFlowStage::PendingShopNode:
		if (Snapshot.PendingShopNode.Offers.Num() > 0)
		{
			SelectedShopOfferIndex = SelectedShopOfferIndex == INDEX_NONE
				? 0
				: (SelectedShopOfferIndex - 1 + Snapshot.PendingShopNode.Offers.Num()) % Snapshot.PendingShopNode.Offers.Num();
		}
		break;

	default:
		break;
	}

	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::HandleNextChoiceClicked()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (Snapshot.Progression.AvailableNextNodes.Num() > 0)
		{
			SelectedNextNodeIndex = SelectedNextNodeIndex == INDEX_NONE
				? 0
				: (SelectedNextNodeIndex + 1) % Snapshot.Progression.AvailableNextNodes.Num();
		}
		break;

	case EFinalRunFlowStage::PendingEventNode:
		if (Snapshot.PendingEventNode.Options.Num() > 0)
		{
			SelectedEventOptionIndex = SelectedEventOptionIndex == INDEX_NONE
				? 0
				: (SelectedEventOptionIndex + 1) % Snapshot.PendingEventNode.Options.Num();
		}
		break;

	case EFinalRunFlowStage::PendingShopNode:
		if (Snapshot.PendingShopNode.Offers.Num() > 0)
		{
			SelectedShopOfferIndex = SelectedShopOfferIndex == INDEX_NONE
				? 0
				: (SelectedShopOfferIndex + 1) % Snapshot.PendingShopNode.Offers.Num();
		}
		break;

	default:
		break;
	}

	RebuildVisual();
}

void UFinalRunFlowOverlayScreen::HandlePrimaryActionClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowMissingSubsystem", "当前无法访问 RunFlowSubsystem。"));
		RebuildVisual();
		return;
	}

	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	bool bAccepted = false;
	FText SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionSucceeded", "操作已提交。");
	FText FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionFailed", "操作提交失败。");

	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			bAccepted = RunFlowSubsystem->AdvanceToNode(SelectedNode->NodeId);
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceSucceeded", "已推进到选中节点。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceFailed", "推进节点失败。");
		}
		break;

	case EFinalRunFlowStage::PendingRewardNode:
		bAccepted = RunFlowSubsystem->ResolveRewardNode();
		SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveRewardSucceeded", "已确认奖励节点。");
		FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveRewardFailed", "确认奖励节点失败。");
		break;

	case EFinalRunFlowStage::PendingEventNode:
		if (const FFinalRunEventOptionViewData* SelectedOption = GetSelectedEventOption())
		{
			bAccepted = RunFlowSubsystem->ResolveEventOption(SelectedOption->OptionId);
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveEventSucceeded", "已提交事件选项。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveEventFailed", "提交事件选项失败。");
		}
		break;

	case EFinalRunFlowStage::PendingShopNode:
		if (const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedShopOffer())
		{
			bAccepted = RunFlowSubsystem->ResolveShopOffer(SelectedOffer->OfferId);
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveShopSucceeded", "已提交商店商品。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowResolveShopFailed", "提交商店商品失败。");
		}
		break;

	default:
		break;
	}

	RefreshAfterFlowAction(bAccepted, SuccessText, FailureText);
}

void UFinalRunFlowOverlayScreen::HandleSecondaryActionClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowMissingSubsystemSecondary", "当前无法访问 RunFlowSubsystem。"));
		RebuildVisual();
		return;
	}

	if (GetCachedSnapshot().Progression.FlowStage != EFinalRunFlowStage::PendingBattleReward)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowSecondaryUnavailable", "当前阶段没有可用的次要操作。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->SkipPendingBattleReward();
	RefreshAfterFlowAction(
		bAccepted,
		NSLOCTEXT("FinalFlowUI", "RunFlowSkipRewardSucceeded", "已跳过战后卡牌奖励。"),
		NSLOCTEXT("FinalFlowUI", "RunFlowSkipRewardFailed", "跳过战后卡牌奖励失败。"));
}

void UFinalRunFlowOverlayScreen::HandleCloseClicked()
{
	RequestCloseOverlay();
}

void UFinalRunFlowOverlayScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (WidgetTree->RootWidget == nullptr)
	{
		UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RunFlowOverlayRoot"));
		RootOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = RootOverlay;

		USizeBox* PanelSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RunFlowOverlayPanelSizeBox"));
		PanelSizeBox->SetWidthOverride(520.0f);

		UOverlaySlot* PanelSlot = RootOverlay->AddChildToOverlay(PanelSizeBox);
		if (PanelSlot != nullptr)
		{
			PanelSlot->SetHorizontalAlignment(HAlign_Right);
			PanelSlot->SetVerticalAlignment(VAlign_Fill);
			PanelSlot->SetPadding(FMargin(0.0f, 72.0f, 24.0f, 72.0f));
		}

		UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RunFlowOverlayPanel"));
		PanelBorder->SetBrushColor(FLinearColor(0.035f, 0.04f, 0.035f, 0.92f));
		PanelBorder->SetPadding(FMargin(18.0f));
		PanelSizeBox->SetContent(PanelBorder);

		UScrollBox* ScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("RunFlowOverlayScrollBox"));
		PanelBorder->SetContent(ScrollBox);

		ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RunFlowOverlayContent"));
		ScrollBox->AddChild(ContentBox);

		TitleText = CreateStageLabel(TEXT("RunFlowOverlayTitle"), 22);
		ContentBox->AddChildToVerticalBox(TitleText);

		SummaryText = CreateStageLabel(TEXT("RunFlowOverlaySummary"), 14);
		ContentBox->AddChildToVerticalBox(SummaryText);

		CurrentStageText = CreateStageLabel(TEXT("CurrentStageText"), 13);
		ContentBox->AddChildToVerticalBox(CurrentStageText);

		CurrentNodeText = CreateStageLabel(TEXT("RunFlowCurrentNode"), 13);
		ContentBox->AddChildToVerticalBox(CurrentNodeText);

		RouteSummaryText = CreateStageLabel(TEXT("RouteSummaryText"), 12);
		ContentBox->AddChildToVerticalBox(RouteSummaryText);

		RouteNodeListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RouteNodeListBox"));
		ContentBox->AddChildToVerticalBox(RouteNodeListBox);

		StageDetailText = CreateStageLabel(TEXT("RunFlowStageDetail"), 13);
		ContentBox->AddChildToVerticalBox(StageDetailText);

		SelectionText = CreateStageLabel(TEXT("RunFlowSelection"), 13);
		ContentBox->AddChildToVerticalBox(SelectionText);

		ActionListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ActionListBox"));
		ContentBox->AddChildToVerticalBox(ActionListBox);

		RewardOptionListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardOptionListBox"));
		ContentBox->AddChildToVerticalBox(RewardOptionListBox);

		NextNodeListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NextNodeListBox"));
		ContentBox->AddChildToVerticalBox(NextNodeListBox);

		EventOptionListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EventOptionListBox"));
		ContentBox->AddChildToVerticalBox(EventOptionListBox);

		ShopOfferListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ShopOfferListBox"));
		ContentBox->AddChildToVerticalBox(ShopOfferListBox);

		FeedbackText = CreateStageLabel(TEXT("RunFlowOverlayFeedback"), 12);
		ContentBox->AddChildToVerticalBox(FeedbackText);
	}

	const bool bCanCreateFallbackChildren = ContentBox != nullptr;

	if (RewardOption0Button == nullptr && bCanCreateFallbackChildren)
	{
		RewardOption0Button = CreateStageButton(
			TEXT("RunFlowRewardOption0Button"),
			TEXT("RunFlowRewardOption0ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOption0", "选择卡牌 1"),
			RewardOption0ButtonText);
		ContentBox->AddChildToVerticalBox(RewardOption0Button);
	}

	if (RewardOption1Button == nullptr && bCanCreateFallbackChildren)
	{
		RewardOption1Button = CreateStageButton(
			TEXT("RunFlowRewardOption1Button"),
			TEXT("RunFlowRewardOption1ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOption1", "选择卡牌 2"),
			RewardOption1ButtonText);
		ContentBox->AddChildToVerticalBox(RewardOption1Button);
	}

	if (RewardOption2Button == nullptr && bCanCreateFallbackChildren)
	{
		RewardOption2Button = CreateStageButton(
			TEXT("RunFlowRewardOption2Button"),
			TEXT("RunFlowRewardOption2ButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardOption2", "选择卡牌 3"),
			RewardOption2ButtonText);
		ContentBox->AddChildToVerticalBox(RewardOption2Button);
	}

	if (PreviousChoiceButton == nullptr && bCanCreateFallbackChildren)
	{
		PreviousChoiceButton = CreateStageButton(
			TEXT("RunFlowPreviousChoiceButton"),
			TEXT("RunFlowPreviousChoiceButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowPreviousChoice", "上一个"),
			PreviousChoiceButtonText);
		ContentBox->AddChildToVerticalBox(PreviousChoiceButton);
	}

	if (NextChoiceButton == nullptr && bCanCreateFallbackChildren)
	{
		NextChoiceButton = CreateStageButton(
			TEXT("RunFlowNextChoiceButton"),
			TEXT("RunFlowNextChoiceButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowNextChoice", "下一个"),
			NextChoiceButtonText);
		ContentBox->AddChildToVerticalBox(NextChoiceButton);
	}

	if (PrimaryActionButton == nullptr && bCanCreateFallbackChildren)
	{
		PrimaryActionButton = CreateStageButton(
			TEXT("RunFlowPrimaryActionButton"),
			TEXT("RunFlowPrimaryActionButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowPrimaryAction", "执行当前操作"),
			PrimaryActionButtonText);
		ContentBox->AddChildToVerticalBox(PrimaryActionButton);
	}

	if (SecondaryActionButton == nullptr && bCanCreateFallbackChildren)
	{
		SecondaryActionButton = CreateStageButton(
			TEXT("RunFlowSecondaryActionButton"),
			TEXT("RunFlowSecondaryActionButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowSecondaryAction", "跳过"),
			SecondaryActionButtonText);
		ContentBox->AddChildToVerticalBox(SecondaryActionButton);
	}

	if (CloseButton == nullptr && bCanCreateFallbackChildren)
	{
		CloseButton = CreateStageButton(
			TEXT("RunFlowCloseButton"),
			TEXT("RunFlowCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "RunFlowCloseAction", "关闭"),
			CloseButtonText);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}

	if (RewardOption0Button)
	{
		RewardOption0Button->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleRewardOption0Clicked);
	}
	if (RewardOption1Button)
	{
		RewardOption1Button->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleRewardOption1Clicked);
	}
	if (RewardOption2Button)
	{
		RewardOption2Button->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleRewardOption2Clicked);
	}
	if (PreviousChoiceButton)
	{
		PreviousChoiceButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandlePreviousChoiceClicked);
	}
	if (NextChoiceButton)
	{
		NextChoiceButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleNextChoiceClicked);
	}
	if (PrimaryActionButton)
	{
		PrimaryActionButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandlePrimaryActionClicked);
	}
	if (SecondaryActionButton)
	{
		SecondaryActionButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleSecondaryActionClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UFinalRunFlowOverlayScreen::HandleCloseClicked);
	}
}

void UFinalRunFlowOverlayScreen::RebuildVisual()
{
	ClearFocusableWidgets();

	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;
	const bool bPendingBattleReward = Snapshot.PendingBattleReward.bHasPendingReward
		|| Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowOverlayTitle", "Run 主流程"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowOverlaySummary", "阶段：{0}\n金币 {1}  ·  牌库 {2}  ·  遗物 {3}"),
			FormatFlowStageText(Progression.FlowStage),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.DeckCount),
			FText::AsNumber(Snapshot.RelicCount)));
	}

	if (CurrentStageText)
	{
		CurrentStageText->SetText(BuildCurrentStageText());
	}

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCompactCurrentNodeText(Progression));
	}

	if (RouteSummaryText)
	{
		RouteSummaryText->SetText(BuildRouteSummaryText());
	}

	RebuildRouteNodeList();

	if (StageDetailText)
	{
		StageDetailText->SetText(BuildStageDetailText());
	}

	if (SelectionText)
	{
		SelectionText->SetText(BuildSelectionText());
		SelectionText->SetVisibility(ESlateVisibility::Collapsed);
	}

	RebuildOptionLists();

	if (GapText)
	{
		GapText->SetText(FText::GetEmpty());
	}

	if (FeedbackText)
	{
		RefreshFeedbackText(NSLOCTEXT("FinalFlowUI", "RunFlowOverlayFeedbackDefault", "等待当前流程操作。"));
	}

	const int32 RewardCount = Snapshot.PendingBattleReward.RewardEntries.Num();
	if (RewardOption0Button)
	{
		RewardOption0Button->SetVisibility(ESlateVisibility::Collapsed);
		RewardOption0Button->SetIsEnabled(bPendingBattleReward && Snapshot.PendingBattleReward.bCanClaim && RewardCount > 0);
	}
	if (RewardOption0ButtonText)
	{
		RewardOption0ButtonText->SetText(FormatRewardOptionText(Snapshot.PendingBattleReward, 0));
	}

	if (RewardOption1Button)
	{
		RewardOption1Button->SetVisibility(ESlateVisibility::Collapsed);
		RewardOption1Button->SetIsEnabled(bPendingBattleReward && Snapshot.PendingBattleReward.bCanClaim && RewardCount > 1);
	}
	if (RewardOption1ButtonText)
	{
		RewardOption1ButtonText->SetText(FormatRewardOptionText(Snapshot.PendingBattleReward, 1));
	}

	if (RewardOption2Button)
	{
		RewardOption2Button->SetVisibility(ESlateVisibility::Collapsed);
		RewardOption2Button->SetIsEnabled(bPendingBattleReward && Snapshot.PendingBattleReward.bCanClaim && RewardCount > 2);
	}
	if (RewardOption2ButtonText)
	{
		RewardOption2ButtonText->SetText(FormatRewardOptionText(Snapshot.PendingBattleReward, 2));
	}

	if (PreviousChoiceButton)
	{
		PreviousChoiceButton->SetVisibility(ESlateVisibility::Collapsed);
		PreviousChoiceButton->SetIsEnabled(CanUsePreviousNext());
	}
	if (PreviousChoiceButtonText)
	{
		PreviousChoiceButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowPreviousChoiceLabel", "上一个"));
	}

	if (NextChoiceButton)
	{
		NextChoiceButton->SetVisibility(ESlateVisibility::Collapsed);
		NextChoiceButton->SetIsEnabled(CanUsePreviousNext());
	}
	if (NextChoiceButtonText)
	{
		NextChoiceButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowNextChoiceLabel", "下一个"));
	}

	if (PrimaryActionButton)
	{
		const bool bUsePrimaryFallback = Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingRewardNode;
		PrimaryActionButton->SetVisibility((bUsePrimaryFallback && CanUsePrimaryAction()) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		PrimaryActionButton->SetIsEnabled(CanUsePrimaryAction());
		RegisterFocusableWidget(PrimaryActionButton);
	}
	if (PrimaryActionButtonText)
	{
		PrimaryActionButtonText->SetText(BuildPrimaryActionText());
	}

	if (SecondaryActionButton)
	{
		SecondaryActionButton->SetVisibility(CanUseSecondaryAction() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		SecondaryActionButton->SetIsEnabled(CanUseSecondaryAction());
		RegisterFocusableWidget(SecondaryActionButton);
	}
	if (SecondaryActionButtonText)
	{
		SecondaryActionButtonText->SetText(BuildSecondaryActionText());
	}

	if (CloseButton)
	{
		CloseButton->SetVisibility(ESlateVisibility::Visible);
		CloseButton->SetIsEnabled(true);
		RegisterFocusableWidget(CloseButton);
	}
	if (CloseButtonText)
	{
		CloseButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RunFlowCloseLabel", "关闭"));
	}

	FocusFirstAvailableAction();
}

void UFinalRunFlowOverlayScreen::ClampSelectionIndices()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();

	if (Snapshot.Progression.AvailableNextNodes.Num() <= 0)
	{
		SelectedNextNodeIndex = INDEX_NONE;
	}
	else if (!Snapshot.Progression.AvailableNextNodes.IsValidIndex(SelectedNextNodeIndex))
	{
		SelectedNextNodeIndex = 0;
	}

	if (Snapshot.PendingEventNode.Options.Num() <= 0)
	{
		SelectedEventOptionIndex = INDEX_NONE;
	}
	else if (!Snapshot.PendingEventNode.Options.IsValidIndex(SelectedEventOptionIndex))
	{
		SelectedEventOptionIndex = 0;
	}

	if (Snapshot.PendingShopNode.Offers.Num() <= 0)
	{
		SelectedShopOfferIndex = INDEX_NONE;
	}
	else if (!Snapshot.PendingShopNode.Offers.IsValidIndex(SelectedShopOfferIndex))
	{
		SelectedShopOfferIndex = 0;
	}
}

void UFinalRunFlowOverlayScreen::ClearOptionLists()
{
	if (ActionListBox)
	{
		ActionListBox->ClearChildren();
		ActionListBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (RewardOptionListBox)
	{
		RewardOptionListBox->ClearChildren();
		RewardOptionListBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (NextNodeListBox)
	{
		NextNodeListBox->ClearChildren();
		NextNodeListBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (EventOptionListBox)
	{
		EventOptionListBox->ClearChildren();
		EventOptionListBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ShopOfferListBox)
	{
		ShopOfferListBox->ClearChildren();
		ShopOfferListBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalRunFlowOverlayScreen::RebuildRouteNodeList()
{
	if (WidgetTree == nullptr || RouteNodeListBox == nullptr)
	{
		return;
	}

	RouteNodeListBox->ClearChildren();

	const TArray<FFinalRunRouteNodeViewData>& Nodes = GetCachedSnapshot().RouteOverview.Nodes;
	if (Nodes.IsEmpty())
	{
		RouteNodeListBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const TSubclassOf<UFinalRunRouteNodeEntryWidget> EntryClass = UFinalUIWidgetClassSettings::GetRunRouteNodeEntryWidgetClass();
	UClass* ResolvedEntryClass = EntryClass.Get() ? EntryClass.Get() : UFinalRunRouteNodeEntryWidget::StaticClass();
	for (const FFinalRunRouteNodeViewData& Node : Nodes)
	{
		UFinalRunRouteNodeEntryWidget* NodeEntry = WidgetTree->ConstructWidget<UFinalRunRouteNodeEntryWidget>(
			ResolvedEntryClass,
			*FString::Printf(TEXT("RunRouteNode_%s_%s"), *Node.NodeId.ToString(), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
		if (NodeEntry == nullptr)
		{
			continue;
		}

		NodeEntry->ApplyRouteNodeView(Node);
		if (UVerticalBoxSlot* NodeSlot = RouteNodeListBox->AddChildToVerticalBox(NodeEntry))
		{
			NodeSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		}
	}

	RouteNodeListBox->SetVisibility(RouteNodeListBox->GetChildrenCount() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFinalRunFlowOverlayScreen::RebuildOptionLists()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	ClearOptionLists();

	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const TSubclassOf<UFinalRunFlowOptionButton> OptionButtonClass = UFinalUIWidgetClassSettings::GetRunFlowOptionButtonClass();
	auto AddOption = [this, OptionButtonClass](
		UVerticalBox* ListBox,
		const FFinalRunFlowOptionButtonData& OptionData)
	{
		if (ListBox == nullptr)
		{
			return;
		}

		UClass* ResolvedOptionButtonClass = OptionButtonClass.Get() ? OptionButtonClass.Get() : UFinalRunFlowOptionButton::StaticClass();
		UFinalRunFlowOptionButton* OptionWidget = WidgetTree->ConstructWidget<UFinalRunFlowOptionButton>(
			ResolvedOptionButtonClass,
			*FString::Printf(TEXT("RunFlowOption_%d_%s"), static_cast<int32>(OptionData.Kind), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
		if (OptionWidget == nullptr)
		{
			return;
		}

		OptionWidget->ConfigureOption(OptionData);
		OptionWidget->OnOptionClicked.AddUObject(this, &UFinalRunFlowOverlayScreen::HandleListOptionClicked);
		RegisterFocusableWidget(OptionWidget->GetFocusTarget());
		if (UVerticalBoxSlot* OptionSlot = ListBox->AddChildToVerticalBox(OptionWidget))
		{
			OptionSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
		ListBox->SetVisibility(ESlateVisibility::Visible);
	};

	const bool bPendingBattleReward = Snapshot.PendingBattleReward.bHasPendingReward
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward;
	if (Snapshot.AvailableFlowActions.Num() > 0)
	{
		for (const FFinalRunFlowActionViewData& Action : Snapshot.AvailableFlowActions)
		{
			AddOption(ActionListBox, BuildFlowActionOptionData(Action));
		}
		return;
	}

	if (bPendingBattleReward)
	{
		for (int32 RewardIndex = 0; RewardIndex < Snapshot.PendingBattleReward.RewardEntries.Num(); ++RewardIndex)
		{
			AddOption(
				RewardOptionListBox,
				BuildRewardOptionData(Snapshot.PendingBattleReward, RewardIndex));
		}
		return;
	}

	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		for (int32 NodeIndex = 0; NodeIndex < Snapshot.Progression.AvailableNextNodes.Num(); ++NodeIndex)
		{
			const FFinalRunNodeOptionViewData& Node = Snapshot.Progression.AvailableNextNodes[NodeIndex];
			AddOption(
				NextNodeListBox,
				BuildNextNodeOptionData(Node, NodeIndex, Snapshot.Progression.bCanAdvanceToNextNode));
		}
		break;

	case EFinalRunFlowStage::PendingEventNode:
		for (int32 OptionIndex = 0; OptionIndex < Snapshot.PendingEventNode.Options.Num(); ++OptionIndex)
		{
			const FFinalRunEventOptionViewData& Option = Snapshot.PendingEventNode.Options[OptionIndex];
			AddOption(
				EventOptionListBox,
				BuildEventOptionData(Option, OptionIndex, Snapshot.PendingEventNode.bCanResolve && !Snapshot.PendingEventNode.bResolved));
		}
		break;

	case EFinalRunFlowStage::PendingShopNode:
		for (int32 OfferIndex = 0; OfferIndex < Snapshot.PendingShopNode.Offers.Num(); ++OfferIndex)
		{
			const FFinalRunShopOfferViewData& Offer = Snapshot.PendingShopNode.Offers[OfferIndex];
			AddOption(
				ShopOfferListBox,
				BuildShopOfferOptionData(Offer, OfferIndex, Snapshot.PendingShopNode.bCanResolve && !Snapshot.PendingShopNode.bResolved));
		}
		break;

	default:
		break;
	}
}

void UFinalRunFlowOverlayScreen::HandleRewardOptionClicked(const int32 RewardIndex)
{
	const FFinalRunPendingBattleRewardViewData& PendingReward = GetCachedSnapshot().PendingBattleReward;
	if (!PendingReward.RewardEntries.IsValidIndex(RewardIndex))
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowRewardOptionMissingFeedback", "当前没有对应的战后卡牌候选。"));
		RebuildVisual();
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowRewardMissingSubsystem", "当前无法访问 RunFlowSubsystem，无法领取奖励。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->ClaimPendingBattleRewardById(PendingReward.RewardEntries[RewardIndex].RewardId);
	RefreshAfterFlowAction(
		bAccepted,
		NSLOCTEXT("FinalFlowUI", "RunFlowClaimRewardSucceeded", "已领取战后卡牌奖励。"),
		NSLOCTEXT("FinalFlowUI", "RunFlowClaimRewardFailed", "领取战后卡牌奖励失败。"));
}

void UFinalRunFlowOverlayScreen::HandleListOptionClicked(UFinalRunFlowOptionButton* OptionButton)
{
	if (OptionButton == nullptr)
	{
		return;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunFlowListOptionMissingSubsystem", "当前无法访问 RunFlowSubsystem。"));
		RebuildVisual();
		return;
	}

	bool bAccepted = false;
	FText SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowListActionSucceeded", "操作已提交。");
	FText FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowListActionFailed", "操作提交失败。");

	switch (OptionButton->GetOptionKind())
	{
	case EFinalRunFlowOptionKind::Reward:
		bAccepted = RunFlowSubsystem->ClaimPendingBattleRewardById(OptionButton->GetPayloadId());
		SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowListClaimRewardSucceeded", "已领取战后卡牌奖励。");
		FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowListClaimRewardFailed", "领取战后卡牌奖励失败。");
		break;

	case EFinalRunFlowOptionKind::NextNode:
		bAccepted = RunFlowSubsystem->AdvanceToNode(OptionButton->GetPayloadId());
		SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowListAdvanceSucceeded", "已推进到选中节点。");
		FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowListAdvanceFailed", "推进节点失败。");
		break;

	case EFinalRunFlowOptionKind::EventOption:
		bAccepted = RunFlowSubsystem->ResolveEventOption(OptionButton->GetPayloadId());
		SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowListEventSucceeded", "已提交事件选项。");
		FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowListEventFailed", "提交事件选项失败。");
		break;

	case EFinalRunFlowOptionKind::ShopOffer:
		bAccepted = RunFlowSubsystem->ResolveShopOffer(OptionButton->GetPayloadId());
		SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowListShopSucceeded", "已提交商店商品。");
		FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowListShopFailed", "提交商店商品失败。");
		break;

	case EFinalRunFlowOptionKind::FlowAction:
		switch (OptionButton->GetCommandType())
		{
		case EFinalRunCommandType::ClaimPendingBattleReward:
			bAccepted = RunFlowSubsystem->ClaimPendingBattleRewardById(OptionButton->GetPayloadId());
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionClaimRewardSucceeded", "已领取战后卡牌奖励。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionClaimRewardFailed", "领取战后卡牌奖励失败。");
			break;

		case EFinalRunCommandType::SkipPendingBattleReward:
			bAccepted = RunFlowSubsystem->SkipPendingBattleReward();
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionSkipRewardSucceeded", "已跳过战后卡牌奖励。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionSkipRewardFailed", "跳过战后卡牌奖励失败。");
			break;

		case EFinalRunCommandType::AdvanceToNode:
			bAccepted = RunFlowSubsystem->AdvanceToNode(OptionButton->GetPayloadId());
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionAdvanceSucceeded", "已推进到选中节点。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionAdvanceFailed", "推进节点失败。");
			break;

		case EFinalRunCommandType::ResolveReward:
			bAccepted = RunFlowSubsystem->ResolveRewardNode();
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionResolveRewardSucceeded", "已确认奖励节点。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionResolveRewardFailed", "确认奖励节点失败。");
			break;

		case EFinalRunCommandType::ResolveEvent:
			bAccepted = RunFlowSubsystem->ResolveEventOption(OptionButton->GetPayloadId());
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionResolveEventSucceeded", "已提交事件选项。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionResolveEventFailed", "提交事件选项失败。");
			break;

		case EFinalRunCommandType::ResolveShop:
			bAccepted = RunFlowSubsystem->ResolveShopOffer(OptionButton->GetPayloadId());
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionResolveShopSucceeded", "已提交商店商品。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionResolveShopFailed", "提交商店商品失败。");
			break;

		case EFinalRunCommandType::LeaveShop:
			bAccepted = RunFlowSubsystem->LeaveShop();
			SuccessText = NSLOCTEXT("FinalFlowUI", "RunFlowActionLeaveShopSucceeded", "已离开商店。");
			FailureText = NSLOCTEXT("FinalFlowUI", "RunFlowActionLeaveShopFailed", "离开商店失败。");
			break;

		default:
			break;
		}
		break;
	}

	RefreshAfterFlowAction(bAccepted, SuccessText, FailureText);
}

bool UFinalRunFlowOverlayScreen::RefreshAfterFlowAction(const bool bAccepted, const FText& SuccessText, const FText& FailureText)
{
	if (UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
		SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
			? RunFlowSubsystem->GetLastFlowMessage()
			: (bAccepted ? SuccessText : FailureText));
	}
	else
	{
		SetLastActionFeedback(bAccepted ? SuccessText : FailureText);
	}

	ClampSelectionIndices();
	RebuildVisual();
	return bAccepted;
}

FText UFinalRunFlowOverlayScreen::BuildStageDetailText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const EFinalRunFlowStage FlowStage = Snapshot.Progression.FlowStage;
	if (Snapshot.PendingBattleReward.bHasPendingReward || FlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowBattleRewardDetail", "战斗{0}。\n金币 +{1} 已入账。\n选择 1 张卡加入牌库，或跳过本次卡牌奖励。"),
			FormatBattleOutcomeText(Snapshot.PendingBattleReward.SourceBattleOutcome),
			FText::AsNumber(Snapshot.PendingBattleReward.RewardGold));
	}

	switch (FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceDetail", "当前节点已完成。\n选择下一站继续旅程。可选节点：{0}"),
			FText::AsNumber(Snapshot.Progression.AvailableNextNodes.Num()));

	case EFinalRunFlowStage::PendingRewardNode:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowRewardNodeDetail", "{0}\n{1}\n奖励：\n{2}"),
			FormatOptionalText(Snapshot.PendingRewardNode.Title, NSLOCTEXT("FinalFlowUI", "RunFlowRewardNodeNoTitle", "奖励节点")),
			FormatOptionalText(Snapshot.PendingRewardNode.Summary, NSLOCTEXT("FinalFlowUI", "RunFlowRewardNodeNoSummary", "确认当前节点奖励。")),
			FText::FromString(BuildRewardPresentationSummaryString(Snapshot.PendingRewardNode.RewardEntryViews, Snapshot.PendingRewardNode.RewardEntries)));

	case EFinalRunFlowStage::PendingEventNode:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowEventNodeDetail", "{0}\n{1}\n选择一个处理方式。选项：{2}"),
			FormatOptionalText(Snapshot.PendingEventNode.Title, NSLOCTEXT("FinalFlowUI", "RunFlowEventNodeNoTitle", "事件节点")),
			FormatOptionalText(Snapshot.PendingEventNode.Summary, NSLOCTEXT("FinalFlowUI", "RunFlowEventNodeNoSummary", "选择一个事件选项。")),
			FText::AsNumber(Snapshot.PendingEventNode.Options.Num()));

	case EFinalRunFlowStage::PendingShopNode:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowShopNodeDetail", "{0}\n{1}\n选择要购买的商品。商品：{2}"),
			FormatOptionalText(Snapshot.PendingShopNode.Title, NSLOCTEXT("FinalFlowUI", "RunFlowShopNodeNoTitle", "商店节点")),
			FormatOptionalText(Snapshot.PendingShopNode.Summary, NSLOCTEXT("FinalFlowUI", "RunFlowShopNodeNoSummary", "选择一个商店商品。")),
			FText::AsNumber(Snapshot.PendingShopNode.Offers.Num()));

	case EFinalRunFlowStage::RunEnded:
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunFlowEndedDetail", "本局已结束。\n最终金币：{0}\n牌库数量：{1}\n遗物数量：{2}\n最近战斗结果：{3}"),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.DeckCount),
			FText::AsNumber(Snapshot.RelicCount),
			FormatBattleOutcomeText(Snapshot.LastBattleOutcome));

	case EFinalRunFlowStage::PreparingBattle:
	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoActionDetail", "当前阶段没有需要 RunFlowOverlay 处理的操作。");
	}
}

FText UFinalRunFlowOverlayScreen::BuildSelectionText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			return BuildNextNodeSelectionText(*SelectedNode);
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoNextNodeSelection", "当前没有可选下一节点。");

	case EFinalRunFlowStage::PendingEventNode:
		if (const FFinalRunEventOptionViewData* SelectedOption = GetSelectedEventOption())
		{
			return BuildEventOptionSelectionText(*SelectedOption);
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoEventSelection", "当前事件没有可选项。");

	case EFinalRunFlowStage::PendingShopNode:
		if (const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedShopOffer())
		{
			return BuildShopOfferSelectionText(*SelectedOffer);
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowNoShopSelection", "当前商店没有商品。");

	default:
		return FText::GetEmpty();
	}
}

FText UFinalRunFlowOverlayScreen::BuildPrimaryActionText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			return FText::Format(
				NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceAction", "继续到：{0}"),
				FormatRunNodeDisplayName(SelectedNode->DisplayName, SelectedNode->NodeId, SelectedNode->NodeType));
		}
		return NSLOCTEXT("FinalFlowUI", "RunFlowAdvanceActionMissing", "没有可推进节点");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowResolveRewardAction", "确认奖励节点");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowResolveEventAction", "选择当前事件选项");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowResolveShopAction", "购买当前商品");

	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowPrimaryActionUnavailable", "当前无主操作");
	}
}

FText UFinalRunFlowOverlayScreen::BuildSecondaryActionText() const
{
	if (GetCachedSnapshot().Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward
		|| GetCachedSnapshot().PendingBattleReward.bHasPendingReward)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowSkipRewardAction", "跳过卡牌奖励");
	}

	return NSLOCTEXT("FinalFlowUI", "RunFlowSecondaryActionUnavailable", "当前无次要操作");
}

FText UFinalRunFlowOverlayScreen::BuildRouteSummaryText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunRouteOverviewViewData& RouteOverview = Snapshot.RouteOverview;
	if (RouteOverview.Nodes.IsEmpty())
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowRouteSummaryEmpty", "路线总览暂不可用。");
	}

	int32 VisitedCount = 0;
	int32 ResolvedCount = 0;
	int32 ReachableCount = 0;
	int32 LockedCount = 0;
	for (const FFinalRunRouteNodeViewData& Node : RouteOverview.Nodes)
	{
		VisitedCount += Node.bVisited ? 1 : 0;
		ResolvedCount += Node.bResolved ? 1 : 0;
		ReachableCount += Node.bReachable ? 1 : 0;
		LockedCount += Node.bLocked ? 1 : 0;
	}

	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunFlowRouteSummary", "路线：{0}\n节点 {1} | 已访问 {2} | 已解决 {3} | 可达 {4} | 锁定 {5}"),
		FormatOptionalText(RouteOverview.DisplayName, FormatOptionalName(RouteOverview.RouteId, NSLOCTEXT("FinalFlowUI", "RunFlowRouteSummaryUnnamed", "未命名路线"))),
		FText::AsNumber(RouteOverview.Nodes.Num()),
		FText::AsNumber(VisitedCount),
		FText::AsNumber(ResolvedCount),
		FText::AsNumber(ReachableCount),
		FText::AsNumber(LockedCount));
}

FText UFinalRunFlowOverlayScreen::BuildCurrentStageText() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const EFinalRunFlowStage FlowStage = Snapshot.Progression.FlowStage;
	if (Snapshot.PendingBattleReward.bHasPendingReward || FlowStage == EFinalRunFlowStage::PendingBattleReward)
	{
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageBattleReward", "当前阶段：战后奖励");
	}

	switch (FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageNodeAdvance", "当前阶段：选择下一节点");

	case EFinalRunFlowStage::PendingRewardNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageRewardNode", "当前阶段：奖励节点");

	case EFinalRunFlowStage::PendingEventNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageEventNode", "当前阶段：事件节点");

	case EFinalRunFlowStage::PendingShopNode:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageShopNode", "当前阶段：商店节点");

	case EFinalRunFlowStage::RunEnded:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageRunEnded", "当前阶段：Run 已结束");

	case EFinalRunFlowStage::PreparingBattle:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStagePreparingBattle", "当前阶段：准备战斗");

	case EFinalRunFlowStage::None:
	default:
		return NSLOCTEXT("FinalFlowUI", "RunFlowStageNone", "当前阶段：无待处理流程");
	}
}

bool UFinalRunFlowOverlayScreen::CanUsePreviousNext() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		return Snapshot.Progression.AvailableNextNodes.Num() > 1;

	case EFinalRunFlowStage::PendingEventNode:
		return Snapshot.PendingEventNode.Options.Num() > 1;

	case EFinalRunFlowStage::PendingShopNode:
		return Snapshot.PendingShopNode.Offers.Num() > 1;

	default:
		return false;
	}
}

bool UFinalRunFlowOverlayScreen::CanUsePrimaryAction() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	switch (Snapshot.Progression.FlowStage)
	{
	case EFinalRunFlowStage::AwaitingNodeAdvance:
		if (const FFinalRunNodeOptionViewData* SelectedNode = GetSelectedNextNode())
		{
			return Snapshot.Progression.bCanAdvanceToNextNode && !SelectedNode->bLocked;
		}
		return false;

	case EFinalRunFlowStage::PendingRewardNode:
		return Snapshot.PendingRewardNode.bHasPendingContent
			&& Snapshot.PendingRewardNode.bCanResolve
			&& !Snapshot.PendingRewardNode.bResolved;

	case EFinalRunFlowStage::PendingEventNode:
		if (const FFinalRunEventOptionViewData* SelectedOption = GetSelectedEventOption())
		{
			return Snapshot.PendingEventNode.bHasPendingContent
				&& Snapshot.PendingEventNode.bCanResolve
				&& !Snapshot.PendingEventNode.bResolved
				&& SelectedOption->bSelectable;
		}
		return false;

	case EFinalRunFlowStage::PendingShopNode:
		if (const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedShopOffer())
		{
			return Snapshot.PendingShopNode.bHasPendingContent
				&& Snapshot.PendingShopNode.bCanResolve
				&& !Snapshot.PendingShopNode.bResolved
				&& SelectedOffer->bPurchasable
				&& !SelectedOffer->bPurchased;
		}
		return false;

	default:
		return false;
	}
}

bool UFinalRunFlowOverlayScreen::CanUseSecondaryAction() const
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	return (Snapshot.PendingBattleReward.bHasPendingReward
		|| Snapshot.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward)
		&& Snapshot.PendingBattleReward.bCanClaim;
}

const FFinalRunNodeOptionViewData* UFinalRunFlowOverlayScreen::GetSelectedNextNode() const
{
	const TArray<FFinalRunNodeOptionViewData>& Nodes = GetCachedSnapshot().Progression.AvailableNextNodes;
	return Nodes.IsValidIndex(SelectedNextNodeIndex) ? &Nodes[SelectedNextNodeIndex] : nullptr;
}

const FFinalRunEventOptionViewData* UFinalRunFlowOverlayScreen::GetSelectedEventOption() const
{
	const TArray<FFinalRunEventOptionViewData>& Options = GetCachedSnapshot().PendingEventNode.Options;
	return Options.IsValidIndex(SelectedEventOptionIndex) ? &Options[SelectedEventOptionIndex] : nullptr;
}

const FFinalRunShopOfferViewData* UFinalRunFlowOverlayScreen::GetSelectedShopOffer() const
{
	const TArray<FFinalRunShopOfferViewData>& Offers = GetCachedSnapshot().PendingShopNode.Offers;
	return Offers.IsValidIndex(SelectedShopOfferIndex) ? &Offers[SelectedShopOfferIndex] : nullptr;
}
