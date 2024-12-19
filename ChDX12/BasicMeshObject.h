#pragma once

class CD3D12Renderer;

class CBasicMeshObject
{
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD				m_dwInitRefCount;

	CD3D12Renderer* m_pRenderer = nullptr;

	// 버텍스 데이터
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

	BOOL InitCommonResources();
	void CleanupSharedResources();

	BOOL InitRootSignature();
	BOOL InitPipelineState();

	void Cleanup();

public:
	BOOL Initialize(CD3D12Renderer* pRenderer);
	void Draw(ID3D12GraphicsCommandList* pCommandList);
	BOOL CreateMesh();

	CBasicMeshObject();
	~CBasicMeshObject();
};