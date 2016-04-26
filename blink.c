#include <msp430.h>

#define LED_BIT BIT0

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  P1DIR |= LED_BIT;
  while(1) {
    P1OUT |= LED_BIT;
    for (int i = 0; i < 10000; i++);
    P1OUT &= ~LED_BIT;
    for (int i = 0; i < 10000; i++);
  }
}
