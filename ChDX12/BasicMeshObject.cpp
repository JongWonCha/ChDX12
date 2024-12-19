#include "pch.h"
#include "typedef.h"
#include <d3dcompiler.h>
#include <d3dx12.h>
#include "D3D_Util/D3DUtil.h"
#include "D3D12Renderer.h"
#include "BasicMeshObject.h"

ID3D12RootSignature* CBasicMeshObject::m_pRootSignature = nullptr;
ID3D12PipelineState* CBasicMeshObject::m_pPipelineState = nullptr;
DWORD CBasicMeshObject::m_dwInitRefCount = 0;

CBasicMeshObject::CBasicMeshObject()
{
}

CBasicMeshObject::~CBasicMeshObject()
{
	Cleanup();
}

BOOL CBasicMeshObject::Initialize(CD3D12Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	BOOL bResult = InitCommonResources();
	return bResult;
}

BOOL CBasicMeshObject::InitCommonResources()
{
	if (m_dwInitRefCount)
		goto lb_true;

	InitRootSignature();
	InitPipelineState();

lb_true:
	m_dwInitRefCount++;
	return m_dwInitRefCount;
}

BOOL CBasicMeshObject::InitRootSignature()
{
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();

	// 속이 비어있는 루트 시그니처를 만든다. 안쓸거기 때문에 비어있는 것으로.
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
		__debugbreak();

	if (FAILED(pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature))))
		__debugbreak();

	if (pSignature)
	{
		pSignature->Release();
		pSignature = nullptr;
	}
	if (pError)
	{
		pError->Release();
		pError = nullptr;
	}
	return TRUE;
}

BOOL CBasicMeshObject::InitPipelineState()
{
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();

	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;
#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	if (FAILED(D3DCompileFromFile(L"Shaders\\shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, nullptr)))
		__debugbreak();

	if (FAILED(D3DCompileFromFile(L"Shaders\\shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, nullptr)))
		__debugbreak();

	// 버텍스 인풋 레이아웃 정의
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	// 이 과정에서 루트 시그니처가 필요하기 때문에 빈것이라도 루트 시그니처를 만든 것이다.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_pRootSignature;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	if (FAILED(pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState))))
		__debugbreak();

	if (pVertexShader)
	{
		pVertexShader->Release();
		pVertexShader = nullptr;
	}
	if (pPixelShader)
	{
		pPixelShader->Release();
		pPixelShader = nullptr;
	}

	return TRUE;
}

BOOL CBasicMeshObject::CreateMesh()
{
	// 외부에서 버텍스 데이터와 텍스처를 입력하는 식으로 변경할 예정.
	// 지금은 일단 오브젝트 내부에서 설정

	BOOL bResult = FALSE;
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();

	// 버텍스 버퍼 생성
	// 삼각형의 지오메트리 정의
	BasicVertex Vertices[] =
	{
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT VertexBufferSize = sizeof(Vertices);

	// 버텍스 버퍼를 만드는 부분
	// 이 샘플에서는 버텍스 데이터를 CPU 메모리에다가 만들고 그걸 써먹을 것.
	// 시스템 메모리에 버텍스 버퍼를 VertexBufferSize만큼 잡는다.
	if (FAILED(pD3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pVertexBuffer)
	)))
	{
		__debugbreak();
	}

	//Vertice 데이터를 버텍스 버퍼에 카피한다.
	UINT8* pVertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0);

	if (FAILED(m_pVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin))))
		__debugbreak();

	memcpy(pVertexDataBegin, Vertices, sizeof(Vertices));
	m_pVertexBuffer->Unmap(0, nullptr);

	// 버텍스 버퍼 뷰 초기화
	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(BasicVertex);
	m_VertexBufferView.SizeInBytes = VertexBufferSize;

	bResult = TRUE;

lb_return:
	return bResult;
}

void CBasicMeshObject::Draw(ID3D12GraphicsCommandList* pCommandList)
{
	// 루트 시그니처 세팅
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);

	pCommandList->SetPipelineState(m_pPipelineState);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	pCommandList->DrawInstanced(3, 1, 0, 0);
}

void CBasicMeshObject::Cleanup()
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
	CleanupSharedResources();
}

void CBasicMeshObject::CleanupSharedResources()
{
	if (!m_dwInitRefCount)
		return;

	DWORD ref_count = --m_dwInitRefCount;
	if (!ref_count)
	{
		if (m_pRootSignature)
		{
			m_pRootSignature->Release();
			m_pRootSignature = nullptr;
		}
		if (m_pPipelineState)
		{
			m_pPipelineState->Release();
			m_pPipelineState = nullptr;
		}
	}
}