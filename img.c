/*
 * img.c
 *
 *  Created on: Jun 28, 2015
 *      Author: glouthan
 */


#include "img.h"

static const unsigned long palette_bw[]=
{
     0x000000,      0xffffff
};

static const unsigned long palette_wb[]=
{
     0xffffff,      0x000000
};

static const uint8_t pixel_fingerprint_1BPP_UNCOMP[] =
{
        0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b11111011, 0b11110111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b11101110, 0b11011100, 0b00111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b11010011, 0b00100111, 0b11000111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b11101000, 0b11111000, 0b01111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b10010111, 0b10111111, 0b10000000, 0b00111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b00111101, 0b11011100, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111100, 0b11000111, 0b01100111, 0b00000111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11110111, 0b01111001, 0b10111001, 0b11111111, 0b11100111, 0b11111111,
        0b11111111, 0b11111111, 0b11111100, 0b01000110, 0b11001111, 0b10001110, 0b11101111, 0b11111111,
        0b11111111, 0b11111111, 0b11110011, 0b10111001, 0b11111001, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11101110, 0b01110110, 0b01001111, 0b00011111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111011, 0b10011011, 0b00110001, 0b11111000, 0b00000111, 0b11111111,
        0b11111111, 0b11111111, 0b01100111, 0b01101101, 0b10011100, 0b00011110, 0b00111110, 0b11111111,
        0b11111111, 0b11111111, 0b10011101, 0b10011000, 0b11100111, 0b11111111, 0b11111001, 0b11111111,
        0b11111111, 0b11111110, 0b00100111, 0b01100110, 0b00111000, 0b11111000, 0b00011111, 0b01111111,
        0b11111111, 0b11111110, 0b11001100, 0b10011001, 0b10001110, 0b00100111, 0b11111001, 0b10111111,
        0b11111111, 0b11111111, 0b10110011, 0b01101100, 0b01100011, 0b11111111, 0b00100011, 0b01111111,
        0b11111111, 0b11111110, 0b11011110, 0b11010111, 0b00011111, 0b10000001, 0b11011101, 0b11011111,
        0b11111111, 0b11111101, 0b11110111, 0b00101000, 0b11000111, 0b11111111, 0b01110111, 0b01101111,
        0b11111111, 0b11100111, 0b10011110, 0b11011111, 0b00111001, 0b11111001, 0b11001101, 0b11011111,
        0b11111111, 0b11111100, 0b11100111, 0b01100110, 0b11001110, 0b01010011, 0b11110111, 0b01101111,
        0b11111111, 0b11110011, 0b10010000, 0b10011011, 0b00110000, 0b00000111, 0b11001101, 0b11011111,
        0b11111111, 0b11100110, 0b01100100, 0b01101100, 0b10011001, 0b11111100, 0b01111110, 0b01101111,
        0b11111111, 0b11011001, 0b10011011, 0b00110110, 0b11000111, 0b11111001, 0b11110011, 0b11110111,
        0b11111111, 0b10110110, 0b01101100, 0b10010011, 0b00110000, 0b10001111, 0b10001110, 0b11101111,
        0b11111111, 0b11101101, 0b10011011, 0b01101101, 0b10011111, 0b11111110, 0b01111011, 0b10110111,
        0b11111111, 0b10010011, 0b01100100, 0b10110110, 0b11100011, 0b11110011, 0b11001111, 0b11111111,
        0b11111111, 0b01100001, 0b10011011, 0b01011011, 0b00110000, 0b00011111, 0b01111011, 0b10110111,
        0b11111111, 0b10011011, 0b00110101, 0b00101100, 0b10011111, 0b11111100, 0b11101110, 0b11111111,
        0b11111111, 0b00110100, 0b11011010, 0b11010110, 0b11001111, 0b11110011, 0b10011001, 0b10110111,
        0b11111110, 0b11001101, 0b10110101, 0b01011010, 0b01100111, 0b11100111, 0b01100110, 0b11001011,
        0b11111101, 0b10110010, 0b11111011, 0b10101101, 0b10100000, 0b00011101, 0b11011001, 0b10110111,
        0b11111110, 0b01101001, 0b01111111, 0b11110111, 0b00011111, 0b11111011, 0b10110110, 0b01101011,
        0b11111100, 0b10010010, 0b11110101, 0b01011110, 0b11011111, 0b10000110, 0b01101101, 0b10110111,
        0b11111011, 0b01101111, 0b11011010, 0b10111011, 0b11101111, 0b10111110, 0b11011011, 0b01111111,
        0b11111100, 0b11010111, 0b11110101, 0b11111101, 0b00110011, 0b11111101, 0b11100110, 0b11111111,
        0b11111011, 0b10101111, 0b11011010, 0b11011100, 0b11001111, 0b11110011, 0b00111011, 0b01101011,
        0b11110110, 0b11010111, 0b01110111, 0b10101010, 0b01111111, 0b11100110, 0b01101111, 0b11110111,
        0b11111011, 0b01101010, 0b10101011, 0b11101101, 0b10001111, 0b11011110, 0b10010101, 0b10101011,
        0b11111100, 0b10010111, 0b01011110, 0b11010010, 0b01100010, 0b00110011, 0b00101011, 0b01111111,
        0b11111001, 0b01101001, 0b10111101, 0b11101101, 0b10111000, 0b00110110, 0b11011010, 0b11111011,
        0b11110110, 0b11010101, 0b11011010, 0b11111110, 0b11001111, 0b11101101, 0b10111101, 0b11110101,
        0b11101001, 0b00101101, 0b10110111, 0b11111101, 0b11111100, 0b00011011, 0b01011010, 0b01011011,
        0b11110110, 0b11010011, 0b01100111, 0b11111111, 0b11111111, 0b11100110, 0b10101101, 0b10101111,
        0b11101101, 0b00100110, 0b11011111, 0b11111111, 0b11111111, 0b11111011, 0b01111111, 0b11010111,
        0b11111011, 0b01010011, 0b10101111, 0b11111111, 0b11111111, 0b11110110, 0b01111101, 0b01101011,
        0b11100100, 0b10101100, 0b11010111, 0b11111111, 0b11111111, 0b11101011, 0b11110110, 0b11110101,
        0b11011101, 0b01110011, 0b10111111, 0b11111111, 0b11111111, 0b11100101, 0b11101111, 0b01111011,
        0b11101010, 0b01110100, 0b11101111, 0b10000000, 0b00000011, 0b11110111, 0b11010101, 0b10111111,
        0b11011110, 0b11011110, 0b00111111, 0b10000000, 0b00000011, 0b11110101, 0b10101010, 0b01011101,
        0b10110101, 0b01101101, 0b11011111, 0b10000000, 0b00000011, 0b11110110, 0b01010101, 0b11101011,
        0b11001010, 0b10110110, 0b01101111, 0b10000000, 0b00000011, 0b11100111, 0b10010110, 0b11011111,
        0b10110111, 0b11011011, 0b10110111, 0b11111111, 0b11111111, 0b11111001, 0b01101001, 0b01010101,
        0b11101111, 0b00001101, 0b11011111, 0b11111111, 0b11111111, 0b11110111, 0b01111010, 0b10101011,
        0b11010110, 0b10111110, 0b11011111, 0b11111111, 0b11111111, 0b11110111, 0b01010111, 0b11101111,
        0b10101111, 0b10011010, 0b11001111, 0b10000000, 0b00000011, 0b11110110, 0b10100101, 0b01010101,
        0b11011010, 0b01011011, 0b01001111, 0b10000000, 0b00000011, 0b11101101, 0b10101011, 0b11111101,
        0b11110101, 0b01101111, 0b01001111, 0b10000000, 0b00000011, 0b11101101, 0b01100001, 0b11101011,
        0b10101101, 0b11101111, 0b01001111, 0b10000000, 0b00000011, 0b11101101, 0b01011101, 0b11001011,
        0b11010101, 0b10101101, 0b01101111, 0b11111111, 0b11111111, 0b11111101, 0b11011101, 0b10110111,
        0b11110110, 0b10101011, 0b01011111, 0b11111111, 0b11111111, 0b11110101, 0b11011001, 0b00111111,
        0b11101010, 0b10101010, 0b11011111, 0b11111111, 0b11111111, 0b11110110, 0b10110011, 0b01101011,
        0b11011010, 0b10111010, 0b10111111, 0b11111111, 0b11111111, 0b11111110, 0b10110110, 0b11011111,
        0b11101010, 0b10110110, 0b10101111, 0b11111111, 0b11111111, 0b11101010, 0b10110111, 0b11010111,
        0b11011010, 0b10100101, 0b01010111, 0b11111111, 0b11111111, 0b11001010, 0b10110110, 0b11001111,
        0b11110001, 0b11101110, 0b00100010, 0b11110001, 0b10111001, 0b01010101, 0b11011100, 0b11011111,
        0b11001101, 0b11101011, 0b00001101, 0b11001111, 0b01011010, 0b10110101, 0b11011101, 0b10011111,
        0b11111010, 0b11010101, 0b01110011, 0b01101001, 0b10101101, 0b11101011, 0b01011101, 0b10111111,
        0b11111101, 0b01101010, 0b11010101, 0b11111110, 0b11011010, 0b11011101, 0b01101101, 0b10111111,
        0b11110110, 0b11110111, 0b01100011, 0b00000011, 0b00100100, 0b10110111, 0b11101111, 0b00101111,
        0b11111001, 0b10011111, 0b01011100, 0b00000001, 0b10011011, 0b01011010, 0b11011110, 0b01111111,
        0b11110111, 0b11110111, 0b11110001, 0b11111100, 0b11100110, 0b10110111, 0b01101110, 0b11011111,
        0b11111101, 0b11111100, 0b11011111, 0b11011111, 0b00011011, 0b01101010, 0b11010010, 0b11111111,
        0b11111111, 0b10111011, 0b10111110, 0b11010000, 0b11001110, 0b11011100, 0b10101100, 0b10111111,
        0b11111101, 0b11110110, 0b11101101, 0b01111001, 0b00110001, 0b10110101, 0b01110101, 0b11111111,
        0b11111111, 0b11101101, 0b11011011, 0b11111111, 0b11100110, 0b11011111, 0b10111101, 0b01111111,
        0b11111111, 0b10110111, 0b10110110, 0b11111111, 0b00010011, 0b11101101, 0b01010010, 0b11111111,
        0b11111111, 0b11101111, 0b11101101, 0b11000000, 0b01011110, 0b10111111, 0b00111111, 0b11111111,
        0b11111110, 0b00111001, 0b10111101, 0b11111100, 0b00100110, 0b00011011, 0b00111111, 0b11111111,
        0b11111111, 0b10001110, 0b01110111, 0b11111111, 0b11100111, 0b11100000, 0b10000111, 0b11111111,
        0b11111111, 0b11110001, 0b11101110, 0b00000111, 0b11111000, 0b11111111, 0b11101111, 0b11111111,
        0b11111111, 0b11011110, 0b01111001, 0b10110000, 0b00111110, 0b01111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111011, 0b00111100, 0b01111111, 0b11101111, 0b11000001, 0b11111111, 0b11111111,
        0b11111111, 0b11011110, 0b11100111, 0b11011111, 0b11110001, 0b11110111, 0b11111111, 0b11111111,
        0b11111111, 0b11110111, 0b10111000, 0b11110000, 0b00001111, 0b00011111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11110111, 0b10011100, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11011100, 0b11110111, 0b10000001, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11100111, 0b11111100, 0b11111111, 0b11000001, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b00111101, 0b11011111, 0b11000001, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11001111, 0b11111000, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111000, 0b11111111, 0b10000000, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b11000000, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
};

