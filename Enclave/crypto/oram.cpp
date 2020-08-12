#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"
#include <string.h>
#include <string>
#include <map>
#include <vector>

#define BLOCKMAXSIZE 32768

#define KEYSIZE 20
#define NUMSIZE 4
#define VALSIZE 100
#define VALMAXNUM 20
#define STASHMAXSIZE 200

#define PAIRMAXNUM \
    (BLOCKMAXSIZE / (KEYSIZE + NUMSIZE))

#define ENTRYSIZE \
    (KEYSIZE + 2 * NUMSIZE + VALSIZE * VALMAXNUM)
#define ENTRYMAXNUM \
    ((BLOCKMAXSIZE - NUMSIZE) / ENTRYSIZE)

/* bitmap_pos list*/
// #define bitmap_pair_key_pos(i) (bitmap + NUMSIZE + i * (KEYSIZE + NUMSIZE))
// #define bitmap_pair_flag_pos(i) (bitmap + NUMSIZE + i * (KEYSIZE + NUMSIZE) + KEYSIZE)
/* entry_pos list */
#define entry_key_pos(entry) (entry)
// #define entry_flag_pos(entry) (entry + KEYSIZE)
#define entry_group_pos(entry) (entry + KEYSIZE )
#define entry_val_num_pos(entry) (entry + KEYSIZE + NUMSIZE)
#define entry_val_pos(entry, val_pos) (entry + KEYSIZE + 2 * NUMSIZE + val_pos * VALSIZE)
/* record_pos list */
#define record_entries_num_pos(record) (record)
#define record_entry_pos(record, i) (record + NUMSIZE + i * ENTRYSIZE)

#define BITMAPFILE "/sgx/db/bitmap.db"
#define DATAFILE "/sgx/db/data.db"

int bitmap_read = 0;
int tree_level = 2;
int pair_number = 0;
// char bitmap[BLOCKMAXSIZE]={'\0'};
std::map<std::string, std::vector<int>> bitmap;
std::map<std::string, std::string> pass;
// int records_filled[RECORDMAXNUM]={ 0 };
// int records_flag[RECORDMAXNUM]={ 0 };
// char records[RECORDMAXNUM][BLOCKMAXSIZE];
int stash_num = 0;
int entries_filled[STASHMAXSIZE] = {0};
char entries[STASHMAXSIZE][ENTRYSIZE];

char *oram_aes_key = NULL;


int rand()
{
    int ret;
    sgx_read_rand((unsigned char *)&ret, 4);
    if(ret<0) return -ret;
    return ret;
}

void just_add_bitmap(char* key)
{
    std::string key_string = std::string(key);
    int new_flag = rand() % (2 << (tree_level - 1) >> 1) + ((2 << (tree_level - 1) >> 1) + 1);
    bitmap[key_string].push_back(new_flag);
    pass[key_string] = "fjs19980712";
}
// int is_half_full(int number)
// {
//     return number >= (2 << (tree_level - 1));
// }

int adjust_entry_flag(int entry_flag)
{
    while (entry_flag <= (2 << (tree_level - 1) >> 1))
    {
        entry_flag = (entry_flag << 1) - (rand() % 2);
    }
    return entry_flag;
}

void resize()
{
    tree_level++;
    for (std::map<std::string, std::vector<int>>::iterator it = bitmap.begin(); it != bitmap.end(); ++it)
    {
        for (int i = 0; i < it->second.size(); i++)
        {
            it->second[i] = adjust_entry_flag(it->second[i]);
        }
    }
    for(int i=0;i<STASHMAXSIZE;i++)
    {
        if(entries_filled[i])
        {
            int entry_group = *((int *)(entry_group_pos(entries[i])));
            char entry_key[KEYSIZE + 1];
            memcpy(entry_key, entry_key_pos(entries[i]), KEYSIZE);
            entry_key[KEYSIZE] = '\0';
            std::string key_string = std::string(entry_key);
            bitmap[key_string][entry_group] = adjust_entry_flag(bitmap[key_string][entry_group]);
        }
    }
    char null_data[BLOCKMAXSIZE] = {'\0'};
    memset(null_data, 0, BLOCKMAXSIZE);
    char encrypted_null_data[BLOCKMAXSIZE] = {'\0'};
    aes_encrypt(null_data, oram_aes_key, encrypted_null_data);
    for (int i = ((2 << (tree_level - 2)) - 1); i < ((2 << (tree_level - 1)) - 1); i++)
        ocall_write_to_file(DATAFILE, encrypted_null_data, i, BLOCKMAXSIZE);
}

