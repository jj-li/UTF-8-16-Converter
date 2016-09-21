#include "utfconverter.h"

char* filename;
char* outputName;
endianness source;
endianness conversion;
int sparky;
int verbosity;
int totalGlyphs;
int totalSurrogates;
int totalAsciis;
int filenamePos;
int noMoreCheckingNextFlag;
int vCounter;
int oldOptind;
/*Variables for the time counting*/
int tps;
double readRealTime;
double readUserTime;
double readSysTime;
clock_t readRealStart;
clock_t readRealEnd;
struct tms* readCpuStart;
struct tms* readCpuEnd;
double writeRealTime;
double writeUserTime;
double writeSysTime;
clock_t writeRealStart;
clock_t writeRealEnd;
struct tms* writeCpuStart;
struct tms* writeCpuEnd;
double convertRealTime;
double convertUserTime;
double convertSysTime;
clock_t convertRealStart;
clock_t convertRealEnd;
struct tms* convertCpuStart;
struct tms* convertCpuEnd;
int main(int argc, char** argv)
{
	/*
		Potential ERRORS?
		-v everywhere still gotta work
	*/
	int fd;
	int rv;
	Glyph* glyph;
	unsigned int buf[2];
	char* hostname;
	struct utsname* systemName; 
	struct stat* fileData;
	char* filePath;
	/*scp -r -P 24 ../hw2 jijli@sparky.ic.stonybrook.edu:
	*/
	/*After calling parse_args(), filename and conversion should be set. */
	verbosity = 0;
	filename = malloc(1);
	outputName = malloc(1);
	filenamePos = 0;
	noMoreCheckingNextFlag = 0;
	vCounter = -1;
	oldOptind = 1;
	parse_args(argc, argv);

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("File does not exist.\n");
		return EXIT_FAILURE;
	}
	rv = 0; 
	memset(buf, 0, sizeof(buf));
	glyph = malloc(sizeof(Glyph)+1); 
	sparky = 0;
	totalGlyphs = 0;
	totalSurrogates = 0;
	totalAsciis = 0;
	readRealTime = 0;
	readUserTime = 0;
	readSysTime = 0;
	readCpuStart = malloc(sizeof(struct tms)+1);
	readCpuEnd = malloc(sizeof(struct tms)+1);
	writeRealTime = 0;
	writeUserTime = 0;
	writeSysTime = 0;
	writeCpuStart = malloc(sizeof(struct tms)+1);
	writeCpuEnd = malloc(sizeof(struct tms)+1);
	convertRealTime = 0;
	convertUserTime = 0;
	convertSysTime = 0;
	convertCpuStart = malloc(sizeof(struct tms)+1);
	convertCpuEnd = malloc(sizeof(struct tms)+1);
	tps = sysconf(_SC_CLK_TCK); 
	/*Handle BOM bytes for UTF16 specially. 
    Read our values into the first and second elements.*/
	readRealStart = times(readCpuStart);
	if((rv = read(fd, &buf[0], 1)) == 1 && 
			(rv = read(fd, &buf[1], 1)) == 1){
		readRealEnd = times(readCpuEnd);
		readRealTime = (double)(readRealEnd - readRealStart);
		readUserTime = ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
		readSysTime = ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)(tps));
		void* memset_return; 

		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is little endian*/
			source = LITTLE; 
		} else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is big endian*/
			source = BIG;
		} else if (buf[0] == 0xff000000 && buf[1] == 0xfe000000){
			source = LITTLE;
			sparky = 1;
		} else if (buf[0] == 0xfe000000 && buf[1] == 0xff000000){
			source = BIG;
			sparky = 1;
		} else {
			/*file has no BOM*/
			free(glyph); 
			free(readCpuStart);
			free(readCpuEnd);
			free(writeCpuStart);
			free(writeCpuEnd);
			free(convertCpuStart);
			free(convertCpuEnd);
			fprintf(stderr, "File has no BOM.\n");
			quit_converter(NO_FD); 
		}
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
		/* Memory write failed, recover from it: */
		if(memset_return == NULL){
			/* tweak write permission on heap memory. */
			/*asm("movl $8, %esi\n\t"
			    "movl $.LC0, %edi\n\t"
			    "movl $0, %eax");*/
			/* Now make the request again. */
			free(readCpuStart);
			free(readCpuEnd);
			free(writeCpuStart);
			free(writeCpuEnd);
			free(convertCpuStart);
			free(convertCpuEnd);
			return EXIT_FAILURE;
			memset(glyph, 0, sizeof(Glyph)+1);
		}
		if (conversion == LITTLE) {
			glyph->surrogate = false;
			glyph->bytes[0] = 0xff;
			glyph->bytes[1] = 0xfe;
			write_glyph(glyph);
		}
		else {
			glyph->surrogate = false;
			glyph->bytes[0] = 0xfe;
			glyph->bytes[1] = 0xff;
			write_glyph(glyph);
		}
	}

	/* Now deal with the rest of the bytes.*/
	readRealStart = times(readCpuStart);
	while((rv = read(fd, &buf[0], 1)) == 1 &&  
			(rv = read(fd, &buf[1], 1)) == 1){
		readRealEnd = times(readCpuEnd);
		readRealTime = readRealTime + (double)(readRealEnd - readRealStart);
		readUserTime = readUserTime + ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)tps);
		
		void* memset_return;

		write_glyph(fill_glyph(glyph, buf, source, &fd));
		totalGlyphs = totalGlyphs + 1;
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
	        /* Memory write failed, recover from it: */
	        if(memset_return == NULL){
		        /* tweak write permission on heap memory. */
		        /*asm("movl $8, %esi\n\t"
		            "movl $.LC0, %edi\n\t"
		            "movl $0, %eax");*/
		        /* Now make the request again. */
		        free(readCpuStart);
				free(readCpuEnd);
				free(writeCpuStart);
				free(writeCpuEnd);
				free(convertCpuStart);
				free(convertCpuEnd);
		        return EXIT_FAILURE;
		        memset(glyph, 0, sizeof(Glyph)+1);
	        }
	    readRealStart = times(readCpuStart);
	}
	readRealEnd = times(readCpuEnd);
	readRealTime = readRealTime + (double)(readRealEnd - readRealStart);
	readUserTime = readUserTime + ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
	readSysTime = readSysTime + ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)tps);
	free(readCpuEnd);
	free(readCpuStart);
	free(writeCpuStart);
	free(writeCpuEnd);
	free(convertCpuStart);
	free(convertCpuEnd);
	if (verbosity >= 1) {
		fileData = malloc(sizeof(struct stat)+1);
		if (fileData == NULL){
			printf("Memory error.\n");
			quit_converter(fd);
		}
		if (stat(filename, fileData) == 0) {
			printf("Input file size: %ld kb\n", fileData->st_size);
			free(fileData);
		}
		else {
			free(fileData);
			/*some error*/
		}
		filePath = realpath(filename, NULL);
		if (filePath != NULL) {
			printf("Input file path: %s\n", filePath);
			free(filePath);
		}
		else {
			free(filePath);
			printf("Memory error.\n");
			quit_converter(fd);
		}
		if (source == BIG) {
			printf("Input file encoding: UTF-16BE\n");
		}
		else {
			printf("Input file encoding: UTF-16LE\n");
		}
		if (conversion == BIG) {
			printf("Output encoding: UTF-16BE\n");
		}
		else {
			printf("Output encoding: UTF-16LE\n");
		}
		hostname = malloc(sizeof(char)+50);
		if (hostname == NULL){
			printf("Memory error.\n");
			quit_converter(fd);
		}
		if (gethostname(hostname, sizeof(hostname)+50) == 0) {
			printf("Hostmachine: %s\n", hostname);
			free(hostname);
		}
		else {
			free(hostname);
			/*some error*/
		}
		systemName = malloc(sizeof(struct utsname)+1);
		if (systemName == NULL){
			printf("Memory error.\n");
			quit_converter(fd);
		}
		if (uname(systemName) == 0) {
			printf("Operating System: %s\n", systemName->sysname);
			free(systemName);
		} 
		else {
			free(systemName);
			/*some error*/
		}
		if (verbosity == 2) {
			int surrogatePercentage;
			int asciiPercentage;

			surrogatePercentage = (totalSurrogates*1.0)/(totalGlyphs*1.0) * 1000;
			asciiPercentage = (totalAsciis*1.0)/(totalGlyphs*1.0) * 1000;
			printf("Reading: real=%.1f, user=%.1f, sys=%.1f\n", readRealTime, readUserTime, readSysTime);
			printf("Converting: real=%.1f, user=%.1f, sys=%.1f\n", convertRealTime, convertUserTime, convertSysTime);
			printf("Writing: real=%.1f, user=%.1f, sys=%.1f\n", writeRealTime, writeUserTime, writeSysTime);
			if (asciiPercentage % 10 >= 5) {
				printf("ASCII: %d%%\n", (asciiPercentage/10 + 1));
			}
			else {
				printf("ASCII: %d%%\n", (asciiPercentage/10));

			}
			if (surrogatePercentage % 10 >= 5) {
				printf("Surrogates: %d%%\n", (int)(surrogatePercentage/10 + 1));
			}
			else {
				printf("Surrogates: %d%%\n", (int)(surrogatePercentage/10));

			}
			printf("Glyphs: %d\n", totalGlyphs);
		}
	}

	free(glyph);
	quit_converter(NO_FD);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph)
{
	convertRealStart = times(convertCpuStart);
	/* Use XOR to be more efficient with how we swap values. */
	unsigned int temp = glyph->bytes[0] ^ glyph->bytes[1];
	glyph->bytes[0] = (glyph->bytes[0] & 0) ^ glyph->bytes[1];
	glyph->bytes[1] = glyph->bytes[1] ^ temp;
	/*if(glyph->surrogate){  //If a surrogate pair, swap the next two bytes.
		temp = glyph->bytes[2] ^ glyph->bytes[3];
		glyph->bytes[2] = (glyph->bytes[2] & 0) ^ glyph->bytes[3];
		glyph->bytes[3] = glyph->bytes[3] ^ glyph->bytes[2];
	}*/
	glyph->end = conversion;

	convertRealEnd = times(convertCpuEnd);
	convertRealTime = convertRealTime + (double)(convertRealEnd - convertRealStart);
	convertUserTime = convertUserTime + ((double)(convertCpuEnd->tms_utime - convertCpuStart->tms_utime) / (double)tps);
	convertSysTime = convertSysTime + ((double)(convertCpuEnd->tms_stime - convertCpuStart->tms_stime) / (double)tps);

	return glyph;
}

