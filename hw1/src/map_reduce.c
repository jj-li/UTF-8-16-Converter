//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "map_reduce.h"
#include <string.h>
#include <dirent.h>
//Implement map_reduce.h functions here.

int validateargs(int argc, char** argv){
	char statement[] = "Usage: \t./mapreduce [h|v] FUNC DIR\n\tFUNC\tWhich operation you would like to run on the data:\n\t\tana - Analysis of various text files in a directory.\n\t\tstats - Calculates stats on files which contain only numbers.\n\tDIR\tThe directory in which the files are located.\n\n\tOptions:\n\t-h\tPrints this help menu.\n\t-v\tPrints the map function's results, stating the file it's from.\n";
	//If no arguments are provided
	if (argc < 2) {
		printf("%s",statement);
		printf("\nLess than 2%d\n", -1);//To Delete
		return -1;
	}
	char hFlag[] = "-h";
	char vFlag[] = "-v";
	char ana[] = "ana";
	char stats[] = "stats";
	int i;
	//Check for -h first
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i],hFlag) == 0) {
			printf("%s",statement);
			printf("\nAsked for help%d\n", EXIT_SUCCESS);//To Delete
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
							printf("\nFound a -v DIR error%d\n", -1);//To Delete
							closedir(oDir);
							return -1;
						}
						else {
							printf("\nFound a -v%d\n", exitNumber);//To Delete
							closedir(oDir);
							return exitNumber;
						}
					}
					else {
						printf("%s",statement);
						printf("\nFound a -v DIR error%d\n", -1);//To Delete
						return -1;
					}
					
				}
			}
			printf("%s",statement);
			printf("\nFound a -v ana/stats error%d\n", -1);//To Delete
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
					printf("\nFound a -v DIR error%d\n", -1);//To Delete
					closedir(oDir);
					return -1;
				}
				else {
					printf("\nFound ana/stats%d\n", exitNumber);//To Delete
					closedir(oDir);
					return exitNumber;
				}
			}
			else {
				printf("%s",statement);
				printf("\nFound ana/stats error%d\n", -1);//To Delete
				return -1;
			}
		}	
	}
	//Invalid arguments
	printf("%s",statement);
	printf("\nInvalid command%d\n", -1);//To Delete
	return -1;
}