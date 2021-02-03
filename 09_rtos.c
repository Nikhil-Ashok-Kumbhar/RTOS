// RTOS Framework - Spring 2020
// J Losh

// Student Name:NIKHIL ASHOK KUMBHAR
// TO DO: Add your name on this line.  Do not include your ID number in the file.

// Add xx_ prefix to all files in your project
// 09_rtos.c
// 09_tm4c123gh6pm_startup_ccs.c
// 09_other files (except uart0.x and wait.x)
// (xx is a unique number that will be issued in class)
// Please do not change any function name in this code or the thread priorities

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// 6 Pushbuttons and 5 LEDs, UART
// LEDs on these pins:
// Blue:   PF2 (on-board)
// Red:    PA2
// Orange: PA3
// Yellow: PA4
// Green:  PE0
// PBs on these pins
// PB0:    PC4
// PB1:    PC5
// PB2:    PC6
// PB3:    PC7
// PB4:    PD6
// PB5:    PD7
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//Commands Used
//1-ps on/off
//2-pi on/off
//3-preempt on/off
//4-sched rr/pr
//5-ipcs
//6-kill pidno
//7-pidof
//8-taskname &
//9-reset

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "wait.h"

#define BLUE_LED (*((volatile uint32_t *)(0x42000000 + (0x400253FC - 0x40000000) * 32 + 2 * 4))) //PF2
#define GREEN_LED (*((volatile uint32_t *)(0x42000000 + (0x400243FC - 0x40000000) * 32 + 0 * 4))) //PE0
#define YELLOW_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC - 0x40000000) * 32 + 4 * 4))) //PA4
#define ORANGE_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC - 0x40000000) * 32 + 3 * 4))) //PA3
#define RED_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC - 0x40000000) * 32 + 2 * 4))) //PA2
#define POWER (*((volatile uint32_t *)(0x42000000 + (0x400253FC - 0x40000000) * 32 + 3 * 4))) //PF3

#define PB0 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 4 * 4))) //Push Button-1
#define PB1 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 5 * 4))) //Push Button-2
#define PB2 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 6 * 4))) //Push Button-3
#define PB3 (*((volatile uint32_t *)(0x42000000 + (0x400063FC - 0x40000000) * 32 + 7 * 4))) //Push Button-4
#define PB4 (*((volatile uint32_t *)(0x42000000 + (0x400073FC - 0x40000000) * 32 + 6 * 4))) //Push Button-5
#define PB5 (*((volatile uint32_t *)(0x42000000 + (0x400073FC - 0x40000000) * 32 + 7 * 4))) //Push Button-6



#define BLUE_LED_MASK 4
#define GREEN_LED_MASK 1
#define YELLOW_LED_MASK 16
#define ORANGE_LED_MASK 8
#define RED_LED_MASK 4
#define POWER_LED_MASK 8

#define PB0_MASK 16
#define PB1_MASK 32
#define PB2_MASK 64
#define PB3_MASK 128
#define PB4_MASK 64
#define PB5_MASK 128


extern void  setPsp(uint32_t *PSP);
extern void* getPsp();
extern void  pushR4_11();
extern void  popR4_11();
extern void  pushDummy(uint32_t pid);
extern uint32_t* getR0();


#define YIELD 50
#define SLEEP 51
#define WAIT  52
#define POST  53
#define KILL  54
#define CPU  55

#define MAX_CHAR 80
#define MAX_FIELDS 6

#define UART_TX_MASK 2
#define UART_RX_MASK 1


// Initialize UART0
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R = 0;


    //enable clk on timer 1
