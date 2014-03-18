#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<errno.h>
#include<string.h>
#include <sys/wait.h>
#include<fcntl.h>
#include<limits.h>
//Defining A Structure
typedef struct node
{
	char name[10000];
	int status;
	int pid;
	char name2[10000];
}node;
char seps[]=" \t";
node procs[10000];
int counters=0;
int flag=0;
pid_t pp;
char *source;
int array[10000];
int ctr=0;
char *in1='\0';
char *out1='\0';
//Function for executing all the basic shell commands
void execs(char *arr[],int rlflag,int rgflag)
{
	int sin=dup(STDIN_FILENO);
	int sout=dup(STDOUT_FILENO);
	pp=fork();
	if(flag==0)
	{
		if(pp<0)
		{
			perror("Error");
			exit(0);
		}
		else if (pp==0)
		{
			if(rlflag==1)
			{
				FILE *fp=fopen(in1,"r");
				dup2(fileno(fp),0);
				fclose(fp);
			}
			if(rgflag==1)
			{
				FILE *fp=fopen(out1,"w+");
				dup2(fileno(fp),1);
				fclose(fp);
			}
			int val=execvp(arr[0],arr);
			if(val<0)
			{
				perror("Command not found");
				exit(0);
			}
		}
		else
		{	int s;
			waitpid(pp,&s,WUNTRACED);
			if(WIFSTOPPED(s))
			{
				strcpy(procs[counters].name,arr[0]);
				procs[counters].pid=pp;
				procs[counters++].status=1;
			}

		}

	}
	else if (flag==1)
	{
		if(pp<0)
		{
			perror("Error");
			exit(0);
		}
		else if (pp==0)
		{	if(rlflag==1)
			{
				FILE *fp=fopen(in1,"r");
				dup2(fileno(fp),0);
			}
			if(rgflag==1)
			{
				FILE *fp=fopen(out1,"w+");
				dup2(fileno(fp),1);
			}

			int val=execvp(arr[0],arr);
			if(val<0)
			{
				perror("Command Not Found");
				exit(0);
			}
		}
		else
		{
			strcpy(procs[counters].name,arr[0]);
			if((strcmp(arr[0],"gedit")==0||strcmp(arr[0],"emacs")==0)&&arr[1]!=NULL)
				strcpy(procs[counters].name2,arr[1]);
			procs[counters].pid=pp;
			procs[counters++].status=1;
		}
	}
	dup2(sout,1);
	dup2(sin,0);
	close(sout);
	close(sin);
}
int pflag=0,lo=0;
//Function for displaying prompt
void prompt()
{	
	char *username,*cwd=NULL,hostname[100],finalpath[1000];
	strcpy (finalpath,"~");
	username=getenv("USER");
	gethostname(hostname,100);
	cwd=getcwd(cwd,1000);
	char *path=strstr(cwd,source);
	if(path==NULL)
	{
		printf("%s@%s:%s> ",username,hostname,cwd);
		return;
	}
	path=path+strlen(source);
	strcat(finalpath,path);
	printf("%s@%s:%s> ",username,hostname,finalpath);
	return;
}
int rgflag,rlflag;
//Function for implementing cd command
void changedir(char *arr[])
{
	char *cwd='\0',pres[10000],arg[10000];

	if ((arr[1]==NULL) || (!strcmp(arr[1],"~")) || (!strcmp(arr[1],"~/")))
	{
		if(chdir(source))
			perror("chdir()");
		else
			return;
	}
	else
	{
		strcpy(arg,arr[1]);
	}
	cwd = getcwd(cwd,1000);
	strcpy(pres,cwd);
	strcat(pres,"/");
	strcat(pres, arr[1]);
	if(chdir(pres))
		perror("chdir()");
	else
		return;
}
//Function for parsing with redirection handling included
int point;int i,j;
void parse(char input[],char *arr[])
{	
	for(lo=0;lo<i;lo++)
		if(input[lo]=='|')
		{	
			pflag=1;
			break;
		}
	for(lo=0;lo<i;lo++)
		if(input[lo]=='<')
		{
			rlflag=1;
			break;
		}
	for(lo=0;lo<i;lo++)
		if(input[lo]=='>')
		{	rgflag=1;
			break;
		}

	int i,j;ctr=0;
	char input3[10000];
	strcpy(input3,input);
	char input4[10000];
	strcpy(input4,input);
	char *part=strtok(input,seps);
	while(part!='\0')
	{
		arr[ctr]=part;
		if(!strcmp(part,">"))
		{
			arr[ctr]='\0';
			point=ctr;
		}
		else if(!strcmp(part,"<"))
		{
			arr[ctr]='\0';
			point=ctr;
		}
		ctr++;
		part=strtok(NULL,seps);
	}
	arr[ctr]='\0';ctr++;
	if(rlflag==1)
	{
		char sep[]="<";
		char *part=strtok(input3,sep);
		part=strtok(NULL,sep);
		char *part2;
		part2=strtok(part,seps);
		in1=part2;
	}
	if(rgflag==1)
	{
		char sep1[]=">";
		char *part1=strtok(input4,sep1);
		part1=strtok(NULL,sep1);
		char *part3=strtok(part1,seps);
		out1=part3;
	}

	return;
}
//Function for handling background processes
void child(int signum)
{
	int p,i=0,count=0;
	p = waitpid(WAIT_ANY, NULL, WNOHANG);
	for(i=0;i<counters;i++)
	{
		if(procs[i].pid==p)
		{
			int j=0;
			fflush(stdout);
			printf("\n%s %d exited normally\n",procs[i].name,procs[i].pid);
			prompt();
			fflush(stdout);
			procs[i].status=0;
		}
	}
	signal(SIGCHLD, child);
	return;
}
//Function for handling signals
void signalhandling(int signum)
{
	if(signum==20)
		kill(pp,20);
	if(signum==2 || signum ==3)
	{
		fflush(stdout);
		printf("\n");
		prompt();
		fflush(stdout);
		signal(SIGINT, signalhandling);
		signal(SIGQUIT, signalhandling);
		signal(SIGTSTP,signalhandling);
	}
	return;
}
int main ()
{
	signal(SIGINT,SIG_IGN);
	signal(SIGINT,signalhandling);
	signal(SIGCHLD,SIG_IGN);
	signal(SIGCHLD,child);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTSTP,signalhandling);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGQUIT,signalhandling);
	source=getenv("PWD");
	pid_t main_pid=getpid();
	//If-else satements for different commands
	while(1)
	{
		prompt();
		j=0;i=0;
		char input[10000]={'\0'},ch;
		char *arr[10000];
		scanf("%c",&ch);
		int sflag=0;
		if(ch=='\n')
			sflag=1;
		while(ch!='\n')
		{
			if(ch=='&')
			{
				scanf("%c",&ch);
				flag=1;
				i--;
				continue;
			}
			input[i++]=ch;
			scanf("%c",&ch);
		}
		input[i]='\0';
		char input1[10000];
		strcpy(input1,input);
		ctr=0;
		rlflag=0;rgflag=0;pflag=0;
		parse(input1,arr);
		char input2[10000];
		strcpy(input2,input);
		if(arr[0]==NULL)
		{
			continue;
		}
//When there are both pipes and redirection		
		if(pflag!=0)
		{
			char *baby = NULL, *arrs[1000];
			int newPipe[2], oldPipe[2], pipesCount, aCount=0,i,status; 
			pid_t pid;char *ar[10000];
			pipesCount = 0;
			char *parts=strtok(input2,"|");
			while(parts!=NULL)
			{
				arrs[pipesCount]=parts;
				pipesCount++;
				parts=strtok(NULL,"|");
			}
			arrs[pipesCount++]=parts;
			pipesCount--;		

			int incop, outcop;				
			// creating copy of stdin and stdout 
			incop=dup(STDIN_FILENO);	
			outcop=dup(STDOUT_FILENO);

			for(i = 0; i < pipesCount; i++) 
				// For each command 
			{ 
				
				if(i < pipesCount-1) 
					pipe(newPipe); 
				// create a pipe 
				rlflag=0;rgflag=0;
				parse(arrs[i],ar);
				pid = fork();
				if(i>0 && i<pipesCount-1)	
				{
					dup2(oldPipe[0], 0); 
					close(oldPipe[1]);
					close(oldPipe[0]);
				}
				else if(i==pipesCount-1)	
					//for last command.
				{
					dup2(oldPipe[0], 0); 
					close(oldPipe[1]);
					close(oldPipe[0]);

				}

				if(pid == 0)  
					// Child 
				{
					if(i==0)
					{
						dup2(newPipe[1], 1); 
						close(newPipe[0]);
						close(newPipe[1]);

					}
					if(i>0 && i<pipesCount-1)
					{
						dup2(newPipe[1], 1); 
						close(newPipe[0]);
						close(newPipe[1]);

					}
					if(rlflag==1)
					{
						FILE *fp=fopen(in1,"r");
						dup2(fileno(fp),0);
					}
					if(rgflag==1)
					{
						int fp=open(out1,O_CREAT | O_WRONLY,0700);
						dup2(fp,1);
					}

					int val=execvp(ar[0],ar);
					if(val<0)
					{
						perror("Command Not Found");
						exit(0);
					}


				} 
				else 
				{		
					waitpid(pid, &status, 0);



					// saving pipes

					if(i < pipesCount-1) 
					{
						oldPipe[0] = newPipe[0];
						oldPipe[1] = newPipe[1];
					}
				}
			}

			//restore stdin and stdout 

			close(oldPipe[0]);
			close(newPipe[0]);
			close(oldPipe[1]);
			close(newPipe[1]);
			
			dup2(incop, 0);
			dup2(outcop, 1);
			close(incop);
			close(outcop);
			continue;
		}
//User Defined Commands
		if (strcmp(arr[0],"quit")==0)
			return;
		else if(strcmp(arr[0],"cd")==0)
			changedir(arr);
		else if(strcmp(arr[0],"pinfo")==0&&arr[1]=='\0')
		{
			int p=getpid(),ank=0,anks=0;char a[100],b[100];
			while(p!=0)
			{
				a[ank++]=(p%10+'0');
				p/=10;
			}
			int k=0;
			for(anks=ank-1;anks>=0;anks--)
			{
				b[k++]=a[anks];
			}
			b[k]='\0';
			char fp[1000],fp2[1000];
			strcpy(fp,"/proc/");
			strcat(fp,b);
			strcpy(fp2,fp);
			strcat(fp,"/status");
			FILE *fo;
			char *l;
			size_t len=0;
			ssize_t re;
			fo=fopen(fp,"r");
			if(fo>0)
			{
				int ctr2=0;
				while(re=getline(&l,&len,fo)!=-1)
				{
					if(ctr2==0||ctr2==1||ctr2==3||ctr2==11)
						printf("%s",l);
					ctr2++;
				}
				fclose(fo);
			}
			strcat(fp2,"/exe");
			int nn;char ar[50];
			nn=readlink(fp2,ar,50);
			ar[nn]='\0';
			printf("Executable Path: %s\n",ar);
			//	fo=fopen(fp,r);
			//			printf("%s\n",fp);
		}
		else if(strcmp(arr[0],"pinfo")==0)
		{
			//printf("The PID of Process is - %d\n",getpid());
			int p=atoi(arr[1]),ank=0,anks=0;char a[100],b[100];
			//printf("%s\n",arr[1]);
			while(p!=0)
			{
				a[ank++]=(p%10+'0');
				p/=10;
			}
			int k=0;
			for(anks=ank-1;anks>=0;anks--)
			{
				b[k++]=a[anks];
			}
			b[k]='\0';
			char fp[1000],fp2[1000];
			strcpy(fp,"/proc/");
			//                      printf("%s\n",b);
			strcat(fp,b);
			strcpy(fp2,fp);
			strcat(fp,"/status");
			//printf("%s\n",fp);
			FILE *fo;
			char *l;
			size_t len=0;
			ssize_t re;
			fo=fopen(fp,"r");
			if(fo>0)
			{
				int ctr2=0;
				while(re=getline(&l,&len,fo)!=-1)
				{
					if(ctr2==0||ctr2==11||ctr2==1||ctr2==3)
						printf("%s",l);
					ctr2++;
				}
				fclose(fo);
			}
			else
			{
				printf("Invalid PID\n");
				continue;
			}
			strcat(fp2,"/exe");
			int nn;char ar[50];
			nn=readlink(fp2,ar,50);
			ar[nn]='\0';
			printf("Executable Path: %s\n",ar);
			//                      printf("%s\n",fp);
		}
		else if(strcmp(arr[0],"jobs")==0)
		{
			int ctr1=0,ctr5=0;
			for(ctr1=0;ctr1<counters;ctr1++)
			{
				if(procs[ctr1].status==1)
				{
					if((strcmp(procs[ctr1].name,"gedit")==0||strcmp(procs[ctr1].name,"emacs")==0)&&arr[1]!=NULL)
						printf("%d. %s %s [%d]\n",ctr5+1,procs[ctr1].name,procs[ctr1].name2,procs[ctr1].pid);	
					else	
						printf("%d. %s [%d]\n",ctr5+1,procs[ctr1].name,procs[ctr1].pid);
					array[ctr5]=ctr1;
					ctr5++;
				}
			}
		}
		else if(strcmp(arr[0],"kjob")==0)
		{
			int st;
			/*int ctr1=0;
			  for(ctr1=0;ctr1<counters;ctr1++)
			  {
			  if(procs[ctr1].status==1)
			  {
			//printf("%d. %s [%d]\n",ctr1+1,procs[ctr1].name,procs[ctr1].pid);
			}
			}*/
			if(atoi(arr[1])-1>=counters||atoi(arr[1])-1<0)
			{
				printf("Wrong Index No.\n");
				continue;
			}
			kill(procs[array[atoi(arr[1])-1]].pid,atoi(arr[2]));
		}
		else if(strcmp(arr[0],"overkill")==0)
		{
			int ctr1=0;
			for(ctr1=0;ctr1<counters;ctr1++)
			{
				if(procs[ctr1].status==1)
					kill(procs[ctr1].pid,9);
			}
		}
		else if(strcmp(arr[0],"fg")==0)
		{
			int s;
			procs[array[atoi(arr[1])-1]].status=0;
			waitpid(procs[array[atoi(arr[1])-1]].pid,&s,0);
			printf("%d %s exited normally\n",procs[array[atoi(arr[1])-1]].pid,procs[array[atoi(arr[1])-1]].name);
		}
		else
		{
			if(flag==1)
				strcat(input,"&");
			execs(arr,rlflag,rgflag);
		}
		flag=0;
	}
	return 0;
}
