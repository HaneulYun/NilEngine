#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "Renderer.h"
#include "Win32Application.h"

using namespace Microsoft::WRL;

int Renderer::ShowDirectX12()
{
    UINT dxgiFactoryFlags = 0;

    {
        ComPtr<ID3D12Debug> debugController;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }

    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
    
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            break;
    }

    ComPtr<ID3D12Device> device;
    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ComPtr<ID3D12CommandQueue> commandQueue;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    const UINT FrameCount = 2U;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = 100;
    swapChainDesc.Height = 100;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    ComPtr<IDXGISwapChain3> swapChain;
    {
        ComPtr<IDXGISwapChain1> swapChain1;
        factory->CreateSwapChainForHwnd(commandQueue.Get(), Win32Application::GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain1);
        swapChain1.As(&swapChain);
    }

    UINT frameIndex = swapChain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    ComPtr<ID3D12Resource> renderTargets[FrameCount];
    for (UINT n = 0; n < FrameCount; ++n)
    {
        swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]));
        device->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    ComPtr<ID3D12CommandAllocator> commandAllocator;
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));


    ComPtr<ID3D12PipelineState> pipelineState;

    ComPtr<ID3D12GraphicsCommandList> commandList;
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    commandList->Close();

    ComPtr<ID3D12Fence> fence;
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    UINT fenceValue = 1;
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);


    commandAllocator->Reset();

    commandList->Reset(commandAllocator.Get(), pipelineState.Get());
    
    D3D12_RESOURCE_BARRIER resourceBarrier{
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE,
        D3D12_RESOURCE_TRANSITION_BARRIER{
            renderTargets[frameIndex].Get(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET }
    };
    commandList->ResourceBarrier(1, &resourceBarrier);

    rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += frameIndex * rtvDescriptorSize;

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    resourceBarrier.Transition = {
            renderTargets[frameIndex].Get(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT };
    commandList->ResourceBarrier(1, &resourceBarrier);

    commandList->Close();


    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, ppCommandLists);

    swapChain->Present(1, 0);

    UINT fencevalue = fenceValue;
    commandQueue->Signal(fence.Get(), fencevalue);
    ++fenceValue;

    if (fence->GetCompletedValue() < fencevalue)
    {
        fence->SetEventOnCompletion(fencevalue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();

    CloseHandle(fenceEvent);

    return 0;
}