// void read_bitmap()
// {
//     // read bitmap
//     char encrypted_bitmap[BLOCKMAXSIZE];
//     ocall_read_to_buffer(BITMAPFILE, encrypted_bitmap, 0, BLOCKMAXSIZE);
//     /*
//     tree level: NUMSIZE |pair number: NUMSIZE | [pairs]: pair size * pair number
//     bitmap pair:
//     {
//         address: KEYSIZE,
//         leafnode: NUMSIZE
//     }
//     */
//     aes_decrypt(encrypted_bitmap, oram_aes_key, bitmap);
//     // tree_level = *((int *)bitmap);
//     pair_number = *((int *)(bitmap + NUMSIZE));
// }

void read_entries_on_path(int flag)
{
    while (true)
    {
        int offset = flag - 2;
        char encrypted_record[BLOCKMAXSIZE];
        char record[BLOCKMAXSIZE];
        ocall_read_to_buffer(DATAFILE, encrypted_record, offset, BLOCKMAXSIZE);
        /*
        entry number: NUMSIZE | [entries]: entry size * entry number
        entries in record:
        {
            address: KEYSIZE,
            flag: NUMSIZE,
            group: NUMSIZE,
            val_number: NUMSIZE,
            val: [VALSIZE]
        }
        */
        aes_decrypt(encrypted_record, oram_aes_key, record);
        int entries_num = *((int *)(record_entries_num_pos(record)));
        for (int i = 0; i < entries_num; i++)
        {
            int num = 0;
            while (entries_filled[num]&&num<STASHMAXSIZE-1)
                ++num;
            memcpy(entries[num], record_entry_pos(record,i),ENTRYSIZE);
            entries_filled[num] = 1;
            ++stash_num;
        }
        if (flag > 2)
        {
            if (flag % 2)
                flag = (flag + 1) / 2;
            else
                flag /= 2;
        }
        else
            break;
    }
}

void swap_entries(int a, int b)
{
    if (!entries_filled[a] && !entries_filled[b]) return;
    char tmp[ENTRYSIZE];
    memcpy(tmp, entries[a], ENTRYSIZE);
    memcpy(entries[a], entries[b], ENTRYSIZE);
    memcpy(entries[b], tmp, ENTRYSIZE);
    int tmp_flag=entries_filled[a];
    entries_filled[a]=entries_filled[b];
    entries_filled[b]=tmp_flag;
}

char *get_entry(char *key, int group)
{
    for (int i = 0; i < STASHMAXSIZE; i++)
    {
        if (entries_filled[i] && !strncmp(entry_key_pos(entries[i]), key, KEYSIZE) && group == *(int *)(entry_group_pos(entries[i]))) //遍历，找到key和group对应的entry
        {
            return entries[i];
        }
    }
    return NULL;
}