const tImage  fingerprint_1BPP_UNCOMP=
{
     IMAGE_FMT_1BPP_UNCOMP,
     64,
     94,
     2,
     palette_wb,
     pixel_fingerprint_1BPP_UNCOMP,
};

static const uint8_t pixel_flag[] =
{
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xff, 0x3f, 0xc0, 0x0f, 0xf0, 0x3f, 0xc0,
        0xff, 0x3f, 0xc0, 0x0f, 0xf0, 0x3f, 0xc0,
        0xfc, 0xc0, 0x3f, 0xf0, 0x0f, 0xcf, 0xc0,
        0xfc, 0xc0, 0x3f, 0xf0, 0x0f, 0xcf, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xc0, 0x0f, 0xf0, 0x33, 0xc0,
        0xfc, 0xcf, 0xc0, 0x0f, 0xf0, 0x33, 0xc0,
        0xfc, 0xc0, 0x3f, 0xf0, 0x0f, 0xc3, 0xc0,
        0xfc, 0xc0, 0x3f, 0xf0, 0x0f, 0xc3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xf3, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xfc, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const tImage flag1 =
{
    IMAGE_FMT_1BPP_UNCOMP,
    50,
    50,
    2,
    palette_wb,
    pixel_flag,
};

static const uint8_t pixel_cloud1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xc0, 0x7f,
0xff, 0x80, 0x3f,
0xff, 0x80, 0x1f,
0xf1, 0x00, 0x0f,
0xc0, 0x00, 0x03,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xe0, 0x00, 0x07,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
};

