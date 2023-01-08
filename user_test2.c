/** user_test2.c
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int test_read()
{
    printf("-----------------------READ------------------------------\n");
    
    char *c = (char *) calloc(100, sizeof(char));

    int fd = open("/proc/mytaskinfo", O_RDONLY);

    if (fd < 0) 
    {
        perror("r1"); 
        exit(1); 
    }

    int sz;
    while ((sz = read(fd, c, 100)) > 0)
    {
        printf("%s\n",c);
    }

    c[sz] = '\0';
}
int test_write()
{

    printf("-----------------------WRITE------------------------------\n");

    int fd = open("/proc/mytaskinfo", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0)
    {   
        perror("r1");
        exit(1);
    }

    char *buf = "10";

    int w = write(fd, buf, 3);

    printf("You'll see %s process.\n", buf);
    close(fd);

}

int main()
{
    test_write();
    test_read();
}