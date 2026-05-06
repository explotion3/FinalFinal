#include "UI/Screens/Flow/FinalRunShopNodeOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
#include "Styling/CoreStyle.h"

using namespace FinalRunFlowScreenUtils;

namespace
{
const FFinalRunShopOfferViewData* GetSelectedOfferView(const FFinalRunPendingShopNodeViewData& PendingShopNode, const int32 SelectedOfferIndex)
{
	return PendingShopNode.Offers.IsValidIndex(SelectedOfferIndex)
		? &PendingShopNode.Offers[SelectedOfferIndex]
		: nullptr;
}

FString BuildShopOffersSummaryString(const TArray<FFinalRunShopOfferViewData>& Offers, const int32 SelectedOfferIndex)
{
	if (Offers.Num() <= 0)
	{
		return NSLOCTEXT("FinalFlowUI", "ShopNodeOffersEmpty", "当前没有公开的商店商品。").ToString();
	}

	FString OffersSummary;
	for (int32 Index = 0; Index < Offers.Num(); ++Index)
	{
		const FFinalRunShopOfferViewData& Offer = Offers[Index];
		OffersSummary += FString::Printf(
			TEXT("%s[%d] %s | OfferId: %s | 价格: %d | 可购买: %s | 已购买: %s"),
			Index == SelectedOfferIndex ? TEXT("> ") : TEXT("  "),
			Index + 1,
			*FormatOptionalText(Offer.DisplayName, NSLOCTEXT("FinalFlowUI", "ShopNodeOfferUnnamed", "未命名商品")).ToString(),
			Offer.OfferId != NAME_None ? *Offer.OfferId.ToString() : TEXT("None"),
			Offer.Price,
			Offer.bPurchasable ? TEXT("是") : TEXT("否"),
			Offer.bPurchased ? TEXT("是") : TEXT("否"));

		if (!Offer.AvailabilityMessage.IsEmpty())
		{
			OffersSummary += FString::Printf(TEXT(" | 限制: %s"), *Offer.AvailabilityMessage.ToString());
		}

		const int32 RewardEntryCount = Offer.RewardEntryViews.Num() > 0 ? Offer.RewardEntryViews.Num() : Offer.RewardEntries.Num();
		OffersSummary += FString::Printf(TEXT(" | 奖励条目数: %d\n"), RewardEntryCount);
	}

	OffersSummary.TrimEndInline();
	return OffersSummary;
}

FName FindFirstShopOfferIconId(const FFinalRunShopOfferViewData& Offer)
{
	for (const FFinalRunRewardEntryViewData& RewardEntryView : Offer.RewardEntryViews)
	{
		if (RewardEntryView.IconId != NAME_None)
		{
			return RewardEntryView.IconId;
		}
	}
	return NAME_None;
}

EFinalRunRewardPresentationKind FindFirstShopOfferPresentationKind(const FFinalRunShopOfferViewData& Offer)
{
	return Offer.RewardEntryViews.Num() > 0
		? Offer.RewardEntryViews[0].PresentationKind
		: EFinalRunRewardPresentationKind::None;
}

EFinalRunRewardVisualTier FindFirstShopOfferVisualTier(const FFinalRunShopOfferViewData& Offer)
{
	return Offer.RewardEntryViews.Num() > 0
		? Offer.RewardEntryViews[0].VisualTier
		: EFinalRunRewardVisualTier::None;
}
}

void UFinalRunShopOfferEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();

	if (OfferButton)
	{
		OfferButton->OnClicked.AddUniqueDynamic(this, &UFinalRunShopOfferEntryWidget::HandleClicked);
	}

	RefreshBoundWidgets();
}

void UFinalRunShopOfferEntryWidget::ApplyOfferView(const FFinalRunShopOfferEntryViewData& InViewData)
{
	CachedViewData = InViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnOfferViewApplied(CachedViewData);
}

void UFinalRunShopOfferEntryWidget::HandleClicked()
{
	OnOfferClicked.Broadcast(this);
}

void UFinalRunShopOfferEntryWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	OfferButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("OfferButton"));
	WidgetTree->RootWidget = OfferButton;

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ShopOfferRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.12f, 0.08f, 0.04f, 0.94f));
	RootBorder->SetPadding(FMargin(8.0f));
	OfferButton->AddChild(RootBorder);

	UVerticalBox* TextBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ShopOfferTextBox"));
	RootBorder->SetContent(TextBox);

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetAutoWrapText(true);
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 14));
	TextBox->AddChildToVerticalBox(TitleText);

	DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DescriptionText"));
	DescriptionText->SetAutoWrapText(true);
	DescriptionText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(DescriptionText);

	PriceText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PriceText"));
	PriceText->SetAutoWrapText(true);
	PriceText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(PriceText);

	PreviewRewardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PreviewRewardText"));
	PreviewRewardText->SetAutoWrapText(true);
	PreviewRewardText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	TextBox->AddChildToVerticalBox(PreviewRewardText);

	StateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StateText"));
	StateText->SetAutoWrapText(true);
	StateText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(StateText);

	DisabledReasonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DisabledReasonText"));
	DisabledReasonText->SetAutoWrapText(true);
	DisabledReasonText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	TextBox->AddChildToVerticalBox(DisabledReasonText);
}

void UFinalRunShopOfferEntryWidget::RefreshBoundWidgets()
{
	if (TitleText)
	{
		TitleText->SetText(CachedViewData.Title);
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(CachedViewData.Description);
		DescriptionText->SetVisibility(CachedViewData.Description.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (PriceText)
	{
		PriceText->SetText(CachedViewData.PriceText);
		PriceText->SetVisibility(CachedViewData.PriceText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (PreviewRewardText)
	{
		PreviewRewardText->SetText(CachedViewData.PreviewRewardText);
		PreviewRewardText->SetVisibility(CachedViewData.PreviewRewardText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (StateText)
	{
		StateText->SetText(CachedViewData.StateText);
	}
	if (DisabledReasonText)
	{
		DisabledReasonText->SetText(CachedViewData.DisabledReason);
		DisabledReasonText->SetVisibility(CachedViewData.DisabledReason.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (OfferButton)
	{
		OfferButton->SetIsEnabled(CachedViewData.bEnabled);
	}
	if (IconImage)
	{
		IconImage->SetVisibility(CachedViewData.IconId.IsNone() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (TierVisual)
	{
		TierVisual->SetVisibility(CachedViewData.VisualTier == EFinalRunRewardVisualTier::None ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (PurchasedVisual)
	{
		PurchasedVisual->SetVisibility(CachedViewData.bPurchased ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UFinalRunShopNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	if (LeaveShopButton)
	{
		LeaveShopButton->OnClicked.AddUniqueDynamic(this, &UFinalRunShopNodeOverlayScreen::HandleLeaveShopClicked);
	}
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	NormalizeSelectedOfferIndex();
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::HandlePreviousOfferClicked()
{
	StepSelectedOffer(-1);
}

void UFinalRunShopNodeOverlayScreen::HandleNextOfferClicked()
{
	StepSelectedOffer(1);
}

void UFinalRunShopNodeOverlayScreen::HandlePurchaseOfferClicked()
{
	const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedOfferView(GetCachedSnapshot().PendingShopNode, SelectedOfferIndex);
	if (SelectedOffer == nullptr || SelectedOffer->OfferId == NAME_None)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "ShopNodeMissingSelectedOffer", "当前没有可提交的商店商品。"));
		RebuildVisual();
		return;
	}

	HandlePurchaseOfferById(SelectedOffer->OfferId);
}

void UFinalRunShopNodeOverlayScreen::HandlePurchaseOfferById(const FName OfferId)
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "ShopNodeMissingRunFlow", "当前无法访问 RunFlowSubsystem，无法提交商店节点购买请求。"));
		RebuildVisual();
		return;
	}

	if (OfferId == NAME_None)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "ShopNodeMissingOfferId", "当前没有可提交的商店商品。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->ResolveShopOffer(OfferId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAccepted
			? NSLOCTEXT("FinalFlowUI", "ShopNodeResolveSucceeded", "已转发 ResolveShop。")
			: NSLOCTEXT("FinalFlowUI", "ShopNodeResolveFailed", "ResolveShop 执行失败。")));
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::HandleOfferClicked(UFinalRunShopOfferEntryWidget* OfferEntry)
{
	if (OfferEntry == nullptr)
	{
		return;
	}

	HandlePurchaseOfferById(OfferEntry->GetOfferViewData().OfferId);
}

void UFinalRunShopNodeOverlayScreen::HandleLeaveShopClicked()
{
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "ShopNodeMissingRunFlowForLeave", "当前无法访问 RunFlowSubsystem，无法离开商店。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->LeaveShop();
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAccepted
			? NSLOCTEXT("FinalFlowUI", "ShopNodeLeaveSucceeded", "已离开商店。")
			: NSLOCTEXT("FinalFlowUI", "ShopNodeLeaveFailed", "离开商店失败。")));
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::HandleCloseClicked()
{
	RequestCloseOverlay();
}

void UFinalRunShopNodeOverlayScreen::EnsureWidgetTree()
{
	EnsureBaseWidgetTree(FLinearColor(0.11f, 0.08f, 0.05f, 0.96f), TEXT("ShopNodeOverlayRoot"), TEXT("ShopNodeOverlayContent"));
	if (ContentBox == nullptr)
	{
		return;
	}

	if (CurrentNodeText == nullptr)
	{
		CurrentNodeText = CreateStageLabel(TEXT("ShopNodeOverlayCurrentNode"), 13);
		ContentBox->InsertChildAt(2, CurrentNodeText);
	}
	if (NodeText == nullptr)
	{
		NodeText = CurrentNodeText;
	}

	if (OfferListBox == nullptr)
	{
		OfferListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OfferListBox"));
		ContentBox->InsertChildAt(3, OfferListBox);
	}

	if (RewardPreviewBox == nullptr)
	{
		RewardPreviewBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardPreviewBox"));
		ContentBox->InsertChildAt(4, RewardPreviewBox);
	}

	if (OffersListText == nullptr)
	{
		OffersListText = CreateStageLabel(TEXT("ShopNodeOverlayOffersList"), 13);
		OffersListText->SetVisibility(ESlateVisibility::Collapsed);
		ContentBox->InsertChildAt(5, OffersListText);
	}

	if (SelectedOfferText == nullptr)
	{
		SelectedOfferText = CreateStageLabel(TEXT("ShopNodeOverlaySelectedOffer"), 13);
		SelectedOfferText->SetVisibility(ESlateVisibility::Collapsed);
		ContentBox->InsertChildAt(6, SelectedOfferText);
	}

	if (PreviousOfferButton == nullptr)
	{
		PreviousOfferButton = CreateStageButton(
			TEXT("ShopNodeOverlayPrevOfferButton"),
			TEXT("ShopNodeOverlayPrevOfferButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodePrevOfferButton", "上一件商品"),
			PreviousOfferButtonText);
		PreviousOfferButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandlePreviousOfferClicked);
		ContentBox->AddChildToVerticalBox(PreviousOfferButton);
	}

	if (NextOfferButton == nullptr)
	{
		NextOfferButton = CreateStageButton(
			TEXT("ShopNodeOverlayNextOfferButton"),
			TEXT("ShopNodeOverlayNextOfferButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodeNextOfferButton", "下一件商品"),
			NextOfferButtonText);
		NextOfferButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandleNextOfferClicked);
		ContentBox->AddChildToVerticalBox(NextOfferButton);
	}

	if (PurchaseOfferButton == nullptr)
	{
		PurchaseOfferButton = CreateStageButton(
			TEXT("ShopNodeOverlayPurchaseButton"),
			TEXT("ShopNodeOverlayPurchaseButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodePurchaseButton", "购买当前商品"),
			PurchaseOfferButtonText);
		PurchaseOfferButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandlePurchaseOfferClicked);
		ContentBox->AddChildToVerticalBox(PurchaseOfferButton);
	}

	if (LeaveShopButton == nullptr)
	{
		LeaveShopButton = CreateStageButton(
			TEXT("ShopNodeOverlayLeaveButton"),
			TEXT("ShopNodeOverlayLeaveButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodeLeaveButton", "离开商店"),
			LeaveShopButtonText);
		LeaveShopButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandleLeaveShopClicked);
		ContentBox->AddChildToVerticalBox(LeaveShopButton);
	}

	if (CloseButton == nullptr)
	{
		CloseButton = CreateStageButton(
			TEXT("ShopNodeOverlayCloseButton"),
			TEXT("ShopNodeOverlayCloseButtonText"),
			NSLOCTEXT("FinalFlowUI", "ShopNodeCloseButton", "关闭商店节点页"),
			CloseButtonText);
		CloseButton->OnClicked.AddDynamic(this, &UFinalRunShopNodeOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunShopNodeOverlayScreen::NormalizeSelectedOfferIndex()
{
	const TArray<FFinalRunShopOfferViewData>& Offers = GetCachedSnapshot().PendingShopNode.Offers;
	if (Offers.Num() <= 0)
	{
		SelectedOfferIndex = INDEX_NONE;
		return;
	}

	if (!Offers.IsValidIndex(SelectedOfferIndex))
	{
		const int32 FirstOfferIndex = Offers.IndexOfByPredicate([](const FFinalRunShopOfferViewData& Offer)
		{
			return Offer.OfferId != NAME_None;
		});

		SelectedOfferIndex = FirstOfferIndex != INDEX_NONE ? FirstOfferIndex : 0;
	}
}

void UFinalRunShopNodeOverlayScreen::StepSelectedOffer(const int32 Direction)
{
	const int32 OfferCount = GetCachedSnapshot().PendingShopNode.Offers.Num();
	if (OfferCount <= 0)
	{
		SelectedOfferIndex = INDEX_NONE;
		RebuildVisual();
		return;
	}

	NormalizeSelectedOfferIndex();
	SelectedOfferIndex = (SelectedOfferIndex + Direction + OfferCount) % OfferCount;
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::RebuildVisual()
{
	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunPendingShopNodeViewData& PendingShopNode = Snapshot.PendingShopNode;
	const FFinalRunProgressionViewData& Progression = Snapshot.Progression;
	const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedOfferView(PendingShopNode, SelectedOfferIndex);

	if (TitleText)
	{
		TitleText->SetText(FormatOptionalText(
			PendingShopNode.Title,
			NSLOCTEXT("FinalFlowUI", "ShopNodeOverlayTitleText", "商店节点页")));
	}

	if (SummaryText)
	{
		SummaryText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "ShopNodeOverlaySummaryText", "{1}\n\n{2}\n\n商品 {6} | 当前金币 {7}"),
			FormatFlowStageText(Progression.FlowStage),
			FormatOptionalText(PendingShopNode.Title, NSLOCTEXT("FinalFlowUI", "ShopNodeNoTitle", "未公开标题")),
			FormatOptionalText(PendingShopNode.Summary, NSLOCTEXT("FinalFlowUI", "ShopNodeNoSummary", "当前没有额外摘要说明。")),
			FormatBool(PendingShopNode.bHasPendingContent),
			FormatBool(PendingShopNode.bCanResolve),
			FormatBool(PendingShopNode.bResolved),
			FText::AsNumber(PendingShopNode.Offers.Num()),
			FText::AsNumber(Snapshot.Gold),
			FText::AsNumber(Snapshot.RelicCount),
			FText::AsNumber(Snapshot.DeckCount)));
	}

	if (NodeText)
	{
		NodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (OffersListText)
	{
		OffersListText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "ShopNodeOverlayOffersText", "商店商品列表:\n{0}"),
			FText::FromString(BuildShopOffersSummaryString(PendingShopNode.Offers, SelectedOfferIndex))));
		OffersListText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SelectedOfferText)
	{
		if (SelectedOffer == nullptr)
		{
			SelectedOfferText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodeSelectedOfferMissing", "当前没有可预览的商店商品。"));
		}
		else
		{
			SelectedOfferText->SetText(FText::Format(
				NSLOCTEXT("FinalFlowUI", "ShopNodeSelectedOfferText", "当前选中商品: {0}\nOfferId: {1}\n描述: {2}\n价格: {3}\n可购买: {4}\n已购买: {5}\n可用性说明: {6}\n奖励条目:\n{7}"),
				FormatOptionalText(SelectedOffer->DisplayName, NSLOCTEXT("FinalFlowUI", "ShopNodeSelectedOfferNoName", "未公开商品名")),
				FormatOptionalName(SelectedOffer->OfferId, NSLOCTEXT("FinalFlowUI", "ShopNodeSelectedOfferNoId", "None")),
				FormatOptionalText(SelectedOffer->Description, NSLOCTEXT("FinalFlowUI", "ShopNodeSelectedOfferNoDescription", "当前没有公开商品描述。")),
				FText::AsNumber(SelectedOffer->Price),
				FormatBool(SelectedOffer->bPurchasable),
				FormatBool(SelectedOffer->bPurchased),
				FormatOptionalText(SelectedOffer->AvailabilityMessage, NSLOCTEXT("FinalFlowUI", "ShopNodeSelectedOfferNoAvailability", "当前没有额外限制说明。")),
				FText::FromString(BuildRewardPresentationSummaryString(SelectedOffer->RewardEntryViews, SelectedOffer->RewardEntries))));
		}
		SelectedOfferText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"ShopNodeOverlayGapText",
			"选择商品购买；可购买多个商品。点击离开商店后结束当前商店节点并进入路线推进阶段。"));
	}

	if (FeedbackText)
	{
		RefreshFeedbackText(NSLOCTEXT("FinalFlowUI", "ShopNodeOverlayFeedbackDefault", "购买商品会转发 OfferId；离开商店会提交 LeaveShop，由 RunFlowSubsystem 统一刷新或切页。"));
	}

	if (PreviousOfferButton)
	{
		PreviousOfferButton->SetIsEnabled(PendingShopNode.Offers.Num() > 1);
		PreviousOfferButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (NextOfferButton)
	{
		NextOfferButton->SetIsEnabled(PendingShopNode.Offers.Num() > 1);
		NextOfferButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (PurchaseOfferButton)
	{
		PurchaseOfferButton->SetIsEnabled(
			PendingShopNode.bHasPendingContent
			&& PendingShopNode.bCanResolve
			&& !PendingShopNode.bResolved
			&& SelectedOffer != nullptr
			&& SelectedOffer->OfferId != NAME_None
			&& SelectedOffer->bPurchasable
			&& !SelectedOffer->bPurchased);
		PurchaseOfferButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (PurchaseOfferButtonText)
	{
		if (!PendingShopNode.bHasPendingContent)
		{
			PurchaseOfferButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodePurchaseButtonMissing", "当前没有待处理商店节点内容"));
		}
		else if (PendingShopNode.bResolved)
		{
			PurchaseOfferButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodePurchaseButtonResolved", "当前商店节点已解析"));
		}
		else if (SelectedOffer == nullptr || SelectedOffer->OfferId == NAME_None)
		{
			PurchaseOfferButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodePurchaseButtonNoOffer", "当前没有可提交的商店商品"));
		}
		else if (SelectedOffer->bPurchased)
		{
			PurchaseOfferButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodePurchaseButtonPurchased", "当前商品已购买"));
		}
		else if (!PendingShopNode.bCanResolve || !SelectedOffer->bPurchasable)
		{
			PurchaseOfferButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodePurchaseButtonBlocked", "当前商品暂不可购买"));
		}
		else
		{
			PurchaseOfferButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodePurchaseButton", "购买当前商品"));
		}
	}

	if (LeaveShopButton)
	{
		LeaveShopButton->SetIsEnabled(PendingShopNode.bHasPendingContent && !PendingShopNode.bResolved);
		LeaveShopButton->SetVisibility(ESlateVisibility::Visible);
	}

	if (LeaveShopButtonText)
	{
		if (PendingShopNode.bResolved)
		{
			LeaveShopButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodeLeaveButtonResolved", "商店已离开"));
		}
		else
		{
			LeaveShopButtonText->SetText(NSLOCTEXT("FinalFlowUI", "ShopNodeLeaveButton", "离开商店"));
		}
	}

	RebuildOfferList();
}

void UFinalRunShopNodeOverlayScreen::RebuildOfferList()
{
	if (OfferListBox == nullptr || WidgetTree == nullptr)
	{
		return;
	}

	OfferListBox->ClearChildren();
	const FFinalRunPendingShopNodeViewData& PendingShopNode = GetCachedSnapshot().PendingShopNode;
	const TSubclassOf<UFinalRunShopOfferEntryWidget> ConfiguredEntryClass = UFinalUIWidgetClassSettings::GetRunShopOfferEntryWidgetClass();
	UClass* EntryClass = ConfiguredEntryClass.Get() ? ConfiguredEntryClass.Get() : UFinalRunShopOfferEntryWidget::StaticClass();

	for (int32 OfferIndex = 0; OfferIndex < PendingShopNode.Offers.Num(); ++OfferIndex)
	{
		UFinalRunShopOfferEntryWidget* OfferEntry = WidgetTree->ConstructWidget<UFinalRunShopOfferEntryWidget>(
			EntryClass,
			FName(*FString::Printf(TEXT("ShopOfferEntry_%d"), OfferIndex)));
		if (OfferEntry == nullptr)
		{
			continue;
		}

		OfferEntry->OnOfferClicked.AddUObject(this, &UFinalRunShopNodeOverlayScreen::HandleOfferClicked);
		OfferEntry->ApplyOfferView(BuildOfferEntryData(OfferIndex));

		UVerticalBoxSlot* EntrySlot = OfferListBox->AddChildToVerticalBox(OfferEntry);
		if (EntrySlot)
		{
			EntrySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
	}

	OfferListBox->SetVisibility(PendingShopNode.Offers.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

FFinalRunShopOfferEntryViewData UFinalRunShopNodeOverlayScreen::BuildOfferEntryData(const int32 OfferIndex) const
{
	FFinalRunShopOfferEntryViewData EntryData;
	const FFinalRunPendingShopNodeViewData& PendingShopNode = GetCachedSnapshot().PendingShopNode;
	if (!PendingShopNode.Offers.IsValidIndex(OfferIndex))
	{
		EntryData.Title = NSLOCTEXT("FinalFlowUI", "ShopOfferEntryInvalid", "无效商店商品");
		EntryData.StateText = NSLOCTEXT("FinalFlowUI", "ShopOfferEntryInvalidState", "不可购买");
		return EntryData;
	}

	const FFinalRunShopOfferViewData& Offer = PendingShopNode.Offers[OfferIndex];
	EntryData.OfferId = Offer.OfferId;
	EntryData.OfferIndex = OfferIndex;
	EntryData.Title = FormatOptionalText(
		Offer.DisplayName,
		FormatOptionalName(Offer.OfferId, NSLOCTEXT("FinalFlowUI", "ShopOfferEntryTitleFallback", "未命名商品")));
	EntryData.Description = FormatOptionalText(
		Offer.Description,
		NSLOCTEXT("FinalFlowUI", "ShopOfferEntryDescriptionFallback", "无额外商品说明。"));
	EntryData.PriceText = FText::Format(
		NSLOCTEXT("FinalFlowUI", "ShopOfferEntryPrice", "价格 {0} 金"),
		FText::AsNumber(Offer.Price));
	EntryData.PreviewRewardText = FText::FromString(BuildRewardPresentationSummaryString(Offer.RewardEntryViews, Offer.RewardEntries));
	EntryData.DisabledReason = Offer.AvailabilityMessage;
	EntryData.IconId = FindFirstShopOfferIconId(Offer);
	EntryData.PresentationKind = FindFirstShopOfferPresentationKind(Offer);
	EntryData.VisualTier = FindFirstShopOfferVisualTier(Offer);
	EntryData.bPurchased = Offer.bPurchased;
	EntryData.bEnabled = PendingShopNode.bHasPendingContent
		&& PendingShopNode.bCanResolve
		&& !PendingShopNode.bResolved
		&& Offer.bPurchasable
		&& !Offer.bPurchased
		&& Offer.OfferId != NAME_None;

	if (Offer.bPurchased)
	{
		EntryData.StateText = NSLOCTEXT("FinalFlowUI", "ShopOfferEntryPurchased", "已购买");
	}
	else if (EntryData.bEnabled)
	{
		EntryData.StateText = NSLOCTEXT("FinalFlowUI", "ShopOfferEntryEnabled", "可购买");
	}
	else if (PendingShopNode.bResolved)
	{
		EntryData.StateText = NSLOCTEXT("FinalFlowUI", "ShopOfferEntryNodeResolved", "商店已完成");
	}
	else
	{
		EntryData.StateText = FormatOptionalText(
			Offer.AvailabilityMessage,
			NSLOCTEXT("FinalFlowUI", "ShopOfferEntryDisabled", "暂不可购买"));
	}

	return EntryData;
}
