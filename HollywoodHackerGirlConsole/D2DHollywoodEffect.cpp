#include "D2DHollywoodEffect.h"
#include <d2d1effecthelpers.h>
#include "D2DHollywoodEffectShader.h"
#include <fstream>

const UINT32 D2DMatrixPixelShader_InstructionCount = 4;

#define XML(X) TEXT(#X) 

using namespace std;

HRESULT __stdcall D2DHollywoodEffect::CreateD2DHollywoodEffectImpl(_Outptr_ IUnknown** ppEffectImpl) 
{ 
    *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new (std::nothrow) D2DHollywoodEffect()); 
 
    if (*ppEffectImpl == nullptr) 
    { 
        return E_OUTOFMEMORY; 
    } 
    else 
    { 
        return S_OK; 
    } 
} 

HRESULT D2DHollywoodEffect::Register(_In_ ID2D1Factory1* pFactory) 
{ 
    PCWSTR pszXml = 
        XML( 
            <?xml version='1.0'?> 
            <Effect> 
                <!-- System Properties --> 
                <Property name='DisplayName' type='string' value='Matrix'/> 
                <Property name='Author' type='string' value='Microsoft Corporation'/> 
                <Property name='Category' type='string' value='Stylize'/> 
                <Property name='Description' type='string' value='Adds a ripple effect that can be animated'/> 
                <Inputs> 
                    <Input name='Source'/> 
                </Inputs> 
                <!-- Custom Properties go here --> 
                <Property name='Offset' type='float'> 
                    <Property name='DisplayName' type='string' value='Offset'/> 
                    <Property name='Min' type='float' value='0.0' /> 
                    <Property name='Max' type='float' value='1.0' /> 
                    <Property name='Default' type='float' value='0.0' /> 
                </Property> 
				<Property name='Bounds' type='vector2'> 
                    <Property name='DisplayName' type='string' value='Image Bounds'/>  
                    <Property name='Default' type='vector2' value='(0.0, 0.0)' /> 
                </Property> 
            </Effect> 
            ); 
 
    const D2D1_PROPERTY_BINDING bindings[] = 
    { 
        D2D1_VALUE_TYPE_BINDING(L"Offset", &SetOffset, &GetOffset), 
        D2D1_VALUE_TYPE_BINDING(L"Bounds", &SetBounds, &GetBounds) 
    }; 
 
    return pFactory->RegisterEffectFromString( 
        CLSID_D2DHollywoodEffect, 
        pszXml, 
        bindings, 
        ARRAYSIZE(bindings), 
        CreateD2DHollywoodEffectImpl 
        ); 
} 
 
D2DHollywoodEffect::D2DHollywoodEffect() : refCount(1) 
{

}

float D2DHollywoodEffect::GetOffset() const
{
	return constants.offset;
}

HRESULT D2DHollywoodEffect::SetOffset(float value)
{
	constants.offset = value;
	return S_OK;
}

D2D_POINT_2F D2DHollywoodEffect::GetBounds() const
{
	return constants.bounds;
}

HRESULT D2DHollywoodEffect::SetBounds(D2D_POINT_2F value)
{
	constants.bounds = value;
	return S_OK;
}

IFACEMETHODIMP D2DHollywoodEffect::Initialize( 
    _In_ ID2D1EffectContext* pEffectContext, 
    _In_ ID2D1TransformGraph* pTransformGraph 
    ) 
{
	HRESULT hr = S_OK;
	effectContext = pEffectContext; 
 
	ifstream infile;
	infile.open("D2DHollywoodEffect.cso", ios::binary |  ios::ate);
	int length = infile.tellg();
	infile.seekg (0, ios::beg);
	
	auto data = new char[length];
	infile.read(data, length);
	infile.close();
 
	hr = pEffectContext->LoadPixelShader(GUID_D2DHollywoodPixelShader, reinterpret_cast<BYTE*>(data), length);
 
	delete[] data;
	
    if (SUCCEEDED(hr)) 
    { 
        hr = pTransformGraph->SetSingleTransformNode(this); 
    }
 
	return hr; 
}

