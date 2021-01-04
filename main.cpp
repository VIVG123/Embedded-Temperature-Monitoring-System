#include "mbed.h"
#include "rtos.h"
#include "Keypad.h"
#include "mbed.h"
#include "mbed_rpc.h"


#define curveMAX 100 


Serial        pcr(p9, p10);  
Serial        PC(USBTX, USBRX);
RtosTimer    *LedTimer;
DigitalOut    Led1(LED1);
DigitalOut    Led2(LED2);
Timeout       tout ;
LocalFileSystem local("local");

//Timer tout;
 


int input;
int break1;
int freq = 1;
int wave = 1;
int range = 1;
int group_num = 0;
int counter;
int chs1 = 1 ;
int channelselected= 0;
int sensorselected = 1; 
int channelsensortable[9] = {35,1, 1, 1, 1, 1, 1, 1, 1 };
int i;
int curveVal = 100; 
int baud;


bool flag = true;
bool start = false;
bool channelswitchflag = 0;


float T1= 290.1f;
float sensor1voltage,sensor2voltage,sensor3voltage,sensor4voltage,sensor5voltage,sensor6voltage,sensor7voltage,sensor8voltage;
float sensorVoltage[9] = {0};
float sensorTemp[9] = {0};
float kybstrnum=0.0f;
float x1,x2,y1,y2;

double m[curveMAX] = {1};
double c[curveMAX] = {1};
double x[curveMAX] = {1};
double y[curveMAX] = {1};
double dx[curveMAX] = {1};
double dy[curveMAX] = {1};
 

RPCVariable<float> rpcTdata1(&sensorTemp[1],"KRDG?1");
RPCVariable<float> rpcTdata2(&sensorTemp[2],"KRDG?2");
RPCVariable<float> rpcTdata3(&sensorTemp[3],"KRDG?3");
RPCVariable<float> rpcTdata4(&sensorTemp[4],"KRDG?4");
RPCVariable<float> rpcTdata5(&sensorTemp[5],"KRDG?5");
RPCVariable<float> rpcTdata6(&sensorTemp[6],"KRDG?6");
RPCVariable<float> rpcTdata7(&sensorTemp[7],"KRDG?7");
RPCVariable<float> rpcTdata8(&sensorTemp[8],"KRDG?8");
 
char id = '0'; 
char display[12]={"000.0 K"};
char Keytable[] = { 
    '1', '2', '3', // r0
    '4', '5', '6', // r1
    '7', '8', '9', // r2
    '*', '0', '#' // r3
  // c0   c1  c2
 };

//SPI for Display(16*2)
SPI spi(p5, NC, p7); //(p5, NC, p7)  ///p6  from proto1 pcb  // mosi, miso, sclk //p6 used for emergency
DigitalOut cs(p28); //27  // p8 removed passwordbecause of clash with g. display// this is cs , in display it is STB

//SPI for MPC 3208
SPI spi2(p11, p12, p13);  /// for ADC MAX 3208
DigitalOut cs2(p8);
 
int32_t       Index = -1;
 
uint32_t cbAfterInput(uint32_t index)
{
    Index = index; 
    return 0;
}

void timeoutNoIP() 
{
    break1 = 1 ;
    PC.printf("Inside Timeout");
    return;
}

float readchannel( int ch)  
{
    cs2 = 0; 
            
    spi2.write( 0x06 | (ch >> 2) ); // High
    uint8_t d1 = spi2.write( ch << 6 ) & 0x0F; // Low
    uint8_t d2 = spi2.write(0x00);
            
    cs2 = 1; 
            
    uint16_t d = (d1 << 8) | d2;
    
    float volt = 10.0 * (float)d / 4095;
    return(volt);
}


void displayinitialize()
{
    int counter;
    spi.format(16,3);
    spi.frequency(1000000);  // spi freq  1 MHz
    cs = 0;
    spi.write(0xF830);
    cs=1;
    for (counter=0;counter<1000;counter++);//wait(.01);
    
   
    cs = 0;
    spi.write(0xF80C); //0C
    cs=1;
    for (counter=0;counter<1000;counter++);// wait(.01);

return;
}

 
void displaychar(unsigned int  mychar)
{

    unsigned int myint= 0x0;
    cs = 0;
    
    myint= (0xFA00 | (mychar));
    spi.write(myint);
    cs=1;
    for (counter=0;counter<1000;counter++);   //wait(.01);
    
    return;
}


