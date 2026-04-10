#include "UI/Screens/FinalModalScreenBase.h"

UFinalModalScreenBase::UFinalModalScreenBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenLayer = EFinalUIScreenLayer::Modal;
	DesiredInputConfig.InputMode = EFinalUIInputMode::UIOnly;
	DesiredInputConfig.bShowMouseCursor = true;
	DesiredInputConfig.bHideCursorDuringCapture = false;
}
