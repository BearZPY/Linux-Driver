#ifndef _PWM_H_
#define _PWM_H_


/* Use 'k' as magic number */
#define PWM_IOC_MAGIC 'k'  
/* Please use a different 8-bit number in your code */
 
#define PWM_IOC_RESET               _IO(PWM_IOC_MAGIC, 0)
#define PWM_IOC_SET_TIME0_FREQ      _IOR(PWM_IOC_MAGIC, 1, int)
#define PWM_IOC_SET_TIME0_DUTY      _IOW(PWM_IOC_MAGIC, 2, int)
#define PWM_IOC_SET_TIME1_FREQ      _IOR(PWM_IOC_MAGIC, 3, int)
#define PWM_IOC_SET_TIME1_DUTY      _IOW(PWM_IOC_MAGIC, 4, int)
#define PWM_IOC_SET_TIME2_FREQ      _IOR(PWM_IOC_MAGIC, 5, int)
#define PWM_IOC_SET_TIME2_DUTY      _IOW(PWM_IOC_MAGIC, 6, int)
#define PWM_IOC_SET_TIME3_FREQ      _IOR(PWM_IOC_MAGIC, 7, int)
#define PWM_IOC_SET_TIME3_DUTY      _IOW(PWM_IOC_MAGIC, 8, int)

#define PWM_IOC_MAXNR 10

#endif

