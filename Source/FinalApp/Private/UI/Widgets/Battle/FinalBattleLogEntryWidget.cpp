#include "UI/Widgets/Battle/FinalBattleLogEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

namespace
{
FText BuildLogEntryText(const FFinalBattleHUDLogEntry& Entry)
{
	TArray<FString> Lines;
	if (!Entry.TitleText.IsEmpty())
	{
		Lines.Add(Entry.TitleText.ToString());
	}
	if (!Entry.SummaryText.IsEmpty())
	{
		Lines.Add(Entry.SummaryText.ToString());
	}
	if (!Entry.DetailText.IsEmpty())
	{
		Lines.Add(Entry.DetailText.ToString());
	}

	return Lines.Num() > 0
		? FText::FromString(FString::Join(Lines, TEXT("\n")))
		: NSLOCTEXT("FinalBattleHUD", "EmptyLogEntry", "无公开事件详情");
}
}

void UFinalBattleLogEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BattleLogEntryBorder"));
		RootBorder->SetBrushColor(FLinearColor(0.13f, 0.13f, 0.14f, 0.95f));
		RootBorder->SetPadding(FMargin(8.0f));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BattleLogEntryLabel"));
		LabelText->SetAutoWrapText(true);
		RootBorder->SetContent(LabelText);
		WidgetTree->RootWidget = RootBorder;
	}
}

void UFinalBattleLogEntryWidget::Configure(const FFinalBattleHUDLogEntry& InEntry)
{
	CachedLabel = BuildLogEntryText(InEntry);
	RebuildVisual();
}

void UFinalBattleLogEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		LabelText->SetText(CachedLabel);
	}
}