void displaygotofirstchar( int lineno )
{
    int counter1,counter2; 
    if (lineno == 1)
    {
        cs = 0;
        spi.write(0xF880);
        cs=1;
        for (counter1=0;counter1<1000;counter1++);// wait(.01);
        cs  =1;   
    }
    
    else if (lineno==2)
    {
        cs = 0;
        spi.write(0xF8C0);
        cs=1;
        for (counter2=0;counter2<1000;counter2++); //wait(.01);
        cs  =1;   
    }

    return;  
}



void writestringspi_line1( char *s )
{

    char *mystr;
    mystr = s;

    while (*mystr)
    {

        displaychar(*mystr);
        mystr = mystr+1;

    }

    return;
} 



void displaychar_line2( unsigned int  mychar )
{
    int counter1,counter2;
 
    unsigned int myint= 0x0; 
 
  
    cs = 0;
    spi.write(0xF80C);  
    cs=1;
    for (counter1=0;counter1<1000;counter1++);// wait(.01);
     
    cs = 0;
     
    if (mychar == '*'){ mychar = '%'; } // added to make line2 * key work like %    
    myint= (0xFA00 | (mychar));
    spi.write(myint);
    cs=1;
    for (counter2=0;counter2<1000;counter2++);// wait(.01);

    return;
}



void writestringspi_line2( char *s )
{
    int counter;
    char *mystr;
    mystr = s;

    cs = 0;
    spi.write(0xF8C0);
    cs=1;
    for (counter=0;counter<100000;counter++);
    //wait(.01);

    while ( *mystr )
    {
        displaychar_line2(*mystr); 
        mystr = mystr+1;
    }
    
    return;
}


void blanklineandline2()
{
    displaygotofirstchar(1);
    sprintf(display,"                ");  
    for (counter=0;counter<1000;counter++);//wait(.01);
    writestringspi_line1(display);  
    
    sprintf(display,"                ");  
    writestringspi_line2(display);  
    for (counter=0;counter<1000;counter++);//wait(.01);
}   

void blankline2()
{   displaygotofirstchar(2) ;
    sprintf(display,"                ");  
    writestringspi_line2(display);  
    for (counter=0;counter<1000;counter++);//wait(.01);
}

