#include <Windows.h>
#include <vector>            
#include <DirectXMath.h>     
#include "global.h"
#include "polygon.h"

using namespace DirectX;     

// D3D12 初期化
void InitD3D12(HWND hwnd);

void BeginFrame();
void EndFrame();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// polygon インスタンス
polygon triangle;
polygon quad;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // 1. ウィンドウクラス登録
    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GameWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClass(&wc);

    // 2. ウィンドウ作成
    HWND hwnd = CreateWindowW(
        L"GameWindow",
        L"My Game",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_windowWidth, g_windowHeight,
        NULL, NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, nCmdShow);

    // 3. DirectX12 初期化
    InitD3D12(hwnd);

    // 4. polygon 初期化
    {
        // 白い三角形
        std::vector<XMFLOAT3> triPos =
        {
            {  0.0f,  0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            { -0.5f, -0.5f, 0.0f }
        };
        std::vector<XMFLOAT4> triColor =
        {
            {1,1,1,1},
            {1,1,1,1},
            {1,1,1,1}
        };
        triangle.Initialize(g_device, triPos, triColor);

        // 緑の四角形（2つの三角形）
        std::vector<XMFLOAT3> quadPos =
        {
            {-0.3f,  0.3f, 0.0f},
            { 0.3f,  0.3f, 0.0f},
            { 0.3f, -0.3f, 0.0f},

            {-0.3f,  0.3f, 0.0f},
            { 0.3f, -0.3f, 0.0f},
            {-0.3f, -0.3f, 0.0f}
        };
        std::vector<XMFLOAT4> quadColor(6, { 0,1,0,1 });
        quad.Initialize(g_device, quadPos, quadColor);
    }

    // 5. メッセージループ
    MSG msg{};
    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return 0;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // フレーム開始
        BeginFrame();

        // polygon 描画
        triangle.Draw(g_commandList);
        quad.Draw(g_commandList);

        // フレーム終了
        EndFrame();
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
