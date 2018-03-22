#include "slave.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{
	printf("slave run\n");
	struct mail_t get;
	while(1) {
		int sysfs_fd;
		sysfs_fd=open("/sys/kernel/hw2/mailbox",O_RDWR);

		if(receive_from_fd(sysfs_fd,&get) > 0) {
			int count=0;
			FILE *file =fopen(get.file_path,"r");
			printf("file_path=%s\n",get.file_path);
			char file_get[1000];
			while(fgets(file_get,1000,file)) {
				printf("fget=%s\n",file_get);
//				fgets(file_get,1000,file);
				char *p1=file_get;
				while(strstr(p1,get.data.query_word)!=NULL) {
					p1=strstr(p1,get.data.query_word)+1;
					count++;
				}
			}
			fclose(file);
			get.data.word_count=count;
			printf("count=%d\n",get.data.word_count);

			int a;
			a=send_to_fd(sysfs_fd,&get);
			while(a == -1) {
				a=send_to_fd(sysfs_fd,&get);
			}
		}
		close(sysfs_fd);
	}
}


int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	printf("slave send\n");
	char send[100]= {};
	sprintf(send,"%d",mail->data.word_count);
	strcat(send," ");
	strcat(send,mail->file_path);
	int ret_val = write(sysfs_fd,send,sizeof(send));
	if (ret_val == ERR_FULL) {
		return -1;
	} else {
		return 1;
	}
}

int receive_from_fd(int sysfs_fd, struct mail_t *mail)
{
	char buffer[100]= {};
	int ret_val = read(sysfs_fd,buffer,sizeof(buffer));
	if (ret_val == ERR_EMPTY) {
		close(sysfs_fd);
		sysfs_fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
		//	printf("empty\n");
		return -1;
	} else {
		if(buffer==NULL) {
			close(sysfs_fd);
			sysfs_fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
			return -1;
		}
		printf("res buf=%s\n",buffer);
		sscanf(buffer,"%s %s",mail->data.query_word,mail->file_path);
	}
	close(sysfs_fd);
	sysfs_fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	return 1;
}

