#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	int i, key = -1;
	char charkey;

	for(i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printf("\nUse this program to set the current active key.\nAfter setting the key, you can use encr and decr with that key.\nUsage: setkey [OPTION]... [KEY]\n\nCommand line options:\n        -h, --help: Show help prompt.\n        -s, --secret: Enter the key via STDIN. Hide key when entering it\n");
			exit();		
		}
		else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--secret") == 0) 
		{
			key = 0;
			printf("Enter the key: ");
			setecho(0);
			while(read(0, &charkey, 1) > 0) { // posto je zvezdica citamo sa read
				if (charkey > 47 && charkey < 58) // ako je cifra pravimo broj
					key = key*10 + charkey-48;
				else break;
			}
			setkey(key);
			setecho(1);
			exit();	
		}
		else key = atoi(argv[i]); // ako nije sekret samo odradimo ascii to int
	}

	setkey(key);
	exit();
}
