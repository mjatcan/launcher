#define DIRECTINPUT_VERSION 0x0800
#include "stdafx.h"
#include <tlhelp32.h>
#include "resource.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <tchar.h>
#include "LauncherEngine.h"
#include "CRC.h"
#include "MD5.h"
#include "sha.hpp"
#include <dwmapi.h>
#include "gif.h"
#include "GDIHelper.h"
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma warning(disable:4244)
#pragma warning(disable:4129)
static int iW = 802;
static int iH = 594;
static int iWOff = 0;
CheckSum Check;
MD5	md5;

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};


thyke_Test* thyke_t = NULL;
// Font
LPD3DXFONT m_font = NULL;
RECT TextStatePos;
RECT TextNoticePos;
RECT TextPagePos;
int NoticeOffsetY = 0;
int TextStateBaseX = 0;

// Textures
IDirect3DTexture9* LauncherBackgroundTexture = NULL;

IDirect3DTexture9* StartButtonTexture = NULL;
IDirect3DTexture9* StartButtonHoverTexture = NULL;
IDirect3DTexture9* StartButtonDownTexture = NULL;

//IDirect3DTexture9* HomePageButtonTexture = NULL;
//IDirect3DTexture9* HomePageButtonHoverTexture = NULL;
//IDirect3DTexture9* HomePageButtonDownTexture = NULL;

IDirect3DTexture9* SettingsButtonTexture = NULL;
IDirect3DTexture9* SettingsButtonDownTexture = NULL;
IDirect3DTexture9* SettingsButtonHoverTexture = NULL;

IDirect3DTexture9* CloseButtonTexture = NULL;
IDirect3DTexture9* CloseButtonHoverTexture = NULL;
IDirect3DTexture9* CloseButtonDownTexture = NULL;

//IDirect3DTexture9* DiscordButtonTexture = NULL;
//IDirect3DTexture9* DiscordButtonHoverTexture = NULL;
//IDirect3DTexture9* DiscordButtonDownTexture = NULL;
//
//IDirect3DTexture9* ForumButtonTexture = NULL;
//IDirect3DTexture9* ForumButtonHoverTexture = NULL;
//IDirect3DTexture9* ForumButtonDownTexture = NULL;
//
//IDirect3DTexture9* FacebookButtonTexture = NULL;
//IDirect3DTexture9* FacebookButtonHoverTexture = NULL;
//IDirect3DTexture9* FacebookButtonDownTexture = NULL;

IDirect3DTexture9* ProgressTexture = NULL;
IDirect3DTexture9* ProgressFillTexture = NULL;


// Sprites
LPD3DXSPRITE LauncherSprite = NULL;

// Vectors
D3DXVECTOR3 LauncherBackgorundPosition(0, 0, 0);
D3DXVECTOR3 StartButtonPosition(608.0f + iWOff, 509.0f, 0);
//D3DXVECTOR3 HomePageButtonPosition(23.0f + iWOff, 511.0f, 0);
D3DXVECTOR3 SettingsButtonPosition(161.0f + iWOff, 511.0f, 0);
D3DXVECTOR3 CloseButtonPosition(765.0f + iWOff, 3.0f, 0);
//D3DXVECTOR3 DiscordButtonPosition(608.0f + iWOff, 509.0f, 0);
//D3DXVECTOR3 ForumButtonPosition(23.0f + iWOff, 511.0f, 0);
//D3DXVECTOR3 FacebookButtonPosition(161.0f + iWOff, 511.0f, 0);
D3DXVECTOR3 ProgressPosition(24.0f + iWOff, 557.0f, 0);
RECT pbFill;

// Surfaces
D3DSURFACE_DESC StartButtonSurface;
//D3DSURFACE_DESC HomePageButtonSurface;
D3DSURFACE_DESC SettingsButtonSurface;
D3DSURFACE_DESC CloseButtonSurface;
//D3DSURFACE_DESC DiscordButtonSurface;
//D3DSURFACE_DESC ForumButtonSurface;
//D3DSURFACE_DESC FacebookButtonSurface;
D3DSURFACE_DESC ProgressSurface;
// States
enum ButtonState
{
    STATE_NORMAL = 0,
    STATE_HOVER,
    STATE_DOWN,
    STATE_UP
};
// Statics
#define COL_N D3DCOLOR_ARGB(255, 200, 200, 200)
#define COL_H D3DCOLOR_ARGB(255, 255, 255, 255)
#define COL_D D3DCOLOR_ARGB(255, 150, 150, 150)
#define COL_X D3DCOLOR_ARGB(255, 80, 80, 80)
ButtonState states[7] = {};
static ButtonState lastStartState = STATE_NORMAL;
//static ButtonState lastHomepageState = STATE_NORMAL;
static ButtonState lastSettingsState = STATE_NORMAL;
static ButtonState lastCloseState = STATE_NORMAL;
//static ButtonState lastDiscordState = STATE_NORMAL;
//static ButtonState lastForumState = STATE_NORMAL;
//static ButtonState lastFacebookState = STATE_NORMAL;

