#include <Windows.h>


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(

    HINSTANCE hInstance,      // アプリケーションの識別番号
    HINSTANCE hPrevInstance,  // 基本使わなくていい
    LPSTR lpCmdLine,          // コマンドライン引数（起動時のオプション）
    int nCmdShow              // ウィンドウの表示方法（最大化、最小化など）
)
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
    HWND hwnd = CreateWindowA(
        "GameWindow",
        "My Game",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, nCmdShow);

    bool nextFrame = true;
    while (nextFrame)
    {


        MSG msg{};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                nextFrame = false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return 0;
    }
    // 3. メッセージループ
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
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
    case WM_PAINT:

        return 0;

    case WM_KEYDOWN:

        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

