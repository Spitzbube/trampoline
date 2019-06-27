#include "tpl_machine.h"
/*my code */

#include "tpl_machine_interface.h"
#include "tpl_os_application_def.h"
#include "tpl_os_definitions.h"
#include "tpl_os.h"
#if WITH_MEMORY_PROTECTION == YES
#include "tpl_memory_protection.h"
#endif
#if WITH_AUTOSAR == YES
#include "tpl_as_definitions.h"
#endif
#include "tpl_os_interrupt.h"

//#include "tpl_msp430x_definitions.h"
#include "msp430.h"

/*end my code */
//#define KERNEL_STACK_SIZE 400
//VAR(uint16,AUTOMATIC) tpl_kernel_stack[KERNEL_STACK_SIZE/2];

//function that should be defined for msp430...

#if TASK_COUNT > 0
extern FUNC(void, OS_CODE) CallTerminateTask(void);
#endif

#if ISR_COUNT > 0
extern FUNC(void, OS_CODE) CallTerminateISR2(void);
#endif

//extern FUNC(void, OS_CODE) CallTerminateTask(void);

//extern FUNC(void, OS_CODE) CallTerminateISR2(void);

//copied from cortex
//extern FUNC(void, OS_CODE) tpl_init_external_interrupts();
//
extern FUNC(void, OS_CODE) tpl_init_it_priority();

#define GPR_ON_EXCEPTION_FRAME  5
#define LR_IDX                  5
#define SR_IDX                  48
#define PC_IDX                  50
#define xPSR_IDX                7

FUNC(void, OS_CODE) tpl_init_context(
  CONST(tpl_proc_id, OS_APPL_DATA) proc_id)
{
#if WITH_PAINT_REGISTERS == YES || WITH_PAINT_STACK == YES
  VAR(int, AUTOMATIC) i;
#endif

  //pointer to the static descriptor of the process
  CONSTP2CONST(tpl_proc_static, AUTOMATIC, OS_APPL_DATA) the_proc = tpl_stat_proc_table[proc_id];

  //pointer to the context of the process
  CONSTP2VAR(msp430x_core_context, AUTOMATIC, OS_APPL_DATA) l_tpl_context = the_proc->context;

  //pointer to the stack of the process
  CONSTP2VAR(tpl_stack_word, AUTOMATIC, OS_APPL_DATA) stack = the_proc->stack.stack_zone;

  //size of the stack in 16 bit words above the exceptionf rame
  CONST(uint16, AUTOMATIC) size_of_stack_above_exception_frame = (the_proc->stack.stack_size - MSP430X_CORE_EXCEPTION_FRAME_SIZE) >> 2; /* changed from 2 to 1, maybe wrong*/

  //pointer to the exception frame
  CONSTP2VAR(tpl_stack_word, AUTOMATIC, OS_APPL_DATA) exception_frame = stack + size_of_stack_above_exception_frame;

#if WITH_PAINT_REGISTERS == YES
  l_tpl_context->gpr5 = 
  l_tpl_context->gpr6 = 
  l_tpl_context->gpr7 = 
  l_tpl_context->gpr8 = 
  l_tpl_context->gpr9 = 
  l_tpl_context->gpr10 = 
  l_tpl_context->gpr11 = OS_STACK_PATTERN;

  for(i = 0; i < GPR_ON_EXCEPTION_FRAME; i++)
  {
    exception_frame[i] = OS_STACK_PATTERN;
  }
#endif

  l_tpl_context->stackPointer = (uint16)exception_frame;

#if WITH_PAINT_STACK == YES
  for(i = 0; i < size_of_stack_above_exception_frame; i++)
  {
    stack[i] = OS_STACK_PATTERN;
  }
#endif

#if TASK_COUNT > 0
#if   ISR_COUNT > 0
  exception_frame[LR_IDX] = (IS_ROUTINE == the_proc->type) ? 
    (uint16)(CallTerminateISR2) :
    (uint16)(CallTerminateTask) ;
#else 
  exception_frame[LR_IDX] = (uint16)(CallTerminateTask);
#endif
#else
#if ISR_COUNT > 0
  exception_frame[LR_IDX] = (uint16)(CallTerminateISR2);
#else
exception_frame[LR_IDX] = NULL;
#endif
#endif
  //status register. Set the GIE bit (Global interrupt)
  exception_frame[SR_IDX] = (((uint32)(the_proc->entry) >> 16) << 12) | 0x8;
  exception_frame[PC_IDX] = (uint16)(the_proc->entry);

  exception_frame[xPSR_IDX] = 0x0100;

#if WITH_AUTOSAR_STACK_MONITORING == YES && WITH_PAINT_STACK == NO
  (*(uint8 *)(the_proc->stack.stack_zone)) = OS_STACK_PATTERN;
#endif

}

FUNC (void, OS_CODE) tpl_init_machine_generic (void)
{
  //copied from cortex
#if WITH_MEMORY_PROTECTION == YES
  tpl_init_mp();
#endif
}

FUNC(void, OS_CODE) tpl_init_machine_specific (void)
{
  //copied from cortex
  //tpl_set_systick_timer();
  //tpl_init_external_interrupts();
  //tpl_init_it_priority();
}

uint32 tpl_reentrancy_counter = 0;

void tpl_disable_interrupts() {
  __disable_interrupt();
}
void tpl_enable_interrupts() {
  __enable_interrupt();
}
void tpl_disable_os_interrupts() {
  tpl_disable_interrupts();
}
void tpl_enable_os_interrupts() {
  tpl_enable_interrupts();
}

//void tpl_sc_handler() {}

void idle_function(void)
{
	while(1);
}

void tpl_init_machine() {
  tpl_init_machine_generic();
  tpl_init_machine_specific();
};

void tpl_shutdown() {};
