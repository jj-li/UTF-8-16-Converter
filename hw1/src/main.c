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
    //validateargs(argc,argv);
    memset(analysis_space, 0, sizeof(analysis_space));
    struct Analysis ana1 = {0};
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
    }
    
}
