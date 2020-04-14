
/*
 *File: WMC2.c
 * Author: Partheeban Elangovan
 * Created on 1 June 2017
 */

#pragma config WDT = OFF // required to switch off watch dog timer
                        // it upsets timing and resets periodically

// include for pic register definitions and library functions
#include <xc.h>
#include <string.h>         // this library of string functions is allowed
#include <stdlib.h>         // require for ftoa routine declaration
#include "lcd.h"            // this library supplied with PICsimlab is allowed
#include "atraso.h"         // this library supplied with PICsimlab is allowed
                            // used in LCD routines

// defines for hardware ports and some constants
#define _XTAL_FREQ 10000000     //must match crystal frequency
                            // required for __delay_ms() routine but not others

#define STOP     PORTBbits.RB0        // STOP button
#define  UP      PORTBbits.RB1        // UP button
#define DOWN     PORTBbits.RB2        // DOWN button
#define ENTER    PORTBbits.RB3        // ENTER button
#define DOORSWITCH PORTAbits.RA5      // DOOR SWITCH

#define WASHLED  LATBbits.LB4        // WASH LED
#define RINSELED LATBbits.LB5        // RINSE LED
#define DISPSENSE LATBbits.LB6       // DISPSENSE LED
#define DOORLOCK PORTBbits.RB7        // DOOR Solenoid

#define MOTOR    LATCbits.LC2        // drives fan in simulator
#define INLET    LATCbits.LC0        // drives relay 1 in simulator
#define OUTLET   LATEbits.LE0        // drives relay 2 in simulator
#define SPINSPEED LATCbits.LC1       // drives buzzer in simulator

// function prototypes
void mydelay(int n);                 // optional generic delay
void init_ports(void);               // part1
void init_adc(void);                 // part2
unsigned int myadc(unsigned char ch);// optional separate adc conversion routine
float mass(void);                    // part3
unsigned int speed(void);            // part4
unsigned char button(void);          // part5
void status(unsigned char s, unsigned char c); //part6
void motor(unsigned char m);         // part7
void inlet(unsigned char i);         // part8
void outlet(unsigned char o);        // part9

void operate (unsigned char s, float maxmass, int washtime);
void spin (int spindelay);
void cycle(int c);
void menu(void);
void remaintime(int t);
void stop(void);
void lock (void);

//Global variables
unsigned char str[16],str1[16];
char *buff;
unsigned int speedofdrum, b;
int cycl =1;
float massindrum;

//main C program which utilizes the functions/subroutines above to initialize all ports and the ADC, 
//then calls all the functions/subroutines in some logical sequence so that their functions can be fully tested on the simulator.
void main(void)             //part 10
{   
    init_ports();
    init_adc();
    lcd_init();
    while(1)        //endless loop
    {     
        menu();
        if(b == 4) stop();        
    }
}

void mydelay(int n) // n = number of 0.01 second delays
{
    int ii;         //students may use the delay in atraso.c
    for(ii=0;ii<n;ii++)
    {
        b = button();
        if (b == 4 || b == 5) break;
        __delay_ms(10); //this routine can only accept values up to 78! 
    }
}

//init_ports subroutine to initialize all the I/O ports you
void init_ports(void)
{
    TRISA = 0b00100011;     //RA0>>Mass sensor, RA1>>Speed sensor, RA5>>Door switch
    TRISB = 0b00001111;     //RB0-3>>buttons, RB4-6>>LED output, RB7>>Lock SOLENOID
    TRISC = 0b00000000;     //RC0>>INLET, RC2>>Motor, RC1>>Buzzer"spin speed"
    TRISD = 0b00000000;     //All output to drive LCD data
    TRISE = 0b00000000;     //RE0>>OUTLET, RE0 and RE1 used to drive controls for lcd 
   
    LATA = 0x00;            // All PORTA pins equal zero
    LATB = 0x00;            // All PORTB pins equal zero
    LATC = 0x00;            // All PORTC pins equal zero
    LATD = 0x00;            // All PORTD pins equal zero
    LATE = 0x00;            // All PORTE pins equal zero
}

