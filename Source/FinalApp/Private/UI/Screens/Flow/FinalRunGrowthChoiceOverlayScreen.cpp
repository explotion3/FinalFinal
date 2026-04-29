#include "UI/Screens/Flow/FinalRunGrowthChoiceOverlayScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "UI/Screens/Flow/FinalRunFlowOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunFlowScreenUtils.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"

using namespace FinalRunFlowScreenUtils;

namespace
{
FText FormatGrowthChoiceTypeText(const EFinalGrowthChoiceType ChoiceType)
{
	switch (ChoiceType)
	{
	case EFinalGrowthChoiceType::AttributeGrowth:
		return NSLOCTEXT("FinalFlowUI", "GrowthChoiceTypeAttribute", "属性成长");

	case EFinalGrowthChoiceType::CardEvolution:
		return NSLOCTEXT("FinalFlowUI", "GrowthChoiceTypeEvolution", "卡牌进化");

	case EFinalGrowthChoiceType::Special:
		return NSLOCTEXT("FinalFlowUI", "GrowthChoiceTypeSpecial", "特殊成长");

	default:
		return NSLOCTEXT("FinalFlowUI", "GrowthChoiceTypeUnknown", "未知成长");
	}
}

FText FormatGrowthAttributeTypeText(const EFinalGrowthAttributeType AttributeType)
{
	switch (AttributeType)
	{
	case EFinalGrowthAttributeType::RootBone:
		return NSLOCTEXT("FinalFlowUI", "GrowthAttributeRootBone", "根骨");

	case EFinalGrowthAttributeType::Insight:
		return NSLOCTEXT("FinalFlowUI", "GrowthAttributeInsight", "悟性");

	case EFinalGrowthAttributeType::KillingIntent:
		return NSLOCTEXT("FinalFlowUI", "GrowthAttributeKillingIntent", "杀意");

	default:
		return NSLOCTEXT("FinalFlowUI", "GrowthAttributeUnknown", "未知属性");
	}
}

FText BuildGrowthChoiceTitle(const FFinalRunGrowthChoiceInstance& Choice)
{
	if (!Choice.DisplayName.IsEmpty())
	{
		return Choice.DisplayName;
	}

	if (Choice.ChoiceType == EFinalGrowthChoiceType::AttributeGrowth)
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "GrowthChoiceAttributeTitleFallback", "{0} +{1}"),
			FormatGrowthAttributeTypeText(Choice.AttributeType),
			FText::AsNumber(Choice.AttributeDelta));
	}

	if (Choice.ChoiceType == EFinalGrowthChoiceType::CardEvolution)
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "GrowthChoiceEvolutionTitleFallback", "{0} -> {1}"),
			Choice.FromCardId.IsValid() ? FText::FromName(Choice.FromCardId.Value) : NSLOCTEXT("FinalFlowUI", "GrowthChoiceEvolutionFromUnknown", "旧卡"),
			Choice.ToCardId.IsValid() ? FText::FromName(Choice.ToCardId.Value) : NSLOCTEXT("FinalFlowUI", "GrowthChoiceEvolutionToUnknown", "新卡"));
	}

	return NSLOCTEXT("FinalFlowUI", "GrowthChoiceTitleUnknown", "未命名成长");
}

