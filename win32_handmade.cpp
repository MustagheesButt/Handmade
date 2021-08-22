#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>

#define internal static
#define global static
#define local_persist static
#define PI32 3.14159265359

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#include "win32_handmade.h"
#include "handmade.cpp"

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

global bool32 Running;
global win32_offscreen_buffer BackBuffer;
global LPDIRECTSOUNDBUFFER SecondaryBuffer;



internal void Win32LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary)
    {
        // TODO: Add diagnostics
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }

    if (!XInputLibrary)
    {
        // TODO: Add diagnostics
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else
    {
        // TODO: Add diagnostics
    }
}

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if (DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if ((SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))))
            {
                // Create primary buffer
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Result = PrimaryBuffer->SetFormat(&WaveFormat);
                    if (SUCCEEDED(Result))
                    {
                        OutputDebugStringA("Primary buffer created\n");
                    }
                    else
                    {
                        // TODO: Diagnostics
                    }
                }
                else
                {
                    // TODO: Diagnostics
                }
            }
            else
            {
                // TODO: Diagnostics
            }

            // Create secondary buffer
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0);
            if (SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created\n");
            }
            else
            {
                // TODO: Diagnostics
            }
        }
        else
        {
            // TODO: Diagnostics
        }
    }
    else
    {
        // TODO: Diagnostics
    }
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension x;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    x.Width = ClientRect.right - ClientRect.left;
    x.Height = ClientRect.bottom - ClientRect.top;

    return x;
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
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

    int MemorySize = Width * Height * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, MemorySize, MEM_COMMIT, PAGE_READWRITE);
    
    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer *Buffer, int X, int Y, int Width, int Height)
{
    // TODO: Aspect ratio correction
    StretchDIBits(
        DeviceContext,
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer->Width, Buffer->Height,
        Buffer->Memory,
        &Buffer->Info,
        DIB_RGB_COLORS, SRCCOPY
    );
}