Glyph* fill_glyph(Glyph* glyph, unsigned int data[2], endianness end, int* fd)
{
	unsigned int bits;

	if (sparky == 1) {
		data[0] = data[0] >> 24;
		data[1] = data[1] >> 24;
	}
	glyph->bytes[0] = data[0];
	glyph->bytes[1] = data[1];

	/* bits presents the codepoint */
	bits = 0; 
	if (end == BIG) {
		bits = bits | ((data[0] << 8) + data[1]);
	}
	else {
		bits = bits | (data[0] + (data[1] << 8));
	}
	if (bits <= 0x007F) {
		totalAsciis = totalAsciis + 1;
	}
	/* Check high surrogate pair using its special value range.*/
	if(bits >= 0xD800 && bits <= 0xDBFF){ 
		readRealStart = times(readCpuStart);
		if(read(*fd, &(data[0]), 1) == 1 && 
			read(*fd, &(data[1]), 1) == 1){
			readRealEnd = times(readCpuEnd);
			readRealTime = readRealTime + (double)(readRealEnd - readRealStart);
			readUserTime = readUserTime + ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
			readSysTime = readSysTime + ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)tps);
			/* Now remake the bit using the second set of bytes */
			bits = 0;
			if (end == BIG) {
				bits = bits | ((data[0] << 8) + data[1]);
			}
			else {
				bits = bits | (data[0] + (data[1] << 8));
			} 
			if(bits >= 0xDC00 && bits <= 0xDFFF){ /*Check low surrogate pair.*/ 
 				glyph->surrogate = true;
 			} else {
 				lseek(*fd, -2, SEEK_CUR);
 				glyph->surrogate = false; 
 			}
 		} else {
 			readRealEnd = times(readCpuEnd);
			readRealTime = readRealTime + (double)(readRealEnd - readRealStart);
			readUserTime = readUserTime + ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
			readSysTime = readSysTime + ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)tps);
 		}
 	}
 	else {
 		glyph->surrogate = false;
 	}

	if(!glyph->surrogate){
		glyph->bytes[2] = 0;
		glyph->bytes[3] = 0;
	} else {
		glyph->bytes[2] = data[0];
		glyph->bytes[3] = data[1];
	}
	glyph->end = end;
	if (conversion != end) {
		return swap_endianness(glyph);
	}
	return glyph;
}