FFinalRunFlowOptionButtonData BuildGrowthChoiceOptionData(const FFinalRunGrowthChoiceInstance& Choice, const int32 ChoiceIndex, const bool bSelected)
{
	FFinalRunFlowOptionButtonData Data;
	Data.Kind = EFinalRunFlowOptionKind::GrowthChoice;
	Data.PayloadId = Choice.ChoiceInstanceId;
	Data.PayloadIndex = ChoiceIndex;
	Data.Title = BuildGrowthChoiceTitle(Choice);
	Data.Subtitle = FormatGrowthChoiceTypeText(Choice.ChoiceType);
	Data.bEnabled = Choice.IsValid();

	if (Choice.ChoiceType == EFinalGrowthChoiceType::AttributeGrowth)
	{
		Data.Meta = FText::Format(
			NSLOCTEXT("FinalFlowUI", "GrowthChoiceAttributeMeta", "目标属性：{0} | 变化：+{1}"),
			FormatGrowthAttributeTypeText(Choice.AttributeType),
			FText::AsNumber(Choice.AttributeDelta));
	}
	else if (Choice.ChoiceType == EFinalGrowthChoiceType::CardEvolution)
	{
		Data.Meta = FText::Format(
			NSLOCTEXT("FinalFlowUI", "GrowthChoiceEvolutionMeta", "进化路径：{0} -> {1}"),
			Choice.FromCardId.IsValid() ? FText::FromName(Choice.FromCardId.Value) : NSLOCTEXT("FinalFlowUI", "GrowthChoiceEvolutionMetaFromUnknown", "旧卡"),
			Choice.ToCardId.IsValid() ? FText::FromName(Choice.ToCardId.Value) : NSLOCTEXT("FinalFlowUI", "GrowthChoiceEvolutionMetaToUnknown", "新卡"));
	}

	Data.State = bSelected
		? NSLOCTEXT("FinalFlowUI", "GrowthChoiceStateSelected", "当前选择")
		: NSLOCTEXT("FinalFlowUI", "GrowthChoiceStateSelectable", "点击以选择");
	return Data;
}
}

void UFinalRunGrowthChoiceOverlayScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalRunGrowthChoiceOverlayScreen::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	Super::ConfigureFromRunSnapshot(InSnapshot);
	ClampSelectionIndex();
	RebuildVisual();

	if (!InSnapshot.PendingGrowthChoice.bHasPendingChoice)
	{
		if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
		{
			if (UISubsystem->GetActiveOverlayScreen() == this)
			{
				UISubsystem->CloseOverlayScreen(this);
			}
		}
	}
}

bool UFinalRunGrowthChoiceOverlayScreen::SelectChoiceByIndex(const int32 ChoiceIndex)
{
	if (!GetCachedSnapshot().PendingGrowthChoice.Choices.IsValidIndex(ChoiceIndex))
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "GrowthChoiceSelectIndexInvalid", "当前没有对应的成长候选。"));
		RebuildVisual();
		return false;
	}

	SelectedChoiceIndex = ChoiceIndex;
	RebuildVisual();
	return true;
}

bool UFinalRunGrowthChoiceOverlayScreen::ConfirmCurrentChoice()
{
	const FFinalRunGrowthChoiceInstance* SelectedChoice = GetSelectedChoice();
	if (SelectedChoice == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "GrowthChoiceMissingSelection", "当前没有可确认的成长选项。"));
		RebuildVisual();
		return false;
	}

	UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem();
	if (RunFlowSubsystem == nullptr)
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "GrowthChoiceMissingSubsystem", "当前无法访问 RunFlowSubsystem。"));
		RebuildVisual();
		return false;
	}

	const bool bAccepted = RunFlowSubsystem->SelectGrowthChoice(SelectedChoice->ChoiceInstanceId);
	ConfigureFromRunSnapshot(RunFlowSubsystem->GetCurrentRunSnapshot());
	SetLastActionFeedback(!RunFlowSubsystem->GetLastFlowMessage().IsEmpty()
		? RunFlowSubsystem->GetLastFlowMessage()
		: (bAccepted
			? NSLOCTEXT("FinalFlowUI", "GrowthChoiceApplySucceeded", "已应用当前成长。")
			: NSLOCTEXT("FinalFlowUI", "GrowthChoiceApplyFailed", "提交成长选择失败。")));
	RebuildVisual();
	return bAccepted;
}

int32 UFinalRunGrowthChoiceOverlayScreen::GetSelectedChoiceIndex() const
{
	return SelectedChoiceIndex;
}

FName UFinalRunGrowthChoiceOverlayScreen::GetSelectedChoiceInstanceId() const
{
	if (const FFinalRunGrowthChoiceInstance* SelectedChoice = GetSelectedChoice())
	{
		return SelectedChoice->ChoiceInstanceId;
	}

	return NAME_None;
}

