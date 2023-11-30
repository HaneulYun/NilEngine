#include "framework.h"
#include "Win32Application.h"
#include "Renderer.h"

HWND Win32Application::m_hwnd = nullptr;

int Win32Application::Run(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"NILWINDOWS";
    RegisterClassEx(&windowClass);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindowW(windowClass.lpszClassName, L"NilWindows", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    ShowWindow(m_hwnd, nCmdShow);

    Renderer::ShowDirectX12();

    // Main sample loop.
    MSG msg = {};
    while(msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
