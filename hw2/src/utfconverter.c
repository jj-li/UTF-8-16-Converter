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
clock_t clockStart;
clock_t clockEnd;
struct tms* cpuStart;
struct tms* cpuEnd;
double writeRealTime;
double writeUserTime;
double writeSysTime;
double convertRealTime;
double convertUserTime;
double convertSysTime;
int main(int argc, char** argv)
{
	/*
		Potential ERRORS?
		
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
	filename = malloc(2);
	outputName = malloc(2);
	parse_args(argc, argv);
	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("File does not exist.\n");
		return EXIT_FAILURE;
	}
	numBytes = 0;
	rv = 0; 
	memset(buf, 0, sizeof(buf));
	clockStart = 0;
	clockEnd = 0;
	glyph = malloc(sizeof(Glyph)+1); 
	sparky = 0;
	totalGlyphs = 1;
	totalSurrogates = 0;
	totalAsciis = 0;
	cpuStart = malloc(sizeof(struct tms)+1);
	cpuEnd = malloc(sizeof(struct tms)+1);
	readRealTime = 0;
	readUserTime = 0;
	readSysTime = 0;
	writeRealTime = 0;
	writeUserTime = 0;
	writeSysTime = 0;
	convertRealTime = 0;
	convertUserTime = 0;
	convertSysTime = 0;
	tps = sysconf(_SC_CLK_TCK); 
	/*Handle BOM bytes for UTF16 specially. 
    Read our values into the first and second elements.*/

	clockStart = times(cpuStart);
	if((rv = read(fd, &buf[0], 1)) == 1 && 
			(rv = read(fd, &buf[1], 1)) == 1){
		void* memset_return; 
		clockEnd = times(cpuEnd);
		readRealTime = (double)(clockEnd - clockStart);
		readUserTime = ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		readSysTime = ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)(tps));
		
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
					print_help();
					quit_converter(fd);
				}
			}
			else {
				free(glyph); 
				print_help();
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
					quit_converter(fd);
				}
			}
			else {
				free(glyph); 
				quit_converter(fd);
			}
			
		}
		else {
			/*file has no BOM*/
			free(glyph); 
			fprintf(stderr, "File has no BOM.\n");
			quit_converter(fd); 
		}
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
		/* Memory write failed, recover from it: */
		if(memset_return == NULL){
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
			quit_converter(fd);
		}
	}

	/* Now deal with the rest of the bytes.*/
	clockStart = times(cpuStart);
	while((rv = read(fd, &buf[0], 1)) == 1 &&  
			(rv = read(fd, &buf[1], 1)) == 1){
		void* memset_return;
		clockEnd = times(cpuEnd);
		readRealTime = readRealTime + (double)(clockEnd - clockStart);
		readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);

		write_glyph(fill_glyph(glyph, buf, source, &fd));
		totalGlyphs = totalGlyphs + 1;
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
	        /* Memory write failed, recover from it: */
	        if(memset_return == NULL){
		        quit_converter(fd);
	        }
	    clockStart = times(cpuStart);
	}
	clockEnd = times(cpuEnd);
	readRealTime = readRealTime + (double)(clockEnd - clockStart);
	readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
	readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
	
	free(cpuStart);
	free(cpuEnd);
	free(glyph);
	
	if (verbosity >= 1) {
		fileData = malloc(sizeof(struct stat)+1);
		if (fileData == NULL){
			printf("Memory error.\n");
			quit_converter(fd);
		}
		memset((void*)fileData, 0, sizeof(struct stat)+1);
		if (stat(filename, fileData) == 0) {
			char fSize[100];
			memset((char*)fSize, 0, sizeof(fSize));
			sprintf(fSize, "%jd", fileData->st_size);
			write(STDERR_FILENO, "Input file size: ", (int)sizeof(char)*strlen("Input file size: "));
			write(STDERR_FILENO, fSize, sizeof(fSize));
			write(STDERR_FILENO, " kb\n", (int)sizeof(char)*strlen(" kb\n"));
			free(fileData);
		}
		else {
			free(fileData);
			/*some error*/
		}
		filePath = realpath(filename, NULL);
		if (filePath != NULL) {
			write(STDERR_FILENO, "Input file path: ", (int)sizeof(char)*strlen("Input file path: "));
			write(STDERR_FILENO, filePath, (int)sizeof(char)*strlen(filePath));
			write(STDERR_FILENO, "\n", (int)sizeof(char)*strlen("\n"));
			free(filePath);
		}
		else {
			free(filePath);
			printf("Memory error.\n");
			quit_converter(fd);
		}
		if (source == BIG) {
			write(STDERR_FILENO, "Input file encoding: UTF-16BE\n", (int)sizeof(char)*strlen("Input file encoding: UTF-16BE\n"));
		}
		else if (source == EIGHT) {
			write(STDERR_FILENO, "Input file encoding: UTF-8\n", (int)sizeof(char)*strlen("Input file encoding: UTF-8\n"));
		}	
		else {
			write(STDERR_FILENO, "Input file encoding: UTF-16LE\n", (int)sizeof(char)*strlen("Input file encoding: UTF-16LE\n"));
		}
		if (conversion == BIG) {
			write(STDERR_FILENO, "Output encoding: UTF-16BE\n", (int)sizeof(char)*strlen("Output encoding: UTF-16BE\n"));
		}
		else {
			write(STDERR_FILENO, "Output encoding: UTF-16LE\n", (int)sizeof(char)*strlen("Output encoding: UTF-16LE\n"));
		}
		hostname = malloc(sizeof(char)+56);
		if (hostname == NULL){
			printf("Memory error.\n");
			quit_converter(fd);
		}
		memset((void*)hostname, 0, sizeof(hostname)+56);
		if (gethostname(hostname, sizeof(hostname)+56) == 0) {
			write(STDERR_FILENO, "Hostmachine: ", (int)sizeof(char)*strlen("Hostmachine: "));
			write(STDERR_FILENO, hostname, (int)sizeof(hostname)+56);
			write(STDERR_FILENO, "\n", (int)sizeof(char)*strlen("\n"));
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
		memset((void*)systemName, 0, sizeof(struct utsname)+1);
		if (uname(systemName) == 0) {
			write(STDERR_FILENO, "Operating System: ", (int)sizeof(char)*strlen("Operating System: "));
			write(STDERR_FILENO, systemName->sysname, (int)sizeof(char)*strlen(systemName->sysname));
			write(STDERR_FILENO, "\n", (int)sizeof(char)*strlen("\n"));
			free(systemName);
		} 
		else {
			free(systemName);
			/*some error*/
		}
		if (verbosity == 2) {
			int surrogatePercentage;
			int asciiPercentage;
			char fSize[100];
			memset((char*)fSize, 0, sizeof(fSize));
			
			surrogatePercentage = (totalSurrogates*1.0)/(totalGlyphs*1.0) * 1000;
			asciiPercentage = (totalAsciis*1.0)/(totalGlyphs*1.0) * 1000;
			sprintf(fSize, "Reading: real=%.1f, user=%.1f, sys=%.1f\n", readRealTime, readUserTime, readSysTime);
			write(STDERR_FILENO, fSize, sizeof(fSize));

			memset((char*)fSize, 0, sizeof(fSize));
			sprintf(fSize, "Converting: real=%.1f, user=%.1f, sys=%.1f\n", convertRealTime, convertUserTime, convertSysTime);
			write(STDERR_FILENO, fSize, sizeof(fSize));
			
			memset((char*)fSize, 0, sizeof(fSize));
			sprintf(fSize, "Writing: real=%.1f, user=%.1f, sys=%.1f\n", writeRealTime, writeUserTime, writeSysTime);
			write(STDERR_FILENO, fSize, sizeof(fSize));

			memset((char*)fSize, 0, sizeof(fSize));
			if (asciiPercentage % 10 >= 5) {
				sprintf(fSize, "%d", (asciiPercentage/10 + 1));
				write(STDERR_FILENO, "ASCII: ", (int)sizeof(char)*strlen("ASCII: "));
				write(STDERR_FILENO, fSize, sizeof(fSize));
				write(STDERR_FILENO, "%\n", (int)sizeof(char)*strlen("%\n"));
			}
			else {
				sprintf(fSize, "%d", (asciiPercentage/10));
				write(STDERR_FILENO, "ASCII: ", (int)sizeof(char)*strlen("ASCII: "));
				write(STDERR_FILENO, fSize, sizeof(fSize));
				write(STDERR_FILENO, "%\n", (int)sizeof(char)*strlen("%\n"));
			}
			memset((char*)fSize, 0, sizeof(fSize));
			if (surrogatePercentage % 10 >= 5) {
				sprintf(fSize, "%d", (int)(surrogatePercentage/10 + 1));
				write(STDERR_FILENO, "Surrogates: ", (int)sizeof(char)*strlen("Surrogates: "));
				write(STDERR_FILENO, fSize, sizeof(fSize));
				write(STDERR_FILENO, "%\n", (int)sizeof(char)*strlen("%\n"));
			}
			else {
				sprintf(fSize, "%d", (int)(surrogatePercentage/10));
				write(STDERR_FILENO, "Surrogates: ", (int)sizeof(char)*strlen("Surrogates: "));
				write(STDERR_FILENO, fSize, sizeof(fSize));
				write(STDERR_FILENO, "%\n", (int)sizeof(char)*strlen("%\n"));
			}
			memset((char*)fSize, 0, sizeof(fSize));
			sprintf(fSize, "%d", totalGlyphs);
			write(STDERR_FILENO, "Glyphs: ", (int)sizeof(char)*strlen("Glyphs: "));
			write(STDERR_FILENO, fSize, sizeof(fSize));
			write(STDERR_FILENO, "\n", (int)sizeof(char)*strlen("\n"));
		}
	}

	free(outputName);
	free(filename);
	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(fd);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph)
{
	unsigned int temp;
	clockStart = times(cpuStart);
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

	clockEnd = times(cpuEnd);
	convertRealTime = convertRealTime + (double)(clockEnd - clockStart);
	convertUserTime = convertUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
	convertSysTime = convertSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);

	return glyph;
}

