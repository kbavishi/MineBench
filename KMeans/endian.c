#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char a,b,c,d;
} Four_byte;

int main(int argc, char **argv) {
    int        i;
    long       len;
    FILE      *fptr;
    Four_byte *buf;

    if (argc != 2) {
	fprintf(stderr,"Usage: %s <filename>\n",argv[0]);
	exit(1);
    }

    /* find the length of the file */
    fptr = fopen(argv[1], "r");
    fseek(fptr, 0, SEEK_END);
    len = ftell(fptr);

    /* allocate buffer and read all data from file */
    buf = (Four_byte*) malloc(len);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, 1, len, fptr);
    fclose(fptr);

    /* assuming all data stored in the file are 4-byte type */
    for (i=0; i<len/4; i++) {
        char tmp;
        tmp = buf[i].a; buf[i].a = buf[i].d; buf[i].a = tmp;
        tmp = buf[i].b; buf[i].b = buf[i].c; buf[i].b = tmp;
    }

    printf("Convert Endianness of input file \"%s\" to \"out.dat\"\n", argv[1]);
    fptr = fopen("out.dat", "w");
    fwrite(buf, 1, len, fptr);
    fclose(fptr);
    free(buf);

    return 0;
}
