#include "UI/Root/FinalUIRootLayout.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
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
		if (UOverlaySlot* ScreenSlot = Overlay->AddChildToOverlay(Screen))
		{
			ScreenSlot->SetHorizontalAlignment(HAlign_Fill);
			ScreenSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

UOverlay* UFinalUIRootLayout::GetLayerWidget(EFinalUIScreenLayer Layer) const
{
	return ResolveLayer(Layer);
}

void UFinalUIRootLayout::EnsureLayoutTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (HasBoundLayerWidgets())
	{
		if (HUDLayer)
		{
			HUDLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (OverlayLayer)
		{
			OverlayLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (ModalLayer)
		{
			ModalLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (TooltipLayer)
		{
			TooltipLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (ToastLayer)
		{
			ToastLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		return;
	}

	if (RootCanvas != nullptr)
	{
		return;
	}

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	auto AddFullScreenLayer = [this](const TCHAR* LayerName) -> UOverlay*
	{
		UOverlay* LayerWidget = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), LayerName);
		LayerWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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

bool UFinalUIRootLayout::HasBoundLayerWidgets() const
{
	return HUDLayer != nullptr
		|| OverlayLayer != nullptr
		|| ModalLayer != nullptr
		|| TooltipLayer != nullptr
		|| ToastLayer != nullptr;
}

UOverlay* UFinalUIRootLayout::ResolveLayer(EFinalUIScreenLayer Layer) const
{
	switch (Layer)
	{
	case EFinalUIScreenLayer::HUD:
		return HUDLayer != nullptr ? HUDLayer.Get() : HUDLayerWidget.Get();

	case EFinalUIScreenLayer::Overlay:
		return OverlayLayer != nullptr ? OverlayLayer.Get() : OverlayLayerWidget.Get();

	case EFinalUIScreenLayer::Modal:
		return ModalLayer != nullptr ? ModalLayer.Get() : ModalLayerWidget.Get();

	case EFinalUIScreenLayer::Tooltip:
		return TooltipLayer != nullptr ? TooltipLayer.Get() : TooltipLayerWidget.Get();

	case EFinalUIScreenLayer::Toast:
		return ToastLayer != nullptr ? ToastLayer.Get() : ToastLayerWidget.Get();

	default:
		return nullptr;
	}
}
