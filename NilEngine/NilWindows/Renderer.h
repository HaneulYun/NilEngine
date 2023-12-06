#pragma once

class Renderer
{
public:
	void Initialize();
	void Destroy();
	void Render();

	UINT GetWidth() const { return width; }
	UINT GetHeight() const { return height; }

private:
	UINT width{ 1920 };
	UINT height{ 1080 };

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	static const UINT FrameCount = 2U;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	ComPtr<IDXGISwapChain3> swapChain;
	ComPtr<ID3D12Resource> renderTargets[FrameCount];

	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	UINT rtvDescriptorSize;
	
	UINT frameIndex;
	HANDLE fenceEvent;
	ComPtr<ID3D12Fence> fence;
	UINT fenceValue;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
};
