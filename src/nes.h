#ifndef NES_H
#define NES_H

#include "SD.h"
#include "core/cpu6502.h"
#include <TFT_eSPI.h>

class Nes
{
public:
    Nes();
    ~Nes();

    Cpu6502 cpu;
    void reset();
    void clockFrame();
    void insertCartridge(Cartridge* cartridge);
    void setController(uint8_t state);
    uint8_t getControllerState();

    void connectScreen(TFT_eSPI* screen);
    void connectFramebuffer(uint8_t* framebuffer);

    void setPalette(uint8_t palette);
    void setVolume(uint8_t volume);

    void saveState();
    void loadState();

private:
    static TFT_eSPI* ptr_screen;
    static void nesDrawCallback(uint8_t* framebuffer, uint32_t size);
};

#endif
