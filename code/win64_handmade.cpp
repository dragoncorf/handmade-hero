#include <windows.h>

#define internal static 
#define local_persist static 
#define global_variable static 
//TODO(diego): this is a global for now
global_variable bool Running;

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height){
    StretchDIBits(DeviceContext,
        X, Y, Width, Height, 
        X, Y, Width, Height,
        BitmapMemory, 
        &BitmapInfo, 
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
            OutputDebugStringA("WM_SIZE\n");
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
            OutputDebugStringA("WM_SIZE\n");
        } break;
        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
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

    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    if(RegisterClassA(&WindowClass)) {
        HWND WindowHandle = CreateWindowExA(
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
        if(WindowHandle) {
            Running = true;
            while(Running) {
                MSG Message;
                BOOL MessageResult = GetMessage(&Message,0,0,0);
                if (MessageResult > 0) {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } else {
                    break;
                }
            }
        } else {

        }
    } else {

    }


    return (0);
}
