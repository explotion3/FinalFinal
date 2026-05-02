#include "UI/Widgets/Battle/FinalBattleEnemyOverheadWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "Subsystems/UI/FinalUISubsystem.h"

void UFinalBattleEnemyOverheadWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (InspectButton)
	{
		InspectButton->OnClicked.AddDynamic(this, &UFinalBattleEnemyOverheadWidget::HandleInspectButtonClicked);
	}
}

FReply UFinalBattleEnemyOverheadWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InspectButton == nullptr && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && TryInspectEnemy())
	{
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFinalBattleEnemyOverheadWidget::ApplyEnemyOverheadView(const FFinalBattleEnemyOverheadViewData& ViewData)
{
	EnemyOverheadViewData = ViewData;
	RefreshBoundWidgets();
	OnEnemyOverheadViewApplied(EnemyOverheadViewData);
}

void UFinalBattleEnemyOverheadWidget::RefreshBoundWidgets()
{
	SetRenderOpacity(EnemyOverheadViewData.bIsAlive ? 1.0f : DefeatedOpacity);
	const ESlateVisibility ActiveVisibility = bAllowInspectOnClick
		? ESlateVisibility::Visible
		: ESlateVisibility::SelfHitTestInvisible;
	if (bHideDefeatedWidget)
	{
		SetVisibility(EnemyOverheadViewData.bIsAlive ? ActiveVisibility : ESlateVisibility::Collapsed);
	}
	else
	{
		SetVisibility(ActiveVisibility);
	}

	if (NameText)
	{
		NameText->SetText(EnemyOverheadViewData.DisplayName);
	}

	if (HPText)
	{
		HPText->SetText(BuildHPText());
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(EnemyOverheadViewData.HealthPercent);
	}

	if (ShieldFrameBar)
	{
		ShieldFrameBar->SetPercent(EnemyOverheadViewData.ShieldFramePercent);
		ShieldFrameBar->SetVisibility(
			!bHideShieldFrameWhenEmpty || EnemyOverheadViewData.CurrentShield > 0
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed);
	}

	if (BreakBar)
	{
		BreakBar->SetPercent(EnemyOverheadViewData.BreakPercent);
	}

	if (InitiativeText)
	{
		InitiativeText->SetText(!EnemyOverheadViewData.InitiativeText.IsEmpty()
			? EnemyOverheadViewData.InitiativeText
			: FText::AsNumber(EnemyOverheadViewData.CurrentInitiative));
	}

	if (IntentText)
	{
		IntentText->SetText(EnemyOverheadViewData.IntentText);
	}

	if (TargetedVisual)
	{
		TargetedVisual->SetVisibility(EnemyOverheadViewData.bIsTargeted
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	}

	if (DefeatedVisual)
	{
		DefeatedVisual->SetVisibility(!EnemyOverheadViewData.bIsAlive
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	}

	if (StatusBox)
	{
		StatusBox->ClearChildren();

		const int32 VisibleCount = FMath::Min(MaxVisibleStatusEntries, EnemyOverheadViewData.Statuses.Num());
		for (int32 StatusIndex = 0; StatusIndex < VisibleCount; ++StatusIndex)
		{
			const FFinalBattleOverheadStatusViewData& StatusView = EnemyOverheadViewData.Statuses[StatusIndex];
			UTextBlock* StatusText = WidgetTree
				? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
				: NewObject<UTextBlock>(this);
			if (StatusText == nullptr)
			{
				continue;
			}

			StatusText->SetText(BuildStatusText(StatusView));
			StatusText->SetFont(FSlateFontInfo(StatusText->GetFont().FontObject, 12));
			StatusBox->AddChild(StatusText);
		}

		if (EnemyOverheadViewData.Statuses.Num() > VisibleCount)
		{
			UTextBlock* OverflowText = WidgetTree
				? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
				: NewObject<UTextBlock>(this);
			if (OverflowText)
			{
				OverflowText->SetText(FText::FromString(FString::Printf(TEXT("+%d"), EnemyOverheadViewData.Statuses.Num() - VisibleCount)));
				OverflowText->SetFont(FSlateFontInfo(OverflowText->GetFont().FontObject, 12));
				StatusBox->AddChild(OverflowText);
			}
		}
	}
}

void UFinalBattleEnemyOverheadWidget::HandleInspectButtonClicked()
{
	InspectEnemy();
}

bool UFinalBattleEnemyOverheadWidget::InspectEnemy()
{
	return TryInspectEnemy();
}

bool UFinalBattleEnemyOverheadWidget::TryInspectEnemy() const
{
	if (!bAllowInspectOnClick || EnemyOverheadViewData.RuntimeUnitId.IsNone())
	{
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		return false;
	}

	const UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>();
	UFinalBattleWidgetController* BattleWidgetController = UISubsystem ? UISubsystem->GetBattleWidgetController() : nullptr;
	return BattleWidgetController ? BattleWidgetController->InspectEnemyByUnitId(EnemyOverheadViewData.RuntimeUnitId) : false;
}

FText UFinalBattleEnemyOverheadWidget::BuildHPText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleEnemyOverhead", "HPTextFormat", "{0}/{1}"),
		FText::AsNumber(EnemyOverheadViewData.CurrentHP),
		FText::AsNumber(EnemyOverheadViewData.MaxHP));
}

FText UFinalBattleEnemyOverheadWidget::BuildStatusText(const FFinalBattleOverheadStatusViewData& StatusView) const
{
	if (StatusView.CurrentStacks > 1)
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleEnemyOverhead", "StatusWithStacksFormat", "{0} x{1}"),
			StatusView.DisplayName,
			FText::AsNumber(StatusView.CurrentStacks));
	}

	return StatusView.DisplayName;
}
