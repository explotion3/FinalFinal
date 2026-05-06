#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"

void UFinalRunStageOverlayScreenBase::ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot)
{
	CachedSnapshot = InSnapshot;
	LastActionFeedback = FText::GetEmpty();
}

void UFinalRunStageOverlayScreenBase::RequestCloseOverlay()
{
	if (!CanCloseOverlay())
	{
		SetLastActionFeedback(NSLOCTEXT("FinalFlowUI", "RunStageOverlayCloseRejected", "当前流程页暂时不能关闭。"));
		RefreshFeedbackText(NSLOCTEXT("FinalFlowUI", "RunStageOverlayCloseRejectedFallback", "当前流程页暂时不能关闭。"));
		return;
	}

	if (UFinalUISubsystem* UISubsystem = ResolveUISubsystem())
	{
		UISubsystem->CloseOverlayScreen(this);
	}
}

bool UFinalRunStageOverlayScreenBase::CanCloseOverlay() const
{
	return true;
}

UWidget* UFinalRunStageOverlayScreenBase::GetDefaultFocusWidget() const
{
	return nullptr;
}

void UFinalRunStageOverlayScreenBase::EnsureBaseWidgetTree(const FLinearColor& RootTint, const TCHAR* RootName, const TCHAR* ContentName)
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), RootName);
	RootBorder->SetBrushColor(RootTint);
	RootBorder->SetPadding(FMargin(24.0f));
	WidgetTree->RootWidget = RootBorder;

	ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), ContentName);
	RootBorder->SetContent(ContentBox);

	TitleText = CreateStageLabel(TEXT("RunStageOverlayTitle"), 22);
	ContentBox->AddChildToVerticalBox(TitleText);

	SummaryText = CreateStageLabel(TEXT("RunStageOverlaySummary"), 14);
	ContentBox->AddChildToVerticalBox(SummaryText);

	GapText = CreateStageLabel(TEXT("RunStageOverlayGap"), 12);
	ContentBox->AddChildToVerticalBox(GapText);

	FeedbackText = CreateStageLabel(TEXT("RunStageOverlayFeedback"), 12);
	ContentBox->AddChildToVerticalBox(FeedbackText);
}

UTextBlock* UFinalRunStageOverlayScreenBase::CreateStageLabel(const TCHAR* Name, const int32 FontSize) const
{
	if (WidgetTree == nullptr)
	{
		return nullptr;
	}

	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}

UButton* UFinalRunStageOverlayScreenBase::CreateStageButton(const TCHAR* ButtonName, const TCHAR* LabelName, const FText& LabelText, TObjectPtr<UTextBlock>& OutLabelText)
{
	if (WidgetTree == nullptr)
	{
		return nullptr;
	}

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), ButtonName);
	OutLabelText = CreateStageLabel(LabelName, 13);
	if (OutLabelText != nullptr)
	{
		OutLabelText->SetText(LabelText);
		Button->AddChild(OutLabelText);
	}

	return Button;
}

FText UFinalRunStageOverlayScreenBase::BuildFeedbackText(const FText& DefaultText) const
{
	if (!LastActionFeedback.IsEmpty())
	{
		return LastActionFeedback;
	}

	if (!CachedSnapshot.Progression.CurrentNodeStateMessage.IsEmpty())
	{
		return CachedSnapshot.Progression.CurrentNodeStateMessage;
	}

	if (const UFinalRunFlowSubsystem* RunFlowSubsystem = ResolveRunFlowSubsystem())
	{
		const FText FlowMessage = RunFlowSubsystem->GetLastFlowMessage();
		if (!FlowMessage.IsEmpty())
		{
			return FlowMessage;
		}
	}

	return DefaultText;
}

void UFinalRunStageOverlayScreenBase::RefreshFeedbackText(const FText& DefaultText)
{
	if (FeedbackText)
	{
		FeedbackText->SetText(BuildFeedbackText(DefaultText));
	}
}

void UFinalRunStageOverlayScreenBase::SetLastActionFeedback(const FText& InFeedbackText)
{
	LastActionFeedback = InFeedbackText;
}

UFinalRunFlowSubsystem* UFinalRunStageOverlayScreenBase::ResolveRunFlowSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UFinalRunFlowSubsystem>();
	}

	return nullptr;
}

UFinalUISubsystem* UFinalRunStageOverlayScreenBase::ResolveUISubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UFinalUISubsystem>();
	}

	return nullptr;
}
