#include "chip8.h"
#include <iostream>
#include <random>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
/* #include <fstream> */

/* #define DEFAULT_PROGRAM_COUNTER 0x200 */

unsigned char chip8Fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8()
  : dt(0)
  , st(0)
  , sp(0)
  , pc(DEFAULT_PROGRAM_COUNTER)
  , ix(0)
  , drawFlag(0)
  , debugFlag(0)
  , opcode(0)
{
  for (int i = 0; i < 16; ++i) {
    V[i] = 0;
    stack[i] = 0;
    keypad[i] = 0;
  }

  for (int i = 0; i < CHIP8_RAM_SIZE; ++i)
    ram[i] = 0;

  for (int i = 0; i < CHIP8_DISPLAY_SIZE; ++i)
    display[i] = 0;
}

Chip8::Chip8(const char *filePath)
  : dt(0)
  , st(0)
  , sp(0)
  , pc(DEFAULT_PROGRAM_COUNTER)
  , ix(0)
  , drawFlag(0)
  , debugFlag(0)
  , opcode(0)
{
  loadRom(filePath);
}

Chip8::~Chip8() {

}

void Chip8::init() {
  dt = 0;
  st = 0;
  sp = 0;
  pc = DEFAULT_PROGRAM_COUNTER;
  ix = 0;
  drawFlag = 0;
  /* debugFlag = 0; */
  opcode = 0;

  for (int i = 0; i < 16; ++i) {
    V[i] = 0;
    stack[i] = 0;
    keypad[i] = 0;
  }

  for (int i = 0; i < CHIP8_RAM_SIZE; ++i)
    ram[i] = 0;

  for (int i = 0; i < CHIP8_DISPLAY_SIZE; ++i)
    display[i] = 0;

}

uint8_t Chip8::getNibble(uint16_t val, unsigned int nibNum) const {
  return (val & (0x000F << (nibNum * 4))) >> (4 * nibNum);
}

bool Chip8::loadRom(const char *filePath) {
  init();
  std::cout << "Loading ROM: " << filePath << std::endl;

  FILE *rom = fopen(filePath, "rb");
  if (rom == NULL) {
    std::cerr << "Failed to open ROM: " << filePath << std::endl;
    return false;
  }

  fseek(rom, 0, SEEK_END);
  long long unsigned int romSize = ftell(rom);
  rewind(rom);

  char *romBuffer = new char[romSize];
  if (romBuffer == NULL) {
    std::cerr << "Couldn't allocate memory for ROM" << std::endl;
    return false;
  }

  size_t res = fread(romBuffer, sizeof(char), (size_t) romSize, rom);
  if (res != romSize) {
    std::cerr << "Failed to read ROM" << std::endl;
    return false;
  }

  if (0x1000 - 0x200 <= romSize) {
    std::cerr << "ROM too large to run" << std::endl;
    return false;
  } else {
    for (int i = 0; i < romSize; ++i)
      ram[i + 0x200] = (uint8_t) romBuffer[i];
  }

  fclose(rom);
  delete[] romBuffer;
  return true;

}

uint8_t Chip8::getOpcodeNibble(unsigned int nibNum) const {
  return getNibble(opcode, nibNum);
}

bool Chip8::getDrawFlag() const {
  return drawFlag;
}

void Chip8::setDrawFlag(bool drawFlag) {
  this->drawFlag = drawFlag;
}

bool Chip8::getDisplayValue(int px) const {
  if (px >= CHIP8_DISPLAY_SIZE) {
    std::cerr << "Invalid pixel index " << px << " (maximum " << CHIP8_DISPLAY_SIZE - 1 << ")" << std::endl;
    return false;
  }
  else
    return display[px];
}

bool Chip8::getKeypadValue(int kp) const {
  if (kp >= CHIP8_KEYPAD_SIZE) {
    std::cerr << "Invalid keypad index " << kp << " (maximum " << CHIP8_KEYPAD_SIZE - 1 << ")" << std::endl;
    return false;
  }
  else
    return keypad[kp];
}

void Chip8::setKeypadValue(int kp, bool val) {
  if (kp >= CHIP8_KEYPAD_SIZE) {
    std::cerr << "Invalid keypad index " << kp << " (maximum " << CHIP8_KEYPAD_SIZE - 1 << ")" << std::endl;
    return;
  }
  else
    keypad[kp] = val;
}

void Chip8::fetchOpcode() {
  opcode = (ram[pc] << 8) | ram[pc + 1];
  pc += 2;
}

void Chip8::setDebugFlag(bool debugFlag) {
  this->debugFlag = debugFlag;
}

void Chip8::trace() {
  std::cout << "Opcode: " << std::hex << (int) opcode << std::endl;
  for (int i = 0; i < CHIP8_NUM_REGISTERS; ++i)
    std::cout << "V" << std::hex << i << ": " << std::hex << (int) V[i] << " ";
  std::cout << std::endl;
  std::cout << "DT: " << std::hex << (int) dt << " ";
  std::cout << "ST: " << std::hex << (int) st << " ";
  std::cout << "SP: " << std::hex << (int) sp << " ";
  std::cout << "PC: " << std::hex << (int) pc << " ";
  std::cout << "I: " << std::hex  << (int) ix << " " << std::endl << std::endl;
}

