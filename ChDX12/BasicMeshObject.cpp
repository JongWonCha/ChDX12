#include "pch.h"
#include "typedef.h"
#include <d3dcompiler.h>
#include <d3dx12.h>
#include "D3D_Util/D3DUtil.h"
#include "D3D12Renderer.h"
#include "BasicMeshObject.h"
#include "D3D12ResourceManager.h"

ID3D12RootSignature* CBasicMeshObject::m_pRootSignature = nullptr;
ID3D12PipelineState* CBasicMeshObject::m_pPipelineState = nullptr;
DWORD CBasicMeshObject::m_dwInitRefCount = 0;

CBasicMeshObject::CBasicMeshObject()
{
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
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 : �ؽ�ó

	CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

	// default sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	SetDefaultSamplerDesc(&sampler, 0);
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	// ��ǲ ���̾ƿ� �㰡, Ư�� ���������� ���������� ���� ���ʿ��� ���� �ź�
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// ���� ����ִ� ��Ʈ �ñ״�ó�� �����. �Ⱦ��ű� ������ ����ִ� ������.
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();

	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	if (FAILED(D3DCompileFromFile(L"./Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, nullptr)))
	{
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"./Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, nullptr)))
	{
		__debugbreak();
	}

	// ���ؽ� ��ǲ ���̾ƿ� ����
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 28,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// �� �������� ��Ʈ �ñ״�ó�� �ʿ��ϱ� ������ ����̶� ��Ʈ �ñ״�ó�� ���� ���̴�.
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
	if (FAILED(pD3DDeivce->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState))))
	{
		__debugbreak();
	}

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

BOOL CBasicMeshObject::CreateDescriptorTable()
{
	BOOL bResult = FALSE;
	ID3D12Device5* pD3DDeivce = m_pRenderer->INL_GetD3DDevice();

	// �������� Descriptor Table�� ����� Descriptor Heap - 
	// Descriptor Table
	// | TEX
	m_srvDescriptorSize = pD3DDeivce->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 32 ����Ʈ

	// Descriptor Heap ����
	D3D12_DESCRIPTOR_HEAP_DESC commonHeapDesc = {};
	commonHeapDesc.NumDescriptors = DESCRIPTOR_COUNT_FOR_DRAW;
	commonHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	commonHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (FAILED(pD3DDeivce->CreateDescriptorHeap(&commonHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap))))
	{
		__debugbreak();
		goto lb_return;
	}
	bResult = TRUE;
lb_return:
	return bResult;
}

BOOL CBasicMeshObject::CreateMesh()
{
	// �ܺο��� ���ؽ� �����Ϳ� �ؽ�ó�� �Է��ϴ� ������ ������ ����.
	// ������ �ϴ� ������Ʈ ���ο��� ����

	BOOL bResult = FALSE;
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CD3D12ResourceManager* pResourceManager = m_pRenderer->INL_GetResourceManager();

	// ���ؽ� ���� ����
	// �ﰢ���� ������Ʈ�� ����
	BasicVertex Vertices[] =
	{
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
	};

	const UINT VertexBufferSize = sizeof(Vertices);

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(BasicVertex), (DWORD)_countof(Vertices), &m_VertexBufferView, &m_pVertexBuffer, Vertices)))
	{
		__debugbreak();
		goto lb_return;
	}

	// �ؽ�ó ����
	const UINT TexWidth = 16;
	const UINT TexHeight = 16;
	DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	BYTE* pImage = (BYTE*)malloc(TexWidth * TexHeight * 4);
	memset(pImage, 0, TexWidth * TexHeight * 4);

	BOOL bFirstColorIsWhite = TRUE;

	for (UINT y = 0; y < TexHeight; y++)
	{
		for (UINT x = 0; x < TexWidth; x++)
		{

			RGBA* pDest = (RGBA*)(pImage + (x + y * TexWidth) * 4);

			pDest->r = rand() % 255;
			pDest->g = rand() % 255;
			pDest->b = rand() % 255;

			if ((bFirstColorIsWhite + x) % 2)
			{
				pDest->r = 255;
				pDest->g = 255;
				pDest->b = 255;
			}
			else
			{
				pDest->r = 0;
				pDest->g = 0;
				pDest->b = 0;
			}
			pDest->a = 255;
		}
		bFirstColorIsWhite++;
		bFirstColorIsWhite %= 2;
	}
	pResourceManager->CreateTexture(&m_pTexResource, TexWidth, TexHeight, TexFormat, pImage);

	free(pImage);

	CreateDescriptorTable();

	// �ؽ�ó ���ҽ��κ��� ���̴� ���ҽ� �� ����
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = TexFormat;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // rgba�� ��� ���� �� ���ΰ�?(����� ������Ʈ 4�� ���)
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		CD3DX12_CPU_DESCRIPTOR_HANDLE srv(m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), BASIC_MESH_DESCRIPTOR_INDEX_TEX, m_srvDescriptorSize);
		pD3DDevice->CreateShaderResourceView(m_pTexResource, &SRVDesc, srv); // CPU �� ��ũ���͸� ����� �ش�.
	}


	bResult = TRUE;

lb_return:
	return bResult;
}

void CBasicMeshObject::Draw(ID3D12GraphicsCommandList* pCommandList)
{
	// ��Ʈ �ñ״�ó ����
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);

	pCommandList->SetDescriptorHeaps(1, &m_pDescriptorHeap);// ���̺��� ������ ����. �� ���̺��� ��� ���� �޸𸮿� �ö� �ִ��� �˷���� ��.

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTable(m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	pCommandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorTable);

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

	if (m_pDescriptorHeap)
	{
		m_pDescriptorHeap->Release();
		m_pDescriptorHeap = nullptr;
	}

	if (m_pTexResource)
	{
		m_pTexResource->Release();
		m_pTexResource = nullptr;
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

CBasicMeshObject::~CBasicMeshObject()
{
	Cleanup();
}