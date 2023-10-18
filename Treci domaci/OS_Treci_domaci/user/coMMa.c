#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

int
main(int argc, char *argv[])
{
	char *najduza_rec;
	char *najkraca_rec; 
	char *najduza_rec_u_celom_tekstu; 
	char *najkraca_rec_u_celom_tekstu;
	char *komanda;
	
	int *cnt, *max, *min, *com; 
	if (get_data("com", &com) < 0) { 
		printf("coMMa: command not found\n"); 
		exit();
	}

	if (get_data("cnt", &cnt) < 0) { 
		printf("coMMa: counter not found\n"); 
		exit();
	}

	if (get_data("bigword", &najduza_rec) < 0) { 
		printf("coMMa: global biggest word not found\n"); 
		exit();
	}

	if (get_data("smallword", &najkraca_rec) < 0) { 
		printf("coMMa: global smallest word not found\n"); 
		exit();
	}
	
	if (get_data("max", &max) < 0) { 
		printf("coMMa: biggest word length not found\n"); 
		exit();
	}
	
	if (get_data("min", &min) < 0) { 
		printf("coMMa: smallest word length not found\n"); 
		exit();
	}	

	if (get_data("lastbig", &najduza_rec_u_celom_tekstu) < 0) { 
		printf("coMMa: biggest word in last sentence not found\n");
		exit();
	}

	if (get_data("lastsmall", &najkraca_rec_u_celom_tekstu) < 0) { 
		printf("coMMa: smallest word in last sentence not found\n"); 
		exit();
	}

	while (1) {
		gets(komanda, 20);
		komanda[strlen(komanda)-1] = 0;

		if (strcmp(komanda, "latest") == 0) {
			printf("Latest sentence: %d", *cnt);
			printf(" Local extrema ->");
			printf("longest : %s", najduza_rec_u_celom_tekstu);
			printf(" shortest: %s\n", najkraca_rec_u_celom_tekstu);
		}
		else if (strcmp(komanda, "global extrema") == 0)  {
			printf("Global extrema: \n");
			printf("Biggest word: %s\nBiggest size: %d\n", najduza_rec, *max);
			printf("Smallest word: %s\nSmallest size: %d\n", najkraca_rec, *min);
		}
		else if (strcmp(komanda, "pause") == 0) {
			*com = 1;
			printf("coMMa: pause\n");		
		}
		else if (strcmp(komanda, "resume") == 0) {
			*com = 0;
			printf("coMMa: resume\n");
		}
		else if (strcmp(komanda, "end") == 0) {
			*com = 2;
			printf("coMMa: exit\n");
			exit();	
		}
		else printf("coMMa: command %s does not exist\n", komanda);
	}

	exit();
}
