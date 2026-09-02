#include "bus.h"
#include "cpu6502.h"

Bus::Bus()
{
    memset(RAM, 0, sizeof(RAM));
    ppu.connectBus(this);
}

Bus::~Bus()
{
}

IRAM_ATTR void Bus::cpuWrite(uint16_t addr, uint8_t data)
{
    PROFILE_SCOPE(PROF_BUS_CPU_WRITE);
    if (uint8_t* p = write_pages[addr >> 8])
    {
        p[addr & 0xFF] = data;
        return;
    }
    write_handlers[addr >> 8](this, addr, data);
}

IRAM_ATTR uint8_t Bus::cpuRead(uint16_t addr)
{
    PROFILE_SCOPE(PROF_BUS_CPU_READ);
    if (uint8_t* p = read_pages[addr >> 8]) return p[addr & 0xFF];
    return read_handlers[addr >> 8](this, addr);
}

void Bus::setController(uint8_t state)
{
    controller = state;
}

uint8_t Bus::getControllerState()
{
    return controller;
}

void Bus::reset()
{
    for (auto& i : RAM) i = 0x00;

    buildPageTables();
    cart->reset();
    cart->mapPages(this);
    ppu.buildPPUPageTables();
    cart->mapPPUPages(&ppu);

    ppu.reset();
}

IRAM_ATTR void Bus::setPPUMirrorMode(MIRROR mirror)
{
    ppu.setMirror(mirror);
}

MIRROR Bus::getPPUMirrorMode()
{
    return ppu.getMirror();
}

void Bus::insertCartridge(Cartridge* cartridge)
{
    cart = cartridge;
    cart->connectBus(this);
    ppu.connectCartridge(cartridge);
}

void Bus::connectCPU(Cpu6502* n)
{
    cpu = n;
}

IRAM_ATTR void Bus::IRQ()
{
    cpu->IRQ();
}

IRAM_ATTR void Bus::NMI()
{
    cpu->NMI();
}

static void defaultWriteHandler(Bus* b, uint16_t a, uint8_t d)
{
    return;
}

static uint8_t defaultReadHandler(Bus* b, uint16_t a)
{
    return 0x00;
}

// Builds the memory map for bus read and writes
void Bus::buildPageTables()
{
    for (int p = 0; p < NUM_PAGES; p++)
    {
        read_pages[p] = nullptr;
        write_pages[p] = nullptr;
        read_handlers[p] = defaultReadHandler;
        write_handlers[p] = defaultWriteHandler;
    }

    // $0000 - $1FFF: 2KB RAM mirrored x4
    for (int p = 0x00; p <= 0x1F; p++)
    {
        uint8_t* base = &RAM[(p % 8) * PAGE_SIZE];
        read_pages[p] = base;
        write_pages[p] = base;
    }

    // $2000 - $3FFF: PPU registers, mirrored every 8 bytes
    for (int p = 0x20; p <= 0x3F; p++)
    {
        read_handlers[p] = [](Bus* b, uint16_t a) -> uint8_t { return b->ppu.cpuRead(a & 0x0007); };
        write_handlers[p] = [](Bus* b, uint16_t a, uint8_t d) { b->ppu.cpuWrite(a & 0x0007, d); };
    }

    // $4000 - $401F: APU/IO
    read_handlers[0x40] = [](Bus* b, uint16_t a) -> uint8_t
    {
        if (a == 0x4016)
        {
            uint8_t value = b->controller_state & 1;
            if (!b->controller_strobe) b->controller_state >>= 1;
            return value | 0x40;
        }
        return 0x00;
    };
    write_handlers[0x40] = [](Bus* b, uint16_t a, uint8_t d)
    {
        if (a == 0x4014) b->cpu->OAM_DMA(d);
        else if (a <= 0x4013 || a == 0x4015 || a == 0x4017) b->cpu->apuWrite(a, d);
        else if (a == 0x4016)
        {
            b->controller_strobe = d & 1;
            if (b->controller_strobe) b->controller_state = b->controller;
        }
    };
}