//    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;

    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOF | SYSCTL_RCGC2_GPIOE |SYSCTL_RCGC2_GPIOD | SYSCTL_RCGC2_GPIOC ;

    GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTD_CR_R = PB5_MASK;

    // Configure LED pins
    //PORTf
    GPIO_PORTF_DIR_R = POWER_LED_MASK | BLUE_LED_MASK;  // bits 1 and 3 are outputs
    GPIO_PORTF_DR2R_R = POWER_LED_MASK | BLUE_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R = POWER_LED_MASK | BLUE_LED_MASK;  // enable LEDs

    //PORTA
        GPIO_PORTA_DIR_R = RED_LED_MASK | ORANGE_LED_MASK | YELLOW_LED_MASK;
        GPIO_PORTA_DR2R_R = RED_LED_MASK | ORANGE_LED_MASK | YELLOW_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
        GPIO_PORTA_DEN_R = RED_LED_MASK | ORANGE_LED_MASK | YELLOW_LED_MASK;  // enable LEDs

    //PORT E
        GPIO_PORTE_DIR_R = GREEN_LED_MASK;
        GPIO_PORTE_DR2R_R = GREEN_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
        GPIO_PORTE_DEN_R = GREEN_LED_MASK;  // enable LEDs

        //PORTC
        GPIO_PORTC_DEN_R = PB0_MASK | PB1_MASK | PB2_MASK | PB3_MASK;  // enable LEDs and pushbuttons
        GPIO_PORTC_PUR_R = PB0_MASK | PB1_MASK | PB2_MASK | PB3_MASK; // enable internal pull-up for push button

        //PORT D
        GPIO_PORTD_DEN_R = PB4_MASK | PB5_MASK;  // enable LEDs and pushbuttons
        GPIO_PORTD_PUR_R = PB4_MASK | PB5_MASK; // enable internal pull-up for push button

        SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;                                      // Enable Systick timer
        TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                                                // Disable timer before configuring (safe programming)
        TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;                                          // Configure as 32-bit timer (A+B)
        TIMER1_TAILR_R = 0xFFFFFFFF;                                                    //max value
        TIMER1_TAV_R = 0;                                                               //initail value set to 0
        TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD | TIMER_TAMR_TACDIR;                     // Configure for periodic mode and Count Up timer



}

//
void initUart0()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable clocks
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

    // Configure UART0 pins
    GPIO_PORTA_DIR_R |= UART_TX_MASK;                   // enable output on UART0 TX pin
    GPIO_PORTA_DIR_R &= ~UART_RX_MASK;                   // enable input on UART0 RX pin
    GPIO_PORTA_DR2R_R |= UART_TX_MASK;                  // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTA_DEN_R |= UART_TX_MASK | UART_RX_MASK;    // enable digital on UART0 pins
    GPIO_PORTA_AFSEL_R |= UART_TX_MASK | UART_RX_MASK;  // use peripheral to drive PA0, PA1
    GPIO_PORTA_PCTL_R &= ~(GPIO_PCTL_PA1_M | GPIO_PCTL_PA0_M); // clear bits 0-7
    GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;
                                                        // select UART0 to drive pins PA0 and PA1: default, added for clarity

    // Configure UART0 to 115200 baud, 8N1 format
    UART0_CTL_R = 0;                                    // turn-off UART0 to allow safe programming
    UART0_CC_R = UART_CC_CS_SYSCLK;                     // use system clock (40 MHz)
    UART0_IBRD_R = 21;                                  // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R = 45;                                  // round(fract(r)*64)=45
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;    // configure for 8N1 w/ 16-level FIFO
    UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
                                                        // enable TX, RX, and module
}



//-----------------------------------------------------------------------------
// String Function
//-----------------------------------------------------------------------------

void copyString(char *to, const char *from){

    while(*from){
        *to++ = *from++;
    }
    *to = '\0';
}

int stringcompare(char *cmd, char *usercmd){

    while((*cmd!=0)||(*usercmd!=0)){
        if(*cmd!= *usercmd){
            return *cmd - *usercmd;
        }
        *cmd++;
        *usercmd++;
    }

    return 0;
}

uint16_t asctoi(char *usercmd){

    uint16_t result= 0;

    while(*usercmd !='\0' ){

        result = result * 10 + *usercmd - '0' ;

        *usercmd++;
    }

    return result;

}

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------
char str[MAX_CHAR + 1];
uint8_t pos[MAX_FIELDS];
uint8_t argCount;
uint8_t argNo;
char numstr[20];

uint8_t i,p;
uint32_t processid;
bool EnablePriority = true;
bool Priority_inherit = true;
bool Preempt_enable = true;
bool Pingpong = false;

uint16_t switchbuffer = 0;


static uint32_t *task_point;
// function pointer
typedef void (*_fn)();

// semaphore
#define MAX_SEMAPHORES 5
#define MAX_QUEUE_SIZE 5
struct semaphore
{
    uint16_t count;
    uint16_t queueSize;
    char semaname[16];
    uint16_t currentUser;
    uint32_t processQueue[MAX_QUEUE_SIZE]; // store task index here
} semaphores[MAX_SEMAPHORES];
uint8_t semaphoreCount = 0;

struct semaphore *keyPressed, *keyReleased, *flashReq, *resource, *semaphorept;

// task
#define STATE_INVALID    0 // no task
#define STATE_UNRUN      1 // task has never been run
#define STATE_READY      2 // has run, can resume at any time
#define STATE_DELAYED    3 // has run, but now awaiting timer
#define STATE_BLOCKED    4 // has run, but now blocked by semaphore

