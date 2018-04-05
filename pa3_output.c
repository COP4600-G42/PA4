#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define CLASS_NAME    "pa3_output_class"
#define DEVICE_NAME   "pa3_output"
#define BUFFER_LENGTH 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Derya Hancock <deryahancock@knights.ucf.edu>, Jerasimos Strakosha <jstrakosha@knights.ucf.edu>, Richard Zarth <rlziii@knights.ucf.edu>");
MODULE_DESCRIPTION("A simple character-mode device output driver");
MODULE_VERSION("2.0");

/* FUNCTION PROTOTYPES */
int int_module(void);
void cleanup_module(void);
static int dev_open(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static int dev_release(struct inode *, struct file *);

/* GLOBAL VARIABLES */
static int majorNumber;
static struct class *pa3OutputClass = NULL;
static struct device *pa3OutputDevice = NULL;
static int numberOfOpens = 0;

/* MUTEX LOCK */
extern struct mutex pa3_mutex;

/* EXTERNAL VARIABLES */
extern int messageLen;
extern char message[BUFFER_LENGTH];

static struct file_operations fops =
{
    .open    = dev_open,
    .read    = dev_read,
    .release = dev_release,
};

int init_module(void)
{
    printk(KERN_INFO "PA3 OUTPUT: Initializing module.\n");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

    if (majorNumber < 0)
    {
        printk(KERN_ALERT "PA3 OUTPUT: Failed to register a major number.\n");

        return majorNumber;
    }

    printk(KERN_INFO "PA3 OUTPUT: Registered with major number %d.\n", majorNumber);

    pa3OutputClass = class_create(THIS_MODULE, CLASS_NAME);

    if (IS_ERR(pa3OutputClass))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);

        printk(KERN_ALERT "PA3 OUTPUT: Failed to register a class.\n");

        return PTR_ERR(pa3OutputClass);
    }

    printk(KERN_INFO "PA3 OUTPUT: Device class registered.\n");

    pa3OutputDevice = device_create(pa3OutputClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

    if (IS_ERR(pa3OutputDevice))
    {
        class_destroy(pa3OutputClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);

        printk(KERN_ALERT "PA3 OUTPUT: Failed to create device.\n");

        return PTR_ERR(pa3OutputDevice);
    }

    printk(KERN_INFO "PA3 OUTPUT: Device created successfully.\n");

    return 0;
}

void cleanup_module(void)
{
    device_destroy(pa3OutputClass, MKDEV(majorNumber, 0));
    class_unregister(pa3OutputClass);
    class_destroy(pa3OutputClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "PA3 OUTPUT: Removing module.\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "\nPA3 OUTPUT: OPEN Full string: %s\n", message);

    if (!mutex_trylock(&pa3_mutex))
    {
        printk(KERN_ALERT "PA3 OUTPUT: Device already in use by another process.\n");

        return -EBUSY;
    }

    numberOfOpens++;

    printk(KERN_INFO "PA3 OUTPUT: Device has been opened %d time(s).\n", numberOfOpens);

    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int errorCount = 0;
    int i = 0;
    int stringLen = messageLen;

    printk(KERN_INFO "\nPA3 OUTPUT: READ Full string: %s\n", message);

    // If the requested read length is more than the available space
    // Then reduce the read length to the maximum available
    // Else use the requested read length

    if (len > messageLen)
    {
        len = messageLen;
    }

    errorCount = copy_to_user(buffer, message, len);

     while (stringLen > 0)
     {
        message[i] = message[i + len];
        i++;
        stringLen--;
     }

    // Reduce the message lenght each time we read
    messageLen -= len;

    if (errorCount == 0)
    {
        printk(KERN_INFO "PA3 OUTPUT: Sent %d characters to the user.\n", len);

        printk(KERN_INFO "PA3 OUTPUT: READ Full string: %s\n", message);

        return len;
    } else {
        printk(KERN_INFO "PA3 OUTPUT: Failed to send %d characters to the user.\n", errorCount);

        return -EFAULT;
    }
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&pa3_mutex);

    printk(KERN_INFO "\nPA3 OUTPUT: RELEASE Full string: %s\n", message);

    printk(KERN_INFO "PA3 OUTPUT: Device successfully closed.\n");

    return 0;
}
