#pragma once

class Renderer
{
public:
	void Initialize();
	void Destroy();
	void Render();

private:
	static const UINT FrameCount = 2U;

	ComPtr<IDXGISwapChain3> swapChain;
	ComPtr<ID3D12Resource> renderTargets[FrameCount];

	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	UINT rtvDescriptorSize;
	
	UINT frameIndex;
	HANDLE fenceEvent;
	ComPtr<ID3D12Fence> fence;
	UINT fenceValue;
};