#define MAX_TASKS 10       // maximum number of valid tasks
uint8_t taskCurrent = 0;   // index of last dispatched task
uint8_t taskCount = 0;     // total number of valid tasks

uint32_t stack[MAX_TASKS][512]; //2048 byte for each task
uint32_t sum[MAX_TASKS]; //buffer for pingpong


// REQUIRED: add store and management for the memory used by the thread stacks
//           thread stacks must start on 1 kiB boundaries so mpu can work correctly

struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread
    void *spInit;                  // location of original stack pointer
    void *sp;                      // location of stack pointer for thread
    int8_t priority;               // 0=highest to 15=lowest
    int8_t currentPriority;        // used for priority inheritance
    uint32_t ticks;                // ticks until sleep complete
    uint32_t delta[2];             //two buffers for pingpong
    uint32_t cpupercent;           //%cpu usage
    char name[16];                 // name of task used in ps command
    void *semaphore;               // pointer to the semaphore that is blocking the thread
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// RTOS Kernel Functions
//-----------------------------------------------------------------------------


// REQUIRED: initialize systick for 1ms system timer
void initRtos()
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }



}

// REQUIRED: Implement prioritization to 16 levels
int rtosScheduler()
{
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    uint8_t Priority = 0;
    uint8_t found = 0;
    uint8_t val;
    static uint8_t Count = 0;
    while (!ok)
    {

        task++;
        if (task >= MAX_TASKS){
            task = 0;
        }
        if(EnablePriority){

            while(Priority<16){        //checks priority
                for(i=1; i<=MAX_TASKS; i++){

                    val = ((i+(Count-1)) % MAX_TASKS);  //circular buffer
                    if((tcb[val].currentPriority == Priority)&&((tcb[val].state == STATE_READY || tcb[val].state == STATE_UNRUN))){
                        found = 1;

                        task = val;
                        break;
                    }

                }

                if(found){
                    Count = val + 1;     //Count Variable Store the current task dispatched + 1.so it can start from next task
                    return task;
                }
                else{
                Priority++;
                }


            }

        }
        else{
            ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
        }

    }
    return task;
}



bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;
    uint8_t i = 0;
    bool found = false;
    // REQUIRED: store the thread name
    // add task if room in task list
    // allocate stack space for a thread and assign to sp below
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            tcb[i].state = STATE_UNRUN;
            tcb[i].pid = fn;
            tcb[i].spInit = &stack[i][511]; // point to top of the stack
            tcb[i].sp = tcb[i].spInit;  // point to top of the stack
            tcb[i].priority = priority;
            tcb[i].currentPriority = priority;
            copyString(tcb[i].name, name);
            // increment task count
            taskCount++;
            ok = true;
        }
    }
    // REQUIRED: allow tasks switches again
    return ok;
}

// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn)
{
    uint8_t k =0;
    uint8_t found = 0;
    while(k<MAX_TASKS){
        if(tcb[k].pid == fn){
            found =1;
            break;
        }

        k++;
    }
    if(found){
            tcb[k].state = STATE_UNRUN;  //set state
            tcb[k].sp = tcb[k].spInit;   //Load intial stack value
     }

}

// REQUIRED: modify this function to destroy a thread
// REQUIRED: remove any pending semaphore waiting
void destroyThread(_fn fn)
{
    __asm(" SVC #54");
}

// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{

    uint8_t k =0;
    uint8_t found = 0;
    while(k<MAX_TASKS){
        if(tcb[k].pid == fn){
            found =1;
            break;
        }

        k++;
    }
    if(found){
        tcb[k].priority = priority;
        tcb[k].currentPriority = priority;
     }
}

struct semaphore* createSemaphore(uint8_t count,const char sname[])
{
    struct semaphore *pSemaphore = 0;
    if (semaphoreCount < MAX_SEMAPHORES)
    {
        pSemaphore = &semaphores[semaphoreCount++];
        pSemaphore->count = count;
        copyString(pSemaphore->semaname, sname); //store semaphore name
    }
    return pSemaphore;
}

