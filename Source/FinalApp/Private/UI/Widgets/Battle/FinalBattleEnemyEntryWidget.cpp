#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

namespace
{
FText BuildEnemyStatusSummary(const TArray<FText>& Texts, const FText& EmptyText)
{
	if (Texts.Num() == 0)
	{
		return EmptyText;
	}

	TArray<FString> Segments;
	for (int32 Index = 0; Index < FMath::Min(Texts.Num(), 2); ++Index)
	{
		Segments.Add(Texts[Index].ToString());
	}

	if (Texts.Num() > 2)
	{
		Segments.Add(FString::Printf(TEXT("+%d"), Texts.Num() - 2));
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
}

FText TruncateEnemyLine(const FText& Text, const int32 MaxChars = 40)
{
	const FString Source = Text.ToString();
	if (Source.Len() <= MaxChars)
	{
		return Text;
	}

	return FText::FromString(Source.Left(MaxChars - 1) + TEXT("…"));
}
}

void UFinalBattleEnemyEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		SelectButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EnemyButton"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EnemyLabel"));
		LabelText->SetAutoWrapText(true);
		SelectButton->AddChild(LabelText);
		WidgetTree->RootWidget = SelectButton;
	}

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UFinalBattleEnemyEntryWidget::HandleButtonClicked);
	}
}

void UFinalBattleEnemyEntryWidget::Configure(UFinalBattleEnemyPanelController* InController, const FFinalBattleHUDEnemyEntry& InEntry)
{
	PanelController = InController;
	RuntimeUnitId = InEntry.RuntimeUnitId;
	const FText PhaseText = !InEntry.PhaseProgressText.IsEmpty()
		? InEntry.PhaseProgressText
		: FText::GetEmpty();
	const FText IntentText = InEntry.bActedThisRound
		? FText::Format(NSLOCTEXT("FinalBattleHUD", "EnemyIntentActed", "{0} | 已行动"), TruncateEnemyLine(InEntry.IntentText))
		: TruncateEnemyLine(InEntry.IntentText);
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "EnemyEntryFormat", "#{0} {1}\nHP {2}/{3} | Shield {4}\nBreak {5}/{6} | Init {7}\n{8}{9}\n状态: {10}"),
		FText::AsNumber(InEntry.PositionIndex),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.CurrentHP),
		FText::AsNumber(InEntry.MaxHP),
		FText::AsNumber(InEntry.CurrentShield),
		FText::AsNumber(InEntry.CurrentBreakValue),
		FText::AsNumber(InEntry.MaxBreakValue),
		FText::AsNumber(InEntry.CurrentInitiative),
		IntentText,
		!PhaseText.IsEmpty() ? FText::Format(NSLOCTEXT("FinalBattleHUD", "EnemyPhaseSuffix", "\n{0}"), PhaseText) : FText::GetEmpty(),
		BuildEnemyStatusSummary(InEntry.StatusTexts, NSLOCTEXT("FinalBattleHUD", "NoEnemyStatus", "无")));
	bSelected = InEntry.bSelected;
	RebuildVisual();
}

void UFinalBattleEnemyEntryWidget::HandleButtonClicked()
{
	if (PanelController.IsValid())
	{
		PanelController->SelectEnemyByUnitId(RuntimeUnitId);
	}
}

void UFinalBattleEnemyEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		if (bSelected)
		{
			LabelText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "SelectedEnemyPrefix", "[目标]\n{0}"), CachedLabel));
		}
		else
		{
			LabelText->SetText(CachedLabel);
		}
	}

	if (SelectButton)
	{
		SelectButton->SetBackgroundColor(bSelected ? FLinearColor(0.82f, 0.24f, 0.24f, 1.0f) : FLinearColor(0.18f, 0.18f, 0.18f, 1.0f));
	}
}
