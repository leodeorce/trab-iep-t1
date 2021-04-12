All going well, the process to build the kernel module should be straightforward, provided that you have installed the Linux headers as described in the first article. The steps are as follows:

$ make
$ ls -l *.ko
$ ls -l test 
$ sudo insmod chardrv.ko
$ lsmod

The device is now present in the /dev directory, with the following attributes:

$ cd /dev
$ ls -l char*
 crw------- 1 root root 240, 0 Apr  8 19:28 chardrv

You can see that, in this case, the major number is 240 — this is automatically assigned by the code!

We can then use the writechardrv and readchardrv program to test that the LKM is working correctly. 

$ sudo ./writechardrv

 Starting device write code example...
 Type in a short string to send to the kernel module:
 This is a test of the chardrv LKM
 Writing message to the device [This is a test of the chardrv LKM].
 Goodbye!

$ sudo ./readchardrv
 Starting device read code example...
 Reading from the device...
 The received message is: [This is a test of the chardrv LKM(32 letters)]
 Goodbye!

$ sudo rmmod ebbchar

$ sudo dmesg

There are two significant problems with the current LKM. The first is that the LKM device can only be accessed with superuser permissions, and the second is that the current LKM is not multi-process safe.

Adding Mutex Locks

The Linux kernel provides a full implementation of semaphores — a data type (struct semaphore) that is used for controlling access by multiple processes to a shared resource. The easiest way to use this semaphore code within the kernel is to use mutexes, as there is a full set of helper functions and macros available.

A simple way to prevent the problems described above is to prevent two processes from using the /dev/chardrv device at the same time. A mutex is a lock that can set (put down) before a process begins using a shared resource. The lock can then be released (brought up) when the process is finished using the shared resource. When the lock has been set, no other process can access the locked code region. Once the mutex lock has been released by the process that locked it, the shared region of code is once again available to be accessed by the other process, which in turn locks the resource.

There are only very minor changes required to the code in order to implement mutex locks. An outline of these changes is provided below:

####################################################
#include <linux/mutex.h>	         // Required for the mutex functionality
...

static DEFINE_MUTEX(chardrv_mutex);  // A macro that is used to declare a new mutex that is visible in this file
                                     // results in a semaphore variable chardrv_mutex with value 1 (unlocked)
                                     // DEFINE_MUTEX_LOCKED() results in a variable with value 0 (locked)
...

static int __init chardrv_init(void){
   ...
   mutex_init(&chardrv_mutex);       // Initialize the mutex lock dynamically at runtime
   ...
}

static void __exit chardrv_exit(void){
   mutex_destroy(&chardrv_mutex);        /// destroy the dynamically-allocated mutex
   ...
}

static int dev_open(struct inode *inodep, struct file *filep){
   if(!mutex_trylock(&chardrv_mutex)){    // Try to acquire the mutex (i.e., put the lock on/down)
                                          // returns 1 if successful and 0 if there is contention
      printk(KERN_ALERT "Chardrv: Device in use by another process");
      return -EBUSY;
   }
   ...
}

static int dev_release(struct inode *inodep, struct file *filep){
   mutex_unlock(&chardrv_mutex);          /// Releases the mutex (i.e., the lock goes up)
   ...
}
 
######################################################


