#pragma comment( lib, "d3d9.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib")
#else
#pragma comment( lib, "d3dx9.lib")
#endif

#pragma comment(lib, "winmm.lib")
#pragma comment( lib, "QTE_Module.lib")

#include "..\QTE_Module\QTE_Module.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <timeapi.h>
#include <string>
#include <vector>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <tchar.h>

using namespace NS_QTE_Module;

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

class Sprite : public ISprite
{
public:

    Sprite(LPDIRECT3DDEVICE9 dev)
        : m_pD3DDevice(dev)
    {
    }

    void DrawImage(const int x, const int y, const int transparency) override
    {
        D3DXVECTOR3 pos {(float)x, (float)y, 0.f};
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect = {
            0,
            0,
            static_cast<LONG>(m_width),
            static_cast<LONG>(m_height) };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(
            m_pD3DTexture,
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->End();

    }

    void DrawImageRect(const int x, const int y, const int srcWidth, const int srcHeight, const int transparency) override
    {
        D3DXVECTOR3 pos {(float)x, (float)y, 0.f};
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect = { 0, 0, srcWidth, srcHeight };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(
            m_pD3DTexture,
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->End();
    }

    void DrawImageScaled(const int x, const int y, const int width, const int height, const int transparency) override
    {
        D3DXMATRIX oldTransform;
        D3DXMATRIX transform;
        D3DXVECTOR3 pos { (float)x, (float)y, 0.f };
        RECT rect = {
            0,
            0,
            static_cast<LONG>(m_width),
            static_cast<LONG>(m_height) };
        D3DXVECTOR3 center { 0, 0, 0 };
        float scaleX = (float)width / (float)m_width;
        float scaleY = (float)height / (float)m_height;

        D3DXMatrixScaling(&transform, scaleX, scaleY, 1.0f);
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        m_D3DSprite->GetTransform(&oldTransform);
        m_D3DSprite->SetTransform(&transform);
        pos.x = pos.x / scaleX;
        pos.y = pos.y / scaleY;
        m_D3DSprite->Draw(
            m_pD3DTexture,
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->SetTransform(&oldTransform);
        m_D3DSprite->End();
    }

    void Load(const std::wstring& filepath) override
    {
        LPD3DXSPRITE tempSprite { nullptr };
        if (FAILED(D3DXCreateSprite(m_pD3DDevice, &m_D3DSprite)))
        {
            throw std::exception("Failed to create a sprite.");
        }

        if (FAILED(D3DXCreateTextureFromFile(
            m_pD3DDevice,
            filepath.c_str(),
            &m_pD3DTexture)))
        {
            throw std::exception("Failed to create a texture.");
        }

        D3DSURFACE_DESC desc { };
        if (FAILED(m_pD3DTexture->GetLevelDesc(0, &desc)))
        {
            throw std::exception("Failed to create a texture.");
        }
        m_width = desc.Width;
        m_height = desc.Height;
    }

    ISprite* Create()
    {
        return new Sprite(m_pD3DDevice);
    }

    ~Sprite()
    {
        if (m_D3DSprite != nullptr)
        {
            m_D3DSprite->Release();
        }

        if (m_pD3DTexture != nullptr)
        {
            m_pD3DTexture->Release();
        }
    }

    void OnDeviceLost()
    {
        m_D3DSprite->OnLostDevice();
    }

    void OnDeviceReset()
    {
        m_D3DSprite->OnResetDevice();
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXSPRITE m_D3DSprite = NULL;
    LPDIRECT3DTEXTURE9 m_pD3DTexture = NULL;
    UINT m_width { 0 };
    UINT m_height { 0 };
};

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
LPD3DXFONT g_pFont = NULL;
LPD3DXMESH pMesh = NULL;
D3DMATERIAL9* pMaterials = NULL;
LPDIRECT3DTEXTURE9* pTextures = NULL;
DWORD dwNumMaterials = 0;
LPD3DXEFFECT pEffect = NULL;
D3DXMATERIAL* d3dxMaterials = NULL;
float f = 0.0f;
unsigned long long g_startTime = 0;

QTE_Module* story = nullptr;
QTE_Module::BarResult g_lastResult = QTE_Module::BarResult::None;

void TextDraw(LPD3DXFONT pFont, const wchar_t* text, int X, int Y)
{
    RECT rect = { X,Y,0,0 };
    pFont->DrawText(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 0, 0));
}

HRESULT InitD3D(HWND hWnd)
{
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
    {
        return E_FAIL;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Flags = 0;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
    {
        if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
        {
            return(E_FAIL);
        }
    }

    HRESULT hr = D3DXCreateFont(
        g_pd3dDevice,
        20,
        0,
        FW_HEAVY,
        1,
        false,
        SHIFTJIS_CHARSET,
        OUT_TT_ONLY_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE,
        _T("ＭＳ ゴシック"),
        &g_pFont);
    if FAILED(hr)
    {
        return(E_FAIL);
    }

    LPD3DXBUFFER pD3DXMtrlBuffer = NULL;

    if (FAILED(D3DXLoadMeshFromX(_T("cube.x"), D3DXMESH_SYSTEMMEM,
        g_pd3dDevice, NULL, &pD3DXMtrlBuffer, NULL,
        &dwNumMaterials, &pMesh)))
    {
        MessageBox(NULL, _T("Xファイルの読み込みに失敗しました"), NULL, MB_OK);
        return E_FAIL;
    }
    d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    pMaterials = new D3DMATERIAL9[dwNumMaterials];
    pTextures = new LPDIRECT3DTEXTURE9[dwNumMaterials];

    for (DWORD i = 0; i < dwNumMaterials; i++)
    {
        pMaterials[i] = d3dxMaterials[i].MatD3D;
        pMaterials[i].Ambient = pMaterials[i].Diffuse;
        pTextures[i] = NULL;

        int len = MultiByteToWideChar(CP_UTF8, 0, d3dxMaterials[i].pTextureFilename, -1, nullptr, 0);
        std::wstring result(len - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, d3dxMaterials[i].pTextureFilename, -1, &result[0], len);

        if (!result.empty())
        {
            if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, result.c_str(), &pTextures[i])))
            {
                MessageBox(NULL, _T("テクスチャの読み込みに失敗しました"), NULL, MB_OK);
            }
        }
    }
    pD3DXMtrlBuffer->Release();

    D3DXCreateEffectFromFile(
        g_pd3dDevice,
        _T("simple.fx"),
        NULL,
        NULL,
        D3DXSHADER_DEBUG,
        NULL,
        &pEffect,
        NULL
    );

    return S_OK;
}

void InitStory()
{
    Sprite* sprGrowingCircle = new Sprite(g_pd3dDevice);
    sprGrowingCircle->Load(_T("qte_growing_circle.png"));
    Sprite* sprTargetCircle = new Sprite(g_pd3dDevice);
    sprTargetCircle->Load(_T("qte_target_circle.png"));
    Sprite* sprButton = new Sprite(g_pd3dDevice);
    sprButton->Load(_T("qte_button.png"));
    story->SetCircleSprites(sprGrowingCircle, sprTargetCircle, sprButton, 1600, 900);
}

VOID Cleanup()
{
    SAFE_RELEASE(pMesh);
    SAFE_RELEASE(g_pFont);
    delete[] pMaterials;
    pMaterials = nullptr;
    delete[] pTextures;
    pTextures = nullptr;
    SAFE_RELEASE(pEffect);
    SAFE_RELEASE(g_pd3dDevice);
    SAFE_RELEASE(g_pD3D);
    timeEndPeriod(1);
}

VOID Render()
{
    if (NULL == g_pd3dDevice)
    {
        return;
    }
    float elapsedSec = (GetTickCount64() - g_startTime) / 1000.0f;
    f = elapsedSec * 0.5f;

    D3DXMATRIX mat;
    D3DXMATRIX View, Proj;
    D3DXMatrixPerspectiveFovLH(&Proj, D3DXToRadian(45), 1600.0f / 900.0f, 1.0f, 10000.0f);
    D3DXVECTOR3 vec1(3 * sinf(f), 3, -3 * cosf(f));
    D3DXVECTOR3 vec2(0, 0, 0);
    D3DXVECTOR3 vec3(0, 1, 0);
    D3DXMatrixLookAtLH(&View, &vec1, &vec2, &vec3);
    D3DXMatrixIdentity(&mat);
    mat = mat * View * Proj;
    pEffect->SetMatrix("matWorldViewProj", &mat);

    if (story != nullptr)
    {
        if (story->Update())
        {
            g_lastResult = story->GetBarResult();
            story->Finalize();
            delete story;
            story = nullptr;
        }
    }

    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(100, 100, 100), 1.0f, 0);

    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        wchar_t msg[128];
        wcscpy_s(msg, 128, _T("M key: start  Enter/Space/A: stop"));
        TextDraw(g_pFont, msg, 0, 0);

