/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#ifdef __XC32
    #include <xc.h>          /* Defines special funciton registers, CP0 regs  */
#endif

#include <plib.h>            /* Include to use PIC32 peripheral libraries     */
#include <stdint.h>          /* For uint32_t definition                       */
#include <stdbool.h>         /* For true/false definition                     */
#include "user.h"            /* variables/params used by user.c               */
#include "system.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* TODO Initialize User Ports/Peripherals/Project here */

void InitApp(void)
{
    /* Setup analog functionality and port direction */

    /* Initialize peripherals */
    SYSTEMConfigPerformance(GetSystemClock());
    mOSCSetPBDIV(OSC_PB_DIV_1);	// match the PBus to the fuse setting
    // disable JTAG
    mJTAGPortEnable(0);

    // enable global interrupt handling
    INTEnableSystemMultiVectoredInt();

    AD1PCFG = 0xFFFF;
    TRISDbits.TRISD8=0; //STA013 reset
    STA013_RESET=1;

    TRISGbits.TRISG9=1;  //STA013 DATREQ
    TRISDbits.TRISD11=0; //sta013en

    TRISAbits.TRISA15=1; //SDA
    TRISAbits.TRISA14=1; //SCL

    TRISDbits.TRISD9=0; //CS
    TRISDbits.TRISD10=0; //SCK
    TRISDbits.TRISD0=0; //SDO
    TRISCbits.TRISC4=1; //SDI;

    TRISBbits.TRISB0=0; //LED
    TRISBbits.TRISB1=0;

    TRISGbits.TRISG15=1;

    

    LED1=0;
    LED2=0;




}
