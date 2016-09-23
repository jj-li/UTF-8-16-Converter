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
int numBytes;
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
	parse_args(argc, argv);

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("File does not exist.\n");
		return EXIT_FAILURE;
	}
	numBytes = 0;
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
		void* memset_return; 
		readRealEnd = times(readCpuEnd);
		readRealTime = (double)(readRealEnd - readRealStart);
		readUserTime = ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
		readSysTime = ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)(tps));
		

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
		} else if (buf[0] == 0xef && buf[1] == 0xbb){
			if ((rv = read(fd, &buf[0], 1)) == 1) {
				if (buf[0] == 0xbf) {
					source = EIGHT;
				}
				else {
					free(glyph); 
					free(readCpuStart);
					free(readCpuEnd);
					free(writeCpuStart);
					free(writeCpuEnd);
					free(convertCpuStart);
					free(convertCpuEnd);
					quit_converter(fd);
				}
			}
			else {
				free(glyph); 
				free(readCpuStart);
				free(readCpuEnd);
				free(writeCpuStart);
				free(writeCpuEnd);
				free(convertCpuStart);
				free(convertCpuEnd);
				quit_converter(fd);
			}
			
		}
		else if (buf[0] == 0xef000000 && buf[1] == 0xbb000000){
			if ((rv = read(fd, &buf[0], 1)) == 1) {
				if (buf[0] == 0xbf000000) {
					source = EIGHT;
					sparky = 1;
				}
				else {
					free(glyph); 
					free(readCpuStart);
					free(readCpuEnd);
					free(writeCpuStart);
					free(writeCpuEnd);
					free(convertCpuStart);
					free(convertCpuEnd);
					quit_converter(fd);
				}
			}
			else {
				free(glyph); 
				free(readCpuStart);
				free(readCpuEnd);
				free(writeCpuStart);
				free(writeCpuEnd);
				free(convertCpuStart);
				free(convertCpuEnd);
				quit_converter(fd);
			}
			
		}
		else {
			/*file has no BOM*/
			free(glyph); 
			free(readCpuStart);
			free(readCpuEnd);
			free(writeCpuStart);
			free(writeCpuEnd);
			free(convertCpuStart);
			free(convertCpuEnd);
			fprintf(stderr, "File has no BOM.\n");
			quit_converter(fd); 
		}
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
		/* Memory write failed, recover from it: */
		if(memset_return == NULL){
			free(readCpuStart);
			free(readCpuEnd);
			free(writeCpuStart);
			free(writeCpuEnd);
			free(convertCpuStart);
			free(convertCpuEnd);
			quit_converter(fd);
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
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
		/* Memory write failed, recover from it: */
		if(memset_return == NULL){
			free(readCpuStart);
			free(readCpuEnd);
			free(writeCpuStart);
			free(writeCpuEnd);
			free(convertCpuStart);
			free(convertCpuEnd);
			quit_converter(fd);
		}
	}

	/* Now deal with the rest of the bytes.*/
	readRealStart = times(readCpuStart);
	while((rv = read(fd, &buf[0], 1)) == 1 &&  
			(rv = read(fd, &buf[1], 1)) == 1){
		void* memset_return;
		readRealEnd = times(readCpuEnd);
		readRealTime = readRealTime + (double)(readRealEnd - readRealStart);
		readUserTime = readUserTime + ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)tps);

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
		        quit_converter(fd);
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
	free(filename);
	free(outputName);
	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(fd);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph)
{
	unsigned int temp;
	convertRealStart = times(convertCpuStart);
	/* Use XOR to be more efficient with how we swap values. */
	temp = glyph->bytes[0] ^ glyph->bytes[1];
	glyph->bytes[0] = (glyph->bytes[0] & 0) ^ glyph->bytes[1];
	glyph->bytes[1] = glyph->bytes[1] ^ temp;
	if(glyph->surrogate){
		temp = glyph->bytes[2] ^ glyph->bytes[3];
		glyph->bytes[2] = (glyph->bytes[2] & 0) ^ glyph->bytes[3];
		glyph->bytes[3] = glyph->bytes[3] ^ temp;
	}
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
	readRealStart = times(readCpuStart);
	if (sparky == 1) {
			data[0] = data[0] >> 24;
			data[1] = data[1] >> 24;
	}
	/*UTF-8 ENCODING*/
	if (end == EIGHT) {
		if (data[0] <= 0x7F) {/*ONE BYTE*/
			lseek(*fd, -1, SEEK_CUR);
			bits = data[0];
			glyph->bytes[0] = data[0];
			numBytes = 1;

		} 
		else if (data[0] >= 0xC0 && data[0] <= 0xDF) {/*TWO BYTES*/
			data[0] = data[0] & 0x1F;
			data[1] = data[1] & 0x3F;
			glyph->bytes[0] = data[0];
			glyph->bytes[1] = data[1];
			bits = (data[0] << 6) + data[1];
			numBytes = 2;
		}
		else if (data[0] >= 0xE0 && data[0] <= 0xEF) {/*THREE BYTES*/
			data[0] = data[0] & 0xF;
			data[1] = data[1] & 0x3F;
			glyph->bytes[0] = data[0];
			glyph->bytes[1] = data[1];
			bits = (data[0] << 12) + (data[1] << 6);
			if(read(*fd, &(data[0]), 1) == 1){
				if (sparky == 1) {
					data[0] = data[0] >> 24;
				}
				data[0] = data[0] & 0x3F;
				bits = bits + data[0];
				glyph->bytes[2] = data[0];
				numBytes = 3;
			}
			else {
				/*Corrupted File*/
			}
		}
		else if (data[0] >= 0xF0 && data[0] <= 0xF7) {/*FOUR BYTES*/
			data[0] = data[0] & 0x7;
			data[1] = data[1] & 0x3F;
			glyph->bytes[0] = data[0];
			glyph->bytes[1] = data[1];
			bits = (data[0] << 18) + (data[1] << 12);
			if(read(*fd, &(data[0]), 1) == 1 && 
				read(*fd, &(data[1]), 1) == 1){
				if (sparky == 1) {
					data[0] = data[0] >> 24;
					data[1] = data[1] >> 24;
				}
				data[0] = data[0] & 0x3F;
				data[1] = data[1] & 0x3F;
				bits = bits + (data[0] << 6) + data[1];
				glyph->bytes[2] = data[0];
				glyph->bytes[3] = data[1];
				numBytes = 4;
			}
			else {
				/*Corrupted File*/
			}
		}
		else {/*ERROR UNKNOWN ENCODING!*/
			print_help();
			free(readCpuStart);
			free(readCpuEnd);
			free(writeCpuStart);
			free(writeCpuEnd);
			free(convertCpuStart);
			free(convertCpuEnd);
		    quit_converter(*fd);
		}
		if (bits > 0x1FFFFF) {
			/*Outside of code point range*/
			print_help();
			free(readCpuStart);
			free(readCpuEnd);
			free(writeCpuStart);
			free(writeCpuEnd);
			free(convertCpuStart);
			free(convertCpuEnd);
		    quit_converter(*fd);
		}
		glyph->end = end;
		readRealEnd = times(readCpuEnd);
		readRealTime = readRealTime + (double)(readRealEnd - readRealStart);
		readUserTime = readUserTime + ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)tps);
		return convert(glyph, conversion);
	}
	else { /*UTF-16 ENCODING*/
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
			if(read(*fd, &(data[0]), 1) == 1 && 
				read(*fd, &(data[1]), 1) == 1){
				/* Now remake the bit using the second set of bytes */
				bits = 0;
				if (sparky == 1) {
					data[0] = data[0] >> 24;
					data[1] = data[1] >> 24;
				}
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
		readRealEnd = times(readCpuEnd);
		readRealTime = readRealTime + (double)(readRealEnd - readRealStart);
		readUserTime = readUserTime + ((double)(readCpuEnd->tms_utime - readCpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(readCpuEnd->tms_stime - readCpuStart->tms_stime) / (double)tps);
		if (conversion != end) {
			return swap_endianness(glyph);
		}
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
				free(filename);
				free(outputName);
				close(STDERR_FILENO);
				close(STDIN_FILENO);
				close(STDOUT_FILENO);
				close(NO_FD);
				exit(0);
				break;
			case 'u':
				if ((strcmp(optarg, "16LE") != 0) && (strcmp(optarg, "16BE") != 0)) {
					fprintf(stderr, "Invalid conversion mode.\n");
					print_help();
					quit_converter(NO_FD);
				}
				endian_convert = optarg;
				break;
			case 'z':
				if((strcmp(optarg, "") == 0)){ 
					fprintf(stderr, "Converson mode not given.\n");
					print_help();
					quit_converter(NO_FD);
				}
				endian_convert = optarg;
				break;
			case 'v':
				if (verbosity < 2) {
					verbosity = verbosity + 1;
				}
				break;
			default:
				fprintf(stderr, "Unrecognized argument.\n");
				quit_converter(NO_FD);
				break;
		}
	}
	if (optind < argc) {
		free(filename);
		filename = strdup(argv[optind]);
	}
	else {
		printf("No filename given.");
		quit_converter(NO_FD);
	}
	if ((optind+1) < argc) {
		free(outputName);
		outputName = strdup(argv[(optind+1)]);
	}
	if(endian_convert == NULL){
		fprintf(stderr, "Converson mode not given.\n");
		print_help();
		quit_converter(NO_FD);
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
	exit(EXIT_FAILURE);
	/* Ensure that the file is included regardless of where we start compiling from. */
}

Glyph* convert(Glyph* glyph, endianness end) {
	unsigned int bits;
	unsigned int msb;
	unsigned int lsb;

	convertRealStart = times(convertCpuStart);
	bits = 0;

	if (numBytes == 4) {
		bits = (glyph->bytes[0] << 18) + (glyph->bytes[1] << 12) + (glyph->bytes[2] << 6) + glyph->bytes[3];
	}
	else if (numBytes == 2) {
		bits = (glyph->bytes[0] << 6) + glyph->bytes[1];
	}
	else if (numBytes == 3) {
		bits = (glyph->bytes[0] << 12) + (glyph->bytes[1] << 6) + glyph->bytes[2];
	}
	else {
		bits = glyph->bytes[0];
	}
	if (bits > 0x10000) {
		bits = bits - 0x10000;
		msb = (bits >> 10) + 0xD800;
		lsb = (bits & 0x3FF) + 0xDC00;
		glyph->surrogate = true;
		if (end == LITTLE) {
			glyph->bytes[0] = msb & 0xFF;
			glyph->bytes[1] = msb >> 8;
			glyph->bytes[2] = lsb & 0xFF;
			glyph->bytes[3] = lsb >> 8;
		}
		else {
			glyph->bytes[1] = msb & 0xFF;
			glyph->bytes[0] = msb >> 8;
			glyph->bytes[3] = lsb & 0xFF;
			glyph->bytes[2] = lsb >> 8;
		}
	}
	else {
		glyph->surrogate = false;
		if (end == LITTLE) {
			glyph->bytes[0] = bits & 0xFF;
			glyph->bytes[1] = bits >> 8;
		}
		else {
			glyph->bytes[1] = bits & 0xFF;
			glyph->bytes[0] = bits >> 8;
		}
	}
	glyph->end = end;
	convertRealEnd = times(convertCpuEnd);
	convertRealTime = convertRealTime + (double)(convertRealEnd - convertRealStart);
	convertUserTime = convertUserTime + ((double)(convertCpuEnd->tms_utime - convertCpuStart->tms_utime) / (double)tps);
	convertSysTime = convertSysTime + ((double)(convertCpuEnd->tms_stime - convertCpuStart->tms_stime) / (double)tps);

	return glyph;
}