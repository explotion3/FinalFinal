#include "UI/Widgets/Battle/FinalBattleEnemyDetailWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
#include "UI/Widgets/Battle/FinalBattleEnemyDetailStatusLineWidget.h"

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
		if (IntentIconImage)
		{
			IntentIconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
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

	if (IntentNameText)
	{
		IntentNameText->SetText(!EnemyDetailViewData.IntentNameText.IsEmpty()
			? EnemyDetailViewData.IntentNameText
			: EnemyDetailViewData.IntentText);
	}

	if (IntentDetailText)
	{
		IntentDetailText->SetText(EnemyDetailViewData.IntentText);
	}

	RefreshIntentIcon();

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

	RefreshStatusLines();
}

void UFinalBattleEnemyDetailWidget::HandleCloseClicked()
{
	if (UFinalBattleEnemyDetailPanelController* DetailController = Cast<UFinalBattleEnemyDetailPanelController>(GetWidgetController()))
	{
		DetailController->ClearInspectedEnemy();
	}
}

void UFinalBattleEnemyDetailWidget::RefreshIntentIcon()
{
	if (IntentIconImage == nullptr)
	{
		return;
	}

	if (const FSlateBrush* IntentBrush = IntentIconBrushes.Find(EnemyDetailViewData.IntentIconId))
	{
		IntentIconImage->SetBrush(*IntentBrush);
		IntentIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		return;
	}

	if (bHideIntentIconWhenMissingBrush)
	{
		IntentIconImage->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	IntentIconImage->SetBrush(DefaultIntentIconBrush);
	IntentIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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

void UFinalBattleEnemyDetailWidget::RefreshStatusLines()
{
	if (EmptyStatusText)
	{
		const bool bHasStatuses = EnemyDetailViewData.Statuses.Num() > 0;
		EmptyStatusText->SetVisibility(bHasStatuses ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
		EmptyStatusText->SetText(NSLOCTEXT("FinalBattleEnemyDetail", "NoStatuses", "无状态"));
	}

	if (StatusBox == nullptr)
	{
		return;
	}

	StatusBox->ClearChildren();

	UClass* LineWidgetClass = StatusLineWidgetClass
		? StatusLineWidgetClass.Get()
		: UFinalUIWidgetClassSettings::GetBattleEnemyDetailStatusLineWidgetClass().Get();
	if (LineWidgetClass == nullptr)
	{
		LineWidgetClass = UFinalBattleEnemyDetailStatusLineWidget::StaticClass();
	}

	const int32 VisibleCount = FMath::Min(MaxVisibleStatusEntries, EnemyDetailViewData.Statuses.Num());
	for (int32 StatusIndex = 0; StatusIndex < VisibleCount; ++StatusIndex)
	{
		UFinalBattleEnemyDetailStatusLineWidget* StatusLineWidget = WidgetTree
			? WidgetTree->ConstructWidget<UFinalBattleEnemyDetailStatusLineWidget>(LineWidgetClass)
			: NewObject<UFinalBattleEnemyDetailStatusLineWidget>(this, LineWidgetClass);
		if (StatusLineWidget == nullptr)
		{
			continue;
		}

		StatusLineWidget->SetPresentationContext(GetWidgetController(), GetViewModel());
		StatusLineWidget->ApplyStatusLineView(EnemyDetailViewData.Statuses[StatusIndex]);
		StatusBox->AddChild(StatusLineWidget);
	}

	if (EnemyDetailViewData.Statuses.Num() > VisibleCount)
	{
		UTextBlock* OverflowText = WidgetTree
			? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
			: NewObject<UTextBlock>(this);
		if (OverflowText)
		{
			OverflowText->SetText(FText::FromString(FString::Printf(TEXT("+%d"), EnemyDetailViewData.Statuses.Num() - VisibleCount)));
			StatusBox->AddChild(OverflowText);
		}
	}
}
