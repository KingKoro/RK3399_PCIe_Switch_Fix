#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/smp.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Florian");
MODULE_DESCRIPTION("A Test Driver for Specific Single Core Execution LKM");

#define CPU_ID (5)

//Carrier struct for function parameters, castable to void*
struct argument_carrier_castable
{
	//Input Parameters
	int something_number;
	int another_number;
	//Return Value
	int return_something;
};

static void test_func_smp(void *arguments)
{
	//Check if CPU id is 5
	int this_cpu = get_cpu();
	pr_info("[MYMODULE] test_func_smp called with CPU Core: %d \n", this_cpu);
	//Extract Function Arguments, copy to local variables
	struct argument_carrier_castable *argument_ptr_casted;
	argument_ptr_casted = (struct argument_carrier_castable*)arguments;
	int something_number = argument_ptr_casted->something_number;
	int another_number = argument_ptr_casted->another_number;
	//Test arguments
	pr_info("[MYMODULE] number A = %d \n", something_number);
	pr_info("[MYMODULE] number B = %d \n", another_number);
	//Set return Value, by reference
	argument_ptr_casted->return_something = something_number + another_number;
	put_cpu();
	//Signal OK Execution
	return; //Optional, only for good practice
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void)
{
	int this_cpu = get_cpu();
	//immediately leave non-preemptive/atomic execution becasue any sleep/wait method called in this function-area (like e.g. wait_for_completion() or msleep())
	//will cause Kernel Bug + Trace
	// get_cpu() = disable preemptive execution
	// put_cpu() = enable preemptive execution
	// Alternative: Just call smp_processor_id() to get CPU ID without preemptive disable
	put_cpu();
	pr_info("[MYMODULE] Loading Singlecore Test Kernel Module...\n");
	pr_info("[MYMODULE] MAIN called with CPU Core: %d \n", this_cpu);
	//Prepare Arguments for function
	struct argument_carrier_castable argument_carrier;
	argument_carrier.something_number = 1;
	argument_carrier.another_number = 2;
	argument_carrier.return_something = 0; //optional
	void *arguments_ptr = (void*)&argument_carrier;
	//Call function
	int ret = smp_call_function_single(CPU_ID, test_func_smp, arguments_ptr, 1);
	//Handle possible Error
	if(ret) 
	{
 	
    		pr_info("[MYMODULE] ERROR: Cannot run function on other Core!\n");
 		goto err_module;
	}
	//Show result
	pr_info("[MYMODULE] Result from function = %d \n", argument_carrier.return_something);
	//put_cpu();
  	
	return 0;
	 
	err_module:
  		//put_cpu();
  
  	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) 
{
	pr_info("[MYMODULE] Unloading Singlecore Test Kernel Module...\n");
}

module_init(my_init);
module_exit(my_exit);


