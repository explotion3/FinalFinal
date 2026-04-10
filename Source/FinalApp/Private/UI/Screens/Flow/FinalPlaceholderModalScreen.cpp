#include "UI/Screens/Flow/FinalPlaceholderModalScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/UI/FinalUISubsystem.h"

namespace
{
UTextBlock* CreateLabel(UWidgetTree* WidgetTree, const TCHAR* Name, const int32 FontSize)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetAutoWrapText(true);
	Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), FontSize));
	return Text;
}
}

void UFinalPlaceholderModalScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	RebuildVisual();
}

void UFinalPlaceholderModalScreen::ConfigureModal(const FText& InTitle, const FText& InBody)
{
	CachedTitle = InTitle;
	CachedBody = InBody;
	RebuildVisual();
}

void UFinalPlaceholderModalScreen::HandleCloseClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->CloseModalScreen(this);
		}
	}
}

void UFinalPlaceholderModalScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PlaceholderModalRoot"));
	RootBorder->SetBrushColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.94f));
	RootBorder->SetPadding(FMargin(28.0f));
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PlaceholderModalContent"));
	RootBorder->SetContent(ContentBox);

	TitleText = CreateLabel(WidgetTree, TEXT("PlaceholderModalTitle"), 20);
	ContentBox->AddChildToVerticalBox(TitleText);

	BodyText = CreateLabel(WidgetTree, TEXT("PlaceholderModalBody"), 13);
	ContentBox->AddChildToVerticalBox(BodyText);

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PlaceholderModalCloseButton"));
	CloseButtonText = CreateLabel(WidgetTree, TEXT("PlaceholderModalCloseButtonText"), 13);
	CloseButtonText->SetText(NSLOCTEXT("FinalFlowUI", "PlaceholderModalCloseButton", "关闭模态"));
	CloseButton->AddChild(CloseButtonText);
	CloseButton->OnClicked.AddDynamic(this, &UFinalPlaceholderModalScreen::HandleCloseClicked);
	ContentBox->AddChildToVerticalBox(CloseButton);
}

void UFinalPlaceholderModalScreen::RebuildVisual()
{
	if (TitleText)
	{
		TitleText->SetText(!CachedTitle.IsEmpty()
			? CachedTitle
			: NSLOCTEXT("FinalFlowUI", "PlaceholderModalDefaultTitle", "流程模态占位"));
	}

	if (BodyText)
	{
		BodyText->SetText(!CachedBody.IsEmpty()
			? CachedBody
			: NSLOCTEXT("FinalFlowUI", "PlaceholderModalDefaultBody", "当前页面用于承接后续确认类交互。"));
	}
}
