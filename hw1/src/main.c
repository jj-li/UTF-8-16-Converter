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
    //int sum = map("rsrc/ana_light/", stats_space, sizeof(Stats), cat);
    //printf("%d\n",sum);
    /*struct Analysis ana1 = {0};
    struct Analysis ana2 = {0};
    struct Analysis ana3 = {0};
    int i;
    for (i = 0; i < 128; i++) {
    	ana1.ascii[i] = i;
    	ana2.ascii[i] = 99;
    	ana3.ascii[i] = i;
    }
    ana1.filename = "One\0";
    ana1.lnlen = 10;
    ana1.lnno = 100;
    ana2.filename = "Two\0";
    ana2.lnlen = 20;
    ana2.lnno = 500;
    ana3.filename = "Three\0";
    ana3.lnlen = 30;
    ana3.lnno = 300;
    analysis_space[0] = ana1;
    analysis_space[1] = ana2;
    analysis_space[2] = ana3;
    struct Analysis final = analysis_reduce(3, analysis_space);
    printf("The file name is %s\n", final.filename);
    printf("The longest line length is %d\n", final.lnlen);
    printf("The longest line number is %d\n", final.lnno);
    for (i = 0; i < 128; i++) {
    	printf("The histogram data is %d at %d\n", final.ascii[i], i);
    }*/
    /*Stats stat1 = {0};
    Stats stat2 = {0};
    Stats stat3 = {0};
    int i;
    for (i = 0; i < NVAL; i++) {
    	stat1.histogram[i] = i;
    	stat2.histogram[i] = i;
    	stat3.histogram[i] = i;
    }
    stat1.filename = "One\0";
    stat1.sum = 10;
    stat1.n = 100;
    stat2.filename = "Two\0";
    stat2.sum = 20;
    stat2.n = 200;
    stat3.filename = "Three\0";
    stat3.sum = 30;
    stat3.n = 300;
    stats_space[0] = stat1;
    stats_space[1] = stat2;
    stats_space[2] = stat3;
    Stats final = stats_reduce(3, stats_space);
    printf("The file name is %s\n", final.filename);
    printf("The total sum is %d\n", final.sum);
    printf("The total n is %d\n", final.n);
    for (i = 0; i < NVAL; i++) {
    	printf("The histogram data is %d at %d\n", final.histogram[i], i);
    }*/
    /* TO KEEP
    int success = validateargs(argc,argv);
    if (success == -1){
    	return EXIT_FAILURE;
    }
    else if (success == 0) {
    	return EXIT_SUCCESS;
    }
    memset(analysis_space, 0, sizeof(analysis_space));
    memset(stats_space, 0, sizeof(stats_space));
    */
   	int numFiles = nfiles("rsrc/ana_light/");
    int nBytes = map("rsrc/ana_light/", analysis_space, sizeof(analysis_space[0]), analysis);
    int i;
    for (i = 0; i < numFiles; i++) {
    	printf("%p\n", &(analysis_space[i]));
    	if (i == (numFiles-1)) {
    		//analysis_print(analysis_reduce(numFiles, analysis_space), nBytes, 5);
    	}
    	else {
    		//analysis_print(analysis_space[i], nBytes, 0);
    	}
    }
}
