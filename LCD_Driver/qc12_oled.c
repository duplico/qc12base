/* --COPYRIGHT--,BSD
 * Copyright (c) 2013, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
//
//! \addtogroup display_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// READ ME
//
// This template driver is intended to be modified for creating new LCD drivers
// It is setup so that only qc12_oledPixelDraw() and DPYCOLORTRANSLATE()
// and some LCD size configuration settings in the header file qc12_oled.h
// are REQUIRED to be written. These functions are marked with the string
// "TemplateDisplayFix" in the comments so that a search through qc12_oled.c and
// qc12_oled.h can quickly identify the necessary areas of change.
//
// qc12_oledPixelDraw() is the base function to write to the LCD
// display. Functions like WriteData(), WriteCommand(), and SetAddress()
// are suggested to be used to help implement the qc12_oledPixelDraw()
// function, but are not required. SetAddress() should be used by other pixel
// level functions to help optimize them.
// 
// This is not an optimized driver however and will significantly impact 
// performance. It is highly recommended to first get the prototypes working
// with the single pixel writes, and then go back and optimize the driver.
// Please see application note www.ti.com/lit/pdf/slaa548 for more information
// on how to fully optimize LCD driver files. In int16_t, driver optimizations
// should take advantage of the auto-incrementing of the LCD controller. 
// This should be utilized so that a loop of WriteData() can be used instead
// of a loop of qc12_oledPixelDraw(). The pixel draw loop contains both a
// SetAddress() + WriteData() compared to WriteData() alone. This is a big time 
// saver especially for the line draws and qc12_oledPixelDrawMultiple.
// More optimization can be done by reducing function calls by writing macros,
// eliminating unnecessary instructions, and of course taking advantage of other 
// features offered by the LCD controller. With so many pixels on an LCD screen
// each instruction can have a large impact on total drawing time.
//
//*****************************************************************************


//*****************************************************************************
//
// Include Files
//
//*****************************************************************************
#include <msp430.h>
#include "grlib.h"
#include "qc12_oled.h"
#include <driverlib/MSP430FR5xx_6xx/driverlib.h>
#include <stdint.h>

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

#pragma DATA_SECTION (oled_memory, ".ramdata");

/* Global buffer for the display. This is especially useful on 1BPP, 2BPP, and 4BPP
  displays. This allows you to update pixels while reading the neighboring pixels
  from the buffer instead of a read from the LCD controller. A buffer is not required
  as a read followed by a write can be used instead.*/
uint8_t oled_memory[LCD_X_SIZE*PAGES]; // TODO???
//                        (LCD_X_SIZE * LCD_Y_SIZE * BPP + 7) / 8];

//*****************************************************************************
//
// Suggested functions to help facilitate writing the required functions below
//
//*****************************************************************************

#define RESPORT     GPIO_PORT_P2
#define RESPIN      GPIO_PIN6

#define DCPORT      GPIO_PORT_P2
#define DCPIN       GPIO_PIN7

#define THISISDATA  GPIO_setOutputHighOnPin(DCPORT, DCPIN)
#define THISISCMD   GPIO_setOutputLowOnPin(DCPORT, DCPIN);

#define GRAM_BUFFER(page, column) oled_memory[(((PAGES-1)-page) * LCD_X_SIZE) + column]

uint8_t ok_to_send = 1;

// Writes a command to the LCD controller
static void
WriteCommand(uint8_t ucCommand)
{
	while (!ok_to_send);
    THISISCMD;
	ok_to_send = 0;
	EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, ucCommand);
}

const uint8_t zero_address_cmds[] = {
        0x20,
        0x00, // horizontal addressing mode

        0x21,
        0x00, // starting column
        LCD_X_SIZE-1, // ending column

        0x22,
        0x00, // starting page
        PAGES-1 // ending page
};

// Zeros the pixel address of the LCD driver
void ZeroAddress()
{
    for (uint8_t i=0; i<8; i++) {
        WriteCommand(zero_address_cmds[i]);
    }
}


// Initializes the pins required for the GPIO-based LCD interface.
// This function configures the GPIO pins used to control the LCD display
// when the basic GPIO interface is in use. On exit, the LCD controller
// has been reset and is ready to receive command and data writes.
//static void
//InitGPIOLCDInterface(void)
//{
//    /* Initialize the hardware to configure the ports properly for communication */
//} // (Covered by Grace's init function)

