#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <SDL2/SDL.h>
#include "NES.hpp"

const int NES_WIDTH = 256;
const int NES_HEIGHT = 240;
int SCALE = 3;

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [options] <rom_file.nes>\n"
              << "Options:\n"
              << "  --scale <1-8>    Set integer display scale (default: 3)\n"
              << "Controls:\n"
              << "  A: Z | B: X | Select: Shift | Start: Enter\n"
              << "  D-Pad: Arrow keys (Up, Down, Left, Right)\n"
              << "  Pause: P | Reset: R | Scale: 1, 2, 3, 4\n";
}

int main(int argc, char* argv[]) {
    std::string romPath = "";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--scale" && i + 1 < argc) {
            SCALE = std::stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else {
            romPath = arg;
        }
    }

    if (romPath.empty()) {
        std::cerr << "Error: No ROM file specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << " NES Emulator - Loading ROM..." << std::endl;
    std::cout << " ROM Path: " << romPath << std::endl;
    std::cout << " Window Scale: " << SCALE << "x (" << (NES_WIDTH * SCALE) << "x" << (NES_HEIGHT * SCALE) << ")" << std::endl;
    std::cout << "=======================================" << std::endl;

    NES nes;
    if (!nes.loadCartridge(romPath)) {
        std::cerr << "Error: Failed to load ROM file: " << romPath << std::endl;
        return 1;
    }

    std::cout << "ROM successfully loaded! Initializing SDL2 video & audio..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "NES Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        NES_WIDTH * SCALE, NES_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        NES_WIDTH, NES_HEIGHT
    );

    // Audio setup using SDL_QueueAudio
    SDL_AudioSpec desiredSpec{};
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_F32SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 1024;
    desiredSpec.callback = nullptr;

    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, nullptr, 0);
    if (audioDevice > 0) {
        SDL_PauseAudioDevice(audioDevice, 0);
        std::cout << "Audio initialized (44.1 kHz, 32-bit float)." << std::endl;
    } else {
        std::cout << "Warning: Audio device initialization skipped/failed: " << SDL_GetError() << std::endl;
    }

    std::cout << "\nEmulator running. Controls:" << std::endl;
    std::cout << "  D-Pad: Arrow Keys | A: Z | B: X | Select: Shift | Start: Enter" << std::endl;
    std::cout << "  Pause/Resume: P | Reset: R | Window Scale: 1, 2, 3, 4" << std::endl;
    std::cout << "Press ESC or close the window to exit.\n" << std::endl;

    bool running = true;
    bool paused = false;
    SDL_Event event;

    std::vector<float> audioBuffer;
    audioBuffer.reserve(1024);

    uint32_t audioClockCounter = 0;
    const uint32_t cpuClocksPerAudioSample = 40; // ~1.78MHz / 44100 Hz

    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    uint32_t totalFrames = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: running = false; break;
                case SDLK_z: nes.getController(0).setButton(Controller::A, true); break;
                case SDLK_x: nes.getController(0).setButton(Controller::B, true); break;
                case SDLK_RSHIFT:
                case SDLK_LSHIFT: nes.getController(0).setButton(Controller::SELECT, true); break;
                case SDLK_RETURN: nes.getController(0).setButton(Controller::START, true); break;
                case SDLK_UP: nes.getController(0).setButton(Controller::UP, true); break;
                case SDLK_DOWN: nes.getController(0).setButton(Controller::DOWN, true); break;
                case SDLK_LEFT: nes.getController(0).setButton(Controller::LEFT, true); break;
                case SDLK_RIGHT: nes.getController(0).setButton(Controller::RIGHT, true); break;
                case SDLK_p:
                    paused = !paused;
                    std::cout << (paused ? "Emulator paused." : "Emulator resumed.") << std::endl;
                    break;
                case SDLK_r:
                    nes.reset();
                    std::cout << "Emulator reset." << std::endl;
                    break;
                case SDLK_1: SDL_SetWindowSize(window, NES_WIDTH * 1, NES_HEIGHT * 1); break;
                case SDLK_2: SDL_SetWindowSize(window, NES_WIDTH * 2, NES_HEIGHT * 2); break;
                case SDLK_3: SDL_SetWindowSize(window, NES_WIDTH * 3, NES_HEIGHT * 3); break;
                case SDLK_4: SDL_SetWindowSize(window, NES_WIDTH * 4, NES_HEIGHT * 4); break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_z: nes.getController(0).setButton(Controller::A, false); break;
                case SDLK_x: nes.getController(0).setButton(Controller::B, false); break;
                case SDLK_RSHIFT:
                case SDLK_LSHIFT: nes.getController(0).setButton(Controller::SELECT, false); break;
                case SDLK_RETURN: nes.getController(0).setButton(Controller::START, false); break;
                case SDLK_UP: nes.getController(0).setButton(Controller::UP, false); break;
                case SDLK_DOWN: nes.getController(0).setButton(Controller::DOWN, false); break;
                case SDLK_LEFT: nes.getController(0).setButton(Controller::LEFT, false); break;
                case SDLK_RIGHT: nes.getController(0).setButton(Controller::RIGHT, false); break;
                }
            }
        }

        if (!paused) {
            // Run system until frame complete
            while (!nes.getPPU().frameComplete) {
                nes.clock();

                audioClockCounter++;
                if (audioClockCounter >= cpuClocksPerAudioSample) {
                    audioClockCounter = 0;
                    audioBuffer.push_back(nes.getAPU().getOutputSample());
                }
            }
            nes.getPPU().frameComplete = false;
            totalFrames++;

            // Queue audio
            if (audioDevice > 0 && !audioBuffer.empty()) {
                if (SDL_GetQueuedAudioSize(audioDevice) < 4096 * sizeof(float)) {
                    SDL_QueueAudio(audioDevice, audioBuffer.data(), audioBuffer.size() * sizeof(float));
                }
                audioBuffer.clear();
            }

            // Render frame
            SDL_UpdateTexture(
                texture, NULL,
                nes.getPPU().frameBuffer.data(),
                NES_WIDTH * sizeof(uint32_t)
            );

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // Frame timing (~60 FPS)
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - lastFrameTime;
        if (elapsed.count() < 16.666) {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(16.666 - elapsed.count()));
        }
        lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    std::cout << "Exiting NES Emulator (Total frames rendered: " << totalFrames << ")." << std::endl;

    if (audioDevice > 0) {
        SDL_CloseAudioDevice(audioDevice);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