void selectSensor()
{   
    int togglecounter = 1 ;
    sensorselected = 1;
    break1 = 0;
    Index = -1;
    PC.printf("\nSensor curve type or type of sensor:\n");
    
    blanklineandline2();
    displaygotofirstchar(1);
    sprintf(display,"Select Sensor");   
    writestringspi_line1(display);  
    
    //default sensor display
    channelsensortable[1] = chs1 ;
    displaygotofirstchar(2);
    
    if( channelsensortable[channelselected] == 1 )
    {   togglecounter = 1 ;
        sensorselected = 1 ;
        sprintf(display,"DT470<*><ENTER>");
        writestringspi_line2(display);   
    }    
    
    else if( channelsensortable[channelselected] == 2 )
    {   togglecounter = 2 ;
        sensorselected = 2 ;
        sprintf(display,"PT100<*><ENTER>");
        writestringspi_line2(display);   
    }    
    
    else if( channelsensortable[channelselected] == 3 )
    {   togglecounter = 0 ;
        sensorselected = 3 ;
        sprintf(display,"Sens3<*><ENTER>");
        writestringspi_line2(display);   
    }    
   
    
    PC.printf("1. DT470\n");
    PC.printf("2. PT100\n");
    PC.printf("3. Sensor3\n");
    PC.printf("Press any other number to escape\n");
    
    
    while (break1 != 1) 
    {
        __wfi();
        
        if (Index > -1)
        {   
            input = Index + 1;
            switch (input)
            {
                
                case 1:
                    PC.printf("Sensor 1  DT470 selected for this channel,press ENTER\n");
                    
                    displaygotofirstchar(2);
                    sprintf(display,"DT470<*><ENTER>");
                    writestringspi_line2(display);     
                    
                    sensorselected = input;
                    togglecounter = input;
                    
                    break;
                    
                
                case 2:
                    PC.printf("Sensor 2  PT100 selected for this channel,press ENTER\n");
                    
                    displaygotofirstchar(2);
                    sprintf(display,"PT100<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    sensorselected = input;
                    togglecounter = input;

                    break;
                    
                case 3:
                    PC.printf("Sensor 3 selected for this channel,press ENTER\n");
                    
                    displaygotofirstchar(2);
                    sprintf(display,"Sens3<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    sensorselected = input;
                    togglecounter = input;
                    
                    break;
                    
                
                case 10:
                        
                    togglecounter++;
                    displaygotofirstchar(2);
                    
                    if(togglecounter == 1)
                        sprintf(display,"DT470<*><ENTER>");
                    else if(togglecounter == 2)
                        sprintf(display,"PT100<*><ENTER>");
                    else if(togglecounter == 3)
                        sprintf(display,"Sens3<*><ENTER>"); 
                    
                    writestringspi_line2(display);  
                    sensorselected = togglecounter;
                    
                    if (togglecounter==3)
                        togglecounter=0;
                        
                    break;
                                 
                case 12:
                    PC.printf("CHECK channel %d and sensor %d type stored on ENTER pressed\n",channelselected,sensorselected);
                    
                    blanklineandline2();
                    displaygotofirstchar(1);
                    sprintf(display,"CH%d Sensor%1d" , channelselected, sensorselected);
                    writestringspi_line1(display);
                    
                    displaygotofirstchar(2);
                    sprintf(display,"Stored!");
                    writestringspi_line2(display);
                    
                    for (counter=0;counter<10000000;counter++);
                    
                    channelsensortable[channelselected] = sensorselected;
                    
                    if( channelselected == 1 ) 
                        chs1 = sensorselected ;
                        
                    channelsensortable[1] = chs1 ;
                    
                    for (i=1; i<9; i++)
                        PC.printf("CH%d  Sensor %d\n",i,channelsensortable[i]);
                    
                    break1 =1;
                    break;
                    
                      
                default: 
                    break1 =1;
                    PC.printf("you pressed to ESCAPE..No Sensor selected  no ENTER\n");
                    displaygotofirstchar(1);
                    sprintf(display,"Escaping..."); 
                    writestringspi_line1(display); 
                    break;
                    
            }
            
            Index = -1;
        }
    }
}

 
void  sensortypeselect()
{   
    channelselected = 1;
    int togglecounter = 1;
    break1 = 0;
    Index = -1;
    
    PC.printf("\nSelect the channel:\n");
    PC.printf("1. CH 1\n");
    PC.printf("2. CH 2\n");
    PC.printf("3. CH 3\n");
    PC.printf("4. CH 4\n");
    PC.printf("5. CH 5\n");
    PC.printf("6. CH 6\n");
    PC.printf("7. CH 7\n");
    PC.printf("8. CH 8\n");
    PC.printf("Press  any other number to escape\n");

    blanklineandline2();
    displaygotofirstchar(1);
    sprintf(display,"ENTER CH:1 - 8");   
    writestringspi_line1(display); 
     
    
     
    displaygotofirstchar(2);
    sprintf(display,"CH1<*><ENTER>");   
    writestringspi_line1(display);  
    
   //tout.start(); //joby
    
    //tout.attach(&timeoutNoIP, 10.0);
     
    while (break1 != 1) 
    {   
        __wfi();
         
        /*
       if (tout.read() >= 10.0)
       {
     
        PC.printf("Timer Read Success %f\n",tout.read());
        break1=1;
           tout.stop();
           tout.reset();
        return;
      }
      */
        
        if (Index > -1)
        {
            input = Index + 1;
            switch (input)
            {
                case 1:    
                    displaygotofirstchar(2);
                    sprintf(display,"CH1<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH1 selected,press ENTER\n");
                    
                    channelselected = input;
                    togglecounter = input;
                    
                    break;
                    
                case 2:
                    
                    displaygotofirstchar(2);
                    sprintf(display,"CH2<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH2 selected,press ENTER\n");
                    channelselected = input;
                    togglecounter = input;
                    
                    break;
                    
               case 3:
                    
                    displaygotofirstchar(2);
                    sprintf(display,"CH3<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH3 selected,press ENTER\n");
                    channelselected = input;
                    togglecounter = input;
                    
                    break;
                    
               case 4:
                    
                    displaygotofirstchar(2);
                    sprintf(display,"CH4<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH4 selected,press ENTER\n");
                    channelselected = input;
                    togglecounter = input;
                    
                    break;
                    
                case 5:
                    
                    displaygotofirstchar(2);
                    sprintf(display,"CH5<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH5 selected,press ENTER\n");
                    channelselected = input;
                    togglecounter = input;
                    
                    break;
                    
                case 6:
                    
                    displaygotofirstchar(2);
                    sprintf(display,"CH6<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH6 selected,press ENTER\n");
                    channelselected = input;
                    togglecounter = input;
                    
                    break;
                    
                case 7:
                    
                    displaygotofirstchar(2);
                    sprintf(display,"CH7<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH7 selected,press ENTER\n");
                    channelselected = input;
                    togglecounter = input;
                    
                    break;
                    
                    
                case 8:
                    
                    displaygotofirstchar(2);
                    sprintf(display,"CH8<*><ENTER>"); 
                    writestringspi_line2(display);     
                    
                    PC.printf("CH8 selected,press ENTER\n");
                    channelselected = input;
                    togglecounter = 0;
                    
                    break;
                    
                    
                case 10: 
                   
                    togglecounter++;
                    displaygotofirstchar(2);
                    sprintf(display,"CH%1d<*><ENTER>",togglecounter);   
                    writestringspi_line2(display); 
                    
                    channelselected = togglecounter; 
                    
                    if (togglecounter >= 8)
                        togglecounter=0;
                        
                   // tout.stop(); // joby
                    
                    break;
                    
                    
                case 12:
                    PC.printf("channel stored on ENTER pressed\n");
                    PC.printf("calling sensor select fn ..\n");
                    PC.printf("value of ch here in case12 is %d\n",channelselected);
                    selectSensor();
                     
                    break1 =1;
                    break; 
                    
                    
                default:
                    displaygotofirstchar(1);
                    sprintf(display,"Escaping..."); 
                    writestringspi_line1(display); 
                    break1 =1;
                    PC.printf("Pressed other thatn 1to8  to ESCAPE\n");
                    break;
                    
            }
            
            Index = -1;
        }
    }
}


int selectBaud(){
    
    break1 = 0;
    Index = -1;
    
    PC.printf("\nSelect Baud Rate\n");
    PC.printf("1. 4800\n");
    PC.printf("2. 9600\n");
    PC.printf("3. 19200\n");
    PC.printf("4. 38400\n");
    PC.printf("5. 57600\n");
    PC.printf("5. 115200\n");
    PC.printf("Press * to go to main menu\n");
    
    while (break1 != 1) 
    {
        __wfi();
        
        if (Index > -1) 
        {
            input = Index + 1;
            
            switch (input)
            {
                case 1:
                    PC.printf("4800\n");
                    break;
                    
                case 2:
                    PC.printf("9600\n");
                    break;
                    
                case 3:
                    PC.printf("19200\n");
                    break;
                    
                case 4:
                    PC.printf("38400\n");
                    break;
                case 5:
                    PC.printf("Baud Rate=57600\n");
                    break;
                    
                case 6:
                    PC.printf("Baud Rate=115200n");
                    break;
                    
                case 10:
                    break1 =1;
                    break;
                    
                default:
                    PC.printf("Please Input a Valid Input\n");
                    PC.printf("\nSelect Baud Rate\n");
                    PC.printf("1. 4800\n");
                    PC.printf("2. 9600\n");
                    PC.printf("3. 19200\n");
                    PC.printf("4. 38400\n");
                    PC.printf("5. 57600\n");
                    PC.printf("5. 115200\n");
                    PC.printf("Press * to go to main menu\n");
                    break;
                }
            
            Index = -1;
        }
    }
    return 0;
}

double curveDataentry( char *s, int ind )
{
    char onechar;
    char* packet_data = new char[6];
    strcpy(packet_data,"\0"); 
    
    int a1 = 1 ;
    int ctr = 0 ;
    PC.printf( "Enter %s",s );
    
    blanklineandline2();
    displaygotofirstchar(1);
    
    if(*s == 'n')
    {   sprintf(display,"Total Points:");
        writestringspi_line1(display);  
    }
    
    else 
    {   
        sprintf(display, "Enter %s%1d:", s , ind) ; 
        writestringspi_line1(display);
    
        displaygotofirstchar(2);
        if ( *s == 'x')
            sprintf(display,"%7.3f",x[ind]);
        else if ( *s == 'y')
            sprintf(display,"%7.3f",y[ind]);
        writestringspi_line2(display);    
     }
     
     break1 = 0;
     Index = -1;
     
     displaygotofirstchar(2);
     
     while (break1 != 1) 
     {
        __wfi();
       
        
        if (Index > -1) 
        {   if(a1 == 1)
            {    blankline2() ;
                displaygotofirstchar(2);
            }
                
            
            input = Index + 1;
            ctr++ ;
            
            switch (input)
            {
                case 1:
                    a1 = 0;
                    PC.printf("1");
                    onechar='1';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"1");
                    break;
                    
                case 2:
                    a1=0;
                    PC.printf("2");
                    onechar='2';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"2");
                    break;
                    
                case 3:
                    a1=0;
                    PC.printf("3");
                    onechar='3';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"3");
                    break;
                    
                case 4:
                    a1=0;
                    PC.printf("4");
                    onechar='4';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"4");
                    break;
                
                case 5:
                    a1=0;
                    PC.printf("5");
                    onechar='5';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"5");
                    break;
                
                case 6:
                    a1=0;
                    PC.printf("6");
                    onechar='6';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"6");
                    break;
                    
                case 7:
                    a1=0;
                    PC.printf("7");
                    onechar='7';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"7");
                    break;
                    
                case 8:
                    a1=0;
                    PC.printf("8");
                    onechar='8';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"8");
                    break;
                    
                case 9:
                    a1=0;
                    PC.printf("9");
                    onechar='9';
                    
                    displaychar_line2(onechar);
                    strcat(packet_data,"9");
                    break;
                    
                case 10:
                    if(ctr == 1) 
                    { 
                        PC.printf("-") ;
                        onechar='-';
                        
                        displaychar_line2(onechar);
                        strcat(packet_data, "-") ; 
                    }  
                    else 
                    {   
                        PC.printf(".");
                        onechar='.';
                        
                        displaychar_line2(onechar);
                        strcat(packet_data,".");
                    }
                
                    if (a1 == 10 && input == 10)
                    {
                        printf( "Escape...\n" );
                        
                        sprintf(display,"Escaping...");   
                        writestringspi_line2( display );
                        break1  = 1;
                        flag = false;
                    }
                 
                    a1 = input ;
                    break;
                    
                case 11:
                    a1=0;
                    PC.printf("0");
                    onechar='0';
                    
                    displaychar_line2(onechar);
                    strcat( packet_data,"0" );
                    break;
                  
                case 12:
                    if (a1 == 1)
                    {
                        switch ( *s )
                        {
                    
                        case 'x':
                            kybstrnum = x[ind] ;
                            PC.printf("Previous X if ENTER pressed = %f\n", kybstrnum);
                            break ;
                    
                        case 'y':
                            kybstrnum=y[ind];
                            PC.printf("Previous Y if ENTER pressed = %f\n", kybstrnum);
                            break ;
                        
                        }
                    }
                 
                    else
                        kybstrnum=atof(packet_data);

                    break1=1; 
                    break; 
                  
                 default:
                    break;
            }
            
            Index = -1;
        }
    }
    
    return kybstrnum ;    
}