const tImage  cloud =
{
    IMAGE_FMT_1BPP_UNCOMP,
    24,
    24,
    2,
    palette_wb,
    pixel_cloud1BPP_UNCOMP,
};

static const uint8_t pixel_heart1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0x3c, 0xff,
0xfe, 0x18, 0x7f,
0xfc, 0x00, 0x3f,
0xfc, 0x00, 0x3f,
0xfc, 0x00, 0x3f,
0xfc, 0x00, 0x3f,
0xfe, 0x00, 0x7f,
0xff, 0x00, 0xff,
0xff, 0x81, 0xff,
0xff, 0xc3, 0xff,
0xff, 0xe7, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff
};

const tImage  heart=
{
    IMAGE_FMT_1BPP_UNCOMP,
    24,
    24,
    2,
    palette_wb,
    pixel_heart1BPP_UNCOMP,
};

static const uint8_t pixel_idea1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xfe, 0xff, 0x7f,
0xff, 0x7e, 0xff,
0xff, 0xc3, 0xff,
0xff, 0xbd, 0xff,
0xff, 0x66, 0xff,
0xf9, 0x76, 0x9f,
0xff, 0x7e, 0xff,
0xff, 0x7e, 0xff,
0xff, 0xbd, 0xff,
0xfe, 0xbd, 0x7f,
0xfd, 0xc3, 0xbf,
0xff, 0xdb, 0xff,
0xff, 0xc3, 0xff,
0xff, 0xdb, 0xff,
0xff, 0xc3, 0xff,
0xff, 0xe7, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff
};

