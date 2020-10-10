#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"
#include <string>
#include <map>
#include <vector>


class raw_DataEntry
{
private:
    std::string entry_data; //存放所有的val
    std::string entry_key; 
    bool is_valid;
    int position; //该条目在页表中的具体位置，不一定与path相同
    int group; //该条目在同一key下的位次
    int entry_size; //由key_size、head_size和val_size组成，前两者在定义时确定
    static const int entry_max_size=1024; //之后设计为可修改
public:
    raw_DataEntry(char* raw_entry_data, int key_size)
    {
        is_valid = *(raw_entry_data);
        if (is_valid)
        {
            entry_size = key_size + 1;
            entry_key.append(raw_entry_data+1, key_size);
            group = *((int*)(raw_entry_data+1+key_size));
            entry_data=(raw_entry_data+1+key_size+8);
        }
    }
    std::string get_entry_val()
    {
        return entry_data;
    }
    char* to_raw_data()
    {

    }
    bool is_valid()
    {
        return is_valid;
    }
};

class DataEntry
{
private:
    std::string entry_key;
    std::string entry_val;

public:
    DataEntry(std::string entry_key ,std::vector<raw_DataEntry*> raw_DataEntry_list) {
        this->entry_val = "";
        this->entry_key = entry_key;
        for(auto raw_dataentry : raw_DataEntry_list) {
            this->entry_val+=raw_dataentry->get_entry_val();
        }
    }
};

class raw_DataBlock //用于写回时组织数据
{
private:
    std::vector<raw_DataEntry*> block;
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

    char* to_raw_data()
    {
        //
    }

    int add_entry(raw_DataEntry* entry)
    {
        if(this->is_not_full())
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
    std::map<std::string, std::vector<int>> bitmap;

    DataBitmap(std::string table_name)
    {
        read_bitmap_from_file(table_name, bitmap);
    }

    int get_entry_path(std::string key, int group)
    {
        return bitmap[key][group];
    }

};

class DataStack
{
    std::vector<raw_DataEntry*> stack;
    std::string table_name;
    int tree_level; 

    DataBitmap* data_bitmap;

    DataStack(std::string table_name, DataBitmap* data_bitmap, int tree_level)
    {
        this->table_name = table_name;
        this->data_bitmap = data_bitmap;
        this->tree_level = tree_level;
    }

    void read_entries(int path)
    {
        std::vector<std::vector<std::string>> raw_block_data;
        read_block_from_file(table_name, path, raw_block_data);
        for(auto raw_entry_data : raw_block_data)
        {
            raw_DataEntry* new_data_entry = new raw_DataEntry(raw_entry_data);
            stack.push_back(new_data_entry);
        }
    }

    void write_back_entries(int path, int entries_per_block)
    {
        reorder_entries();
        std::vector<raw_DataBlock*> write_back_blocks;
        for(int i=0;i<tree_level;++i)
        {
            raw_DataBlock* new_block = new raw_DataBlock(entries_per_block);
            write_back_blocks.push_back(new_block);
        }

        auto it=stack.begin();
        while(it!=stack.end())
        {
            DataEntry* entry = *it;
            int break_flag = 0;

            std::string entry_key = entry->get_key();
            int entry_group = entry->get_group();
            int entry_path = data_bitmap->get_entry_path(entry_key, entry_group);
            int block_pos = path;
            for(int i=0;i<tree_level;++i)
            {
                if(is_pos_fit_entry_path(entry_path, block_pos))
                {
                    write_back_blocks[i]->add_entry(entry);
                    it = stack.erase(it);
                    break_flag=1;
                    break;
                }
                block_pos = get_pos_parent(block_pos);
            }
            if(!break_flag) ++it;
        }

        //然后将write_back_blocks转变为raw data，写回

    }

    void reorder_entries()
    {
        for(int i = 0; i < stack.size(); ++i)
        {
            int new_idx = rand() % (stack.size() - i) + i;
            DataEntry* tmp = NULL;
            tmp = stack[i];
            stack[i] = stack[new_idx];
            stack[new_idx] = tmp;
        }
    }

    int get_pos_parent(int pos)
    {
        if(pos % 2) return (pos + 1) / 2;
        else return pos / 2;
    }

    bool is_pos_fit_entry_path(int entry_path, int target_pos)
    {
        while(entry_path > target_pos)
        {
            entry_path = get_pos_parent(entry_path);
        }
        return entry_path == target_pos;
    }

};


class DataTable
{
    std::string table_name;
    DataBitmap* data_bitmap;
    DataStack* data_stack;

    int tree_level; 

    int entries_per_block;

    // int last_path=0; //上一次读取时的路径叶子节点
    
    DataTable(std::string table_name)
    {
        this->table_name = table_name;
        data_val_list_creator = new DataValListCreator(table_name);
        data_bitmap = new DataBitmap(table_name);
        data_stack = new DataStack(table_name, data_val_list_creator, data_bitmap, tree_level);

    }

    void add_data_entry(std::vector<std::string> data)
    {
        DataEntry* new_data_entry = new DataEntry(data);
        // temp_stack.push_back(new_data_entry);
    }

}