//init_adc subroutine to initialize the ADC ready to read the measure an analog input.
void init_adc(void)
{
    ADCON0 = 0b00000011;    // bits5,4,3,2 select channel RA0 as default
    ADCON1 = 0b00001101;    // bits7,6,5,4 selects RA0 and RA1 only as analogs, Vcc-Vss ref
    ADCON2 = 0b10000001;    // bit7 right justified, acquistion 0, Fosc/8
}

unsigned int myadc(unsigned char ch)    // this code used in the mass and speed routines
{
    unsigned int a;
    ADCON0bits.CHS0 = ch;      // as only selected channel 0 or 1
            
    ADCON0bits.GO = 1;          // start conversion
    while(ADCON0bits.DONE == 1); // wait till end of conversion
    a = ADRESL;
    a = a + 256*ADRESH;         // get result from ADRESH:ADRESL  
    return a;
}

//mass function to read the ADC channel you have used (to 10 bit resolution), calculate and return the mass of the clothes and water (in kg) as a float value.
float mass(void)
{
   //Channel 0
    unsigned int m;
    float f;
    m = myadc(0);       //read channel 0 for mass
    f = (10.5*(float)m / 1024.0)-2.0; // scale to 10.5kg full scale
    return f; 
}

/*
//speed function to read the ADC channel you have used (to 10 bit resolution), calculate and return the speed (in rpm) as an unsigned integer value.
unsigned int speed(void)
{
    //channel 1
    unsigned int m;
    float f;
    m = myadc(1);       //read channel 1 for speed 0 -1023 returend
    f = (1100.0*(float) m / 819.2);       //scale to 1100 at 4v (4/5*1024 = 819.2)
    m = (int)f;         // convert the float speed to int
    return m; 
}
*/

// button function to read the status of all buttons and return a single byte
//(char) whose value is 0 if no button is pressed, 1 for the STOP button, 2 for
// the UP button, or 3 for the DOWN button, or 4 for the ENTER button.
unsigned char button(void)
{
        if(UP == 0) return 1;
        else if(DOWN == 0) return 2;
        else if(ENTER == 0) return 3;
        if (STOP == 0)  return 4;
        if(DOORSWITCH == 0) return 5;
        return 0;
}

//status subroutine that displays the status of machine,
void status(unsigned char s, unsigned char c)
{
    if(s == 1) WASHLED = c;
    if(s == 2) RINSELED = c;
    if(s == 3) DISPSENSE = c;
    if(s == 4) SPINSPEED = c;
    if(s == 5) DOORLOCK = c;
}

//motor subroutine that accepts a parameter m, where m = 0 turns off the motor and m = 1 turns on the motor.
void motor(unsigned char m)
{
    MOTOR = m;      // will work as MOTOR is just one bit
}

//inlet subroutine that accepts a parameter i, where i = 1 turns the inlet on and i = 0 turns the inlet off.
void inlet(unsigned char i)
{
   INLET = i;  // will work as INLET is just one bit
}

//outlet subroutine that accepts a parameter o, where o = 1 turns the outlet and pump on and o = 0 turns the outlet and pump off.
void outlet(unsigned char o)
{
    OUTLET = o; // will work as OUTLET is just one bit
}