const tImage  idea =
{
    IMAGE_FMT_1BPP_UNCOMP,
    24,
    24,
    2,
    palette_wb,
    pixel_idea1BPP_UNCOMP,
};

static const uint8_t pixel_lightning1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xc0, 0x7f,
0xff, 0x80, 0x3f,
0xff, 0x80, 0x1f,
0xf1, 0x00, 0x0f,
0xc0, 0x00, 0x03,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xc0, 0x00, 0x01,
0xe0, 0x00, 0x07,
0xff, 0x81, 0xff,
0xff, 0x83, 0xff,
0xff, 0x07, 0xff,
0xff, 0x0f, 0xff,
0xff, 0xc3, 0xff,
0xff, 0xc7, 0xff,
0xff, 0x8f, 0xff,
0xff, 0x9f, 0xff,
0xff, 0xbf, 0xff
};

const tImage  lightning=
{
    IMAGE_FMT_1BPP_UNCOMP,
    24,
    24,
    2,
    palette_wb,
    pixel_lightning1BPP_UNCOMP,
};

static const uint8_t pixel_emptyheart1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0x3c, 0xff,
0xfe, 0x18, 0x7f,
0xfc, 0xc3, 0x3f,
0xfd, 0xff, 0xbf,
0xfd, 0xff, 0xbf,
0xfc, 0xff, 0x3f,
0xfe, 0x7e, 0x7f,
0xff, 0x3c, 0xff,
0xff, 0x99, 0xff,
0xff, 0xc3, 0xff,
0xff, 0xe7, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff
};

const tImage empty_heart =
{
    IMAGE_FMT_1BPP_UNCOMP,
    24,
    24,
    2,
    palette_wb,
    pixel_emptyheart1BPP_UNCOMP,
};

