#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define CLASS_NAME    "pa4_input_class"
#define DEVICE_NAME   "pa4_input"
#define BUFFER_LENGTH 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Derya Hancock <deryahancock@knights.ucf.edu>, Jerasimos Strakosha <jstrakosha@knights.ucf.edu>, Richard Zarth <rlziii@knights.ucf.edu>");
MODULE_DESCRIPTION("A simple character-mode device input driver");
MODULE_VERSION("2.0");

/* FUNCTION PROTOTYPES */
int int_module(void);
void cleanup_module(void);
static int dev_open(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int dev_release(struct inode *, struct file *);

/* GLOBAL VARIABLES */
static int majorNumber;
static struct class *pa4InputClass = NULL;
static struct device *pa4InputDevice = NULL;
static int numberOfOpens = 0;
static char receivedMessage[BUFFER_LENGTH] = {0};
static char rewrittenMessage[BUFFER_LENGTH] = {0};

/* MUTEX LOCK */
struct mutex pa4_mutex;
EXPORT_SYMBOL(pa4_mutex);

/* EXTERNAL VARIABLES */
static int messageLen;
EXPORT_SYMBOL(messageLen);
static char message[BUFFER_LENGTH] = {0};
EXPORT_SYMBOL(message);

static struct file_operations fops =
{
    .open    = dev_open,
    .write   = dev_write,
    .release = dev_release,
};

int init_module(void)
{
    printk(KERN_INFO "PA4  INPUT: Initializing module.\n");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

    if (majorNumber < 0)
    {
        printk(KERN_ALERT "PA4  INPUT: Failed to register a major number.\n");

        return majorNumber;
    }

    printk(KERN_INFO "PA4  INPUT: Registered with major number %d.\n", majorNumber);

    pa4InputClass = class_create(THIS_MODULE, CLASS_NAME);

    if (IS_ERR(pa4InputClass))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);

        printk(KERN_ALERT "PA4  INPUT: Failed to register a class.\n");

        return PTR_ERR(pa4InputClass);
    }

    printk(KERN_INFO "PA4  INPUT: Device class registered.\n");

    pa4InputDevice = device_create(pa4InputClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

    if (IS_ERR(pa4InputDevice))
    {
        class_destroy(pa4InputClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);

        printk(KERN_ALERT "PA4  INPUT: Failed to create device.\n");

        return PTR_ERR(pa4InputDevice);
    }

    mutex_init(&pa4_mutex);

    printk(KERN_INFO "PA4  INPUT: Device created successfully.\n");

    return 0;
}

void cleanup_module(void)
{
    mutex_destroy(&pa4_mutex);

    device_destroy(pa4InputClass, MKDEV(majorNumber, 0));
    class_unregister(pa4InputClass);
    class_destroy(pa4InputClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "PA4  INPUT: Removing module.\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "\nPA4  INPUT: OPEN Full string: %s\n", message);

    if (!mutex_trylock(&pa4_mutex))
    {
        printk(KERN_ALERT "PA4  INPUT: Device already in use by another process.\n");

        return -EBUSY;
    }

    numberOfOpens++;

    printk(KERN_INFO "PA4  INPUT: Device has been opened %d time(s).\n", numberOfOpens);

    return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    int errorCount = 0;
    int i = 0, k=0, j=0;
    int startLen = messageLen;
    int ucfCount=0;
    char * replace = "ndefeated 2018 National Champions UCF" //38 chars


    printk(KERN_INFO "\nPA4  INPUT: WRITE Full string: %s\n", message);

    // If the requested write length is more than the available space
    // Then reduce the write length to the maximum available
    // Else use the requested write length
    if  ((len + messageLen) > BUFFER_LENGTH)
    {
        len = BUFFER_LENGTH - messageLen;
        messageLen = BUFFER_LENGTH;
    } else {
        messageLen += len;
    }

    errorCount = copy_from_user(receivedMessage, buffer, len);

    printk(KERN_INFO"PA4  INPUT: before the loop i = %d, startLen = %d, len = %d, messageLen = %d\n", i, startLen, len, messageLen);

    for (i = 0; i < len; i++)
    {
        // Before writing check again if where we are writing is not bigger than the buffer
        if (startLen > BUFFER_LENGTH)
        {
            // Get out of the for loop
            break;
        }

                if (receivedMessage[i]=="U")
                {
                    message[startLen] = receivedMessage[i];
                    startLen++;

                    if(i+2<= len)
                    {    
                        if (receivedMessage[i+1]=="C" && receivedMessage[i+2]=="F") 
                        {
                            if (startLen+37 <= BUFFER_LENGTH)
                             {
                                //write whole U  ndefeated 2018 National Champions UCF//36 chars
                                for (k=0;k<37;k++)
                                {   
                                   message[startLen] = replace[k] ;
                                   startLen++;
                                }
          
                             }  
                             else if (startLen+37 > BUFFER_LENGTH  )
                             {
                                //means you can write partial 
                                m= BUFFER_LENGTH -startLen;
                                for (j=0;j<m;j++)
                                {   
                                   message[startLen] = replace[j] ;
                                   startLen++;
                                }

                             }   
                        }
                    }    
                }

                else
                {
                    message[startLen] = receivedMessage[i];
                    startLen++;


                }    
    
    }

    

    printk(KERN_INFO "PA4  INPUT: Received %zu characters from the user.\n", len);

    printk(KERN_INFO "PA4  INPUT: WRITE Full string: %s\n", message);

    return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    mutex_unlock(&pa4_mutex);

    printk(KERN_INFO "\nPA4  INPUT: RELEASE Full string: %s\n", message);

    printk(KERN_INFO "PA4  INPUT: Device successfully closed.\n");

    return 0;
}
