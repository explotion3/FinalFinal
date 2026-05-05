#include "UI/Widgets/Battle/FinalBattleCharacterDetailWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
#include "UI/Widgets/Battle/FinalBattleCharacterDetailPassiveLineWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterDetailStatusLineWidget.h"

void UFinalBattleCharacterDetailWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UFinalBattleCharacterDetailWidget::HandleCloseClicked);
	}
}

void UFinalBattleCharacterDetailWidget::ApplyCharacterDetailView(const FFinalBattleHUDCharacterDetailData& ViewData)
{
	CharacterDetailViewData = ViewData;
	RefreshBoundWidgets();
	OnCharacterDetailViewApplied(CharacterDetailViewData);
}

void UFinalBattleCharacterDetailWidget::RefreshBoundWidgets()
{
	const ESlateVisibility ContentVisibility = CharacterDetailViewData.bHasCharacter
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;
	const ESlateVisibility EmptyVisibility = CharacterDetailViewData.bHasCharacter
		? ESlateVisibility::Collapsed
		: ESlateVisibility::SelfHitTestInvisible;

	if (ContentRoot)
	{
		ContentRoot->SetVisibility(ContentVisibility);
	}

	if (EmptyText)
	{
		EmptyText->SetVisibility(EmptyVisibility);
		EmptyText->SetText(NSLOCTEXT("FinalBattleCharacterDetail", "NoInspectedCharacter", "未查看角色"));
	}

	if (!CharacterDetailViewData.bHasCharacter)
	{
		return;
	}

	if (TitleText)
	{
		TitleText->SetText(CharacterDetailViewData.DisplayName);
	}

	if (RoleText)
	{
		RoleText->SetText(CharacterDetailViewData.RoleText);
	}

	if (StateText)
	{
		StateText->SetText(CharacterDetailViewData.bCollapsed
			? NSLOCTEXT("FinalBattleCharacterDetail", "CollapsedState", "已崩溃")
			: NSLOCTEXT("FinalBattleCharacterDetail", "ActiveState", "可行动"));
	}

	if (StressText)
	{
		StressText->SetText(BuildStressText());
	}

	if (StressBar)
	{
		StressBar->SetPercent(CharacterDetailViewData.StressPercent);
	}

	if (BreakthroughText)
	{
		BreakthroughText->SetText(BuildBreakthroughText());
	}

	if (BreakthroughBar)
	{
		BreakthroughBar->SetPercent(CharacterDetailViewData.BreakthroughFillNormalized);
	}

	if (VitalText)
	{
		VitalText->SetText(BuildVitalText());
	}

	if (AwakenText)
	{
		AwakenText->SetText(BuildAwakenText());
	}

	if (CollapseText)
	{
		CollapseText->SetText(BuildCollapseText());
	}

	if (GrowthText)
	{
		GrowthText->SetText(BuildGrowthText());
	}

	if (RuntimeStatsText)
	{
		RuntimeStatsText->SetText(BuildRuntimeStatsText());
	}

	if (UltimateNameText)
	{
		UltimateNameText->SetText(CharacterDetailViewData.UltimateNameText);
	}

	if (UltimateCostText)
	{
		UltimateCostText->SetText(BuildUltimateCostText());
	}

	if (UltimateStateText)
	{
		UltimateStateText->SetText(BuildUltimateStateText());
	}

	if (UltimateRulesText)
	{
		UltimateRulesText->SetText(CharacterDetailViewData.UltimateRulesText);
	}

	RefreshStatusLines();
	RefreshPassiveLines();
}

void UFinalBattleCharacterDetailWidget::HandleCloseClicked()
{
	if (UFinalBattleCharacterDetailPanelController* DetailController = Cast<UFinalBattleCharacterDetailPanelController>(GetWidgetController()))
	{
		DetailController->ClearInspectedCharacter();
	}
}

FText UFinalBattleCharacterDetailWidget::BuildStressText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "StressTextFormat", "压力 {0}/{1}"),
		FText::AsNumber(CharacterDetailViewData.CurrentStress),
		FText::AsNumber(CharacterDetailViewData.StressCap));
}

FText UFinalBattleCharacterDetailWidget::BuildBreakthroughText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "BreakthroughTextFormat", "突破 {0}/{1}"),
		FText::AsNumber(CharacterDetailViewData.BreakthroughValue),
		FText::AsNumber(CharacterDetailViewData.BreakthroughRequiredValue));
}

FText UFinalBattleCharacterDetailWidget::BuildVitalText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "VitalTextFormat", "Vital {0}"),
		FText::AsNumber(CharacterDetailViewData.VitalShare));
}

FText UFinalBattleCharacterDetailWidget::BuildAwakenText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "AwakenTextFormat", "苏醒 {0}/{1}"),
		FText::AsNumber(CharacterDetailViewData.CurrentAwakenCount),
		FText::AsNumber(CharacterDetailViewData.CurrentAwakenThreshold));
}

FText UFinalBattleCharacterDetailWidget::BuildCollapseText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "CollapseTextFormat", "崩溃 {0}"),
		FText::AsNumber(CharacterDetailViewData.CollapseCount));
}

FText UFinalBattleCharacterDetailWidget::BuildGrowthText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "GrowthTextFormat", "根骨 {0} | 悟性 {1} | 杀意 {2}"),
		FText::AsNumber(CharacterDetailViewData.RootBone),
		FText::AsNumber(CharacterDetailViewData.Insight),
		FText::AsNumber(CharacterDetailViewData.KillingIntent));
}

