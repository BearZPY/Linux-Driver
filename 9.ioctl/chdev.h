#ifndef _CHDEV_H_
#define _CHDEV_H_


#define CHDEV_DEBUG


/* undef it, just in case */
#undef PDEBUG             
#ifdef CHDEV_DEBUG
    #ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
	#define PDEBUG(fmt,args...)    printk(KERN_EMERG "chdev: " fmt,## args)
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

#define CHDEV_BUFF_LEN 4


/* Use 'k' as magic number */
#define CHDEV_IOC_MAGIC 'k'  
/* Please use a different 8-bit number in your code */
 
#define CHDEV_IOC_RESET    _IO(CHDEV_IOC_MAGIC, 0)     
#define CHDEV_IOC_ROL	   _IOR(CHDEV_IOC_MAGIC, 1, int)
#define CHDEV_IOC_ROR      _IOW(CHDEV_IOC_MAGIC, 2, int)

#define CHDEV_IOC_MAXNR 4

#endif

