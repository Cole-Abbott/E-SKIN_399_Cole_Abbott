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
#define SAMPLE_LEN 500
#define ADC_SRC_ADDR_2 (const void *)((&ADCDATA0) + ADCHS_CH2)
#define ADC_SRC_SIZE 3*sizeof(uint32_t) // *3 to capture ADCDATA2-ADCDATA4 in 1 transfer


//SECTION: GLOBAL VARIABLES
volatile bool ADC_2_result_ready = false;


__COHERENT uint32_t adc_buf[3*SAMPLE_LEN];
__COHERENT uint32_t adc_data[2*SAMPLE_LEN];



static void ADC_2_ResultReadyCallback(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    ADC_2_result_ready = true;
}


int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);

    
    ADCGIRQEN1bits.AGIEN2 = 1;	//Enb ADC2 AN2 interrupts for DMA
//    ADCGIRQEN1bits.AGIEN4 = 1;	//Enb ADC4 AN4 interrupts for DMA
    
    //Setup DMA interupt callback and start a transfer
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, ADC_2_ResultReadyCallback, 0);
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, ADC_SRC_ADDR_2, ADC_SRC_SIZE, adc_buf, sizeof (adc_buf), sizeof (uint32_t));
    

    TMR3_Start(); //turn on Timer 3 to trigger ADC_2
    OCMP1_Enable(); // turn on OCMP1 to trigger ADC_4


    while (true) {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();

        
        if(ADC_2_result_ready == true) {
//        if(ADC_2_result_ready == true && ADC_4_result_ready == true) {
            
            ADC_2_result_ready = false;
//            ADC_4_result_ready = false;
            
            TMR2_Start();
            int count = TMR2_CounterGet();
            for (int i = 0; i < SAMPLE_LEN; i++) {
                adc_data[i*2] =  adc_buf[i*3];
                adc_data[i*2 + 1] =  adc_buf[i*3 + 2];
            }
            TMR2_Stop();
            count = TMR2_CounterGet() - count;
            printf("Time: %d\n", count);
            

            for (int i = 0; i < SAMPLE_LEN; i++) {
                float input_voltage_2 = (float) adc_buf[(i*3) ] * ADC_VREF / ADC_MAX_COUNT;
                float input_voltage_4 = (float) adc_buf[(i*3) + 2] * ADC_VREF / ADC_MAX_COUNT;

                printf(">ADC:%.2f \n", input_voltage_2);
                printf(">ADC:%.2f \n", input_voltage_4); //ADC 4 triggers 1st

            }


            //delay
            for (int i = 0; i < 10; i++) {
                TMR2_Start();
                TMR2 = 0x0;
                while (TMR2_CounterGet() < 30000); // Wait for 30000 timer ticks 
                TMR2_Stop();
            }
 

            
            //start new DMA transfer
            DMAC_ChannelTransfer(DMAC_CHANNEL_0, ADC_SRC_ADDR_2, ADC_SRC_SIZE, adc_buf, sizeof (adc_buf), sizeof (uint32_t));
//            DMAC_ChannelTransfer(DMAC_CHANNEL_1, ADC_SRC_ADDR_4, ADC_SRC_SIZE, adc_buf_4, sizeof (adc_buf_4), sizeof (uint16_t));
            printf("done\r\n");
        }
        
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE);
}


/*******************************************************************************
 End of File
 */