// Initialize DisplayBuffer.
// This function initializes the display buffer and discards any cached data.
static void
InitLCDDisplayBuffer(void *pvDisplayData, uint16_t ulValue)
{

	// OK, what I want in this buffer is:
	// LCD_X_SIZEx64 - 8 pages, each 8 bits high and LCD_X_SIZE elements across
	// So our buffer[0..LCD_X_SIZE-1] will cover the top LCD_X_SIZEx8 of the screen.

	uint16_t page=0,col=0;
	for (page=0; page<LCD_Y_SIZE/PAGES; page++) { // for each row... (j = [0..8))
		for (col=0; col < LCD_X_SIZE; col++) { // fill the columns (i = [0..LCD_X_SIZE-1)):
			GRAM_BUFFER(page, col) = ulValue;
		}
	}
}

// Initializes the display driver.
// This function initializes the LCD controller
// TemplateDisplayFix
void
qc12_oledInit(void)
{
    InitLCDDisplayBuffer(0, 0);

    const char SSD1306_init[] = {
            0xAE, 		// Display off
            0xD5, 0x80, // Clock divide / oscillator
            0xA8, 0x3F, // Multiplex ratio
            0xd3, 0x00, // Display offset
            0x40,		// Start line
            0x8d, 0x14, // Charge pump
            0xa1,		// Segment re-map
            //		0xc8,		// COM output scan direction
            0xda, 0x12,	// COM pins hardware configuration
            0x81, 0x10, // Contrast control
            0xD9, 0xF1, // Pre-charge period
            0xDB, 0x40, // V_COMH deselect level
            0xA4,		// Entire display on/off (A5/A4)
            0xA6,		// Normal/inverse display
            0x20, 0x00, // Horizontal addressing mode.
            // CLEAR SCREEN (according to screen datasheet)
            0xAF,		// Display ON!
    };

    EUSCI_A_SPI_clearInterrupt(EUSCI_A1_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    THISISCMD;
    //	OLED_RES_LOW;
    GPIO_setOutputLowOnPin(RESPORT, RESPIN);
    //	// Delay for 200ms at 16Mhz
    __delay_cycles(1000);
    //	OLED_RES_HIGH;
    GPIO_setOutputHighOnPin(RESPORT, RESPIN);
    __delay_cycles(1000);
    THISISDATA;

    for (uint8_t i=0; i<sizeof SSD1306_init; i++) {
        WriteCommand(SSD1306_init[i]);
    }

    while (EUSCI_A_SPI_isBusy(EUSCI_A1_BASE));
}




//*****************************************************************************
//
// All the following functions (below) for the LCD driver are required by grlib
//
//*****************************************************************************

//*****************************************************************************
//
//! Draws a pixel on the screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the pixel.
//! \param lY is the Y coordinate of the pixel.
//! \param ulValue is the color of the pixel.
//!
//! This function sets the given pixel to a particular color.  The coordinates
//! of the pixel are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
// TemplateDisplayFix
static void
qc12_oledPixelDraw(void *pvDisplayData, int16_t lX, int16_t lY,
                                   uint16_t ulValue)
{  
  /* This function already has checked that the pixel is within the extents of  
  the LCD screen and the color ulValue has already been translated to the LCD.
  */

	uint16_t mapped_x = MAPPED_X(lX, lY);
	uint16_t mapped_y = MAPPED_Y(lX, lY);

	// Our COLUMN NUMBER is just x.
	// Our PAGE NUMBER is y/8
	// This is our ROW VALUE (by shifting 0b10000000 >> by row number):
	uint8_t val = 0x80 >> mapped_y % 8;

	// clear pixel
	GRAM_BUFFER(mapped_y/8, mapped_x) &= ~val;
	// write pixel // if needed
	if (ulValue) { // && !(GRAM_BUFFER(lY/8, lX) & val)) {
		GRAM_BUFFER(mapped_y/8, mapped_x) |= val;
	}
}

//*****************************************************************************
//
//! Draws a horizontal sequence of pixels on the screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the first pixel.
//! \param lY is the Y coordinate of the first pixel.
//! \param lX0 is sub-pixel offset within the pixel data, which is valid for 1
//! or 4 bit per pixel formats.
//! \param lCount is the number of pixels to draw.
//! \param lBPP is the number of bits per pixel; must be 1, 4, or 8.
//! \param pucData is a pointer to the pixel data.  For 1 and 4 bit per pixel
//! formats, the most significant bit(s) represent the left-most pixel.
//! \param pucPalette is a pointer to the palette used to draw the pixels.
//!
//! This function draws a horizontal sequence of pixels on the screen, using
//! the supplied palette.  For 1 bit per pixel format, the palette contains
//! pre-translated colors; for 4 and 8 bit per pixel formats, the palette
//! contains 24-bit RGB values that must be translated before being written to
//! the display.
//!
//! \return None.
//
//*****************************************************************************
static void
qc12_oledPixelDrawMultiple(void *pvDisplayData, int16_t lX,
                                           int16_t lY, int16_t lX0, int16_t lCount,
                                           int16_t lBPP,
                                           const uint8_t *pucData,
                                           const uint16_t *pucPalette)
{
	uint16_t ulByte;

    //
    // Determine how to interpret the pixel data based on the number of bits
    // per pixel.
    //
    switch(lBPP)
    {
        // The pixel data is in 1 bit per pixel format
        case 1:
        {
            // Loop while there are more pixels to draw
            while(lCount > 0)
            {
                // Get the next byte of image data
                ulByte = *pucData++;

                // Loop through the pixels in this byte of image data
                for(; (lX0 < 8) && lCount; lX0++, lCount--)
                {
                    // Draw this pixel in the appropriate color
					qc12_oledPixelDraw(pvDisplayData, lX++, lY, 
											((uint16_t *)pucPalette)[(ulByte >> (7 - lX0)) & 1]);
                }
                
                // Start at the beginning of the next byte of image data
                lX0 = 0;
            }
            // The image data has been drawn
            
            break;
        }

		// The pixel data is in 2 bit per pixel format
        case 2:
        {
            // Loop while there are more pixels to draw
            while(lCount > 0)
            {
                // Get the next byte of image data
                ulByte = *pucData++;

                // Loop through the pixels in this byte of image data
                for(; (lX0 < 4) && lCount; lX0++, lCount--)
                {
                    // Draw this pixel in the appropriate color
					qc12_oledPixelDraw(pvDisplayData, lX++, lY, 
											((uint16_t *)pucPalette)[(ulByte >> (6 - (lX0 << 1))) & 3]);
                }
                
                // Start at the beginning of the next byte of image data
                lX0 = 0;
            }
            // The image data has been drawn
            
            break;
		}
        // The pixel data is in 4 bit per pixel format
        case 4:
        {
            // Loop while there are more pixels to draw.  "Duff's device" is
            // used to jump into the middle of the loop if the first nibble of
            // the pixel data should not be used.  Duff's device makes use of
            // the fact that a case statement is legal anywhere within a
            // sub-block of a switch statement.  See
            // http://en.wikipedia.org/wiki/Duff's_device for detailed
            // information about Duff's device.
            switch(lX0 & 1)
            {
                case 0:
                  
                    while(lCount)
                    {
                        // Get the upper nibble of the next byte of pixel data
                        // and extract the corresponding entry from the palette
                        ulByte = (*pucData >> 4);    
                        ulByte = (*(uint16_t *)(pucPalette + ulByte));
                        // Write to LCD screen
                        qc12_oledPixelDraw(pvDisplayData, lX++, lY, ulByte);
                        
                        // Decrement the count of pixels to draw
                        lCount--;
                        
                        // See if there is another pixel to draw
                        if(lCount)
                        {
                case 1:
                            // Get the lower nibble of the next byte of pixel
                            // data and extract the corresponding entry from
                            // the palette
                            ulByte = (*pucData++ & 15);
                            ulByte = (*(uint16_t *)(pucPalette + ulByte));
                            // Write to LCD screen
                            qc12_oledPixelDraw(pvDisplayData, lX++, lY, ulByte);

                            // Decrement the count of pixels to draw
                            lCount--;
                        }
                    }
            }
            // The image data has been drawn.
           
            break;
        }

        // The pixel data is in 8 bit per pixel format
        case 8:
        {
            // Loop while there are more pixels to draw
            while(lCount--)
            {
                // Get the next byte of pixel data and extract the
                // corresponding entry from the palette
                ulByte = *pucData++;
                ulByte = (*(uint16_t *)(pucPalette + ulByte));
                // Write to LCD screen
                qc12_oledPixelDraw(pvDisplayData, lX++, lY, ulByte);
            }
            // The image data has been drawn
            break;
        }
    }
}

//*****************************************************************************
//
//! Draws a horizontal line.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX1 is the X coordinate of the start of the line.
//! \param lX2 is the X coordinate of the end of the line.
//! \param lY is the Y coordinate of the line.
//! \param ulValue is the color of the line.
//!
//! This function draws a horizontal line on the display.  The coordinates of
//! the line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
qc12_oledLineDrawH(void *pvDisplayData, int16_t lX1, int16_t lX2,
                                   int16_t lY, uint16_t ulValue)
{
  /* Ideally this function shouldn't call pixel draw. It should have it's own
  definition using the built in auto-incrementing of the LCD controller and its 
  own calls to SetAddress() and WriteData(). Better yet, SetAddress() and WriteData()
  can be made into macros as well to eliminate function call overhead. */
  
  do
  {
    qc12_oledPixelDraw(pvDisplayData, lX1, lY, ulValue);
  }
  while(lX1++ < lX2);
}

//*****************************************************************************
//
//! Draws a vertical line.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the line.
//! \param lY1 is the Y coordinate of the start of the line.
//! \param lY2 is the Y coordinate of the end of the line.
//! \param ulValue is the color of the line.
//!
//! This function draws a vertical line on the display.  The coordinates of the
//! line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
qc12_oledLineDrawV(void *pvDisplayData, int16_t lX, int16_t lY1,
                                   int16_t lY2, uint16_t ulValue)
{
  do
  {
    qc12_oledPixelDraw(pvDisplayData, lX, lY1, ulValue);
  }
  while(lY1++ < lY2);
}

//*****************************************************************************
//
//! Fills a rectangle.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param pRect is a pointer to the structure describing the rectangle.
//! \param ulValue is the color of the rectangle.
//!
//! This function fills a rectangle on the display.  The coordinates of the
//! rectangle are assumed to be within the extents of the display, and the
//! rectangle specification is fully inclusive (in other words, both sXMin and
//! sXMax are drawn, along with sYMin and sYMax).
//!
//! \return None.
//
//*****************************************************************************
static void
qc12_oledRectFill(void *pvDisplayData, const tRectangle *pRect,
                                  uint16_t ulValue)
{
  int16_t x0 = pRect->sXMin;
  int16_t x1 = pRect->sXMax;
  int16_t y0 = pRect->sYMin;
  int16_t y1 = pRect->sYMax;
  
  while(y0++ <= y1)
  {
    qc12_oledLineDrawH(pvDisplayData, x0, x1, y0, ulValue);
  }
}

//*****************************************************************************
//
//! Translates a 24-bit RGB color to a display driver-specific color.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param ulValue is the 24-bit RGB color.  The least-significant byte is the
//! blue channel, the next byte is the green channel, and the third byte is the
//! red channel.
//!
//! This function translates a 24-bit RGB color into a value that can be
//! written into the display's frame buffer in order to reproduce that color,
//! or the closest possible approximation of that color.
//!
//! \return Returns the display-driver specific color.
//
//*****************************************************************************
static uint16_t
qc12_oledColorTranslate(void *pvDisplayData,
                                        uint32_t  ulValue)
{
	/* The DPYCOLORTRANSLATE macro should be defined in TemplateDriver.h */
	
    //
    // Translate from a 24-bit RGB color to a color accepted by the LCD.
    //
    return(DPYCOLORTRANSLATE(ulValue));
}

#define OLED_STATE_IDLE 0
#define OLED_STATE_ZEROING 1
#define OLED_STATE_DATA 2

uint8_t oled_state = OLED_STATE_IDLE;
uint8_t zeroing_index = 0;

uint16_t writing_data = 0;

//*****************************************************************************
//
//! Flushes any cached drawing operations.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//!
//! This functions flushes any cached drawing operations to the display.  This
//! is useful when a local frame buffer is used for drawing operations, and the
//! flush would copy the local frame buffer to the display.
//!
//! \return None.
//
//*****************************************************************************
static void
qc12_oledFlush(void *pvDisplayData)
{
  // Flush Buffer here. This function is not needed if a buffer is not used,
  // or if the buffer is always updated with the screen writes.
    oled_state = OLED_STATE_ZEROING;
    THISISCMD;
    writing_data = 0;
    zeroing_index = 1;
    EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, zero_address_cmds[0]);

//	for (uint16_t i=0; i<LCD_X_SIZE*8; i++) { // TODO
//		WriteData(oled_memory[i]);
//	}


//	int16_t i=0,j=0;
//	for(i =0; i< LCD_Y_SIZE; i++)
//	for(j =0; j< (LCD_X_SIZE * BPP + 7) / 8; j++)
//		qc12_oledPixelDraw(pvDisplayData, j, i, Template_Memory[i * LCD_Y_SIZE + j]);
}

