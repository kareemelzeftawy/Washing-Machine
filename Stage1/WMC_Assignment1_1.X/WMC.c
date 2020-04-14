

#include <xc.h>

#define _XTAL_FREQ 10000000

//Global variables
 bit m=0, i=0, o=0;
const char SegCode[11] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x66,0x80};
	//                       0    1    2    3    4    5    6    7    8    9   .
static char Segment[4] = {0x00,0x00,0x00,0x00};	
static unsigned char ColCount=0x00;
void Display(void);
void HTO7S(unsigned int Num);
unsigned int result;


//init_ports subroutine to initialize all the I/O ports you
void init_ports(void)
{
    PORTA = 0;                   // All PORTA pins are cleared
    TRISA = 0b11100011;           // Pin 2&3&4 output - pin 0&1&7&8&5input

    //The PORTB pins is configured as an active low input. 
    PORTB = 0;                  // All PORTB pins are cleared
    TRISB = 0b00111111;         // pin0&1&2&3&4&5 are inputs
    RBPU = 0;                  // Pull-up resistors are enabled
    //The PORTD pins is configured as output for 7-segment display
    TRISD = 0;              // define PORTD as output
    PORTD = 0;             // all PORTD  pins equal zero
    //The PORTC pins is configured as output
    TRISC = 0;               // define PORTC as output
    PORTC = 0;               // all PORTD  pins equal zero
    //The PORTE pins is configured as output
    TRISE = 0;               // define PORTE as output
    PORTE = 0;               // all PORTE  pins equal zero
}

   //button function to read the status of all buttons and return a single byte (char) whose value is 0 if no button is pressed, 1 for the BACK button, 2 for the UP button, or 3 for the DOWN button, or 4 for the ENTER button.
char button(void)
{
    PORTD = 0;             // all PORTD  pins equal zero
    char push;
        if(PORTBbits.RB2 == 0)
        {
            push = '1';
        }
        else if(PORTBbits.RB3 == 0)
        {
            push = '2';
        }
        else if(PORTBbits.RB4 == 0)
        {
            push = '3';
        }
        else if(PORTBbits.RB5 == 0)
        {
            push = '4';
        }
        else if(PORTBbits.RB0 == 0)
        {
            push = '5';
        }
         else if(PORTBbits.RB1 == 0)
        {
            push = '6';
        }
         else if(PORTAbits.RA5 == 0)
        {
            push = '7';
        }
        else
        {
            push = '0';
        }
    __delay_ms(1); // delay of 1ms
    return push;
}

//init_adc subroutine to initialize the ADC ready to read the measure an analog input.
void init_adc(void)
{
    ADCON1 = 0b00001011;
    ADCON2 = 0b10000001;
    CMCON =0x07;		//Turn off comparator module
}

//mass function to read the ADC channel you have used (to 10 bit resolution), calculate and return the mass of the clothes and water (in kg) as a float value.
float mass(void)
{
   //Channel 0
    ADCON0 = 0b00000001;
    float res;
    ADCON0bits.GO_DONE =1;
    while(ADCON0bits.GO_DONE ==1);
    res = ADRESL+(ADRESH*0x100);
    return(res); 
}

//speed function to read the ADC channel you have used (to 10 bit resolution), calculate and return the speed (in rpm) as an unsigned integer value.
unsigned int speed(void)
{
    //channel 1
    ADCON0 = 0b00000101;
    unsigned int res;
    ADCON0bits.GO_DONE =1;
    while(ADCON0bits.GO_DONE ==1);
    res = ADRESL+(ADRESH*0x100);
    return(res); 
}

//status subroutine that displays the status of machine,
void status(void)
{
    //inlet
    if(i == 1) 
    {
        PORTDbits.RD0 = 1;
        __delay_ms(1);
    }
    else 
    {
        PORTDbits.RD0 = 0;
        __delay_ms(1);
    }
    //outlet //rinse
    if(o == 1) 
    {
        PORTDbits.RD1 = 1;
        __delay_ms(1);
        PORTDbits.RD4 = 1;
        __delay_ms(1);
    }
    else 
    {
        PORTDbits.RD1 = 0;
        __delay_ms(1);
        PORTDbits.RD4 = 0;
        __delay_ms(1);
    }
    //stop
    if(i ==0 && m == 0 && o == 0)
    {
        PORTDbits.RD2 = 1;
        __delay_ms(1);
    }
    else 
    {
        PORTDbits.RD2 = 0;
        __delay_ms(1);
    }
    //spin //wash
    if(m == 1) 
    {
        PORTDbits.RD3 = 1;
        __delay_ms(1);
        PORTDbits.RD5 = 1;
        __delay_ms(1);
    }
    else 
    {
        PORTDbits.RD3 = 0;
        __delay_ms(1);
        PORTDbits.RD5 = 0;
        __delay_ms(1);
    }   
}

