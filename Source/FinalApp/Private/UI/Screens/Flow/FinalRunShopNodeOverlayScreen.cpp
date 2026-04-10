#include "UI/Screens/Flow/FinalRunShopNodeOverlayScreen.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"

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

		OffersSummary += FString::Printf(TEXT(" | 奖励条目数: %d\n"), Offer.RewardEntries.Num());
	}

	OffersSummary.TrimEndInline();
	return OffersSummary;
}
}

void UFinalRunShopNodeOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
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
	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "ShopNodeMissingRunFlow", "当前无法访问 RunFlowSubsystem，无法提交商店节点购买请求。"));
		RebuildVisual();
		return;
	}

	const FFinalRunShopOfferViewData* SelectedOffer = GetSelectedOfferView(GetCachedSnapshot().PendingShopNode, SelectedOfferIndex);
	if (SelectedOffer == nullptr || SelectedOffer->OfferId == NAME_None)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "ShopNodeMissingSelectedOffer", "当前没有可提交的商店商品。"));
		RebuildVisual();
		return;
	}

	const bool bAccepted = RunFlowSubsystem->ResolveShopOffer(SelectedOffer->OfferId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAccepted
			? NSLOCTEXT("FinalFlowUI", "ShopNodeResolveSucceeded", "已转发 ResolveShop。")
			: NSLOCTEXT("FinalFlowUI", "ShopNodeResolveFailed", "ResolveShop 执行失败。")));
	RebuildVisual();
}

void UFinalRunShopNodeOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
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

	if (OffersListText == nullptr)
	{
		OffersListText = CreateStageLabel(TEXT("ShopNodeOverlayOffersList"), 13);
		ContentBox->InsertChildAt(3, OffersListText);
	}

	if (SelectedOfferText == nullptr)
	{
		SelectedOfferText = CreateStageLabel(TEXT("ShopNodeOverlaySelectedOffer"), 13);
		ContentBox->InsertChildAt(4, SelectedOfferText);
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
			NSLOCTEXT("FinalFlowUI", "ShopNodeOverlaySummaryText", "流程阶段: {0}\n商店标题: {1}\n商店摘要: {2}\n节点内容存在: {3}\n可解析: {4}\n已解析: {5}\n商品数: {6}\n当前金币: {7} | 遗物数: {8} | 牌库数: {9}"),
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

	if (CurrentNodeText)
	{
		CurrentNodeText->SetText(BuildCurrentNodeSummaryText(Progression));
	}

	if (OffersListText)
	{
		OffersListText->SetText(FText::Format(
			NSLOCTEXT("FinalFlowUI", "ShopNodeOverlayOffersText", "商店商品列表:\n{0}"),
			FText::FromString(BuildShopOffersSummaryString(PendingShopNode.Offers, SelectedOfferIndex))));
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
				FText::FromString(BuildRewardEntriesSummaryString(SelectedOffer->RewardEntries))));
		}
	}

	if (GapText)
	{
		GapText->SetText(NSLOCTEXT(
			"FinalFlowUI",
			"ShopNodeOverlayGapText",
			"当前页已真实消费 PendingShopNode 的标题、摘要、商品列表、价格、可购买状态与奖励条目。剩余缺口主要是 richer 布局、商品图标、分页、刷新与二次确认表现。"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "ShopNodeOverlayFeedbackDefault", "当前页面会把选中的 OfferId 通过 ResolveShop 转发给 RunFlowSubsystem，由它统一刷新或切页。")));
	}

	if (PreviousOfferButton)
	{
		PreviousOfferButton->SetIsEnabled(PendingShopNode.Offers.Num() > 1);
	}

	if (NextOfferButton)
	{
		NextOfferButton->SetIsEnabled(PendingShopNode.Offers.Num() > 1);
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
}