HRESULT D2DHollywoodEffect::UpdateConstants() 
{  
    return drawInfo->SetPixelShaderConstantBuffer(reinterpret_cast<BYTE*>(&constants), sizeof(constants)); 
} 
 
IFACEMETHODIMP D2DHollywoodEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType) 
{ 
    return UpdateConstants(); 
} 
 
IFACEMETHODIMP D2DHollywoodEffect::SetGraph(_In_ ID2D1TransformGraph* pGraph) 
{ 
    return E_NOTIMPL; 
} 

IFACEMETHODIMP D2DHollywoodEffect::SetDrawInfo(_In_ ID2D1DrawInfo* pDrawInfo) 
{ 
    HRESULT hr = S_OK; 
 
    drawInfo = pDrawInfo; 
 
    hr = drawInfo->SetPixelShader(GUID_D2DHollywoodPixelShader); 

 
    return hr; 
} 
 
IFACEMETHODIMP D2DHollywoodEffect::MapOutputRectToInputRects( 
    _In_ const D2D1_RECT_L* pOutputRect, 
    _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects, 
    UINT32 inputRectCount 
    ) const 
{ 
    if (inputRectCount != 1) 
    { 
        return E_INVALIDARG; 
    } 
 
	*pInputRects = *pOutputRect;
    return S_OK; 
} 
 
IFACEMETHODIMP D2DHollywoodEffect::MapInputRectsToOutputRect( 
    _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputRects, 
    _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputOpaqueSubRects, 
    UINT32 inputRectCount, 
    _Out_ D2D1_RECT_L* pOutputRect, 
    _Out_ D2D1_RECT_L* pOutputOpaqueSubRect 
    ) 
{ 
    if (inputRectCount != 1) 
    { 
        return E_INVALIDARG; 
    } 
 
    *pOutputRect = pInputRects[0]; 
    inputRect = pInputRects[0]; 
 
    ZeroMemory(pOutputOpaqueSubRect, sizeof(*pOutputOpaqueSubRect)); 
 
    return S_OK; 
} 
 
IFACEMETHODIMP D2DHollywoodEffect::MapInvalidRect( 
    UINT32 inputIndex, 
    D2D1_RECT_L invalidInputRect, 
    _Out_ D2D1_RECT_L* pInvalidOutputRect 
    ) const 
{ 
    HRESULT hr = S_OK; 
 
    *pInvalidOutputRect = inputRect; 
 
    return hr; 
} 

IFACEMETHODIMP_(UINT32) D2DHollywoodEffect::GetInputCount() const 
{ 
    return 1; 
} 
 
IFACEMETHODIMP_(ULONG) D2DHollywoodEffect::AddRef() 
{ 
    refCount++; 
    return refCount; 
} 
 
IFACEMETHODIMP_(ULONG) D2DHollywoodEffect::Release() 
{ 
    refCount--; 
 
    if (refCount == 0) 
    { 
        delete this; 
        return 0; 
    } 
    else 
    { 
        return refCount; 
    } 
} 
 
IFACEMETHODIMP D2DHollywoodEffect::QueryInterface( 
    _In_ REFIID riid, 
    _Outptr_ void** ppOutput 
    ) 
{ 
    *ppOutput = nullptr; 
    HRESULT hr = S_OK; 
 
    if (riid == __uuidof(ID2D1EffectImpl)) 
    { 
        *ppOutput = reinterpret_cast<ID2D1EffectImpl*>(this); 
    } 
    else if (riid == __uuidof(ID2D1DrawTransform)) 
    { 
        *ppOutput = static_cast<ID2D1DrawTransform*>(this); 
    } 
    else if (riid == __uuidof(ID2D1Transform)) 
    { 
        *ppOutput = static_cast<ID2D1Transform*>(this); 
    } 
    else if (riid == __uuidof(ID2D1TransformNode)) 
    { 
        *ppOutput = static_cast<ID2D1TransformNode*>(this); 
    } 
    else if (riid == __uuidof(IUnknown)) 
    { 
        *ppOutput = this; 
    } 
    else 
    { 
        hr = E_NOINTERFACE; 
    } 
 
    if (*ppOutput != nullptr) 
    { 
        AddRef(); 
    } 
 
    return hr; 
} 