static HCURSOR hCursorNormal;
static HCURSOR hCursorHand;
static HCURSOR hCursorClick;
HWND mainWindow;
HMODULE mainInstance;
size_t sayfalar;
size_t sayfa;

// Engine
Launcher* Engine;
int loadCount = 0;
std::string launcherdir;
//std::string Address_HomePage;
//std::string Address_Discord;
//std::string Address_Forum;
//std::string Address_Facebook;
CHAR WP[MAXCHAR];

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();

static void _string_format(const std::string fmt, std::string* result, va_list args)
{
    char buffer[1024];
    _vsnprintf(buffer, sizeof(buffer), fmt.c_str(), args);
    *result = buffer;
}

static std::string string_format(const std::string fmt, ...)
{
    std::string result;
    va_list ap;

    va_start(ap, fmt);
    _string_format(fmt, &result, ap);
    va_end(ap);

    return result;
}

extern std::string hashfile(const std::string filename)
{
    SHA1 checksum;
    return checksum.from_file(filename);
}

std::string GetDLLDir()
{
    char result[MAX_PATH];
    return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}

std::string GetDir()
{
    std::string dir = GetDLLDir();
   dir.replace(dir.size() - 13, 13, "");
    return dir;
}

std::string FileDirGet(std::string FileName)
{
    std::string x;
    std::string name = xorstr("%s/%s");
    x = string_format(name, GetDir().c_str(), FileName.c_str());
    return x;
}
void NxACSEncrypt()
{
  
}

void NxACSEncryptt()
{
   
}

void WebLinkAdded()
{
    //Address_HomePage = "https://mykozone.com/";
    //Address_Forum = "https://forum.mykozone.com/";
    //Address_Facebook = "https://www.facebook.com/mykozone1098/";      //AlfaACS Lisans
    //Address_Discord = "https://discord.gg/mykozone";
}

int WINAPI SocketSystem()
{
    Engine->Start();
    return 1;
}

// dosyalardan
bool LoadTextures()
{
    D3DXCreateSprite(g_pd3dDevice, &LauncherSprite);
    bool textureFail = false;
    std::vector<std::string> unloadedResources;

    HRESULT res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\Background.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &LauncherBackgroundTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\Background.png"));
    }
    // Start
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\StartMouseOut.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &StartButtonTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\StartMouseOut.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\StartMouseOver.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &StartButtonHoverTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\StartMouseOver.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\StartMouseClick.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &StartButtonDownTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\StartMouseClick.png"));
    } 
    // HomePage
    /*res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\HomeMouseOut.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &HomePageButtonTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\HomeMouseOut.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\HomeMouseOver.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &HomePageButtonHoverTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\HomeMouseOver.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\HomeMouseClick.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &HomePageButtonDownTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\HomeMouseClick.png"));
    }*/
    // Option
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\OptionsMouseOut.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &SettingsButtonTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\OptionsMouseOut.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\OptionsMouseOver.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &SettingsButtonHoverTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\OptionsMouseOver.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\OptionsMouseClick.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &SettingsButtonDownTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\OptionsMouseClick.png"));
    }
    // Close
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\CloseMouseOut.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &CloseButtonTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\CloseMouseOut.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\CloseMouseOver.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &CloseButtonHoverTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\CloseMouseOver.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\CloseMouseClick.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &CloseButtonDownTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\CloseMouseClick.png"));
    }
    // Discord
    /*res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\DiscordMouseOut.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &DiscordButtonTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\DiscordMouseOut.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\DiscordMouseOver.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &DiscordButtonHoverTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\DiscordMouseOver.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\DiscordMouseClick.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &DiscordButtonDownTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\DiscordMouseClick.png"));
    }*/
    // Forum   
    /*res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\ForumMouseOut.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &ForumButtonTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\ForumMouseOut.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\ForumMouseOver.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &ForumButtonHoverTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\ForumMouseOver.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\ForumMouseClick.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &ForumButtonDownTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\ForumMouseClick.png"));
    }*/
    // Facebook
    /*res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\FacebookMouseOut.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &FacebookButtonTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\FacebookMouseOut.png"));
    }  
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\FacebookMouseOver.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &FacebookButtonHoverTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\FacebookMouseOver.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("NxACS\\Launcher\\FacebookMouseClick.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &FacebookButtonDownTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("NxACS\\Launcher\\FacebookMouseClick.png"));
    }*/
    // Progress Bar
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\ProgressEmpty.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &ProgressTexture);
    if (res != D3D_OK) {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\ProgressEmpty.png"));
    }
    res = D3DXCreateTextureFromFileEx(g_pd3dDevice, xorstr("AlfaACS\\Launcher\\ProgressValue.png"), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DCOLOR_ARGB(128, 128, 128, 128), 0, 0, &ProgressFillTexture);
    if (res != D3D_OK)
    {
        textureFail = true;
        unloadedResources.push_back(xorstr("AlfaACS\\Launcher\\ProgressValue.png"));
    }
   
    launcherdir = GetDir();
    WebLinkAdded();

    if (textureFail)
    {
        std::string fails = "";
        for (std::string i : unloadedResources)
            fails += i + "\n";

        MessageBoxA(mainWindow, std::string(xorstr("Resources couldn't be loaded:\n") + fails).c_str(), "Error", MB_ICONEXCLAMATION);
        return false;
    }

    StartButtonTexture->GetLevelDesc(0, &StartButtonSurface);
    //HomePageButtonTexture->GetLevelDesc(0, &HomePageButtonSurface);
    SettingsButtonTexture->GetLevelDesc(0, &SettingsButtonSurface);
    CloseButtonTexture->GetLevelDesc(0, &CloseButtonSurface);
    //DiscordButtonTexture->GetLevelDesc(0, &DiscordButtonSurface);
    //ForumButtonTexture->GetLevelDesc(0, &ForumButtonSurface);
    //FacebookButtonTexture->GetLevelDesc(0, &FacebookButtonSurface);
    ProgressTexture->GetLevelDesc(0, &ProgressSurface);

    return true;
}

