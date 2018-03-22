#include "master.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>

//char send[1000];
//char buffer[50];

int count=0;
int sendnumber=0;
int main(int argc, char **argv)
{
	printf("master run");
	int NUM_SLAVE=1;
	int sysfs_fd;
	sysfs_fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	char *QUERY_WORD;
	char *DIRECTORY;
	char *num;

	int opt;
	while((opt=getopt(argc,argv,"q:d:s:"))!=-1) {
		switch(opt) {
		case 'q':
			QUERY_WORD=optarg;
			break;
		case 'd':
			DIRECTORY=optarg;
			break;
		case 's':
			num=optarg;
			NUM_SLAVE=atoi(num);
			break;
		}
	}
	printf("q=%s d=%s s=%d \n",QUERY_WORD,DIRECTORY,NUM_SLAVE);

	DIR *dir;
	struct dirent *ptr;
	dir=opendir(DIRECTORY);
	char *name=malloc(sizeof(char)*100);
	char send[50][50];
	while((ptr=readdir(dir))!=NULL) {
		name=ptr->d_name;
		if(name[0]!='.') {
			strcat(send[sendnumber],DIRECTORY);
			strcat(send[sendnumber],name);
			printf("send=%s \n",send[sendnumber]);
			sendnumber++;
		}
	}

	struct mail_t *mail=malloc(sizeof(struct mail_t)*sendnumber);
	int d;
	for(d=0; d<sendnumber; d++) {
		strcpy(mail[d].data.query_word,"");
		strcpy(mail[d].file_path,"");
		strcpy(mail[d].data.query_word,QUERY_WORD);
		strcpy(mail[d].file_path,send[d]);
	}

	pid_t slave_pid[NUM_SLAVE];
	int j;
	for(j=0; j<NUM_SLAVE; j++) {
		slave_pid[j]=fork();
		if(slave_pid[j]==0) {
			printf("fork\n");
			execl("/home/user/hw2_mailbox/slave",NULL);
			printf("error\n");
		}
	}
	printf("sendnum=%d\n",sendnumber);
	int a=0,b=0;
	struct mail_t get;
	while (a!=sendnumber||b!=sendnumber) {
		if(a!=sendnumber) {
			int sendf=send_to_fd(sysfs_fd,&mail[a]);
//			printf("a=%d\n",a);
			if(sendf == 1) {
				a++;
			}
		}
		if(b!=sendnumber) {
			int recf=receive_from_fd(sysfs_fd,&get);
//			printf("b=%d\n",b);
			if(recf == 1) {
				b++;
			}
		}
	}
	close(sysfs_fd);
	printf("count=%u\n",count);
	int p;
	for(p=0; p<NUM_SLAVE; p++) {
		kill(slave_pid[p],2);
	}

}


int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	char msend[100]= {};
	strcpy(msend,mail->data.query_word);
	strcat(msend," ");
	strcat(msend,mail->file_path);
	printf("sendmesg:%s\n",msend);
	int ret_val = write(sysfs_fd,msend,sizeof(msend));
	if (ret_val == ERR_FULL) {
		printf("full");
		return -1;
	} else {
		return 1;
	}
}

int receive_from_fd(int sysfs_fd, struct mail_t *mail)
{
	close(sysfs_fd);
	sysfs_fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	char mget[100]= {};
	strcpy(mget,"");
	int ret_val = read(sysfs_fd,mget,sizeof(mget));
	//if (mget[0] < '0' || mget[0] < '9'){
//		return -1;
//	}
	if (ret_val == ERR_EMPTY) {
		return -1;
	} else {
		printf("getmesg:%s\n",mget);
		int gg=0;
		sscanf(mget,"%d",&gg);
		count+=gg;
	}
	close(sysfs_fd);
	sysfs_fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	return 1;
}
