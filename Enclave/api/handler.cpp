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
    // std::map<std::string, int> test;
    // test.insert(std::pair<std::string, int>("123\000456",1));
    // test["123"]=2;
    // printf("test: %d\n",test.size());
    // char key1[KEYSIZE]="1111k";
    // char key2[KEYSIZE]="2222k";
    // char val1[VALSIZE]="1111y";
    // char val2[VALSIZE]="2222y";
    // write_val(key1, val1);
    // write_val(key2, val2);
    // write_val(key1, val2);
    // char val[2048]={'\0'};
    // int num;
    // read_val(key1,val,&num);
    // printf("%d\n",num);
    // read_val(key2, val, &num);
    // printf("%d\n", num);
    char key[21]="1111";
    char val[21]="1111";
    key[20] = '\0';
    val[20] = '\0';
    write_val(key,val);
    // clock_t total_time=0;
    for(int i=0;i<1000;i++)
    {
        // std::string k="kkkk";
        // k+=(char)('0'+i);
        // std::string v="vvvv";
        // v+=(char)('0'+i);
        // memcpy(key, k.c_str(), k.length()+1);
        // memcpy(val, v.c_str(), v.length()+1);
        sgx_read_rand((unsigned char *)key, KEYSIZE);
        sgx_read_rand((unsigned char *)val, KEYSIZE);
        for(int j=0;j<20;j++)
        {
            key[j]=key[j]%10;
            val[j]=val[j]%10;
            key[j]+='0';
            val[j]+='0';
        }
        // clock_t begin = clock();
        write_val(key,val);
        // clock_t end = clock();
        // total_time+=(end-begin);
    }
    char ret[2048];
    int num;
    read_val("1111",ret,&num);
    // read_val("kkkk9", ret, &num);
    printf("read %d record(s). ",num);
}

