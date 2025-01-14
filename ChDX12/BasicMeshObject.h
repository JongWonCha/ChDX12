#pragma once

enum BASIC_MESH_DESCRIPTOR_INDEX
{
	BASIC_MESH_DESCRIPTOR_INDEX_CBV = 0,
	BASIC_MESH_DESCRIPTOR_INDEX_TEX = 1
};

class CD3D12Renderer;

class CBasicMeshObject
{
public:
	static const UINT DESCRIPTOR_COUNT_FOR_DRAW = 1;	// | Constant Buffer (전의 강의와 비교하여 텍스처는 사용하지 않음.)

private:
	// 모든 CBasicMeshObject 인스턴스들이 공유함.
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
	void Draw(ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos);
	BOOL CreateMesh();

	CBasicMeshObject();
	~CBasicMeshObject();
};