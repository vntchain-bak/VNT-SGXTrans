#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"

class DataEntryFactory
{

public:
    DataEntryFactory(int key_size)
    {
    }

    create_raw_DataEntry()
    {
    }

    create_DataEntry()
};

class raw_DataEntry
{
private:
    std::string entry_data; //存放所有的val
    std::string entry_key;
    bool valid;
    int position;                                //该条目在页表中的具体位置，不一定与path相同
    int group;                                   //该条目在同一key下的位次
    int entry_size;                              //由key_size、head_size和val_size组成，前两者在定义时确定
    static const int entry_max_size = EntrySize; //之后设计为可修改
public:
    raw_DataEntry(char *raw_entry_data, int key_size)
    {
        valid = *(raw_entry_data);
        if (valid)
        {
            entry_size = key_size + 1;
            entry_key.append(raw_entry_data + 1, key_size);
            group = *((int *)(raw_entry_data + 1 + key_size));
            entry_data = (raw_entry_data + 1 + key_size + 8);
        }
    }
    std::string get_entry_val()
    {
        return entry_data;
    }

    std::string get_key()
    {
        return entry_key;
    }

    int get_group()
    {
        return group;
    }

    char *to_raw_data()
    {
    }
    bool is_valid()
    {
        return valid;
    }
};

class DataEntry
{
private:
    std::string entry_key;
    SGX_JSON *entry_val;

public:
    DataEntry(std::string entry_key, std::vector<raw_DataEntry *> raw_DataEntry_list)
    {
        std::string val;
        this->entry_key = entry_key;
        for (auto raw_dataentry : raw_DataEntry_list)
        {
            val += raw_dataentry->get_entry_val();
        }
        entry_val = new SGX_JSON(val);
    }

    bool append_val(std::string append)
    {
        entry_val.append_to_arr("address", append);
    }

    std::vector<raw_DataEntry *> to_raw_DataEntry()
    {
        std::vector<raw_DataEntry *> ret;
        // std::vector<std::string> val_list = entry_val.to_string();
    }

    ~DataEntry()
    {
        delete entry_val;
    }
};

class raw_DataBlock //用于写回时组织数据
{
private:
    std::vector<raw_DataEntry *> block;
    int entries_num;
    int max_entries_num;

public:
    raw_DataBlock(int max_entries_num)
    {
        this->max_entries_num = max_entries_num;
    }

    int is_not_full()
    {
        return entries_num < max_entries_num;
    }

    char *to_raw_data()
    {
        //
    }

    int add_entry(raw_DataEntry *entry)
    {
        if (this->is_not_full())
        {
            block.push_back(entry);
            entries_num++;
            return 0;
        }
        return 1;
    }
};

class DataBitmap
{
private:
    std::map<std::string, std::vector<int>> bitmap;

public:
    DataBitmap(std::string table_name)
    {
        // read_bitmap_from_file(table_name, bitmap);
    }
    int get_entry_path(raw_DataEntry *entry)
    {
        return bitmap[entry->get_key()][entry->get_group()];
    }
};

class DataStack
{
private:
    std::vector<raw_DataEntry *> stack;
    std::string table_name;
    int tree_level;
    int key_size;

    DataBitmap *data_bitmap;

    int rand()
    {
        int ret;
        sgx_read_rand((unsigned char *)&ret, 4);
        if (ret < 0)
            return -ret;
        return ret;
    }

    void reorder_entries()
    {
        for (int i = 0; i < stack.size(); ++i)
        {
            int new_idx = rand() % (stack.size() - i) + i;
            raw_DataEntry *tmp = NULL;
            tmp = stack[i];
            stack[i] = stack[new_idx];
            stack[new_idx] = tmp;
        }
    }

    int get_pos_parent(int pos)
    {
        if (pos % 2)
            return (pos + 1) / 2;
        else
            return pos / 2;
    }

    bool is_pos_fit_entry_path(int entry_path, int target_pos)
    {
        while (entry_path > target_pos)
        {
            entry_path = get_pos_parent(entry_path);
        }
        return entry_path == target_pos;
    }

public:
    DataStack(std::string table_name, DataBitmap *data_bitmap, int tree_level, int key_size)
    {
        this->table_name = table_name;
        this->data_bitmap = data_bitmap;
        this->tree_level = tree_level;
        this->key_size = key_size;
    }

    void add_entry(raw_DataEntry *entry)
    {
        stack.push_back(entry);
    }

    void read_entries(int path)
    {
        char raw_block_data[BlockSize];
        read_block_from_file(table_name, path, raw_block_data);
        for (int i = 0; i < EntryNum; ++i)
        {
            char *raw_entry_data = raw_block_data + i * EntrySize;
            raw_DataEntry *new_data_entry = new raw_DataEntry(raw_entry_data, key_size);
            stack.push_back(new_data_entry);
        }
    }

    void write_back_entries(int path, int entries_per_block)
    {
        reorder_entries();
        std::vector<raw_DataBlock *> write_back_blocks;
        for (int i = 0; i < tree_level; ++i)
        {
            raw_DataBlock *new_block = new raw_DataBlock(entries_per_block);
            write_back_blocks.push_back(new_block);
        }

        auto it = stack.begin();
        while (it != stack.end())
        {
            raw_DataEntry *entry = *it;
            int break_flag = 0;

            int entry_path = data_bitmap->get_entry_path(entry);
            int block_pos = path;
            for (int i = 0; i < tree_level; ++i)
            {
                if (is_pos_fit_entry_path(entry_path, block_pos))
                {
                    write_back_blocks[i]->add_entry(entry);
                    it = stack.erase(it);
                    break_flag = 1;
                    break;
                }
                block_pos = get_pos_parent(block_pos);
            }
            if (!break_flag)
                ++it;
        }

        //然后将write_back_blocks转变为raw data，写回
    }
};

class DataTable
{
private:
    std::string table_name;
    DataBitmap *data_bitmap;
    DataStack *data_stack;

    int tree_level;
    int key_size;

    // int last_path=0; //上一次读取时的路径叶子节点
public:
    DataTable(std::string table_name, int key_size = 16)
    {
        this->table_name = table_name;
        this->key_size = key_size;
        data_bitmap = new DataBitmap(table_name);
        data_stack = new DataStack(table_name, data_bitmap, tree_level, key_size);
    }

    void add_data_entry(char *data)
    {
        raw_DataEntry *new_data_entry = new raw_DataEntry(data, key_size);
        data_stack->add_entry(new_data_entry);
    }

    void update_entry(std::string entry_key, std::string val)
    {
    }
};