VOID CenterWindow(HWND hwnd, HWND hwndParent, int Width, int Height)
{
    RECT rc;
    if (hwndParent == NULL)
        hwndParent = GetDesktopWindow();

    GetClientRect(hwndParent, &rc);
    MoveWindow(hwnd,(rc.right - rc.left - Width) / 2,(rc.bottom - rc.top - Height) / 2,Width,Height,TRUE);
    return;
}

int GetTextWidth(const char* szText, LPD3DXFONT pFont)
{
    RECT rcRect = { 0,0,0,0 };
    if (pFont)
    {
        pFont->DrawTextA(NULL, szText, strlen(szText), &rcRect, DT_CALCRECT,
            D3DCOLOR_XRGB(0, 0, 0));
    }
    return rcRect.right - rcRect.left;
}

static int lisansTarih[] = { 01, 12, 2050 }; // Lisans
// iki lisans þekli de ayný anda çalýþýr
static std::string ipLisanslari[] = { xorstr("176.98.43.221"), xorstr("176.98.43.221"), xorstr("176.98.43.221") };    //AlfaACS lisans
// x den öncesine bakar
static std::string subnetLisanlar[] = { xorstr("176.98.43.221") };

bool IsLicensed(std::string ip)
{
    bool ret = false;
    for (std::string pattern : subnetLisanlar)
    {
        const char* tmp = pattern.c_str();
        bool f = true;
        for (size_t i = 0; i < pattern.length(); i++)
            if (tmp[i] != 'x' && tmp[i] != ip.c_str()[i])
                f = false;
        ret = f;
    }

    for (std::string license : ipLisanslari)
        if (ip == license)
            ret = true;

    return ret;
}

