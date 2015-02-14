

#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "msg.h"
#include "kernel.h"
#include "thread.h"
#include "board.h"
#include "vtimer.h"
#include "ringbuffer.h"
#include "periph/uart.h"
#include "periph_conf.h"
#include "ps.h"

#define DEV                 UART_1
#define BAUDRATE            115200

void plot_regs(void);

void prtbin(unsigned int input)
{
    for (char i = 0; i < 32; i++) {
        printf(" %u ", (input >> (31 - i)) & 0x0001);
    }
    puts("\n");
}

void uart_print(uart_t dev, char *str)
{
    int i = 0;

    while (str[i] != '\0') {
        uart_write_blocking(dev, str[i++]);
    }
}

int main(void)
{
    char *hello1 = "BLABLABLA 4 UART_1:\n";

    puts("\nTesting interrupt driven mode of UART driver\n");

    printf("Initializing UART_%i @ %i", DEV, BAUDRATE);
    if (uart_init_blocking(DEV, BAUDRATE) >= 0) {
        puts("   ...done");
    }
    else {
        puts("   ...failed");
        return 1;
    }

GCLK->CLKCTRL.reg = (uint16_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1 | (SERCOM5_GCLK_ID_CORE << GCLK_CLKCTRL_ID_Pos)));
while (GCLK->STATUS.bit.SYNCBUSY);
GCLK->CLKCTRL.reg = (uint16_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1 | (SERCOM5_GCLK_ID_SLOW << GCLK_CLKCTRL_ID_Pos)));
while (GCLK->STATUS.bit.SYNCBUSY);

    plot_regs();

    printf("Begin TX on UART_%i\n", DEV);

    //uart_print(DEV, hello1);
    uart_write_blocking(UART_1, 'Z');
    uart_write_blocking(UART_1, 'A');
    uart_write_blocking(UART_1, 'B');
    uart_write_blocking(UART_0, 'Y');


    printf("End TX on UART_%i\n", DEV);

    return 0;
}

void plot_regs(void) {

    uint32_t test;
    uint16_t test16;
    uint8_t test8;

    PortGroup* port_group_0 = &UART_0_PORT;
    SercomUsart* uart_dev_0 = &UART_0_DEV;
    PortGroup* port_group_1 = &UART_1_PORT;
    SercomUsart* uart_dev_1 = &UART_1_DEV;

    test = PM->APBCMASK.reg;
    printf("APBCMASK: ");
    prtbin(test);
    test = GCLK->GENCTRL.reg;
    printf("GENCTRL : ");
    prtbin(test);
    printf("\n");
/*
    GCLK->CLKCTRL.bit.ID = (uint8_t)(SERCOM0_GCLK_ID_CORE);
    while (GCLK->STATUS.bit.SYNCBUSY);
    test16 = GCLK->CLKCTRL.reg;
    printf("CLKCTRL0 : ");
    prtbin(test16);
    printf("\n");
*/
    GCLK->CLKCTRL.bit.ID = (uint8_t)(SERCOM5_GCLK_ID_CORE);
    while (GCLK->STATUS.bit.SYNCBUSY);
    test16 = GCLK->CLKCTRL.reg;
    printf("CLKCTRL1 integer: %i\n", test16);
    printf("CLKCTRL1 : ");
    prtbin(test16);
    printf("\n");

    test = port_group_0->DIRSET.reg;
    printf("DIRSET0  : ");
    prtbin(test);
    test = port_group_1->DIRSET.reg;
    printf("DIRSET1  : ");
    prtbin(test);
    printf("\n");

    test8 = port_group_0->PINCFG[5].reg;
    printf("PINCFG0  : ");
    prtbin(test8);
    test8=0;
    test8 = port_group_1->PINCFG[23].reg;
    printf("PINCFG1  : ");
    prtbin(test8);
    printf("\n");

    test = uart_dev_0->CTRLA.reg;
    printf("CTRLA0   : ");
    prtbin(test);
    test = uart_dev_1->CTRLA.reg;
    printf("CTRLA1   : ");
    prtbin(test);
    printf("\n");

    test = uart_dev_0->CTRLB.reg;
    printf("CTRLB0   : ");
    prtbin(test);
    test = uart_dev_1->CTRLB.reg;
    printf("CTRLB1   : ");
    prtbin(test);
    printf("\n");

    test = uart_dev_0->BAUD.reg;
    printf("BAUD0    : ");
    prtbin(test);
    test = uart_dev_1->BAUD.reg;
    printf("BAUD1    : ");
    prtbin(test);
    printf("\n");

/*
    test = PAC0->WPCLR.reg;
    printf("WPCLR0    : ");
    prtbin(test);
    printf("\n");
    test = PAC0->WPSET.reg;
    printf("WPSET0    : ");
    prtbin(test);
    printf("\n");
    test = PAC1->WPCLR.reg;
    printf("WPCLR1    : ");
    prtbin(test);
    printf("\n");
    test = PAC0->WPSET.reg;
    printf("WPSET1    : ");
    prtbin(test);
    printf("\n");
    test = PAC2->WPCLR.reg;
    printf("WPCLR2    : ");
    prtbin(test);
    printf("\n");
    test = PAC2->WPSET.reg;
    printf("WPSET2    : ");
    prtbin(test);
    printf("\n");
    */
}