// Wash and Rinse function
void operate(unsigned char s, float maxmass, int washtime)
{
    int stat;
    if (b != 4)
    {
        inlet(1);       //turn on the inlet for the water
        lcd_cmd(L_CLR);
        lcd_cmd(L_L1);
        lcd_str("open the door &");
        lcd_cmd(L_L2);
        lcd_str("put the clothes");
        b = button();
        while( b != 5 ) 
        {
            b = button();
            if (b == 4) break;
        }
    
        massindrum = mass();
        if(massindrum <= maxmass && b != 4)    //fill until total clothes (up to 5kg) + water = 7.5 kg
        {
            lcd_cmd(L_CLR);
            lcd_cmd(L_L1);
            lcd_str("    Warning!    ");
            while(massindrum <= maxmass)
            {
                b = button();
                if (b == 4) break;
                buff = ftoa(massindrum,&stat);
                strcpy(str,buff);
                strcpy(str1,"mass ");
                strcat(str1,str);
                lcd_cmd(L_L2);
                lcd_str(str1);
                massindrum = mass(); 
            }
        }
        if(massindrum > (maxmass+0.1) && b != 4)      //fill until total clothes (up to 5kg) + water = 7.5 kg
        {
            lcd_cmd(L_CLR);
            lcd_cmd(L_L1);
            lcd_str("    Warning!    ");
            lcd_cmd(L_L2);
            lcd_str("mass over limit");
            while(massindrum > (maxmass+0.1))
            {
                b = button();
                if (b == 4) break;
                massindrum = mass(); 
            }
        }
        inlet(0);           // turn off the inlet
    
        if(b != 4)
        {
            status(5,1);    // Door Locked
            status(s,1);    //Turn on Wash led
            if (s == 1)
            {
                lcd_cmd(L_CLR);
                lcd_cmd(L_L1);
                if (cycl == 1) lcd_str("Normal");
                else if (cycl == 2) lcd_str("Delicate");
                else lcd_str("Quick");
                buff = ftoa(maxmass,&stat);
                strcpy(str,buff);
                strcpy(str1,"mass ");
                strcat(str1,str);
                lcd_cmd(L_L2);
                lcd_str("                "); //erase the line 2
                lcd_cmd(L_L2);
                lcd_str(str1);
            }
            mydelay(100);
            if(s == 1 && b != 4)  
            {
                status(3,1);     // Turn on the Dispense LED
                lcd_cmd(L_CLR);
                lcd_cmd(L_L1);
                lcd_str("Dispense");
                remaintime(2);  // dispense (2 second pulse)
                status(3,0);    // Turn off the Dispense LED
            }
        if (b !=4)
        {
            motor(1);           // rotate the drum with the motor
            lcd_cmd(L_CLR);
            lcd_cmd(L_L1);
            if (s == 1) lcd_str("Wash");
            else lcd_str("Rinse");
            remaintime(washtime);   //Wash period
            motor(0);           // turn off the motor
        }
        if (b !=4)
        {
            outlet(1);          // drain the water via the outlet & pump
            lcd_cmd(L_CLR);
            lcd_cmd(L_L1);
            lcd_str("drain the water");
            remaintime(washtime);   // drain time 2 sec
            outlet(0);          // turn off the outlet
            status(s,0);
        }
        } 
    }
}

// Spin Function
void spin(int spindelay)
{
    if (b!= 4)
    {
    status(4,1);    // will control the buzzer
    motor(1);
    outlet(1);
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("spin");
    remaintime(spindelay);
    motor(0);
    outlet(0);
    status(4,0);
    status(5,0);        // Turn off Wash LED 
    }
}

// To choose the selected cycle
void cycle(int c)
{
    switch (c)
    {
        case 1:     //Normal cycle 
            operate(1,7.50,5);  // fill until total clothes (up to 5kg) + water = 7.5 kg, dispense (2 second pulse), wash (45 minutes),
            status (5,0); // Door locked off
            operate(2,7.50,3);  // rinse (10 minutes)
            spin(2);            // spin (5 minutes)
            break;
        case 2:     //Delicate
            operate(1,5.00,3);   // fill until total clothes (up to 3kg) + water = 5 kg, dispense (2 second pulse), wash (12 minutes)
            status (5,0); // Door locked off
            operate(2,5.00,1);   // rinse (3 minutes)
            spin(1);            // spin (3 minutes)
            break;
        case 3:     //Quick
            operate(1,5.00,4);   // fill until total clothes (up to 3kg) + water = 5 kg, dispense (2 second pulse), wash (20 minutes)
            status (5,0); // Door locked off
            operate(2,5.00,2);   // rinse (5 minutes)           
            spin(1);            // spin (3 minutes)
            break;
        default:
            break;  
    }
}

