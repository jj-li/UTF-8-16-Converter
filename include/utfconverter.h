#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/times.h>
#include <stdint.h>


#define MAX_BYTES 4
#define SURROGATE_SIZE 4
#define NON_SURROGATE_SIZE 2
#define NO_FD -1
#define OFFSET 4

#define FIRST  10000000
#define SECOND 20000000
#define THIRD  30000000
#define FOURTH 40000000

#ifdef __STDC__
#define P(x) x
#else
#define P(x) ()
#endif

/** The enum for endianness. */
typedef enum {LITTLE, BIG, EIGHT, EIGHTONE, EIGHTTWO, EIGHTTHREE, EIGHTFOUR} endianness;

/** The struct for a codepoint glyph. */
typedef struct Glyph {
	unsigned char bytes[MAX_BYTES];
	endianness end;
	bool surrogate;
} Glyph;

/** The given filename. */
extern char* filename;

/** The usage statement. */
const char* USAGE[12] = {"Command line utility for converting files from UTF-16LE to UTF-16BE or viceversa.\n",
	"Usage: ./utf [-h|--help] [-v|-vv] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]\n",
	"  Option arguments:",
	"    -h, --help\t    Displays this usage.",
	"    -v, -vv\t    Toggles the verbosity of the program to level 1 or 2.\n",
	"  Mandatory argument:",
	"    -u OUT_ENC, --UTF=OUT_ENC\tSets the output encoding.",
	"\t\t\t\tValid values for OUT_ENC: 8, 16LE, 16BE\n",
	"  Positional Arguments:",
	"    IN_FILE\tThe file to convert.",
	"    [OUT_FILE] Output file name. If not present, defaults to stdout.",
	"\0"};

/** Which endianness to convert to. */
extern endianness conversion;

/** Which endianness the source file is in. */
extern endianness source;

/**
 * A function that swaps the endianness of the bytes of an encoding from
 * LE to BE and vice versa.
 *
 * @param glyph The pointer to the glyph struct to swap.
 * @return Returns a pointer to the glyph that has been swapped.
 */
Glyph* swap_endianness(Glyph* glyph);

/**
 * Fills in a glyph with the given data in data[2], with the given endianness 
 * by end.
 *
 * @param glyph 	The pointer to the glyph struct to fill in with bytes.
 * @param data[2]	The array of data to fill the glyph struct with.
 * @param end	   	The endianness enum of the glyph.
 * @param fd 		The int pointer to the file descriptor of the input 
 * 			file.
 * @return Returns a pointer to the filled-in glyph.
 */
Glyph* fill_glyph(Glyph* glyph, unsigned int data[2], endianness end, int* fd);

/**
 * Writes the given glyph's contents to stdout.
 *
 * @param glyph The pointer to the glyph struct to write to stdout.
 */
void write_glyph (Glyph* glyph);

/**
 * Calls getopt() and parses arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments as an array of string.
 */
void parse_args(int argc, char** argv);

/**
 * Prints the usage statement.
 */
void print_help();

/**
 * Closes file descriptors and frees list and possibly does other
 * bookkeeping before exiting.
 *
 * @param The fd int of the file the program has opened. Can be given
 * the macro value NO_FD (-1) to signify that we have no open file
 * to close.
 */
void quit_converter(int fd);

/**
* A function that converts a UTF-8 glyph to a UTF-16LE or UTF-16BE
* glyph, and returns the result as a pointer to the converted glyph.
*
* @param glyph The UTF-8 glyph to convert.
* @param end The endianness to convert to (UTF-16LE or UTF-16BE).
* @return The converted glyph.
*/
Glyph* convert(Glyph* glyph, endianness end);

/**
* A function that converts a UTF-16LE or UTF-16BE glyph to a
* UTF-8 glyph, and returns the result as a pointer to the
* converted glyph.
*
* @param glyph The UTF-16LE/BE glyph to convert.
* @param end The endianness of the source glyph.
* @return The converted glyph.
*/
Glyph* convert_reverse(Glyph* glyph, endianness end);