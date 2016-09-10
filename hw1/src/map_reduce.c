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
			if (i == 1) {
				printf("%s",statement);
				return EXIT_SUCCESS;
			}
			else {
				printf("%s",statement);
				return -1;
			}
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
	char c = dir[strlen(dir)-1];
	char newDirPath[strlen(dir)+2];
	if (c != '/') {
		strcpy(newDirPath, dir);
		strcat(newDirPath, "/\0");
		dir = newDirPath;
	}
	DIR *oDir = opendir(dir);
	if (oDir == NULL) {
		closedir(oDir);
		return -1;
	}
	struct dirent *temp = readdir(oDir);
	int sum = 0;
	while (temp != NULL) {
		if (strcmp(temp->d_name, ".\0") != 0 && strcmp(temp->d_name, "..\0") != 0 ){
			char fullPath[strlen(dir)+strlen(temp->d_name)+1];
			strcpy(fullPath, dir);
			strcat(fullPath, temp->d_name);
			strcat(fullPath, "\0");
			FILE *oF = fopen(fullPath, "r");
			if (oF == NULL) {
				return -1;
			}
			memset(results, 0, size);
			int returnedNumber = act(oF, results, temp->d_name);
			if (returnedNumber == -1) {
				return -1;
			}
			fclose(oF);
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
		if (ana.lnlen < anas[i].lnlen) {
			ana.lnno = anas[i].lnno;
			ana.filename = anas[i].filename;
			ana.lnlen = anas[i].lnlen;
			ana.lnno = anas[i].lnno;
			int j;
			for (j = 0; j < 128; j++) {
				ana.ascii[j] = ana.ascii[j] + anas[i].ascii[j];
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

void stats_print(Stats res, int hist) {
	int numElements = sizeof(res.histogram)/sizeof(res.histogram[0]);;
	int totalCount = 0;
	int i;
	if (hist != 0) {
		printf("Histogram:\n");
		int j;
		for (i = 0; i < numElements; i++) {
			if (res.histogram[i] > 0) {
				printf("%d :", i);
				for (j = 0; j < res.histogram[i]; j++) {
					printf("-");
				}
				totalCount = totalCount + res.histogram[i];
				printf("\n");
			}
		}
		printf("\n");
	}
	else {
		printf("File: %s\n", res.filename);
		for (i = 0; i < numElements; i++) {
			totalCount = totalCount + res.histogram[i];
		}
	}
	int numbers[totalCount];
	int j = 0;
	for (i = 0; i < numElements; i++) {
		int temp = res.histogram[i];
		while (temp > 0) {
			numbers[j] = i;
			temp--;
			j++;
		}

	}
	printf("Count: %d\n", res.n);
	double count = totalCount;
	printf("Mean: %.6f\n", res.sum/count);
	int min = 0;
	int max = numElements-1;
	int maxOccurrence = 0;
	int modes[numElements];
	double median;
	double q1;
	double q3;
	//Finding min and the maximum occurence of a number
	for (i = 0; i < numElements; i++) {
		if (res.histogram[i] > 0) {
			if (res.histogram[i] > maxOccurrence) {
				maxOccurrence = res.histogram[i];
			}
			if (res.histogram[min] == 0) {
				min = i;
			}
		}
		modes[i] = 0;
	}
	//Finding max and mode(s)
	int numModes = 0;
	for (i = numElements-1; i >= 0; i--) {
		if (res.histogram[i] > 0) {
			if (res.histogram[max] == 0) {
				max = i;
			}
			if (res.histogram[i] >= maxOccurrence) {
				modes[i] = 1;
				numModes++;
			}
		}
	}
	//Finding median
	int medianPosition;
	int q1TotalCount;
	int q3TotalCount;
	if (totalCount % 2 != 0) {
		medianPosition = totalCount/2 + 1;
		q1TotalCount = (totalCount-1)/2;
		q3TotalCount = (totalCount-1)/2;
		median = numbers[medianPosition-1];
	}
	else {
		medianPosition = totalCount/2;
		q1TotalCount = medianPosition;
		q3TotalCount = medianPosition;
		median = (numbers[medianPosition-1] + numbers[(medianPosition)]) / 2.0;
	}
	//Finding Q1
	int q1Numbers[q1TotalCount];
	j = 0;
	for (i = 0; i < q1TotalCount; i++) {
		q1Numbers[i] = numbers[i];

	}
	if (q1TotalCount % 2 != 0) {
		q1 = q1Numbers[(q1TotalCount/2 + 1)];
	}
	else {
		q1 = (q1Numbers[(q1TotalCount/2)] + q1Numbers[(q1TotalCount/2-1)]) / 2.0;
	}
	//Finding Q3
	int q3Numbers[q3TotalCount];
	j = totalCount-1;
	for (i = 0; i < q3TotalCount; i++) {
		q3Numbers[i] = numbers[j];
		j = j - 1;
	}
	if (q3TotalCount % 2 != 0) {
		q3 = q3Numbers[(q3TotalCount/2 + 1)];
	}
	else {
		q3 = (q3Numbers[(q3TotalCount/2)] + q3Numbers[(q3TotalCount/2-1)]) / 2.0;
	}
	printf("Mode: ");
	for (i = 0; i < numElements; i++) {
		if (modes[i] > 0) {
			printf("%d", i);
			numModes--;
			if (numModes > 0) {
				printf(" ");
			}
			else {
				printf("\n");
				break;
			}
		}
	}
	printf("Median: %0.6f\n", median);
	printf("Q1: %0.6f\n", q1);
	printf("Q3: %0.6f\n", q3);
	printf("Min: %d\n", min);
	printf("Max: %d\n", max);
}

int analysis(FILE* f, void* res, char* filename) {
	struct Analysis ana = {0};
	ana.filename = filename;
	char c;
	int lnlen = 0;
	ana.lnlen = 0;
	ana.lnno = 0;
	int lnno = 0;
	int bytesRead = 0;
	while((c = fgetc(f)) != EOF) {
        ana.ascii[c] = ana.ascii[c] + 1;
        if (c == 10) {
        	lnno = lnno + 1;
        	if (lnlen > ana.lnlen) {
        		ana.lnlen = lnlen;
        		ana.lnno = lnno;
        	}
        	lnlen = 0;
        }
        else {
        	lnlen = lnlen + 1;
        }
        bytesRead = bytesRead + 1;
    }
    memcpy(res, &ana, sizeof(ana));
    return bytesRead;
}

int stats(FILE* f, void* res, char* filename) {
	Stats sta = {0};
	sta.sum = 0;
	sta.n = 0;
	sta.filename = filename;
	int currentNum = -1;
	int moreDigits = 0; //0 False, 1 True
	char c;
	while((c = fgetc(f)) != EOF) {
		if (c == '-') {
			if ((c = fgetc(f)) != EOF) {
				if (c >= '0' && c<= '9') {
					return -1;
				}
			}
		}
        if (c >= '0' && c<= '9') {
        	if (moreDigits == 0) {
        		currentNum = c - '0';
        	}
        	else {
        		currentNum = currentNum * 10;
        		currentNum = currentNum + (c - '0');
        	}
        	moreDigits = 1;
        }
        else {
        	if (c != ' ' && c != '\n') {
        		return -1;
        	} 
        	moreDigits = 0;
        	if (currentNum > (NVAL-1)) {
        		return -1;
        	}
        	if (currentNum >= 0) {
        		sta.histogram[currentNum] = sta.histogram[currentNum] + 1;
        		sta.n = sta.n + 1;
        		sta.sum = sta.sum + currentNum;
        		currentNum = -1;
        	}
        }
    }
    memcpy(res, &sta, sizeof(sta));
    return 0;
}