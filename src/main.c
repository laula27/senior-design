
#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for M_PI
#include <stdio.h>

// Be sure to change this to your login...
const char login[] = "huan1650";
#define LENGTH 128
#define COMMAND_MAX_LENGTH 16
#define UNKNOWN_COMMAND "unknown command"
#define COMMAND_TOO_LONG "command too long"
// commands
#define A1_ON  "a1 on"
#define A1_OFF "a1 off"
#define CR '\r'
#define LF '\n'
#define CRLF "\r\n"

char usart_buf[COMMAND_MAX_LENGTH];
unsigned short usart_buf_length=0;

//============================================================================
// setup_tim1()    (Autotest #1)
// Configure Timer 1 and the PWM output pins.
// Parameters: none
//============================================================================
void nano_wait(unsigned int n) {
    asm(    "         mov r0,%0\n"
            "repeat:  sub r0,#83\n"
            "         bgt repeat\n" : : "r"(n) : "r0", "cc");
}

void delay(int ms)
{
    int i;
    for(; ms>0 ;ms--)

    {
        for(i =0; i<3195;i++);

    }
}
int get_string_length(const char* s) {
    int len=0;
    while (s[len] != 0) {
        len++;
    }

    return len;
}
int is_equal(const char* one, const char* two) {
    int i;

    if (get_string_length(one) != get_string_length(two)) {
        return -1;
    }

    for (i=0; i<get_string_length(one); i++) {
        if (one[i] != two[i]) {
            return -1;
        }
    }

    return 0;
}
void init_output(void) {
    // GPIO structure for port initialization
    GPIO_InitTypeDef GPIO_Settings;

    // enable clock on APB2
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    // configure port A1 for driving an LED
    GPIO_Settings.GPIO_Pin = GPIO_Pin_1;
    GPIO_Settings.GPIO_Mode = GPIO_PuPd_UP;
    GPIO_Settings.GPIO_Speed = GPIO_Speed_50MHz;   // highest speed
    GPIO_Init(GPIOA, &GPIO_Settings) ;             // initialize port
}
void setup_tim1()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    //pa8 tim1ch1 pa9 ch2 pa10 ch3 pa11 ch4
    GPIOA->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9 | GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    GPIOA->MODER |=GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1;
    //set the alternate function
    GPIOA->AFR[1] = 0x2222;
    //enable the clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    //enable the MOE bit of the break and dead-time register (BDTR)
    TIM1->BDTR |=TIM_BDTR_MOE;
    //Set the prescaler to divide by 1, and set the ARR so that an update event occurs 20000 times per second.
    TIM1->PSC = 0;
    TIM1->ARR = 65536-1;
    //Configure the "capture/compare mode registers", CCMR1 and CCMR2, to set channels 1, 2, 3, and 4 for PWM mode 1.
    TIM1->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    TIM1->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);
    TIM1->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 |TIM_CCMR1_OC2M_1 |TIM_CCMR1_OC2M_2);  //110
    TIM1->CCMR2 |= (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2);
    TIM1->CCMR2 |= TIM_CCMR2_OC4PE;
//Configure the "capture/compare mode registers", CCMR1 and CCMR2, to set channels 1, 2, 3, and 4 for PWM mode 1.
    TIM1->CCER |= TIM_CCER_CC4E;
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->CCER |= TIM_CCER_CC2E;
    TIM1->CCER |= TIM_CCER_CC3E;
    //enable the timer
    TIM1->CR1 |= TIM_CR1_CEN;

}
void setup_tim2()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    //pa0 pa1 pa2 pa3
    GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    GPIOA->MODER |=GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1 | GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
    //set the alternate function
    GPIOA->AFR[0] = 0x2222;
    //enable the clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    //enable the MOE bit of the break and dead-time register (BDTR)
    TIM2->BDTR |=TIM_BDTR_MOE;
    //Set the prescaler to divide by 1, and set the ARR so that an update event occurs 20000 times per second.
    TIM2->PSC = 0;
    TIM2->ARR = 65536-1;
    //Configure the "capture/compare mode registers", CCMR1 and CCMR2, to set channels 1, 2, 3, and 4 for PWM mode 1.
    TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    TIM2->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);
    TIM2->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 |TIM_CCMR1_OC2M_1 |TIM_CCMR1_OC2M_2);  //110
    TIM2->CCMR2 |= (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2);
    TIM2->CCMR2 |= TIM_CCMR2_OC4PE;
//Configure the "capture/compare mode registers", CCMR1 and CCMR2, to set channels 1, 2, 3, and 4 for PWM mode 1.
    TIM2->CCER |= TIM_CCER_CC4E;
    TIM2->CCER |= TIM_CCER_CC1E;
    TIM2->CCER |= TIM_CCER_CC2E;
    TIM2->CCER |= TIM_CCER_CC3E;
    //enable the timer
    TIM2->CR1 |= TIM_CR1_CEN;

}
void setup_GPIO()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
    //clean pc6 pc7 pc8 pc9
    GPIOC->PUPDR &= ~(0xff000);
    //pull down pc6 pc7 pc8 pc9
    GPIOC->PUPDR |= 0xaa000;
}

