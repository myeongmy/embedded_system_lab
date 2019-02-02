#include <ucos_ii.h>
#include <stm32f4xx.h>
#include "GLCD.h"




void LED_Init (void) {
  /* Enable clock for GPIOG, GPIOH and GPIOI                                  */
  RCC->AHB1ENR |= (1UL << 6) | (1UL << 7) | (1UL << 8) ;

  /* Configure Port G pins PG6, PG7, PG8:                                     */
  /* Pins in Push-pull output mode, 50 MHz Fast Speed with Pull-up resistors  */
  GPIOG->MODER   &= ~((3UL << 2*6) | (3UL << 2*7) | (3UL << 2*8));
  GPIOG->MODER   |=  ((1UL << 2*6) | (1UL << 2*7) | (1UL << 2*8));
  GPIOG->OTYPER  &= ~((1UL <<   6) | (1UL <<   7) | (1UL <<   8));
  GPIOG->OSPEEDR &= ~((3UL << 2*6) | (3UL << 2*7) | (3UL << 2*8));
  GPIOG->OSPEEDR |=  ((2UL << 2*6) | (2UL << 2*7) | (2UL << 2*8));
  GPIOG->PUPDR   &= ~((3UL << 2*6) | (3UL << 2*7) | (3UL << 2*8));
  GPIOG->PUPDR   |=  ((1UL << 2*6) | (1UL << 2*7) | (1UL << 2*8));

  /* Configure Port H: PH2, PH3, PH6, PH7                                     */
  /* Pins in Push-pull output mode, 50 MHz Fast Speed with Pull-up resistors  */
  GPIOH->MODER   &= ~((3UL << 2*2) | (3UL << 2*3) | (3UL << 2*6) | (3UL << 2*7));
  GPIOH->MODER   |=  ((1UL << 2*2) | (1UL << 2*3) | (1UL << 2*6) | (1UL << 2*7));
  GPIOH->OTYPER  &= ~((1UL <<   2) | (1UL <<   3) | (1UL <<   6) | (1UL <<   7));
  GPIOH->OSPEEDR &= ~((3UL << 2*2) | (3UL << 2*3) | (3UL << 2*6) | (3UL << 2*7));
  GPIOH->OSPEEDR |=  ((2UL << 2*2) | (2UL << 2*3) | (2UL << 2*6) | (2UL << 2*7));
  GPIOH->PUPDR   &= ~((3UL << 2*2) | (3UL << 2*3) | (3UL << 2*6) | (3UL << 2*7));
  GPIOH->PUPDR   |=  ((1UL << 2*2) | (1UL << 2*3) | (1UL << 2*6) | (1UL << 2*7));

  /* Configure Port I pin PI10:                                               */
  /* Pins in Push-pull output mode, 50 MHz Fast Speed with Pull-up resistors  */
  GPIOI->MODER   &= ~(3UL << 2*10);
  GPIOI->MODER   |=  (1UL << 2*10);
  GPIOI->OTYPER  &= ~(1UL <<   10);
  GPIOI->OSPEEDR &= ~(3UL << 2*10);
  GPIOI->OSPEEDR |=  (2UL << 2*10);
  GPIOI->PUPDR   &= ~(3UL << 2*10);
  GPIOI->PUPDR   |=  (1UL << 2*10);
}

void BUT_Init (void) {
  /* Enable clock and init GPIO inputs */
  RCC->AHB1ENR |= (1UL << 0) |
                  (1UL << 2) |
                  (1UL << 6) ;

  GPIOA->MODER &= ~(3UL << 2* 0);
  GPIOC->MODER &= ~(3UL << 2*13);
  GPIOG->MODER &= ~(3UL << 2*15);
}

#define TASK_STK_SIZE 512
#define N_TASKS 2

OS_STK TaskStk[N_TASKS][TASK_STK_SIZE];

void	wait	(void)		{
int d;
for	(d	=	0;	d	<	2000000;	d++);
}
void Task1(void *pdata)
{
		tetrismain();
	
while	(1)	
{	
}
}
void Task2(void *pdata){
	GLCD_Init();
	pongmain();
	
	while(1){
	}
}


#define  TASK_START_PRIO                        4
#define  TASK_START_STK_SIZE                    128
static   OS_STK      TaskStartStk[TASK_START_STK_SIZE];

static  void  TaskStart (void *p_arg)
{
	 INT8U err;
   INT32U  cnts;

	SystemCoreClockUpdate();
   cnts = SystemCoreClock / OS_TICKS_PER_SEC;
	 OS_CPU_SysTickInit(cnts);
   
		err = OSTaskCreate(Task1, (void *)0, &TaskStk[0][TASK_STK_SIZE - 1], 10);
	 err = OSTaskCreate(Task2, (void *)0, &TaskStk[1][TASK_STK_SIZE - 1], 11);

	 OSTaskDel(OS_PRIO_SELF); 	
}


int  main (void)
{
    INT8U  err;

    LED_Init();

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"              */
		
    OSTaskCreateExt(TaskStart,                               /* Create the start task                                    */
                    (void *)0,
                    (OS_STK *)&TaskStartStk[TASK_START_STK_SIZE - 1],
                    TASK_START_PRIO,
                    TASK_START_PRIO,
                    (OS_STK *)&TaskStartStk[0],
                    TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);


    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)       */
}