void write_entries_to_path(int flag)
{
    for (int i = 0; i < STASHMAXSIZE - 1; i++)
    {
        int new_idx = rand() % (STASHMAXSIZE - i) + i;
        swap_entries(i, new_idx);
    }

    char *write_back = (char *)malloc(tree_level * BLOCKMAXSIZE);
    memset(write_back, '\0', tree_level * BLOCKMAXSIZE);
    for (int i = 0; i < STASHMAXSIZE; i++)
    {
        if (entries_filled[i])
        {
            // int entry_flag = *((int *)(entry_flag_pos(entries[i])));
            int entry_group = *((int *)(entry_group_pos(entries[i])));
            char entry_key[KEYSIZE+1];
            memcpy(entry_key, entry_key_pos(entries[i]), KEYSIZE);
            entry_key[KEYSIZE]='\0';
            std::string key_string = std::string(entry_key);
            int entry_flag = bitmap[key_string][entry_group];
            // entry_flag = adjust_entry_flag(entry_flag);
            // memcpy(entry_flag_pos(entries[i]), &entry_flag, NUMSIZE);
            int current_node = flag;
            for (int j = 0; j < tree_level; j++)
            {
                int left = (current_node - 1) * (2 << j >> 1) + 1;
                int right = current_node * (2 << j >> 1);
                if (entry_flag >= left && entry_flag <= right) // 如果在对应路径上
                {
                    int entries_num = *((int *)(write_back + j * BLOCKMAXSIZE));
                    if (entries_num < ENTRYMAXNUM) //如果该节点上有剩余空位
                    {
                        memcpy(record_entry_pos(write_back + j * BLOCKMAXSIZE, entries_num), entries[i], ENTRYSIZE);
                        ++entries_num;
                        memcpy(record_entries_num_pos(write_back + j * BLOCKMAXSIZE), &entries_num, NUMSIZE);
                        entries_filled[i] = 0;
                        --stash_num;
                        break;
                    }
                }
                if (current_node % 2)
                    current_node = (current_node + 1) / 2;
                else
                    current_node /= 2;
            }
        }
    }
    for (int i = 0; i < tree_level; i++)
    {
        int offset = flag - 2;
        char encrypted_record[BLOCKMAXSIZE];
        aes_encrypt(write_back + i * BLOCKMAXSIZE, oram_aes_key, encrypted_record);
        ocall_write_to_file(DATAFILE, encrypted_record, offset, BLOCKMAXSIZE);
        if (flag > 2)
        {
            if (flag % 2)
                flag = (flag + 1) / 2;
            else
                flag /= 2;
        }
    }
    free(write_back);
    if (stash_num + tree_level * ENTRYMAXNUM >= STASHMAXSIZE || pair_number > ENTRYMAXNUM * ((2 << (tree_level - 1)) - 1) / 2)
        resize();
}

int read_val(char *key, char *val, int *total_num)
{

    int success = 0;
    char ret_val[BLOCKMAXSIZE] = {'\0'};
    *total_num = 0;
    // if (!bitmap_read)
    // {
    //     read_bitmap();
    //     bitmap_read = 1;
    // }
    std::string key_string = std::string(key);
    // for (int i = 0; i < pair_number; i++)
    // {
        // if (!strncmp(bitmap_pair_key_pos(i), key, KEYSIZE)) //查找bitmap中对应key
        if (bitmap.find(key_string)!=bitmap.end()) //查找bitmap中对应key
        {
            // int flag = *((int *)(bitmap_pair_flag_pos(i))); //如果存在，记录该值
            for(int i=0;i<bitmap[key_string].size();++i)
            {
                int flag = bitmap[key_string][i];
                int new_flag = rand() % (2 << (tree_level - 1) >> 1) + ((2 << (tree_level - 1) >> 1) + 1);
                read_entries_on_path(flag);                        //如果存在，则需要将对应路径上的数据块全部读入
                char *entry = get_entry(key, i);             //找到对应的entry
                bitmap[key_string][i]=new_flag;
                // memcpy(entry_flag_pos(entry), &new_flag, NUMSIZE); //修改flag
                int val_num = *((int *)(entry_val_num_pos(entry)));
                memcpy(ret_val + (*total_num) * VALSIZE, entry_val_pos(entry, 0), val_num * VALSIZE); //将所有val汇总
                *total_num += val_num;
                write_entries_to_path(flag);
            }
            memcpy(val, ret_val, BLOCKMAXSIZE);
            success = 1;
        }
        
    // }
    return success;
}

