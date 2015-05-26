/* --COPYRIGHT--,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
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
// sysctl.h - Driver for the SYSCTL Module.
//
//*****************************************************************************

#ifndef __MSP430WARE_SYSCTL_H__
#define __MSP430WARE_SYSCTL_H__

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_SYS__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// The following are values that can be passed to the BSLRAMAssignment
// parameter for functions: SysCtl_setRAMAssignedToBSL().
//
//*****************************************************************************
#define SYSCTL_BSLRAMASSIGN_NORAM                                  (!(SYSBSLR))
#define SYSCTL_BSLRAMASSIGN_LOWEST16BYTES                             (SYSBSLR)

//*****************************************************************************
//
// The following are values that can be passed to the BSLSizeSelect parameter
// for functions: SysCtl_setBSLSize().
//
//*****************************************************************************
#define SYSCTL_BSLSIZE_SEG3                      (~(SYSBSLSIZE0 + SYSBSLSIZE1))
#define SYSCTL_BSLSIZE_SEGS23                                     (SYSBSLSIZE0)
#define SYSCTL_BSLSIZE_SEGS123                                    (SYSBSLSIZE1)
#define SYSCTL_BSLSIZE_SEGS1234                     (SYSBSLSIZE0 + SYSBSLSIZE1)

//*****************************************************************************
//
// The following are values that can be passed to the mailboxSizeSelect
// parameter for functions: SysCtl_JTAGMailboxInit().
//
//*****************************************************************************
#define SYSCTL_JTAGMBSIZE_16BIT                                    (!(JMBMODE))
#define SYSCTL_JTAGMBSIZE_32BIT                                       (JMBMODE)

//*****************************************************************************
//
// The following are values that can be passed to the autoClearInboxFlagSelect
// parameter for functions: SysCtl_JTAGMailboxInit().
//
//*****************************************************************************
#define SYSCTL_JTAGINBOX0AUTO_JTAGINBOX1AUTO       (!(JMBCLR0OFF + JMBCLR1OFF))
#define SYSCTL_JTAGINBOX0AUTO_JTAGINBOX1SW                         (JMBCLR1OFF)
#define SYSCTL_JTAGINBOX0SW_JTAGINBOX1AUTO                         (JMBCLR0OFF)
#define SYSCTL_JTAGINBOX0SW_JTAGINBOX1SW              (JMBCLR0OFF + JMBCLR1OFF)

//*****************************************************************************
//
// The following are values that can be passed to the mailboxFlagMask parameter
// for functions: SysCtl_getJTAGMailboxFlagStatus(), and
// SysCtl_clearJTAGMailboxFlagStatus().
//
//*****************************************************************************
#define SYSCTL_JTAGOUTBOX_FLAG0                                     (JMBOUT0FG)
#define SYSCTL_JTAGOUTBOX_FLAG1                                     (JMBOUT1FG)
#define SYSCTL_JTAGINBOX_FLAG0                                       (JMBIN0FG)
#define SYSCTL_JTAGINBOX_FLAG1                                       (JMBIN1FG)

//*****************************************************************************
//
// The following are values that can be passed to the inboxSelect parameter for
// functions: SysCtl_getJTAGInboxMessage16Bit().
//
//*****************************************************************************
#define SYSCTL_JTAGINBOX_0                                                (0x0)
#define SYSCTL_JTAGINBOX_1                                                (0x2)

//*****************************************************************************
//
// The following are values that can be passed to the outboxSelect parameter
// for functions: SysCtl_setJTAGOutgoingMessage16Bit().
//
//*****************************************************************************
#define SYSCTL_JTAGOUTBOX_0                                               (0x0)
#define SYSCTL_JTAGOUTBOX_1                                               (0x2)

//*****************************************************************************
//
// The following are values that can be passed toThe following are values that
// can be returned by the SysCtl_getBSLEntryIndication() function.
//
//*****************************************************************************
#define SYSCTL_BSLENTRY_INDICATED                                         (0x1)
#define SYSCTL_BSLENTRY_NOTINDICATED                                      (0x0)

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Sets the JTAG pins to be exclusively for JTAG until a BOR occurs.
//!
//! This function sets the JTAG pins to be exclusively used for the JTAG, and
//! not to be shared with the GPIO pins. This setting can only be cleared when
//! a BOR occurs.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_enableDedicatedJTAGPins(uint16_t baseAddress);

//*****************************************************************************
//
//! \brief Returns the indication of a BSL entry sequence from the Spy-Bi-Wire.
//!
//! This function returns the indication of a BSL entry sequence from the Spy-
//! Bi-Wire.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//!
//! \return One of the following:
//!         - \b SysCtl_BSLENTRY_INDICATED
//!         - \b SysCtl_BSLENTRY_NOTINDICATED
//!         \n indicating if a BSL entry sequence was detected
//
//*****************************************************************************
extern uint8_t SysCtl_getBSLEntryIndication(uint16_t baseAddress);

//*****************************************************************************
//
//! \brief Enables PMM Access Protection.
//!
//! This function enables the PMM Access Protection, which will lock any
//! changes on the PMM control registers until a BOR occurs.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_enablePMMAccessProtect(uint16_t baseAddress);

//*****************************************************************************
//
//! \brief Enables RAM-based Interrupt Vectors.
//!
//! This function enables RAM-base Interrupt Vectors, which means that
//! interrupt vectors are generated with the end address at the top of RAM,
//! instead of the top of the lower 64kB of flash.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_enableRAMBasedInterruptVectors(uint16_t baseAddress);

//*****************************************************************************
//
//! \brief Disables RAM-based Interrupt Vectors.
//!
//! This function disables the interrupt vectors from being generated at the
//! top of the RAM.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_disableRAMBasedInterruptVectors(uint16_t baseAddress);

//*****************************************************************************
//
//! \brief Initializes JTAG Mailbox with selected properties.
//!
//! This function sets the specified settings for the JTAG Mailbox system. The
//! settings that can be set are the size of the JTAG messages, and the auto-
//! clearing of the inbox flags. If the inbox flags are set to auto-clear, then
//! the inbox flags will be cleared upon reading of the inbox message buffer,
//! otherwise they will have to be reset by software using the
//! SYS_clearJTAGMailboxFlagStatus() function.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//! \param mailboxSizeSelect is the size of the JTAG Mailboxes, whether 16- or
//!        32-bits.
//!        Valid values are:
//!        - \b SYSCTL_JTAGMBSIZE_16BIT [Default] - the JTAG messages will take
//!           up only one JTAG mailbox (i. e. an outgoing message will take up
//!           only 1 outbox of the JTAG mailboxes)
//!        - \b SYSCTL_JTAGMBSIZE_32BIT - the JTAG messages will be contained
//!           within both JTAG mailboxes (i. e. an outgoing message will take
//!           up both Outboxes of the JTAG mailboxes)
//!        \n Modified bits are \b JMBMODE of \b SYSJMBC register.
//! \param autoClearInboxFlagSelect decides how the JTAG inbox flags should be
//!        cleared, whether automatically after the corresponding outbox has
//!        been written to, or manually by software.
//!        Valid values are:
//!        - \b SYSCTL_JTAGINBOX0AUTO_JTAGINBOX1AUTO [Default] - both JTAG
//!           inbox flags will be reset automatically when the corresponding
//!           inbox is read from.
//!        - \b SYSCTL_JTAGINBOX0AUTO_JTAGINBOX1SW - only JTAG inbox 0 flag is
//!           reset automatically, while JTAG inbox 1 is reset with the
//!        - \b SYSCTL_JTAGINBOX0SW_JTAGINBOX1AUTO - only JTAG inbox 1 flag is
//!           reset automatically, while JTAG inbox 0 is reset with the
//!        - \b SYSCTL_JTAGINBOX0SW_JTAGINBOX1SW - both JTAG inbox flags will
//!           need to be reset manually by the
//!        \n Modified bits are \b JMBCLR0OFF and \b JMBCLR1OFF of \b SYSJMBC
//!        register.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_JTAGMailboxInit(uint16_t baseAddress,
                                   uint8_t mailboxSizeSelect,
                                   uint8_t autoClearInboxFlagSelect);

//*****************************************************************************
//
//! \brief Returns the status of the selected JTAG Mailbox flags.
//!
//! This function will return the status of the selected JTAG Mailbox flags in
//! bit mask format matching that passed into the mailboxFlagMask parameter.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//! \param mailboxFlagMask is the bit mask of JTAG mailbox flags that the
//!        status of should be returned.
//!        Mask value is the logical OR of any of the following:
//!        - \b SYSCTL_JTAGOUTBOX_FLAG0 - flag for JTAG outbox 0
//!        - \b SYSCTL_JTAGOUTBOX_FLAG1 - flag for JTAG outbox 1
//!        - \b SYSCTL_JTAGINBOX_FLAG0 - flag for JTAG inbox 0
//!        - \b SYSCTL_JTAGINBOX_FLAG1 - flag for JTAG inbox 1
//!
//! \return A bit mask of the status of the selected mailbox flags.
//
//*****************************************************************************
extern uint8_t SysCtl_getJTAGMailboxFlagStatus(uint16_t baseAddress,
                                               uint8_t mailboxFlagMask);

//*****************************************************************************
//
//! \brief Clears the status of the selected JTAG Mailbox flags.
//!
//! This function clears the selected JTAG Mailbox flags.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//! \param mailboxFlagMask is the bit mask of JTAG mailbox flags that the
//!        status of should be cleared.
//!        Mask value is the logical OR of any of the following:
//!        - \b SYSCTL_JTAGOUTBOX_FLAG0 - flag for JTAG outbox 0
//!        - \b SYSCTL_JTAGOUTBOX_FLAG1 - flag for JTAG outbox 1
//!        - \b SYSCTL_JTAGINBOX_FLAG0 - flag for JTAG inbox 0
//!        - \b SYSCTL_JTAGINBOX_FLAG1 - flag for JTAG inbox 1
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_clearJTAGMailboxFlagStatus(uint16_t baseAddress,
                                              uint8_t mailboxFlagMask);

//*****************************************************************************
//
//! \brief Returns the contents of the selected JTAG Inbox in a 16 bit format.
//!
//! This function returns the message contents of the selected JTAG inbox. If
//! the auto clear settings for the Inbox flags were set, then using this
//! function will automatically clear the corresponding JTAG inbox flag.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//! \param inboxSelect is the chosen JTAG inbox that the contents of should be
//!        returned
//!        Valid values are:
//!        - \b SYSCTL_JTAGINBOX_0 - return contents of JTAG inbox 0
//!        - \b SYSCTL_JTAGINBOX_1 - return contents of JTAG inbox 1
//!
//! \return The contents of the selected JTAG inbox in a 16 bit format.
//
//*****************************************************************************
extern uint16_t SysCtl_getJTAGInboxMessage16Bit(uint16_t baseAddress,
                                                uint8_t inboxSelect);

//*****************************************************************************
//
//! \brief Returns the contents of JTAG Inboxes in a 32 bit format.
//!
//! This function returns the message contents of both JTAG inboxes in a 32 bit
//! format. This function should be used if 32-bit messaging has been set in
//! the SYS_JTAGMailboxInit() function. If the auto clear settings for the
//! Inbox flags were set, then using this function will automatically clear
//! both JTAG inbox flags.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//!
//! \return The contents of both JTAG messages in a 32 bit format.
//
//*****************************************************************************
extern uint32_t SysCtl_getJTAGInboxMessage32Bit(uint16_t baseAddress);

//*****************************************************************************
//
//! \brief Sets a 16 bit outgoing message in to the selected JTAG Outbox.
//!
//! This function sets the outgoing message in the selected JTAG outbox. The
//! corresponding JTAG outbox flag is cleared after this function, and set
//! after the JTAG has read the message.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//! \param outboxSelect is the chosen JTAG outbox that the message should be
//!        set it.
//!        Valid values are:
//!        - \b SYSCTL_JTAGOUTBOX_0 - set the contents of JTAG outbox 0
//!        - \b SYSCTL_JTAGOUTBOX_1 - set the contents of JTAG outbox 1
//! \param outgoingMessage is the message to send to the JTAG.
//!        \n Modified bits are \b MSGHI and \b MSGLO of \b SYSJMBOx register.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_setJTAGOutgoingMessage16Bit(uint16_t baseAddress,
                                               uint8_t outboxSelect,
                                               uint16_t outgoingMessage);

//*****************************************************************************
//
//! \brief Sets a 32 bit message in to both JTAG Outboxes.
//!
//! This function sets the 32-bit outgoing message in both JTAG outboxes. The
//! JTAG outbox flags are cleared after this function, and set after the JTAG
//! has read the message.
//!
//! \param baseAddress is the Base Address of the SYSCTL Module.
//! \param outgoingMessage is the message to send to the JTAG.
//!        \n Modified bits are \b MSGHI and \b MSGLO of \b SYSJMBOx register.
//!
//! \return None
//
//*****************************************************************************
extern void SysCtl_setJTAGOutgoingMessage32Bit(uint16_t baseAddress,
                                               uint32_t outgoingMessage);

//*****************************************************************************
//
// The SysCtl_enableBSLProtect API has been deprecated.
//
//*****************************************************************************
#ifndef DEPRECATED
extern void SysCtl_enableBSLProtect(uint16_t baseAddress);
#endif

//*****************************************************************************
//
// The SysCtl_disableBSLProtect API has been deprecated.
//
//*****************************************************************************
#ifndef DEPRECATED
extern void SysCtl_disableBSLProtect(uint16_t baseAddress);
#endif

//*****************************************************************************
//
// The SysCtl_enableBSLMemory API has been deprecated.
//
//*****************************************************************************
#ifndef DEPRECATED
extern void SysCtl_enableBSLMemory(uint16_t baseAddress);
#endif

//*****************************************************************************
//
// The SysCtl_disableBSLMemory API has been deprecated.
//
//*****************************************************************************
#ifndef DEPRECATED
extern void SysCtl_disableBSLMemory(uint16_t baseAddress);
#endif

//*****************************************************************************
//
// The SysCtl_setRAMAssignedToBSL API has been deprecated.
//
//*****************************************************************************
#ifndef DEPRECATED
extern void SysCtl_setRAMAssignedToBSL(uint16_t baseAddress,
                                       uint8_t BSLRAMAssignment);
#endif

//*****************************************************************************
//
// The SysCtl_setBSLSize API has been deprecated.
//
//*****************************************************************************
#ifndef DEPRECATED
extern void SysCtl_setBSLSize(uint16_t baseAddress,
                              uint8_t BSLSizeSelect);
#endif

//*****************************************************************************
//
// The following are deprecated values. These were updated to new names to add
// compatibility across Texas Instruments microcontrollers.
//
//*****************************************************************************
#ifndef DEPRECATED
#define SYS_BSLRAMASSIGN_NORAM                        SYSCTL_BSLRAMASSIGN_NORAM
#define SYS_BSLRAMASSIGN_LOWEST16BYTES        SYSCTL_BSLRAMASSIGN_LOWEST16BYTES
#define SYS_BSLSIZE_SEG3                                    SYSCTL_BSLSIZE_SEG3
#define SYS_BSLSIZE_SEGS23                                SYSCTL_BSLSIZE_SEGS23
#define SYS_BSLSIZE_SEGS123                              SYSCTL_BSLSIZE_SEGS123
#define SYS_BSLSIZE_SEGS1234                            SYSCTL_BSLSIZE_SEGS1234
#define SYS_JTAGMBSIZE_16BIT                            SYSCTL_JTAGMBSIZE_16BIT
#define SYS_JTAGMBSIZE_32BIT                            SYSCTL_JTAGMBSIZE_32BIT
#define SYS_JTAGINBOX0AUTO_JTAGINBOX1AUTO  SYSCTL_JTAGINBOX0AUTO_JTAGINBOX1AUTO
#define SYS_JTAGINBOX0AUTO_JTAGINBOX1SW      SYSCTL_JTAGINBOX0AUTO_JTAGINBOX1SW
#define SYS_JTAGINBOX0SW_JTAGINBOX1AUTO      SYSCTL_JTAGINBOX0SW_JTAGINBOX1AUTO
#define SYS_JTAGINBOX0SW_JTAGINBOX1SW          SYSCTL_JTAGINBOX0SW_JTAGINBOX1SW
#define SYS_JTAGOUTBOX_FLAG0                            SYSCTL_JTAGOUTBOX_FLAG0
#define SYS_JTAGOUTBOX_FLAG1                            SYSCTL_JTAGOUTBOX_FLAG1
#define SYS_JTAGINBOX_FLAG0                              SYSCTL_JTAGINBOX_FLAG0
#define SYS_JTAGINBOX_FLAG1                              SYSCTL_JTAGINBOX_FLAG1
#define SYS_JTAGINBOX_0                                      SYSCTL_JTAGINBOX_0
#define SYS_JTAGINBOX_1                                      SYSCTL_JTAGINBOX_1
#define SYS_JTAGOUTBOX_0                                    SYSCTL_JTAGOUTBOX_0
#define SYS_JTAGOUTBOX_1                                    SYSCTL_JTAGOUTBOX_1
#define SYS_BSLENTRY_INDICATED                        SYSCTL_BSLENTRY_INDICATED
#define SYS_BSLENTRY_NOTINDICATED                  SYSCTL_BSLENTRY_NOTINDICATED
#endif

//*****************************************************************************
//
// The following are deprecated API names. These were updated to new names to
// add compatibility across Texas Instruments microcontrollers.
//
//*****************************************************************************
#ifndef DEPRECATED
#define SYS_enableBSLProtect                            SysCtl_enableBSLProtect
#define SYS_disableBSLProtect                          SysCtl_disableBSLProtect
#define SYS_enableBSLMemory                              SysCtl_enableBSLMemory
#define SYS_disableBSLMemory                            SysCtl_disableBSLMemory
#define SYS_setRAMAssignedToBSL                      SysCtl_setRAMAssignedToBSL
#define SYS_setBSLSize                                        SysCtl_setBSLSize
#define SYS_enableDedicatedJTAGPins              SysCtl_enableDedicatedJTAGPins
#define SYS_getBSLEntryIndication                  SysCtl_getBSLEntryIndication
#define SYS_enablePMMAccessProtect                SysCtl_enablePMMAccessProtect
#define SYS_enableRAMBasedInterruptVectors                                    \
                                          SysCtl_enableRAMBasedInterruptVectors
#define SYS_disableRAMBasedInterruptVectors                                   \
                                         SysCtl_disableRAMBasedInterruptVectors
#define SYS_JTAGMailboxInit                              SysCtl_JTAGMailboxInit
#define SYS_getJTAGMailboxFlagStatus            SysCtl_getJTAGMailboxFlagStatus
#define SYS_clearJTAGMailboxFlagStatus        SysCtl_clearJTAGMailboxFlagStatus
#define SYS_getJTAGInboxMessage16Bit            SysCtl_getJTAGInboxMessage16Bit
#define SYS_getJTAGInboxMessage32Bit            SysCtl_getJTAGInboxMessage32Bit
#define SYS_setJTAGOutgoingMessage16Bit      SysCtl_setJTAGOutgoingMessage16Bit
#define SYS_setJTAGOutgoingMessage32Bit      SysCtl_setJTAGOutgoingMessage32Bit
#endif

//*****************************************************************************
//
// The following are deprecated struct names. These were updated to new names
// to add compatibility across Texas Instruments microcontrollers.
//
//*****************************************************************************
#ifndef DEPRECATED
#endif

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif
#endif // __MSP430WARE_SYSCTL_H__
