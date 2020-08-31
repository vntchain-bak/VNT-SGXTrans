#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"

#include <string.h>
#include <map>
#include <string>
#include <time.h>

#define PACKAGESIZE 2048
#define REQUEST_READ 0
#define REQUEST_SEND 1

#define KEYSIZE 20
#define NUMSIZE 4
#define VALSIZE 20

/* read requst position*/
#define request_flag_pos(request) (request)
#define request_read_address_num_pos(request) (request + NUMSIZE)
#define request_read_address_pos(request, i) (request + NUMSIZE + i * KEYSIZE)


char* aes_key=NULL;

void ecall_handler_init()
{
    aes_key = (char *)malloc(32);
    sgx_read_rand((unsigned char *)aes_key, 32);
    oram_init();
}

void ecall_handle(char* request, char* result)
{
    char decrypted_request[PACKAGESIZE];
    aes_decrypt(request, aes_key, decrypted_request);
    
    int flag = *((int*)(request_flag_pos(decrypted_request)));
    
    if(flag == REQUEST_READ)
    {

    }
    else 
    {

    }

    char plain_result[PACKAGESIZE];
    aes_encrypt(plain_result, aes_key, result);
}

void handle_read_val(char* key, char* val, int* total_num)
{
    read_val(key,val,total_num);
}

void handle_write_val(char* key, char* val)
{
    write_val(key, val);
}

void handle_just(char* key)
{
    just_add_bitmap(key);
}

void ecall_test_handle()
{
    
    char key[21]="1111";
    char val[21]="1111";
    key[20] = '\0';
    val[20] = '\0';
    write_val(key,val);
    for(int i=0;i<1000;i++)
    {

        sgx_read_rand((unsigned char *)key, KEYSIZE);
        sgx_read_rand((unsigned char *)val, KEYSIZE);
        for(int j=0;j<20;j++)
        {
            key[j]=key[j]%10;
            val[j]=val[j]%10;
            key[j]+='0';
            val[j]+='0';
        }
        write_val(key,val);
    }
    char ret[2048];
    int num;
    read_val("1111",ret,&num);
    printf("read %d record(s). ",num);
}