int write_val(char *key, char *val)
{
    int success = 0;
    // if (!bitmap_read)
    // {
    //     read_bitmap();
    //     bitmap_read = 1;
    // }
    // int flag = -1;
    // int group = -1;
    std::string key_string = std::string(key);
    // for (int i = 0; i < pair_number; i++)
    // {
        // if (!strncmp(bitmap_pair_key_pos(i), key, KEYSIZE)) //查找bitmap中是否已经存在对应key
        if (bitmap.find(key_string) != bitmap.end()) //查找bitmap中对应key
        {
            for (int i = 0; i < bitmap[key_string].size(); ++i)
            {
                // int flag = *((int *)(bitmap_pair_flag_pos(i))); //如果存在，记录该值
                int flag = bitmap[key_string][i];
                int new_flag = rand() % (2 << (tree_level - 1) >> 1) + ((2 << (tree_level - 1) >> 1) + 1);
                read_entries_on_path(flag);            //如果存在，则需要将对应路径上的数据块全部读入
                char *entry = get_entry(key, i); //找到对应的entry
                int val_num = *((int *)(entry_val_num_pos(entry)));
                if (val_num < VALMAXNUM)
                {
                    memcpy(entry_val_pos(entry, val_num), val, VALSIZE); //在正确位置写入val
                    ++val_num;
                    memcpy(entry_val_num_pos(entry), &val_num, NUMSIZE);
                    bitmap[key_string][i] = new_flag;
                    // memcpy(entry_flag_pos(entry), &new_flag, NUMSIZE);   //更新flag
                    write_entries_to_path(flag);                         //将数据写回
                    success = 1;                                         //写入成功
                    break;                                               //写入只需要一次
                }
                else
                {
                    // group = *((int *)(entry_group_pos(entry)));
                    write_entries_to_path(flag);
                    // flag = -1; //寻找下一个
                }
            }
        }
    // }
    // ++group;
    if(!success)
    {
        int new_flag = rand() % (2 << (tree_level - 1) >> 1) + ((2 << (tree_level - 1) >> 1) + 1);
        bitmap[key_string].push_back(new_flag);
        // memcpy(bitmap_pair_key_pos(pair_number), key, KEYSIZE);        //如果不存在对应key的记录，首先在bitmap上新增一个
        // memcpy(bitmap_pair_flag_pos(pair_number), &new_flag, NUMSIZE); //随机给予flag
        for (int i = 0; i < STASHMAXSIZE; i++)
        {
            if (!entries_filled[i]) //找到一个空位置，写入所有信息
            {
                int new_val_num = 1;
                int group = bitmap[key_string].size()-1;
                memcpy(entry_key_pos(entries[i]), key, KEYSIZE);
                // memcpy(entry_flag_pos(entries[i]), &new_flag, NUMSIZE);
                memcpy(entry_group_pos(entries[i]), &group, NUMSIZE);
                memcpy(entry_val_num_pos(entries[i]), &new_val_num, NUMSIZE);
                memcpy(entry_val_pos(entries[i], 0), val, VALSIZE);
                entries_filled[i]=1;
                ++stash_num;
                ++pair_number;
                read_entries_on_path(new_flag);
                write_entries_to_path(new_flag);
                success = 1; //写入成功
                break;
            }
        }
        if (!success)
            printf("error");
        // if(is_half_full(pair_number)) resize();
    }
    return success;
}

void oram_init()
{
    oram_aes_key = (char *)malloc(32);
    sgx_read_rand((unsigned char *)oram_aes_key, 32);
    // tree_level = 6;
    char null_data[BLOCKMAXSIZE]={'\0'};
    memset(null_data, 0, BLOCKMAXSIZE);
    char encrypted_null_data[BLOCKMAXSIZE]={'\0'};
    aes_encrypt(null_data, oram_aes_key, encrypted_null_data);
    for(int i=0;i< ((2<<(tree_level-1))-1); i++)
        ocall_write_to_file(DATAFILE, encrypted_null_data, i, BLOCKMAXSIZE);
    // ocall_write_to_file(BITMAPFILE, encrypted_null_data, 0, BLOCKMAXSIZE);
    // read_bitmap();
}
