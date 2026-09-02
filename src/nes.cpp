#include "nes.h"

TFT_eSPI* Nes::ptr_screen = nullptr;

Nes::Nes()
{
    cpu.bus.ppu.setDrawCallback(nesDrawCallback);
}

Nes::~Nes()
{
}

void Nes::reset()
{
    cpu.reset();
}

IRAM_ATTR void Nes::clockFrame()
{
    cpu.clockFrame();
}

void Nes::insertCartridge(Cartridge* cartridge)
{
    cpu.bus.insertCartridge(cartridge);
}

void Nes::setController(uint8_t state)
{
    cpu.bus.setController(state);
}

uint8_t Nes::getControllerState()
{
    return cpu.bus.getControllerState();
}

void Nes::connectScreen(TFT_eSPI* screen)
{
    ptr_screen = screen;
}

void Nes::connectFramebuffer(uint8_t* framebuffer)
{
    cpu.bus.ppu.connectFramebuffer(framebuffer);
}

IRAM_ATTR void Nes::nesDrawCallback(uint8_t* framebuffer, uint32_t size)
{
#ifndef COMPOSITE_VIDEO
    uint16_t* pixels = (uint16_t*)framebuffer;
    uint32_t pixel_count = size / sizeof(uint16_t);
    #ifndef DISABLE_DMA
    ptr_screen->pushPixelsDMA(pixels, pixel_count);
    #else
    ptr_screen->pushPixels(pixels, pixel_count);
    #endif
#endif
}

void Nes::setPalette(uint8_t palette)
{
    cpu.bus.ppu.setPalette(palette);
}

void Nes::setVolume(uint8_t volume)
{
    cpu.apu.setVolume(volume);
}

void Nes::saveState()
{
    if (!SD.exists("/states")) SD.mkdir("/states");
    uint32_t CRC32 = cpu.bus.cart->CRC32;

    static char CRC32_str[9];
    sprintf(CRC32_str, "%08lX", (unsigned long)CRC32);

    static char filename[32];
    sprintf(filename, "/states/%s.state", CRC32_str);

    static char tmp_filename[32 + 4];
    sprintf(tmp_filename, "%s.tmp", filename);
    File state = SD.open(tmp_filename, FILE_WRITE);
    if (!state) return;

    // Header for verification - ANEMOIA + CRC32
    state.print("ANEMOIA");
    state.write((const uint8_t*)CRC32_str, 8);

    // Write emulator state
    state.write(cpu.bus.RAM, sizeof(cpu.bus.RAM));
    cpu.dumpState(state);
    cpu.bus.ppu.dumpState(state);
    cpu.bus.cart->dumpState(state);

    state.close();

    SD.remove(filename);
    SD.rename(tmp_filename, filename);
}

void Nes::loadState()
{
    uint32_t CRC32 = cpu.bus.cart->CRC32;

    static char CRC32_str[9];
    sprintf(CRC32_str, "%08lX", (unsigned long)CRC32);

    static char filename[32];
    sprintf(filename, "/states/%s.state", CRC32_str);
    if (!SD.exists(filename)) return;

    File state = SD.open(filename, FILE_READ);
    if (!state) return;

    // Verify header
    static char header[8];
    static char CRC[9];
    state.read((uint8_t*)&header, 7);
    header[7] = '\0';
    state.read((uint8_t*)&CRC, 8);
    CRC[8] = '\0';

    if (strcmp(header, "ANEMOIA") != 0)
    {
        state.close();
        return;
    }
    if (strcmp(CRC, CRC32_str) != 0)
    {
        state.close();
        return;
    }

    // Load state
    state.read(cpu.bus.RAM, sizeof(cpu.bus.RAM));
    cpu.loadState(state);
    cpu.bus.ppu.loadState(state);
    cpu.bus.cart->loadState(state);

    state.close();
}
