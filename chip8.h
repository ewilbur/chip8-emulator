#ifndef CHIP8_H_
#define CHIP8_H_

#include <stdint.h>

#define CHIP8_NUM_REGISTERS 16
#define CHIP8_STACK_SIZE 16
#define CHIP8_KEYPAD_SIZE 16
#define CHIP8_DISPLAY_SIZE (64 * 32)
#define CHIP8_RAM_SIZE 0x1000
#define DEFAULT_PROGRAM_COUNTER 0x200

class Chip8 {
  public:
    /* | Registers */
    uint8_t V[CHIP8_NUM_REGISTERS]; // General purpose registers
    uint8_t dt;  // Delay timer
    uint8_t st;  // Sound timer
    uint8_t sp;  // Stack pointer
    uint16_t pc; // Program counter
    uint16_t ix; // Index register

    /* | Storage */
    uint8_t ram[CHIP8_RAM_SIZE];
    uint16_t stack[CHIP8_STACK_SIZE];

    /* | IO Storage */
    bool display[CHIP8_DISPLAY_SIZE];
    bool keypad[CHIP8_KEYPAD_SIZE];

    /* | Flags */
    bool drawFlag;
    bool debugFlag;

    /* | Misc */
    uint16_t opcode;

    /* Helper functions */
    uint8_t getNibble(uint16_t,unsigned int) const;
    uint8_t getOpcodeNibble(unsigned int) const;
    void fetchOpcode();
    void trace();

  public:
    Chip8();
    Chip8(const char*);
    ~Chip8();

    void executeCycle();

    void init();

    bool loadRom(const char*);

    bool getDrawFlag() const;
    void setDrawFlag(bool);

    bool getDisplayValue(int) const;

    bool getKeypadValue(int) const;
    void setKeypadValue(int, bool);

    void setDebugFlag(bool);

};

#endif // CHIP8_H_
