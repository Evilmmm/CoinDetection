#include <XC.h>
#include <sys/attribs.h>
#include <stdio.h>

#pragma config FNOSC = FRCPLL       // Internal Fast RC oscillator (8 MHz) w/ PLL
#pragma config FPLLIDIV = DIV_2     // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_20     // PLL Multiply (now 80 MHz)
#pragma config FPLLODIV = DIV_2     // Divide After PLL (now 40 MHz) see figure 8.1 in datasheet for more info
#pragma config FWDTEN = OFF         // Watchdog Timer Disabled
#pragma config FPBDIV = DIV_1       // PBCLK = SYCLK

#define SYSCLK 40000000L
#define FREQ 100000L // We need the ISR for timer 1 every 10 us
#define FREQ2 1L // We need the ISR for timer 1 every 10 us
#define Baud2BRG(desired_baud)( (SYSCLK / (16*desired_baud))-1)

#define UP_DOWN 210
#define UP_UP 100
#define UP_DROP 130

#define DOWN_DROPPING 105
#define DOWN_RIGHT 120
#define DOWN_R1 140
#define DOWN_R2 160
#define DOWN_R3 180
#define DOWN_R4 200
#define DOWN_R5 220
#define DOWN_LEFT 240
#define DOWN_MIDDLE 140

volatile int coin_flag, pickup_flag, perim_flag, ISR_pw_up=UP_UP, ISR_pw_down = DOWN_MIDDLE, ISR_pw1=100, ISR_pw2=100, ISR_pw3=100, ISR_pw4=100, ISR_cnt=0, ISR_frc;
// The Interrupt Service Routine for timer 1 is used to generate one or more standard
// hobby servo signals.  The servo signal has a fixed period of 20ms and a pulse width
// between 0.6ms and 2.4ms.


int ADCRead(char analogPIN)
{
    AD1CHS = analogPIN << 16;    // AD1CHS<16:19> controls which analog pin goes to the ADC
 
    AD1CON1bits.SAMP = 1;        // Begin sampling
    while(AD1CON1bits.SAMP);     // wait until acquisition is done
    while(!AD1CON1bits.DONE);    // wait until conversion done
 
    return ADC1BUF0;             // result stored in ADC1BUF0
}


void __ISR(_TIMER_1_VECTOR, IPL5SOFT) Timer1_Handler(void)
{
	IFS0CLR=_IFS0_T1IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0

	ISR_cnt++;
	
	LATBbits.LATB6 = ISR_cnt>ISR_pw1?0:1;
	LATBbits.LATB10 = ISR_cnt>ISR_pw2?0:1;
	LATBbits.LATB0 = ISR_cnt>ISR_pw3?0:1;
	LATBbits.LATB1 = ISR_cnt>ISR_pw4?0:1;
	if(pickup_flag == 1){
		LATBbits.LATB3 = ISR_cnt>ISR_pw_down?0:1;
		LATBbits.LATB2 = ISR_cnt>ISR_pw_up?0:1;
	}
	/*if(ISR_cnt<ISR_pw)
	{
		LATBbits.LATB6 = 1;
	}
	else
	{
		LATBbits.LATB6 = 0;
	}*/
	
	if(ISR_cnt>=2000)
	{
		ISR_cnt=0; // 2000 * 10us=20ms
		ISR_frc++;	
	}
}

// Use the core timer to wait for 1 ms.
void wait_1ms(void)
{

    unsigned int ui;

    _CP0_SET_COUNT(0); // resets the core timer count

    


   // get the core timer count


   while ( _CP0_GET_COUNT() < (SYSCLK/(2*1000)) );
}

void waitms(int len)

{

	while(len--) wait_1ms();

}

#define PIN_PERIOD (PORTB&(1<<5))

