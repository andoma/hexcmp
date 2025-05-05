#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>



static void
print_diff(const uint8_t *a, const uint8_t *b, int len, uint32_t offset)
{
  uint8_t diff[len];
  printf("%08x  ", offset);

  int any_diff = 0;
  for(int i = 0; i < len; i++) {
    diff[i] = a[i] != b[i];
    any_diff |= diff[i];
  }

  for(int i = 0; i < len; i++) {
    if(i == 8)
      printf(" ");
    printf("%02x ", a[i]);
  }

  printf("|");
  for(int i = 0; i < len; i++) {
    char c = a[i];
    if(c < 32 || c > 126)
      c = '.';
    printf("%c", c);
  }
  printf("|");

  if(any_diff) {
    printf(" ");

    int bold = 0;

    for(int i = 0; i < len; i++) {

      if(!diff[i] && bold) {
        bold = 0;
        printf("\033[0m");
      }

      if(i > 0)
        printf("%s", i == 8 ? "  " : " ");

      if(diff[i] && !bold) {
        bold = 1;
        printf("\033[1m\033[4m");
      }


      printf("%02x", b[i]);
    }
    if(bold)
      printf("\033[0m");
  }
  printf("\n");
}



int
main(int argc, char **argv)
{
  uint8_t b1[1024];
  uint8_t b2[1024];

  if(argc != 3) {
    fprintf(stderr, "Usage: %s FILE1 FILE2\n",
            argv[0]);
    exit(1);
  }

  int fd1 = open(argv[1], O_RDONLY);
  if(fd1 == -1) {
    fprintf(stderr, "Unable to open %s -- %m\n", argv[1]);
    exit(1);
  }
  int fd2 = open(argv[2], O_RDONLY);
  if(fd2 == -1) {
    fprintf(stderr, "Unable to open %s -- %m\n", argv[2]);
    exit(1);
  }

  uint32_t offset = 0;

  while(1) {

    int r1 = read(fd1, b1, sizeof(b1));
    if(r1 < 0) {
      perror("read");
      exit(1);
    }

    int r2 = read(fd2, b2, sizeof(b2));
    if(r2 < 0) {
      perror("read");
      exit(1);
    }

    if(r1 == 0 && r2 == 0)
      return 0;

    int r = MIN(r1, r2);

    for(int i = 0; i < r; i += 16) {
      int csize = MIN(r - i, 16);
      print_diff(b1 + i, b2 + i, csize, offset + i);
    }
    offset += sizeof(b1);
  }
}
