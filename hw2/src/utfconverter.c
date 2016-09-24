#include "utfconverter.h"


char* outputName;
char* filename;
char* filePath;
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
int bomExist;
int fd;
int main(int argc, char** argv)
{
	/*
		Potential ERRORS?
		
	*/
	int rv;
	Glyph* glyph;
	unsigned int buf[2];
	char* hostname;
	struct utsname* systemName; 
	struct stat* fileData;
	
	int od; /*For reading the output file if there is any.*/

	/*After calling parse_args(), filename and conversion should be set. */
	verbosity = 0;
	filename = malloc(2);
	if (filename == NULL) {
		return EXIT_FAILURE;
	}
	outputName = malloc(2);
	if (outputName == NULL) {
		return EXIT_FAILURE;
	}
	parse_args(argc, argv);
	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		print_help();
		quit_converter(NO_FD);
	}
	numBytes = 0;
	rv = 0; 
	od = 0;
	if (memset(buf, 0, sizeof(buf)) == NULL) {
		quit_converter(fd);
	}
	clockStart = 0;
	clockEnd = 0;
	glyph = malloc(sizeof(Glyph)+1);
	if (glyph == NULL) {
		quit_converter(fd);
	}
	sparky = 0;
	totalGlyphs = 1;
	totalSurrogates = 0;
	totalAsciis = 0;
	cpuStart = malloc(sizeof(struct tms)+1);
	if (cpuStart == NULL) {
		free(glyph);
		quit_converter(fd);
	}
	cpuEnd = malloc(sizeof(struct tms)+1);
	if (cpuEnd == NULL) {
		free(glyph);
		free(cpuStart);
		quit_converter(fd);
	}
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
	bomExist = 0;

	/*Handle BOM bytes for UTF16 specially. 
    Read our values into the first and second elements.*/
	clockStart = times(cpuStart);
	if((rv = read(fd, &buf[0], 1)) == 1 && 
			(rv = read(fd, &buf[1], 1)) == 1){
		void* memset_return; 
		clockEnd = times(cpuEnd);
		readRealTime = ((double)(clockEnd - clockStart) / (double)tps);
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
					free(cpuStart);
					free(cpuEnd);
					print_help();
					quit_converter(fd);
				}
			}
			else {
				free(glyph); 
				free(cpuStart);
				free(cpuEnd);
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
					free(cpuStart);
					free(cpuEnd);
					quit_converter(fd);
				}
			}
			else {
				free(glyph);
				free(cpuStart);
				free(cpuEnd);
				quit_converter(fd);
			}
			
		}
		else {
			/*file has no BOM*/
			free(glyph); 
			free(cpuStart);
			free(cpuEnd);
			quit_converter(fd); 
		}
		
		/*Deal with output file if there is one.*/
		if(memset(buf, 0, sizeof(buf)) == NULL) {
			free(glyph);
			free(cpuStart);
			free(cpuEnd);
			quit_converter(fd);
		}
		od = open(outputName, O_RDWR | O_APPEND);
		if (od != -1) {
			/* Look for BOM*/
			if((rv = read(od, &buf[0], 1)) == 1 && 
				(rv = read(od, &buf[1], 1)) == 1){
				if (sparky == 1) {
					buf[0] = buf[0] >> 24;
					buf[1] = buf[1] >> 24;
				}
				if(buf[0] == 0xff && buf[1] == 0xfe && 
					conversion == LITTLE){
					bomExist = 1;
				} else if(buf[0] == 0xfe && buf[1] == 0xff && 
					conversion == BIG){
					bomExist = 1;
				}
				else if(buf[0] == 0xef && buf[1] == 0xbb){
					if ((rv = read(od, &buf[0], 1)) == 1) {
						if (buf[0] == 0xbf && conversion == EIGHT) {
							bomExist = 1;
						}
						else {
							free(cpuStart);
							free(cpuEnd);
							print_help();
							quit_converter(fd);
						}
					}
					else {
						free(cpuStart);
						free(cpuEnd);
						print_help();
						quit_converter(fd);
					}
					
				}
				else {
					free(cpuStart);
					free(cpuEnd);
					print_help();
					quit_converter(fd);
				}
			}
			else {
				if (bomExist == 0) {
					memset_return = memset(glyph, 0, sizeof(Glyph)+1);
					if(memset_return == NULL){
						free(cpuStart);
						free(cpuEnd);
						print_help();
						quit_converter(fd);
					}
					if (conversion == LITTLE) {
						glyph->surrogate = false;
						glyph->bytes[0] = 0xff;
						glyph->bytes[1] = 0xfe;
						write_glyph(glyph);
					}
					else if (conversion == BIG) {
						glyph->surrogate = false;
						glyph->bytes[0] = 0xfe;
						glyph->bytes[1] = 0xff;
						write_glyph(glyph);
					}
					else {
						glyph->surrogate = true;
						glyph->bytes[0] = 0xef;
						glyph->bytes[1] = 0xbb;
						glyph->bytes[2] = 0xbf;
						glyph->end = EIGHTTHREE;
						write_glyph(glyph);
						
					}
				}
			}
			close(od);
		}
		else {
			/* Now write the BOM out*/
			memset_return = memset(glyph, 0, sizeof(Glyph)+1);
			if(memset_return == NULL){
				free(glyph);
				free(cpuStart);
				free(cpuEnd);
				quit_converter(fd);
			}
			if (conversion == LITTLE) {
				glyph->surrogate = false;
				glyph->bytes[0] = 0xff;
				glyph->bytes[1] = 0xfe;
				write_glyph(glyph);
			}
			else if (conversion == BIG) {
				glyph->surrogate = false;
				glyph->bytes[0] = 0xfe;
				glyph->bytes[1] = 0xff;
				write_glyph(glyph);
			}
			else {
				glyph->surrogate = true;
				glyph->bytes[0] = 0xef;
				glyph->bytes[1] = 0xbb;
				glyph->bytes[2] = 0xbf;
				glyph->end = EIGHTTHREE;
				write_glyph(glyph);
				
			}
		}
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
		/* Memory write failed, recover from it: */
		if(memset_return == NULL){
			free(glyph);
			free(cpuStart);
			free(cpuEnd);
			quit_converter(fd);
		}
	}

	/* Now deal with the rest of the bytes.*/
	clockStart = times(cpuStart);
	if (memset(buf, 0, sizeof(buf)) == NULL) {
		free(glyph);
		free(cpuStart);
		free(cpuEnd);
		quit_converter(fd);
	}
	while((rv = read(fd, &buf[0], 1)) == 1){
		void* memset_return;
		clockEnd = times(cpuEnd);
		readRealTime = readRealTime + ((double)(clockEnd - clockStart) / (double)tps);
		readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);

		write_glyph(fill_glyph(glyph, buf, source, &fd));
		totalGlyphs = totalGlyphs + 1;
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
	        /* Memory write failed, recover from it: */
	        if(memset_return == NULL){
	        	free(glyph);
	        	free(cpuStart);
				free(cpuEnd);
		        quit_converter(fd);
	        }
	    clockStart = times(cpuStart);
	    memset(buf, 0, sizeof(buf));
	}
	clockEnd = times(cpuEnd);
	readRealTime = readRealTime + ((double)(clockEnd - clockStart) / (double)tps);
	readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
	readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
	
	free(cpuStart);
	free(cpuEnd);
	free(glyph);
	if (verbosity >= 1) {
		char absoluteFilePath[400];
		fileData = malloc(sizeof(struct stat)+1);
		if (fileData == NULL){
			quit_converter(fd);
		}
		if (memset((void*)fileData, 0, sizeof(struct stat)+1) == NULL) {
			free(fileData);
			quit_converter(fd);
		}
		if (stat(filename, fileData) == 0) {
			char fSize[100];
			memset((char*)fSize, 0, sizeof(fSize));
			sprintf(fSize, "%d", (int)fileData->st_size);
			write(STDERR_FILENO, "\nInput file size: ", (int)sizeof(char)*strlen("\nInput file size: "));
			write(STDERR_FILENO, fSize, sizeof(fSize));
			write(STDERR_FILENO, " kb\n", (int)sizeof(char)*strlen(" kb\n"));
			free(fileData);
		}
		else {
			free(fileData);
			/*some error*/
			quit_converter(fd);
		}
		if (memset((void*)absoluteFilePath, 0, sizeof(absoluteFilePath)) == NULL) {
			quit_converter(fd);
		}
		filePath = realpath(filename, absoluteFilePath);
		if (filePath != NULL) {
			write(STDERR_FILENO, "Input file path: ", (int)sizeof(char)*strlen("Input file path: "));
			write(STDERR_FILENO, filePath, (int)sizeof(absoluteFilePath));
			write(STDERR_FILENO, "\n", (int)sizeof(char)*strlen("\n"));
		}
		else {
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
		hostname = malloc(100);
		if (hostname == NULL){
			quit_converter(fd);
		}
		if(memset((void*)hostname, 0, 100) == NULL){
			free(hostname);
			quit_converter(fd);
		}
		if (gethostname(hostname, 100) == 0) {
			write(STDERR_FILENO, "Hostmachine: ", (int)sizeof(char)*strlen("Hostmachine: "));
			write(STDERR_FILENO, hostname, 100);
			write(STDERR_FILENO, "\n", (int)sizeof(char)*strlen("\n"));
			free(hostname);
		}
		else {
			free(hostname);
			quit_converter(fd);
		}
		systemName = malloc(sizeof(struct utsname)+1);
		if (systemName == NULL){
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
			quit_converter(fd);
		}
		if (verbosity == 2) {
			int surrogatePercentage;
			int asciiPercentage;
			char fSize[100];
			if (memset((char*)fSize, 0, sizeof(fSize)) == NULL) {
				quit_converter(fd);
			}
			
			surrogatePercentage = (totalSurrogates*1.0)/(totalGlyphs*1.0) * 1000;
			asciiPercentage = (totalAsciis*1.0)/(totalGlyphs*1.0) * 1000;
			sprintf(fSize, "Reading: real=%f, user=%f, sys=%f\n", readRealTime, readUserTime, readSysTime);
			write(STDERR_FILENO, fSize, sizeof(fSize));

			if (memset((char*)fSize, 0, sizeof(fSize)) == NULL) {
				quit_converter(fd);
			}
			sprintf(fSize, "Converting: real=%f, user=%f, sys=%f\n", convertRealTime, convertUserTime, convertSysTime);
			write(STDERR_FILENO, fSize, sizeof(fSize));
			
			if (memset((char*)fSize, 0, sizeof(fSize)) == NULL) {
				quit_converter(fd);
			}
			sprintf(fSize, "Writing: real=%f, user=%f, sys=%f\n", writeRealTime, writeUserTime, writeSysTime);
			write(STDERR_FILENO, fSize, sizeof(fSize));

			if (memset((char*)fSize, 0, sizeof(fSize)) == NULL) {
				quit_converter(fd);
			}
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
			if (memset((char*)fSize, 0, sizeof(fSize)) == NULL) {
				quit_converter(fd);
			}
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
			if (memset((char*)fSize, 0, sizeof(fSize)) == NULL) {
				quit_converter(fd);
			}
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
	return EXIT_SUCCESS;
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
	convertRealTime = convertRealTime + ((double)(clockEnd - clockStart) / (double)tps);
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
		if (conversion == end) {
			/*Trying to convert utf8 to utf8*/
			free(cpuStart);
			free(cpuEnd);
			free(glyph);
			print_help();
			quit_converter(*fd);
		}
		if (data[0] > 0x7F) {
			if(read(*fd, &data[1], 1) == 1) {
				if (sparky == 1) {
					data[1] = data[1] >> 24;
				}
			}
			else {/*corrupted file*/
				free(cpuStart);
				free(cpuEnd);
				free(glyph);
				print_help();
				quit_converter(*fd);
			}
		}
		if (data[0] <= 0x7F) {/*ONE BYTE*/
			bits = data[0];
			glyph->bytes[0] = data[0];
			numBytes = 1;
			totalAsciis = totalAsciis + 1;
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
				free(cpuStart);
				free(cpuEnd);
				free(glyph);
				print_help();
				quit_converter(*fd);
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
				free(cpuStart);
				free(cpuEnd);
				free(glyph);
				print_help();
				quit_converter(*fd);
				/*Corrupted File*/
			}
		}
		else {/*ERROR UNKNOWN ENCODING!*/
			free(cpuStart);
			free(cpuEnd);
			free(glyph);
			print_help();
		    quit_converter(*fd);
		}
		if (bits > 0x1FFFFF) {
			/*Outside of code point range*/
			free(cpuStart);
			free(cpuEnd);
			free(glyph);
			print_help();
		    quit_converter(*fd);
		}
		glyph->end = end;
		clockEnd = times(cpuEnd);
		readRealTime = readRealTime + ((double)(clockEnd - clockStart) / (double)tps);
		readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
		return convert(glyph, conversion);
	}
	else { /*UTF-16 ENCODING*/
		if(read(*fd, &data[1], 1) == 1) {
			if (sparky == 1) {
				data[1] = data[1] >> 24;
			}
		}
		else {
			free(cpuStart);
			free(cpuEnd);
			free(glyph);
			print_help();
		   	quit_converter(*fd);
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
	 				free(cpuStart);
					free(cpuEnd);
					free(glyph);
					print_help();
		   			quit_converter(*fd);
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
		readRealTime = readRealTime + ((double)(clockEnd - clockStart) / (double)tps);
		readUserTime = readUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		readSysTime = readSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
		if (conversion == EIGHT) {
			return convert_reverse(glyph, end);
		} else {
			if (conversion != end) {
				return swap_endianness(glyph);
			}
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
		if (bomExist == 1) {
			unsigned char tempBytes[2];
			if (conversion == BIG) {
				tempBytes[0] = '\0';
				tempBytes[1] = '\n';
				fwrite(tempBytes, sizeof(tempBytes[0]), 2, wd);
			}
			else if (conversion == EIGHT) {
				tempBytes[0] = '\n';
				fwrite(tempBytes, sizeof(tempBytes[0]), 1, wd);;
			}
			else{
				tempBytes[0] = '\n';
				tempBytes[1] = '\0';
				fwrite(tempBytes, sizeof(tempBytes[0]), 2, wd);;
			}
			bomExist = 0;
		}
		if (conversion == EIGHT) {
			if (glyph->end == EIGHTONE) {
				fwrite(glyph->bytes, sizeof(glyph->bytes[0]), 1, wd);
			}
			else if(glyph->end == EIGHTTWO) {
				fwrite(glyph->bytes, sizeof(glyph->bytes[0]), 2, wd);
			}
			else if(glyph->end == EIGHTTHREE) {
				fwrite(glyph->bytes, sizeof(glyph->bytes[0]), 3, wd);
			}
			else{
				fwrite(glyph->bytes, sizeof(glyph->bytes[0]), 4, wd);
			}
		}
		else {
			if(glyph->surrogate){
				fwrite(glyph->bytes, sizeof(glyph->bytes[0]), SURROGATE_SIZE, wd);
			} else {
				fwrite(glyph->bytes, sizeof(glyph->bytes[0]), NON_SURROGATE_SIZE, wd);		
			}
		}
		clockEnd = times(cpuEnd);
		writeRealTime = writeRealTime + ((double)(clockEnd - clockStart) / (double)tps);
		writeUserTime = writeUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		writeSysTime = writeSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
		fclose(wd);
	}
	else {
		clockStart = times(cpuStart);
		if (conversion == EIGHT) {
			if (glyph->end == EIGHTONE) {
				write(STDOUT_FILENO, glyph->bytes, 1);
			}
			else if(glyph->end == EIGHTTWO) {
				write(STDOUT_FILENO, glyph->bytes, 2);
			}
			else if(glyph->end == EIGHTTHREE) {
				write(STDOUT_FILENO, glyph->bytes, 3);
			}
			else{
				write(STDOUT_FILENO, glyph->bytes, 4);
			}
		}
		else {
			if(glyph->surrogate){
				write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
			} else {
				write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
			}
		}
		clockEnd = times(cpuEnd);
		writeRealTime = writeRealTime + ((double)(clockEnd - clockStart) / (double)tps);
		writeUserTime = writeUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
		writeSysTime = writeSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
	}	
}

void parse_args(int argc, char** argv)
{
	int option_index, c, h, u;
	char* endian_convert;
	static struct option long_options[] = {
		{"help", no_argument, 0, 'y'},
		{"UTF=", required_argument, 0, 'z'},
		{"h", no_argument, 0, 'x'},
		{0, 0, 0, 0}
	};
	endian_convert = NULL;
	h = 0;
	u = 0;

	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	while((c = getopt_long(argc, argv, "hvu:", long_options, &option_index)) != -1){
		switch(c){ 
			case 'h':
				h = h+1;
				break;
			case 'y':
				h = h+1;
				break;
			case 'u':
				u = u +1;
				break;
			case 'z':
				u = u + 1;
				break;
			case 'v':
				break;
			case 'x':
			default:
				print_help();
				quit_converter(NO_FD);
				break;
		}
	}
	if (h > 0) {
		print_help();
		free(filename);
		free(outputName);
		close(STDERR_FILENO);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(NO_FD);
		exit(EXIT_SUCCESS);
	}
	if (u == 0 || u > 1) {
		print_help();
		quit_converter(NO_FD);
	}
	optind = 1;
	while((c = getopt_long(argc, argv, "hvu:", long_options, &option_index)) != -1){
		switch(c){ 
			case 'h':
				break;
			case 'y':
				break;
			case 'u':
				if ((strcmp(optarg, "16LE") != 0) && (strcmp(optarg, "16BE") != 0) && (strcmp(optarg, "8") != 0)) {
					print_help();
					quit_converter(NO_FD);
				}
				endian_convert = optarg;
				break;
			case 'z':
				if(strcmp(optarg, "") == 0){ 
					print_help();
					quit_converter(NO_FD);
				}
				if ((strcmp(argv[(optind-1)], "--UTF=16LE") == 0) || (strcmp(argv[(optind-1)], "--UTF=16BE") == 0) || (strcmp(argv[(optind-1)], "--UTF=8") == 0)) {
					endian_convert = optarg;
				}
				else {
					print_help();
					quit_converter(NO_FD);
				}
				break;
			case 'v':
				if (verbosity < 2) {
					verbosity = verbosity + 1;
				}
				break;
			case 'x':
				break;
			default:
				print_help();
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
		if (strcmp(argv[(optind+1)], filename) != 0) {
			free(outputName);
			outputName = strdup(argv[(optind+1)]);
		}
		else {
			print_help();
			quit_converter(NO_FD);
		}
	}
	if(endian_convert == NULL){
		print_help();
		quit_converter(NO_FD);
	}

	if(strcmp(endian_convert, "16LE") == 0){ 
		conversion = LITTLE;
	} else if(strcmp(endian_convert, "16BE") == 0){
		conversion = BIG;
	} else if(strcmp(endian_convert, "8") == 0){
		conversion = EIGHT;
	} else {
		print_help();
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
	/* if end is UTF8 then just return. no converting*/
	if (end == EIGHT) {
		return glyph;
	}
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
		totalSurrogates = totalSurrogates + 1;
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
	convertRealTime = convertRealTime + ((double)(clockEnd - clockStart) / (double)tps);
	convertUserTime = convertUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
	convertSysTime = convertSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);

	return glyph;
}

Glyph* convert_reverse(Glyph* glyph, endianness end) {
	unsigned int msb;
	unsigned int lsb;
	unsigned int bits;

	clockStart = times(cpuStart);
	msb = 0;
	lsb = 0;
	bits = 0;
	if (end == BIG) {
		msb = (glyph->bytes[0] << 8) + glyph->bytes[1];
	}
	else {
		msb = glyph->bytes[0] + (glyph->bytes[1] << 8);
	}
	if (glyph->surrogate) {
		if (end == BIG) {
			lsb = (glyph->bytes[2] << 8) + glyph->bytes[3];
		}
		else {
			lsb = glyph->bytes[2] + (glyph->bytes[3] << 8);
		}
		bits = (msb - 0xD800) << 10;
		bits = bits + (lsb - 0xDC00);
		bits = bits + 0x10000;
	}
	else {
		bits = msb;
	}
	if (bits <= 0x7F) {
		/*ONE BYTE*/
		glyph->surrogate = false;
		glyph->bytes[0] = bits;
		glyph->bytes[1] = 0;
		glyph->end = EIGHTONE;
	}
	else if (bits >= 0x80 && bits <= 0x7FF) {
		/*TWO BYTES*/
		glyph->bytes[0] = 0xC0 + (bits >> 6);
		glyph->bytes[1] = 0x80 + (bits & 0x3F);
		glyph->end = EIGHTTWO;
		glyph->surrogate = false;
	}
	else if (bits >= 0x800 && bits <= 0xFFFF) {
		/*THREE BYTES*/
		glyph->bytes[0] = 0xE0 + (bits >> 12);
		glyph->bytes[1] = 0x80 + ((bits >> 6) & 0x3F);
		glyph->bytes[2] = 0x80 + (bits & 0x3F);
		glyph->end = EIGHTTHREE;
		glyph->surrogate = true;
	}
	else if (bits >= 0x10000 && bits <= 0x1FFFFF) {
		/*FOUR BYTES*/
		glyph->bytes[0] = 0xF0 + (bits >> 18);
		glyph->bytes[1] = 0x80 + ((bits >> 12) & 0x3F);
		glyph->bytes[2] = 0x80 + ((bits >> 6) & 0x3F);
		glyph->bytes[3] = 0x80 + (bits & 0x3F);
		glyph->end = EIGHTFOUR;
		glyph->surrogate = true;
	}
	else {
		/*Outside of code point range*/
		free(cpuStart);
		free(cpuEnd);
		free(glyph);
		print_help();
		quit_converter(fd);
	}
	clockEnd = times(cpuEnd);
	convertRealTime = convertRealTime + ((double)(clockEnd - clockStart) / (double)tps);
	convertUserTime = convertUserTime + ((double)(cpuEnd->tms_utime - cpuStart->tms_utime) / (double)tps);
	convertSysTime = convertSysTime + ((double)(cpuEnd->tms_stime - cpuStart->tms_stime) / (double)tps);
	return glyph;
}