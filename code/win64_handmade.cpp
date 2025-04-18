#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define internal static 
#define local_persist static 
#define global_variable static 
//TODO(diego): this is a global for now

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

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

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable int XOffset = 0;
global_variable int YOffset = 0;

struct win32_window_dimension {
    int Width;
    int Height;
};


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void) {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary) {
        HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if(XInputLibrary) {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

internal win32_window_dimension GetWindowDimension(HWND Window) {
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return Result;
}

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset) {
    uint8 *Row = (uint8 *)Buffer->Memory;
    uint8 Red = 0; 

    for(int Y=0; Y < Buffer->Height; ++Y){
        uint32 *Pixel = (uint32 *)Row;

        for(int X = 0; X < Buffer->Width; ++X) {
            uint32 Blue = (X + Red + XOffset);
            uint32 Green = (Y + YOffset);
            Red = X + Y + XOffset + YOffset;

            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }

        Row += Buffer->Pitch;
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
    // When the biHeight field is negative makes windows treat it as top-down,
    // the first three bytes are in the top left corner instead of bottom-left.
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Buffer->Width*Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow( win32_offscreen_buffer *Buffer, HDC DeviceContext, 
                                            int WindowWidth, int WindowHeight) { 
    StretchDIBits(DeviceContext, 
                    0, 0, WindowWidth, WindowHeight,
                    0, 0, Buffer->Width, Buffer->Height,
                    Buffer->Memory, 
                    &Buffer->Info, 
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
        } break;
        case WM_DESTROY: {
            //TODO(diego): Handle this with a message to the user
            GlobalRunning = false;
        } break;
        case WM_CLOSE: {
            //TODO(diego): Handle this with a message to the user
            GlobalRunning = false;
        } break;
        case WM_ACTIVATEAPP: {
            OutputDebugStringA("ACTIVE\n");
        } break;

        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            uint32 VKCode = WParam;
            bool32 WasDown = LParam & (1 << 30);
            bool32 IsDown = LParam & (1 << 31);
            if (WasDown != IsDown) {
                if (VKCode == 'W') {
                    OutputDebugStringA("W");
                } else if (VKCode == 'A') {
                    OutputDebugStringA("A");
                } else if (VKCode == 'S') {
                    OutputDebugStringA("S");
                } else if (VKCode == 'D') {
                    OutputDebugStringA("D");
                } else if (VKCode == 'Q') {
                    OutputDebugStringA("Q");
                } else if (VKCode == 'E') {
                    OutputDebugStringA("E");
                } else if (VKCode == VK_UP) {
                } else if (VKCode == VK_DOWN) {
                } else if (VKCode == VK_LEFT) {
                } else if (VKCode == VK_RIGHT) {
                } else if (VKCode == VK_ESCAPE) {
                    if (IsDown) {
                        OutputDebugStringA("isDown");
                    }
                    if (WasDown) {
                        OutputDebugStringA("WasDown");
                    }
                    OutputDebugStringA("\n");
                } else if (VKCode == VK_SPACE) {
                }
            }
            bool32 AltKeyWasDown = (LParam & (1 << 29));
            if ((VKCode == VK_F4) && AltKeyWasDown) {
                GlobalRunning = false;
            }
        } break;

        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension WindowDimension = GetWindowDimension(Window);

            // RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
            Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, WindowDimension.Width, WindowDimension.Height);
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
    Win32LoadXInput();
    WNDCLASSA WndowClassA = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1200, 720);
    WndowClassA.style = CS_HREDRAW | CS_VREDRAW;
    WndowClassA.lpfnWndProc = Win32MainWindowCallback;
    WndowClassA.hInstance = Instance;
    // WndowClassA.hIcon
    WndowClassA.lpszClassName = "HandmadeHeroWindowClass";
    if(RegisterClassA(&WndowClassA)) {
        HWND Window = CreateWindowExA(
            0,
            WndowClassA.lpszClassName, 
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
            GlobalRunning = true;
            while(GlobalRunning) {
                MSG Message;

                while(PeekMessage(&Message,0,0,0, PM_REMOVE)) {
                    if(Message.message == WM_QUIT) {
                        GlobalRunning = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {
                    XINPUT_STATE ControllerState;
                    if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                        //Controller is plugged in
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                        if (Up) {
                            YOffset++;
                        }
                        if (Down) {
                            YOffset--;
                        }
                        if (Left) {
                            XOffset--;
                        }
                        if (Right) {
                            XOffset++;
                        }
                        if(AButton) {
                            XOffset += 2;
                        }
                    } else {
                        //The controller is not available
                    }
                }
                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 60000;
                Vibration.wRightMotorSpeed = 60000;
                XInputSetState(0, &Vibration);
                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);
                
                HDC DeviceContext = GetDC(Window);
                win32_window_dimension WindowDimension = GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, WindowDimension.Width, WindowDimension.Height);
                ReleaseDC(Window, DeviceContext);

                //XOffset++;
            }
        } else {

        }
    } else {

    }


    return (0);
}
