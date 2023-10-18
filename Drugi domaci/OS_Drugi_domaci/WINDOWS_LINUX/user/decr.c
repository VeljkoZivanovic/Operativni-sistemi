#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"

void
help()
{
	printf("\nUse this program to decrypt files written od the disk.\nUsage: encr [OPTION]... [FILE]...\n\nCommand line options:\n        -h, --help: Show help prompt.\n        -a, --decrypt-all files in CWD with current key.\n");
}

void 
decrypt(char *file_name)
{
	int fd = open(file_name, O_WRONLY);
	if (fd < 0) 
	{
		printf("File does not exist.\n");
		return;
	}
	
	int res = decr(fd);
	//printf("%s\n", file_name);
	if (res == -1)
		printf("Key does not exist.\n");
	else if (res == -2)
		printf("Node is T_DEV type.\n");
	else if (res == -3)
		printf("File is not encrypted.\n");
	
	close(fd);
}

char*
fmtname(char *path)
{
	static char buf[DIRSIZ+1];
	char *p;

	// Find first character after last slash.
	for(p=path+strlen(path); p >= path && *p != '/'; p--)
		;
	p++;

	// Return blank-padded name.
	if(strlen(p) >= DIRSIZ)
		return p;
	memmove(buf, p, strlen(p));
	//memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
	return buf;
}


void
decrypt_all()
{
	char buf[512], *p;
	int fd;
	struct stat st;
	struct dirent de;

	if((fd = open(".", 0)) < 0){
		fprintf(2, "ls: cannot open %s\n", ".");
		exit();
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "ls: cannot stat %s\n", ".");
		close(fd);
		exit();
	}
	
	strcpy(buf, ".");
	p = buf+strlen(buf);
	*p++ = '/';
	while(read(fd, &de, sizeof(de)) == sizeof(de)){
		if(de.inum == 0)
			continue;
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
		if(stat(buf, &st) < 0){
			printf("ls: cannot stat %s\n", buf);
			continue;
		}
		//printf("%s\n", fmtname(buf));
		decrypt(fmtname(buf));
	}
}


int
main(int argc, char *argv[])
{
	int i;
	char file_name[20] = "long.txt";

	for(i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			help();
			exit();		
		}
		else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--decrypt-all") == 0) 
		{
			decrypt_all();
			exit();
		}
		else strcpy(file_name, argv[i]);
	}

	if (strcmp(file_name, "") == 0) {
		help();
		exit();	
	}
	int fd = open(file_name, O_WRONLY);
	if (fd < 0) 
	{
		printf("File does not exist.\n");
		exit();
	}
	
	decrypt(file_name);
	
	exit();
}