void interpolate( double x[], double y[] )
{
    for (i=0; i<curveVal-1; i++)
    {
        m[i] = (y[i+1] - y[i]) / (x[i+1] - x[i]);
        c[i] = y[i] - m[i]*x[i];
    }
}



double sensor3VtoT(double volt)
{
    double temp = 0;
    
    for (i=0; i < curveVal-2; i++)
    {
        if (volt >= x[i] && volt < x[i+1])
        {
            temp = m[i]*volt + c[i];
            return temp;
        } 
    }
    
    if (volt >= x[i] && volt <= x[i+1])
    {
        temp = m[i]*volt + c[i];
        return temp;
    }
    return 0;
}


float convert_D_to_T(float D)
{
 //calibration dt470 starts here 
 
    float  T;
    D *= 409.5;
     
    if  (D > 3522)  
        T = 481 - (454 * D) / 4096;

    else if  (D > 3145) 
        T = 754 - (779 * D) / 4096;

    else if  (D > 2762) 
        T = 810 - (844 * D) / 4096;

    else if( D > 2721) 
        T = 2207 - (2930 * D / 4096);

    else if (D > 2704)   
        T = 3661 - (5120 * D) / 4096;

    else if (D > 2662) 
        T = 5908 - (8533 * D) / 4096;

    
    else if (D > 2560)
        T = 6538 - (9498 * D) / 4096;

    
    else if (D > 2481)
        T = 5975 - (8582 * D) / 4096;

    
    else if (D > 2397) 
        T = 5974 - (8582 * D) / 4096;

    else if (D > 2166) 
        T = 5535 - (7817 * D) / 4096;
 
    else if (D > 1865) 
        T = 5322 - (7402 * D) / 4096;
   
    else if (D > 1378)  
        T = 5186 - (7086 * D) / 4096;

    else if (D > 1026)
        T = 5124 - (6873 * D) / 4096;

    else if (D > 728)
        T = 5123 - (6873 * D) / 4096;

    else if (D > 304) 
        T = 5108 - (6827 * D) / 4096;

    
    else
        T = 0;

    T= T/10.0 ; 
    
    return(T) ;

}

