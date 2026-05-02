#include "UI/Widgets/Battle/FinalBattleEnemyDetailWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"

void UFinalBattleEnemyDetailWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UFinalBattleEnemyDetailWidget::HandleCloseClicked);
	}
}

void UFinalBattleEnemyDetailWidget::ApplyEnemyDetailView(const FFinalBattleHUDEnemyDetailData& ViewData)
{
	EnemyDetailViewData = ViewData;
	RefreshBoundWidgets();
	OnEnemyDetailViewApplied(EnemyDetailViewData);
}

void UFinalBattleEnemyDetailWidget::RefreshBoundWidgets()
{
	const ESlateVisibility ContentVisibility = EnemyDetailViewData.bHasEnemy
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;
	const ESlateVisibility EmptyVisibility = EnemyDetailViewData.bHasEnemy
		? ESlateVisibility::Collapsed
		: ESlateVisibility::SelfHitTestInvisible;

	if (ContentRoot)
	{
		ContentRoot->SetVisibility(ContentVisibility);
	}

	if (EmptyText)
	{
		EmptyText->SetVisibility(EmptyVisibility);
		EmptyText->SetText(NSLOCTEXT("FinalBattleEnemyDetail", "NoInspectedEnemy", "未查看敌人"));
	}

	if (!EnemyDetailViewData.bHasEnemy)
	{
		return;
	}

	if (TitleText)
	{
		TitleText->SetText(EnemyDetailViewData.DisplayName);
	}

	if (HPText)
	{
		HPText->SetText(BuildHPText());
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(EnemyDetailViewData.HealthPercent);
	}

	if (ShieldText)
	{
		ShieldText->SetText(BuildShieldText());
	}

	if (ShieldFrameBar)
	{
		ShieldFrameBar->SetPercent(EnemyDetailViewData.ShieldFramePercent);
		ShieldFrameBar->SetVisibility(EnemyDetailViewData.CurrentShield > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (BreakText)
	{
		BreakText->SetText(BuildBreakText());
	}

	if (BreakBar)
	{
		BreakBar->SetPercent(EnemyDetailViewData.BreakPercent);
	}

	if (InitiativeText)
	{
		InitiativeText->SetText(!EnemyDetailViewData.InitiativeText.IsEmpty()
			? EnemyDetailViewData.InitiativeText
			: FText::AsNumber(EnemyDetailViewData.CurrentInitiative));
	}

	if (IntentText)
	{
		IntentText->SetText(EnemyDetailViewData.IntentText);
	}

	if (PhaseText)
	{
		PhaseText->SetText(EnemyDetailViewData.PhaseProgressText);
	}

	if (RankText)
	{
		RankText->SetText(EnemyDetailViewData.EnemyRankTag.IsNone()
			? FText::GetEmpty()
			: FText::FromName(EnemyDetailViewData.EnemyRankTag));
	}

	if (TargetStateText)
	{
		if (!EnemyDetailViewData.bIsAlive)
		{
			TargetStateText->SetText(NSLOCTEXT("FinalBattleEnemyDetail", "DefeatedState", "已击败"));
		}
		else if (EnemyDetailViewData.bIsCurrentBattleTarget)
		{
			TargetStateText->SetText(NSLOCTEXT("FinalBattleEnemyDetail", "CurrentTargetState", "当前目标"));
		}
		else
		{
			TargetStateText->SetText(NSLOCTEXT("FinalBattleEnemyDetail", "InspectedState", "正在查看"));
		}
	}

	if (StatusBox)
	{
		StatusBox->ClearChildren();
		const int32 VisibleCount = FMath::Min(MaxVisibleStatusEntries, EnemyDetailViewData.Statuses.Num());
		for (int32 StatusIndex = 0; StatusIndex < VisibleCount; ++StatusIndex)
		{
			UTextBlock* StatusText = WidgetTree
				? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
				: NewObject<UTextBlock>(this);
			if (StatusText == nullptr)
			{
				continue;
			}

			StatusText->SetAutoWrapText(true);
			StatusText->SetText(BuildStatusText(EnemyDetailViewData.Statuses[StatusIndex]));
			StatusBox->AddChild(StatusText);
		}
	}
}

void UFinalBattleEnemyDetailWidget::HandleCloseClicked()
{
	if (UFinalBattleEnemyDetailPanelController* DetailController = Cast<UFinalBattleEnemyDetailPanelController>(GetWidgetController()))
	{
		DetailController->ClearInspectedEnemy();
	}
}

FText UFinalBattleEnemyDetailWidget::BuildHPText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleEnemyDetail", "HPTextFormat", "HP {0}/{1}"),
		FText::AsNumber(EnemyDetailViewData.CurrentHP),
		FText::AsNumber(EnemyDetailViewData.MaxHP));
}

FText UFinalBattleEnemyDetailWidget::BuildShieldText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleEnemyDetail", "ShieldTextFormat", "护盾 {0}"),
		FText::AsNumber(EnemyDetailViewData.CurrentShield));
}

FText UFinalBattleEnemyDetailWidget::BuildBreakText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleEnemyDetail", "BreakTextFormat", "Break {0}/{1}"),
		FText::AsNumber(EnemyDetailViewData.CurrentBreakValue),
		FText::AsNumber(EnemyDetailViewData.MaxBreakValue));
}

FText UFinalBattleEnemyDetailWidget::BuildStatusText(const FFinalBattleHUDEnemyDetailStatusEntry& StatusView) const
{
	FText HeaderText;
	if (StatusView.RemainingDuration > 0)
	{
		HeaderText = FText::Format(
			NSLOCTEXT("FinalBattleEnemyDetail", "StatusHeaderWithDuration", "{0} x{1} ({2})"),
			StatusView.DisplayName,
			FText::AsNumber(StatusView.CurrentStacks),
			FText::AsNumber(StatusView.RemainingDuration));
	}
	else
	{
		HeaderText = FText::Format(
			NSLOCTEXT("FinalBattleEnemyDetail", "StatusHeader", "{0} x{1}"),
			StatusView.DisplayName,
			FText::AsNumber(StatusView.CurrentStacks));
	}

	if (StatusView.SummaryText.IsEmpty())
	{
		return HeaderText;
	}

	return FText::Format(
		NSLOCTEXT("FinalBattleEnemyDetail", "StatusWithSummary", "{0}\n{1}"),
		HeaderText,
		StatusView.SummaryText);
}