// Menu to choose the cycle
void menu(void)
{
   unsigned char menu[4][20];
   cycl = 1;
   strcpy(menu[0],"Choose Cycle:");
   strcpy(menu[1],"1 - Normal");
   strcpy(menu[2],"2 - Delicate");
   strcpy(menu[3],"3- Quick");
   lcd_cmd(L_CLR);  // Clear LCD 
   lcd_cmd(L_L1);   // First line
   lcd_str(menu[0]);   
   lcd_cmd(L_L2);   // Second line
   lcd_str(menu[1]);
   cycl = 1;    //Normal cycle
   b = button();
   while(b != 3)
   {
       switch(b)
       {
           case 1:  //UP button 
               if(cycl == 3)
               {                 
                   cycl = 2;
                   lcd_cmd(L_CLR);  // Clear LCD   
                   lcd_cmd(L_L2);   // Second line
                   lcd_str(menu[3]); 
                   lcd_cmd(L_L1);   // First line
                   lcd_str(menu[2]); 
               }
               else if(cycl == 2)
                 {
                   cycl = 1;
                   lcd_cmd(L_CLR);                    
                   lcd_cmd(L_L1); // go to line 1
                   lcd_str(menu[0]);
                   lcd_cmd(L_L2);
                   lcd_str(menu[1]);
                 }
               break;
           case 2:  // Down button
               if (cycl == 1)
               {
                   cycl = 2;
                   lcd_cmd(L_CLR);  // Clear LCD   
                   lcd_cmd(L_L2);   // Second line
                   lcd_str(menu[3]); 
                   lcd_cmd(L_L1);   // First line
                   lcd_str(menu[2]); 
               }
               else if (cycl == 2)
               {
                   cycl = 3;
                   lcd_cmd(L_CLR);                    
                   lcd_cmd(L_L1); // go to line 1
                   lcd_str(menu[2]);
                   lcd_cmd(L_L2);
                   lcd_str(menu[3]);
               }
               break;
           case 4:  //stop
               lcd_cmd(L_CLR);  // Clear LCD 
               lcd_cmd(L_L1);   // First line
               lcd_str(menu[0]);   
               lcd_cmd(L_L2);   // Second line
               lcd_str(menu[1]); 
               cycl = 1;
               break;
           default:
               break;
       }
       b = button();
   }
   if (b == 3) // Enter button
   {
       cycle(cycl);  
   }
}

// To display the remaining time
void remaintime(int t)
{
    for(unsigned int i=0; i<t; i++)
    {
        if (b == 4) break;
        if (b == 5) lock();
        lcd_cmd(L_L2);
        strcpy(str,"Remaining: ");
        itoa(str1,t-i,10);
        strcat(str,str1);
        lcd_str(str);
        mydelay((i+1)*100);
      
    }
}

// function when press on stop button 
void stop (void)
{
    cycl = 0;   //cancel cycle
    status(1,0);    //  turn off Wash LED
    status(2,0);    //  turn off Rinse LED
    status(3,0);    // turn off Dispense LED 
    inlet(0);       // turn off the inlet if it was opened
    outlet(1);          // drain the water via the outlet & pump
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("drain the water");
    lcd_cmd(L_L2);
    lcd_str("Remaining: 1 sec");
    atraso_ms(1000);   // drain time 1 sec
    outlet(0);      // turn off the outlet
    status(5,0);    // Door not locked
}

void lock (void)
{
    b = 0;
    lcd_cmd(L_L2);
    lcd_str("Door locked!");
    atraso_ms(1000);
}
