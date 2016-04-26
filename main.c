#include <msp430.h>

// Global variables
static volatile unsigned int gTemp = 0;        // Raw ADC10 value from the most recent conversion
static volatile unsigned int gTimerTicks = 0;  // Total number of seconds
static volatile char gState = 0;               // Keep track of which operations need to be done while in the main loop

#define ADCMATH BIT0
#define TATICK	BIT1

int main( void )
{
  // The temperature reading from the external temperature sensor (after conversion from ADC counts)
  unsigned int temperature = 25;
  unsigned int timeLimit = 480;

  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  // Disable interrupts for the setup
  //__disable_interrupt();

  // Enable the crystal osc fault. The xtal startup process
  IE1 |= OFIE;     // An immedate Osc Fault will occur next

  // Set up clock system
  //BCSCTL1 = 0x0080;
  BCSCTL1 |= XT2OFF; //Turn off XT2, it is not used for MCLK or SMCLK
  BCSCTL2 &= RSEL0;

  //BCSCTL2 = 0x0088;
  BCSCTL2 |= SELM_2 + SELS; //XT2CLK for MCLK and SMCLK

  //BCSCTL3 = 0x0004;
  BCSCTL3 |= XCAP_1;

  // Set up the timer A - Use the system master crystal and have it count up
  // With this crystal TACCR will be 32768
  TACTL = TACLR | TASSEL0; // Clear timer A, Select clock source 0 (LFXTAL)
  TACCR0 = 32768;          // 0ne Second PLZ!

  // Setting up the ADC, will make some changes later
  ADC10CTL0 = REFON | REFBURST | ADC10SR | ADC10SHT0 | ADC10SHT1 | SREF0 ;
  ADC10CTL1 = 0x0000; // THIS SETS INCHx TO A0
  ADC10AE0 = BIT0; //ENABLE THE 0 PIN FOR ADC USE

  //Setup is done------------------------

  //Enable interrupts now that setup is done
  __enable_interrupt();

  // Enable the compare ISR on TimerA CCR0
  TACCTL0 |= CCIE;

  //Starting the timer in up mode
  TACTL |= MC0;   // going into up mode starts the (one second) timer!

  // ADC STUFF---------------------------
  // Turn on the ADC and ref voltage
  ADC10CTL0 |=  ADC10ON + REFON + ENC + ADC10SC; //this will also take readings and start conversion, ADC interrupts when conversion is done

  //LOOPS
  while(1)
  {
    if(gState & ADCMATH){
      // Add all the conversion math goes here!!
      temperature = gTemp;
    }
    else if(gState & TATICK){
      if(gTimerTicks >= timeLimit){

      }
    }
  else __low_power_mode_0();
  }
  return 0;
}

#pragma vector=ADC10_VECTOR
void __attribute__ ((interrupt(ADC10_VECTOR))) adc10_isr (void)
{
  gTemp = ADC10MEM; 			// Store the ADC result in the global variable gTemp
  gState |= ADCMATH;			// Set this state so that the result can be processed in the main loop
  __low_power_mode_off_on_exit();	// Wake the processor when this ISR exits
}

void __attribute__ ((interrupt(TIMERA0_VECTOR))) timera0_isr (void)
{
  gTimerTicks++;			// Add one to the timer counter
  ADC10CTL0 |= ADC10SC;			// Start another conversion
  TACCTL0 &= ~CCIFG;			// Clear the timer interrupt flag
  gState |= TATICK;			// Set the state so that the main loop will check the elapsed time
  __low_power_mode_off_on_exit();	// Wake the processor when this ISR exits
}

// DONT USE THIS ONE!!!!! AAAAAAAA X____X
// void __attribute__ ((interrupt(TIMERA1_VECTOR))) timera1_isr (void)
// {
//   //NOPE
//   __no_operation();
// }

/*********************************************
The LF XTAL error flag is going to get set
when the crystal starts up. Catch it and clear
it here.

(Warning: We arent checking for any
          other sources of NMIs!)
**********************************************/
void __attribute__ ((interrupt(NMI_VECTOR))) nmi_isr (void)
{
  __no_operation();
  unsigned int i;
  do
  {
    IFG1 &= ~OFIFG;                         // Clear OSCFault flag
    for (i = 0xFFF; i > 0; i--);            // Time for flag to set
    //P1OUT ^= 0x01;                          // Toggle P1.0 using exclusive-OR
  }
  while (IFG1 & OFIFG);                     // OSCFault flag still set?
  IE1 |= OFIE;                              // Enable Osc Fault
}
