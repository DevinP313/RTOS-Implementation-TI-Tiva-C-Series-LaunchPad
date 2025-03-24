/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           APPLICATION CODE
*
*                                      Texas Instruments TM4C129x
*                                                on the
*                                             DK-TM4C129X
*                                           Development Kit
*       		Modified by Dr. Samir A. Rawashdeh, for the TM4C123GH6PM microcontroller on the 
*						TM4C123G Tiva C Series Launchpad (TM4C123GXL), November 2014, updated March 2021
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
* Note(s)       : None.
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  "app_cfg.h"
#include  <cpu_core.h>
#include  <os.h>

#include  "..\bsp\bsp.h"
#include  "..\bsp\bsp_led.h"
#include  "..\bsp\bsp_sys.h"

// SAR Addition
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define sw1Flag 0x01
#define sw2Flag 0x02


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK       AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK       ButtonMonitorStk[APP_CFG_TASK_START_STK_SIZE]; //ButtonMonitor
static  OS_STK       BlinkyStk[APP_CFG_TASK_START_STK_SIZE]; //Blinky
static  OS_STK       ButtonAlertStk[APP_CFG_TASK_START_STK_SIZE]; //ButtonAlert
static  OS_STK       DebuggingVarsStk[APP_CFG_TASK_START_STK_SIZE]; //DebuggingVars

/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskCreate         (void);
static  void  AppTaskStart          (void       *p_arg);
static void ButtonMonitor(void *p_arg);
static void Blinky(void *p_arg);
static void ButtonAlert(void *p_arg);
static void DebuggingVars(void *p_arg);

OS_FLAG_GRP *switchFlags;
OS_EVENT *UARTsemaphore;

/*$PAGE*/
/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : (1) It is assumed that your code will call main() once you have performed all necessary
*                   initialization.
*********************************************************************************************************
*/

int  main (void)
{
#if (OS_TASK_NAME_EN > 0)
    CPU_INT08U  err;
#endif

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR     cpu_err;
#endif

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)"TM4C129XNCZAD",
                (CPU_ERR  *)&cpu_err);
#endif

    CPU_IntDis();                                               /* Disable all interrupts.                              */

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"          */

    OSTaskCreateExt((void (*)(void *)) AppTaskStart,           /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_START_PRIO,
                    (INT16U          ) APP_CFG_TASK_START_PRIO,
                    (OS_STK         *)&AppTaskStartStk[0],
                    (INT32U          ) APP_CFG_TASK_START_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));							
		

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_TASK_START_PRIO, "Start", &err);
#endif

		//OSFlagCreate
		switchFlags = OSFlagCreate(0x00, &err);
										
		//OSSemCreate
		UARTsemaphore = OSSemCreate(1);
										
    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */

    while (1) {
        ;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           App_TaskStart()
*
* Description : Startup task example code.
*
* Arguments   : p_arg       Argument passed by 'OSTaskCreate()'.
*
* Returns     : none.
*
* Created by  : main().
*
* Notes       : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    uint32_t sw2_status;

		CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;

		(void)p_arg;                                                /* See Note #1                                              */


   (void)&p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */

    cpu_clk_freq = BSP_SysClkFreqGet();                         /* Determine SysTick reference freq.                    */
    cnts         = cpu_clk_freq                                 /* Determine nbr SysTick increments                     */
                 / (CPU_INT32U)OS_TICKS_PER_SEC;

    OS_CPU_SysTickInit(cnts);
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                                   */
#endif

    Mem_Init();

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif


		BSP_LED_Toggle(0);
		OSTimeDlyHMSM(0, 0, 0, 200);
		BSP_LED_Toggle(0);
		BSP_LED_Toggle(1);
		OSTimeDlyHMSM(0, 0, 0, 200);
		BSP_LED_Toggle(1);
		BSP_LED_Toggle(2);
		OSTimeDlyHMSM(0, 0, 0, 200);    
		BSP_LED_Toggle(2);

		OSTimeDlyHMSM(0, 0, 1, 0);   

		AppTaskCreate();                                            /* Creates all the necessary application tasks.         */

					
		while (DEF_ON) {

        OSTimeDlyHMSM(0, 0, 0, 100);			
			

			// Demonstrating button interface
//			sw2_status = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4); 
//			if (sw2_status == 0) 
//			{
//			UARTprintf("\n\r Button2 was pressed\n\r");	// Probably needs to be protected by semaphore
//			
//			}

    }
}

