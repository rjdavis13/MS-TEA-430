
#include "io430.h"

static volatile unsigned int temp = 0;
static volatile unsigned int timerTicks = 0;

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;  
  
  // Disable interrupts for the setup
  //__disable_interrupt();

  // Enable the crystal osc fault. The xtal startup process
  IE1 |= OFIE;     // An immedate Osc Fault will occur next

  // Set up clock system (remember its hex) also for future ref, no MAGIC Numbers! I mean do you remember what any of these do?! now you have to look them up again
  BCSCTL1 = 0x0080;
  BCSCTL2 = 0x0088;
  BCSCTL3 = 0x0004;
  
  // Set up the timer A - Use the system master crystal and have it count up
  // With this crystal TACCR will be 32768
  TACTL = TACLR | TASSEL0; //clear timer A, Select clock source 0
  TACCR0 = 32768;          // 0ne Second PLZ! 
  
  // Setting up the ADC, will make some changes later
  ADC10CTL0 = REFON | REFBURST | ADC10SR | ADC10SHT0 | ADC10SHT1 | SREF0 ;
  ADC10CTL1 = 0x0000; // THIS SETS INCHx TO A0
  ADC10AE0 = BIT0; //ENABLE THE 0 PIN FOR ADC USE

  //Setup is done------------------------
  
  //Enable interrupts now that setup is done
  __enable_interrupt();
  
  // Enable the compare ISR.
  TACCTL0 |= CCIE;
  //Starting the timer in up mode
  TACTL |= MC0;   // going into up mode starts the (one second) timer!
  
  // ADC STUFF---------------------------
  // Turn on the ADC and ref voltage
  ADC10CTL0 |=  ADC10ON + REFON + ENC + ADC10SC; //this will also take readings and start conversion, ADC interrupts when conversion is done
  
  //LOOPS
  while(1)
  {
    __low_power_mode_0();
  }
  return 0;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) 
{
	temp = ADC10MEM; //temp variable is set to the value the ADC has just finished converting
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void)
{
  timerTicks++;
  ADC10CTL0 |= ADC10SC; // Start another conversion
  TACCTL1 &= ~CCIFG;
}

// DONT USE THIS ONE!!!!! AAAAAAAA X____X
#pragma vector=TIMERA1_VECTOR
__interrupt void TIMERA1_ISR(void)
{
  //NOPE
  __no_operation();
}

/*********************************************
The LF XTAL error flag is going to get set
when the crystal starts up. Catch it and clear
it here.

(Warning: We arent checking for any
          other sources of NMIs!)
**********************************************/
#pragma vector=NMI_VECTOR
__interrupt void nmi_ (void)
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
