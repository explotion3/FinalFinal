#include "UI/Screens/FinalScreenBase.h"

EFinalUIScreenLayer UFinalScreenBase::GetScreenLayer() const
{
	return ScreenLayer;
}

FFinalUIInputConfig UFinalScreenBase::GetDesiredInputConfig() const
{
	return DesiredInputConfig;
}

void UFinalScreenBase::HandleScreenOpened()
{
}

void UFinalScreenBase::HandleScreenClosed()
{
}
