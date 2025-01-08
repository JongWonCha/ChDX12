#pragma once

enum BASIC_MESH_DESCRIPTOR_INDEX
{
	BASIC_MESH_DESCRIPTOR_INDEX_TEX = 0
};

class CD3D12Renderer;

class CBasicMeshObject
{
	static const UINT DESCRIPTOR_COUNT_FOR_DRAW = 1;	// | Tex

	// 모든 CBasicMeshObject 인스턴스들이 공유함.
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD				m_dwInitRefCount;

	CD3D12Renderer* m_pRenderer = nullptr;

	// 버텍스 데이터
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

	// 텍스처
	ID3D12Resource* m_pTexResource = nullptr;

	// Descriptor Table을 위한 자료
	UINT m_srvDescriptorSize = 0;
	ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr;

	BOOL InitCommonResources();
	void CleanupSharedResources();

	BOOL InitRootSignature();
	BOOL InitPipelineState();

	BOOL CreateDescriptorTable();

	void Cleanup();

public:
	BOOL Initialize(CD3D12Renderer* pRenderer);
	void Draw(ID3D12GraphicsCommandList* pCommandList);
	BOOL CreateMesh();

	CBasicMeshObject();
	~CBasicMeshObject();
};