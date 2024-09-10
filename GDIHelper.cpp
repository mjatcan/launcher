
#include "stdafx.h"
//#include "Engine.h"
#include "GDIHelper.h"

extern GDIHelper gdiHelper;
extern HWND mainWindow;
HWND GDIHelper::staticControl = NULL;
HWND GDIHelper::MainHWNDControl = NULL;
Image* GDIHelper::m_pImage = NULL;
GUID* GDIHelper::m_pDimensionIDs = NULL;
UINT GDIHelper::m_FrameCount = 0;
PropertyItem* GDIHelper::m_pItem = NULL;
UINT GDIHelper::m_iCurrentFrame = 0;
UINT_PTR GDIHelper::unique_id = 0;
BOOL GDIHelper::m_bIsPlaying = FALSE;
BOOL GDIHelper::isPlayable = FALSE;
BOOL GDIHelper::isLooped = FALSE;
int GDIHelper::xPosition = 0;
int GDIHelper::yPosition = 0;
int GDIHelper::width = 0;
int GDIHelper::height = 0;
int GDIHelper::animation_duration = 0;

/** GDIHelper is a class helper to display images and animated GIF **/
GDIHelper::GDIHelper() {}

/** Function to destroy objects and arrays, call this function on WM_DESTROY of WinProc. **/
void GDIHelper::Destroy() {
    if(m_pDimensionIDs) {
        delete[] m_pDimensionIDs;
    }

    if(m_pItem) {
        free(m_pItem);
    }

    if(m_pImage) {
        delete m_pImage;
    }
    m_bIsPlaying = FALSE;
    isPlayable = FALSE;
    RemoveWindowSubclass(staticControl, &StaticControlProc, unique_id);
}

extern HWND hLoadHwnd;

/** Private function, call this function as thread to animate the GIF image. **/
void GDIHelper::run()
{
    if(m_bIsPlaying == TRUE) {
        return;
    }
    m_iCurrentFrame = 0;
    GUID Guid = FrameDimensionTime;
    m_pImage->SelectActiveFrame(&Guid, m_iCurrentFrame);
    ++m_iCurrentFrame;
    m_bIsPlaying = TRUE;
    animation_duration = ((UINT*)m_pItem[0].value)[m_iCurrentFrame] * 10;
    
    while(isPlayable) {
        std::this_thread::sleep_for(std::chrono::milliseconds(animation_duration));
        m_pImage->SelectActiveFrame(&Guid, m_iCurrentFrame);

        m_iCurrentFrame = (++m_iCurrentFrame) % m_FrameCount;
        InvalidateRect(staticControl, NULL, FALSE);
        UpdateWindow(staticControl);
       
        if (!isLooped && m_iCurrentFrame == 0)
        {
            ShowWindow(hLoadHwnd, SW_HIDE);
            ShowWindow(staticControl, SW_HIDE);
            gdiHelper.Destroy();

            std::string param = std::to_string(GetCurrentProcessId());
            if ((long)ShellExecuteA(NULL, NULL, xorstr("KnightOnLine.exe"), param.c_str(), NULL, SW_RESTORE) == ERROR_FILE_NOT_FOUND)
                MessageBoxA(mainWindow, xorstr("KnightOnLine.exe bulunamadi."), xorstr("Launcher"), MB_ICONINFORMATION);
               
            ::PostQuitMessage(0);
            TerminateProcess(GetCurrentProcess(), 0);
            ExitProcess(0);
            break;
        }
    }
}

/** Private function, accessible only in this class, check if file exist. **/
bool GDIHelper::IsFileExist(string file_name) {
    struct stat buffer;
    return (stat(file_name.c_str(), &buffer) == 0);
}

/** Private function, function to count and get the frame of image. **/
void GDIHelper::GetImageFrame() {
    UINT count = m_pImage->GetFrameDimensionsCount();
    m_pDimensionIDs = new GUID[count];
    m_pImage->GetFrameDimensionsList(m_pDimensionIDs, count);

    m_FrameCount = m_pImage->GetFrameCount(&m_pDimensionIDs[0]);

    UINT TotalBuffer = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay);
    m_pItem = (PropertyItem*)malloc(TotalBuffer);
    m_pImage->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, m_pItem);

    if(m_FrameCount > 1) {  // frame of GIF is more than one, all good, we don't want the error of `Access violation reading location`
        OutputDebugString(_T("NOTICED: GDIHelper::InitializeImage >> Image file has more than 1 frame, its playable (2).\n"));
        isPlayable = TRUE;  // is playable
    
        std::thread t(run); // Start the animation as thread.
        t.detach();         // this will be non-blocking thread.
    }
}

