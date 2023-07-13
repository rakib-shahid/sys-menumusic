// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <inttypes.h>

#include <SDL.h>
#include <SDL_mixer.h>

// #include "include/mp3.c"

// Include the main libnx system header, for Switch development
#include <switch.h>

// Size of the inner heap (adjust as necessary).
#define INNER_HEAP_SIZE 0x80000

#ifdef __cplusplus
extern "C" {
#endif

// Sysmodules should not use applet*.
u32 __nx_applet_type = AppletType_None;

// Sysmodules will normally only want to use one FS session.
u32 __nx_fs_num_sessions = 1;

Mix_Music *audio;


// Newlib heap configuration function (makes malloc/free work).
void __libnx_initheap(void)
{
    static u8 inner_heap[INNER_HEAP_SIZE];
    extern void* fake_heap_start;
    extern void* fake_heap_end;

    // Configure the newlib heap.
    fake_heap_start = inner_heap;
    fake_heap_end   = inner_heap + sizeof(inner_heap);
}

// Service initialization.
void __appInit(void)
{
    Result rc;

    // Open a service manager session.
    rc = smInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    // Retrieve the current version of Horizon OS.
    rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }
    // setInitialize();
    pmdmntInitialize();
    // nsInitialize();
    pminfoInitialize();

    // Enable this if you want to use HID.
    // rc = hidInitialize();
    // if (R_FAILED(rc))
    //     diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

    // Enable this if you want to use time.
    /*rc = timeInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_Time));

    __libnx_init_time();*/

    // Disable this if you don't want to use the filesystem.
    rc = fsInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    // Disable this if you don't want to use the SD card filesystem.
    fsdevMountSdmc();

    // Add other services you want to use here.

    // Start SDL with audio support
    SDL_Init(SDL_INIT_AUDIO);

    // Load support for the MP3 format
    Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG);
    
    // open 44.1KHz, signed 16bit, system byte order,
    //  stereo audio, using 4096 byte chunks
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);

    // Close the service manager session.
    smExit();
}

// Service deinitialization.
void __appExit(void)
{
    Mix_CloseAudio();
    // Shuts down SDL subsystems
    SDL_Quit();
    // Close extra services you added to __appInit here.
    fsdevUnmountAll(); // Disable this if you don't want to use the SD card filesystem.
    fsExit(); // Disable this if you don't want to use the filesystem.
    //timeExit(); // Enable this if you want to use time.
    // hidExit(); // Enable this if you want to use HID.
    // socketExit();
    pminfoExit();
    // nsExit();
    pmdmntExit();
    // setExit();
}

#ifdef __cplusplus
}
#endif


void musicThread(void* _)
{
    audio = Mix_LoadMUS("sdmc:/music/home.mp3");
    Mix_FadeInMusic(audio,-1,1000);
    u64 processId = 0;
    u64 lastProcessId = 0;
    u64 programId = 0;
    u64 compare = 0x0100000000001000;
    while (true)
    {
        if (R_SUCCEEDED(pmdmntGetApplicationProcessId(&processId))){
            if (lastProcessId != processId){
                if (R_SUCCEEDED(pminfoGetProgramId(&programId, processId))){
                    if (programId >= compare){
                        if (Mix_PlayingMusic() == 1){
                            Mix_FadeOutMusic(1000);
                            Mix_FreeMusic(audio);
                        }
                    }
                }
                lastProcessId = processId;
            }
            
        }
        else {
            lastProcessId = 0;
            if (Mix_PlayingMusic() == 0){
                audio = Mix_LoadMUS("sdmc:/music/home.mp3");
                Mix_FadeInMusic(audio,-1,1000);
            }
        }

        svcSleepThread(1e+9l);
    }
}

u64 mainLoopSleepTime = 50;
static Thread music_thread;
// Main program entrypoint

int main(int argc, char* argv[])
{
    // Initialization code can go here.
    
    
    Result rc = threadCreate(&music_thread, musicThread, NULL, NULL, 0x8000, 0x2C, 3);
    if (R_SUCCEEDED(rc)){
        threadStart(&music_thread);
    }
    while (appletMainLoop()){
        
        svcSleepThread(mainLoopSleepTime * 1e+6L);
    }
    // Your code / main loop goes here.
    // If you need threads, you can use threadCreate etc.

    // Deinitialization and resources clean up code can go here.
    threadClose(&music_thread);
    return 0;
}