void write_glyph(Glyph* glyph)
{
	FILE* wd;
	wd = fopen(outputName, "a");
	if (wd != NULL) {
		writeRealStart = times(writeCpuStart);
		if(glyph->surrogate){
			fwrite(glyph->bytes, sizeof(glyph->bytes[0]), SURROGATE_SIZE, wd);
			totalSurrogates = totalSurrogates + 1;
		} else {
			fwrite(glyph->bytes, sizeof(glyph->bytes[0]), NON_SURROGATE_SIZE, wd);		}
			writeRealEnd = times(writeCpuEnd);
			writeRealTime = writeRealTime + (double)(writeRealEnd - writeRealStart);
			writeUserTime = writeUserTime + ((double)(writeCpuEnd->tms_utime - writeCpuStart->tms_utime) / (double)tps);
			writeSysTime = writeSysTime + ((double)(writeCpuEnd->tms_stime - writeCpuStart->tms_stime) / (double)tps);
			fclose(wd);
	}
	else {
		writeRealStart = times(writeCpuStart);
		if(glyph->surrogate){
			write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
			totalSurrogates = totalSurrogates + 1;
		} else {
			/*unsigned int bits = 0; 
			if (conversion == BIG) {
				bits = bits | (glyph->bytes[0] + (glyph->bytes[1] << 8));
			}
			else {
				bits = bits | ((glyph->bytes[0] << 8) + glyph->bytes[1]);
			}
			*/
			write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
		}
		writeRealEnd = times(writeCpuEnd);
		writeRealTime = writeRealTime + (double)(writeRealEnd - writeRealStart);
		writeUserTime = writeUserTime + ((double)(writeCpuEnd->tms_utime - writeCpuStart->tms_utime) / (double)tps);
		writeSysTime = writeSysTime + ((double)(writeCpuEnd->tms_stime - writeCpuStart->tms_stime) / (double)tps);
	}	
}

