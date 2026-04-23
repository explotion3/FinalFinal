#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

namespace
{
FText JoinEnemyStatusTextArray(const TArray<FText>& Texts, const FText& EmptyText)
{
	if (Texts.Num() == 0)
	{
		return EmptyText;
	}

	TArray<FString> Segments;
	for (const FText& Entry : Texts)
	{
		Segments.Add(Entry.ToString());
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
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
		: NSLOCTEXT("FinalBattleHUD", "EnemyPhaseProgressUnavailable", "阶段信息未就绪");
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "EnemyEntryFormat", "#{0} {1}\nHP {2}/{3}  Shield {4}  Break {5}/{6}  Init {7}\n{8}\n{9}\nStatus: {10}"),
		FText::AsNumber(InEntry.PositionIndex),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.CurrentHP),
		FText::AsNumber(InEntry.MaxHP),
		FText::AsNumber(InEntry.CurrentShield),
		FText::AsNumber(InEntry.CurrentBreakValue),
		FText::AsNumber(InEntry.MaxBreakValue),
		FText::AsNumber(InEntry.CurrentInitiative),
		InEntry.bActedThisRound
			? FText::Format(NSLOCTEXT("FinalBattleHUD", "EnemyIntentActed", "{0} | 本回合已行动"), InEntry.IntentText)
			: InEntry.IntentText,
		PhaseText,
		JoinEnemyStatusTextArray(InEntry.StatusTexts, NSLOCTEXT("FinalBattleHUD", "NoEnemyStatus", "无")));
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