#pragma vector=USCI_A1_VECTOR
__interrupt void EUSCI_A1_ISR(void)
{
    switch (__even_in_range(UCA1IV, 4)) {
    //Vector 2 - RXIFG
    case 2:
        // The OLED display can't talk to us. It has no outputs.
        // TODO: make sure this never happens please.
        // We received some garbage sent to us while we were sending.
        EUSCI_B_SPI_receiveData(EUSCI_A1_BASE); // Throw it away.
        break; // End of RXIFG ///////////////////////////////////////////////////////

    case 4: // Vector 4 - TXIFG : I just sent a byte.
        if (oled_state == OLED_STATE_DATA) {
            if (writing_data == LCD_X_SIZE*8) {
                writing_data = 0;
                oled_state = OLED_STATE_IDLE;
                ok_to_send = 1;
                // done with the data.
            }
            else if (writing_data) { // still more data to send:
                EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, oled_memory[writing_data]);
                writing_data++;
            }
        } else if (oled_state == OLED_STATE_ZEROING) {
            if (zeroing_index == 8) { // Done zeroing... // TODO: Demeter
                oled_state = OLED_STATE_DATA;
                THISISDATA;
                EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, oled_memory[0]);
                writing_data = 1;
            } else {
                EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, zero_address_cmds[zeroing_index]);
                zeroing_index++;
            }
        } else { // OLED_STATE_IDLE
            ok_to_send = 1;
        }

        break; // End of TXIFG /////////////////////////////////////////////////////

    default: break;
    } // End of ISR flag switch ////////////////////////////////////////////////////
}


