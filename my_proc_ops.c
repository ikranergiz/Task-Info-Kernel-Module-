#include <linux/slab.h>    /*for kmalloc()*/
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#include <asm/uaccess.h>

#include <linux/module.h>  /* Needed by all modules */

#include <linux/sched/signal.h> // Needed for for_each_task macro

#include <linux/jiffies.h>
#include <linux/param.h>
#include <linux/types.h>

#define MYBUF_SIZE 1000
#define MYDATA_SIZE 256

int process_number_from_user = 3;
int i = 0;


struct my_data
{
    int size;
    char *buf; /* my data starts here */
};

struct process_info
{
    size_t process_counter;
    pid_t pid;
    u64 utime;
    u64 stime;
    u64 total_time;
    u64 vruntime;
};
/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 * @brief
 *
 * @param inode
 * @param file
 * @return int
 */
int my_open(struct inode *inode, struct file *file)
{
    struct my_data *my_data = kmalloc(sizeof(struct my_data) * MYBUF_SIZE, GFP_KERNEL);
    my_data->buf = kmalloc(sizeof(char) * MYBUF_SIZE, GFP_KERNEL);
    my_data->size = snprintf(my_data->buf, MYBUF_SIZE,  "process running times\n");
    my_data->size = MYBUF_SIZE;

    file->private_data = my_data;

    return 0;
}
/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 *
 * @param inode
 * @param file
 * @return int
 */
int my_release(struct inode *inode, struct file *file)
{
    /*free all memories*/
    struct my_data *my_data = file->private_data;
    kfree(my_data->buf);
    kfree(my_data);

    return 0;
}

/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 * @brief copy data from mydata->buffer to user_buf,
 * file: opened file data
 * usr_buf: usr buffer
 * offset: the cursor position on the mydata->buf from the last call
 * file:
 */
ssize_t my_read(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{
    struct process_info *process_info = kmalloc(sizeof(struct process_info)* MYBUF_SIZE, GFP_KERNEL);
    struct my_data *my_data = (struct my_data *)file->private_data;
    
    int len = 0;

    struct task_struct* task;
    size_t process_counter = 0;

    for_each_process(task) 
    {
        if (task && process_counter < MYBUF_SIZE)
        {
            //Gettting task info.
            process_info[process_counter].process_counter = process_counter;
            process_info[process_counter].pid = task->pid;
            process_info[process_counter].utime = nsecs_to_jiffies64(task->utime) / HZ;
            process_info[process_counter].stime = nsecs_to_jiffies64(task->stime) / HZ;
            process_info[process_counter].total_time = process_info[process_counter].utime + process_info[process_counter].stime;
            process_info[process_counter].vruntime = task->se.vruntime;

            ++process_counter;
        }

    }

    for (int i = 0; i < process_counter; ++i)
    {
        for (int j = 0; j < process_counter; ++j)
        {
            if(process_info[i].total_time > process_info[j].total_time)
            {
                
                struct process_info temp;

                //Swapping
                temp.process_counter = process_info[i].process_counter;
                temp.pid = process_info[i].pid;
                temp.utime = process_info[i].utime;
                temp.stime =  process_info[i].stime;
                temp.total_time = temp.stime +  temp.utime;
                temp.vruntime = process_info[i].vruntime; 

                process_info[i].process_counter = process_info[j].process_counter;
                process_info[i].pid = process_info[j].pid;
                process_info[i].utime = process_info[j].utime;
                process_info[i].stime = process_info[j].stime;
                process_info[i].total_time = process_info[j].utime + process_info[j].stime;
                process_info[i].vruntime = process_info[j].vruntime;

                process_info[j].process_counter = temp.process_counter;
                process_info[j].pid = temp.pid;
                process_info[j].utime = temp.utime;
                process_info[j].stime = temp.stime;
                process_info[j].total_time = temp.utime + temp.stime;
                process_info[j].vruntime = temp.vruntime;            
            }
        }
    }
   
    //Copy number of 'process_number_from_user' process to user.
    len = min((int)(my_data->size - *offset), (int)size);
    
    if ((len <= 0) )
        return 0; //end of file
    
    if(i > process_number_from_user){
        i = 0;
        process_number_from_user = 3;
        return 0;
    }

    if (copy_to_user(usr_buf, my_data->buf, len))
        return -EFAULT;
    
    if(i <= process_number_from_user)
    {
        *offset += snprintf(my_data->buf, (MYBUF_SIZE), "%d)\tpid = %d\tutime = %llu\tstime = %llu\tutime+stime = %llu\tvruntime = %llu\n",i+1,
                        process_info[i].pid,
                        process_info[i].utime,
                        process_info[i].stime,
                        process_info[i].total_time,
                        process_info[i].vruntime);

        i += 1;
    }
    
    kfree(process_info);
    return len; /*the number of bytes copied*/
}

ssize_t my_read_simple(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{

    char buf[MYBUF_SIZE] = {'\0'};

    int len = sprintf(buf, "Hello World\n");

    if (copy_to_user(usr_buf, buf, len))
        return -EFAULT;

    return len; /*the number of bytes copied*/
}

/**
 * @brief TODO: task infoya gore ayarlamaniz gerekiyor
 *
 * @param file
 * @param usr_buf
 * @param size
 * @param offset
 * @return ssize_t
 */
ssize_t my_write(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{
    char *buf = kmalloc(size + 1, GFP_KERNEL);

    if (copy_from_user(buf, usr_buf, size))
        return -EFAULT;

    *offset += size;
   
    //Convert char* to int.
    if (kstrtoint(buf, 10, &process_number_from_user) == 0)
    {
        printk(KERN_INFO "process_number_from_user: %d\n", process_number_from_user);
    }

    buf[size] = '\0';

    printk(KERN_INFO "the value of kernel buf: %s", buf);

    kfree(buf);
    return size;
}
