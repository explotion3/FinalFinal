#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

namespace
{
FText JoinCardLines(const TArray<FText>& Lines)
{
	TArray<FString> Segments;
	for (const FText& Line : Lines)
	{
		if (!Line.IsEmpty())
		{
			Segments.Add(Line.ToString());
		}
	}

	return FText::FromString(FString::Join(Segments, TEXT("\n")));
}
}

void UFinalBattleCardEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		CardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CardButton"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CardLabel"));
		LabelText->SetAutoWrapText(false);
		CardButton->AddChild(LabelText);
		WidgetTree->RootWidget = CardButton;
	}

	if (CardButton)
	{
		CardButton->OnClicked.AddDynamic(this, &UFinalBattleCardEntryWidget::HandleButtonClicked);
	}
}

void UFinalBattleCardEntryWidget::Configure(UFinalBattleHandPanelController* InController, int32 InHandIndex, const FFinalBattleHUDCardEntry& InEntry)
{
	PanelController = InController;
	HandIndex = InHandIndex;
	TArray<FText> MetaSegments;
	if (InEntry.bRetained)
	{
		MetaSegments.Add(NSLOCTEXT("FinalBattleHUD", "CardRetainedTag", "保留"));
	}

	if (InEntry.bCollapsedCard)
	{
		MetaSegments.Add(NSLOCTEXT("FinalBattleHUD", "CardCollapsedTag", "崩溃"));
	}

	const FText KeywordText = !InEntry.KeywordText.IsEmpty()
		? InEntry.KeywordText
		: NSLOCTEXT("FinalBattleHUD", "NoCardKeywords", "无关键词");
	const FText OwnerText = !InEntry.OwnerDisplayName.IsEmpty()
		? FText::Format(NSLOCTEXT("FinalBattleHUD", "CardOwnerShort", "Owner {0}"), InEntry.OwnerDisplayName)
		: FText::GetEmpty();
	const FText MetaText = MetaSegments.Num() > 0
		? FText::FromString(FString::JoinBy(MetaSegments, TEXT(" · "), [](const FText& Entry) { return Entry.ToString(); }))
		: FText::GetEmpty();

	TArray<FText> Lines;
	Lines.Add(InEntry.DisplayName);
	Lines.Add(FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CardCostTypeLine", "AP {0} | {1}"),
		FText::AsNumber(InEntry.RuntimeCostAP),
		InEntry.TypeText));
	if (!OwnerText.IsEmpty())
	{
		Lines.Add(OwnerText);
	}
	Lines.Add(!MetaText.IsEmpty()
		? FText::Format(NSLOCTEXT("FinalBattleHUD", "CardKeywordMetaLine", "{0} | {1}"), KeywordText, MetaText)
		: KeywordText);
	CachedLabel = JoinCardLines(Lines);
	RebuildVisual();
}

void UFinalBattleCardEntryWidget::HandleButtonClicked()
{
	if (PanelController.IsValid())
	{
		PanelController->PlayCardByHandIndex(HandIndex);
	}
}

void UFinalBattleCardEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		LabelText->SetText(CachedLabel);
	}

	if (CardButton)
	{
		CardButton->SetBackgroundColor(FLinearColor(0.17f, 0.23f, 0.34f, 1.0f));
	}
}