//*****************************************************************************
//
//! Send command to clear screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//!
//! This function does a clear screen and the Display Buffer contents
//! are initialized to the current background color.
//!
//! \return None.
//
//*****************************************************************************
static void
qc12_oledClearScreen (void *pvDisplayData, uint16_t ulValue)
{
	// This fills the entire display to clear it
	// Some LCD drivers support a simple command to clear the display
//	while(y0++ <= (LCD_Y_SIZE - 1))
	uint8_t val = ulValue? 0xff:0;
	for (uint16_t i=0; i<LCD_X_SIZE*8; i++)
		oled_memory[i] = val;
//	for (uint8_t y0=0; y0<LCD_Y_SIZE; y0++)
//	{
//		qc12_oledLineDrawH(pvDisplayData, 0, LCD_X_SIZE - 1, y0, ulValue);
//	}
}

//*****************************************************************************
//
//! The display structure that describes the driver for the blank template.
//
//*****************************************************************************
const tDisplay g_sqc12_oled =
{
    sizeof(tDisplay),
    oled_memory,
#if defined(PORTRAIT) || defined(PORTRAIT_FLIP)
    LCD_Y_SIZE,
    LCD_X_SIZE,
#else
    LCD_X_SIZE,
    LCD_Y_SIZE,
#endif
    qc12_oledPixelDraw,
    qc12_oledPixelDrawMultiple,
    qc12_oledLineDrawH,
    qc12_oledLineDrawV,
    qc12_oledRectFill,
    qc12_oledColorTranslate,
    qc12_oledFlush,
    qc12_oledClearScreen
};

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
