#include "general.h"
#include <math.h>

#include "handmade.cpp"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>

struct win32_offscreen_buffer{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

global_variable bool32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
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
        HMODULE XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    if (!XInputLibrary) {
        HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if(XInputLibrary) {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuiDevice, LPDIRECTSOUND *ppDS, _Pre_null_ LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

void *PlatformLoadFile(char *Filename) {
    return 0;
}

internal void Win32InitSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize) {
    // Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary) {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary,"DirectSoundCreate");
       
        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = WaveFormat.nChannels*WaveFormat.wBitsPerSample / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if (SUCCEEDED(Error)) {
                        OutputDebugStringA("Primary buffer format was set. \n");
                    } else {

                    }
                } else {

                }
            } else {

            }
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if (SUCCEEDED(Error)) {
                OutputDebugStringA("Secondary buffer format was set. \n");
            }
            BufferDescription.dwBufferBytes = BufferSize;
        } else {

        }
        
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
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
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
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
}

struct win32_sound_output {
    int SamplesPerSecond;
    int ToneHz;
    int16 ToneVolume;
    uint32 RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample; 
    int SecondaryBufferSize;
    real32 tSine;
    int LatencySampleCount;
};

void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD BytesToLock, DWORD BytesToWrite) {
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    HRESULT Error = GlobalSecondaryBuffer->Lock(BytesToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0);
    if (SUCCEEDED(Error))
    {
        // TODO: Asserts
        DWORD Regions1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16 *SampleOut = (int16 *)Region1;
        for (DWORD SampleIndex = 0; SampleIndex < Regions1SampleCount; ++SampleIndex)
        {
            real32 SineValue = sin(SoundOutput->tSine);
            int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;
            
            SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (real32)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Regions2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        SampleOut = (int16 *)Region2;
        for (DWORD SampleIndex = 0; SampleIndex < Regions2SampleCount; ++SampleIndex)
        {
            real32 SineValue = sin(SoundOutput->tSine);
            int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;
            
            SoundOutput->tSine += 2.0f * Pi32 * 1.0f / (real32)SoundOutput->WavePeriod;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR     CommandLine,
    int      ShowCode 
) {
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    Win32LoadXInput();
    WNDCLASSA WindowClassA = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1200, 720);
    WindowClassA.style = CS_HREDRAW | CS_VREDRAW;
    WindowClassA.lpfnWndProc = Win32MainWindowCallback;
    WindowClassA.hInstance = Instance;
    // WindowClassA.hIcon
    WindowClassA.lpszClassName = "HandmadeHeroWindowClass";

    if(RegisterClassA(&WindowClassA)) {
        HWND Window = CreateWindowExA(
            0,
            WindowClassA.lpszClassName, 
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
            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.ToneHz = 440;
            SoundOutput.ToneVolume = 1500;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 20;

            Win32InitSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;

            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);

            uint64 LastCycleCount = __rdtsc();

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

                        bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool32 LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool32 RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool32 AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool32 BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool32 XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool32 YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 StickX = Pad->sThumbLX;
                        int16 StickY = Pad->sThumbLY;

                        if (Up) {
                            YOffset--;
                        }
                        if (Down) {
                            YOffset++;
                        }
                        if (Left) {
                            XOffset--;
                        }
                        if (Right) {
                            XOffset++;
                        }

                        XOffset += StickX / 4096;
                        YOffset -= StickY / 4096;

                        SoundOutput.ToneHz = 512 + (int)(256.0f*((real32)StickY / 30000.0f));
                        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
                        
                    } else {
                        //The controller is not available
                    }
                }

                // XINPUT_VIBRATION Vibration;
                // Vibration.wLeftMotorSpeed = 0;
                // Vibration.wRightMotorSpeed = 0;
                // XInputSetState(0, &Vibration);
                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackBuffer.Memory;
                Buffer.Width = GlobalBackBuffer.Width;
                Buffer.Height = GlobalBackBuffer.Height;
                Buffer.Pitch = GlobalBackBuffer.Pitch;
                GameUpdateAndRender(&Buffer, XOffset, YOffset);
                
                DWORD PlayCursor;
                DWORD WriteCursor;
                if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor,&WriteCursor))) {
                    DWORD BytesToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
                    DWORD TargetCursor = ( PlayCursor + SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample ) % SoundOutput.SecondaryBufferSize;
                    DWORD BytesToWrite;

                    if (BytesToLock > TargetCursor) {
                        BytesToWrite = (SoundOutput.SecondaryBufferSize - BytesToLock);
                        BytesToWrite += TargetCursor;
                    }
                    else {
                        BytesToWrite = TargetCursor - BytesToLock;
                    }

                    Win32FillSoundBuffer(&SoundOutput, BytesToLock, BytesToWrite);
                }

                HDC DeviceContext = GetDC(Window);
                win32_window_dimension WindowDimension = GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, WindowDimension.Width, WindowDimension.Height);
                ReleaseDC(Window, DeviceContext);

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);

                uint64 EndCycleCount = __rdtsc();

                uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                int64 CounterElapse = EndCounter.QuadPart - LastCounter.QuadPart;
                real32  MSPerFrame = ((1000.0f * (real32)CounterElapse) / (real32)PerfCountFrequency);
                real32 FPS = (real32)PerfCountFrequency / (real32)CounterElapse;
                real32 MCPF = (int32)CyclesElapsed / (1000 * 1000);

                char StatsBuffer[256];
                sprintf(StatsBuffer, "ms/f: %.2f,  fps: %.2f,  mc/f: %.2f\n", MSPerFrame, FPS, MCPF);
                OutputDebugStringA(StatsBuffer);

                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }
        } else {

        }
    } else {

    }

    return (0);
}