/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/
#ifdef __XC32
    #include <xc.h>          /* Defines special funciton registers, CP0 regs  */
#endif

#include <plib.h>           /* Include to use PIC32 peripheral libraries      */
#include <stdint.h>         /* For uint32_t definition                        */
#include <stdbool.h>        /* For true/false definition                      */

#include "system.h"         /* System funct/params, like osc/periph config    */
#include "user.h"           /* User funct/params, such as InitApp             */
#include "Compiler.h"

#include "diskio.h"
#include "pff.h"


unsigned char I2C_Read(unsigned char Address);
void I2C_Write(unsigned char Address, unsigned char data);
void I2C_Init(void);
unsigned char Verify_STA013(void);
void Reset_STA013(void);
void Play_STA013(unsigned char play);
void Run_STA013(void);
void STA013_SendData(unsigned char data);
char Setup_STA013(void);
unsigned char PlayMP3(char* song);
void Volume_STA013(unsigned char volume);


extern void xmit_spi (BYTE d);
extern BYTE rcv_spi (void);
extern void init_spi (void);
extern void spi_change_baud(void);
extern DSTATUS disk_initialize (void);
/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/* i.e. uint32_t <variable_name>; */

#define BUF_SIZE 512
unsigned char AudioBuf[BUF_SIZE];
FATFS fs;			/* File system object */
_DIR dir;			/* Directory object */
FILINFO fno;		/* File information */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

int32_t main(void)
{
    unsigned char i,j;
    BYTE res;
    char songname[20]="ok1.mp3";
    const char path[10]="Song";
    unsigned short fcount;


#ifndef PIC32_STARTER_KIT
    /*The JTAG is on by default on POR.  A PIC32 Starter Kit uses the JTAG, but
    for other debug tool use, like ICD 3 and Real ICE, the JTAG should be off
    to free up the JTAG I/O */
    DDPCONbits.JTAGEN = 0;
#endif

    /*Refer to the C32 peripheral library documentation for more
    information on the SYTEMConfig function.
    
    This function sets the PB divider, the Flash Wait States, and the DRM
    /wait states to the optimum value.  It also enables the cacheability for
    the K0 segment.  It could has side effects of possibly alter the pre-fetch
    buffer and cache.  It sets the RAM wait states to 0.  Other than
    the SYS_FREQ, this takes these parameters.  The top 3 may be '|'ed
    together:
    
    SYS_CFG_WAIT_STATES (configures flash wait states from system clock)
    SYS_CFG_PB_BUS (configures the PB bus from the system clock)
    SYS_CFG_PCACHE (configures the pCache if used)
    SYS_CFG_ALL (configures the flash wait states, PB bus, and pCache)*/

    /* TODO Add user clock/system configuration code if appropriate.  */
    SYSTEMConfig(SYS_FREQ, SYS_CFG_ALL); 

    /* Initialize I/O and Peripherals for application */
    InitApp();

    /*Configure Multivector Interrupt Mode.  Using Single Vector Mode
    is expensive from a timing perspective, so most applications
    should probably not use a Single Vector Mode*/
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

    /* TODO <INSERT USER APPLICATION CODE HERE> */


    I2C_Init();
    Reset_STA013(); //reset only after init i2c
  
  
    if(Verify_STA013()==0)
    {
        while(1){
            LED1^=1;
            DelayMs(200);
        }
    }
 

    if(Setup_STA013()!=FR_OK)
    {
        while(1){
            LED1^=1;
            DelayMs(200);
        }
    }

    LED1=1;

    Run_STA013();   //start STA013
    Play_STA013(1);

    Volume_STA013(245);

    LED2=1;
    
//Play A single Song
/*
    while(1)
    {
      PlayMP3(songname);
    }
*/

// play a folder with folder name "Song"
    fcount=0;
    unsigned char count=0;
    while(1)
    {

        pf_mount(&fs);
        res = pf_opendir(&dir,path);   //root dir
        count=0;
        do{
            res = pf_readdir(&dir, &fno);
            count++;
        }while(count<=fcount);

        if(res!=FR_OK){
           break ;
        }

        if (!fno.fname[0]) //no more file
        {
            break;
        }

        if (fno.fattrib & AM_DIR)   //is directory
        {
            fcount++;
        }
        else{       //is file

            fcount++;

            i=0; j=0;
            while(path[i]!=0){  //search for null termination
                songname[i]=path[i];
                i++;
            }
            songname[i]='/'; i++;
            while(fno.fname[j]!=0){
                songname[i]=fno.fname[j];
                i++;j++;
            }
            songname[i]=0;
            pf_mount(0);
            PlayMP3(songname);

        }



    }
     

    while(1)
    {

       

    }
}



void Volume_STA013(unsigned char volume){
#define DLA 0x46
#define DRA 0x48
unsigned char vol;

vol=255-volume;

I2C_Write(DLA, vol);    //left channel volume
I2C_Write(DRA, vol);    //right channel volume

}

