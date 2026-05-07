#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <math.h>
#include "emulator.h"

#define RESOURCES_PATH(path) ("./resources/" path)

const KeyboardKey layout[16] = {
    KEY_X,
    KEY_ONE,
    KEY_TWO,
    KEY_THREE,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_Z,
    KEY_C,
    KEY_FOUR,
    KEY_R,
    KEY_F,
    KEY_V
};

int main(int argc, char* argv[]) {
    uint16_t quirks = 35;
    // 35 - original
    // 84 - super-modern
    // 86 - super-legacy
    // 40 - xo

    // Autoload
    char games[100] = "./resources/";
    char *rom = NULL;
    int frecuency = 16;
    int type = 0;

    for(int i = 0; i < argc; i++) {
        if(strcmp("-q", argv[i]) == 0 && i + 1 < argc) {
            quirks = atoi(argv[++i]);
        }else if(strcmp("-f", argv[i]) == 0 && i + 1 < argc) {
            frecuency = atoi(argv[++i]);
        }else if(strcmp("-t", argv[i]) == 0 && i + 1 < argc) {
            type = atoi(argv[++i]);
        }else {
            rom =  argv[++i];
        }
    }

    Emulator emulator;

    emulator_init(&emulator);

    if(rom == NULL) {
        rom = strcat(games, "BOOT.txt");
        emulator_load_file(&emulator, rom);
    } else {
        rom = strcat(strcat(games, "games/"), rom);

        if (type == 0) {
            emulator_load_rom(&emulator, rom);
        } else {
            emulator_load_file(&emulator, rom);
        }
    }

    emulator.quirk_shift = quirks & 0b1000000;
    emulator.quirk_increment_i_by_register = quirks & 0b0100000;
    emulator.quirk_no_increment_i = quirks & 0b0010000;
    emulator.quirk_wrap = quirks & 0b0001000;
    emulator.quirk_jump = quirks & 0b0000100;
    emulator.quirk_blank = quirks & 0b0000010;
    emulator.quirk_logic = quirks & 0b0000001;

    InitWindow(64 * 20, 32 * 20, "CHIP-8 Emulator");
    InitAudioDevice();
    SetTargetFPS(60);
    
    RenderTexture2D texture = LoadRenderTexture(64 * 20, 32 * 20);
    Shader shader = LoadShader(0, RESOURCES_PATH("shaders/crt.glsl"));

    // Sound
    Wave wave = { 0 };
    wave.frameCount = 44100;
    wave.sampleRate = 44100;
    wave.sampleSize = 32;
    wave.channels = 1;

    wave.data = (float *)malloc(wave.frameCount * wave.channels * sizeof(float));

    float sound_frecuency = 440.0f;

    for(int i = 0; i < wave.frameCount; i++) {
        ((float *)wave.data)[i] = sinf(2.0f * PI * sound_frecuency * i / wave.sampleRate);
    }

    Sound sound = LoadSoundFromWave(wave);
    free(wave.data);

    double frame_duration = 1.0f / 60.0f;
    double time0 = 0.0f;

    while(!WindowShouldClose()) {
        double time1 = GetFrameTime();
        time0 += time1;

        if(time0 >= frame_duration) {
            // Timers
            if (emulator.delay_timer > 0) {
                emulator.delay_timer--;
            }
            if (emulator.sound_timer > 0) {
                emulator.sound_timer--;

                if(!IsSoundPlaying(sound)) { PlaySound(sound); }
            } else {
                StopSound(sound);
            }

            time0 -= frame_duration;
            emulator.display_rendered = FALSE;
        }

        // Inputs
        for(int i = 0; i < 0x10; i++) {
            if(IsKeyDown(layout[i])) {
                emulator.keypad[i] = 1;
            }else {
                emulator.keypad[i] = 0;
            }
        }

        // Steps
        if(emulator.quirk_blank) {
            if(!emulator.display_rendered) {
                for (int i = 0; i < frecuency; i++) {
                    if(!emulator.display_rendered){
                        emulator_step(&emulator);
                    }
                }
            }
        }else {
            for (int i = 0; i < frecuency; i++) {
                emulator_step(&emulator);
            }
        }

        // Render
        BeginTextureMode(texture);
        ClearBackground(BLACK);

        int screen_width = !emulator.original? 128: 64;
        int screen_height = !emulator.original? 64: 32;
        int screen_scale = GetScreenWidth() / screen_width;

        for(int y = 0; y < screen_height; y++) {
            for(int x = 0; x < screen_width; x++) {
                if(emulator.display[y * screen_width + x]) {
                    DrawRectangle(
                        x * screen_scale,
                        y * screen_scale,
                        screen_scale,
                        screen_scale,
                        WHITE
                    );
                }
            }
        }

        EndTextureMode();

        if(IsKeyPressed(KEY_SPACE)) emulator.shader = !emulator.shader;
        if(IsKeyPressed(KEY_LEFT_CONTROL)) emulator_restart(&emulator);

        // Shader
        BeginDrawing();

        if (emulator.shader) {
            BeginShaderMode(shader);

            DrawTextureRec(
                texture.texture,
                (Rectangle){0, 0, (float)texture.texture.width,
                (float)-texture.texture.height},
                (Vector2){ 0, 0}, WHITE
            );

            EndShaderMode();
        } else {
            DrawTextureRec(
                texture.texture,
                (Rectangle){0, 0, (float)texture.texture.width,
                (float)-texture.texture.height},
                (Vector2){0, 0}, WHITE
            );
        }

        EndDrawing();
    }

    UnloadSound(sound);
    UnloadShader(shader);
    UnloadRenderTexture(texture);
    CloseWindow();

    return 0;
}