LRESULT CALLBACK GDIHelper::StaticControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (uMsg)
    {
    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        Graphics g(hdc);
        g.DrawImage(m_pImage, 0, 0, width, height);

        EndPaint(hwnd, &ps);
        return TRUE;
    }
    break;
    case WM_LBUTTONDBLCLK:
    {
        int a = 0;
    }
    break;
    }
   return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

/** Function to set the image and initialize all required variables. **/
void GDIHelper::SetImage(int uunique_id, int xxPosition, int yyPosition, int wwidth, int hheight, Image* image, HWND hwnd) {
    unique_id = uunique_id;
    xPosition = xxPosition;
    yPosition = yyPosition;
    width = wwidth;
    height = hheight;

    staticControl = CreateWindowEx(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, xPosition, yPosition, width, height, hwnd, NULL, NULL, NULL); //create the static control.
    SetWindowSubclass(staticControl, &StaticControlProc, unique_id, 0);

    m_pImage = image;
    GetImageFrame(); //Initialize the image.
}

/** Function to Display Image from Local File. **/
void GDIHelper::DisplayImageFromFile(string file_name, HWND hWnd, UINT_PTR uunique_id, int xxPosition, int yyPosition, int wwidth, int hheight, bool looped) {
    if(!IsFileExist(file_name)) {
        OutputDebugString(_T("ERROR: GDIHelper::LoadImageFromFile >> Invalid file or not exist\n"));
        return; 
    }

    isLooped = looped;
    std::wstring widestr = std::wstring(file_name.begin(), file_name.end()); // Convert the string file_name to wstring.
    SetImage(uunique_id, xxPosition, yyPosition, wwidth, hheight, Image::FromFile(widestr.c_str()), hWnd); //Set image and Control
}


/** Function to Display Image from Local File. **/
void GDIHelper::thyke_display(string file_name, HWND hWndKO, HWND &hWnd, UINT_PTR uunique_id, int xxPosition, int yyPosition, int wwidth, int hheight, bool looped) {
    if(!IsFileExist(file_name)) {
        OutputDebugString(_T("ERROR: GDIHelper::LoadImageFromFile >> Invalid file or not exist\n"));
        return; 
    }

    isLooped = looped;
    std::wstring widestr = std::wstring(file_name.begin(), file_name.end()); // Convert the string file_name to wstring.
   
    unique_id = uunique_id;
    xPosition = xxPosition;
    yPosition = yyPosition;
    width = wwidth;
    height = hheight;

    staticControl = CreateWindowEx(0, "STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, xPosition, yPosition, width, height, hWndKO, NULL, NULL, NULL); //create the static control.
    hWnd = staticControl;
    SetWindowSubclass(staticControl, &StaticControlProc, unique_id, 0);

    m_pImage = Image::FromFile(widestr.c_str());
    GetImageFrame(); //Initialize the image.
}

/** Function to Load Image from Resources. **/
void GDIHelper::DisplayImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype, HWND hWnd, UINT_PTR uunique_id, int xxPosition, int yyPosition, int wwidth, int hheight) {

    IStream* pStream = nullptr;
    Gdiplus::Bitmap* pBmp = nullptr;
    HGLOBAL hGlobal = nullptr;

    HRSRC hrsrc = FindResourceW(GetModuleHandle(NULL), resid, restype);     // get the handle to the resource
    if(hrsrc) {
        DWORD dwResourceSize = SizeofResource(hMod, hrsrc);
        if(dwResourceSize > 0) {
            HGLOBAL hGlobalResource = LoadResource(hMod, hrsrc); // load it
            if(hGlobalResource) {
                void* imagebytes = LockResource(hGlobalResource); // get a pointer to the file bytes

                hGlobal = ::GlobalAlloc(GHND, dwResourceSize); // copy image bytes into a real hglobal memory handle
                if(hGlobal) {
                    void* pBuffer = ::GlobalLock(hGlobal);
                    if(pBuffer) {
                        memcpy(pBuffer, imagebytes, dwResourceSize);
                        HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
                        if(SUCCEEDED(hr)) {
                            hGlobal = nullptr; // pStream now owns the global handle and will invoke GlobalFree on release
                            pBmp = new Gdiplus::Bitmap(pStream);
                        }
                    }
                }
            }
        }
    }

    if(pStream) {
        pStream->Release();
        pStream = nullptr;
    }

    if(hGlobal) {
        GlobalFree(hGlobal);
    }

    SetImage(uunique_id, xxPosition, yyPosition, wwidth, hheight, pBmp, hWnd);
}
