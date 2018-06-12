#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."
#define SECTOR 512


/* Variable to store pointer to program name */
char *progname; 

/* Variable to store pointer to the filename for the file being read. */
char *elfname;

/* Structure to store command line options */
static struct {
    int vm;
    int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt,...);

int main(int argc, char **argv)
{
    /* Process command line options */
    progname = argv[0];
    options.vm = 0;
    options.extended = 0;
    while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) 
    {
        char *option = &argv[1][2];

        if (strcmp (option, "vm") == 0) {
            options.vm = 1;
        } else if (strcmp (option, "extended") == 0) {
            options.extended = 1;
        } else {
            error ("%s: invalid option\nusage: %s %s\n", progname, progname, ARGS);
        }
        argc--;
        argv++;
    }
    if (options.vm == 1) {
      /* This option is not needed in project 1 so we doesn't bother
       * implementing it*/
      error ("%s: option --vm not implemented\n", progname);
    }
    if (argc < 3) {
        /* at least 3 args (createimage bootblock kernel) */
        error ("usage: %s %s\n", progname, ARGS);
    }
    create_image (argc - 1, argv + 1);
    return 0;
}

char *read_to_array(FILE *fp, Elf32_Off offset, uint32_t filesz)
{   
    char *data = calloc(1, filesz); // creating a char array the size of given segment    
    fseek(fp, offset, SEEK_SET);    // Seek to where first segment start
    
    fread(data, filesz, 1, fp);  // Read the segment
    return data;
}

Elf32_Ehdr elf_header(FILE *fp)
{
    Elf32_Ehdr hdr; // Declare elf header

    fread(&hdr, sizeof(Elf32_Ehdr), 1, fp); // Read the elf header

    return hdr;
}

Elf32_Phdr prog_header(FILE *fp, Elf32_Ehdr hdr, int j)
{
    Elf32_Phdr phdr; // Declare program header

    fseek(fp, hdr.e_phoff + (j *hdr.e_phentsize), SEEK_SET); // Locate program header
    fread(&phdr, sizeof(Elf32_Phdr), 1, fp); // Read program header into phdr

    return phdr;
}

static void boot_pad(FILE *fp)
{
    fseek(fp, SECTOR-2, SEEK_SET); // Locate where to put magic signature
    fputs("\x55", fp);  // Magic
    fputs("\xaa", fp);  // Signature  
}

static void padding(FILE *fp)
{
    int curr = ftell(fp); // Find out byte number you are on
    fseek(fp, SECTOR -(curr%SECTOR), SEEK_END); // Pad by searching to nearest 512 bytes
}

int kernel_size(FILE *img)
{
    fseek(img, 0, SEEK_CUR); // go to end of image file (padding included)
    int size = (ftell(img) - SECTOR)/SECTOR; // Remove size of boot sector when calculating kernel size in sectors
    fseek(img, 2, SEEK_SET); // Locate where to write kernel size
    fputc(size, img); // Write kernel size to image
    return size;
}

static void create_image(int nfiles, char *files[])
{
    int i, j;

    Elf32_Ehdr hdr;// Declare an elf header
    Elf32_Phdr phdr; // Declare a program header
    FILE *img = fopen("image", "w+"); // Open the image file
    char *data;
    //int pos;

    for(i = 0; i < nfiles; i++) // For each file
    {
        FILE *fp = fopen(files[i], "rb"); // Open file
        hdr = elf_header(fp); // Read elf header

        for(j = 0; j < hdr.e_phnum; j++) // For each program header in current file
        {
            phdr = prog_header(fp, hdr, j); // Get program header
            data = read_to_array(fp, phdr.p_offset, phdr.p_filesz); // Put data in char array     
            fwrite(data, phdr.p_filesz, 1, img); // Write data to image file

            if (i == 0) // If file is boot block
            {
                boot_pad(img); // Pad boot sector to make it 512 bytes
            }
            else // If not boot block (kernel)
            {
                padding(img); // Pad so dividable by 512
                kernel_size(img); //Write kernel size to image
            }                      
        }       
    }
}


/* print an error message and exit */
static void error(char *fmt, ...)
{
    va_list args;

    va_start (args, fmt);
    vfprintf (stderr, fmt, args);
    va_end (args);
    if (errno != 0) {
        perror (NULL);
    }
    exit (EXIT_FAILURE);
}