static const uint8_t pixel_check1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe1,
0xff, 0xff, 0xff, 0xe1,
0xfe, 0x00, 0x00, 0x07,
0xfe, 0x00, 0x00, 0x07,
0xfe, 0x7f, 0xfe, 0x1f,
0xfe, 0x7f, 0xfe, 0x1f,
0xfe, 0x7f, 0xf8, 0x7f,
0xfe, 0x7f, 0xf8, 0x7f,
0xfe, 0x67, 0xe1, 0xff,
0xfe, 0x67, 0xe1, 0xff,
0xfe, 0x61, 0x86, 0x7f,
0xfe, 0x61, 0x86, 0x7f,
0xfe, 0x78, 0x1e, 0x7f,
0xfe, 0x78, 0x1e, 0x7f,
0xfe, 0x7e, 0x7e, 0x7f,
0xfe, 0x7e, 0x7e, 0x7f,
0xfe, 0x7f, 0xfe, 0x7f,
0xfe, 0x7f, 0xfe, 0x7f,
0xfe, 0x00, 0x00, 0x7f,
0xfe, 0x00, 0x00, 0x7f,
0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff
};

const tImage  check=
{
    IMAGE_FMT_1BPP_UNCOMP,
    32,
    24,
    2,
    palette_wb,
    pixel_check1BPP_UNCOMP,
};

static const uint8_t pixel_nocheck1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff,
0xfe, 0x00, 0x00, 0x7f,
0xfe, 0x00, 0x00, 0x7f,
0xfe, 0x36, 0xda, 0x7f,
0xfe, 0x5b, 0x6c, 0x7f,
0xfe, 0x6d, 0xb6, 0x7f,
0xfe, 0x36, 0xda, 0x7f,
0xfe, 0x5b, 0x6c, 0x7f,
0xfe, 0x6d, 0xb6, 0x7f,
0xfe, 0x36, 0xda, 0x7f,
0xfe, 0x5b, 0x6c, 0x7f,
0xfe, 0x6d, 0xb6, 0x7f,
0xfe, 0x36, 0xda, 0x7f,
0xfe, 0x5b, 0x6c, 0x7f,
0xfe, 0x6d, 0xb6, 0x7f,
0xfe, 0x36, 0xda, 0x7f,
0xfe, 0x5b, 0x6c, 0x7f,
0xfe, 0x00, 0x00, 0x7f,
0xfe, 0x00, 0x00, 0x7f,
0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff
};

const tImage nocheck =
{
    IMAGE_FMT_1BPP_UNCOMP,
    32,
    24,
    2,
    palette_wb,
    pixel_nocheck1BPP_UNCOMP,
};

static const uint8_t pixel_jakethedog1BPP_UNCOMP[] =
{
0xff, 0xff, 0xff,
0xff, 0xff, 0xf0,
0xfe, 0x03, 0xf0,
0xf9, 0xfc, 0xf0,
0xf7, 0xff, 0x70,
0xef, 0xff, 0xb0,
0xec, 0x7e, 0x30,
0xd3, 0xbd, 0xd0,
0xd7, 0x81, 0x90,
0xb3, 0x22, 0x10,
0xb0, 0xff, 0x10,
0xb8, 0xf7, 0x50,
0xb4, 0xeb, 0x30,
0xce, 0xdd, 0x70,
0xfe, 0xdd, 0x70,
0xff, 0x3c, 0xf0,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
0xff, 0xff, 0xff,
};

const tImage jakethedog =
{
    IMAGE_FMT_1BPP_UNCOMP,
    20,
    20,
    2,
    palette_wb,
    pixel_jakethedog1BPP_UNCOMP,
};


#include "generated_images.h"

const qc12_anim_t *idle_anims[] = {
        &walking, &walking_left, &running, &running_left, &zombie, &zombie_left, &jump, &march, &exercise_1, &flap_arms, &silly, &dance, &wave_right
};
const uint8_t idle_anim_count = 13;

const qc12_anim_t *moody_idle_anims[] = {
        &bored_arms, &bored_crouch, &bored_wave, &bored_sad, &bored_yawn, &bored_talk_to_hand, &zombie, &zombie_left
};
const uint8_t moody_idle_anim_count = 8;

const qc12_anim_t *play_cause[] = {
        &trapdoor_creator,
        &earthquake_creator,
        &mimic_creator
};

const qc12_anim_t *play_effect[] = {
        &trapdoor_effect,
        &earthquake_effect,
        &mimic_effect
};

const qc12_anim_t *play_observe[] = {
        &trapdoor_creator_observer,
        &earthquake_creator_observer,
        &mimic_creator_observer
};

const uint8_t play_anim_count = 3;
