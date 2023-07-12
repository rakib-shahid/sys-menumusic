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
    nsInitialize();
    pminfoInitialize();

    // Enable this if you want to use HID.
    rc = hidInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

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
    Mix_Init(MIX_INIT_MP3);
    
    // open 44.1KHz, signed 16bit, system byte order,
    //  stereo audio, using 4096 byte chunks
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 8192);

    // Load sound file to use
    // Sound from https://freesound.org/people/jens.enk/sounds/434610/

    
    

    // Close the service manager session.
    smExit();
}

// Service deinitialization.
void __appExit(void)
{
    // Free the loaded sound
    

    // Shuts down SDL subsystems
    SDL_Quit();
    // Close extra services you added to __appInit here.
    fsdevUnmountAll(); // Disable this if you don't want to use the SD card filesystem.
    fsExit(); // Disable this if you don't want to use the filesystem.
    //timeExit(); // Enable this if you want to use time.
    hidExit(); // Enable this if you want to use HID.
    // socketExit();
    pminfoExit();
    nsExit();
    pmdmntExit();
    // setExit();
}

#ifdef __cplusplus
}
#endif

// void musicThread(void* _)
// {
    
//     // while (true)
//     // {
//     //     svcSleepThread(-1);
//     // }
// }

u64 mainLoopSleepTime = 50;
// static Thread music_thread;
// Main program entrypoint

int main(int argc, char* argv[])
{
    // Initialization code can go here.
    // FILE * Soundfile = fopen("sdmc:/music/home.mp3","r");
    u64 processId;
    u64 programId;
    u64 compare = 0x0100000000001000;
    Mix_Music *audio = Mix_LoadMUS("sdmc:/music/home.mp3");
    Mix_PlayMusic(audio, -1);
    int x = 0;
    while (appletMainLoop()){
        if (R_SUCCEEDED(pmdmntGetApplicationProcessId(&processId))){
            if (R_SUCCEEDED(pminfoGetProgramId(&programId, processId))){
                if (programId >= compare){
                    if (x == 0){
                        Mix_PauseMusic();
                    }
                    // earlier logic, unlikely to work anyways since am unable to get home title id
                }
            }
        }
        // get application process id failed meaning no app running
        else{
            if (x == 1){
                Mix_ResumeMusic();
            }
        }
        x = Mix_PausedMusic();
        svcSleepThread(mainLoopSleepTime * 1e+6L);
    }
    Mix_HaltMusic();
    Mix_FreeMusic(audio);
    // Your code / main loop goes here.
    // If you need threads, you can use threadCreate etc.

    // Deinitialization and resources clean up code can go here.
    return 0;
}