void UFinalRunGrowthChoiceOverlayScreen::HandlePrimaryActionClicked()
{
	ConfirmCurrentChoice();
}

void UFinalRunGrowthChoiceOverlayScreen::HandleCloseClicked()
{
	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

void UFinalRunGrowthChoiceOverlayScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	const bool bNeedsFallbackTree = WidgetTree->RootWidget == nullptr;
	EnsureBaseWidgetTree(
		FLinearColor(0.09f, 0.08f, 0.12f, 0.94f),
		TEXT("RunGrowthChoiceOverlayRoot"),
		TEXT("RunGrowthChoiceOverlayContent"));

	if (!bNeedsFallbackTree || ContentBox == nullptr)
	{
		return;
	}

	CharacterSummaryText = CreateStageLabel(TEXT("RunGrowthCharacterSummaryText"), 13);
	ContentBox->AddChildToVerticalBox(CharacterSummaryText);

	SelectionSummaryText = CreateStageLabel(TEXT("RunGrowthSelectionSummaryText"), 12);
	ContentBox->AddChildToVerticalBox(SelectionSummaryText);

	GrowthChoiceListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RunGrowthChoiceListBox"));
	ContentBox->AddChildToVerticalBox(GrowthChoiceListBox);

	PrimaryActionButton = CreateStageButton(
		TEXT("RunGrowthPrimaryActionButton"),
		TEXT("RunGrowthPrimaryActionButtonText"),
		NSLOCTEXT("FinalFlowUI", "GrowthChoicePrimaryActionFallback", "确认当前成长"),
		PrimaryActionButtonText);
	if (PrimaryActionButton != nullptr)
	{
		PrimaryActionButton->OnClicked.AddUniqueDynamic(this, &UFinalRunGrowthChoiceOverlayScreen::HandlePrimaryActionClicked);
		ContentBox->AddChildToVerticalBox(PrimaryActionButton);
	}

	CloseButton = CreateStageButton(
		TEXT("RunGrowthCloseButton"),
		TEXT("RunGrowthCloseButtonText"),
		NSLOCTEXT("FinalFlowUI", "GrowthChoiceCloseFallback", "关闭"),
		CloseButtonText);
	if (CloseButton != nullptr)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UFinalRunGrowthChoiceOverlayScreen::HandleCloseClicked);
		ContentBox->AddChildToVerticalBox(CloseButton);
	}
}

