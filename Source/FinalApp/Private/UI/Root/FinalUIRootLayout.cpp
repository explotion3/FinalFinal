#include "UI/Root/FinalUIRootLayout.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "UI/Screens/FinalScreenBase.h"

void UFinalUIRootLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureLayoutTree();
}

void UFinalUIRootLayout::SetPersistentHUD(UFinalScreenBase* InScreen)
{
	ClearLayer(EFinalUIScreenLayer::HUD);
	AddScreenToLayer(InScreen, EFinalUIScreenLayer::HUD);
}

void UFinalUIRootLayout::ClearLayer(EFinalUIScreenLayer Layer)
{
	if (UOverlay* Overlay = ResolveLayer(Layer))
	{
		Overlay->ClearChildren();
	}
}

void UFinalUIRootLayout::AddScreenToLayer(UFinalScreenBase* Screen, EFinalUIScreenLayer Layer)
{
	if (Screen == nullptr)
	{
		return;
	}

	if (UOverlay* Overlay = ResolveLayer(Layer))
	{
		Overlay->AddChild(Screen);
	}
}

UOverlay* UFinalUIRootLayout::GetLayerWidget(EFinalUIScreenLayer Layer) const
{
	return ResolveLayer(Layer);
}

void UFinalUIRootLayout::EnsureLayoutTree()
{
	if (WidgetTree == nullptr || RootCanvas != nullptr)
	{
		return;
	}

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	auto AddFullScreenLayer = [this](const TCHAR* LayerName) -> UOverlay*
	{
		UOverlay* LayerWidget = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), LayerName);
		UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(LayerWidget);
		Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		Slot->SetOffsets(FMargin(0.0f));
		return LayerWidget;
	};

	HUDLayerWidget = AddFullScreenLayer(TEXT("HUDLayer"));
	OverlayLayerWidget = AddFullScreenLayer(TEXT("OverlayLayer"));
	ModalLayerWidget = AddFullScreenLayer(TEXT("ModalLayer"));
	TooltipLayerWidget = AddFullScreenLayer(TEXT("TooltipLayer"));
	ToastLayerWidget = AddFullScreenLayer(TEXT("ToastLayer"));
}

UOverlay* UFinalUIRootLayout::ResolveLayer(EFinalUIScreenLayer Layer) const
{
	switch (Layer)
	{
	case EFinalUIScreenLayer::HUD:
		return HUDLayerWidget;

	case EFinalUIScreenLayer::Overlay:
		return OverlayLayerWidget;

	case EFinalUIScreenLayer::Modal:
		return ModalLayerWidget;

	case EFinalUIScreenLayer::Tooltip:
		return TooltipLayerWidget;

	case EFinalUIScreenLayer::Toast:
		return ToastLayerWidget;

	default:
		return nullptr;
	}
}
