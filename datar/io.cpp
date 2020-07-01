#include "io.hpp"

uint8_t r_base;
#ifdef HARDIN
std::ifstream* i_file;
//int i_file_pos;
char r_char;
#else
char* r_buffer;
int r_buf_index;
uint32_t r_buf_size;
#endif
uint8_t r_pos;
#ifdef HARDOUT
std::ofstream* o_file;
#else
char* w_buffer;
int w_buf_index;
#endif
uint8_t w_pos=8;
uint8_t r_byte;
//uint8_t w_byte=0;
uint8_t w_byte;
uint8_t w_base;

#ifdef HARDIN
void read(bool& bl)
{
//    char r_char;
    if(r_pos)
    {
        r_pos--;
        bl=(r_byte>>r_pos)&1;
    }
    else
    {
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
        r_pos=7;
        bl=r_byte>>7;
    }
}
void read(uint8_t& byte)
{
//    char r_char;
    if(r_pos>=r_base)
    {
        r_pos-=r_base;
        byte=(r_byte>>r_pos)&(0xFF>>(8-r_base));
    }
    else if(!r_pos)
    {
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
        r_pos=8-r_base;
        byte=r_byte>>r_pos;
    }
    else
    {
        byte=(r_byte<<(r_base-r_pos))&(0xFF>>(8-r_base));
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
        r_pos+=8-r_base;
        byte+=r_byte>>r_pos;
    }
}
void read(uint16_t& word)
{
//    char r_char;
    if(!r_pos)
    {
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
        r_pos=8;
    }
    uint8_t base=r_base;
    word=0;
    if(base>=r_pos && r_pos!=8)
    {
        base-=r_pos;
        word+=(r_byte&(~(0xFFFF<<r_pos)))<<base;
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
        r_pos=8;
    }
    if(base>=r_pos)
    {
        base-=r_pos;
        word+=r_byte<<base;
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
    }
    if(base!=0)
    {
        r_pos-=base;
        word+=(r_byte>>r_pos)&(~(0xFFFF<<base));
    }
}
void read(uint32_t& dword)
{
//    char r_char;
    if(!r_pos)
    {
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
        r_pos=8;
    }
    uint8_t base=r_base;
    dword=0;
    if(base>=r_pos && r_pos!=8)
    {
        base-=r_pos;
        dword+=(r_byte&(~(0xFFFFFFFF<<r_pos)))<<base;
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
        r_pos=8;
    }
    while(base>=r_pos)
    {
        base-=r_pos;
        dword+=r_byte<<base;
        i_file->get(r_char);
        r_byte=r_char;
        //  i_file_pos++;
    }
    if(base)
    {
        r_pos-=base;
        dword+=(r_byte>>r_pos)&(~(0xFFFFFFFF<<base));
    }
}
#else
void read(bool& bl)
{
    if(r_pos)
    {
        r_pos--;
        bl=(r_byte>>r_pos)&1;
    }
    else
    {
        r_byte=r_buffer[r_buf_index++];
        r_pos=7;
        bl=r_byte>>7!=0;
    }
}
void read(uint8_t& byte)
{
    if(r_pos>=r_base)
    {
        r_pos-=r_base;
        byte=(r_byte>>r_pos)&(0xFF>>(8-r_base));
    }
    else if(!r_pos)
    {
        r_byte=r_buffer[r_buf_index++];
        r_pos=8-r_base;
        byte=r_byte>>r_pos;
    }
    else
    {
        byte=(r_byte<<(r_base-r_pos))&(0xFF>>(8-r_base));
        r_byte=r_buffer[r_buf_index++];
        r_pos+=8-r_base;
        byte+=r_byte>>r_pos;
    }
}
void read(uint16_t& word)
{
    if(!r_pos)
    {
        r_byte=r_buffer[r_buf_index++];
        r_pos=8;
    }
    uint8_t base=r_base;
    word=0;
    if(base>=r_pos && r_pos!=8)
    {
        base-=r_pos;
        word+=(r_byte&(~(0xFFFF<<r_pos)))<<base;
        r_byte=r_buffer[r_buf_index++];
        r_pos=8;
    }
    if(base>=r_pos)
    {
        base-=r_pos;
        word+=r_byte<<base;
        r_byte=r_buffer[r_buf_index++];
    }
    if(base)
    {
        r_pos-=base;
        word+=(r_byte>>r_pos)&(~(0xFFFF<<base));
    }
}
void read(uint32_t& dword)
{
    if(!r_pos)
    {
        r_byte=r_buffer[r_buf_index++];
        r_pos=8;
    }
    uint8_t base=r_base;
    dword=0;
    if(base>=r_pos && r_pos!=8)
    {
        base-=r_pos;
        dword+=(r_byte&(~(0xFFFFFFFF<<r_pos)))<<base;
        r_byte=r_buffer[r_buf_index++];
        r_pos=8;
    }
    while(base>=r_pos)
    {
        base-=r_pos;
        dword+=r_byte<<base;
        r_byte=r_buffer[r_buf_index++];
    }
    if(base)
    {
        r_pos-=base;
        dword+=(r_byte>>r_pos)&(~(0xFFFFFFFF<<base));
    }
}
#endif
#ifdef HARDOUT
void write(bool bl)
{
    w_pos--;
    if(bl)
        w_byte+=1<<w_pos;
    if(!w_pos)
    {
        o_file->put(w_byte);
        w_byte=0;
        w_pos=8;
    }
}
void write(uint8_t byte)
{
    if(w_pos>w_base)
    {
        w_pos-=w_base;
        w_byte+=byte<<w_pos;
    }
    else if(w_pos<w_base)
    {
        w_byte+=byte>>(w_base-w_pos);
        o_file->put(w_byte);
        w_pos+=8-w_base;
        w_byte=byte<<w_pos;
    }
    else
    {
        w_byte+=byte;
        o_file->put(w_byte);
        w_byte=0;
        w_pos=8;
    }
}
void write(uint16_t word)
{
    uint8_t base=w_base;
    if(base>=w_pos && w_pos!=8)
    {
        base-=w_pos;
        w_byte+=word>>base;
        o_file->put(w_byte);
        w_byte=0;
        w_pos=8;
    }
    if(base>=w_pos)
    {
        base-=w_pos;
        o_file->put(char(uint8_t(word>>base)));
    }
    if(base)
    {
        w_pos-=base;
        w_byte+=word<<w_pos;
    }
}
void write(uint32_t dword)
{
    uint8_t base=w_base;
    if(base>=w_pos && w_pos!=8)
    {
        base-=w_pos;
        w_byte+=dword>>base;
        o_file->put(w_byte);
        w_byte=0;
        w_pos=8;
    }
    while(base>=w_pos)
    {
        base-=w_pos;
        o_file->put(char(uint8_t(dword>>base)));
    }
    if(base)
    {
        w_pos-=base;
        w_byte+=dword<<w_pos;
    }
}
#else
void write(bool bl)
{
    w_pos--;
    if(bl)
        w_byte+=1<<w_pos;
    if(!w_pos)
    {
        w_buffer[w_buf_index++]=w_byte;
        w_byte=0;
        w_pos=8;
    }
}
void write(uint8_t byte)
{
    if(w_pos>w_base)
    {
        w_pos-=w_base;
        w_byte+=byte<<w_pos;
    }
    else if(w_pos<w_base)
    {
        w_byte+=byte>>(w_base-w_pos);
        w_buffer[w_buf_index++]=w_byte;
        w_pos+=8-w_base;
        w_byte=byte<<w_pos;
    }
    else
    {
        w_byte+=byte;
        w_buffer[w_buf_index++]=w_byte;
        w_byte=0;
        w_pos=8;
    }
}
void write(uint16_t word)
{
    uint8_t base=w_base;
    if(base>=w_pos && w_pos!=8)
    {
        base-=w_pos;
        w_byte+=word>>base;
        w_buffer[w_buf_index++]=w_byte;
        w_byte=0;
        w_pos=8;
    }
    if(base>=w_pos)
    {
        base-=w_pos;
        w_buffer[w_buf_index++]=word>>base;
    }
    if(base)
    {
        w_pos-=base;
        w_byte+=word<<w_pos;
    }
}
void write(uint32_t dword)
{
    uint8_t base=w_base;
    if(base>=w_pos && w_pos!=8)
    {
        base-=w_pos;
        w_byte+=dword>>base;
        w_buffer[w_buf_index++]=w_byte;
        w_byte=0;
        w_pos=8;
    }
    while(base>=w_pos)
    {
        base-=w_pos;
        w_buffer[w_buf_index++]=dword>>base;
    }
    if(base)
    {
        w_pos-=base;
        w_byte+=dword<<w_pos;
    }
}
#endif
uint8_t get_base(uint32_t n)
{
    if(n<3)
        return n;
    for(uint8_t i=0;i<=32;i++)
        if(n>>i==0)
            return i;
    return 0;
}
