#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

void UFinalBattleCardEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		CardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CardButton"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CardLabel"));
		LabelText->SetAutoWrapText(true);
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
	TArray<FString> MetaSegments;
	if (InEntry.bRetained)
	{
		MetaSegments.Add(TEXT("保留"));
	}

	if (InEntry.bCollapsedCard)
	{
		MetaSegments.Add(TEXT("崩溃牌"));
	}

	if (!InEntry.RulesText.IsEmpty())
	{
		MetaSegments.Add(InEntry.RulesText.ToString());
	}

	const FText MetaText = MetaSegments.Num() > 0
		? FText::FromString(FString::Join(MetaSegments, TEXT(" | ")))
		: NSLOCTEXT("FinalBattleHUD", "NoCardRules", "无额外说明");
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CardEntryFormat", "{0}\n{1} | AP {2} | Owner {3}\n{4}\n{5}"),
		InEntry.DisplayName,
		InEntry.TypeText,
		FText::AsNumber(InEntry.RuntimeCostAP),
		!InEntry.OwnerDisplayName.IsEmpty() ? InEntry.OwnerDisplayName : NSLOCTEXT("FinalBattleHUD", "UnknownCardOwner", "未知"),
		!InEntry.KeywordText.IsEmpty()
			? InEntry.KeywordText
			: NSLOCTEXT("FinalBattleHUD", "NoCardKeywords", "无关键词"),
		MetaText);
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
