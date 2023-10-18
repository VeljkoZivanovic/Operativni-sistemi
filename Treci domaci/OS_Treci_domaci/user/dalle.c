#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

int
main(int argc, char *argv[])
{
	printf("Commands: \n");
	printf("0 (>_<)=> latest \n");
	printf("1 (>_<)=> global extrema \n");
	printf("2 (>_<)=> pause \n");
	printf("3 (>_<)=> resume \n");
	printf("4 (>_<)=> end \n");

	char* najduza_rec = (char*) malloc(100);//Najduža reč u trenutnoj rečenici
	char* najkraca_rec = (char*) malloc(100);//Najkraca reč u trenutnoj rečenici
	char* najduza_rec_u_celom_tekstu = (char*) malloc(100);//Najduža reč u čitavom tekstu do trenutne rečenice (sa njom)
	char* najkraca_rec_u_celom_tekstu = (char*) malloc(100);//Najkraca reč u čitavom tekstu do trenutne rečenice (sa njom)

	char* putanja = (char*) malloc(100);//putanja do fajla koji obradjujemo
	strcpy(putanja, "../home/README");
	if (argc > 1) strcpy(putanja, argv[1]);

	int com, cnt = 0, max = 0, min = 100; //cnt-brojac, max-velicina najduze reci, min-velicina najkrace reci, com-indikator komande
	 

	int x;
	if ((x = share_data("com", &com, sizeof(int))) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for command\n");
		else if (x == -2) 
			printf("dalle: data with name com already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}
	if ((x = share_data("name", putanja, 100)) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for name\n");
		else if (x == -2) 
			printf("dalle: data with name name already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}

	if ((x = share_data("bigword", najduza_rec, 100)) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for the biggest word\n");
		else if (x == -2) 
			printf("dalle: data with name bigword already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}
	if ((x = share_data("smallword", najkraca_rec, 100)) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for the smallest word\n");
		else if (x == -2) 
			printf("dalle: data with name smallword already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}
	if ((x = share_data("max", &max, sizeof(int))) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for biggest word size\n");
		else if (x == -2) 
			printf("dalle: data with name max already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}	
	if ((x = share_data("min", &min, sizeof(int))) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for smallest word size\n");
		else if (x == -2) 
			printf("dalle: data with name min already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}

	if ((x = share_data("lastbig", najduza_rec_u_celom_tekstu, 100)) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for the biggest word in last sentence\n");
		else if (x == -2) 
			printf("dalle: data with name lastbig already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}
	if ((x = share_data("lastsmall", najkraca_rec_u_celom_tekstu, 100)) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for the smallest word in last senetnce\n");
		else if (x == -2) 
			printf("dalle: data with name lastsmall already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}
	if ((x = share_data("cnt", &cnt, sizeof(int))) < 0) {
		if (x == -1) 
			printf("dalle: can't read parametars for sentence counter\n");
		else if (x == -2) 
			printf("dalle: data with name cnt already exists\n");
		else if (x == -3)
			printf("dalle: data already has 10 elements\n");
		exit();
	}	


	
	int pid1, pid2;
	char* argv1[] = {"/bin/coMMa", 0};
	char* argv2[] = {"/bin/liSa", 0};

	pid1 = fork();
	if (pid1 == 0) exec("/bin/coMMa", argv1);
	else {
		pid2 = fork();
		if (pid2 == 0) exec("/bin/liSa", argv2);
		else {
			wait();
			wait();
		}
	}

	exit();
}
