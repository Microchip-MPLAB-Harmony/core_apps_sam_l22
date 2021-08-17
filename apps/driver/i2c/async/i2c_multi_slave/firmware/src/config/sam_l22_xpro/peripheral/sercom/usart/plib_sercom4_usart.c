/*******************************************************************************
  SERCOM Universal Synchronous/Asynchrnous Receiver/Transmitter PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_sercom4_usart.c

  Summary
    USART peripheral library interface.

  Description
    This file defines the interface to the USART peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:
    None.
*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "interrupts.h"
#include "plib_sercom4_usart.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************


/* SERCOM4 USART baud value for 115200 Hz baud rate */
#define SERCOM4_USART_INT_BAUD_VALUE            (61761UL)

static SERCOM_USART_RING_BUFFER_OBJECT sercom4USARTObj;

// *****************************************************************************
// *****************************************************************************
// Section: SERCOM4 USART Interface Routines
// *****************************************************************************
// *****************************************************************************

#define SERCOM4_USART_READ_BUFFER_SIZE      128
#define SERCOM4_USART_READ_BUFFER_9BIT_SIZE     (128 >> 1)
#define SERCOM4_USART_RX_INT_DISABLE()      SERCOM4_REGS->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_RXC_Msk
#define SERCOM4_USART_RX_INT_ENABLE()       SERCOM4_REGS->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RXC_Msk

static uint8_t SERCOM4_USART_ReadBuffer[SERCOM4_USART_READ_BUFFER_SIZE];

#define SERCOM4_USART_WRITE_BUFFER_SIZE     256
#define SERCOM4_USART_WRITE_BUFFER_9BIT_SIZE  (256 >> 1)
#define SERCOM4_USART_TX_INT_DISABLE()      SERCOM4_REGS->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_DRE_Msk
#define SERCOM4_USART_TX_INT_ENABLE()       SERCOM4_REGS->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_DRE_Msk

static uint8_t SERCOM4_USART_WriteBuffer[SERCOM4_USART_WRITE_BUFFER_SIZE];

