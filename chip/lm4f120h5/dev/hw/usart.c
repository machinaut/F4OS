#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dev/registers.h>
#include <dev/cortex_m.h>
#include <dev/resource.h>
#include <chip/rom.h>
#include <kernel/sched.h>
#include <kernel/semaphore.h>
#include <kernel/fault.h>

#include <dev/hw/usart.h>

struct semaphore usart_semaphore;

resource uart_console = {   .writer     = &usart_putc,
                            .swriter    = &usart_puts,
                            .reader     = &usart_getc,
                            .closer     = &usart_close,
                            .env        = NULL,
                            .sem        = &usart_semaphore};

static void usart_baud(uint32_t baud) __attribute__((section(".kernel")));

uint8_t usart_ready = 0;

void init_usart(void) {
    /* Enable UART0 clock */
    SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0;

    /* Enable GPIOA clock */
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

    ROM_SysCtlDelay(1);

    /* Alternate functions UART */
    GPIO_PORTA_PCTL_R |= (1 << 4) | (1 << 0);

    /* Set to output */
    GPIO_PORTA_DIR_R &= ~(GPIO_PIN_1 | GPIO_PIN_0);

    /* Set PA0 and PA1 to alternate function */
    GPIO_PORTA_AFSEL_R |= GPIO_PIN_1 | GPIO_PIN_0;

    /* Digital pins */
    GPIO_PORTA_DEN_R |= GPIO_PIN_1 | GPIO_PIN_0;

    /* Disable UART */
    UART0_CTL_R &= ~(UART_CTL_UARTEN);

    usart_baud(115200);

    /* Enable FIFO, 8-bit words */
    UART0_LCRH_R = UART_LCRH_FEN | UART_LCRH_WLEN_8;

    /* Use system clock */
    UART0_CC_R = UART_CC_CS_SYSCLK;

    /* Enable UART */
    UART0_CTL_R |= UART_CTL_UARTEN | UART_CTL_RXE | UART_CTL_TXE;

    /* Reset semaphore */
    init_semaphore(&usart_semaphore);

    usart_ready = 1;
}


/* Sets baud rate registers */
void usart_baud(uint32_t baud) {
    float brd = ROM_SysCtlClockGet() / (16 * baud);
    int brdi = (int) brd;
    int brdf = (int) (brd * 64 + 0.5);

    UART0_IBRD_R = brdi;
    UART0_FBRD_R = brdf;
}

void usart_putc(char c, void *env) {
    /* Wait until transmit FIFO not full*/
    while (UART0_FR_R & UART_FR_TXFF) {
        if (task_switching && !IPSR()) {
            release(&usart_semaphore);
            SVC(SVC_YIELD);
            acquire(&usart_semaphore);
        }
    }

    UART0_DR_R = c;
}

void usart_puts(char *s, void *env) {
    while (*s) {
        usart_putc(*s++, env);
    }
}

char usart_getc(void *env) {
    /* Wait for data */
    while (UART0_FR_R & UART_FR_RXFE) {
        if (task_switching && !IPSR()) {
            SVC(SVC_YIELD);
        }
    }

    return UART0_DR_R & UART_DR_DATA_M;
}

void usart_close(resource *resource) {
    panic_print("USART is a fundamental resource, it may not be closed.");
}