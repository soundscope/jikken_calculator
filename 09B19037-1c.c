/* ###################################################################
**     Filename    : main.c
**     Project     : kadai-c
**     Processor   : MKL46Z256VLL4
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2021-06-08, 18:37, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* User includes (#include below this line is not maintained by Processor Expert) */
enum BUTTON_STATUS { OFF = -1, UNSTABLE, ON };
enum BUTTON_WAS_PRESSED_IN_THE_PAST { NEVER_PRESSED, HAS_PRESSED };
enum LED { LED_OFF, LED_ON };
const int THRESHOLD = 100;


void init()
{
    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
    PORTA_PCR1 = PORT_PCR_MUX(1);
    GPIOA_PDDR &= ~(1 << 1);

    SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
    PORTD_PCR5 = PORT_PCR_MUX(1);
    GPIOD_PDDR |= 1 << 5;

    PORTA_PCR14 = PORT_PCR_MUX(1);
    PORTA_PCR15 = PORT_PCR_MUX(1);
    PORTA_PCR16 = PORT_PCR_MUX(1);
    PORTA_PCR17 = PORT_PCR_MUX(1);

    GPIOA_PDDR |= 0xF << 14;
}



int is_button_release_on_hardware()
{
    return ((GPIOA_PDIR >> 1) & 1);
}

int button_status_on_software(int anti_chattaring_counter, const int THRESHOLD)
{
    if (anti_chattaring_counter < -THRESHOLD)
        return OFF;
    if (anti_chattaring_counter > THRESHOLD)
        return ON;
    return UNSTABLE;
}



void output_hex_one_digit(char c)
{
    GPIOA_PCOR = 0xf << 14;     // 14..17 clear
    GPIOA_PSOR = (c - '0') << 14;       //NOTE: GPIOA14~(14+32) may be changed
}


void greenLED(int on_off)
{
    if (on_off)
        GPIOD_PCOR = 1 << 5;    // ON
    else
        GPIOD_PSOR = 1 << 5;    // OFF
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
/* Write your local variable definition here */

/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
    PE_low_level_init();
/*** End of Processor Expert internal initialization.                    ***/

/* Write your code here */
/* For example: for(;;) { } */
    int cnt = 0;
    init();
    output_hex_one_digit('0' + cnt++);
    for (;;) {
        char anti_chattaring_counter = 0;
        int pressed = NEVER_PRESSED;
        for (;;) {
            int button_status =
                button_status_on_software(anti_chattaring_counter, THRESHOLD);
            if (is_button_release_on_hardware()) {
                if (button_status != ON)
                    anti_chattaring_counter = ++anti_chattaring_counter *
                        (button_status == UNSTABLE);
//means if button_status == OFF then anti_chattaring_counter = 0
//and if button_status == UNSSTABLE then anti_chattaring_counter++
            } else {
                if (button_status != OFF)
                    anti_chattaring_counter = --anti_chattaring_counter *
                        (button_status == UNSTABLE);
//means if button_status == ON then anti_chattaring_counter = 0
//and if button_status == UNSSTABLE then anti_chattaring_counter++
            }
            if (button_status == UNSTABLE)
                continue;
            if (button_status == ON) {
                greenLED(LED_ON);       // for debug
                pressed = HAS_PRESSED;
                continue;
            }
            if (pressed == NEVER_PRESSED)
                continue;
//now button is released && the button was pressed once in the past
            greenLED(LED_OFF);  // for debug
            pressed = NEVER_PRESSED;    // have to initialize
            output_hex_one_digit('0' + cnt++);
            cnt %= 10;
        }
    }


  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
    PEX_RTOS_START();           /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
    for (;;) {
    }
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
