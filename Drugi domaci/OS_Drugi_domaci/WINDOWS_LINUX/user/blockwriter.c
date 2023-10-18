#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"

// samo ispisuje abecedu u bafer
void make_buffer(char* buff) {
	for (int i = 0; i < 512; i++)
		buff[i] = 'a'+(i%26);
}


int
main(int argc, char *argv[])
{
	int i;
	int blocks = 150;
	char file_name[20] = "long.txt";
 	char buf[512];
	make_buffer(buf);

	for(i = 1; i < argc; i++) 
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printf("Use this program to create a big file filled with a-z characters.\nDefault filename: long.txt\nDefault blocks: 150\nUsage: blockwriter [OPTION]...\n\nCommand line options:\n        -h, --help: Show help prompt.\n        -b, --blocks: Number of blocks to write.\n        -o, --output-file: Set output filename.");
			exit();		
		}
		else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--blocks") == 0)
			blocks = atoi(argv[++i]);
		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-file") == 0)
			strcpy(file_name, argv[++i]);
	}
	

	int fd = open(file_name, O_CREATE | O_WRONLY);
	if (fd < 0)
	{
		printf("Error: creat big failed!\n");
		exit();
	}

	for(i = 0; i < blocks; i++) // prolazimo kroz blovoe i pisemo
	{
		if (write(fd, buf, 512) != 512)
		{
			printf("Error: write block %d failed\n", i);
			break;
		}
		else 
			printf("Writing block: %d\n", i);
	}

	close(fd);

	
	exit();
}