// GetPeriod() seems to work fine for frequencies between 200Hz and 700kHz.
long int GetPeriod (int n)
{
	int i;
	unsigned int saved_TCNT1a, saved_TCNT1b;
	
    _CP0_SET_COUNT(0); // resets the core timer count
	while (PIN_PERIOD!=0) // Wait for square wave to be 0
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}

    _CP0_SET_COUNT(0); // resets the core timer count
	while (PIN_PERIOD==0) // Wait for square wave to be 1
	{
		if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
	}
	
    _CP0_SET_COUNT(0); // resets the core timer count
	for(i=0; i<n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIOD!=0) // Wait for square wave to be 0
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
		while (PIN_PERIOD==0) // Wait for square wave to be 1
		{
			if(_CP0_GET_COUNT() > (SYSCLK/4)) return 0;
		}
	}

	return  _CP0_GET_COUNT();
}


// The Interrupt Service Routine for timer 1 is used to generate one or more standard
// hobby servo signals.  The servo signal has a fixed period of 20ms and a pulse width
// between 0.6ms and 2.4ms.
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Timer2_Handler(void)
{
	IFS0CLR=_IFS0_T2IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0

	/*int adcval = 0;
	float voltage = 0;
	
	long int count;
	float T, f;

	adcval = ADCRead(12);
	voltage=adcval*3.3/1023.0;
	if(voltage>1.0){
		perim_flag = 1;
	}
	else{
		perim_flag = 0;
	}
	*/
	/*
	if(!pickup_flag){
		count = GetPeriod(30);
		if(count>0){
			T=(count*2.0)/(SYSCLK*100.0);
			f=1/T;
			if(f>192900.0){
				coin_flag = 1;
				//printf("%f", f);
			}
			else{
				coin_flag = 0;
			}
		}
		else{
			coin_flag = 0;
		}
	}*/
}

void SetupTimer1 (void)
{
	// Explanation here: https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
	PR1 =(SYSCLK/FREQ)-1; // since SYSCLK/FREQ = PS*(PR1+1)
	TMR1 = 0;
	T1CONbits.TCKPS = 0; // 3=1:256 prescale value, 2=1:64 prescale value, 1=1:8 prescale value, 0=1:1 prescale value
	T1CONbits.TCS = 0; // Clock source
	T1CONbits.ON = 1;
	IPC1bits.T1IP = 5;
	IPC1bits.T1IS = 0;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
	
	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}

void SetupTimer2 (void)
{
	// Explanation here: https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
	PR2 =(SYSCLK/FREQ2)-1; // since SYSCLK/FREQ = PS*(PR1+1)
	TMR2 = 0;
	T2CONbits.TCKPS = 0; // 3=1:256 prescale value, 2=1:64 prescale value, 1=1:8 prescale value, 0=1:1 prescale value
	T2CONbits.TCS = 0; // Clock source
	T2CONbits.ON = 1;
	IPC2bits.T2IP = 5;
	IPC2bits.T2IS = 0;
	IFS0bits.T2IF = 0;
	IEC0bits.T2IE = 1;
	
	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}

void UART2Configure(int baud_rate)
{
    // Peripheral Pin Select
    U2RXRbits.U2RXR = 4;    //SET RX to RB8
    RPB9Rbits.RPB9R = 2;    //SET RB9 to TX

    U2MODE = 0;         // disable autobaud, TX and RX enabled only, 8N1, idle=HIGH
    U2STA = 0x1400;     // enable TX and RX
    U2BRG = Baud2BRG(baud_rate); // U2BRG = (FPb / (16*baud)) - 1
    
    U2MODESET = 0x8000;     // enable UART2
}

void delay_ms (int msecs)
{	
	int ticks;
	ISR_frc=0;
	ticks=msecs/20;
	while(ISR_frc<ticks);
}

/* SerialReceive() is a blocking function that waits for data on
 *  the UART2 RX buffer and then stores all incoming data into *buffer
 *
 * Note that when a carriage return '\r' is received, a nul character
 *  is appended signifying the strings end
 *
 * Inputs:  *buffer = Character array/pointer to store received data into
 *          max_size = number of bytes allocated to this pointer
 * Outputs: Number of characters received */
unsigned int SerialReceive(char *buffer, unsigned int max_size)
{
    unsigned int num_char = 0;
 
    /* Wait for and store incoming data until either a carriage return is received
     *   or the number of received characters (num_chars) exceeds max_size */
    while(num_char < max_size)
    {
        while( !U2STAbits.URXDA);   // wait until data available in RX buffer
        *buffer = U2RXREG;          // empty contents of RX buffer into *buffer pointer

        while( U2STAbits.UTXBF);    // wait while TX buffer full
        U2TXREG = *buffer;          // echo
 
        // insert nul character to indicate end of string
        if( *buffer == '\r')
        {
            *buffer = '\0';     
            break;
        }
 
        buffer++;
        num_char++;
    }
 
    return num_char;
}

