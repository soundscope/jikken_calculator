/* ###################################################################
 **     Filename    : main.c
 **     Project     : kadai1e
 **     Processor   : MKL46Z256VLL4
 **     Version     : Driver 01.01
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2021-05-13, 11:38, # CodeGen: 0
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 01.01
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
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
enum READ_PHASE { INIT, PRE_NUM, OPE, END_OPE, POST_NUM, CALC, OUTPUT, END,
        ERROR_END };
enum ERROR_CODE { SUCCESS, SYNTAX_ERROR, TOO_LARGE, FINISH };
enum OPERATOR   { PLUS = 10, MINUS, ASTERISK, SLASH, NONE, EQUAL };
enum BUTTON_STATUS { OFF = -1, UNSTABLE, ON };
enum LED { LED_OFF, LED_ON };
enum BUTTON_WAS_PRESSED_IN_THE_PAST { NEVER_PRESSED, HAS_PRESSED };
const int THRESHOLD = 100;
const int MAXI = 2 << 10;       //(2 << 10 == 1024)
const int MOD = 10007;          // prime

void init()
{
    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
    PORTA_PCR1 = PORT_PCR_MUX(1);
    GPIOA_PDDR &= ~(1 << 1);

    SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
    PORTD_PCR5 = PORT_PCR_MUX(1);
    GPIOD_PDDR |= 1 << 5;

    SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
    PORTB_PCR0 = PORT_PCR_MUX(1);
    PORTB_PCR1 = PORT_PCR_MUX(1);
    PORTB_PCR2 = PORT_PCR_MUX(1);
    PORTB_PCR3 = PORT_PCR_MUX(1);

    PORTA_PCR14 = PORT_PCR_MUX(1);
    PORTA_PCR15 = PORT_PCR_MUX(1);
    PORTA_PCR16 = PORT_PCR_MUX(1);
    PORTA_PCR17 = PORT_PCR_MUX(1);
    GPIOB_PDDR &= ~0xF;
    GPIOA_PDDR |= 0xF << 14;
}



int is_button_release_on_hardware()
{
    return  ((GPIOA_PDIR >> 1) & 1);
}

int button_status_on_software(int anti_chattaring_counter, const int THRESHOLD)
{
    if (anti_chattaring_counter < -THRESHOLD)
        return OFF;
    if (anti_chattaring_counter > THRESHOLD)
        return ON;
    return UNSTABLE;
}

int read_hex_one_digit()
{
    int input_num = 0;
    for (int i = 0; i < 4; i++)
        input_num = (input_num << 1) | ((GPIOB_PDIR >> 3 - i) & 1);
    return input_num;
}

void output_hex_one_digit(char c)
{
    GPIOA_PCOR = 0xf << 14;     // 14..17 clear
    GPIOA_PSOR = (c - '0') << 14;       //NOTE: GPIOA14~(14+32) may be changed
}

int output(char *stack_top)
{
    output_hex_one_digit(*stack_top);
    return *(stack_top - 1);
}

int is_operator(int input)
{
    return PLUS <= input && input <= SLASH;
}

int is_decimal(int input)
{
    return 0 <= input && input <= 9;
}

int read_and_check_syntax(int input, int *phase, int *pre_number,
                          int *post_number, int *ope, char **stack_top)
{
    int remain_ans;
    switch (*phase) {
    case INIT:
        /* Falls through. */
    case PRE_NUM:
        if (is_decimal(input)) {
            output_hex_one_digit(input + '0');
            *phase = PRE_NUM;   //confirm pre_number is not null string
            *pre_number = (*pre_number * 10) + input;
            if (*pre_number > MAXI)
                return TOO_LARGE;
            return SUCCESS;
        } else if (is_operator(input) && *phase == PRE_NUM)
            *phase = OPE;
        /* To exclude input == OPERATOR &&
           phase == INIT (means pre_number is null string) */
        /* GOTO case OPE */
        else
            return SYNTAX_ERROR;
        /* Falls through. */
    case OPE:                  //not be jumped here (except for Fall through)
        // input == OPERATOR is ensured
        output_hex_one_digit(input + '0');
        *ope = input;
        *phase = END_OPE;
        return SUCCESS;
    case END_OPE:
        /* Falls through. */
    case POST_NUM:
        if (is_decimal(input)) {
            output_hex_one_digit(input + '0');
            *phase = POST_NUM;  //confirm post_number is not null string
            *post_number = (*post_number * 10) + input;
            if (*pre_number > MAXI)
                return TOO_LARGE;
            return SUCCESS;
        } else if (input == EQUAL && *phase == POST_NUM)
            *phase = CALC;
        /* To exclude input == EQUAL &&
                          phase == END_OPE (means post_number is null string) */
        /* GOTO case CALC */
        else
            return SYNTAX_ERROR;
        /* Falls through. */
    case CALC:
                               //not be jumped here (except for Fall through)
        if (*ope == PLUS)
            remain_ans = *pre_number + *post_number;
        if (*ope == MINUS)
            remain_ans = ((*pre_number - *post_number) + MOD * 1000) % MOD;
        if (*ope == ASTERISK)
            remain_ans = *pre_number * *post_number;
        if (*ope == SLASH){
            if(*post_number == 0) return SYNTAX_ERROR;
            remain_ans = *pre_number / *post_number;
        }

        for (;;) {
            *((*stack_top)++) = '0' + remain_ans % 10;
            if ((remain_ans /= 10) == 0)
                break;
        }
        *phase = OUTPUT;
        /* GOTO case OUTPUT */
        /* Falls through. */
    case OUTPUT:
        if (input != EQUAL)
            return SYNTAX_ERROR;        // have to ensure
        if (output(--(*stack_top)) == 0)
            *phase = END;
        // output() returns 0 if len of remaining stack data is 0;
        return SUCCESS;
    case END:
        if (input != EQUAL)
            return SYNTAX_ERROR;
        return FINISH;
    case ERROR_END:
        return FINISH;
    }
    return SUCCESS;             // dead code
}

