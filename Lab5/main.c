/*
 * File: Lab5_Ex3_MotorControl.c
 * Target: PIC16F877A @ 4MHz
 * Description: 10-Bit Dynamic Motor Controller with 10% increments
 */

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 4000000
#pragma config FOSC = XT, WDTE = OFF, LVP = OFF, BOREN = ON

// Global variable for 10-bit duty cycle (0 to 1000)
uint16_t duty_reg = 0; 

int main(void) {
    // 1. GPIO Configuration
    TRISCbits.TRISC2 = 0;    // RC2/CCP1 as Output for Motor [cite: 209]
    TRISBbits.TRISB0 = 1;    // RB0 as Input (Speed UP) 
    TRISBbits.TRISB1 = 1;    // RB1 as Input (Speed DOWN) 

    // 2. Timer2 Configuration (4kHz Frequency)
    T2CONbits.T2CKPS = 0b00; // Prescaler 1:1 [cite: 210]
    PR2 = 249;               // Set Period for 4 kHz [cite: 212]
    T2CONbits.TMR2ON = 1;    // Turn on Timer2 [cite: 215]

    // 3. Initial CCP1 Configuration (0% Duty Cycle)
    CCP1CON = 0b00001100;    // Standard PWM Mode [cite: 233]
    CCPR1L = 0;

    while(1) {
        // --- Speed UP Logic ---
        if (PORTBbits.RB0 == 0) {       // Button pressed (Active Low)
            __delay_ms(20);             // Software debouncing 
            if (PORTBbits.RB0 == 0) {
                if (duty_reg <= 900) {  // Boundary check [cite: 291]
                    duty_reg += 100;    // 10% increment 
                }
                while(PORTBbits.RB0 == 0); // Wait for button release
            }
        }

        // --- Speed DOWN Logic ---
        if (PORTBbits.RB1 == 0) {
            __delay_ms(20);             // Software debouncing 
            if (PORTBbits.RB1 == 0) {
                if (duty_reg >= 100) {  // Boundary check [cite: 292]
                    duty_reg -= 100;    // 10% decrement 
                }
                while(PORTBbits.RB1 == 0); // Wait for button release
            }
        }

        // --- Apply 10-bit Duty Cycle to Hardware Registers ---
        // Upper 8 bits to CCPR1L, Lower 2 bits to CCP1CON <5:4> [cite: 161, 162]
        CCPR1L = duty_reg >> 2; 
        CCP1CON = (CCP1CON & 0xCF) | ((duty_reg & 0x03) << 4);
        return 0;
    }
}