void parse_args(int argc, char** argv)
{
	int option_index, c;
	char* endian_convert;
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"h", no_argument, 0, 'h'},
		{"UTF", required_argument, 0, 'z'},
		{0, 0, 0, 0}
	};
	endian_convert = NULL;

	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	while((c = getopt_long(argc, argv, "vhu:", long_options, &option_index)) != -1){
		switch(c){ 
			case 'h':
				print_help();
				break;
			case 'u':
				if ((strcmp(optarg, "16LE") != 0) && (strcmp(optarg, "16BE") != 0)) {
					fprintf(stderr, "Invalid conversion mode.\n");
					print_help();
				}
				endian_convert = optarg;
				if(optind < argc){
					free(filename);
					filenamePos = optind;
					filename = strdup(argv[optind]);
				} else {
					fprintf(stderr, "Filename not given.\n");
					print_help();
				}
				break;
			case 'z':
				if((strcmp(optarg, "") == 0)){ 
					fprintf(stderr, "Converson mode not given.\n");
					print_help();
				}
				endian_convert = optarg;
				if(optind > 1){
					free(filename);
					filenamePos = optind;
					filename = strdup(argv[optind]);
				} else {
					fprintf(stderr, "Filename not given.\n");
					print_help();
				}
				break;
			case 'v':
				if (verbosity < 2) {
					verbosity = verbosity + 1;
				}/*
				if (filenamePos > 0 && noMoreCheckingNextFlag == 0) {
					if (optind >= argc) {
						fprintf(stderr, "Filename not given.\n");
						print_help();
					}
					if (strcmp(argv[optind], "-v") != 0) {
						filenamePos = optind;
						if ((optind+1) < argc) {
							if (strcmp(argv[(optind+1)], "-v") == 0) {
								noMoreCheckingNextFlag = 1;
							}
						}
					}							
				}*/
				if (optind > filenamePos) {
					if (optind > oldOptind) {
						vCounter = vCounter + 1;
					}
				}
				break;
			default:
				fprintf(stderr, "Unrecognized argument.\n");
				quit_converter(NO_FD);
				break;
		}
		if (optind > oldOptind) {
			oldOptind = optind;
		}
	}
	/*filename = strdup(argv[filenamePos]);*/
	if ((argc-1) > (vCounter + filenamePos)) {
		free(outputName);
		outputName = strdup(argv[(argc-1)]);
	}
	if(endian_convert == NULL){
		fprintf(stderr, "Converson mode not given.\n");
		print_help();
	}

	if(strcmp(endian_convert, "16LE") == 0){ 
		conversion = LITTLE;
	} else if(strcmp(endian_convert, "16BE") == 0){
		conversion = BIG;
	} else {
		quit_converter(NO_FD);
	}
}

void print_help() {
	int i;
	int elements;
	elements = 12;
	for (i = 0; i < elements; i = i+1) {
		if (i < 11) {
			printf("%s\n", USAGE[i]);
		}
		else {
			printf("%s", USAGE[i]);
		}
	}
	quit_converter(NO_FD);
}

void quit_converter(int fd)
{
	free(filename);
	free(outputName);
	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	if(fd != NO_FD)
		close(fd);
	exit(0);
	/* Ensure that the file is included regardless of where we start compiling from. */
}