void input_error_handle()
{
    output_hex_one_digit('0' + 14);
}

void too_large_error_handle()
{
    output_hex_one_digit('0' + 13);
}

void initLED()
{
    output_hex_one_digit('0' + 15);
}

void greenLED(int on_off)
{
    if (on_off)
        GPIOD_PCOR = 1 << 5;    // ON
    else
        GPIOD_PSOR = 1 << 5;    // OFF
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
    /* Write your local variable definition here */

        /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
    PE_low_level_init();
        /*** End of Processor Expert internal initialization.                    ***/

    /* Write your code here */
    /* For example: for(;;) { } */
    init();
    for (;;) {
        //be jumped here if finish outputting or detect an error
        //init_for_calculation
        int pre_number = 0;
                              // e.g. 30 * 4 -> 30
        int post_number = 0;
                              // e.g. 10 + 40 -> 40
        int ope = 0;
                          // e.g. +(10), -(11), *(12), /(13)
        int phase = INIT;
        char stack[100] = { 0 };        // stack array
        char *stack_top = stack + 1;
        char anti_chattaring_counter = 0;
        int pressed = NEVER_PRESSED;
        initLED();
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
            int status_code =
                read_and_check_syntax(read_hex_one_digit(), &phase,
                                      &pre_number, &post_number, &ope,
                                      &stack_top);
            if (status_code == FINISH)
                break;
            if (status_code == SYNTAX_ERROR)
                input_error_handle();
            if (status_code == TOO_LARGE)
                too_large_error_handle();
            if (status_code != SUCCESS)
                phase = ERROR_END;      // detect an error
        }
    }


        /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
        /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
    PEX_RTOS_START();
                       /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
        /*** End of RTOS startup code.  ***/
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
 **     This file was created by Processor Expert 10.5 [05.21]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