float convert_P_to_T( float voltinD ) // pt 100 curve
{
    voltinD = voltinD * 409.5; // converting the volt into 0 to 4095  
    float f;
    f = 30.05575 + 0.08533 * voltinD + 0.00000180114 * voltinD * voltinD ;
    return f;
}

void continuousreadsensor()
{
  
    interpolate(x,y);
    for (int j = 1; j<9; j++)
        sensorVoltage[j] = readchannel(j-1);
    
    for (int j=1; j<9; j++)
    {
        if (channelsensortable[j] == 1)
            sensorTemp[j] = convert_D_to_T(sensorVoltage[j]); //store these values somewhere!!!
        else if (channelsensortable[j] == 2)
            sensorTemp[j] =  convert_P_to_T(sensorVoltage[j]);
        else if (channelsensortable[j] == 3)
            sensorTemp[j] = sensor3VtoT(sensorVoltage[j]);
            
        PC.printf("V%d = %f\t T = %f\n",j , sensorVoltage[j], sensorTemp[j]);
    }

    for (counter = 0; counter < 1000000; counter++);

    blanklineandline2();

    if (channelswitchflag == 0)
    {
        displaygotofirstchar(1);   
        sprintf(display,"1 %5.1fK %5.1fK",  sensorTemp[1],sensorTemp[2]);
        writestringspi_line1(display);
      
        sprintf(display,"3 %5.1fK %5.1fK",  sensorTemp[3],sensorTemp[4]);
        writestringspi_line2(display);   
    
    }

    else 
    {
        displaygotofirstchar(1);
        sprintf(display,"5 %5.1fK %5.1fK",  sensorTemp[5],sensorTemp[6]);
        writestringspi_line1(display);

        sprintf(display,"7 %5.1fK %5.1fK",  sensorTemp[7],sensorTemp[8]);
        writestringspi_line2(display);
        
    }

    if ( channelswitchflag == 0 ) 
        channelswitchflag = 1;
    else 
        channelswitchflag = 0;

}




