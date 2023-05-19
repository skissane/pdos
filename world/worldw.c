/* example of doing a direct windows call */

#include <stdio.h>

#include <windows.h>

int main(void)
{
    char buf[200];

    printf("hello, world\n");
    GetCurrentDirectory(sizeof buf, buf);
    printf("current directory is %s\n", buf);
    return (0);
}