Glyph* fill_glyph(Glyph* glyph, unsigned int data[2], endianness end, int* fd)
{
	unsigned int bits;
	clockStart = times(cpuStart);
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
			
			
		    quit_converter(*fd);
		}
		if (bits > 0x1FFFFF) {
			/*Outside of code point range*/
			print_help();
			
			
		    quit_converter(*fd);
		}
		if (bits <= 0x007F) {
			totalAsciis = totalAsciis + 1;
		}
		glyph->end = end;
		clockEnd = times(cpuEnd);
		readRealTime = readRealTime + (double)(clockEnd - clockStart);
		readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
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
	 				totalSurrogates = totalSurrogates + 1;
	 			} else {
	 				lseek(*fd, -2, SEEK_CUR);
	 				glyph->surrogate = false; 
	 				/*Return an error for invalid data*/
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
		clockEnd = times(cpuEnd);
		readRealTime = readRealTime + (double)(clockEnd - clockStart);
		readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
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
		clockStart = times(cpuStart);
		if(glyph->surrogate){
			fwrite(glyph->bytes, sizeof(glyph->bytes[0]), SURROGATE_SIZE, wd);
		} else {
			fwrite(glyph->bytes, sizeof(glyph->bytes[0]), NON_SURROGATE_SIZE, wd);		}
			clockEnd = times(cpuEnd);
			writeRealTime = writeRealTime + (double)(clockEnd - clockStart);
			writeUserTime = writeUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
			writeSysTime = writeSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
			fclose(wd);
	}
	else {
		clockStart = times(cpuStart);
		if(glyph->surrogate){
			write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
		} else {
			write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
		}
		clockEnd = times(cpuEnd);
		writeRealTime = writeRealTime + (double)(clockEnd - clockStart);
		writeUserTime = writeUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		writeSysTime = writeSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
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
	if ((argc - optind) > 2) {
		print_help();
		quit_converter(NO_FD);
	}
	if (optind < argc) {
		free(filename);
		filename = strdup(argv[optind]);
	}
	else {
		print_help();
		quit_converter(NO_FD);
	}
	if ((optind+1) < argc) {
		if (strcmp(argv[optind], filename) != 0) {
			outputName = strdup(argv[(optind+1)]);
		}
		else {
			print_help();
			quit_converter(NO_FD);
		}
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

	clockStart = times(cpuStart);
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
	clockEnd = times(cpuEnd);
	convertRealTime = convertRealTime + (double)(clockEnd - clockStart);
	convertUserTime = convertUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
	convertSysTime = convertSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);

	return glyph;
}