unsigned char PlayMP3(char* song)   //song name with dir
{
    //FATFS fs;
    BYTE res;
    unsigned short nbyte=0;
    unsigned short k=BUF_SIZE; //init to 31 to preload buffer
    long filesize;
    //char name[10];
    unsigned char i=0;

    res =pf_mount(&fs);          //init and mount the disk

    res= pf_open(song);
    if(res!=FR_OK){

        return (res);
    }

    //AudioAmp=0; //on the audio amp

    filesize=fs.fsize;

    while(filesize>0)
    {
        if(k>=BUF_SIZE){     //if buffer emptied refill buffer
            res=pf_read(AudioBuf, BUF_SIZE, &nbyte);
            if(res!=FR_OK)
            {
                return(res);
            }
            k=0;        //reset counter
        }

        if(STA013_DATREQ)       // if STA013 request data
        {
            STA013_EN=1;    //enable STA013

            //putcSPI1(AudioBuf[k]); //send data
            SPI1BUF=AudioBuf[k];
            while(!SPI1STATbits.SPIRBF);
            SPI1BUF;

            k++;
            filesize--;        //counting down
            STA013_EN=0;   //disable STA013

        }
    }   //file completed

    //AudioAmp=1; //off the audio amp

    res=pf_mount(0);     //must use 0 for closing a file, NULL is undefined
    return(res);
}

char Setup_STA013(void){

    unsigned long j;

    BYTE res;
    unsigned char buffer[2];
    unsigned short nbyte=0;

    res =pf_mount(&fs);          //init and mount the disk
    res= pf_open("sta013.cfg");  //open the STA013 config file
    if(res!=FR_OK){
        return(res);
    }

    for(j=0; j<2013; j++){  //write all the configuration to STA013

        res=pf_read(buffer, 2, &nbyte);
        if(res!=FR_OK){
            return(res);
        }
        I2C_Write(buffer[0],buffer[1]);
    }

    res=pf_mount(0);     //must use 0 for closing a file, NULL is undefined
    return(res);
}

void Reset_STA013(void){

    STA013_RESET=0;
    DelayMs(500);
    STA013_RESET=1;
    DelayMs(100);
}
//check the presence of STA013
unsigned char Verify_STA013(void){
    unsigned char data;
    data=I2C_Read(0x01);

    if( data==0xAC)
        return(1);
    else
        return (0);
}

void Run_STA013(void){      //leave idle and start decode
    DelayMs(500);
    I2C_Write(114, 1);      //start STA013
}

void Play_STA013(unsigned char play){

    DelayMs(500);
    I2C_Write(19, play);       //play
}

#define i2c_dly() Delay10us(1)
#define SDA_H() TRISAbits.TRISA15=1;
#define SDA_L() TRISAbits.TRISA15=0; LATAbits.LATA15=0;
#define SCL_H() TRISAbits.TRISA14=1;
#define SCL_L() TRISAbits.TRISA14=0; LATAbits.LATA14=0;
//#define SCL TRISAbits.TRISA14
//#define SDA TRISAbits.TRISA15
#define SCL_IN() PORTAbits.RA14
#define SDA_IN() PORTAbits.RA15

void I2C_Init(void){

    TRISAbits.TRISA14=1;        //SCL
    TRISAbits.TRISA15=1;        //SDA

    LATAbits.LATA14=0;
    LATAbits.LATA15=0;


}
void i2c_start(void)
{
  SDA_H();             // i2c start bit sequence
  i2c_dly();
  SCL_H();
  i2c_dly();
  SDA_L();
  i2c_dly();
  SCL_L();
  i2c_dly();
}

void i2c_stop(void)
{
  SDA_L();             // i2c stop bit sequence
  i2c_dly();
  SCL_H();
  i2c_dly();
  SDA_H();
  i2c_dly();
}

unsigned char i2c_rx(char ack)
{
    char x, d=0;
    SDA_H();
    for(x=0; x<8; x++)
    {
        d <<= 1;
        do {
          SCL_H();
        } while(SCL_IN()==0);    // wait for any SCL clock stretching

        i2c_dly();
        if(SDA_IN()) d |= 1;
        SCL_L();
    }
    if(ack)
    {
        SDA_L();
    }
    else
    {
        SDA_H();
    }
    SCL_H()
    i2c_dly();             // send (N)ACK bit
    SCL_L();
    SDA_H();
    return d;
}

unsigned char i2c_tx(unsigned char d)
{
char x;
static unsigned char b;

 for(x=8; x; x--)
  {
    if(d&0x80){
      SDA_H(); }
    else{
      SDA_L();}

    SCL_H();
    i2c_dly();
    d <<= 1;
    SCL_L();
    i2c_dly();
  }
  SDA_H();
  SCL_H();
  i2c_dly();
  b = SDA_IN();          // possible ACK bit
  SCL_L();
  return b;
}

void I2C_Write(unsigned char Address, unsigned char data)
{

    i2c_start();
    if(i2c_tx(0x86)) while(1);
    if(i2c_tx(Address)) while(1);
    if(i2c_tx(data)) while(1);
    i2c_stop();
}


unsigned char I2C_Read(unsigned char Address)
{
    unsigned char data=0;

    i2c_start();
    if(i2c_tx(0x86)) while(1);

    if(i2c_tx(Address)) while(1);
    i2c_start();
    if(i2c_tx(0x87)) while(1);
    data=i2c_rx(0);
    i2c_stop();

    return(data);
}


