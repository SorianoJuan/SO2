#include <string.h>
#include <math.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Priorities at which the tasks are created. */
#define RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define	PERIODIC_TASK_PRIORITY		( tskIDLE_PRIORITY + 4 )
#define	APERIODIC_TASK_PRIORITY		( tskIDLE_PRIORITY + 4 )
/* The bit of port 0 that the LPCXpresso LPC13xx LED is connected. */
#define mainLED_BIT 		( 22 )
#define MAX_POINTERS		10

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define PERIODIC_SEND_FREQUENCY_MS	( 50 / portTICK_RATE_MS )

//Defines del programa
#define MAX_STRING_LEN	60
#define BUFF_TMP 10
#define portNEW_DELAY 200

static void vReceiverTask(void *pvParameters);
static void vPeriodicSenderTask(void *pvParameters);
static void vAperiodicSenderTask(void *pvParameters);

static void vToggleLED(void);
char * vRandomString(int str_length);

void UART3_Init(void);
void UART_Send(char* datos, int size);

/* Declare a variable of type QueueHandle_t to hold the handle of the queue being created. */
QueueHandle_t xPointerQueue;

/*Estructura del ejemplo del libro para el envio de mensaje de usuario */
struct msg_struct {
	char * msg;
	char num;
	int len_str;
};

int main(void) {
	/* P0_22 for the LED. */
	LPC_PINCON->PINSEL1 &= (~(3 << 12));
	LPC_GPIO0->FIODIR |= (1 << mainLED_BIT);

	/* Enable traceanalycer snapshot */
	vTraceEnable(TRC_START);

	/* Create a queue that can hold a maximum of MAX_POINTERS pointers*/
	xPointerQueue = xQueueCreate(MAX_POINTERS, sizeof(struct msg_struct *)); //creo cola de 10 punteros a struct

	if (xPointerQueue != NULL) {

		/*
		 * The size of the stack used by the idle task is defined by the applicationdefined constant configMINIMAL_STACK_SIZE . The value assigned to
		 * this constant in the standard FreeRTOS Cortex-M3 demo applications is the minimum recommended for any task. If your task uses a lot of stack
		 * space, then you must assign a larger value.		 *
		 *
		 *For example, the Cortex-M3 stack is 32 bits wide so, if
		 *usStackDepth is passed in as 100, then 400 bytes of stack space will be allocated (100 * 4 bytes).
		 *
		 */

		//taskcode(function), name, stacksize, parameters, handletask
		xTaskCreate(vReceiverTask, "Transmisor", configMINIMAL_STACK_SIZE,
		NULL, RECEIVE_TASK_PRIORITY, NULL);

		xTaskCreate(vPeriodicSenderTask, "Temp",
		configMINIMAL_STACK_SIZE, NULL, PERIODIC_TASK_PRIORITY,
		NULL);

		xTaskCreate(vAperiodicSenderTask, "User", configMINIMAL_STACK_SIZE,
				NULL, APERIODIC_TASK_PRIORITY	, NULL);

		/**
		 * Passing a uxPriority value above (configMAX_PRIORITIES – 1) will result in the priority assigned to the task being capped silently to the
		 * maximum legitimate value.
		 *
		 */

		vTaskStartScheduler();

	}

	/* If all is well we will never reach here as the scheduler will now be
	 running.  If we do reach here then it is likely that there was insufficient
	 heap available for the idle task to be created. */
	for (;;)
		;
	return 0;
}


/**
 * Tarea de envio periodico representando el seensor de temperatura. Se genera
 * un valor entre 0 y 255, emulando medicion, y esta tarea se encarga de comunicarla
 * con el proceso central.
 *
 *
 * @param void*  pvParameters
 */
