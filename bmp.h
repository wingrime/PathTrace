#pragma once 

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define u32 uint32_t
#define u16 uint16_t
#define s32 int32_t
#define s16 int16_t
#define u8  uint8_t
#define s8  int8_t

#define BMP_PPB 4

/*24 bit saved color*/
struct t_rgbc {
    u8 b;
    u8 g;
    u8 r;
    u8 reserved;
}__attribute__((__packed__));

 struct __attribute__((packed)) t_bmp_header {
  u16 bfType; /*signature = 4D42 */
  u32 bfSize; /*file size in bytes*/
  u16 bfReserved1;
  u16 bfReserved2;
  u32 bfOffBits; /*image offset*/
};
struct  t_bmp_info_header {
    u32 biSize; /*header size bytes= 40*/
    s32 biWidth; /*pixel width*/
    s32 biHeight; /*pixel height*/
    u16 biPlanes; /*planes = 1*/
    u16 bitCount; /* 24 - true color  */
    u32 biCompression; /*0 for uncompressed*/
    u32 biSizeImage; /* o for uncompressed*/
    s32 biXPelsPerMeter;
    s32 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;

}__attribute__((__packed__));


struct BMPData {
    u32 w;
    u32 h;
    t_rgbc * bits;
};
 
 BMPData  allocBMP(u32 w, u32 h) {
    BMPData b;
    b.h = h;
    b.w = w;
    b.bits = (t_rgbc * )calloc(b.h*b.w*BMP_PPB,sizeof(char));
    return b;
 }


int writeBMP(const char * fname, const BMPData& b) {
   FILE *f = fopen(fname,"wb");
   if (!f) {
        return -1;
    }
   
   t_bmp_header header;
   t_bmp_info_header iheader;
   memset(&header,0, sizeof(t_bmp_header));
   memset(&iheader,0, sizeof(t_bmp_info_header));
   header.bfType = 0x4d42;
   header.bfSize = sizeof(t_bmp_header)+sizeof(t_bmp_info_header)+b.w*b.h*BMP_PPB;
   header.bfOffBits = sizeof(t_bmp_header)+sizeof(t_bmp_info_header);

   iheader.biSize = sizeof(t_bmp_info_header);
   iheader.biWidth = b.w;
   iheader.biHeight = b.h;
   iheader.biPlanes = 1;
   iheader.bitCount = 8*BMP_PPB; //(8 bits * ppb)
   fwrite(&header,sizeof(header),(size_t)1,f);
   fwrite(&iheader,sizeof(iheader),(size_t)1,f);

   fwrite((void *)b.bits,b.w*b.h*BMP_PPB,(size_t)1,f);

   fclose(f);
   return 0;
}