// REQUIRED: modify this function to start the operating system, using all created tasks
void startRtos()
{
    NVIC_ST_CTRL_R     = 0;                                                                    // Clear Control bit for safe programming
    NVIC_ST_CURRENT_R  = 0;                                                                    // Start Value
    NVIC_ST_RELOAD_R   = 0x9C3F;                                                              // Set for 1Khz, (40000000/1000) - 1
    NVIC_ST_CTRL_R     = NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN | NVIC_ST_CTRL_ENABLE ;      // set for source as clock interrupt enable and enable the timer.


    _fn fn;


    taskCurrent = rtosScheduler();            // Call scheduler
      setPsp(tcb[taskCurrent].sp);                //set sp
    fn = (_fn)tcb[taskCurrent].pid;            //set pc

    TIMER1_CTL_R |= TIMER_CTL_TAEN;           //turn on timer

    (*fn)();

}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
// push registers, call scheduler, pop registers, return to new function
void yield()
{


    __asm(" SVC #50");
}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
// push registers, set state to delayed, store timeout, call scheduler, pop registers,
// return to new function (separate unrun or ready processing)
void sleep(uint32_t tick)
{
    __asm(" SVC #51");
}

// REQUIRED: modify this function to wait a semaphore with priority inheritance
// return if avail (separate unrun or ready processing), else yield to scheduler using pendsv
void wait(struct semaphore *pSemaphore)
{
    __asm(" SVC #52");
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void post(struct semaphore *pSemaphore)
{
    __asm(" SVC #53");
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr()
{

    if(Preempt_enable){                                 //if preemption turned on
           NVIC_INT_CTRL_R= NVIC_INT_CTRL_PEND_SV;
       }
    uint8_t i,k;
    switchbuffer++;
    for(i=0; i < taskCount; i++){      //calulating ticks

        if(tcb[i].state == STATE_DELAYED){

                tcb[i].ticks--;

                if (tcb[i].ticks == 0){

                    tcb[i].state = STATE_READY;
                }

            }
    }

    if((switchbuffer == 1000)){    //pingpong buffer switch after 1sec
        Pingpong = (!Pingpong);    //toggle Flag
        if(!Pingpong){
            for(k=0;k<MAX_TASKS-1;k++){  //setting write buffer to zero
               tcb[k].delta[0] = 0;
            }
           }
           else{
               for(k=0;k<MAX_TASKS-1;k++){
                 tcb[k].delta[1] = 0;
                }
           }
        switchbuffer = 0;
    }




}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
void pendSvIsr()
{
    pushR4_11();                                        //push r4-r11
    tcb[taskCurrent].sp = getPsp();                     //set psp

    if(!Pingpong){                                      //accumulate values in the buffer
        tcb[taskCurrent].delta[0] = TIMER1_TAV_R;
    }
    else{
        tcb[taskCurrent].delta[1] = TIMER1_TAV_R;
    }
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                    //turnoff timer
    TIMER1_TAV_R = 0;
    taskCurrent = rtosScheduler();

    TIMER1_CTL_R |= TIMER_CTL_TAEN;                     //turnon timer

    if(tcb[taskCurrent].state == STATE_READY)
    {
        setPsp(tcb[taskCurrent].sp);
        popR4_11();                                     //pop r4-r11
    }
    else if(tcb[taskCurrent].state == STATE_UNRUN)
    {

        setPsp(tcb[taskCurrent].sp);
       task_point = getPsp();
       task_point--;
       *task_point = 16777216;                             //xpsr
       task_point--;
       *task_point = (uint32_t)tcb[taskCurrent].pid;       //pc
       task_point--;
       task_point--;
       task_point--;
       task_point--;
       task_point--;
       task_point--;                                       //r0
       setPsp(task_point);
       tcb[taskCurrent].state = STATE_READY ;
    }

}

// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr()
{

    static uint32_t r0;
    r0 = (uint32_t)getR0();
    uint32_t P_id;
    uint32_t *PC;
    uint32_t *temp;


    uint8_t svcNumber,k;
    uint8_t found = 0;


      PC = getPsp();
      ++PC;
      ++PC;
      ++PC;
      ++PC;
      ++PC;
      ++PC;

      temp = (uint32_t)*PC-2;
      svcNumber = *temp & 255;                                                                      //get svc number


    switch(svcNumber)                                                                               // Switch Case for SVC value
       {

       case YIELD:

                     NVIC_INT_CTRL_R= NVIC_INT_CTRL_PEND_SV;                                           // Set pendsv bit to call pendsvISR for task switch
                     break;


       case SLEEP:


                     tcb[taskCurrent].ticks = r0;                                                      // Set sleep timeout value
                     tcb[taskCurrent].state = STATE_DELAYED;                                           // Set state as delayed
                     NVIC_INT_CTRL_R= NVIC_INT_CTRL_PEND_SV;                                           // Set pendsv bit
                     break;

       case WAIT:

                          semaphorept  = (struct semaphore*) r0;
                            if(semaphorept->count > 0 ){
                                semaphorept->count--;
                                semaphorept->currentUser = taskCurrent;                                 // current semaphore user
                            }
                            else{
                                semaphorept->processQueue[semaphorept->queueSize] = taskCurrent;       //adding task to queue
                                semaphorept->queueSize++;
                                tcb[taskCurrent].state = STATE_BLOCKED;
                                tcb[taskCurrent].semaphore = semaphorept;
                                NVIC_INT_CTRL_R= NVIC_INT_CTRL_PEND_SV;

                            if(Priority_inherit){                                                       //checking for priority inheritance
                                for(k=0; k<MAX_TASKS; k++){
                                if((tcb[k].semaphore == tcb[taskCurrent].semaphore)&&(tcb[k].currentPriority) > tcb[taskCurrent].currentPriority){ //check for same semaphore and their respective current priority
                                    found =1;
                                    break;
                                    }
                                }
                                if(found){
                                    tcb[k].currentPriority = tcb[taskCurrent].currentPriority;
                                }

                            }


                            }

                            break;


       case POST:

                         semaphorept  = (struct semaphore*) r0;
                         tcb[taskCurrent].currentPriority = tcb[taskCurrent].priority;                  //current priority back to base priority after priority inheritance
                         semaphorept->count++;
                            if(semaphorept->queueSize > 0 ){
                               tcb[semaphorept->processQueue[0]].state = STATE_READY;                   //if process in queue give it the semaphore and remove it from the queue
                               semaphorept->currentUser = semaphorept->processQueue[0];                 //current user
                               semaphorept->count--;
                               for(i=0; i < (semaphorept->queueSize); i++ ) {
                                   semaphorept->processQueue[i] = semaphorept->processQueue[i+1];
                               }
                               semaphorept->queueSize--;


                            }

                            break;


       case KILL:                                                                                       //Kill process

                              P_id = r0;
                              for(i=0; i<MAX_TASKS; i++){
                                  if((uint32_t)tcb[i].pid == P_id){
                                      semaphorept = tcb[i].semaphore ;
                                      break;
                                  }
                              }
                              if(tcb[i].state == STATE_BLOCKED){                                        //if in queue remove from queue
                                  semaphorept->processQueue[0] = 0;
                                  for(k=0; k<semaphorept->queueSize; k++){
                                      semaphorept->processQueue[k] = semaphorept->processQueue[k+1];
                                  }
                                  semaphorept->queueSize--;
                              }
                              else if((tcb[i].semaphore !=0)){                                          //killing running task which has a semaphore
                                  semaphorept->count++; //check again
                                if(semaphorept->queueSize > 0 ){
                                 tcb[semaphorept->processQueue[0]].state = STATE_READY;                 //if in queue make that task ready
                                 semaphorept->currentUser = semaphorept->processQueue[0];
                                 for(k=0; k < (semaphorept->queueSize); k++ ) {
                                     semaphorept->processQueue[k] = semaphorept->processQueue[k+1];
                                 }
                                 semaphorept->queueSize--;
                                 semaphorept->count--; // check again

                              }


                              }
                              tcb[i].state = STATE_INVALID;                                             //make state invalid


                              break;


       case CPU:                                                                                        //cpu percentage calculation


                           for(k=0; k<MAX_TASKS; k++){                                                   //storing values in sum[maxtasks] depending upon the flag
                               sum[k] = tcb[k].delta[1-Pingpong];
                           }
                           break;
       }





}





//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


// Blocking function that writes a serial character when the UART buffer is not full
void putcUart0(char c)
{
    while (UART0_FR_R & UART_FR_TXFF){                                                      // wait if uart0 tx fifo full

        yield();
    }
    UART0_DR_R = c;                                                                         // write character to fifo
}

// Blocking function that writes a string when the UART buffer is not full
void putsUart0(char* str)
{
    uint8_t i = 0;
    while (str[i] != '\0')
        putcUart0(str[i++]);
}

// Blocking function that returns with serial data once the buffer is not empty
char getcUart0()
{
    while (UART0_FR_R & UART_FR_RXFE){                                                        // wait if uart0 rx fifo empty

        yield();
    }
        return UART0_DR_R & 0xFF;                                                             // get character from fifo
}



// REQUIRED: add code to return a value from 0-63 indicating which of 6 PBs are pressed
uint8_t readPbs()
{

    if(!PB0){
        return 1;
    }
    if(!PB1){
            return 2;
        }
    if(!PB2){
            return 4;
        }
    if(!PB3){
            return 8;
        }
    if(!PB4){
            return 16;
        }
    if(!PB5){
            return 32;
        }
    return 0;
}

//-----------------------------------------------------------------------------
// YOUR UNIQUE CODE
// REQUIRED: add any custom code in this space
//-----------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Task functions
// ------------------------------------------------------------------------------

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle()
{
    while(true)
    {
        ORANGE_LED = 1;
        waitMicrosecond(1000);
        ORANGE_LED = 0;
        yield();
    }
}

void flash4Hz()
{
    while(true)
    {
        GREEN_LED ^= 1;
        sleep(125);
    }
}

void oneshot()
{
    while(true)
    {
        wait(flashReq);
        YELLOW_LED = 1;
        sleep(1000);
        YELLOW_LED = 0;
    }
}

void partOfLengthyFn()
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn()
{
    uint16_t i;
    while(true)
    {
        wait(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
        }
        RED_LED ^= 1;
        post(resource);
    }
}

void readKeys()
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)
        {
            YELLOW_LED ^= 1;
            RED_LED = 1;
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            RED_LED = 0;
        }
        if ((buttons & 4) != 0)
        {
            putsUart0("Restart Flash4Hz \r\n");
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
            putsUart0("Kill Flash4Hz \r\n");
            destroyThread(flash4Hz);
        }
        if ((buttons & 16) != 0)
        {
            putsUart0("Priority of LengthyFn Changed to 4 \r\n");
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce()
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative()
{
    while(true)
    {
        while (readPbs() == 32)
        {
        }
        yield();
    }
}

void important()
{
    while(true)
    {
        wait(resource);
        BLUE_LED = 1;
        sleep(1000);
        BLUE_LED = 0;
        post(resource);
    }
}

// REQUIRED: add processing for the shell commands through the UART here
//           your solution should not use C library calls for strings, as the stack will be too large

void clean(char *str)
{
    int i = 0;
    while (i < 20)
    {
        str[i] = '\0';
        i++;
    }
}



void getString(char *str)                               //Code from 5314 getstring
{


    uint8_t i = 0;
    char c;
    while (1)
    {
        c = getcUart0();
        if (c == 8 || c == 127)
        {

            if (i > 0)
            {
                str[i] = str[i--];
                continue;
            }
        }
        else if (c == 13 || c == 10)
        {
            str[i] = 0;
            break;
        }
        else if (c >= 32)
        {
            str[i++] = c;
            if (i >= MAX_CHAR)
            {
                str[i] = 0;
                break;
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }
    }
}

void parseString(char *str)                         //5314-parsestring
{

    uint8_t i, j,  argC = 0;

    uint8_t test;



    for (i = 0, j = 0; i < str[i] != '\0'; i++)
    {

        if ((str[i] >= 97 && str[i] <= 122) || (str[i] >= 65 && str[i] <= 90) || (str[i] >= 48 && str[i] <= 57) || (str[i] == 46) || (str[i] == 45) || (str[i] == 38))
        {

            if (i > 0)
            {

                if (!((test >= 65 && test <= 90) || (test >= 97 && test <= 122) || (test >= 48 && test <= 57) || (test == 46) || (test == 45)))
                {

                    pos[j++] = i;
                    argC++;
                }
            }
            else
            {

                pos[j++] = i;
                argC++;
            }
        }
        else
        {

            str[i] = 0;
        }

        test = str[i];

        argCount = argC;
    }
}



char *getArgstring(uint8_t argNo)
{

    if (argNo < argCount)
    {

        return &str[pos[argNo]];
    }
    else
    {
      //  putsUart0("error\r\n");
    }

    return (0);
}


bool isCommand(char strcmd[], uint8_t minArg, uint8_t maxArg,uint8_t setflag)
{


    if ((stringcompare(strcmd, getArgstring(setflag)) == 0) && (argCount > minArg) && (argCount <= maxArg))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void numtostr(char *numstr, uint16_t number){       //function to convert itoa

    uint8_t count = 0;
    uint16_t temp;
    temp = number;

    while(temp>0){
        count++;

        temp = temp / 10 ;


  }
    count -=1;
    while(number>0){
        numstr[count] = number % 10 + '0';

        number = number / 10 ;

        count--;
  }
    numstr[count] = '\0';

    putsUart0(numstr);
    clean(numstr);


}
void getpid_no(){                                               //function to get pidnumber

    uint8_t found =0;
    uint8_t k=0;
    while(k<MAX_TASKS){

        if(stringcompare(getArgstring(1), tcb[k].name) == 0){

            found = 1;
            processid = (uint32_t)(tcb[k].pid);
            break;
        }

        k++;
    }

    if(found == 0){
        putsUart0("not a valid process name\r\n");
    }
     else{
         numtostr(numstr,processid);
         }



}

void getipcs(){                                                                  //function to dispaly ipcs
     i=0;
     p = 0;
     putsUart0("Name \t    Wait Count & Task \t     Semaphore Count    Running Task\r\n");
     while(p<MAX_SEMAPHORES - 1){


         putsUart0(semaphores[p].semaname);
         putsUart0("  \t");
         if((semaphores[p].queueSize)==0){
             putsUart0("0");
             putsUart0("  None        ");
         }
         else{
             numtostr(numstr,semaphores[p].queueSize);
             putsUart0("  ");
             putsUart0(tcb[semaphores[p].processQueue[0]].name);
             putsUart0("  \t");
         }
         putsUart0("  \t    ");
         if((semaphores[p].count) == 0){
             putsUart0("0");
         }
         else{
         numtostr(numstr,semaphores[p].count);
         }
         putsUart0("  \t");
         putsUart0("  \t");
        if(((tcb[semaphores[p].currentUser].state) == 0)||((tcb[semaphores[p].currentUser].state) == 4)){
          putsUart0("  None");
        }
        else{
            putsUart0(tcb[semaphores[p].currentUser].name);
        }
         putsUart0("\r\n");
         p++;
     }

}

void getstats(){                                                         //svc call to get stats

    __asm(" SVC #55");
}

void calstats(){                                                         //function to display and calculate ps and cpu usage


    uint8_t k= 0;
    uint32_t convert = 0;


    for(k=0; k<MAX_TASKS-1; k++){

        sum[9] += sum[k];                                               //store the sum in sum[9]

    }

    for(k=0; k<MAX_TASKS-1; k++){
        tcb[k].cpupercent = (10000*(sum[k]))/sum[9];                    //cpu percentage usage calculation

    }

        putsUart0("NAME");
        putsUart0("   \t    ");
        putsUart0("Priority");
        putsUart0("\t");
        putsUart0("PID");
        putsUart0("    \t    ");
        putsUart0("STATE");
        putsUart0("        \t");
        putsUart0("%CPU Usage");
        putsUart0("\r\n");

    for(k=0; k<MAX_TASKS-1; k++){


     putsUart0(tcb[k].name);
     putsUart0("    \t");
     if(tcb[k].priority == 0){
       putsUart0("0");
     }
     else{
     numtostr(numstr,tcb[k].priority);
     }
     putsUart0("    \t");
     numtostr(numstr,(uint32_t)tcb[k].pid);
     putsUart0("  \t");
     if(tcb[k].state == 0){
              putsUart0("STATE INVALID");
          }
     if(tcb[k].state == 1){
         putsUart0("STATE UNRUN");
     }
     if(tcb[k].state == 2){
         putsUart0("STATE READY");
          }
     if(tcb[k].state == 3){
         putsUart0("STATE DELAYED");
          }
     if(tcb[k].state == 4){
         putsUart0("STATE BLOCKED");
          }
     putsUart0("      \t");
     convert = tcb[k].cpupercent/100;

     if(convert == 0){
         putsUart0("00");
     }
     else{
       numtostr(numstr,convert);
     }
       putsUart0(".");
      convert = tcb[k].cpupercent%100;
      if(convert == 0){
          putsUart0("00");
      }
      else{
      numtostr(numstr,convert);
      }
      putsUart0("%");
        putsUart0("\r\n");
    }

}

void shell()
{

    putsUart0("6314-Advance Embedded  Microcontroller\r\n");
    putsUart0("Name-NIKHIL ASHOK KUMBHAR\r\n");
    putsUart0("ID-1001775805\r\n");

    while (true)
    {

        uint8_t flag = 0;
        uint8_t found = 0;
        getString(str);                                                         //Get string Function
        parseString(str);                                                       //Parse string Function

        if(isCommand("pidof",1,2,0)){
            flag = 1;
            putsUart0("Pid:");
            getpid_no();
            putsUart0("\r\n");
        }
        if(isCommand("ipcs",0,1,0)){
            flag = 1;
            getipcs();

        }
        if(isCommand("sched",1,2,0)){
            flag = 1;
            if(stringcompare("rr",getArgstring(1))==0){
                EnablePriority = false;
                putsUart0("Roundrobin Scheduling Enabled\r\n");

            }
            else if(stringcompare("pr",getArgstring(1))==0){
                EnablePriority = true;
                putsUart0("Priority Scheduling Enabled\r\n");

            }
            else{
                putsUart0("Invalid Priority Scheduling Command \r\n");
            }


        }
        if(isCommand("pi",1,2,0)){
            flag = 1;
            if(stringcompare("on",getArgstring(1))==0){
                Priority_inherit = true;
                putsUart0("Priority Inheritance Enabled\r\n");

            }
            else if(stringcompare("off",getArgstring(1))==0){
                Priority_inherit = false;
                putsUart0("Priority Inheritance Disabled\r\n");

            }
            else{
                putsUart0("Invalid Priority Inheritance Command \r\n");
            }


        }

        if(isCommand("preempt",1,2,0)){
            flag = 1;
            if(stringcompare("on",getArgstring(1))==0){
                Preempt_enable = true;
                putsUart0("Preemption Enabled\r\n");

            }
            else if(stringcompare("off",getArgstring(1))==0){
                Preempt_enable = false;
                putsUart0("Preemption Disabled\r\n");

            }
            else{
                putsUart0("Invalid Preemption Command \r\n");
            }


        }

        if(isCommand("kill",1,2,0)){
           flag = 1;
           uint8_t k = 0;
           _fn process;
           uint32_t pid_num = asctoi(getArgstring(1));
           for(k=0; k<MAX_TASKS; k++){
               if((uint32_t)tcb[k].pid == pid_num){
                  found = 1;
                  break;
               }
           }
           if(found){
               if(stringcompare(tcb[k].name,"Idle")==0){
                     putsUart0("Cannot Kill Idle\r\n");
               }
               else{
               putsUart0("Kill Command Executing\r\n");
               process = (_fn)tcb[k].pid;
               destroyThread(process);
               putsUart0("Process Killed\r\n");
               }

          }
           else{
               putsUart0("Invalid Process id \r\n");
           }


        }

        if(isCommand("&",1,2,1)){
            flag = 1;
            uint8_t k = 0;
            _fn process;
            for(k=0; k<MAX_TASKS; k++){
                if(stringcompare(tcb[k].name,getArgstring(0))==0){
                    found = 1;
                    break;
                }

            }
            if(found){
                if(tcb[k].state == STATE_INVALID){
                    process = (_fn)tcb[k].pid;
                    restartThread(process);
                    putsUart0("Process Restored\r\n");

                }
                else{
                    putsUart0("Thread already Created\r\n");
                }
            }
            else{
                putsUart0("Invalid Thread Name\r\n");
            }


        }

        if(isCommand("ps",0,1,0)){
                    flag = 1;
                    putsUart0("PS Command \r\n");
                    getstats();
                    calstats();

                }
        if (isCommand("reset", 0, 1,0))
                {

                    flag = 1;
                    NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
                }

        if(isCommand("clear",0,1,0)){
            flag = 1;
            putsUart0("\033[2J\033[H");
            putsUart0("6314-Advance Embedded  Microcontroller\r\n");
            putsUart0("Name-NIKHIL ASHOK KUMBHAR\r\n");
            putsUart0("ID-1001775805\r\n");
        }

        if(flag == 0){
            putsUart0("Invalid Shell Command \r\n");
        }
        yield();
        putsUart0("\r\n");

    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    bool ok;

    // Initialize hardware
    initHw();
    initRtos();
    initUart0();
//    // Power-up flash
    POWER = 1;
    waitMicrosecond(250000);
    POWER = 0;
    waitMicrosecond(250000);

    // Initialize semaphores
    keyPressed = createSemaphore(1,"keyPressed");
    keyReleased = createSemaphore(0,"keyReleased");
    flashReq = createSemaphore(5,"flashReq");
    resource = createSemaphore(1,"resource");

    // Add required idle process at lowest priority
        ok =  createThread(idle, "Idle", 15, 1024);

        // Add other processes
        ok &= createThread(lengthyFn, "LengthyFn", 12, 1024);
        ok &= createThread(flash4Hz, "Flash4Hz", 8, 1024);
        ok &= createThread(oneshot, "OneShot", 4, 1024);
        ok &= createThread(readKeys, "ReadKeys", 12, 1024);
        ok &= createThread(debounce, "Debounce", 12, 1024);
        ok &= createThread(important, "Important", 0, 1024);
        ok &= createThread(uncooperative, "Uncoop", 12, 1024);
        ok &= createThread(shell, "Shell", 12, 1024);

    // Start up RTOS
    if (ok)
        startRtos(); // never returns
    else
        RED_LED = 1;

    return 0;
}