HBITMAP hBMP;
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CreateMutexA(0, FALSE, xorstr("Local\\$launcher$"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return false;

    GetCurrentDirectoryA(MAX_PATH, WP);

    std::string dosyalarNerdeLenAmq = xorstr("\\AlfaACS\\Launcher\\");

    hBMP = (HBITMAP)LoadImage(NULL, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("bg.bmp")).c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    iW = GetPrivateProfileIntA(xorstr("LAUNCHER"), xorstr("WIDTH"), 992, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    iH = GetPrivateProfileIntA(xorstr("LAUNCHER"), xorstr("HEIGHT"), 576, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());

    int tmpX = 0, tmpY = 0, fontSize = 0, fontWeight;
    char InfoFont[25];
    // BG
    tmpX = GetPrivateProfileIntA(xorstr("BG"), xorstr("X"), 0, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    tmpY = GetPrivateProfileIntA(xorstr("BG"), xorstr("Y"), 0, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    LauncherBackgorundPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    // Start Button
    tmpX = GetPrivateProfileIntA(xorstr("START_BUTTON"), xorstr("X"), 675, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    tmpY = GetPrivateProfileIntA(xorstr("START_BUTTON"), xorstr("Y"), 480, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    StartButtonPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    // Homepage Button
    //tmpX = GetPrivateProfileIntA(xorstr("HOMEPAGE_BUTTON"), xorstr("X"), 23, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //tmpY = GetPrivateProfileIntA(xorstr("HOMEPAGE_BUTTON"), xorstr("Y"), 511, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //HomePageButtonPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    // Settings Button
    tmpX = GetPrivateProfileIntA(xorstr("SETTINGS_BUTTON"), xorstr("X"), 490, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str()); //AlfaACS Launcher Tasarým
    tmpY = GetPrivateProfileIntA(xorstr("SETTINGS_BUTTON"), xorstr("Y"), 500, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str()); //AlfaACS Lisans
    SettingsButtonPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    // Close Button
    tmpX = GetPrivateProfileIntA(xorstr("CLOSE_BUTTON"), xorstr("X"), 950, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    tmpY = GetPrivateProfileIntA(xorstr("CLOSE_BUTTON"), xorstr("Y"), 10, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    CloseButtonPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    //Discord Button
    //tmpX = GetPrivateProfileIntA(xorstr("DISCORD_BUTTON"), xorstr("X"), 608, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //tmpY = GetPrivateProfileIntA(xorstr("DISCORD_BUTTON"), xorstr("Y"), 509, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //DiscordButtonPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    // Forum Button
    //tmpX = GetPrivateProfileIntA(xorstr("FORUM_BUTTON"), xorstr("X"), 161, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //tmpY = GetPrivateProfileIntA(xorstr("FORUM_BUTTON"), xorstr("Y"), 511, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //ForumButtonPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    // Facebook Button
    //tmpX = GetPrivateProfileIntA(xorstr("FACEBOOK_BUTTON"), xorstr("X"), 161, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //tmpY = GetPrivateProfileIntA(xorstr("FACEBOOK_BUTTON"), xorstr("Y"), 511, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    //FacebookButtonPosition = D3DXVECTOR3(tmpX, tmpY, 0);
     // Progressbar
    tmpX = GetPrivateProfileIntA(xorstr("PROGRESSBAR"), xorstr("X"), 7, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    tmpY = GetPrivateProfileIntA(xorstr("PROGRESSBAR"), xorstr("Y"), 432, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    ProgressPosition = D3DXVECTOR3(tmpX, tmpY, 0);
    // Font
    fontSize = GetPrivateProfileIntA(xorstr("FONT"), xorstr("FONT_SIZE"), 10, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    fontWeight = GetPrivateProfileIntA(xorstr("FONT"), xorstr("FONT_WEIGHT"), 500, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    GetPrivateProfileStringA(xorstr("FONT"), xorstr("FAMILY"), "Verdana", InfoFont, 25, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    // Info Text
    TextStateBaseX = GetPrivateProfileIntA(xorstr("INFO_TEXT"), xorstr("X"), 10, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    TextStatePos.top = GetPrivateProfileIntA(xorstr("INFO_TEXT"), xorstr("Y"), 435, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    // Page Text
    TextPagePos.left = GetPrivateProfileIntA(xorstr("PAGE_TEXT"), xorstr("X"), 27, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    TextPagePos.top = GetPrivateProfileIntA(xorstr("PAGE_TEXT"), xorstr("Y"), 550, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    // Notices
    TextNoticePos.left = GetPrivateProfileIntA(xorstr("NOTICES"), xorstr("START_X"), 480, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    TextNoticePos.top = GetPrivateProfileIntA(xorstr("NOTICES"), xorstr("START_Y"), 200, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());
    NoticeOffsetY = GetPrivateProfileIntA(xorstr("NOTICES"), xorstr("OFFSET_Y"), 23, (std::string(WP) + dosyalarNerdeLenAmq + xorstr("UIXSettings.ini")).c_str());

    TextStatePos.right = iW;
    TextStatePos.bottom = iH;
    TextPagePos.right = iW;
    TextPagePos.bottom = iH;
    TextNoticePos.bottom = iH;
    TextNoticePos.right = iW;

    static int noticeBaseY = TextNoticePos.top;

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Launcher"), NULL };
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindowA(wc.lpszClassName, _T(xorstr("Launcher")), WS_POPUP, 100, 100, iW, iH, NULL, NULL, wc.hInstance, NULL);
    CenterWindow(hwnd, NULL, iW, iH);
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(255, 0, 234), 0, LWA_COLORKEY);  // launcher resminin olduðu yerde silinecek olan renk
    mainWindow = hwnd;

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    if (!LoadTextures())
    {
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }

    mainInstance = hInstance;
    sayfa = 1;

    for (int i = 0; i < 6; i++)
        states[i] = STATE_NORMAL;

	Engine = new Launcher();
    Engine->window = mainWindow;
    Engine->cmd = lpCmdLine;

    if (m_font == NULL)
        D3DXCreateFontA(g_pd3dDevice, fontSize, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, MONO_FONT | FF_DONTCARE, InfoFont, &m_font);

#define GUN 0
#define AY 1
#define YIL 2
    time_t rawtime;
    time(&rawtime);
    struct tm* timeInfoEx = localtime(&rawtime);

    bool kappa = false;

    int yil = timeInfoEx->tm_year + 1900;

    if (yil > lisansTarih[YIL])
        kappa = true;
    else if (timeInfoEx->tm_mon > lisansTarih[AY] - 1 && yil == lisansTarih[YIL])
        kappa = true;
    else if (timeInfoEx->tm_mday > lisansTarih[GUN] && timeInfoEx->tm_mon == lisansTarih[AY] - 1 && yil == lisansTarih[YIL])
        kappa = true;

    if (!IsLicensed(Engine->m_settingsIP))
        kappa = true;

    if (kappa)
        Engine->SetState(xorstr("Unknown data stream."));
    else 
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SocketSystem, NULL, NULL, NULL);
   
    hCursorNormal = LoadCursor(NULL, IDC_ARROW);
    hCursorHand = LoadCursor(NULL, IDC_ARROW);
	hCursorClick = LoadCursor(NULL, IDC_ARROW);
    SetCursor(hCursorNormal);

    GetCurrentDirectoryA(MAX_PATH, Engine->WorkingPath);
    thyke_t = new thyke_Test;

    // Main loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {

            if (GetAsyncKeyState(VK_LBUTTON))
                SetCursor(hCursorClick);
            else
                SetCursor(hCursorNormal);

            g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
            g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
            g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
            g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 1.0f, 0);
            if (g_pd3dDevice->BeginScene() >= 0)
            {
                pbFill.left = 0;
                pbFill.right = ProgressSurface.Width * Engine->GetPercent() / 100;
                pbFill.top = 0;
                pbFill.bottom = ProgressSurface.Height;

                LauncherSprite->Begin(D3DXSPRITE_ALPHABLEND);
                LauncherSprite->Draw(LauncherBackgroundTexture, NULL, NULL, &LauncherBackgorundPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                LauncherSprite->Draw(states[0] == STATE_NORMAL ? StartButtonTexture : (states[0] == STATE_DOWN ? StartButtonDownTexture : StartButtonHoverTexture), NULL, NULL, &StartButtonPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                //LauncherSprite->Draw(states[1] == STATE_NORMAL ? HomePageButtonTexture : (states[1] == STATE_DOWN ? HomePageButtonDownTexture : HomePageButtonHoverTexture), NULL, NULL, &HomePageButtonPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                LauncherSprite->Draw(states[2] == STATE_NORMAL ? SettingsButtonTexture : (states[2] == STATE_DOWN ? SettingsButtonDownTexture : SettingsButtonHoverTexture), NULL, NULL, &SettingsButtonPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                LauncherSprite->Draw(states[3] == STATE_NORMAL ? CloseButtonTexture : (states[3] == STATE_DOWN ? CloseButtonDownTexture : CloseButtonHoverTexture), NULL, NULL, &CloseButtonPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                //LauncherSprite->Draw(states[4] == STATE_NORMAL ? DiscordButtonTexture : (states[4] == STATE_DOWN ? DiscordButtonDownTexture : DiscordButtonHoverTexture), NULL, NULL, &DiscordButtonPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                //LauncherSprite->Draw(states[5] == STATE_NORMAL ? ForumButtonTexture : (states[5] == STATE_DOWN ? ForumButtonDownTexture : ForumButtonHoverTexture), NULL, NULL, &ForumButtonPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                //LauncherSprite->Draw(states[6] == STATE_NORMAL ? FacebookButtonTexture : (states[6] == STATE_DOWN ? FacebookButtonDownTexture : FacebookButtonHoverTexture), NULL, NULL, &FacebookButtonPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                LauncherSprite->Draw(ProgressTexture, NULL, NULL, &ProgressPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
                LauncherSprite->Draw(ProgressFillTexture, &pbFill, NULL, &ProgressPosition, D3DCOLOR_ARGB(255, 255, 255, 255));
            
                LauncherSprite->End();

                int width = GetTextWidth(Engine->GetState().c_str(), m_font);

                TextStatePos.left = TextStateBaseX;

                m_font->DrawTextA(NULL, Engine->GetState().c_str(), -1, &TextStatePos, 0, D3DCOLOR_ARGB(255, 255, 255, 255));


                g_pd3dDevice->EndScene();
            }
            HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

            if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
                ResetDevice();
        }
    }
  

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
    
    return msg.wParam;
}

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    HRESULT ret = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice);

    if (ret != D3D_OK)
    {
        switch (ret) 
        {
        case D3DERR_DEVICELOST:
            MessageBoxA(mainWindow, xorstr("DirectX device cannot be created."), "D3DERR_DEVICELOST", MB_ICONEXCLAMATION);
            break;
        case D3DERR_INVALIDCALL:
            MessageBoxA(mainWindow, xorstr("DirectX device cannot be created."), "D3DERR_INVALIDCALL", MB_ICONEXCLAMATION);
            break;
        case D3DERR_NOTAVAILABLE:
            MessageBoxA(mainWindow, xorstr("DirectX device cannot be created."), "D3DERR_NOTAVAILABLE", MB_ICONEXCLAMATION);
            break;
        case D3DERR_OUTOFVIDEOMEMORY:
            MessageBoxA(mainWindow, xorstr("DirectX device cannot be created."), "D3DERR_OUTOFVIDEOMEMORY", MB_ICONEXCLAMATION);
            break;
        default:
            MessageBoxA(mainWindow, xorstr("DirectX device cannot be created."), std::to_string(ret).c_str(), MB_ICONEXCLAMATION);
            break;
        }
        return false;
    }

	return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    //while (hr != D3D_OK)
    //    hr = g_pd3dDevice->Reset(&g_d3dpp);

	//HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	/*if (hr == D3DERR_INVALIDCALL)
		ASSERT(0);*/
}

#define YOUR_UNIQUE_ID 5576
GDIHelper gdiHelper;

static TCHAR szWindowClass[] = _T("Knight OnLine Curse");
static TCHAR szTitle[] = _T("Knight OnLine Curse");
HWND hLoadHwnd;
LRESULT CALLBACK WndProc222(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndProc222(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message)
    {
    case WM_ACTIVATE:
        break;
    case WM_COMMAND:
        break;
    case WM_CREATE:
    {
        //std::string Path = std::string(Engine->WorkingPath) + "\\NxACS\\Launcher\\" + "curse_loading.gif";
        //gdiHelper.DisplayImageFromFile(
        //    Path.c_str(),						//File location
        //    hWnd,								//Handle to the Window
        //    YOUR_UNIQUE_ID,						//Unique ID of your control, declare your own.
        //    0,									//xPosition
        //    0,									//yPosition
        //    700,								//width 
        //    400,								//height
        //    false								//looped
        //);
        
        gdiHelper.DisplayImageFromResource(
            mainInstance,
            MAKEINTRESOURCEW(IDB_LOADING),
            (LPCWSTR)RT_RCDATA,
            hWnd,
            YOUR_UNIQUE_ID,						//Unique ID of your control, declare your own.
            0,									//xPosition
            0,									//yPosition
            700,								//width 
            400								    //height
        );
    }
    break;
    case WM_DESTROY:
    {
        /* Don't forget to call destroy. (2) */
        gdiHelper.Destroy();
        TerminateProcess(GetCurrentProcess(), 0);
        PostQuitMessage(0);
        break;
    }
    break;
    case WM_LBUTTONDBLCLK:
    {
        int a = 0;
    }
    break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int thyke_Test::SetupBanner()
{
    ShowWindow(mainWindow, FALSE);

    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSEX wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc222;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = mainInstance;
    wcex.hIcon = LoadIcon(mainInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), szTitle, NULL);
        return 1;
    }

    HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_POPUP/*WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME*/, 0, 0, 700, 400, NULL, NULL, wcex.hInstance, NULL);
    hLoadHwnd = hwnd;

    if (!hwnd) {
        MessageBox(NULL, _T("Call to CreateWindow failed!"), szTitle, NULL);
        return 1;
    }

    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);

    //Center the Window
    RECT rc;
    GetWindowRect(hwnd, &rc);
    int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
    int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
    SetWindowPos(hwnd, HWND_TOP, xPos, yPos, 700, 400, SWP_NOZORDER);

    //SetWindowPos(FindWindow(Nil, PCHAR(program1)), HWND_TOP, 0, 0, Screen.Width, Screen.Height, SWP_SHOWWINDOW)

    ShowWindow(hwnd, TRUE);
    UpdateWindow(hwnd);

    MSG msg;
    BOOL bRet;
    ZeroMemory(&msg, sizeof(msg));

    while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0) {

        if (bRet == -1)
        {
            goto end;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

end:
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wcex.lpszClassName, wcex.hInstance);
    GdiplusShutdown(gdiplusToken); //dont forget to shut down the gdi+ token.
    return 1;
}


bool _fexists(std::string& filename)
{
    std::ifstream ifile(filename.c_str());
    return (bool)ifile;
}

void StartClick()
{
    if (lastStartState == STATE_DOWN && Engine->IsReady())
    {
        states[0] = STATE_HOVER;
        lastStartState = STATE_HOVER;
		std::string param = std::to_string(GetCurrentProcessId());

        /*string myname(std::string(Engine->WorkingPath) + xorstr("\\KnightOnLine.exe"));
        if (!_fexists(myname))
        {
            MessageBoxA(NULL, xorstr("KnightOnLine.exe not found."), xorstr("Launcher"), MB_OK | MB_ICONEXCLAMATION);
            return;
        }*/
       
        thyke_t->SetupBanner();
    }
}

void HomepageClick()
{
    /*if (lastHomepageState == STATE_DOWN && Engine->IsReady()) {
        states[1] = STATE_HOVER;
        lastHomepageState = STATE_HOVER;
        ShellExecuteA(NULL, NULL, Address_HomePage.c_str(), NULL, NULL, SW_SHOW);
    }*/
}

void SettingsClick()
{
    if (lastSettingsState == STATE_DOWN && Engine->IsReady()) {
        states[2] = STATE_HOVER;
        lastSettingsState = STATE_HOVER;
        if ((long)ShellExecuteA(NULL, NULL, xorstr("Option.exe"), NULL, NULL, SW_RESTORE) == ERROR_FILE_NOT_FOUND)
            MessageBoxA(mainWindow, xorstr("Option.exe not found."), xorstr("Launcher"), MB_ICONINFORMATION);
        else
            ::PostQuitMessage(0);
    }
}

void CloseClick()
{
    if (lastCloseState == STATE_DOWN)
        ::PostQuitMessage(0);
}

void DiscordClick()
{
    /*if (lastDiscordState == STATE_DOWN && Engine->IsReady()) {
        states[4] = STATE_HOVER;
        lastDiscordState = STATE_HOVER;
        std::string param = std::to_string(GetCurrentProcessId());
        ShellExecuteA(NULL, NULL, Address_Discord.c_str(), NULL, NULL, SW_SHOW);
    }*/
}

void ForumClick()
{
    /*if (lastForumState == STATE_DOWN && Engine->IsReady()) {
        states[5] = STATE_HOVER;
        lastForumState = STATE_HOVER;
        ShellExecuteA(NULL, NULL, Address_Forum.c_str(), NULL, NULL, SW_SHOW);
    }*/
}

void FacebookClick()
{
    /*if (lastFacebookState == STATE_DOWN && Engine->IsReady()) {
        states[6] = STATE_HOVER;
        lastFacebookState = STATE_HOVER;
        ShellExecuteA(NULL, NULL, Address_Facebook.c_str(), NULL, NULL, SW_SHOW);
    }*/
}


bool isInArea(int x, int y)
{
    if (x >= StartButtonPosition.x && x <= StartButtonPosition.x + StartButtonSurface.Width && y >= StartButtonPosition.y && y <= StartButtonPosition.y + StartButtonSurface.Height && Engine->IsReady())
    {
        return true;
    }
    /*if (x >= HomePageButtonPosition.x && x <= HomePageButtonPosition.x + HomePageButtonSurface.Width && y >= HomePageButtonPosition.y && y <= HomePageButtonPosition.y + HomePageButtonSurface.Height && Engine->IsReady())
    {
        return true;
    }*/
    if (x >= SettingsButtonPosition.x && x <= SettingsButtonPosition.x + SettingsButtonSurface.Width && y >= SettingsButtonPosition.y && y <= SettingsButtonPosition.y + SettingsButtonSurface.Height && Engine->IsReady())
    {
        return true;
    }
    if (x >= CloseButtonPosition.x && x <= CloseButtonPosition.x + CloseButtonSurface.Width && y >= CloseButtonPosition.y && y <= CloseButtonPosition.y + CloseButtonSurface.Height)
    {
        return true;
    }
    /*if (x >= DiscordButtonPosition.x && x <= DiscordButtonPosition.x + DiscordButtonSurface.Width && y >= DiscordButtonPosition.y && y <= DiscordButtonPosition.y + DiscordButtonSurface.Height && Engine->IsReady())
    {
        return true;
    }*/
    /*if (x >= ForumButtonPosition.x && x <= ForumButtonPosition.x + ForumButtonSurface.Width && y >= ForumButtonPosition.y && y <= ForumButtonPosition.y + ForumButtonSurface.Height && Engine->IsReady())
    {
        return true;
    }*/
    /*if (x >= FacebookButtonPosition.x && x <= FacebookButtonPosition.x + FacebookButtonSurface.Width && y >= FacebookButtonPosition.y && y <= FacebookButtonPosition.y + FacebookButtonSurface.Height && Engine->IsReady())
    {
        return true;
    }*/
   
    return false;
}

void HandleMouse(ButtonState state, int x, int y)
{
    if (x >= StartButtonPosition.x && x <= StartButtonPosition.x + StartButtonSurface.Width && y >= StartButtonPosition.y && y <= StartButtonPosition.y + StartButtonSurface.Height && Engine->IsReady())
    {
        if (lastStartState != STATE_DOWN) states[0] = state;
        if (state == STATE_UP) StartClick();
        else if (lastStartState != STATE_DOWN) lastStartState = state;
    }
    else {
        states[0] = STATE_NORMAL;
        if (state == STATE_UP) lastStartState = STATE_NORMAL;
    }
    /*if (x >= HomePageButtonPosition.x && x <= HomePageButtonPosition.x + HomePageButtonSurface.Width && y >= HomePageButtonPosition.y && y <= HomePageButtonPosition.y + HomePageButtonSurface.Height && Engine->IsReady())
    {
        if (lastHomepageState != STATE_DOWN) states[1] = state;
        if (state == STATE_UP) HomepageClick();
        else if (lastHomepageState != STATE_DOWN) lastHomepageState = state;
    }
    else {
        states[1] = STATE_NORMAL;
        if (state == STATE_UP) lastHomepageState = STATE_NORMAL;
    }*/
    if (x >= SettingsButtonPosition.x && x <= SettingsButtonPosition.x + SettingsButtonSurface.Width && y >= SettingsButtonPosition.y && y <= SettingsButtonPosition.y + SettingsButtonSurface.Height && Engine->IsReady())
    {
        if (lastSettingsState != STATE_DOWN) states[2] = state;
        if (state == STATE_UP) SettingsClick();
        else if (lastSettingsState != STATE_DOWN) lastSettingsState = state;
    }
    else {
        states[2] = STATE_NORMAL;
        if (state == STATE_UP) lastSettingsState = STATE_NORMAL;
    }
    if (x >= CloseButtonPosition.x && x <= CloseButtonPosition.x + CloseButtonSurface.Width && y >= CloseButtonPosition.y && y <= CloseButtonPosition.y + CloseButtonSurface.Height)
    {
        if (lastCloseState != STATE_DOWN) states[3] = state;
        if (state == STATE_UP) CloseClick();
        else if (lastCloseState != STATE_DOWN) lastCloseState = state;
    }
    else {
        states[3] = STATE_NORMAL;
        if (state == STATE_UP) lastCloseState = STATE_NORMAL;
    }
    /*if (x >= DiscordButtonPosition.x && x <= DiscordButtonPosition.x + DiscordButtonSurface.Width && y >= DiscordButtonPosition.y && y <= DiscordButtonPosition.y + DiscordButtonSurface.Height && Engine->IsReady())
    {
        if (lastDiscordState != STATE_DOWN) states[4] = state;
        if (state == STATE_UP) DiscordClick();
        else if (lastDiscordState != STATE_DOWN) lastDiscordState = state;
    }
    else {
        states[4] = STATE_NORMAL;
        if (state == STATE_UP) lastDiscordState = STATE_NORMAL;
    }*/
    /*if (x >= ForumButtonPosition.x && x <= ForumButtonPosition.x + ForumButtonSurface.Width && y >= ForumButtonPosition.y && y <= ForumButtonPosition.y + ForumButtonSurface.Height && Engine->IsReady())
    {
        if (lastForumState != STATE_DOWN) states[5] = state;
        if (state == STATE_UP) ForumClick();
        else if (lastForumState != STATE_DOWN) lastForumState = state;
    }
    else {
        states[5] = STATE_NORMAL;
        if (state == STATE_UP) lastForumState = STATE_NORMAL;
    }*/ 
    /*if (x >= FacebookButtonPosition.x && x <= FacebookButtonPosition.x + FacebookButtonSurface.Width && y >= FacebookButtonPosition.y && y <= FacebookButtonPosition.y + FacebookButtonSurface.Height && Engine->IsReady())
    {
        if (lastFacebookState != STATE_DOWN) states[6] = state;
        if (state == STATE_UP) FacebookClick();
        else if (lastFacebookState != STATE_DOWN) lastFacebookState = state;
    }
    else {
        states[6] = STATE_NORMAL;
        if (state == STATE_UP) lastFacebookState = STATE_NORMAL;
    }*/
}

static int xClick;
static int yClick;

static bool leftMouse = false;
static bool dragWindow = false;

DWORD g_lastMouseX = 0, g_lastMouseY = 0;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SOCKETMSG: {
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_CONNECT: {
            //TRACE("Socket connected..\n");
        } break;
        case FD_CLOSE: {
            Engine->ready = false;
            Engine->SetState("Disconnected.");
            Engine->SetPercent(0);
            Engine->mSocket->Release();
        }  break;
        case FD_READ: {
            Engine->mSocket->Receive();
        } break;
        default: {
            __ASSERT(0, "WM_SOCKETMSG: unknown socket flag.");
        } break;
        }
    } break;
    case WM_LBUTTONDOWN:
        dragWindow = true;
        leftMouse = true;
        RECT prc;
        GetWindowRect(hWnd, &prc);
        g_lastMouseX = LOWORD(lParam);
        g_lastMouseY = HIWORD(lParam);
        SetCapture(hWnd);
        xClick = LOWORD(lParam);
        yClick = HIWORD(lParam);
        HandleMouse(STATE_DOWN, xClick, yClick);
        break;
    case WM_LBUTTONUP:
        leftMouse = false;
        HandleMouse(STATE_UP, LOWORD(lParam), HIWORD(lParam));
        ReleaseCapture();
        dragWindow = false;
        {
            int windowWidth, windowHeight;
            RECT mainWindowRect, desktop;
            GetWindowRect(hWnd, &mainWindowRect);
            const HWND hDesktop = GetDesktopWindow();
            GetWindowRect(hDesktop, &desktop);
            windowHeight = mainWindowRect.bottom - mainWindowRect.top;
            windowWidth = mainWindowRect.right - mainWindowRect.left;
            POINT realPos;
            realPos.x = mainWindowRect.left;
            realPos.y = mainWindowRect.top;
            if (mainWindowRect.right > desktop.right)
                realPos.x = desktop.right - windowWidth;
            else if (mainWindowRect.left < desktop.left)
                realPos.x = desktop.left;

            if (mainWindowRect.bottom > desktop.bottom)
                realPos.y = desktop.bottom - windowHeight;
            else if (mainWindowRect.top < desktop.top)
                realPos.y = desktop.top;
            MoveWindow(hWnd, realPos.x, realPos.y, windowWidth, windowHeight, TRUE);
        }
        break;
    case WM_MOUSEMOVE:
    {
        HandleMouse(STATE_HOVER, LOWORD(lParam), HIWORD(lParam));
        if (GetCapture() == hWnd && dragWindow && leftMouse && !isInArea(LOWORD(lParam), HIWORD(lParam)) && lastCloseState != STATE_DOWN && /*lastDiscordState  != STATE_DOWN &&*/ lastSettingsState != STATE_DOWN && /*lastHomepageState != STATE_DOWN &&*/ lastStartState != STATE_DOWN /*&& lastForumState != STATE_DOWN &&*/ /*lastFacebookState != STATE_DOWN*/)
        {
            RECT mainWindowRect;
            POINT pos;
            int windowWidth, windowHeight;
            pos.x = (int)(short)LOWORD(lParam);
            pos.y = (int)(short)HIWORD(lParam);
            GetWindowRect(hWnd, &mainWindowRect);

            windowHeight = mainWindowRect.bottom - mainWindowRect.top;
            windowWidth = mainWindowRect.right - mainWindowRect.left;

            ClientToScreen(hWnd, &pos);

            MoveWindow(hWnd, pos.x - g_lastMouseX, pos.y - g_lastMouseY, windowWidth, windowHeight, TRUE);
        }
        break;
    }
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SETCURSOR:
        SetCursor(hCursorHand);
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0; 
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