internal LRESULT CALLBACK Win32MainWindowCallback(
    HWND   Window,
    UINT   Message,
    WPARAM WParam,
    LPARAM LParam
)
{
    LRESULT Result = 0;

    switch(Message)
    {
        case WM_DESTROY:
        {
            Running = false;
        } break;

        case WM_CLOSE:
        {
            Running = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("ACTIVE\n");
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = WParam;
            bool32 WasDown = ((LParam & (1 << 30)) != 0);
            bool32 IsDown = ((LParam & (1 << 31)) == 0);

            if (VKCode == 'W')
            {

            }
            else if (VKCode == 'A')
            {

            }
            else if (VKCode == 'S')
            {
                
            }
            else if (VKCode == 'D')
            {

            }
            else if (VKCode == VK_ESCAPE)
            {

            }
            else if (VKCode == VK_SPACE)
            {

            }

            bool32 AltKeyWasDown = (LParam & (1 << 29));
            if (VKCode == VK_F4 && AltKeyWasDown)
            {
                Running = false;
            }
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension Dimension = Win32GetWindowDimension(Window);

            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &BackBuffer, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

internal void Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if (SUCCEEDED(SecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
        uint8 *DestSample = (uint8 *)Region1;
        for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }

        DestSample = (uint8 *)Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }

        SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, sound_output_buffer *SourceBuffer)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if (SUCCEEDED(SecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        DestSample = (int16 *)Region2;
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void Win32ProcessXInput(DWORD XInputButtonState, game_button_state *OldState, DWORD ButtonBit, game_button_state *NewState)
{
    NewState->EndedDown = (XInputButtonState & ButtonBit) == ButtonBit;
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    LARGE_INTEGER QueryPerfFreq;
    QueryPerformanceFrequency(&QueryPerfFreq);
    int64 PerfCountFreq = QueryPerfFreq.QuadPart;

    Win32LoadXInput();

    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&BackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = hInstance;
    // WindowClass.hIcon = ;
    WindowClass.lpszClassName = "HandmadeClassName";

    if (RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Cool Window",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            hInstance,
            0
        );

        if (Window)
        {
            // Since we specified CS_OWNDC, we can just get one device context and use it forever
            // because we are not sharing it with anyone
            HDC DeviceContext = GetDC(Window);

            int XOffset = 0;
            int YOffset = 0;

            win32_sound_output SoundOutput = {};

            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(int16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond/15;

            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
            SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            // TODO: Pool with bitmap VirtualAllocs
            int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            Running = true;

            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);

            uint64 LastCycleCount = __rdtsc();
            while (Running)
            {
                MSG Message;

                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];

                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                int MaxControllers = XUSER_MAX_COUNT;
                if (MaxControllers > ArrayCount(NewInput->Controllers))
                {
                    MaxControllers = ArrayCount(NewInput->Controllers);
                }

                for (DWORD ControllerIndex = 0; ControllerIndex < MaxControllers; ControllerIndex++)
                {
                    game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
                    game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];

                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        // Controller is plugged in
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

                        NewController->IsAnalog = true;
                        NewController->StartX = OldController->EndX;
                        NewController->StartY = OldController->EndY;

                        real32 X;
                        if (Pad->sThumbLX < 0)
                        {
                            X = (real32)Pad->sThumbLX / 32768.0f;
                        }
                        else
                        {
                            X = (real32)Pad->sThumbLX / 32767.0f;
                        }
                        NewController->MinX = NewController->MaxX = NewController->EndX = X;

                        real32 Y;
                        if (Pad->sThumbLY < 0)
                        {
                            Y = (real32)Pad->sThumbLY / 32768.0f;
                        }
                        else
                        {
                            Y = (real32)Pad->sThumbLY / 32767.0f;
                        }
                        NewController->MinY = NewController->MaxY = NewController->EndY = Y;

                        Win32ProcessXInput(Pad->wButtons, &OldController->Down,
                                           XINPUT_GAMEPAD_A, &NewController->Down);
                        Win32ProcessXInput(Pad->wButtons, &OldController->Right,
                                           XINPUT_GAMEPAD_A, &NewController->Right);
                        Win32ProcessXInput(Pad->wButtons, &OldController->Left,
                                           XINPUT_GAMEPAD_A, &NewController->Left);
                        Win32ProcessXInput(Pad->wButtons, &OldController->Up,
                                           XINPUT_GAMEPAD_A, &NewController->Up);
                        Win32ProcessXInput(Pad->wButtons, &OldController->LeftShoulder,
                                           XINPUT_GAMEPAD_A, &NewController->LeftShoulder);
                        Win32ProcessXInput(Pad->wButtons, &OldController->RightShoulder,
                                           XINPUT_GAMEPAD_A, &NewController->RightShoulder);

                        // bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        // bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                    }
                    else
                    {
                        // Controller is not available
                    }
                }

                DWORD ByteToLock;
                DWORD TargetCursor;
                DWORD BytesToWrite;
                DWORD PlayCursor;
                DWORD WriteCursor;
                bool32 SoundIsValid = false;
                if (SUCCEEDED(SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
                    TargetCursor = (PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize;
                    if (ByteToLock > TargetCursor)
                    {
                        BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
                        BytesToWrite += TargetCursor;
                    }
                    else
                    {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }

                    SoundIsValid = true;
                }

                game_offscreen_buffer Buffer = {};
                Buffer.Memory = BackBuffer.Memory;
                Buffer.Width = BackBuffer.Width;
                Buffer.Height = BackBuffer.Height;
                Buffer.Pitch = BackBuffer.Pitch;

                sound_output_buffer SoundBuffer = {};
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;
                GameUpdateAndRender(NewInput, &Buffer, &SoundBuffer);

                if (SoundIsValid)
                    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &BackBuffer, 0, 0, Dimension.Width, Dimension.Height);

                uint64 EndCycleCount = __rdtsc();

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);

                uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                real32 MSPerFrame = (((1000.0f * (real32)CounterElapsed) / (real32)PerfCountFreq));
                real32 FPS = (real32)PerfCountFreq / (real32)CounterElapsed;
                real32 MCPF = (real32)(CyclesElapsed / (1000.0f * 1000.0f));

#if 0
                char Buffer[256];
                sprintf(Buffer, "time/frame: %fms, %f FPS, %f Mc/f\n", MSPerFrame, FPS, MCPF);
                OutputDebugStringA(Buffer);
#endif

                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;

                game_input *Temp = OldInput;
                OldInput = NewInput;
                NewInput = Temp;
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }

    return 0;
}