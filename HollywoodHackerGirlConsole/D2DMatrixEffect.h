#pragma once
#include "d2dSetup.h"
#include <D2d1effectauthor.h>
#include <INITGUID.H>

// {19176F4E-C481-437C-A0F7-A21533DF6589}
DEFINE_GUID(GUID_D2DMatrixPixelShader, 0x19176f4e, 0xc481, 0x437c, 0xa0, 0xf7, 0xa2, 0x15, 0x33, 0xdf, 0x65, 0x89);
// {C4E54546-8E4E-4FAD-99AC-2F502360FAC8}
DEFINE_GUID(CLSID_D2DMatrixEffect, 0xc4e54546, 0x8e4e, 0x4fad, 0x99, 0xac, 0x2f, 0x50, 0x23, 0x60, 0xfa, 0xc8);

class D2DMatrixEffect  : public ID2D1EffectImpl, public ID2D1DrawTransform 
{
private:
	Microsoft::WRL::ComPtr<ID2D1DrawInfo>      drawInfo; 
    Microsoft::WRL::ComPtr<ID2D1EffectContext> effectContext; 
    LONG                                       refCount; 
    D2D1_RECT_L                                inputRect; 

	struct 
    { 
        float offset; 
		D2D_POINT_2F bounds;
    } constants; 
 
	D2DMatrixEffect();
	HRESULT UpdateConstants(); 

public:
	enum SET_VALUE_PROPERTIES {OFFSET_VALUE = 0, BOUNDS = 1};

	static HRESULT Register(_In_ ID2D1Factory1* pFactory); 
 
    static HRESULT __stdcall CreateD2DMatrixEffectImpl(_Outptr_ IUnknown** ppEffectImpl); 

	HRESULT SetOffset(float value); 
    float GetOffset() const; 

	HRESULT SetBounds(D2D_POINT_2F value);
	D2D_POINT_2F GetBounds() const;

	// Declare ID2D1EffectImpl implementation methods. 
    IFACEMETHODIMP Initialize( 
        _In_ ID2D1EffectContext* pContextInternal, 
        _In_ ID2D1TransformGraph* pTransformGraph 
        ); 
 
    IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType); 
 
    IFACEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* pGraph); 
 
    // Declare ID2D1DrawTransform implementation methods. 
    IFACEMETHODIMP SetDrawInfo(_In_ ID2D1DrawInfo* pRenderInfo); 
 
    // Declare ID2D1Transform implementation methods. 
    IFACEMETHODIMP MapOutputRectToInputRects( 
        _In_ const D2D1_RECT_L* pOutputRect, 
        _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects, 
        UINT32 inputRectCount 
        ) const; 
 
    IFACEMETHODIMP MapInputRectsToOutputRect( 
        _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputRects, 
        _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputOpaqueSubRects, 
        UINT32 inputRectCount, 
        _Out_ D2D1_RECT_L* pOutputRect, 
        _Out_ D2D1_RECT_L* pOutputOpaqueSubRect 
        ); 
 
    IFACEMETHODIMP MapInvalidRect( 
        UINT32 inputIndex, 
        D2D1_RECT_L invalidInputRect, 
        _Out_ D2D1_RECT_L* pInvalidOutputRect 
        ) const; 
 
    // Declare ID2D1TransformNode implementation methods. 
    IFACEMETHODIMP_(UINT32) GetInputCount() const; 
 
    // Declare IUnknown implementation methods. 
    IFACEMETHODIMP_(ULONG) AddRef(); 
    IFACEMETHODIMP_(ULONG) Release(); 
    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void** ppOutput); 
};