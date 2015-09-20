#ifndef _LED_H_
#define _LED_H_


#define LED_DEBUG


/* undef it, just in case */
#undef PDEBUG             
#ifdef LED_DEBUG
    #ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
	#define PDEBUG(fmt,args...)    printk(KERN_EMERG "LED: " fmt,## args)
    #else
     /* This one for user space */
        #define PDEBUG(fmt,args...)     fprintf(stderr, fmt, ## args)
    #endif
#else
	/* not debugging: nothing */
    #define PDEBUG(fmt, args¡­) 
#endif

#undef PDEBUGG 
/* nothing: it's a placeholder */
#define PDEBUGG(fmt, args...) 

#define LED_BUFF_LEN 4


/* Use 'k' as magic number */
#define LED_IOC_MAGIC 'k'  
/* Please use a different 8-bit number in your code */
 
#define LED_IOC_RESET    _IO(LED_IOC_MAGIC, 0)     
#define LED_IOC_ROL	   _IOR(LED_IOC_MAGIC, 1, int)
#define LED_IOC_ROR      _IOW(LED_IOC_MAGIC, 2, int)

#define LED_IOC_MAXNR 4

#endif

