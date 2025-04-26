/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes



// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************


//SECTION: DEFINES
#define ADC_VREF                (3.3f)
#define ADC_MAX_COUNT           (4095)
#define SAMPLE_LEN 100
#define ADC_SRC_ADDR (const void *)((&ADCDATA0) + ADCHS_CH2)
#define ADC_SRC_SIZE sizeof(uint16_t)


//SECTION: GLOBAL VARIABLES
volatile bool result_ready = false;


__attribute__((aligned(16))) __COHERENT uint16_t adc_buf[SAMPLE_LEN];

static void ADC_ResultReadyCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    /* Read the ADC result */
    ADCHS_ChannelResultInterruptDisable(ADCHS_CH2);
    DMAC_ChannelDisable(DMAC_CHANNEL_0);

    result_ready = true;
}

int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);

    //Setup DMA interupt callback and start a transfer
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, ADC_ResultReadyCallback, 0);
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, ADC_SRC_ADDR, ADC_SRC_SIZE, adc_buf, sizeof (adc_buf), sizeof (uint16_t));
    
    TMR3_Start(); //turn on Timer 3 to trigger ADC


    while (true) {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();

        /* Wait till ADC conversion result is available */
        if (result_ready == true) {

            result_ready = false;
            for (int i = 0; i < SAMPLE_LEN; i++) {
                float input_voltage = (float) adc_buf[i] * ADC_VREF / ADC_MAX_COUNT;

                printf(">val:%.2f \n", input_voltage);

            }

            //delay
            for (int i = 0; i < 10; i++) {
                TMR2_Start();
                TMR2 = 0x0;
                while (TMR2_CounterGet() < 30000); // Wait for 1000 timer ticks
                TMR2_Stop();
            }
            
            //start new DMA transfer
            DMAC_ChannelTransfer(DMAC_CHANNEL_0, ADC_SRC_ADDR, ADC_SRC_SIZE, adc_buf, sizeof (adc_buf), sizeof (uint16_t));
            ADCHS_ChannelResultInterruptEnable(ADCHS_CH2);
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE);
}


/*******************************************************************************
 End of File
 */

