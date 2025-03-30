#include <windows.h>
#include <stdint.h>

#define internal static 
#define local_persist static 
#define global_variable static 
//TODO(diego): this is a global for now

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;


internal void RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset) {
    uint8 *Row = (uint8 *)Buffer.Memory;

    for(int Y=0; Y < Buffer.Height; ++Y){
        uint32 *Pixel = (uint32 *)Row;

        for(int X = 0; X < Buffer.Width; ++X) {
            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);
            
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Buffer.Pitch;
    }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height) {

    if(Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Buffer->Width*Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, RECT ClientRect, win32_offscreen_buffer Buffer,
                                         int X, int Y, int Width, int Height){
    int WindowWidth = ClientRect.right - ClientRect.left;
    int WindowHeight = ClientRect.bottom - ClientRect.top;
    StretchDIBits(DeviceContext, 
                    0, 0, Buffer.Width, Buffer.Height,
                    0, 0, WindowWidth, WindowHeight,
                    Buffer.Memory, 
                    &Buffer.Info, 
                    DIB_RGB_COLORS, SRCCOPY);
};

LRESULT CALLBACK Win32MainWindowCallback(
  HWND Window,
  UINT Message,
  WPARAM WParam,
  LPARAM LParam 
)
{
    LRESULT Result = 0;

    switch(Message) {
        case WM_SIZE:{
            RECT ClientRect;
            HDC DeviceContext = GetDC(Window);
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(&GlobalBackBuffer, Width, Height);
            Win32DisplayBufferInWindow(DeviceContext, ClientRect, GlobalBackBuffer, 0, 0, Width, Height);
        } break;
        case WM_DESTROY: {
            //TODO(diego): Handle this with a message to the user
            Running = false;
        } break;
        case WM_CLOSE: {
            //TODO(diego): Handle this with a message to the user
            Running = false;
        } break;
        case WM_ACTIVATEAPP: {
            OutputDebugStringA("ACTIVE\n");
        } break;
        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);

            Win32DisplayBufferInWindow(DeviceContext, ClientRect, GlobalBackBuffer, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;
        default:{
            // OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
}

int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR     CommandLine,
    int      ShowCode 
) {
    WNDCLASSA WindowClass = {};

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    if(RegisterClassA(&WindowClass)) {
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName, 
            "HandMade Hero",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0 
        );
        if(Window) {
            Running = true;

            int XOffset = 0;
            int YOffset = 0;

            while(Running) {
                MSG Message;

                while(PeekMessage(&Message,0,0,0, PM_REMOVE)) {
                    if(Message.message == WM_QUIT) {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
                
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect; 
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32DisplayBufferInWindow(DeviceContext, ClientRect, GlobalBackBuffer, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);

                ++XOffset;
                YOffset += 2;
            }
        } else {

        }
    } else {

    }


    return (0);
}
