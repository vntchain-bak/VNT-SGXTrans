#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"

#include <string.h>

#include "e_os.h"

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/pem.h>

#include <openssl/rsa.h>

char publicKey[2048];
char privateKey[2048];
size_t pri_len;
size_t pub_len;
#define KEY_LENGTH 2048 // 密钥长度

void generateRSAKey()
{
    // 公私密钥对
    char *pri_key = NULL;
    char *pub_key = NULL;

    // 生成密钥对
    RSA *keypair = RSA_generate_key(KEY_LENGTH, RSA_3, NULL, NULL);

    BIO *pri = BIO_new(BIO_s_mem());
    BIO *pub = BIO_new(BIO_s_mem());

    PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
    PEM_write_bio_RSAPublicKey(pub, keypair);

    // 获取长度
    pri_len = BIO_ctrl_pending(pri);
    pub_len = BIO_ctrl_pending(pub);

    // // 密钥对读取到字符串
    pri_key = (char *)malloc(pri_len + 1);
    pub_key = (char *)malloc(pub_len + 1);

    BIO_read(pri, pri_key, pri_len);
    BIO_read(pub, pub_key, pub_len);

    pri_key[pri_len] = '\0';
    pub_key[pub_len] = '\0';

    // // 存储密钥对
    strncpy(publicKey, pub_key, pub_len+1);
    strncpy(privateKey, pri_key, pri_len+1);

    // 内存释放
    RSA_free(keypair);
    BIO_free(pri);
    BIO_free(pub);

    free(pri_key);
    free(pub_key);
}

void getRSAKey(char* pub, char* pri)
{
    strncpy(pub, publicKey, pub_len + 1);
    strncpy(pri, privateKey, pri_len + 1);
}

void rsa_pub_encrypt(char* etext,const char* ctext, const char* pub)  
{  
    BIO *keybio = BIO_new_mem_buf(pub, -1);  
, NULL, NULL, NULL);  
  
    int len = RSA_size(rsa);   
    memset(etext, 0, len + 1);  
  
    // 加密函数
    int ret = RSA_public_encrypt(len-11, (unsigned char *)ctext, (unsigned char *)etext, rsa, RSA_PKCS1_PADDING);
  
    // 释放内存  
    BIO_free_all(keybio);  
    RSA_free(rsa);  
  
    return;
}  
  
// 私钥解密    
void rsa_pri_decrypt(char* ctext, const char* etext, const char* pri)  
{   
    BIO *keybio = BIO_new_mem_buf(pri, -1);  

    RSA* rsa = PEM_read_bio_RSAPrivateKey(keybio, NULL, NULL, NULL);  
  
    int len = RSA_size(rsa);  
    memset(ctext, 0, len + 1);  
    // 解密函数  
    int ret = RSA_private_decrypt(len, (unsigned char*)etext, (unsigned char*)ctext, rsa, RSA_PKCS1_PADDING);  

  
    // 释放内存    
    BIO_free_all(keybio);  
    RSA_free(rsa);  
  
    return;  
}