#pragma once

class Win32Application
{
public:
	static int Run(HINSTANCE hInstance, int nCmdShow);

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