int get_ir_sensor()
{
    // res
    //GPIOC->IDR &= ~(0xFFFF);
    if (GPIOC->IDR == 0x40)
    {
        return 6;
    }
    else if(GPIOC->IDR == 0x80)
    {
        return 7;
    }
    else if(GPIOC->IDR == 0x100)
    {
        return 8;
    }
    else
    {
        return 9;
    }
}
void setup_usart5()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;//configure pin PC12 to be routed to USART5_TX.
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;//configure pin PD2 to be routed to USART5_RX.

    //do the alternate function for gpio
    GPIOC->MODER &= ~ GPIO_MODER_MODER12; // Clear mode bits for pin 6 and 7
    GPIOC->MODER |= GPIO_MODER_MODER12_1;; //II Select Alternate Function mode
    GPIOC->AFR[1] &= ~GPIO_AFRH_AFR12;
    GPIOC->AFR[1] |= (2<<16);//for pc12 //af2

    GPIOD->MODER &= ~ GPIO_MODER_MODER2;
    GPIOD->MODER |= GPIO_MODER_MODER2_1;
    GPIOD->AFR[0] &= ~GPIO_AFRL_AFR2;
    GPIOD->AFR[0] |= (2<<8);//for pd2

    //Enable the RCC clock to the USART5 peripheral.
    //find where is the usart5en in either apb1enr or apb2enr ...
    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
    //disable it by turning off its UE bit
    USART5->CR1 &= ~USART_CR1_UE;
    //Set a word size of 8 bits.
    USART5->CR1 &= ~USART_CR1_M;
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


void USART3_4_5_6_7_8_IRQHandler()
{
    //Check and clear the ORE flag.
    if(USART5->ISR &= USART_ISR_ORE)
        USART5->ISR |= USART_ICR_ORECF;

    //Read the new character from the USART5 RDR.
    char x = USART5->RDR;
    //Check if the input_fifo is full. If it is, return from the ISR.
    /*if(fifo_full(&input_fifo))
    {
        return;
    }*/
    //insert_echo_char(x);
}

void enable_tty_interrupt()
{
    //configures USART5 to raise an interrupt every time the receive data register becomes not empty because a new character has been received.
    //if(USART5->RDR!=0)
    USART5->CR1 |= USART_CR1_RXNEIE;
    NVIC->ISER[0]=1<< USART3_8_IRQn;

}

int write_data(int x){

    if (x =='\n')
    {
        while ((USART5->ISR & USART_ISR_TXE) ==0);
        USART5->TDR= '\r';
    }
    // Wait for the USART5 ISR TXE to be set.
    while((USART5->ISR & USART_ISR_TXE) == 0);
    // Write the argument to the USART5 TDR (transmit data register).
    USART5->TDR = x;
    return x;
}
int read_data(){
    if (USART5->RDR =='\r')
    {
        while ((USART5->ISR & USART_ISR_RXNE) ==0);
        return '\n';
    }
    // Wait for the USART5 ISR RXNE to be set.
    while((USART5->ISR & USART_ISR_RXNE) ==0);
    // read the argument to the USART5 RDR (receive data register).
    int x = USART5->RDR;
    return x;
}
void turn_on_pa1(void) {
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

void turn_off_pa1(void) {
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}
// send one byte through USART
void usart_send(const char chr) {
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = chr;
}

void handle_command(char *command) {
    if (is_equal(command, A1_ON) == 0) {
        turn_on_pa1();
    } else if (is_equal(command, A1_OFF) == 0) {
        turn_off_pa1();
    } else {
        usart_send_line(UNKNOWN_COMMAND);
    }
}

// send a string through USART
void usart_send_string(const char *s) {
    int i=0;
    while (s[i]) {
        usart_send(s[i++]);
    }
}
void usart_send_newline(void) {
    usart_send_string(CRLF);
}

void usart_send_line(const char *s) {
    usart_send_string(s);
    usart_send_string(CRLF);
}

void internal_clock();
void demo();


int main(void)
{
    int32_t CH1_DC = 50000;
    int32_t CH2_DC = 50000;
    int ir_val = 0;
    char buffer[LENGTH];
    //int data = 0;
    //int val = 0;


    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    //internal_clock();
    //init_output();
    setup_tim2();
    setup_GPIO(); //For reading IR value
    setup_usart5();//set up UART for Raspberry Pi
    enable_tty_interrupt();
    for(;;) {
        printf("Enter string: ");
        char line[100];
        fgets(line, 99, stdin);
        line[99] = '\0'; // just in case
        printf("You entered: %s", line);
    }
    //ir_val = get_ir_sensor();
    //data = read_data();
   // write_data(6);


    while(1){
       int val = 0;
       while(CH1_DC < 65536)
        {
           //val = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7);
           val = read_data();
           if(val == 1)
           {
               TIM1->CCR2 = 0;
               TIM1->CCR1 = CH1_DC;
               nano_wait(500000000000);

           }
           if(val == 0)
           {
               TIM1->CCR1 = 0;
               TIM1->CCR2 = CH2_DC;
               nano_wait(500000000000);
           }

        }
      /* while(CH1_DC > 0)
        {
           if(val == 1)
           {
               TIM1->CCR1 = CH1_DC;
               nano_wait(500000000000);
               TIM1->CCR1 = 0;
               TIM1->CCR2 = CH2_DC;
               nano_wait(500000000000);
               TIM1->CCR2 = 0;
           }
        }*/
    }




}