//motor subroutine that accepts a parameter m, where m = 0 turns off the motor and m = 1 turns on the motor.
void motor(void)
{
    while(PORTAbits.RA5 == 0)
    {
        PORTAbits.RA2 = 0;// turn off display1
        PORTCbits.RC0 = 1; //turn on relay 1 "motor"
        __delay_ms(1);
        m =1;
        status();
    }
    m=0;
    PORTCbits.RC0 = 0; //turn off relay 1 "motor"
}

//inlet subroutine that accepts a parameter i, where i = 1 turns the inlet on and i = 0 turns the inlet off.
void inlet(void)
{
    while(PORTBbits.RB0 == 0)
    {
        PORTAbits.RA2 = 0;// turn off display1
        i = 1;
        status();
    }
    i = 0;  
}

//outlet subroutine that accepts a parameter o, where o = 1 turns the outlet and pump on and o = 0 turns the outlet and pump off.
void outlet(void)
{
    while(PORTBbits.RB1 == 0)
    {
        PORTAbits.RA2 = 0;// turn off display1
        PORTEbits.RE0 = 1; //turn on relay 1 "motor"
        __delay_ms(1);
        o = 1;
        status();
    }
    o=0;
    PORTEbits.RE0 = 0; //turn on relay 1 "motor"
}

//-------------------------------------
// Display routine
//-------------------------------------
void Display()
{
	if (ColCount>=4) 
	ColCount=0;
    switch(ColCount)
    {
        case 0:
            PORTAbits.RA5 = 1;
            break;
        case 1:
            PORTAbits.RA4 = 1;
            break;
        case 2:
            PORTAbits.RA3 = 1;
            break;
        case 3:
            PORTAbits.RA2 = 1;
            break;
        default:
            break;
    }
	PORTD = SegCode[ColCount];
    __delay_ms(10);
    //PORTA = 0;
	ColCount++;				
}	

//--------------------------------------
// Convert HEX 2 byte to 7-Segment code
//--------------------------------------
void HTO7S(unsigned int Num)
{
    unsigned int res;
 
    res = ((1100*Num)%1023)/1000;
	if(res==10)
	{
	Num=Num+1;
	res=0;
	}
    
    Segment[3]=SegCode[(res)];
    res = (1100*Num)/1023;
    Segment[2]=SegCode[((res%10)/10)];
    Segment[2]=SegCode[(res%10)];
    Segment[0]=SegCode[res/10];
    
	if (Segment[0]==0x3f) 
	Segment[0]=0xFF;	
}
//main C program which utilizes the functions/subroutines above to initialize all ports and the ADC, 
//then calls all the functions/subroutines in some logical sequence so that their functions can be fully tested on the simulator.
void main(void)
{   
    //Local variables
    //button function variable for Display1
    char disp1;
    //For variable
    unsigned int i;
    
    // initialize all the I/O ports we have chosen for our hardware design.
    init_ports(); 
    init_adc();
    //endless loop
    while(1)
    {
        disp1 = button();
        PORTAbits.RA2 = 1;// turn on display1
        switch (disp1)
        {
            case '0':
               PORTD = 0b00111111;// write digit 0         
               break;
            case '1':
                while(PORTBbits.RB2 == 0)
                {
                    PORTD = 0b00000110;// write digit 1
                }
                break;
            case '2':
                while(PORTBbits.RB3 == 0)
                {
                    PORTD = 0b01011011;// write digit 2
                }
                break;
            case '3':
                while(PORTBbits.RB4 == 0)
                {
                    PORTD = 0b01001111;// write digit 3
                }
                    break;
            case '4':
                while(PORTBbits.RB5 == 0)
                {
                    PORTD = 0b01100110;// write digit 4
                }
                break;
            case '5':
                inlet();
                break;
            case '6':
                outlet();
                break;
            case '7':
                motor();
                break;
            default:
                break;
        }
       __delay_ms(1); // delay of 1ms
       PORTAbits.RA2 = 0;// turn off display1
       
       //ADC routine
        
        /*
        result=0;
      
		for (i=0;i<4;i++)
		{
			__delay_ms(1); 
			result=result+speed();
		}
            
		HTO7S(result/4);								
		__delay_ms(200);
        Display();
        __delay_ms(10000);*/
    }
     return;
}