static void vPeriodicSenderTask(void *pvParameters) {

	/*	This parameter is named on the assumption that vTaskDelayUntil() is
	 being used to implement a task that executes periodically and with a
	 fixed frequency. */

	/* The xLastWakeTime variable needs to be initialized with the current tick
	 count. Note that this is the only time the variable is written to explicitly.
	 After this xLastWakeTime is updated automatically internally within
	 vTaskDelayUntil(). */
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	/* Define a task that performs an action every 10 milliseconds. */
	const TickType_t xPeriod = pdMS_TO_TICKS(10);

	struct msg_struct *msg = pvPortMalloc(sizeof(struct msg_struct *));

	/* Enter the loop that defines the task behavior. */
	for (;;) {

		/* This task should execute every 50 milliseconds. Time is measured
		 in ticks. The pdMS_TO_TICKS macro is used to convert milliseconds
		 into ticks. xLastWakeTime is automatically updated within vTaskDelayUntil()
		 so is not explicitly updated by the task. */

		/* Place this task in the blocked state until it is time to run again.
		 The block state is specified in ticks, the constant used converts ticks
		 to ms.  While in the blocked state this task will not consume any CPU
		 time.

		 The parameters to vTaskDelayUntil() specify, instead, the exact tick count value at which the
		 calling task should be moved from the Blocked state into the Ready state. vTaskDelayUntil()
		 is the API function that should be used when a fixed execution period is required (where you
		 want your task to execute periodically with a fixed frequency), as the time at which the calling
		 task is unblocked is absolute, rather than relative to when the function was called (as is the
		 case with vTaskDelay()).

		 */
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

		msg->msg = NULL;
		msg->num = (char) (rand() % 255); //genero valor aleatorio de 0 a 255 y lo casteo a char

		/* Send to the queue - causing the queue receive task to flash its LED.
		 *
		 * 	0 is used as the block time so the sending operation will not block -
		 * 	it shouldn't need to block as the queue should always be empty at this
		 * 	point in the code.
		 *
		 * 	*/

		/*(pdMS_TO_TICKS(5): The maximum amount of time the task should remain in the
		 *Blocked state to wait for space to become available on the queue,
		 *should the queue already be full.*/
		xQueueSend(xPointerQueue, &msg, pdMS_TO_TICKS(1));
		//espera 5 [ms] si la cola esta llena


		/*Free a estructura*/
		vPortFree(msg);

	}
}

/**
 * Funcion Aperiodica representa el ingreso de caracteres de un usuario. Se genera una cadena de longitud
 * variable, de manera aleatoria, y se envia aperiodicamente (i,e cada cierto tiempo que tambien es variable)
 *
 * @param: void*  pvParameters
 *
 */
static void vAperiodicSenderTask(void *pvParameters) {

	portTickType xLastWakeTime;
	/*Aloco estructura donde colocara cada string generado para colocar en la cola*/
	struct msg_struct *msg = pvPortMalloc(sizeof(struct msg_struct *));
	msg->msg = pvPortMalloc(sizeof(char *));

	//Para limpiar
	msg->num = 0;
	int random_len = 0;

	xLastWakeTime = xTaskGetTickCount();

	/* Define a task that performs an action every random milisecons starting*/
	const TickType_t xPeriod = pdMS_TO_TICKS((rand() % (100 - 70) + 70)); //100-200 ms

	for (;;) {
		/* Place this task in the blocked state until it is time to run again.
		 The block state is specified in ticks, the constant used converts ticks
		 to ms.  While in the blocked state this task will not consume any CPU
		 time. */
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

		random_len = (rand() + xTaskGetTickCount()) % (MAX_STRING_LEN - 3);

		/* Send to the queue - causing the queue receive task to flash its LED.*/
		msg->msg = vRandomString(random_len);
		msg->len_str = random_len;

		/*Envio string, esperando hasta 2[ms]  si la cola esta llena
		 *
		 * xQueueSend(), xQueueSendToFront() and xQueueSendToBack() will
		 * return immediately if xTicksToWait is zero and the queue is already full.
		 *
		 * */
		xQueueSend(xPointerQueue, &msg, pdMS_TO_TICKS(2));

		/*Free a estructura*/
		vPortFree(msg->msg);
	}

}

/**
 * Tarea de recepcion. Se fija en la cola si hay algo y lo recibe. Compara campos de la estructura, y lo
 * reenvia por puerto serie.
 *
 *
 * @param void*  pvParameters
 */

