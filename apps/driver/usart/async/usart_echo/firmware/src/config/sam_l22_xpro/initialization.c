/*******************************************************************************
  System Initialization File

  File Name:
    initialization.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, defines the configuration bits,
    and allocates any necessary global system resources,
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "definitions.h"
#include "device.h"


// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************
#pragma config NVMCTRL_BOOTPROT = 0x7
#pragma config NVMCTRL_EEPROM_SIZE = 0x7
#pragma config BOD33USERLEVEL = 0x6U
#pragma config BOD33_DIS = CLEAR
#pragma config BOD33_ACTION = 0x0
#pragma config WDT_ENABLE = CLEAR
#pragma config WDT_ALWAYSON = CLEAR
#pragma config WDT_PER = 0xB
#pragma config WDT_WINDOW = 0xB
#pragma config WDT_EWOFFSET = 0xB
#pragma config WDT_WEN = CLEAR
#pragma config BOD33_HYST = CLEAR
#pragma config NVMCTRL_REGION_LOCKS = 0xffffU




// *****************************************************************************
// *****************************************************************************
// Section: Driver Initialization Data
// *****************************************************************************
// *****************************************************************************
/* Following MISRA-C rules are deviated in the below code block */
/* MISRA C-2012 Rule 7.2 - Deviation record ID - H3_MISRAC_2012_R_7_2_DR_1 */
/* MISRA C-2012 Rule 11.1 - Deviation record ID - H3_MISRAC_2012_R_11_1_DR_1 */
/* MISRA C-2012 Rule 11.3 - Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
/* MISRA C-2012 Rule 11.8 - Deviation record ID - H3_MISRAC_2012_R_11_8_DR_1 */
// <editor-fold defaultstate="collapsed" desc="DRV_USART Instance 0 Initialization Data">

static DRV_USART_CLIENT_OBJ drvUSART0ClientObjPool[DRV_USART_CLIENTS_NUMBER_IDX0];

/* USART transmit/receive transfer objects pool */
static DRV_USART_BUFFER_OBJ drvUSART0BufferObjPool[DRV_USART_QUEUE_SIZE_IDX0];

static const DRV_USART_PLIB_INTERFACE drvUsart0PlibAPI = {
    .readCallbackRegister = (DRV_USART_PLIB_READ_CALLBACK_REG)SERCOM4_USART_ReadCallbackRegister,
    .read_t = (DRV_USART_PLIB_READ)SERCOM4_USART_Read,
    .readIsBusy = (DRV_USART_PLIB_READ_IS_BUSY)SERCOM4_USART_ReadIsBusy,
    .readCountGet = (DRV_USART_PLIB_READ_COUNT_GET)SERCOM4_USART_ReadCountGet,
    .readAbort = (DRV_USART_PLIB_READ_ABORT)SERCOM4_USART_ReadAbort,
    .writeCallbackRegister = (DRV_USART_PLIB_WRITE_CALLBACK_REG)SERCOM4_USART_WriteCallbackRegister,
    .write_t = (DRV_USART_PLIB_WRITE)SERCOM4_USART_Write,
    .writeIsBusy = (DRV_USART_PLIB_WRITE_IS_BUSY)SERCOM4_USART_WriteIsBusy,
    .writeCountGet = (DRV_USART_PLIB_WRITE_COUNT_GET)SERCOM4_USART_WriteCountGet,
    .errorGet = (DRV_USART_PLIB_ERROR_GET)SERCOM4_USART_ErrorGet,
    .serialSetup = (DRV_USART_PLIB_SERIAL_SETUP)SERCOM4_USART_SerialSetup
};

static const uint32_t drvUsart0remapDataWidth[] = { 0x5, 0x6, 0x7, 0x0, 0x1 };
static const uint32_t drvUsart0remapParity[] = { 0x2, 0x0, 0x80000, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU };
static const uint32_t drvUsart0remapStopBits[] = { 0x0, 0xFFFFFFFFU, 0x40 };
static const uint32_t drvUsart0remapError[] = { 0x4, 0x0, 0x2 };

static const DRV_USART_INTERRUPT_SOURCES drvUSART0InterruptSources =
{
    /* Peripheral has single interrupt vector */
    .isSingleIntSrc                        = true,

    /* Peripheral interrupt line */
    .intSources.usartInterrupt             = (int32_t)SERCOM4_IRQn,
};

static const DRV_USART_INIT drvUsart0InitData =
{
    .usartPlib = &drvUsart0PlibAPI,

    /* USART Number of clients */
    .numClients = DRV_USART_CLIENTS_NUMBER_IDX0,

    /* USART Client Objects Pool */
    .clientObjPool = (uintptr_t)&drvUSART0ClientObjPool[0],

    /* Combined size of transmit and receive buffer objects */
    .bufferObjPoolSize = DRV_USART_QUEUE_SIZE_IDX0,

    /* USART transmit and receive buffer buffer objects pool */
    .bufferObjPool = (uintptr_t)&drvUSART0BufferObjPool[0],

    .interruptSources = &drvUSART0InterruptSources,

    .remapDataWidth = drvUsart0remapDataWidth,

    .remapParity = drvUsart0remapParity,

    .remapStopBits = drvUsart0remapStopBits,

    .remapError = drvUsart0remapError,

    .dataWidth = DRV_USART_DATA_8_BIT,
};

// </editor-fold>



// *****************************************************************************
// *****************************************************************************
// Section: System Data
// *****************************************************************************
// *****************************************************************************
/* Structure to hold the object handles for the modules in the system. */
SYSTEM_OBJECTS sysObj;

// *****************************************************************************
// *****************************************************************************
// Section: Library/Stack Initialization Data
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: System Initialization
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: Local initialization functions
// *****************************************************************************
// *****************************************************************************

/* MISRAC 2012 deviation block end */

/*******************************************************************************
  Function:
    void SYS_Initialize ( void *data )

  Summary:
    Initializes the board, services, drivers, application and other modules.

  Remarks:
 */

void SYS_Initialize ( void* data )
{

    /* MISRAC 2012 deviation block start */
    /* MISRA C-2012 Rule 2.2 deviated in this file.  Deviation record ID -  H3_MISRAC_2012_R_2_2_DR_1 */

    NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_RWS(3UL);

    PM_Initialize();

  
    PORT_Initialize();

    CLOCK_Initialize();



    NVMCTRL_Initialize( );

    EVSYS_Initialize();

    SERCOM4_USART_Initialize();

	BSP_Initialize();

    /* MISRAC 2012 deviation block start */
    /* Following MISRA-C rules deviated in this block  */
    /* MISRA C-2012 Rule 11.3 - Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    /* MISRA C-2012 Rule 11.8 - Deviation record ID - H3_MISRAC_2012_R_11_8_DR_1 */

    sysObj.drvUsart0 = DRV_USART_Initialize(DRV_USART_INDEX_0, (SYS_MODULE_INIT *)&drvUsart0InitData);




    /* MISRAC 2012 deviation block end */
    APP_Initialize();


    NVIC_Initialize();


    /* MISRAC 2012 deviation block end */
}

/*******************************************************************************
 End of File
*/