void SERCOM4_USART_Initialize( void )
{
    /*
     * Configures USART Clock Mode
     * Configures TXPO and RXPO
     * Configures Data Order
     * Configures Standby Mode
     * Configures Sampling rate
     * Configures IBON
     */
    SERCOM4_REGS->USART_INT.SERCOM_CTRLA = SERCOM_USART_INT_CTRLA_MODE_USART_INT_CLK | SERCOM_USART_INT_CTRLA_RXPO(0x3UL) | SERCOM_USART_INT_CTRLA_TXPO(0x1UL) | SERCOM_USART_INT_CTRLA_DORD_Msk | SERCOM_USART_INT_CTRLA_IBON_Msk | SERCOM_USART_INT_CTRLA_FORM(0x0UL) | SERCOM_USART_INT_CTRLA_SAMPR(0UL) ;

    /* Configure Baud Rate */
    SERCOM4_REGS->USART_INT.SERCOM_BAUD = (uint16_t)SERCOM_USART_INT_BAUD_BAUD(SERCOM4_USART_INT_BAUD_VALUE);

    /*
     * Configures RXEN
     * Configures TXEN
     * Configures CHSIZE
     * Configures Parity
     * Configures Stop bits
     */
    SERCOM4_REGS->USART_INT.SERCOM_CTRLB = SERCOM_USART_INT_CTRLB_CHSIZE_8_BIT | SERCOM_USART_INT_CTRLB_SBMODE_1_BIT | SERCOM_USART_INT_CTRLB_RXEN_Msk | SERCOM_USART_INT_CTRLB_TXEN_Msk;

    /* Wait for sync */
    while((SERCOM4_REGS->USART_INT.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }


    /* Enable the UART after the configurations */
    SERCOM4_REGS->USART_INT.SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;

    /* Wait for sync */
    while((SERCOM4_REGS->USART_INT.SERCOM_SYNCBUSY) != 0U)
    {
        /* Do nothing */
    }

    /* Initialize instance object */
    sercom4USARTObj.rdCallback = NULL;
    sercom4USARTObj.rdInIndex = 0U;
    sercom4USARTObj.rdOutIndex = 0U;
    sercom4USARTObj.isRdNotificationEnabled = false;
    sercom4USARTObj.isRdNotifyPersistently = false;
    sercom4USARTObj.rdThreshold = 0U;
    sercom4USARTObj.errorStatus = USART_ERROR_NONE;
    sercom4USARTObj.wrCallback = NULL;
    sercom4USARTObj.wrInIndex = 0U;
    sercom4USARTObj.wrOutIndex = 0U;
    sercom4USARTObj.isWrNotificationEnabled = false;
    sercom4USARTObj.isWrNotifyPersistently = false;
    sercom4USARTObj.wrThreshold = 0U;
    if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
    {
        sercom4USARTObj.rdBufferSize = SERCOM4_USART_READ_BUFFER_SIZE;
        sercom4USARTObj.wrBufferSize = SERCOM4_USART_WRITE_BUFFER_SIZE;
    }
    else
    {
        sercom4USARTObj.rdBufferSize = SERCOM4_USART_READ_BUFFER_9BIT_SIZE;
        sercom4USARTObj.wrBufferSize = SERCOM4_USART_WRITE_BUFFER_9BIT_SIZE;
    }
    /* Enable error interrupt */
    SERCOM4_REGS->USART_INT.SERCOM_INTENSET = (uint8_t)SERCOM_USART_INT_INTENSET_ERROR_Msk;

    /* Enable Receive Complete interrupt */
    SERCOM4_REGS->USART_INT.SERCOM_INTENSET = (uint8_t)SERCOM_USART_INT_INTENSET_RXC_Msk;
}

uint32_t SERCOM4_USART_FrequencyGet( void )
{
    return 32000000UL;
}

bool SERCOM4_USART_SerialSetup( USART_SERIAL_SETUP * serialSetup, uint32_t clkFrequency )
{
    bool setupStatus       = false;
    uint32_t baudValue     = 0U;
    uint32_t sampleRate    = 0U;

    if((serialSetup != NULL) && (serialSetup->baudRate != 0U))
    {
        if(clkFrequency == 0U)
        {
            clkFrequency = SERCOM4_USART_FrequencyGet();
        }

        if(clkFrequency >= (16U * serialSetup->baudRate))
        {
            baudValue = 65536U - (uint32_t)(((uint64_t)65536U * 16U * serialSetup->baudRate) / clkFrequency);
            sampleRate = 0U;
        }
        else if(clkFrequency >= (8U * serialSetup->baudRate))
        {
            baudValue = 65536U - (uint32_t)(((uint64_t)65536U * 8U * serialSetup->baudRate) / clkFrequency);
            sampleRate = 2U;
        }
        else if(clkFrequency >= (3U * serialSetup->baudRate))
        {
            baudValue = 65536U - (uint32_t)(((uint64_t)65536U * 3U * serialSetup->baudRate) / clkFrequency);
            sampleRate = 4U;
        }
        else
        {
            /* Do nothing */
        }

        if(baudValue != 0U)
        {
            /* Disable the USART before configurations */
            SERCOM4_REGS->USART_INT.SERCOM_CTRLA &= ~SERCOM_USART_INT_CTRLA_ENABLE_Msk;

            /* Wait for sync */
            while((SERCOM4_REGS->USART_INT.SERCOM_SYNCBUSY) != 0U)
            {
                /* Do nothing */
            }

            /* Configure Baud Rate */
            SERCOM4_REGS->USART_INT.SERCOM_BAUD = (uint16_t)SERCOM_USART_INT_BAUD_BAUD(baudValue);

            /* Configure Parity Options */
            if(serialSetup->parity == USART_PARITY_NONE)
            {
                SERCOM4_REGS->USART_INT.SERCOM_CTRLA =  (SERCOM4_REGS->USART_INT.SERCOM_CTRLA & ~(SERCOM_USART_INT_CTRLA_SAMPR_Msk | SERCOM_USART_INT_CTRLA_FORM_Msk)) | SERCOM_USART_INT_CTRLA_FORM(0x0UL) | SERCOM_USART_INT_CTRLA_SAMPR((uint32_t)sampleRate); 
                SERCOM4_REGS->USART_INT.SERCOM_CTRLB = (SERCOM4_REGS->USART_INT.SERCOM_CTRLB & ~(SERCOM_USART_INT_CTRLB_CHSIZE_Msk | SERCOM_USART_INT_CTRLB_SBMODE_Msk)) | ((uint32_t) serialSetup->dataWidth | (uint32_t) serialSetup->stopBits);
            }
            else
            {
                SERCOM4_REGS->USART_INT.SERCOM_CTRLA =  (SERCOM4_REGS->USART_INT.SERCOM_CTRLA & ~(SERCOM_USART_INT_CTRLA_SAMPR_Msk | SERCOM_USART_INT_CTRLA_FORM_Msk)) | SERCOM_USART_INT_CTRLA_FORM(0x1UL) | SERCOM_USART_INT_CTRLA_SAMPR((uint32_t)sampleRate); 
                SERCOM4_REGS->USART_INT.SERCOM_CTRLB = (SERCOM4_REGS->USART_INT.SERCOM_CTRLB & ~(SERCOM_USART_INT_CTRLB_CHSIZE_Msk | SERCOM_USART_INT_CTRLB_SBMODE_Msk | SERCOM_USART_INT_CTRLB_PMODE_Msk)) | (uint32_t) serialSetup->dataWidth | (uint32_t) serialSetup->stopBits | (uint32_t) serialSetup->parity ;
            }

            /* Wait for sync */
            while((SERCOM4_REGS->USART_INT.SERCOM_SYNCBUSY) != 0U)
            {
                /* Do nothing */
            }

            /* Enable the USART after the configurations */
            SERCOM4_REGS->USART_INT.SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;

            /* Wait for sync */
            while((SERCOM4_REGS->USART_INT.SERCOM_SYNCBUSY) != 0U)
            {
                /* Do nothing */
            }


            if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
            {
                sercom4USARTObj.rdBufferSize = SERCOM4_USART_READ_BUFFER_SIZE;
                sercom4USARTObj.wrBufferSize = SERCOM4_USART_WRITE_BUFFER_SIZE;
            }
            else
            {
                sercom4USARTObj.rdBufferSize = SERCOM4_USART_READ_BUFFER_9BIT_SIZE;
                sercom4USARTObj.wrBufferSize = SERCOM4_USART_WRITE_BUFFER_9BIT_SIZE;
            }

            setupStatus = true;
        }
    }

    return setupStatus;
}

void static SERCOM4_USART_ErrorClear( void )
{
    uint8_t  u8dummyData = 0;

    /* Clear error flag */
    SERCOM4_REGS->USART_INT.SERCOM_INTFLAG = SERCOM_USART_INT_INTFLAG_ERROR_Msk;

    /* Clear all errors */
    SERCOM4_REGS->USART_INT.SERCOM_STATUS = SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk ;

    /* Flush existing error bytes from the RX FIFO */
    while((SERCOM4_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == SERCOM_USART_INT_INTFLAG_RXC_Msk)
    {
        u8dummyData = SERCOM4_REGS->USART_INT.SERCOM_DATA;
    }

    /* Ignore the warning */
    (void)u8dummyData;
}

USART_ERROR SERCOM4_USART_ErrorGet( void )
{
    USART_ERROR errorStatus = sercom4USARTObj.errorStatus;

    sercom4USARTObj.errorStatus = USART_ERROR_NONE;

    return errorStatus;
}


/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static inline bool SERCOM4_USART_RxPushByte(uint16_t rdByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;

    tempInIndex = sercom4USARTObj.rdInIndex + 1U;

    if (tempInIndex >= sercom4USARTObj.rdBufferSize)
    {
        tempInIndex = 0U;
    }

    if (tempInIndex == sercom4USARTObj.rdOutIndex)
    {
        /* Queue is full - Report it to the application. Application gets a chance to free up space by reading data out from the RX ring buffer */
        if(sercom4USARTObj.rdCallback != NULL)
        {
            sercom4USARTObj.rdCallback(SERCOM_USART_EVENT_READ_BUFFER_FULL, sercom4USARTObj.rdContext);

            /* Read the indices again in case application has freed up space in RX ring buffer */
            tempInIndex = sercom4USARTObj.rdInIndex + 1U;

            if (tempInIndex >= sercom4USARTObj.rdBufferSize)
            {
                tempInIndex = 0U;
            }
        }
    }

    /* Attempt to push the data into the ring buffer */
    if (tempInIndex != sercom4USARTObj.rdOutIndex)
    {
        if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
        {
            /* 8-bit */
            SERCOM4_USART_ReadBuffer[sercom4USARTObj.rdInIndex] = (uint8_t)rdByte;
        }
        else
        {
            /* 9-bit */
            ((uint16_t*)&SERCOM4_USART_ReadBuffer)[sercom4USARTObj.rdInIndex] = rdByte;
        }

        sercom4USARTObj.rdInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Data will be lost. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void SERCOM4_USART_ReadNotificationSend(void)
{
    uint32_t nUnreadBytesAvailable;

    if (sercom4USARTObj.isRdNotificationEnabled == true)
    {
        nUnreadBytesAvailable = SERCOM4_USART_ReadCountGet();

        if(sercom4USARTObj.rdCallback != NULL)
        {
            if (sercom4USARTObj.isRdNotifyPersistently == true)
            {
                if (nUnreadBytesAvailable >= sercom4USARTObj.rdThreshold)
                {
                    sercom4USARTObj.rdCallback(SERCOM_USART_EVENT_READ_THRESHOLD_REACHED, sercom4USARTObj.rdContext);
                }
            }
            else
            {
                if (nUnreadBytesAvailable == sercom4USARTObj.rdThreshold)
                {
                    sercom4USARTObj.rdCallback(SERCOM_USART_EVENT_READ_THRESHOLD_REACHED, sercom4USARTObj.rdContext);
                }
            }
        }
    }
}

size_t SERCOM4_USART_Read(uint8_t* pRdBuffer, const size_t size)
{
    size_t nBytesRead = 0U;
    uint32_t rdOutIndex;
    uint32_t rdInIndex;

    /* Take a snapshot of indices to avoid creation of critical section */

    rdOutIndex = sercom4USARTObj.rdOutIndex;
    rdInIndex = sercom4USARTObj.rdInIndex;

    while (nBytesRead < size)
    {
        if (rdOutIndex != rdInIndex)
        {
            if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
            {
                pRdBuffer[nBytesRead++] = SERCOM4_USART_ReadBuffer[rdOutIndex++];
            }
            else
            {
                ((uint16_t*)pRdBuffer)[nBytesRead++] = ((uint16_t*)&SERCOM4_USART_ReadBuffer)[rdOutIndex++];
            }

            if (rdOutIndex >= sercom4USARTObj.rdBufferSize)
            {
                rdOutIndex = 0U;
            }
        }
        else
        {
            /* No more data available in the RX buffer */
            break;
        }
    }

    sercom4USARTObj.rdOutIndex = rdOutIndex;

    return nBytesRead;
}

size_t SERCOM4_USART_ReadCountGet(void)
{
    size_t nUnreadBytesAvailable;
    uint32_t rdOutIndex;
    uint32_t rdInIndex;

    /* Take a snapshot of indices to avoid creation of critical section */
    rdOutIndex = sercom4USARTObj.rdOutIndex;
    rdInIndex = sercom4USARTObj.rdInIndex;

    if ( rdInIndex >=  rdOutIndex)
    {
        nUnreadBytesAvailable =  rdInIndex - rdOutIndex;
    }
    else
    {
        nUnreadBytesAvailable =  (sercom4USARTObj.rdBufferSize -  rdOutIndex) + rdInIndex;
    }

    return nUnreadBytesAvailable;
}

size_t SERCOM4_USART_ReadFreeBufferCountGet(void)
{
    return (sercom4USARTObj.rdBufferSize - 1U) - SERCOM4_USART_ReadCountGet();
}

size_t SERCOM4_USART_ReadBufferSizeGet(void)
{
    return (sercom4USARTObj.rdBufferSize - 1U);
}

bool SERCOM4_USART_ReadNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = sercom4USARTObj.isRdNotificationEnabled;

    sercom4USARTObj.isRdNotificationEnabled = isEnabled;

    sercom4USARTObj.isRdNotifyPersistently = isPersistent;

    return previousStatus;
}

void SERCOM4_USART_ReadThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        sercom4USARTObj.rdThreshold = nBytesThreshold;
    }
}

void SERCOM4_USART_ReadCallbackRegister( SERCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    sercom4USARTObj.rdCallback = callback;

    sercom4USARTObj.rdContext = context;
}


/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static bool SERCOM4_USART_TxPullByte(uint16_t* pWrByte)
{
    bool isSuccess = false;
    uint32_t wrInIndex = sercom4USARTObj.wrInIndex;
    uint32_t wrOutIndex = sercom4USARTObj.wrOutIndex;

    if (wrOutIndex != wrInIndex)
    {
        if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
        {
            *pWrByte = SERCOM4_USART_WriteBuffer[wrOutIndex++];
        }
        else
        {
            *pWrByte = ((uint16_t*)&SERCOM4_USART_WriteBuffer)[wrOutIndex++];
        }


        if (wrOutIndex >= sercom4USARTObj.wrBufferSize)
        {
            wrOutIndex = 0U;
        }

        sercom4USARTObj.wrOutIndex = wrOutIndex;

        isSuccess = true;
    }

    return isSuccess;
}

static inline bool SERCOM4_USART_TxPushByte(uint16_t wrByte)
{
    uint32_t tempInIndex;
    uint32_t wrInIndex = sercom4USARTObj.wrInIndex;
    uint32_t wrOutIndex = sercom4USARTObj.wrOutIndex;

    bool isSuccess = false;

    tempInIndex = wrInIndex + 1U;

    if (tempInIndex >= sercom4USARTObj.wrBufferSize)
    {
        tempInIndex = 0U;
    }
    if (tempInIndex != wrOutIndex)
    {
        if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
        {
            SERCOM4_USART_WriteBuffer[wrInIndex] = (uint8_t)wrByte;
        }
        else
        {
            ((uint16_t*)&SERCOM4_USART_WriteBuffer)[wrInIndex] = wrByte;
        }

        sercom4USARTObj.wrInIndex = tempInIndex;

        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void SERCOM4_USART_WriteNotificationSend(void)
{
    uint32_t nFreeWrBufferCount;

    if (sercom4USARTObj.isWrNotificationEnabled == true)
    {
        nFreeWrBufferCount = SERCOM4_USART_WriteFreeBufferCountGet();

        if(sercom4USARTObj.wrCallback != NULL)
        {
            if (sercom4USARTObj.isWrNotifyPersistently == true)
            {
                if (nFreeWrBufferCount >= sercom4USARTObj.wrThreshold)
                {
                    sercom4USARTObj.wrCallback(SERCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, sercom4USARTObj.wrContext);
                }
            }
            else
            {
                if (nFreeWrBufferCount == sercom4USARTObj.wrThreshold)
                {
                    sercom4USARTObj.wrCallback(SERCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, sercom4USARTObj.wrContext);
                }
            }
        }
    }
}

static size_t SERCOM4_USART_WritePendingBytesGet(void)
{
    size_t nPendingTxBytes;

    /* Take a snapshot of indices to avoid creation of critical section */
    uint32_t wrInIndex = sercom4USARTObj.wrInIndex;
    uint32_t wrOutIndex = sercom4USARTObj.wrOutIndex;

    if ( wrInIndex >= wrOutIndex)
    {
        nPendingTxBytes =  wrInIndex - wrOutIndex;
    }
    else
    {
        nPendingTxBytes =  (sercom4USARTObj.wrBufferSize -  wrOutIndex) + wrInIndex;
    }

    return nPendingTxBytes;
}

size_t SERCOM4_USART_WriteCountGet(void)
{
    size_t nPendingTxBytes;

    nPendingTxBytes = SERCOM4_USART_WritePendingBytesGet();

    return nPendingTxBytes;
}

size_t SERCOM4_USART_Write(uint8_t* pWrBuffer, const size_t size )
{
    size_t nBytesWritten  = 0U;

    while (nBytesWritten < size)
    {
        if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
        {
            if (SERCOM4_USART_TxPushByte(pWrBuffer[nBytesWritten]) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
        else
        {
            if (SERCOM4_USART_TxPushByte(((uint16_t*)pWrBuffer)[nBytesWritten]) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
    }

    /* Check if any data is pending for transmission */
    if (SERCOM4_USART_WritePendingBytesGet() > 0U)
    {
        /* Enable TX interrupt as data is pending for transmission */
        SERCOM4_USART_TX_INT_ENABLE();
    }

    return nBytesWritten;
}

size_t SERCOM4_USART_WriteFreeBufferCountGet(void)
{
    return (sercom4USARTObj.wrBufferSize - 1U) - SERCOM4_USART_WriteCountGet();
}

size_t SERCOM4_USART_WriteBufferSizeGet(void)
{
    return (sercom4USARTObj.wrBufferSize - 1U);
}

bool SERCOM4_USART_WriteNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = sercom4USARTObj.isWrNotificationEnabled;

    sercom4USARTObj.isWrNotificationEnabled = isEnabled;

    sercom4USARTObj.isWrNotifyPersistently = isPersistent;

    return previousStatus;
}

void SERCOM4_USART_WriteThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        sercom4USARTObj.wrThreshold = nBytesThreshold;
    }
}

void SERCOM4_USART_WriteCallbackRegister( SERCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    sercom4USARTObj.wrCallback = callback;

    sercom4USARTObj.wrContext = context;
}



void static SERCOM4_USART_ISR_ERR_Handler( void )
{
    USART_ERROR errorStatus = (USART_ERROR)(SERCOM4_REGS->USART_INT.SERCOM_STATUS & (SERCOM_USART_INT_STATUS_PERR_Msk | SERCOM_USART_INT_STATUS_FERR_Msk | SERCOM_USART_INT_STATUS_BUFOVF_Msk ));

    if(errorStatus != USART_ERROR_NONE)
    {
        /* Save the error to report later */
        sercom4USARTObj.errorStatus = errorStatus;

        /* Clear error flags and flush the error bytes */
        SERCOM4_USART_ErrorClear();

        if(sercom4USARTObj.rdCallback != NULL)
        {
            sercom4USARTObj.rdCallback(SERCOM_USART_EVENT_READ_ERROR, sercom4USARTObj.rdContext);
        }
    }
}

void static SERCOM4_USART_ISR_RX_Handler( void )
{


    while ((SERCOM4_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk) == SERCOM_USART_INT_INTFLAG_RXC_Msk)
    {
        if (SERCOM4_USART_RxPushByte( (uint16_t)SERCOM4_REGS->USART_INT.SERCOM_DATA) == true)
        {
            SERCOM4_USART_ReadNotificationSend();
        }
        else
        {
            /* UART RX buffer is full */
        }
    }
}

void static SERCOM4_USART_ISR_TX_Handler( void )
{
    uint16_t wrByte;

    while ((SERCOM4_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk) == SERCOM_USART_INT_INTFLAG_DRE_Msk)
    {
        if (SERCOM4_USART_TxPullByte(&wrByte) == true)
        {
            if (((SERCOM4_REGS->USART_INT.SERCOM_CTRLB & SERCOM_USART_INT_CTRLB_CHSIZE_Msk) >> SERCOM_USART_INT_CTRLB_CHSIZE_Pos) != 0x01U)
            {
                SERCOM4_REGS->USART_INT.SERCOM_DATA = (uint8_t)wrByte;
            }
            else
            {
                SERCOM4_REGS->USART_INT.SERCOM_DATA = wrByte;
            }

            SERCOM4_USART_WriteNotificationSend();
        }
        else
        {
            /* Nothing to transmit. Disable the data register empty interrupt. */
            SERCOM4_USART_TX_INT_DISABLE();
            break;
        }
    }
}

void SERCOM4_USART_InterruptHandler( void )
{
    bool testCondition = false;
    if(SERCOM4_REGS->USART_INT.SERCOM_INTENSET != 0U)
    {
        /* Checks for error flag */
        testCondition = ((SERCOM4_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_ERROR_Msk) == SERCOM_USART_INT_INTFLAG_ERROR_Msk);
        testCondition = ((SERCOM4_REGS->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_ERROR_Msk) == SERCOM_USART_INT_INTENSET_ERROR_Msk) && testCondition;
        if(testCondition)
        {
            SERCOM4_USART_ISR_ERR_Handler();
        }

        testCondition = ((SERCOM4_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk) == SERCOM_USART_INT_INTFLAG_DRE_Msk);
        testCondition = ((SERCOM4_REGS->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_DRE_Msk) == SERCOM_USART_INT_INTENSET_DRE_Msk) && testCondition;
        /* Checks for data register empty flag */
        if(testCondition)
        {
            SERCOM4_USART_ISR_TX_Handler();
        }

        /* Checks for receive complete empty flag */
        testCondition = (SERCOM4_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk);
        testCondition = (SERCOM4_REGS->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_RXC_Msk) && testCondition;
        if(testCondition)
        {
            SERCOM4_USART_ISR_RX_Handler();
        }
    }
}