#include "D2DHollywoodEffect.h"
#include <d2d1effecthelpers.h>
#include "D2DHollywoodEffectShader.h"
#include <fstream>

const UINT32 D2DMatrixPixelShader_InstructionCount = 4;

#define XML(X) TEXT(#X) 

using namespace std;

HRESULT __stdcall D2DHollywoodEffect::CreateD2DHollywoodEffectImpl(_Outptr_ IUnknown** ppEffectImpl) 
{ 
    // Since the object's refcount is initialized to 1, we don't need to AddRef here. 
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
    // The inspectable metadata of an effect is defined in XML. This can be passed in from an external source 
    // as well, however for simplicity we just inline the XML. 
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
 
    // This defines the bindings from specific properties to the callback functions 
    // on the class that ID2D1Effect::SetValue() & GetValue() will call. 
    const D2D1_PROPERTY_BINDING bindings[] = 
    { 
        D2D1_VALUE_TYPE_BINDING(L"Offset", &SetOffset, &GetOffset), 
        D2D1_VALUE_TYPE_BINDING(L"Bounds", &SetBounds, &GetBounds) 
    }; 
 
    // This registers the effect with the factory, which will make the effect 
    // instantiatable. 
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
	
    // This loads the shader into the Direct2D image effects system and associates it with the GUID passed in. 
    // If this method is called more than once (say by other instances of the effect) with the same GUID, 
    // the system will simply do nothing, ensuring that only one instance of a shader is stored regardless of how 
    // many time it is used. 
    if (SUCCEEDED(hr)) 
    { 
        // The graph consists of a single transform. In fact, this class is the transform, 
        // reducing the complexity of implementing an effect when all we need to 
        // do is use a single pixel shader. 
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
 
// SetGraph is only called when the number of inputs changes. This never happens as we publish this effect 
// as a single input effect. 
IFACEMETHODIMP D2DHollywoodEffect::SetGraph(_In_ ID2D1TransformGraph* pGraph) 
{ 
    return E_NOTIMPL; 
} 
 
// Called to assign a new render info class, which is used to inform D2D on 
// how to set the state of the GPU. 
IFACEMETHODIMP D2DHollywoodEffect::SetDrawInfo(_In_ ID2D1DrawInfo* pDrawInfo) 
{ 
    HRESULT hr = S_OK; 
 
    drawInfo = pDrawInfo; 
 
    hr = drawInfo->SetPixelShader(GUID_D2DHollywoodPixelShader); 
 
    if (SUCCEEDED(hr)) 
    { 
        // Providing this hint allows D2D to optimize performance when processing large images. 
		//drawInfo->SetInstructionCountHint(D2DMatrixPixelShader_InstructionCount); 
    } 
 
    return hr; 
} 
 
// Calculates the mapping between the output and input rects. In this case, 
// we want to request an expanded region to account for pixels that the ripple 
// may need outside of the bounds of the destination. 
IFACEMETHODIMP D2DHollywoodEffect::MapOutputRectToInputRects( 
    _In_ const D2D1_RECT_L* pOutputRect, 
    _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects, 
    UINT32 inputRectCount 
    ) const 
{ 
    // This effect has exactly one input, so if there is more than one input rect, 
    // something is wrong. 
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
    // This effect has exactly one input, so if there is more than one input rect, 
    // something is wrong. 
    if (inputRectCount != 1) 
    { 
        return E_INVALIDARG; 
    } 
 
    *pOutputRect = pInputRects[0]; 
    inputRect = pInputRects[0]; 
 
    // Indicate that entire output might contain transparency. 
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
 
    // Indicate that the entire output may be invalid. 
    *pInvalidOutputRect = inputRect; 
 
    return hr; 
} 

IFACEMETHODIMP_(UINT32) D2DHollywoodEffect::GetInputCount() const 
{ 
    return 1; 
} 
 
// D2D ensures that that effects are only referenced from one thread at a time. 
// To improve performance, we simply increment/decrement our reference count 
// rather than use atomic InterlockedIncrement()/InterlockedDecrement() functions. 
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
 
// This enables the stack of parent interfaces to be queried. In the instance 
// of the Ripple interface, this method simply enables the developer 
// to cast a Ripple instance to an ID2D1EffectImpl or IUnknown instance. 
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