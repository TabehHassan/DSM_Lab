#include <xc.h>
#include <stdint.h>

// Configuration bits (adjust if needed)
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 4000000

// Global variables
volatile uint16_t previous_capture = 0;
volatile uint16_t current_capture = 0;
volatile uint16_t overflow_count = 0;
volatile uint32_t total_ticks = 0;

void __interrupt() ISR(void)
{
    // Timer1 Overflow Interrupt
    if (PIR1bits.TMR1IF)
    {
        overflow_count++;          // Count each overflow
        PIR1bits.TMR1IF = 0;       // Clear flag
    }

    // CCP1 Capture Interrupt
    if (PIR1bits.CCP1IF)
    {
        // Read captured value
        current_capture = ((uint16_t)CCPR1H << 8) | CCPR1L;

        uint16_t delta;

        // 16-bit subtraction (auto handles one overflow)
        delta = current_capture - previous_capture;

        // ----- 32-bit rollover correction -----
        if (current_capture < previous_capture)
        {
            // One overflow already handled by subtraction
            total_ticks = ((uint32_t)(overflow_count - 1) * 65536UL) + delta;
        }
        else
        {
            total_ticks = ((uint32_t)overflow_count * 65536UL) + delta;
        }

        // Reset overflow counter for next cycle
        overflow_count = 0;

        // Update previous capture
        previous_capture = current_capture;

        PIR1bits.CCP1IF = 0;   // Clear CCP flag
    }
}

void main(void)
{
    // I/O Configuration
    TRISC2 = 1;   // CCP1 input (RC2)
    TRISD0 = 0;   // LED output
    PORTDbits.RD0 = 0;

    // Timer1 Configuration
    T1CON = 0x01;   // Timer1 ON, prescaler 1:1, internal clock

    // CCP1 Configuration (Capture mode, rising edge)
    CCP1CON = 0x05; // CCP1M3:CCP1M0 = 0101 → Rising edge capture

    // Enable Interrupts
    PIE1bits.TMR1IE = 1;   // Timer1 overflow interrupt
    PIE1bits.CCP1IE = 1;   // CCP1 interrupt
    INTCONbits.PEIE = 1;   // Peripheral interrupts
    INTCONbits.GIE = 1;    // Global interrupts

    while (1)
    {
        // 10 Hz → 100,000 ticks (1 µs per tick)
        if (total_ticks >= 99900 && total_ticks <= 100100)
        {
            PORTDbits.RD0 = 1;   // LED ON (Frequency Lock)
        }
        else
        {
            PORTDbits.RD0 = 0;   // LED OFF
        }
    }
}