//===========================================================================
// UART_LED
//===========================================================================

#include "stm32f0xx.h"
#include "fifo.h"
#include "tty.h"
#include <string.h> // for memset()
#include <stdio.h> // for printf()

void advance_fattime(void);
void command_shell(void);


// Write your subroutines below.
void setup_usart5()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;
    //configure pin PC12 to be routed to USART5_TX.
    //do the alternate function fro gpio
    GPIOC->MODER &= ~ GPIO_MODER_MODER12; // Clear mode bits for pin 6 and 7
    GPIOC->MODER |= GPIO_MODER_MODER12_1;; //II Select Alternate Function mode
    GPIOC->AFR[1] &= ~GPIO_AFRH_AFR12;
    GPIOC->AFR[1] |= (2<<16);//for pc12 //af2
    GPIOD->MODER &= ~ GPIO_MODER_MODER2;
    GPIOD->MODER |= GPIO_MODER_MODER2_1;
    GPIOD->AFR[0] &= ~GPIO_AFRL_AFR2;
    GPIOD->AFR[0] |= (2<<8);//for pd2

    //LED set up
    // Init clock to GPIOA for LED
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;

    // Init LED on GPIOA
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //Enable the RCC clock to the USART5 peripheral.
    //find where is the usart5en in either apb1enr or apb2enr ...
    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
    //disable it by turning off its UE bit
    USART5->CR1 &= ~USART_CR1_UE;
    //Set a word size of 8 bits.
    USART5->CR1 &= ~USART_CR1_M;
    //USART5->CR1 &= ~((0x10000000)|USART_CR1_M);
    //Set it for one stop bit.
    USART5->CR2 &= ~USART_CR2_STOP;
    //Set it for no parity.
    USART5->CR1 &= ~USART_CR1_PCE;
    //Use 16x oversampling.
    USART5->CR1 &= ~USART_CR1_OVER8;
    //Use a baud rate of 115200 (115.2 kbaud).//look frm table 97
    USART5->BRR =0x1A0;
    //Enable the transmitter and the receiver by setting the TE and RE bits..
    USART5->CR1 |= (USART_CR1_RE | USART_CR1_TE);
    //Enable the USART.
    USART5->CR1 |= USART_CR1_UE;
    //wait USART is ready to transmit and receive.
    while ((USART5->ISR & USART_ISR_TEACK) == 0);
    while ((USART5->ISR & USART_ISR_REACK) == 0);
}


int interrupt_getchar(){
    while(fifo_newline(&input_fifo) == 0){
        asm volatile ("wfi"); // wait for an interrupt
    }
    char ch = fifo_remove(&input_fifo);
        return ch;
}

int better_putchar(int x)
{
    if (x =='\n')
    {
        while ((USART5->ISR & USART_ISR_TXE) ==0);
        USART5->TDR= '\r';
    }
    while ((USART5->ISR & USART_ISR_TXE) ==0);
    USART5->TDR=x;
    return x;
}
int __io_putchar(int ch) {
    return better_putchar(ch);
}

int __io_getchar(void) {
    return interrupt_getchar();
}


void USART3_4_5_6_7_8_IRQHandler()
{
    //Check and clear the ORE flag.
    if(USART5->ISR &= USART_ISR_ORE)
        USART5->ISR |= USART_ICR_ORECF;

    //Read the new character from the USART5 RDR.
    char x = USART5->RDR;
    if(x == 'o') // if letter o was transmitted
    {

        GPIOA->ODR |= GPIO_Pin_1;
    }
    if(x == 'c') // if letter c was transmitted
    {

        GPIOA->ODR &= ~GPIO_Pin_1;
    }
}

void enable_tty_interrupt()
{
    //configures USART5 to raise an interrupt every time the receive data register becomes not empty because a new character has been received.
    //if(USART5->RDR!=0)
    USART5->CR1 |= USART_CR1_RXNEIE;
    NVIC->ISER[0]=1<< USART3_8_IRQn;

}


int main()
{
    setup_usart5();

    // Uncomment these when you're asked to...
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);

    enable_tty_interrupt();
    for(;;) {
        printf("Enter string: ");
        char line[100];
        fgets(line, 99, stdin);
        line[99] = '\0'; // just in case
        printf("You entered: %s", line);
    }


    return 0;
}