// Good information about ADC in PIC32 found here:
// http://umassamherstm5.org/tech-tutorials/pic32-tutorials/pic32mx220-tutorials/adc
void ADCConf(void)
{
    AD1CON1CLR = 0x8000;    // disable ADC before configuration
    AD1CON1 = 0x00E0;       // internal counter ends sampling and starts conversion (auto-convert), manual sample
    AD1CON2 = 0;            // AD1CON2<15:13> set voltage reference to pins AVSS/AVDD
    AD1CON3 = 0x0f01;       // TAD = 4*TPB, acquisition time = 15*TAD 
    AD1CON1SET=0x8000;      // Enable ADC
}

void Motor1Forward(void){
	ISR_pw1=1600;
	ISR_pw2=100;
}

void Motor2Forward(void){
	ISR_pw3=1550;
	ISR_pw4=150;
}

void Motor1Backward(void){
	ISR_pw1=200;
	ISR_pw2=1500;
}

void Motor2Backward(void){
	ISR_pw3=200;
	ISR_pw4=1500;
}

void GoForward(void){
	Motor1Forward();
	Motor2Forward();
}

void GoBackward(void){
	Motor1Backward();
	Motor2Backward();
}

void TurnRight(void){
	Motor1Forward();
	Motor2Backward();
}

void TurnLeft(void){
	Motor1Backward();
	Motor2Forward();
}

void TurnOff(void){
	ISR_pw1=0;
	ISR_pw2=0;
	ISR_pw3=0;
	ISR_pw4=0;
}

void TurnDegreeLeft(void){
	Motor1Forward();
	Motor2Backward();
	delay_ms(100);
	TurnOff();	
}

void TurnDegreeRight(void){
	Motor1Backward();
	Motor2Forward();
	delay_ms(100);
	TurnOff();	
}

void TurnXDegRight(int degrees){
	int running_total = 0;
	for (running_total = 0; running_total*5 < degrees; running_total++){
		TurnDegreeRight();
		delay_ms(90);
	}
}

void TurnXDegLeft(int degrees){
	int running_total = 0;
	for (running_total = 0; running_total*5 < degrees; running_total++){
		TurnDegreeLeft();
		delay_ms(90);
	}
}

void TurnXDeg(int degrees){
	if(degrees<=0){
		TurnXDegRight(-1*degrees);
	}
	else{
		TurnXDegLeft(degrees);
	}
}

/*void SlowUp(int final){
	while(ISR_pw_up<final){
		ISR_pw_up++;
		delay_ms(10);
	}
}*/

void PickUpCoin(void){
	pickup_flag = 1;
	//1
	ISR_pw_up = UP_UP;
	ISR_pw_down = DOWN_MIDDLE;
	delay_ms(1000);
	//2
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_MIDDLE;
	LATBbits.LATB15 = 1;
	delay_ms(1000);
	//3
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_RIGHT;
	delay_ms(500);
	//3.1
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_R1;
	delay_ms(500);
	//3.2
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_R2;
	delay_ms(500);
	//3.3
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_R3;
	delay_ms(500);
	//3.4
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_R4;
	delay_ms(500);
	//3.5
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_R5;
	delay_ms(500);
	//4
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_LEFT;
	delay_ms(1000);
	//5
	ISR_pw_up = UP_DOWN;
	ISR_pw_down = DOWN_MIDDLE;
	delay_ms(1000);
	//6
	ISR_pw_up = UP_UP;
	ISR_pw_down = DOWN_MIDDLE;
	delay_ms(1000);
	//7
	ISR_pw_up = UP_UP;
	ISR_pw_down = DOWN_DROPPING;
	delay_ms(1000);
	//8
	ISR_pw_up = UP_DROP;
	ISR_pw_down = DOWN_DROPPING;
	LATBbits.LATB15 = 0;
	delay_ms(1000);
	//9
	ISR_pw_up = UP_UP;
	ISR_pw_down = DOWN_DROPPING;
	delay_ms(1000);
	//10
	ISR_pw_up = UP_UP;
	ISR_pw_down = DOWN_MIDDLE;
	delay_ms(500);
	pickup_flag = 0;
		
}