FText UFinalBattleCharacterDetailWidget::BuildRuntimeStatsText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "RuntimeStatsTextFormat", "攻击 {0} | 防御 {1} | 削韧率 {2}%\n暴击 {3}% | 暴伤 {4}%"),
		FText::AsNumber(CharacterDetailViewData.RuntimeAttack),
		FText::AsNumber(CharacterDetailViewData.RuntimeDefense),
		FText::AsNumber(FMath::RoundToInt(CharacterDetailViewData.RuntimeBreakRate * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(CharacterDetailViewData.RuntimeCritChance * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(CharacterDetailViewData.RuntimeCritDamage * 100.0f)));
}

FText UFinalBattleCharacterDetailWidget::BuildUltimateCostText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetail", "UltimateCostTextFormat", "EP {0}"),
		FText::AsNumber(CharacterDetailViewData.UltimateCostEP));
}

FText UFinalBattleCharacterDetailWidget::BuildUltimateStateText() const
{
	if (!CharacterDetailViewData.bUltimateDefinitionReady)
	{
		return NSLOCTEXT("FinalBattleCharacterDetail", "UltimateUnavailable", "未就绪");
	}
	if (CharacterDetailViewData.bUltimateUsedThisBattle)
	{
		return NSLOCTEXT("FinalBattleCharacterDetail", "UltimateUsed", "已释放");
	}
	if (CharacterDetailViewData.bUltimateBlockedByCollapse)
	{
		return NSLOCTEXT("FinalBattleCharacterDetail", "UltimateBlockedByCollapse", "崩溃中");
	}
	return CharacterDetailViewData.bUltimateCanActivate
		? NSLOCTEXT("FinalBattleCharacterDetail", "UltimateReady", "可释放")
		: NSLOCTEXT("FinalBattleCharacterDetail", "UltimateNotReady", "未满足");
}

void UFinalBattleCharacterDetailWidget::RefreshStatusLines()
{
	if (StatusBox == nullptr)
	{
		return;
	}

	StatusBox->ClearChildren();
	if (EmptyStatusText)
	{
		const bool bHasStatuses = CharacterDetailViewData.Statuses.Num() > 0;
		EmptyStatusText->SetVisibility(bHasStatuses ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
		EmptyStatusText->SetText(NSLOCTEXT("FinalBattleCharacterDetail", "NoStatuses", "无状态"));
	}

	UClass* LineWidgetClass = StatusLineWidgetClass
		? StatusLineWidgetClass.Get()
		: UFinalUIWidgetClassSettings::GetBattleCharacterDetailStatusLineWidgetClass().Get();
	if (LineWidgetClass == nullptr)
	{
		LineWidgetClass = UFinalBattleCharacterDetailStatusLineWidget::StaticClass();
	}

	const int32 VisibleCount = FMath::Min(MaxVisibleStatusEntries, CharacterDetailViewData.Statuses.Num());
	for (int32 StatusIndex = 0; StatusIndex < VisibleCount; ++StatusIndex)
	{
		UFinalBattleCharacterDetailStatusLineWidget* StatusLineWidget = WidgetTree
			? WidgetTree->ConstructWidget<UFinalBattleCharacterDetailStatusLineWidget>(LineWidgetClass)
			: NewObject<UFinalBattleCharacterDetailStatusLineWidget>(this, LineWidgetClass);
		if (StatusLineWidget == nullptr)
		{
			continue;
		}

		StatusLineWidget->SetPresentationContext(GetWidgetController(), GetViewModel());
		StatusLineWidget->ApplyStatusLineView(CharacterDetailViewData.Statuses[StatusIndex]);
		StatusBox->AddChild(StatusLineWidget);
	}
}

void UFinalBattleCharacterDetailWidget::RefreshPassiveLines()
{
	if (PassiveBox == nullptr)
	{
		return;
	}

	PassiveBox->ClearChildren();
	if (EmptyPassiveText)
	{
		const bool bHasPassives = CharacterDetailViewData.Passives.Num() > 0;
		EmptyPassiveText->SetVisibility(bHasPassives ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
		EmptyPassiveText->SetText(NSLOCTEXT("FinalBattleCharacterDetail", "NoPassives", "无被动"));
	}

	UClass* LineWidgetClass = PassiveLineWidgetClass
		? PassiveLineWidgetClass.Get()
		: UFinalUIWidgetClassSettings::GetBattleCharacterDetailPassiveLineWidgetClass().Get();
	if (LineWidgetClass == nullptr)
	{
		LineWidgetClass = UFinalBattleCharacterDetailPassiveLineWidget::StaticClass();
	}

	const int32 VisibleCount = FMath::Min(MaxVisiblePassiveEntries, CharacterDetailViewData.Passives.Num());
	for (int32 PassiveIndex = 0; PassiveIndex < VisibleCount; ++PassiveIndex)
	{
		UFinalBattleCharacterDetailPassiveLineWidget* PassiveLineWidget = WidgetTree
			? WidgetTree->ConstructWidget<UFinalBattleCharacterDetailPassiveLineWidget>(LineWidgetClass)
			: NewObject<UFinalBattleCharacterDetailPassiveLineWidget>(this, LineWidgetClass);
		if (PassiveLineWidget == nullptr)
		{
			continue;
		}

		PassiveLineWidget->SetPresentationContext(GetWidgetController(), GetViewModel());
		PassiveLineWidget->ApplyPassiveLineView(CharacterDetailViewData.Passives[PassiveIndex]);
		PassiveBox->AddChild(PassiveLineWidget);
	}
}
