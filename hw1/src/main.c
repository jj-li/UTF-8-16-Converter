#include "map_reduce.h"

//Space to store the results for analysis map
struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
Stats stats_space[NFILES];

//Sample Map function action: Print file contents to stdout and returns the number bytes in the file.
int cat(FILE* f, void* res, char* filename) {
    char c;
    int n = 0;
    printf("%s\n", filename);
    while((c = fgetc(f)) != EOF) {
        printf("%c", c);
        n++;
    }
    printf("\n");
    return n;
}

int main(int argc, char** argv) {
    int success = validateargs(argc,argv);
    if (success == -1){

    	return EXIT_FAILURE;
    }
    if (success == 0) {
    	return EXIT_SUCCESS;
    }
    int num;
   	char usage[] = "Usage: \t./mapreduce [h|v] FUNC DIR\n\tFUNC\tWhich operation you would like to run on the data:\n\t\tana - Analysis of various text files in a directory.\n\t\tstats - Calculates stats on files which contain only numbers.\n\tDIR\tThe directory in which the files are located.\n\n\tOptions:\n\t-h\tPrints this help menu.\n\t-v\tPrints the map function's results, stating the file it's from.\n\0";
    if (success == 1) {
    	num = map(argv[2], analysis_space, sizeof(struct Analysis), analysis);
    	if (num == -1) {
    		printf("%s\n", usage);
            return EXIT_FAILURE;
    	}
    	analysis_print(analysis_reduce(nfiles(argv[2]), analysis_space), num, 5);
    }
    if (success == 2){
    	num = map(argv[2], stats_space, sizeof(Stats), stats);
    	if (num == -1) {
    		printf("%s\n", usage);
            return EXIT_FAILURE;
    	}
    	stats_print(stats_reduce(nfiles(argv[2]), stats_space), 5);
    }
    if (success == 3) {
    	int numFiles = nfiles(argv[3]);
    	num = map(argv[3], analysis_space, sizeof(struct Analysis), analysis);
        if (num == -1) {
    		printf("%s\n", usage);
            return EXIT_FAILURE;
    	}
    	int i;
    	for (i = 0; i < numFiles; i++) {
    		analysis_print(analysis_space[i], num, 0);
    		printf("\n");
    	}
        struct Analysis finalAna = analysis_reduce(numFiles, analysis_space);
    	analysis_print(finalAna, num, 5);
    }
    if (success == 4) {
    	int numFiles = nfiles(argv[3]);
    	num = map(argv[3], stats_space, sizeof(Stats), stats);
    	if (num == -1) {
    		printf("%s\n", usage);
            return EXIT_FAILURE;
    	}
    	int i;
    	for (i = 0; i < numFiles; i++) {
    		stats_print(stats_space[i], 0);
    		printf("\n");
    	}
        Stats finalStat = stats_reduce(numFiles, stats_space);
    	stats_print(finalStat, 5);
    }
    return EXIT_SUCCESS;
}
