#include "UI/Screens/FinalOverlayScreenBase.h"

UFinalOverlayScreenBase::UFinalOverlayScreenBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenLayer = EFinalUIScreenLayer::Overlay;
	DesiredInputConfig.InputMode = EFinalUIInputMode::GameAndUI;
	DesiredInputConfig.bShowMouseCursor = true;
	DesiredInputConfig.bHideCursorDuringCapture = false;
}
