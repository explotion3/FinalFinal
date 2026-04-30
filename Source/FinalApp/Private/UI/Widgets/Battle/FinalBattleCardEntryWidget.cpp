#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "Engine/DataTable.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

namespace
{
UTextBlock* CreateFallbackTextBlock(UWidgetTree& WidgetTree, const FName WidgetName, const int32 FontSize)
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

void UFinalBattleCardEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		bUsesFallbackLayout = true;
		CardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CardButton"));
		UVerticalBox* FallbackBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardFallbackBox"));
		CostText = CreateFallbackTextBlock(*WidgetTree, TEXT("CostText"), 18);
		NameText = CreateFallbackTextBlock(*WidgetTree, TEXT("NameText"), 16);
		TypeText = CreateFallbackTextBlock(*WidgetTree, TEXT("TypeText"), 14);
		DescriptionText = WidgetTree->ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass(), TEXT("DescriptionText"));
		if (DescriptionText)
		{
			DescriptionText->SetTextStyleSet(GetCardRichTextStyleSet());
		}

		if (FallbackBox)
		{
			if (CostText)
			{
				FallbackBox->AddChild(CostText);
			}
			if (NameText)
			{
				FallbackBox->AddChild(NameText);
			}
			if (TypeText)
			{
				FallbackBox->AddChild(TypeText);
			}
			if (DescriptionText)
			{
				FallbackBox->AddChild(DescriptionText);
			}
		}

		if (CardButton && FallbackBox)
		{
			CardButton->AddChild(FallbackBox);
		}
		WidgetTree->RootWidget = CardButton;
	}

	if (CardButton)
	{
		CardButton->OnClicked.AddDynamic(this, &UFinalBattleCardEntryWidget::HandleButtonClicked);
		CardButton->OnHovered.AddDynamic(this, &UFinalBattleCardEntryWidget::HandleButtonHovered);
		CardButton->OnUnhovered.AddDynamic(this, &UFinalBattleCardEntryWidget::HandleButtonUnhovered);
	}
}

void UFinalBattleCardEntryWidget::Configure(UFinalBattleHandPanelController* InController, int32 InHandIndex, const FFinalBattleHUDCardEntry& InEntry)
{
	PanelController = InController;
	CardInstanceId = InEntry.CardInstanceId;
	HandIndex = InHandIndex;

	CachedCostText = FText::AsNumber(InEntry.RuntimeCostAP);
	CachedNameText = InEntry.DisplayName;
	CachedTypeText = InEntry.TypeText;
	CachedDescriptionText = !InEntry.RulesText.IsEmpty() ? InEntry.RulesText : InEntry.KeywordText;
	RebuildVisual();
}

void UFinalBattleCardEntryWidget::HandleButtonClicked()
{
	if (PanelController.IsValid())
	{
		PanelController->PlayCardByHandIndex(HandIndex);
	}
}

void UFinalBattleCardEntryWidget::HandleButtonHovered()
{
	OnCardHoverChanged.Broadcast(CardInstanceId, HandIndex, true);
}

void UFinalBattleCardEntryWidget::HandleButtonUnhovered()
{
	OnCardHoverChanged.Broadcast(CardInstanceId, HandIndex, false);
}

void UFinalBattleCardEntryWidget::RebuildVisual()
{
	if (CostText)
	{
		CostText->SetText(CachedCostText);
	}

	if (NameText)
	{
		NameText->SetText(CachedNameText);
	}

	if (TypeText)
	{
		TypeText->SetText(CachedTypeText);
	}

	if (DescriptionText)
	{
		DescriptionText->SetTextStyleSet(GetCardRichTextStyleSet());
		DescriptionText->SetText(CachedDescriptionText);
	}

	if (CardButton && bUsesFallbackLayout)
	{
		CardButton->SetBackgroundColor(FLinearColor(0.17f, 0.23f, 0.34f, 1.0f));
	}
}