void main (void)
{

	volatile unsigned long t = 0;
    char buf[32];
    int pw1, pw2;
    long int count;
	float T, f;
	
	int adcval = 0;
	float voltage = 0;
	
	int coinCounter = 0;
	
	int bonusFlag = 0, appFlag = 0;
	int bonusAngle = 0, bonusCoinPresent = 0;
	int bonusOn = 0, bonusGTG = 0, bonusStutterCount = 0;
	int stutterCoinSeen = 0, stutterEdgeSeen = 0, alignConfirmation = 0, obstacleMet = 0;
	int appComand = 0;
    
	DDPCON = 0;
	
	// Configure the pin we are using for servo control as output
	TRISBbits.TRISB6 = 0;
	LATBbits.LATB6 = 0;	
	
	TRISBbits.TRISB10 = 0;
	LATBbits.LATB10 = 0;	
	
	TRISBbits.TRISB0 = 0;
	LATBbits.LATB0 = 0;	
	
	TRISBbits.TRISB1 = 0;
	LATBbits.LATB1 = 0;	
	
	//arm servos
	TRISBbits.TRISB2 = 0;
	//LATBbits.LATB2 = 0;
		
	TRISBbits.TRISB3 = 0;
	//LATBbits.LATB3 = 0;	
	
	TRISBbits.TRISB15 = 0;
	//LATBbits.LATB15 = 0;
	
	// Configure pins as analog inputs
    ANSELBbits.ANSB12 = 1;   // set RB3 (AN5, pin 7 of DIP28) as analog pin
    TRISBbits.TRISB12 = 1;   // set RB3 as an input	
    
    ANSELBbits.ANSB13 = 1;
    TRISBbits.TRISB13 = 1;   // set RB3 as an input	
	
	ANSELBbits.ANSB14 = 1;
    TRISBbits.TRISB14 = 1;   // set RB3 as an input	
	
	INTCONbits.MVEC = 1;
	
	ADCConf(); // Configure ADC
	
	SetupTimer1(); // Set timer 5 to interrupt every 10 us
	//SetupTimer2(); // Set timer 5 to interrupt every 10 us

	CFGCON = 0;
    UART2Configure(115200);  // Configure UART2 for a baud rate of 115200
	
	ANSELB &= ~(1<<5); // Set RB10 as a digital I/O
    TRISB |= (1<<5);   // configure pin RB5 as input
	
    CNPUB |= (1<<5);   // Enable pull-up resistor for RB5
	
	
	// Give putty a chance to start
	delay_ms(500); // wait 500 ms
	
	/*printf("\x1b[2J\x1b[1;1H"); // Clear screen using ANSI escape sequence.
    printf("Servo signal generator for the PIC32MX130F064B.\r\n");
    printf("By Jesus Calvino-Fraga (c) 2018.\r\n");
    printf("Pulse width between 60 (for 0.6ms) and 240 (for 2.4ms)\r\n");
	*/
	//PickUpCoin();
	LATBbits.LATB15 = 0;
	
	//pickup_flag = 1;
	
	//PickUpCoin();
	//TurnOff();
	perim_flag = 0;
	coin_flag = 0;
	//GoForward();
	//TurnXDeg(15);
	/*delay_ms(1000);
	TurnXDeg(-15);
	delay_ms(500);
	TurnXDeg(15);
	delay_ms(500);*/
	
	//TurnDegreeRight();
	
	//TurnXDeg(-45);
	
	//printf("%f\r", ADCRead(11));
	if(ADCRead(11)>3.0){
		delay_ms(50);
		if(ADCRead(11)>3.0){
			bonusFlag = 1;
		}
		else{
			bonusFlag = 0;
		}
	}
	else{
		bonusFlag = 0;
	}
	
	if(ADCRead(10)>3.0){
		delay_ms(50);
		if(ADCRead(10)>3.0){
			appFlag = 1;
		}
		else{
			appFlag = 0;
		}
	}
	else{
		appFlag = 0;
	}
	
	//printf("%d\n", bonusFlag);
	//printf("%d\n", appFlag);
	
	while (1)
	{
		/*adcval = ADCRead(12); // note that we call pin AN5 (RB3) by it's analog number
        voltage=adcval*3.3/1023.0;
        //printf("AN5=0x%04x, %.3fV\r", adcval, voltage);
        if(voltage>2.0){
        	printf("YEEEEET!!!!!!\r");
        }
        fflush(stdout);*/
        
        if((!bonusFlag || appFlag) && coinCounter < 20){
			
			if(appFlag){
				fflush(stdout);
				SerialReceive(buf, sizeof(buf)-1); // wait here until data is received
				//printf("\n");
				fflush(stdout);
				appComand=atoi(buf);
				
				//Turn left
				if(appComand == 000){
					TurnLeft();
				}
				//right
				if(appComand == 001){
					TurnRight();
				}
				//up
				if(appComand == 002){
					GoForward();
				}
				//down
				if(appComand == 003){
					GoBackward();
				}
				//stop
				if(appComand == 004){
					TurnOff();
				}
				//pick up
				if(appComand == 005){
					PickUpCoin();
				}
				else{
					printf("WTF\n");
				}
				
				delay_ms(1000);				
				
			}
			
			else{
        
				//printf("New pulse width: ");
				//fflush(stdout);
				
		 
				//printf("\n");
				//fflush(stdout);
				//pw1=atoi(buf);
				//if( (pw1>=60) && (pw1<=240) )
				//{
				//ISR_pw1=pw1;
				//ISR_pw2=240-(pw1-60);
				//}
				//else
				//{
				//	printf("%d is out of the valid range\r\n", pw1);
				//}
				
				
				GoForward();
				
				count = GetPeriod(300);
				//printf("%d", count);
				if(count>0){
					T=(count*2.0)/(SYSCLK*100.0);
					f=1/T;
					coin_flag = 0;
					//printf("%f\r", f);
					if(f>19289.0){
						//GoBackward();
						TurnOff();
						delay_ms(100);
						count = GetPeriod(300);
						T=(count*2.0)/(SYSCLK*100.0);
						f=1/T;
						if(f>19289.0){
							coin_flag = 1;
						}
						
						//printf("%f\r", f);
					}
				}
				else{
					coin_flag = 0;
				}
				
				adcval = ADCRead(12);
				voltage=adcval*3.3/1023.0;
				//printf("%f\r", voltage);
				if(voltage>0.5){
					perim_flag = 1;
				}
				else{
					perim_flag = 0;
				}
				
				if(perim_flag==1){
					//printf("YEEEETT!!!!!!!\r");
					GoBackward();
					delay_ms(500);
					TurnLeft();
					delay_ms(1000);
					perim_flag = 0;			
				}
				
				if(coin_flag==1){
					pickup_flag = 1;
					//printf("YEEEETT2.0!!!!!!!\r");
					GoBackward();
					delay_ms(200);
					TurnOff();
					PickUpCoin();
					coin_flag = 0;
					coinCounter++;
				}
				
				waitms(10);  
				
				//PickUpCoin();
				
				/*
				GoBackward();
				delay_ms(2000);
				GoBackward();
				delay_ms(2000);
				TurnLeft();
				delay_ms(2000);
				TurnRight();
				delay_ms(2000);
				TurnOff();
				PickUpCoin();
				*/
			}
		}
		else{
			
			printf("check\n\r");
			fflush(stdout);
		    SerialReceive(buf, sizeof(buf)-1); // wait here until data is received

		 
		    //printf("\n");
		    fflush(stdout);
			bonusCoinPresent=atoi(buf);
			
			//printf("%d\n\r", bonusCoinPresent);
		
			printf("on\n\r");
			fflush(stdout);
	    	SerialReceive(buf, sizeof(buf)-1); // wait here until data is received

	 
	    	//printf("\n");
	    	fflush(stdout);
		    bonusOn=atoi(buf);
		   
		    while(bonusOn != 111){};
		    
		    while(1){		    
		    
			    printf("check\n\r");
			    fflush(stdout);
		    	SerialReceive(buf, sizeof(buf)-1); // wait here until data is received

		 
		    	//printf("\n");
		    	fflush(stdout);
			    bonusCoinPresent=atoi(buf);
			    
			    if(bonusCoinPresent!=666){
					delay_ms(1000);
					TurnXDeg(bonusCoinPresent);
					delay_ms(500);
					printf("confirm\n\r");
					
					fflush(stdout);
					SerialReceive(buf, sizeof(buf)-1); // wait here until data is received

			 
					//printf("\n");
					fflush(stdout);
					alignConfirmation=atoi(buf);
					
					if(alignConfirmation == 222){
					
						obstacleMet = 0;
						while(!obstacleMet){
							GoForward();
	        
							count = GetPeriod(300);
							if(count>0){
								T=(count*2.0)/(SYSCLK*100.0);
								f=1/T;
								coin_flag = 0;
								//printf("%f\r", f);
								if(f>19289.0){
									//GoBackward();
									TurnOff();
									delay_ms(100);
									count = GetPeriod(300);
									T=(count*2.0)/(SYSCLK*100.0);
									f=1/T;
									if(f>19289.0){
										coin_flag = 1;
									}
									
									//printf("%f\r", f);
								}
							}
							else{
								coin_flag = 0;
							}
							
							adcval = ADCRead(12);
							voltage=adcval*3.3/1023.0;
							//printf("%f\r", voltage);
							if(voltage>0.5){
								perim_flag = 1;
							}
							else{
								perim_flag = 0;
							}
							
							if(perim_flag==1){
								//printf("YEEEETT!!!!!!!\r");
								GoBackward();
								delay_ms(500);
								TurnLeft();
								delay_ms(1000);
								perim_flag = 0;	
								obstacleMet = 1;
							}
							
							if(coin_flag==1){
								pickup_flag = 1;
								//printf("YEEEETT2.0!!!!!!!\r");
								GoBackward();
								delay_ms(200);
								TurnOff();
								PickUpCoin();
								coin_flag = 0;
								coinCounter++;
								obstacleMet = 1;
							}
							
							waitms(10);  
						
						}
					
					}
					
			    }
			    else{
			    	//THIS IS HOW FAR THE GRID SEES--------------
					bonusStutterCount =0;
					stutterCoinSeen = 0;
					stutterEdgeSeen = 0;
					while(bonusStutterCount<10 && !stutterCoinSeen && !stutterEdgeSeen){
			    		GoForward();
						
						waitms(150);
	        
						count = GetPeriod(300);
						if(count>0){
							T=(count*2.0)/(SYSCLK*100.0);
							f=1/T;
							coin_flag = 0;
							//printf("%f\r", f);
							if(f>19289.0){
								//GoBackward();
								TurnOff();
								delay_ms(100);
								count = GetPeriod(300);
								T=(count*2.0)/(SYSCLK*100.0);
								f=1/T;
								if(f>19289.0){
									coin_flag = 1;
								}
								
								//printf("%f\r", f);
							}
						}
						else{
							coin_flag = 0;
						}
						
						adcval = ADCRead(12);
						voltage=adcval*3.3/1023.0;
						//printf("%f\r", voltage);
						if(voltage>0.5){
							perim_flag = 1;
						}
						else{
							perim_flag = 0;
						}
						
						if(perim_flag==1){
							//printf("YEEEETT!!!!!!!\r");
							GoBackward();
							delay_ms(500);
							TurnLeft();
							delay_ms(1000);
							perim_flag = 0;
							stutterEdgeSeen = 1;
						}
						
						if(coin_flag==1){
							pickup_flag = 1;
							//printf("YEEEETT2.0!!!!!!!\r");
							GoBackward();
							delay_ms(200);
							TurnOff();
							PickUpCoin();
							coin_flag = 0;
							coinCounter++;
							stutterCoinSeen = 1;
						}
						
						bonusStutterCount++;
						waitms(10);
			    	}
			    	
			    }
			    
			    waitms(10);  	
		    
		    }	    
		
		}	
		
		waitms(10);  
        
	}
}

