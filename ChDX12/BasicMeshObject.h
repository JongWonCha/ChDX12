#pragma once

enum BASIC_MESH_DESCRIPTOR_INDEX
{
	BASIC_MESH_DESCRIPTOR_INDEX_TEX = 0
};

class CD3D12Renderer;

class CBasicMeshObject
{
	static const UINT DESCRIPTOR_COUNT_FOR_DRAW = 1;	// | Tex

	// ��� CBasicMeshObject �ν��Ͻ����� ������.
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD				m_dwInitRefCount;

	CD3D12Renderer* m_pRenderer = nullptr;

	// ���ؽ� ������
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

	// �ؽ�ó
	ID3D12Resource* m_pTexResource = nullptr;

	// Descriptor Table�� ���� �ڷ�
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