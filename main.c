
#include "io430.h"

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;  
  
  // Disable interrupts for the setup
  __disable_interrupt();

  // Set up clock system (remember its hex)
  BCSCTL1 = 0x0080;
  BCSCTL2 = 0x0088;
  BCSCTL3 = 0x0004;
    
  // Set up the timer A - Use the system master crystal and have it count up
  // With this crystal TACCR will be 32768
  TACTL = TACLR | TASSEL0; //clear timer A, Select clock source 0
  TACCR0 = 32768;
  
  // Setting up the ADC, will make some changes later
  ADC10CTL0 = REFON | REFBURST | ADC10SR | ADC10SHT0 | ADC10SHT1 | SREF0 ;
  ADC10CTL1 = 0x0000; // THIS SETS INCHx TO A0
  ADC10AE0 = BIT0; //ENABLE THE 0 PIN FOR ADC USE

  //Setup is done------------------------
  
  //Enable interrupts now that setup is done
  __enable_interrupt();
  //Starting the timer in up mode
  TACTL |= MC0;   // going into up mode starts the clock!
  
  // ADC STUFF---------------------------
  // Turn on the ADC and ref voltage
  ADC10CTL0 |=  ADC10ON + REFON + ENC + ADC10SC;
  
    
  
  
  return 0;
}
