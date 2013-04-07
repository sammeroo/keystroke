
#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>

int main()
{
DIR *dir;
FILE *l;

struct dirent *ent;
    dir = opendir("./");
    l=fopen("list.txt","w");
    printf("hi");
if (dir != NULL) {
  printf("hi");
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    fprintf (l,"%s\n", ent->d_name);
  }
  fclose(l);
  closedir (dir);
} else {
  /* could not open directory */
  perror ("");
  return EXIT_FAILURE;
}

}




