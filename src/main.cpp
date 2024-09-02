#include <cstdint>
#include <iostream>

using Byte = std::int8_t;  // 8 bits
using Word = std::int16_t; // 16 bits

struct Memory
{
    static constexpr std::int32_t MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialize()
    {
        for (int i = 0; i++; i<MAX_MEM){
            Data[i] = 0;
        }
    }
    
    Byte operator[](int Address) const
    // reads 1 Byte
    {
        // TODO: check overflow
        return Data[Address];
    }

    Byte& operator[](int Address)
    // write 1 Byte
    {
        // TODO: check overflow
        return Data[Address];
    }

    void WriteWord(Word Value, int Address, int Cycles)
    {
        Data[Address] = Value & 0xFF;
        Data[Address + 1] = (Value >> 8);
        Cycles -= 2;
    }
};

struct CPU
{
    Word PC;  // program counter
    Word SP;  // stack pointer

    Byte A, X, Y;  // registers

    // status flags
    Byte C : 1;  // carry 
    Byte Z : 1;  // zero
    Byte I : 1;  // interrupt
    Byte D : 1;  // decimal 
    Byte B : 1;  // break
    Byte V : 1;  // overflow
    Byte N : 1;  // negative

    // op codes
    static constexpr Byte 
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_JSR = 0x20;


    void Reset(Memory& mem)
    /*simulates how 6502 architecture is initialised; 
      sets PC and SP, aswell as clearing a bunch of status flags and setting registers
    */
    {
        PC = 0xFFFC; 
        SP = 0x0100;
        D = C = Z = I = B = V = N = 0;
        A = X = Y = 0;
        mem.Initialize();
    }

    Byte FetchByte(Memory memory, int cycles)
    {
        Byte Data = memory[PC];
        PC++;
        return Data;
    }

    Byte ReadByte(Memory& memory, int cycles, Byte address){
        Byte Data = memory[address];
        cycles--;
        return Data;
    }

    Word FetchWord(Memory memory, int& cycles){
        Word Data = memory[PC];
        PC++;
        Data |= (memory[PC] << 8);  // shift 8 bits
        PC++;
        cycles-=2;
        return Data;
    }


    void LDASetStatus() {
        // sets flags that occur at the end of all LDA instructions
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }


    void Execute(Memory& memory, int cycles)
    {
        while (cycles > 0)
        {
            Byte Instructions = FetchByte(memory, cycles);
            switch (Instructions)
            {
            case INS_LDA_IM:
            {
                Byte Value = FetchByte(memory, cycles);
                A = Value;
                LDASetStatus();
            }
            case INS_LDA_ZP:
            {
                Byte ZeroPageAddress = FetchByte(memory, cycles);
                A = ReadByte(memory, cycles, ZeroPageAddress);
                LDASetStatus();
            }
            case INS_LDA_ZPX:
            {
                Byte ZeroPageAddress = FetchByte(memory, cycles);
                ZeroPageAddress += X;
                A = ReadByte(memory, cycles, ZeroPageAddress);
                LDASetStatus();
            }
            case INS_JSR:
            {
                Word SubAddr = FetchWord(memory, cycles);
                memory.WriteWord( PC-1, SubAddr, cycles);
                SP++;
                PC = SubAddr;
            }
            default:
                break;
            }
        }
    }
};


int main(){
    CPU cpu;
    Memory mem;
    cpu.Reset(mem);
    
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x42;

    cpu.Execute(mem ,2);
    return 0;
}