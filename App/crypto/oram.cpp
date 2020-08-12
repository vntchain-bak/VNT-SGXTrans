#include "../App.h"
#include "Enclave_u.h"
#include <fcntl.h>
#include <unistd.h>

void ocall_read_to_buffer(char* filename, char* buffer, int offset, int size)
{
    int fd, n;
    int i;
    fd = open(filename, O_RDONLY);
    i = lseek(fd, size * offset, SEEK_SET);
    n = read(fd, buffer, size);
    close(fd);
}

void ocall_write_to_file(char* filename, char* data, int offset, int size)
{
    int fd, n;
    int i;
    fd = open(filename, O_WRONLY);
    if(fd<0)
    {
        fd = creat(filename, S_IRWXU);
    }
    i = lseek(fd, size * offset, SEEK_SET);
    n = write(fd, data, size);
    close(fd);
}