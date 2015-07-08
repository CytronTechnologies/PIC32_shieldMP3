/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/

/* TODO Define system operating frequency */

/* Microcontroller MIPs (FCY) */
#define SYS_FREQ     60000000L
#define FCY          SYS_FREQ
#define GetInstructionClock()  SYS_FREQ
#define GetSystemClock() SYS_FREQ

#define I2C_CLOCK_FREQ 100000
#define SLAVE_ADDRESS  0x86
#define STA013_RESET    LATDbits.LATD8
#define STA013_DATREQ   PORTGbits.RG9
#define LED1    LATBbits.LATB0
#define LED2    LATBbits.LATB1

#define STA013_EN LATDbits.LATD11




/***********************************
 *******************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */


