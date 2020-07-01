#ifndef IO_H
#define IO_H
#include <fstream>
//#define COMPLEX
#ifdef COMPLEX
#define TABLES
#ifdef TABLES
#define HUFFMAN
/*
#ifdef HUFFMAN
#include "Huffman.hpp"
#endif
*/
#endif
#endif
//#define STAT
//#define STAT1
#define HARDIN
#define HARDOUT
extern uint8_t r_base;
#ifdef HARDIN
extern std::ifstream* i_file;
extern int i_file_pos;
extern char r_char;
#else
extern char* r_buffer;
extern int r_buf_index;
extern uint32_t r_buf_size;
#endif
extern uint8_t r_pos;
extern std::ofstream* o_file;
#ifdef HARDOUT

#else
extern char* w_buffer;
extern int w_buf_index;
#endif
extern uint8_t w_pos;
extern uint8_t r_byte;
extern uint8_t w_byte;
extern uint8_t w_base;
void read(bool& bl);
void read(uint8_t& byte);
void read(uint16_t& word);
void read(uint32_t& dword);
void write(bool bl);
void write(uint8_t byte);
void write(uint16_t word);
void write(uint32_t word);
uint8_t get_base(uint32_t n);

#endif