void save_current_configurations()
{
    
    FILE *fp;
    
    int i;
    
    fp = fopen("/local/datafile.txt", "w");
       
    for (i=0;i<9;i++)
        fprintf( fp, "%d\t",channelsensortable[i] ); 

    fprintf(fp, "\n");
    
    for (i=0;i<100;i++) 
    {
        fprintf(fp, "%lf\t",x[i]);  
        fprintf(fp, "%lf\n",y[i]); 
    }
    
    fprintf(fp, "\n");
    
    for (i=0;i<100;i++) 
    {
        fprintf(fp, "%lf\t",m[i]); 
        fprintf(fp, "%lf\n",c[i]); 
    }
    
    fprintf(fp, "\n");
    
    fclose(fp);
    
    PC.printf("Saved in FLASH Successfully!");
    
    blanklineandline2();
    displaygotofirstchar(1);
    sprintf(display, "Saved in FLASH"); 
    writestringspi_line1(display);
    
    displaygotofirstchar(2);
    sprintf(display, "Successfully!"); 
    writestringspi_line2(display);
    
    for (counter=0;counter<10000000;counter++);

}


void load_saved_configurations()
{
    FILE *fp;
    
    int i;

    fp = fopen("/local/datafile.txt", "r");
    if (fp == NULL) return;
    
    for (i=0;i<9;i++)
    {   
        fscanf(fp, "%d\t",&channelsensortable[i]); 
     
         
    }
    chs1 = channelsensortable[1] ;
    
    
    for (i=0;i<100;i++) 
    {
        fscanf(fp, "%lf\t%lf\n",&x[i],&y[i]); 
    }
  
    
    for (i=0;i<100;i++) 
    {
        fscanf(fp, "%lf\t%lf\n",&m[i], &c[i]); 
    }
    
    fclose(fp);
}


 
int main()
{ 
     char buf[256], outbuf[256];
 
     channelsensortable[0]=0;
  for (i=0;i<curveMAX;i++)
  {  
        m[i] = 1;
        c[i] = 1;
        x[i] = 1;
        y[i] = 1;
        dx[i] = 1;
        dy[i] = 1;
  }
    
    load_saved_configurations();
    
  
    
    //             r0   r1   r2   r3   c0   c1   c2   c3
    Keypad keypad(p21, p22,  p23,  p24, p25, p26,  p27,  NC);
    keypad.attach(&cbAfterInput);
    keypad.start();  // energize the columns c0-c3 of the keypad
 
 
    displayinitialize();
    writestringspi_line1("                  ");
    writestringspi_line2("                  ");
    writestringspi_line2("LOW TEMP");
 
    while (1) 
    {
        __wfi();

        if ( pcr.readable() )
        {
            pcr.gets(buf,256);
            RPC::call(buf,outbuf);
            pcr.printf("%s\n",outbuf);   
        }
        
        continuousreadsensor();
        channelsensortable[1] = chs1 ;
 
        for (i=1;i<9;i++)
        { 
         PC.printf("channel is  %d  sensor is %d\n",i, channelsensortable[i]); 
        }
        
        for (i=0; i<100; i++)
        {
            //PC.printf("x[%d] = %f\t y[%d] = %f\n", i, x[i], i, y[i]);
        }
        
         for (i=0;i<curveVal;i++)
        {         
            PC.printf("x stored is = %f\n",x[i]);            
            PC.printf("y stored is = %f\n",y[i]);
        }
        
    
        if (Index > -1) 
        {
            //PC.printf("inside first if %u \n",Index+1);
            input = Index + 1;
            switch (input)
            {    
                case 2:
                    PC.printf("Calling sensor3curveDataEntry()... \n");
                    id = '2';
                    
                    do
                    {
                        PC.printf("Enter no. of values (must be a positive integer > 2): ");
                        
                        writestringspi_line1(display);
                        curveVal = (int)curveDataentry("n",0);
                        if (!flag)  break;
                    }
                    while (curveVal <= 1 || curveVal > 100);
                    
                    for (i=0;i<curveVal;i++)
                    {
                        if (flag)
                        {
                            dx[i]=curveDataentry("x",i);
                            if(!flag) break ;
                            //PC.printf("x returned is = %f\n",dx[i]);
                            dy[i]=curveDataentry("y",i);
                            if(!flag) break ;
                            //PC.printf("y returned is = %f\n",dy[i]);
                        }
                    }
                    
                    if(flag)
                    {
                        for (i=0;i<curveVal;i++)
                        {
                            x[i]=dx[i];
                            y[i]=dy[i];
                        }
                    }    
                    
                    flag = true;
                    break; 
               
                case 8: 
                    save_current_configurations();
                    break;
               
               case 6:
                    PC.printf("Pressed 6 to enter Sensor Selection for 8 Channels... \n");
                    id = '6';
                    sensortypeselect();
                    break;    
            }
            
            Index = -1;
            
        }
    }
}