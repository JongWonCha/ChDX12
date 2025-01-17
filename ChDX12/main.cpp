﻿// ChDX12.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "pch.h"
#include "Resource.h"
#include "D3D12Renderer.h"

// required .lib files
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND g_hMainWindow = nullptr;                   
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

CD3D12Renderer* g_pRenderer = nullptr;
void* g_pMeshObj = nullptr;
void* g_pTexHandle0 = nullptr;
void* g_pTexHandle1 = nullptr;

float g_fRot0 = 0.0f;
float g_fRot1 = 0.0f;

XMMATRIX g_matWorld0 = {};
XMMATRIX g_matWorld1 = {};

ULONGLONG g_PrvFrameCheckTick = 0;
ULONGLONG g_PrvUpdateTick = 0;
DWORD g_FrameCount = 0;

void RunGame();
void Update();

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHDX12, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    g_hMainWindow = InitInstance(hInstance, nCmdShow);
    if (!g_hMainWindow)
    {
        return FALSE;
    }


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHDX12));

    MSG msg;

    g_pRenderer = new CD3D12Renderer;
    g_pRenderer->Initialize(g_hMainWindow, TRUE, TRUE);
    g_pMeshObj = g_pRenderer->CreateBasicMeshObject();

    g_pTexHandle0 = g_pRenderer->CreateTiledTexture(16, 16, 192, 128, 255);
    g_pTexHandle1 = g_pRenderer->CreateTiledTexture(32, 32, 128, 255, 192);

    while (true)
    {
        BOOL bHasMsg = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);

        if (bHasMsg)
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            RunGame();
        }
    }
    if (g_pMeshObj)
    {
        g_pRenderer->DeleteBasicMeshObject(g_pMeshObj);
        g_pMeshObj = nullptr;
    }
    if (g_pTexHandle0)
    {
        g_pRenderer->DeleteTexture(g_pTexHandle0);
        g_pTexHandle0 = nullptr;
    }
    if (g_pTexHandle1)
    {
        g_pRenderer->DeleteTexture(g_pTexHandle1);
        g_pTexHandle1 = nullptr;
    }
    if (g_pRenderer)
    {
        delete g_pRenderer;
        g_pRenderer = nullptr;
    }
#ifdef _DEBUG
    _ASSERT(_CrtCheckMemory());
#endif
    return (int)msg.wParam;
}

void RunGame()
{
    g_FrameCount++;

    // begin
    ULONGLONG CurTick = GetTickCount64();

    g_pRenderer->BeginRender();

    // 게임 로직
    if (CurTick - g_PrvUpdateTick > 16)
    {
        // 60 fps로 씬 업데이트
        Update();
        g_PrvUpdateTick = CurTick;
    }

    

    // 오브젝트 렌더링
    g_pRenderer->RenderMeshObject(g_pMeshObj, &g_matWorld0, g_pTexHandle0);

    g_pRenderer->RenderMeshObject(g_pMeshObj, &g_matWorld1, g_pTexHandle1);
    // end
    g_pRenderer->EndRender();

    // present
    g_pRenderer->Present();

    if (CurTick - g_PrvFrameCheckTick > 1000)
    {
        g_PrvFrameCheckTick = CurTick;

        WCHAR wchTxt[64];
        swprintf_s(wchTxt, L"FPS : %u", g_FrameCount);
        SetWindowText(g_hMainWindow, wchTxt);

        g_FrameCount = 0;
    }
}

void Update()
{
    //
    // world matrix 0
    //
    g_matWorld0 = XMMatrixIdentity();

    // rotation 
    XMMATRIX matRot0 = XMMatrixRotationX(g_fRot0);

    // translation
    XMMATRIX matTrans0 = XMMatrixTranslation(-0.15f, 0.0f, 0.25f);

    // rot0 x trans0
    g_matWorld0 = XMMatrixMultiply(matRot0, matTrans0);

    //
    // world matrix 1
    //
    g_matWorld1 = XMMatrixIdentity();

    // world matrix 1
    // rotation 
    XMMATRIX matRot1 = XMMatrixRotationY(g_fRot1);

    // translation
    XMMATRIX matTrans1 = XMMatrixTranslation(0.15f, 0.0f, 0.25f);

    // rot1 x trans1
    g_matWorld1 = XMMatrixMultiply(matRot1, matTrans1);

    BOOL	bChangeTex = FALSE;
    g_fRot0 += 0.05f;
    if (g_fRot0 > 2.0f * 3.1415f)
    {
        g_fRot0 = 0.0f;
        bChangeTex = TRUE;
    }

    g_fRot1 += 0.1f;
    if (g_fRot1 > 2.0f * 3.1415f)
    {
        g_fRot1 = 0.0f;
    }
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHDX12));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CHDX12);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return nullptr;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
        {
        if (g_pRenderer)
        {
            RECT rect;
            GetClientRect(hWnd, &rect);
            DWORD dwWndWidth = rect.right - rect.left;
            DWORD dwWndHeight = rect.bottom - rect.top;
            g_pRenderer->UpdateWindowSize(dwWndWidth, dwWndHeight);
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
