#include "mailbox.h"
#include <linux/spinlock.h>

MODULE_LICENSE("Dual BSD/GPL");
static spinlock_t slock;
SPIN_LOCK_UNLOCKED(slock);
unsigned long flags;

struct mailbox_head_t *mtsheadpdf;
struct mailbox_head_t *stmheadpdf;
struct mailbox_entry_t *mtskbuf;
struct mailbox_entry_t *stmkbuf;

static void get_process_name(char *ouput_name);
static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf);
static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count);

static struct kobject *hw2_kobject;
static struct kobj_attribute mailbox_attribute
    = __ATTR(mailbox, 0660, mailbox_read, mailbox_write);

static int num_entry_max = 2;

module_param(num_entry_max, int, S_IRUGO);

static void get_process_name(char *ouput_name)
{
	memcpy(ouput_name, current->comm, sizeof(current->comm));
}

static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf)
{
	spin_lock_irqsave(&slock,flags);

	if(strcmp(current->comm,"master") == 0) {
		if(stmheadpdf->number < 1) {
			spin_unlock_irqrestore(&slock,flags);
			return ERR_EMPTY;
		} else {
			printk("=====master read\n");
			(stmheadpdf->number)--;
			printk("master read buf = %s\n",buf);
			sprintf(buf,"%s",stmkbuf[stmheadpdf->number].path);
			int a = strlen(stmkbuf[stmheadpdf->number].path);
			list_del(&(stmkbuf[stmheadpdf->number].entry));
			strcpy(stmkbuf[stmheadpdf->number].path,"");
			printk("master read buf = %s\n",buf);
			spin_unlock_irqrestore(&slock,flags);
			return a;
		}
	} else {
		if(mtsheadpdf->number < 1) {
			spin_unlock_irqrestore(&slock,flags);
			return ERR_EMPTY;
		} else {
			printk("=====slave read\n");
			(mtsheadpdf->number)--;
			printk("slave read buf = %s \n",buf);
			sprintf(buf,"%s",mtskbuf[mtsheadpdf->number].path);
			printk("slave read buf = %s \n",buf);
			int a = strlen(mtskbuf[mtsheadpdf->number].path);
			list_del(&(mtskbuf[mtsheadpdf->number].entry));
			strcpy(mtskbuf[stmheadpdf->number].path,"");
			spin_unlock_irqrestore(&slock,flags);
			return a;
		}

	}

	/*

		printk("=======================read==========================\n");
		if(headpdf->number<=0) {
			spin_unlock_irqrestore(&slock,flags);
			return ERR_EMPTY;
		}

		if(strcmp(current->comm,"master") == 0) {
			printk("+++++master++++++\n");
			if(
			    kbuf[headpdf->number].path[0] >= '0' &&
			    kbuf[headpdf->number].path[0] <= '9') {
					printk("p= %s\n",kbuf[headpdf->number].path);
					printk("master read\n");
				(headpdf->number)--;
				int a = sprintf(buf,"%s",kbuf[headpdf->number].path);
				printk("buf== %s \n",buf);
				list_del(&(kbuf[headpdf->number].entry));
				spin_unlock_irqrestore(&slock,flags);
				return a;
			} else {
				spin_unlock_irqrestore(&slock,flags);
				return -1;
			}
		} else {
			printk("++++++slave+++++\n");
			if(kbuf[headpdf->number].path[0] < '0'||
			    kbuf[headpdf->number].path[0] > '9') {
				printk("p[0]= %s \n",kbuf[headpdf->number].path);
				printk("slave read\n");
				(headpdf->number)--;
				int a = sprintf(buf,"%s",kbuf[headpdf->number].path);
				printk("buf= %s \n",buf);
				list_del(&(kbuf[headpdf->number].entry));
				spin_unlock_irqrestore(&slock,flags);
				return a;
			} else {
				spin_unlock_irqrestore(&slock,flags);
				return ERR_EMPTY;
			}
		}
	*/
}

static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count)
{
	spin_lock_irqsave(&slock,flags);
	if(strcmp(current->comm,"master") == 0) {
		if(mtsheadpdf->number <= num_entry_max) {
			printk("=======master write\n");
			strcpy(mtskbuf[mtsheadpdf->number].path,buf);
			printk("master write = %s \n",mtskbuf[mtsheadpdf->number].path);
			list_add(&mtskbuf[(mtsheadpdf->number)].entry,&mtsheadpdf->head);
			(mtsheadpdf->number)++;
			spin_unlock_irqrestore(&slock,flags);
			return count;
		} else {
			spin_unlock_irqrestore(&slock,flags);
			return ERR_FULL;
		}
	} else {
		if(stmheadpdf->number <= num_entry_max) {
			printk("=======slave write\n");
			strcpy(stmkbuf[stmheadpdf->number].path,buf);
			printk("slave write = %s\n",stmkbuf[stmheadpdf->number].path);
			list_add(&stmkbuf[(stmheadpdf->number)].entry,&stmheadpdf->head);
			(stmheadpdf->number)++;
			spin_unlock_irqrestore(&slock,flags);
			return count;
		} else {
			spin_unlock_irqrestore(&slock,flags);
			return ERR_FULL;
		}
	}


	/*

		printk("=====================write=================\n");
		printk("headnum====%d\n",headpdf->number);
		if(headpdf->number<=num_entry_max) {
			printk("buf =%s\n",buf);
			strcpy(kbuf[headpdf->number].path,buf);
			list_add(&kbuf[(headpdf->number)].entry,&headpdf->head);
			printk("path =%s\n",kbuf[headpdf->number].path);
			(headpdf->number)++;
			printk("headnext\n");
			spin_unlock_irqrestore(&slock,flags);
			return count;
		} else {
			spin_unlock_irqrestore(&slock,flags);
			return ERR_FULL;
		}*/
}

static int __init mailbox_init(void)
{
	printk("Insert\n");
	hw2_kobject = kobject_create_and_add("hw2", kernel_kobj);
	sysfs_create_file(hw2_kobject, &mailbox_attribute.attr);

	mtsheadpdf=kmalloc(sizeof(struct mailbox_head_t),GFP_KERNEL);
	stmheadpdf=kmalloc(sizeof(struct mailbox_head_t),GFP_KERNEL);
	INIT_LIST_HEAD(&mtsheadpdf->head);
	INIT_LIST_HEAD(&stmheadpdf->head);
	mtsheadpdf->number=0;
	stmheadpdf->number=0;

	mtskbuf=kmalloc(sizeof(struct mailbox_entry_t)*num_entry_max,GFP_KERNEL);
	stmkbuf=kmalloc(sizeof(struct mailbox_entry_t)*num_entry_max,GFP_KERNEL);

	return 0;
}

static void __exit mailbox_exit(void)
{
	printk("Remove\n");
	kobject_put(hw2_kobject);
}

module_init(mailbox_init);
module_exit(mailbox_exit);