void UFinalRunGrowthChoiceOverlayScreen::RebuildVisual()
{
	EnsureWidgetTree();

	const FFinalRunSnapshot& Snapshot = GetCachedSnapshot();
	const FFinalRunPendingGrowthChoiceViewData& PendingGrowthChoice = Snapshot.PendingGrowthChoice;
	const FFinalRunCharacterViewData* TargetCharacter = GetTargetCharacter();
	const bool bHasPendingChoice = PendingGrowthChoice.bHasPendingChoice && PendingGrowthChoice.Choices.Num() > 0;

	if (TitleText)
	{
		TitleText->SetText(NSLOCTEXT("FinalFlowUI", "RunGrowthOverlayTitle", "角色成长"));
	}

	if (SummaryText)
	{
		SummaryText->SetText(bHasPendingChoice
			? FText::Format(
				NSLOCTEXT("FinalFlowUI", "RunGrowthOverlaySummary", "{0}\n选择 1 项成长。确认后会清理当前待处理成长，并返回 Run 主流程。"),
				TargetCharacter != nullptr && !TargetCharacter->DisplayName.IsEmpty()
					? TargetCharacter->DisplayName
					: FormatOptionalName(PendingGrowthChoice.CharacterId.Value, NSLOCTEXT("FinalFlowUI", "RunGrowthUnknownCharacter", "未命名角色")))
			: NSLOCTEXT("FinalFlowUI", "RunGrowthOverlayNoPendingSummary", "当前没有待处理的成长选择。"));
	}

	if (GapText)
	{
		GapText->SetText(FText::GetEmpty());
	}

	if (CharacterSummaryText)
	{
		CharacterSummaryText->SetText(BuildCharacterSummaryText());
		CharacterSummaryText->SetVisibility(bHasPendingChoice ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (SelectionSummaryText)
	{
		SelectionSummaryText->SetText(BuildSelectionSummaryText());
		SelectionSummaryText->SetVisibility(bHasPendingChoice ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	RebuildChoiceList();

	if (PrimaryActionButton)
	{
		PrimaryActionButton->SetVisibility(bHasPendingChoice ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		PrimaryActionButton->SetIsEnabled(GetSelectedChoice() != nullptr);
	}
	if (PrimaryActionButtonText)
	{
		PrimaryActionButtonText->SetText(BuildPrimaryActionText());
	}

	if (CloseButton)
	{
		CloseButton->SetVisibility(ESlateVisibility::Visible);
		CloseButton->SetIsEnabled(true);
	}
	if (CloseButtonText)
	{
		CloseButtonText->SetText(NSLOCTEXT("FinalFlowUI", "RunGrowthClose", "关闭"));
	}

	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(NSLOCTEXT("FinalFlowUI", "RunGrowthDefaultFeedback", "等待选择一项成长。")));
	}
}

void UFinalRunGrowthChoiceOverlayScreen::RebuildChoiceList()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	ClearChoiceList();
	if (GrowthChoiceListBox == nullptr)
	{
		return;
	}

	const FFinalRunPendingGrowthChoiceViewData& PendingGrowthChoice = GetCachedSnapshot().PendingGrowthChoice;
	const TSubclassOf<UFinalRunFlowOptionButton> OptionButtonClass = UFinalUIWidgetClassSettings::GetRunFlowOptionButtonClass();
	UClass* ResolvedOptionButtonClass = OptionButtonClass.Get() ? OptionButtonClass.Get() : UFinalRunFlowOptionButton::StaticClass();

	for (int32 ChoiceIndex = 0; ChoiceIndex < PendingGrowthChoice.Choices.Num(); ++ChoiceIndex)
	{
		const FFinalRunGrowthChoiceInstance& Choice = PendingGrowthChoice.Choices[ChoiceIndex];
		UFinalRunFlowOptionButton* OptionWidget = WidgetTree->ConstructWidget<UFinalRunFlowOptionButton>(
			ResolvedOptionButtonClass,
			*FString::Printf(TEXT("RunGrowthChoiceOption_%d"), ChoiceIndex));
		if (OptionWidget == nullptr)
		{
			continue;
		}

		OptionWidget->ConfigureOption(BuildGrowthChoiceOptionData(Choice, ChoiceIndex, ChoiceIndex == SelectedChoiceIndex));
		OptionWidget->OnOptionClicked.AddUObject(this, &UFinalRunGrowthChoiceOverlayScreen::HandleChoiceOptionClicked);

		if (UVerticalBoxSlot* OptionSlot = GrowthChoiceListBox->AddChildToVerticalBox(OptionWidget))
		{
			OptionSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}
	}

	GrowthChoiceListBox->SetVisibility(PendingGrowthChoice.Choices.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFinalRunGrowthChoiceOverlayScreen::ClearChoiceList()
{
	if (GrowthChoiceListBox)
	{
		GrowthChoiceListBox->ClearChildren();
		GrowthChoiceListBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFinalRunGrowthChoiceOverlayScreen::ClampSelectionIndex()
{
	const TArray<FFinalRunGrowthChoiceInstance>& Choices = GetCachedSnapshot().PendingGrowthChoice.Choices;
	if (Choices.Num() <= 0)
	{
		SelectedChoiceIndex = INDEX_NONE;
		return;
	}

	if (!Choices.IsValidIndex(SelectedChoiceIndex))
	{
		SelectedChoiceIndex = 0;
	}
}

void UFinalRunGrowthChoiceOverlayScreen::HandleChoiceOptionClicked(UFinalRunFlowOptionButton* OptionButton)
{
	if (OptionButton == nullptr)
	{
		return;
	}

	SelectChoiceByIndex(OptionButton->GetPayloadIndex());
}

const FFinalRunGrowthChoiceInstance* UFinalRunGrowthChoiceOverlayScreen::GetSelectedChoice() const
{
	const TArray<FFinalRunGrowthChoiceInstance>& Choices = GetCachedSnapshot().PendingGrowthChoice.Choices;
	return Choices.IsValidIndex(SelectedChoiceIndex) ? &Choices[SelectedChoiceIndex] : nullptr;
}

const FFinalRunCharacterViewData* UFinalRunGrowthChoiceOverlayScreen::GetTargetCharacter() const
{
	const FFinalCharacterId TargetCharacterId = GetCachedSnapshot().PendingGrowthChoice.CharacterId;
	return GetCachedSnapshot().Characters.FindByPredicate([&TargetCharacterId](const FFinalRunCharacterViewData& Character)
	{
		return Character.CharacterId == TargetCharacterId;
	});
}

FText UFinalRunGrowthChoiceOverlayScreen::BuildCharacterSummaryText() const
{
	const FFinalRunCharacterViewData* TargetCharacter = GetTargetCharacter();
	if (TargetCharacter == nullptr)
	{
		return NSLOCTEXT("FinalFlowUI", "RunGrowthMissingCharacterSummary", "当前无法读取角色成长摘要。");
	}

	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunGrowthCharacterSummary", "等级 {0} | 突破 {1}/{2}\n根骨 {3} | 悟性 {4} | 杀意 {5}"),
		FText::AsNumber(TargetCharacter->Level),
		FText::AsNumber(TargetCharacter->BreakthroughValue),
		FText::AsNumber(TargetCharacter->BreakthroughRequiredValue),
		FText::AsNumber(TargetCharacter->RootBone),
		FText::AsNumber(TargetCharacter->Insight),
		FText::AsNumber(TargetCharacter->KillingIntent));
}

FText UFinalRunGrowthChoiceOverlayScreen::BuildSelectionSummaryText() const
{
	const FFinalRunGrowthChoiceInstance* SelectedChoice = GetSelectedChoice();
	if (SelectedChoice == nullptr)
	{
		return NSLOCTEXT("FinalFlowUI", "RunGrowthNoSelectionSummary", "当前没有可用成长选项。");
	}

	if (SelectedChoice->ChoiceType == EFinalGrowthChoiceType::CardEvolution)
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunGrowthSelectionEvolution", "{0}\n{1}\n目标卡牌：{2} -> {3}"),
			BuildGrowthChoiceTitle(*SelectedChoice),
			FormatOptionalText(SelectedChoice->Description, NSLOCTEXT("FinalFlowUI", "RunGrowthSelectionNoDescription", "无额外说明。")),
			SelectedChoice->FromCardId.IsValid() ? FText::FromName(SelectedChoice->FromCardId.Value) : NSLOCTEXT("FinalFlowUI", "RunGrowthSelectionEvolutionFromUnknown", "旧卡"),
			SelectedChoice->ToCardId.IsValid() ? FText::FromName(SelectedChoice->ToCardId.Value) : NSLOCTEXT("FinalFlowUI", "RunGrowthSelectionEvolutionToUnknown", "新卡"));
	}

	return FText::Format(
		NSLOCTEXT("FinalFlowUI", "RunGrowthSelectionAttribute", "{0}\n{1}"),
		BuildGrowthChoiceTitle(*SelectedChoice),
		FormatOptionalText(SelectedChoice->Description, NSLOCTEXT("FinalFlowUI", "RunGrowthSelectionNoDescriptionAttribute", "无额外说明。")));
}

FText UFinalRunGrowthChoiceOverlayScreen::BuildPrimaryActionText() const
{
	if (const FFinalRunGrowthChoiceInstance* SelectedChoice = GetSelectedChoice())
	{
		return FText::Format(
			NSLOCTEXT("FinalFlowUI", "RunGrowthPrimaryAction", "确认：{0}"),
			BuildGrowthChoiceTitle(*SelectedChoice));
	}

	return NSLOCTEXT("FinalFlowUI", "RunGrowthPrimaryActionUnavailable", "当前无可确认成长");
}