void Chip8::executeCycle() {
  fetchOpcode();
  if (debugFlag)
    trace();


  switch (getNibble(opcode, 3)) {
    case 0:
      switch (opcode) {
        case 0x00E0:
          memset(display, 0, CHIP8_DISPLAY_SIZE * sizeof(uint8_t));
          drawFlag = true;
          break;

        case 0x00EE:
          pc = stack[sp--];
          break;

        default:
          std::cerr << "Undefined opcode " << std::hex << opcode << std::endl;
          break;
      }
      break;

    case 1:
      pc = opcode & 0x0FFF;
      break;

    case 2:
      stack[++sp] = pc;
      pc = opcode & 0x0FFF;
      break;

    case 3:
      pc += 2 * (V[getOpcodeNibble(2)] == (opcode & 0x00FF));
      break;

    case 4:
      pc += 2 * (V[getOpcodeNibble(2)] != (opcode & 0x00FF));
      break;

    case 5:
      pc += 2 * (V[getOpcodeNibble(2)] == V[getOpcodeNibble(1)]);
      break;

    case 6:
      V[getOpcodeNibble(2)] = opcode & 0x00FF;
      break;

    case 7:
      V[getOpcodeNibble(2)] += opcode & 0x00FF;
      break;

    case 8:
      switch (getOpcodeNibble(0)) {
        case 0:
          V[getOpcodeNibble(2)] = V[getOpcodeNibble(1)];
          break;

        case 1:
          V[getOpcodeNibble(2)] |= V[getOpcodeNibble(1)];
          break;

        case 2:
          V[getOpcodeNibble(2)] &= V[getOpcodeNibble(1)];
          break;

        case 3:
          V[getOpcodeNibble(2)] ^= V[getOpcodeNibble(1)];
          break;

        // TODO Make sure that overflow detection is correct
        case 4:
          V[getOpcodeNibble(2)] += V[getOpcodeNibble(1)];
          /* V[0xF] = V[getOpcodeNibble(2)] < V[getOpcodeNibble(1)]; */
          V[0xF] = V[getOpcodeNibble(1)] > (0xFF - V[getOpcodeNibble(2)]);
          break;

        case 5:
          V[0xF] = V[getOpcodeNibble(2)] > V[getOpcodeNibble(1)];
          V[getOpcodeNibble(2)] -= V[getOpcodeNibble(1)];
          break;

        case 6:
          V[0xF] = V[getOpcodeNibble(2)] & 0x0001;
          V[getOpcodeNibble(2)] >>= 1;
          break;

        case 7:
          V[0xF] = V[getOpcodeNibble(1)] > V[getOpcodeNibble(2)];
          V[getOpcodeNibble(2)] = V[getOpcodeNibble(1)] - V[getOpcodeNibble(2)];
          break;

        case 0xE:
          V[0xF] = !!(V[getOpcodeNibble(2)] & 0x8000);
          V[getOpcodeNibble(2)] <<= 1;
          break;

        default:
          std::cerr << "Undefined opcode " << std::hex << opcode << std::endl;
          break;
      }
      break;

    case 9:
      pc += 2 * (V[getOpcodeNibble(2)] != V[getOpcodeNibble(1)]);
      break;

    case 0xA:
      ix = opcode & 0x0FFF;
      break;

    case 0xB:
      pc = V[0] + (opcode & 0x0FFF);
      break;

    case 0xC:
      V[getOpcodeNibble(2)] = rand() & (opcode & 0x00FF);
      break;

    case 0xD: {
        uint8_t x = V[getOpcodeNibble(2)];
        uint8_t y = V[getOpcodeNibble(1)];
        uint8_t height = opcode & 0x000F;
        uint8_t pixel;

        V[0xF] = false;

        for (int yline = 0; yline < height; ++yline) {
          pixel = ram[ix + yline];
          for (int xline = 0; xline < 8; ++xline) {
            if((pixel & (0x80 >> xline)) != 0) {
              if (display[(x + xline + ((y + yline) * 64))] == 1)
                V[0xF] = 1;

              display[x + xline + ((y + yline) * 64)] ^= 1;
          }
        }
      }

      drawFlag = true;
    }

      break;

    case 0xE:
      switch (opcode & 0x00FF) {
        case 0x009E:
          pc += 2 * (!!(keypad[V[getOpcodeNibble(2)]]));
          break;

        case 0x00A1:
          pc += 2 * (!(keypad[V[getOpcodeNibble(2)]]));
          break;

        default:
          std::cerr << "Undefined opcode " << std::hex << opcode << std::endl;
          break;
      }
      break;

    case 0xF:
      switch (opcode & 0x00FF) {
        case 0x07:
          V[getOpcodeNibble(2)] = dt;
          break;

        case 0x0A:
          for (int i = 0; i < CHIP8_KEYPAD_SIZE; ++i) {
            if (keypad[i]) {
              V[getOpcodeNibble(2)] = (uint8_t) i;
              goto exitMainSwitch;
            }
          }
          pc -= 2;
          break;

        case 0x15:
          dt = V[getOpcodeNibble(2)];
          break;

        case 0x18:
          st = V[getOpcodeNibble(2)];
          break;

        case 0x1E:
          ix += V[getOpcodeNibble(2)];
          break;

        case 0x29:
          ix = V[getOpcodeNibble(2)] * 0x5;
          break;

        case 0x33:
          ram[ix] = V[getOpcodeNibble(2)] / 100;
          ram[ix + 1] = (V[getOpcodeNibble(2)] / 10) % 10;
          ram[ix + 2] = V[getOpcodeNibble(2)] % 10;
          break;

        case 0x55:
          for (int i = 0; i <= getOpcodeNibble(2); ++i)
            ram[ix + i] = V[i];
          break;

        case 0x65:
          for (int i = 0; i <= getOpcodeNibble(2); ++i)
            V[i] = ram[ix + i];
          break;

        default:
          std::cerr << "Undefined opcode " << std::hex << opcode << std::endl;
          break;
      }
      break;

  default:
      std::cerr << "This can never happen " << std::hex << opcode << std::endl;
  }

  exitMainSwitch:
  if (dt > 0)
    --dt;
  if (st > 0)
    --st;
  return;
}
