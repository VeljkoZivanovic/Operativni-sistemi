#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
int pocetak_reci(char c) {
	if (c != '\n' && c != ' ' && c != '.' && c != ',') return 1;
	return 0;
}

int kraj_reci(char c) {
	if (c == '\n' || c == ' ' || c == '.' || c == ',') return 1;
	return 0;
}

void kopija(char* stara, char* nova, int* staraV, int novaV) {
	*staraV = novaV;
	strncpy(stara, nova, novaV);
	stara[novaV] = 0;
}

int
main(int argc, char *argv[])
{
	int *cnt, *max, *min, *com, fd;
	
	char* najduza_rec;
	char *najkraca_rec;
	char *najduza_rec_u_celom_tekstu;
	char *najkraca_rec_u_celom_tekstu;
	char *ime;
	char *ime_kopija;

	if (get_data("com", &com) < 0) { 
		printf("liSa: command not found\n"); 
		exit();
	}

	if (get_data("name", &ime) < 0) { 
		printf("liSa: name not found\n"); 
		exit();
	}

	if (get_data("cnt", &cnt) < 0) { 
		printf("liSa: counter not found\n"); 
		exit();
	}
	
	if (get_data("bigword", &najduza_rec) < 0) { 
		printf("liSa: global biggest word not found\n"); 
		exit();
	}

	if (get_data("smallword", &najkraca_rec) < 0) { 
		printf("liSa: global smallest word not found\n"); 
		exit();
	}
	
	if (get_data("max", &max) < 0) { 
		printf("liSa: biggest word length not found\n"); 
		exit();
	}
	
	if (get_data("min", &min) < 0) { 
		printf("liSa: smallest word length not found\n"); 
		exit();
	}	

	if (get_data("lastbig", &najduza_rec_u_celom_tekstu) < 0) { 
		printf("liSa: biggest word in last sentence not found\n");
		exit();
	}

	if (get_data("lastsmall", &najkraca_rec_u_celom_tekstu) < 0) { 
		printf("liSa: smallest word in last sentence not found\n"); 
		exit();
	}	

	strcpy(ime_kopija, ime);
	if((fd = open(ime_kopija, 0)) < 0){
		printf("liSa: cannot read file %s\n", ime);
		exit();
	}
	int n, i, pocetak, kraj, maksimum = 0, minimum = 1000, velicina = 0;
	char buffer[512];
	while((n = read(fd, buffer, sizeof(buffer))) > 0) {
		pocetak = 0, kraj = 0;
		for (i = 0; i < n; i++) {
			if (pocetak_reci(buffer[i]) && kraj_reci(buffer[i-1])) pocetak = i;
			if (kraj_reci(buffer[i]) && pocetak_reci(buffer[i-1])) { 
				kraj = i-1;
				int velicina = kraj - pocetak + 1;
				if (velicina > 0 && velicina > maksimum) kopija(najduza_rec_u_celom_tekstu, buffer+pocetak, &maksimum, velicina);
				if (velicina > 0 && velicina < minimum) kopija(najkraca_rec_u_celom_tekstu, buffer+pocetak, &minimum, velicina);
			}

			if (buffer[i] == '.') {
				if (maksimum > *max) kopija(najduza_rec, najduza_rec_u_celom_tekstu, max, strlen(najduza_rec_u_celom_tekstu));
				if (minimum < *min) kopija(najkraca_rec, najkraca_rec_u_celom_tekstu, min, strlen(najkraca_rec_u_celom_tekstu));
				(*cnt)++;
				sleep(150);

				while(1) { 
					if (*com != 1) break; 
					else sleep(1);
				}

				if (*com == 2) {
					printf("liSa: exit\n");
					exit();
				}

				maksimum = 0;
				minimum = 100;
			}
		}
	}

	exit();
}