/*
*********************************************************************************************************
*                                         AppTaskCreate()
*
* Description :  Create the application tasks.
*
* Argument(s) :  none.
*
* Return(s)   :  none.
*
* Caller(s)   :  AppTaskStart()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{										
OSTaskCreate((void (*)(void *)) ButtonMonitor,
										(void           *) 0,
										(OS_STK         *)&ButtonMonitorStk[APP_CFG_TASK_START_STK_SIZE - 1],
										(INT8U           ) 3 );
										
OSTaskCreate((void (*)(void *)) Blinky,
										(void           *) 0,
										(OS_STK         *)&BlinkyStk[APP_CFG_TASK_START_STK_SIZE - 1],
										(INT8U           ) 4 );
										
OSTaskCreate((void (*)(void *)) ButtonAlert,
										(void           *) 0,
										(OS_STK         *)&ButtonAlertStk[APP_CFG_TASK_START_STK_SIZE - 1],
										(INT8U           ) 5 );
										
OSTaskCreate((void (*)(void *)) DebuggingVars,
										(void           *) 0,
										(OS_STK         *)&DebuggingVarsStk[APP_CFG_TASK_START_STK_SIZE - 1],
										(INT8U           ) 6 );
										
}


static void ButtonMonitor(void *p_arg)
{
    //(void)p_arg; // Prevent compiler warning for unused variable
	
		INT8U err;
		int32_t sw1Val;
		int32_t sw2Val;
		
    while (1)
		{
			sw1Val = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
			sw2Val = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
			
			if(sw1Val == 0)
			{
				OSFlagPost(switchFlags, sw1Flag, OS_FLAG_SET, &err);
			}
			
			if(sw2Val == 0)
			{
				OSFlagPost(switchFlags, sw2Flag, OS_FLAG_SET, &err);
			}
        // Delay some time to avoid button bounce and reading too fast
        OSTimeDlyHMSM(0, 0, 0, 500);
    }
}
static void Blinky(void *p_arg)
{
		//(void)p_arg; // Prevent compiler warning for unused variable
	
		INT8U err;
		int32_t blinkRate = 500; //initial blink rate
		OS_FLAGS val;
	
		while(1)
		{
			val = OSFlagAccept(switchFlags, sw1Flag, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, &err);
			/**There's no need to manually clear the sw1Flag with OSFlagPost
				 if using OS_FLAG_CONSUME with OSFlagAccept, 
			   as OS_FLAG_CONSUME should automatically clear the flag**/
			if (val == sw1Flag) 
				{
					blinkRate	= 100 + (rand() % (1000 - 100 + 1));
					
					OSSemPend(UARTsemaphore, 0, &err);
					UARTprintf("Blink rate is now: %d \n\n", blinkRate);
					OSSemPost(UARTsemaphore);
				}
			BSP_LED_Toggle(2);
			OSTimeDlyHMSM(0, 0, 0, blinkRate);
		}
}

static void ButtonAlert(void *p_arg)
{
		INT8U err;
		OS_FLAGS val;
	
		while(1)
		{
			val = OSFlagPend(switchFlags, sw2Flag, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &err);
			
			if (val == sw2Flag)
				{
					OSSemPend(UARTsemaphore, 0, &err);
					UARTprintf("SW2 is pressed \n\n");
					//OSFlagPost(switchFlags, sw2Flag, OS_FLAG_CLR, &err);
					OSSemPost(UARTsemaphore);
				}
			OSTimeDlyHMSM(0, 0, 0, 200);
		}
}

static void DebuggingVars(void *p_arg)
{
		INT8U err;
	
    while (1)
    {
			OSSemPend(UARTsemaphore, 0, &err);
			UARTprintf("OSCPUUsage: %d \n", OSCPUUsage);
      UARTprintf("OSCtxSwCtr: %d \n", OSCtxSwCtr);
      UARTprintf("OSIdleCtr: %d \n", OSIdleCtr);
			UARTprintf("The number of times the task was switched in: %d \n\n", OSTCBPrioTbl[4]->OSTCBCtxSwCtr);
			OSSemPost(UARTsemaphore);
			OSTimeDlyHMSM(0, 0, 0, 1000);
    }
}
