#include <iostream.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
   int fd;
   if ((fd = open(argv[1], O_RDONLY)) < 0){
      perror("cant openfile ");
      exit(errno);
   }
   long flen = lseek(fd, 0, SEEK_END);
   int *ary;
#ifdef SGI
   ary = (int *) mmap((char *)NULL, flen,
                          (PROT_WRITE|PROT_READ),
                          MAP_PRIVATE, fd, 0);
#else
   ary = (int *) mmap((char *)NULL, flen,
                          (PROT_WRITE|PROT_READ),
                          (MAP_FILE|MAP_VARIABLE|MAP_PRIVATE), fd, 0);
#endif
   if (ary == (int *)-1){
      perror("MMAP ERROR");
      exit(errno);
   }
   for (int i=0; i < flen/sizeof(int); i++)
      cout << " " << ary[i];
   cout << endl;

   munmap((caddr_t)ary, flen);
   close(fd);
   
}
