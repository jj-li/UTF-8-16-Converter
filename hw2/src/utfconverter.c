#include "utfconverter.h"

char* filename;
endianness source;
endianness conversion;
int sparky;
int main(int argc, char** argv)
{
	/*
		Potential ERRORS?
		CORRUPTED FILE
		FILE DOES NOT EXIST? GOTTA LET USER KNOW?
		need to work on the asm function
	*/
	int fd;
	int rv;
	Glyph* glyph;
	unsigned int buf[2]; 

	/*scp -r -P 24 ../hw2 jijli@sparky.ic.stonybrook.edu:
	*/
	/*After calling parse_args(), filename and conversion should be set. */
	parse_args(argc, argv);

	fd = open(filename, O_RDONLY); 
	rv = 0; 
	memset(buf, 0, sizeof(buf));
	glyph = malloc(sizeof(Glyph)+1); 
	sparky = 0;
	/*Handle BOM bytes for UTF16 specially. 
    Read our values into the first and second elements.*/
	if((rv = read(fd, &buf[0], 1)) == 1 && 
			(rv = read(fd, &buf[1], 1)) == 1){
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
			free(&glyph->bytes); 
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
			memset(glyph, 0, sizeof(Glyph)+1);
		}
		if (conversion == LITTLE) {
			glyph->surrogate = true;
			glyph->bytes[0] = 0xff;
			glyph->bytes[1] = 0xfe;
			write_glyph(glyph);
		}
		else {
			glyph->surrogate = true;
			glyph->bytes[0] = 0xfe;
			glyph->bytes[1] = 0xff;
			write_glyph(glyph);
		}
	}

	/* Now deal with the rest of the bytes.*/
	while((rv = read(fd, &buf[0], 1)) == 1 &&  
			(rv = read(fd, &buf[1], 1)) == 1){
		void* memset_return;

		write_glyph(fill_glyph(glyph, buf, source, &fd));
		memset_return = memset(glyph, 0, sizeof(Glyph)+1);
	        /* Memory write failed, recover from it: */
	        if(memset_return == NULL){
		        /* tweak write permission on heap memory. */
		        /*asm("movl $8, %esi\n\t"
		            "movl $.LC0, %edi\n\t"
		            "movl $0, %eax");*/
		        /* Now make the request again. */
		        memset(glyph, 0, sizeof(Glyph)+1);
	        }
	}

	free(glyph);
	quit_converter(NO_FD);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph)
{
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

	
	bits = 0; 
	if (end == BIG) {
		bits = bits | (data[0] + (data[1] << 8));
	}
	else {
		bits = bits | ((data[0] << 8) + data[1]);
	}
	/* Check high surrogate pair using its special value range.*/
	/*if(bits > 0x10000){ 
			//bits = 0; bits |= (bytes[FIRST] + (bytes[SECOND] << 8)) 
			if((glyph->bytes[0] << 8) >= 0xD800 && (glyph->bytes[0] << 8) <= 0xDBFF){ //Check low surrogate pair.
				if ((glyph->bytes[1] << 8) >= 0xDC00 && (glyph->bytes[1] << 8) <= 0xDFFF)
				{
					lseek(*fd, -OFFSET, SEEK_CUR); 
					glyph->surrogate = true;
				}
				else {
					print_help();//LONE SURROGATE PAIR! ERROR!
				}
			}
			else {
					print_help();//LONE SURROGATE PAIR! ERROR!
			}
	}*/
	if(bits > 0x000F && bits < 0xF8FF){ 
 			/*bits = '0'; bits |= (bytes[FIRST] + (bytes[SECOND] << 8))*/
 			if(bits > 0xDAAF && bits < 0x00FF){ /*Check low surrogate pair.*/
 				glyph->surrogate = false; 
 			} else {
 				lseek(*fd, 0, SEEK_CUR); 
 				glyph->surrogate = true;
 			}
 	}

	if(!glyph->surrogate){
		glyph->bytes[1] = glyph->bytes[1] & 0;
	}
	glyph->end = end;
	if (conversion != end) {
		return swap_endianness(glyph);
	}
	return glyph;
}

void write_glyph(Glyph* glyph)
{
	if(glyph->surrogate){
		write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
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
}

void parse_args(int argc, char** argv)
{
	int option_index, c;
	char* endian_convert;
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"h", no_argument, 0, 'h'},
		{"UTF", required_argument, 0, 'u'},
		{0, 0, 0, 0}
	};
	endian_convert = NULL;

	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	while((c = getopt_long(argc, argv, "hu:", long_options, &option_index)) != -1){
		switch(c){ 
			case 'h':
				print_help();
				break;
			case 'u':
				if(optind >= argc){
					fprintf(stderr, "Filename not given.\n");
					print_help();
				} 
				else {
					if((strcmp(argv[optind], "16LE") == 0) || (strcmp(argv[optind], "16BE") == 0)){ 
						endian_convert = argv[optind];
						optind = optind + 1;
						if(optind < argc){
							filename = strdup(argv[optind]);
						} else {
							fprintf(stderr, "Filename not given.\n");
							print_help();
						}
					}
					else{
						endian_convert = optarg;
						if(optind > 1){
							filename = strdup(argv[optind]);
						} else {
							fprintf(stderr, "Filename not given.\n");
							print_help();
						}
					}
				}
				break;
			default:
				fprintf(stderr, "Unrecognized argument.\n");
				quit_converter(NO_FD);
				break;
		}

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
	elements = 10;
	for (i = 0; i < elements; i = i+1) {
		if (i < 9) {
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
	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	if(fd != NO_FD)
		close(fd);
	exit(0);
	/* Ensure that the file is included regardless of where we start compiling from. */
}