static void vReceiverTask(void *pvParameters) {

	struct msg_struct * pcReceivedValue; //creo puntero a estructura

	char tmp_buffer[BUFF_TMP];

	/*Modulo UART*/
	UART3_Init();

	for (;;) {
		/* Wait until something arrives in the queue - this task will block
		 indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		 FreeRTOSConfig.h. */

		/* Receive the address of a buffer. */
		if (xQueueReceive(xPointerQueue, /* The handle of the queue. */
		&pcReceivedValue, /* Store the buffer’s address in pcReceivedString. */
		portNEW_DELAY) == pdTRUE) {


			if (pcReceivedValue->msg == NULL) { //Enviar string de tarea periodica

				/*Convierto valor recibido, a base 10, y lo pongo en tmp buf*/
				itoa(pcReceivedValue->num, tmp_buffer, 10);

				/*Concateno \r\n*/
				strcat(tmp_buffer, "\r\n");

				/*Mando la temperatura recibida a traves del puerto UART*/
				UART_Send(tmp_buffer, strlen(tmp_buffer));

			} else {  //Enviar string de tarea aperiodica

				UART_Send(pcReceivedValue->msg, pcReceivedValue->len_str);

			}

			/* toggle LED 22 */
			vToggleLED();

			/*Free a estructura*/
			vPortFree(pcReceivedValue);
		}
	}
}

char * vRandomString(int str_length) {
	static char* caracteres = "311emqsth0auid0ja8vp327qo5vo9dmucylo4n4w26012y9qnojfhwoolna1aa";

	char* var_string = pvPortMalloc(str_length += 3);

	for (int i = 0; i < str_length - 3; i++) {
		var_string[i] = caracteres[rand() % MAX_STRING_LEN];
	}

	return var_string;
}

/**
 * toggle LED 22
 */
static void vToggleLED(void) {
	unsigned long ulLEDState;

	/* Obtain the current P0 state. */
	ulLEDState = LPC_GPIO0->FIOPIN;

	/* Turn the LED off if it was on, and on if it was off. */
	LPC_GPIO0->FIOCLR = ulLEDState & (1 << mainLED_BIT);
	LPC_GPIO0->FIOSET = ((~ulLEDState) & (1 << mainLED_BIT));
}

/**
 * Configuracion Modulo UART3
 */
void UART3_Init(void) {

	/*UART3, chau UART1*/
	LPC_SC->PCONP |= (1 << 25);
	/*Deshabilito tambien UART0 y UART1*/
	LPC_SC->PCONP &= ~(3 << 3);
	/*PCLKSEL1*/
	LPC_SC->PCLKSEL1 |= (1 << 18);

	/*8 bits word*/
	LPC_UART3->LCR = 0x03;
	/*Bit de stop*/
	LPC_UART3->LCR |= (1 << 2);
	/*Config*/
	LPC_UART3->LCR |= 0b10000000;
	LPC_UART3->DLL = 54; //*U3LCR 0b10100001 ; // 115200
	LPC_UART3->DLM = 0; //*U3LCR
	LPC_UART3->LCR &= ~(1 << 7);

	//pin 0 TXD0 pin 1 RXD0 P0
	LPC_PINCON->PINSEL0 = 0b1010; // *PINSEL0 configurar los pines port 0
	LPC_PINCON->PINMODE0 = 0; // *PINMODE0 pin a pull up
}

void UART_Send(char* data, int size) {
	for (int i = 0; i < size; i++) {
		while ((LPC_UART3->LSR & (1 << 5)) == 0) {
		} //*U3LSR // Wait for Previous transmission
		LPC_UART3->THR = data[i]; //*U3THR
	}
}

void vConfigureTimerForRunTimeStats(void) {
	const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00,
			TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	 collecting run time statistical information - basically the percentage
	 of CPU time that each task is utilising.  It is called automatically when
	 the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	 to 1). */

	/* Power up and feed the timer. */
	LPC_SC->PCONP |= 0x02UL;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3 << 2))) | (0x01 << 2);

	/* Reset Timer 0 */
	LPC_TIM0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	LPC_TIM0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	 but not too fast so as to overflow all the time. */
	LPC_TIM0->PR = ( configCPU_CLOCK_HZ / 10000UL) - 1UL;

	/* Start the counter. */
	LPC_TIM0->TCR = TCR_COUNT_ENABLE;
}

/*
 * Necessary functions for FreeRTOS
 */
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
	/* This function will get called if a task overflows its stack. */
	(void) pxTask;
	(void) pcTaskName;
	for (;;)
		;
}
