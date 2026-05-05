#include "UI/Widgets/Battle/FinalBattleCardZoneEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/DataTable.h"

namespace
{
UTextBlock* CreateFallbackCardZoneText(UWidgetTree& WidgetTree, const FName WidgetName, const int32 FontSize)
{
	UTextBlock* TextBlock = WidgetTree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
	if (TextBlock)
	{
		TextBlock->SetAutoWrapText(true);
		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = FontSize;
		TextBlock->SetFont(FontInfo);
	}
	return TextBlock;
}

UDataTable* GetCardRichTextStyleSet()
{
	static TWeakObjectPtr<UDataTable> CachedStyleSet;
	if (CachedStyleSet.IsValid())
	{
		return CachedStyleSet.Get();
	}

	UDataTable* StyleSet = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/BattleHUD/DT_CardRichTextStyle.DT_CardRichTextStyle"));
	CachedStyleSet = StyleSet;
	return StyleSet;
}
}

void UFinalBattleCardZoneEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardZoneEntryFallbackRoot"));
		NameText = CreateFallbackCardZoneText(*WidgetTree, TEXT("NameText"), 14);
		TypeText = CreateFallbackCardZoneText(*WidgetTree, TEXT("TypeText"), 12);
		CostText = CreateFallbackCardZoneText(*WidgetTree, TEXT("CostText"), 12);
		OwnerText = CreateFallbackCardZoneText(*WidgetTree, TEXT("OwnerText"), 11);
		KeywordText = CreateFallbackCardZoneText(*WidgetTree, TEXT("KeywordText"), 11);
		RulesText = WidgetTree->ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass(), TEXT("RulesText"));
		if (RulesText)
		{
			RulesText->SetTextStyleSet(GetCardRichTextStyleSet());
		}

		if (NameText) { RootBox->AddChild(NameText); }
		if (TypeText) { RootBox->AddChild(TypeText); }
		if (CostText) { RootBox->AddChild(CostText); }
		if (OwnerText) { RootBox->AddChild(OwnerText); }
		if (KeywordText) { RootBox->AddChild(KeywordText); }
		if (RulesText) { RootBox->AddChild(RulesText); }

		WidgetTree->RootWidget = RootBox;
	}
}

void UFinalBattleCardZoneEntryWidget::ApplyCardZoneEntryView(const FFinalBattleHUDCardZoneEntry& ViewData)
{
	CachedViewData = ViewData;
	RefreshBoundWidgets();
	OnCardZoneEntryViewApplied(CachedViewData);
}

const FFinalBattleHUDCardZoneEntry& UFinalBattleCardZoneEntryWidget::GetCardZoneEntryViewData() const
{
	return CachedViewData;
}

void UFinalBattleCardZoneEntryWidget::RefreshBoundWidgets()
{
	if (NameText)
	{
		NameText->SetText(CachedViewData.DisplayName);
	}

	if (TypeText)
	{
		TypeText->SetText(CachedViewData.TypeText);
	}

	if (CostText)
	{
		CostText->SetText(FText::Format(
			NSLOCTEXT("FinalBattleHUD", "CardZoneEntryCostFormat", "费用 {0}"),
			FText::AsNumber(CachedViewData.RuntimeCostAP)));
	}

	if (OwnerText)
	{
		OwnerText->SetText(!CachedViewData.OwnerDisplayName.IsEmpty()
			? CachedViewData.OwnerDisplayName
			: FText::FromName(CachedViewData.OwnerUnitId));
	}

	if (KeywordText)
	{
		TArray<FString> Flags;
		if (!CachedViewData.KeywordText.IsEmpty())
		{
			Flags.Add(CachedViewData.KeywordText.ToString());
		}
		if (CachedViewData.bRetained)
		{
			Flags.Add(TEXT("保留"));
		}
		if (CachedViewData.bConsumeOnPlay)
		{
			Flags.Add(TEXT("消耗"));
		}
		if (CachedViewData.bOngoingCard)
		{
			Flags.Add(TEXT("持续"));
		}
		if (CachedViewData.bGeneratedCard)
		{
			Flags.Add(TEXT("衍生"));
		}
		if (CachedViewData.bTemporaryCard)
		{
			Flags.Add(TEXT("临时"));
		}
		KeywordText->SetText(Flags.Num() > 0 ? FText::FromString(FString::Join(Flags, TEXT(" | "))) : FText::GetEmpty());
	}

	if (RulesText)
	{
		RulesText->SetTextStyleSet(GetCardRichTextStyleSet());
		RulesText->SetText(CachedViewData.RulesText);
	}
}