        if (story != nullptr)
        {
            auto result = story->GetBarResult();
            if (result == QTE_Module::BarResult::Success)
            {
                TextDraw(g_pFont, _T("SUCCESS"), 0, 30);
            }
            else if (result == QTE_Module::BarResult::Normal)
            {
                TextDraw(g_pFont, _T("NORMAL"), 0, 30);
            }
            else if (result == QTE_Module::BarResult::Failure)
            {
                TextDraw(g_pFont, _T("FAILURE"), 0, 30);
            }
        }
        else if (g_lastResult != QTE_Module::BarResult::None)
        {
            if (g_lastResult == QTE_Module::BarResult::Success)
            {
                TextDraw(g_pFont, _T("SUCCESS"), 0, 30);
            }
            else if (g_lastResult == QTE_Module::BarResult::Normal)
            {
                TextDraw(g_pFont, _T("NORMAL"), 0, 30);
            }
            else if (g_lastResult == QTE_Module::BarResult::Failure)
            {
                TextDraw(g_pFont, _T("FAILURE"), 0, 30);
            }
        }

        pEffect->SetTechnique("BasicTec");
        UINT numPass;
        pEffect->Begin(&numPass, 0);
        pEffect->BeginPass(0);
        for (DWORD i = 0; i < dwNumMaterials; i++)
        {
            pEffect->SetTexture("texture1", pTextures[i]);
            pMesh->DrawSubset(i);
        }
        if (story != nullptr)
        {
            story->Render();
        }
        pEffect->EndPass();
        pEffect->End();
        g_pd3dDevice->EndScene();
    }

    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        Render();
        return 0;
    case WM_SIZE:
        InvalidateRect(hWnd, NULL, true);
        return 0;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'M':
        {
            if (story != nullptr)
            {
                story->Finalize();
                delete story;
            }
            g_lastResult = QTE_Module::BarResult::None;
            story = new QTE_Module();
            InitStory();
            break;
        }
        case VK_RETURN:
        case VK_SPACE:
        {
            if (story != nullptr)
            {
                story->StopBarAnimation();
            }
            break;
        }
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ INT)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      _T("Window1"), NULL };
    RegisterClassEx(&wc);

    RECT rect;
    SetRect(&rect, 0, 0, 1600, 900);
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    rect.right = rect.right - rect.left;
    rect.bottom = rect.bottom - rect.top;
    rect.top = 0;
    rect.left = 0;

    HWND hWnd = CreateWindow(_T("Window1"), _T("Hello DirectX9 World !!"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom,
        NULL, NULL, wc.hInstance, NULL);

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

        timeBeginPeriod(1);
        g_startTime = GetTickCount64();

        MSG msg;
        bool running = true;
        while (running)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (running)
            {
                Render();
                Sleep(16);
            }
        }
    }

    UnregisterClass(_T("Window1"), wc.hInstance);
    Cleanup();
    _CrtDumpMemoryLeaks();
    return 0;
}
