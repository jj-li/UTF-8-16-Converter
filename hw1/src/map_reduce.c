//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "map_reduce.h"
#include <string.h>
#include <dirent.h>
//Implement map_reduce.h functions here.

int validateargs(int argc, char** argv){
	char statement[] = "Usage: \t./mapreduce [h|v] FUNC DIR\n\tFUNC\tWhich operation you would like to run on the data:\n\t\tana - Analysis of various text files in a directory.\n\t\tstats - Calculates stats on files which contain only numbers.\n\tDIR\tThe directory in which the files are located.\n\n\tOptions:\n\t-h\tPrints this help menu.\n\t-v\tPrints the map function's results, stating the file it's from.\n\0";
	//If no arguments are provided
	if (argc < 2) {
		printf("%s\n",statement);
		return -1;
	}
	char hFlag[] = "-h\0";
	char vFlag[] = "-v\0";
	char ana[] = "ana\0";
	char stats[] = "stats\0";
	int i;
	//Check for -h first
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i],hFlag) == 0) {
			printf("%s",statement);
			return EXIT_SUCCESS;
		}
	}
	int exitNumber;
	//Look for -v next
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i],vFlag) == 0) {
			i++;
			if (i < argc) {
				if (strcmp(argv[i],ana) == 0 || strcmp(argv[i],stats) == 0) {
					if (strcmp(argv[i],ana) == 0) {
						exitNumber = 3;
					}
					else {
						exitNumber = 4;
					}
					i++;
					if (i < argc) {
						//Check if directory is valid
						DIR *oDir = opendir(argv[i]);
						if (oDir == NULL) {
							printf("%s",statement);
							closedir(oDir);
							return -1;
						}
						else {
							closedir(oDir);
							return exitNumber;
						}
					}
					else {
						printf("%s",statement);
						return -1;
					}
					
				}
			}
			printf("%s",statement);
			return -1;
		}
	}
	//Finally check for just ana/stats
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i],ana) == 0 || strcmp(argv[i],stats) == 0) {
			if (strcmp(argv[i],ana) == 0) {
				exitNumber = 1;
			}
			else {
				exitNumber = 2;
			}
			i++;
			if (i < argc) {
				//Check if directory is valid
				DIR *oDir = opendir(argv[i]);
				if (oDir == NULL) {
					printf("%s",statement);
					closedir(oDir);
					return -1;
				}
				else {
					closedir(oDir);
					return exitNumber;
				}
			}
			else {
				printf("%s",statement);
				return -1;
			}
		}	
	}
	//Invalid arguments
	printf("%s",statement);
	return -1;
}

int nfiles(char* dir) {
	DIR *oDir = opendir(dir);
	if (oDir == NULL) {
		closedir(oDir);
		return -1;
	}
	int count = 0;
	struct dirent *temp = readdir(oDir);
	while (temp != NULL) {
		if (strcmp(temp->d_name, ".\0") != 0 && strcmp(temp->d_name, "..\0") != 0 ){
			count = count + 1;
		}
		temp = readdir(oDir);
	}
	if (count == 0) {
		printf("No files present in the directory.");
		closedir(oDir);
		return EXIT_SUCCESS;
	}
	closedir(oDir);
	return count;
}

int map(char* dir, void* results, size_t size, int (*act)(FILE* f, void* res, char* fn)) {
	DIR *oDir = opendir(dir);
	if (oDir == NULL) {
		closedir(oDir);
		return -1;
	}
	//int elementSize = size/NFILES;
	struct dirent *temp = readdir(oDir);
	int sum = 0;
	while (temp != NULL) {
		if (strcmp(temp->d_name, ".\0") != 0 && strcmp(temp->d_name, "..\0") != 0 ){
			char fullPath[strlen(dir)+strlen(temp->d_name)+1];
			strcpy(fullPath, dir);
			strcat(fullPath, temp->d_name);
			FILE *oF = fopen(fullPath, "r");
			memset(results, 0, size);
			int returnedNumber = act(oF, results, temp->d_name);
			if (returnedNumber == -1) {
				return -1;
			}
			fclose(oF);
			//char* tempResults = (char*)results;
			//tempResults = tempResults + elementSize;
			//results = (void*) tempResults;
			results = results + size;
			sum = sum + returnedNumber;
		}
		temp = readdir(oDir);
	}
	closedir(oDir);
	return sum;
}

struct Analysis analysis_reduce(int n, void* results) {
	struct Analysis ana = {0};
	if (n == 0) {
		return ana;
	}
	struct Analysis* anas = (struct Analysis*)results;
	ana.filename = anas[0].filename;
	ana.lnno = anas[0].lnno;
	ana.lnlen = anas[0].lnlen;
	int i;
	for (i = 0; i < 128; i++) {
		ana.ascii[i] = anas[0].ascii[i];
	}
	for (i = 1; i < n; i++) {
		if (ana.lnno < anas[i].lnno) {
			ana.lnno = anas[i].lnno;
			ana.filename = anas[i].filename;
			ana.lnlen = anas[i].lnlen;
			ana.lnno = anas[i].lnno;
			int j;
			for (j = 0; j < 128; j++) {
				ana.ascii[j] = anas[i].ascii[j];
			}
		}
	}
	
	return ana;
}

Stats stats_reduce(int n, void* results) {
	Stats sta = {0};
	if (n == 0) {
		return sta;
	}
	sta.filename = NULL;
	Stats* stas = (Stats*)results;
	sta.sum = stas[0].sum;
	sta.n = stas[0].n;
	int i;
	for (i = 0; i < NVAL; i++) {
		sta.histogram[i] = stas[0].histogram[i];
	}
	for (i = 1; i < n; i++) {
		sta.sum = sta.sum + stas[i].sum;
		sta.n = sta.n + stas[i].n;
		int j;
		for (j = 0; j < NVAL; j++){
			sta.histogram[j] = sta.histogram[j] + stas[i].histogram[j];
		}
		
	}
	
	return sta;
}

void analysis_print(struct Analysis res, int nbytes, int hist) {
	printf("File: %s\n", res.filename);
	printf("Longest line length: %d\n", res.lnlen);
	printf("Longest line number: %d\n", res.lnno);
	if (hist == 0) {
		printf("\n");
	}
	if (hist != 0) {
		printf("Total Bytes in directory: %d\n", nbytes);
		printf("Histogram:\n");
		int i;
		int j;
		for (i = 0; i < 128; i++) {
			if (res.ascii[i] > 0) {
				printf(" %d:", i);
				for (j = 0; j < res.ascii[i]; j++) {
					printf("-");
				}
				printf("\n");
			}
		}
	}
}