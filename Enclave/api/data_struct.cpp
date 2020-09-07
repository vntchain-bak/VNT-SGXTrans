#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"
#include <string>
#include <map>
#include <vector>

class DataVal
{
    std::vector<std::string> val;
    int val_name;
    int val_type;
    int val_num;

    DataVal(int val_name, int val_type)
    {
        this.val_name = val_name;
        this.val_type = val_type;
        this.val_num = 0;
    }

    void add(std::string val)
    {
        val_list.push_back(val);
        val_num++;
    }

    std::vector<std::string> to_raw_data()
    {
        //
    }
};

class DataValListCreator
{
    int val_name_num;
    std::vector<std::string> val_name;
    std::vector<int> val_type;

    DataValListCreator(std::string table_name)
    {
        std::string tmp = read_data_val_from_file(table_name);
        //拆分tmp获取对应数据
    }

    void create(std::vector<std::string> raw_entry_data, std::vector<DataVal*>& entry_data)
    {
        int start = val_name_num;
        for(int i=0;i<val_name_num;++i)
        {
            int val_num = std::stoi(raw_entry_data[i]);
            DataVal* new_data_val = new DataVal(val_name[i], val_type[i]);
            for(int j=0;j<val_num;++j)
            {
                new_data_val->add(raw_entry_data[start+j]);
            }
            entry_data.push_back(new_data_val);
            start+=val_num;
        }
    }

};

class DataEntry 
{
    DataValListCreator* data_val_list_creator;
    std::vector<DataVal*> entry_data; //存放所有的val
    std::string key; 
    int position; //该条目在页表中的具体位置，不一定与path相同
    int group; //该条目在同一key下的位次
    
    DataEntry(std::vector<std::string> raw_entry_data, DataValListCreator* data_val_list_creator)
    {
        this.data_val_list_creator=data_val_list_creator;
        data_val_list_creator->create(raw_entry_data, entry_data);
    }

    std::string get_key()
    {
        return key;
    }

    int get_group()
    {
        return group;
    }

    void add_val(std::string val_name, std::string new_val)
    {
        for(auto data_val: total_data)
        {
            if(data_val->val_name == val_name)
                data_val->add(new_val);
        }
    }

    std::vector<std::string> to_raw_data()
    {
        //
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
    std::vector<DataEntry*> stack;
    std::string table_name;
    int tree_level; //需要初始化

    DataValListCreator* data_val_list_creator;
    DataBitmap* data_bitmap;

    DataStack(std::string table_name, DataValListCreator* data_val_list_creator, DataBitmap* data_bitmap)
    {
        this.table_name = table_name;
        this.data_val_list_creator = data_val_list_creator;
        this.data_bitmap = data_bitmap;
    }

    void read_entries(int path)
    {
        std::vector<std::vector<std::string>> raw_block_data;
        read_block_from_file(table_name, path, raw_block_data);
        for(auto raw_entry_data : raw_block_data)
        {
            DataEntry* new_data_entry = new DataEntry(raw_entry_data, data_val_list_creator);
            stack.push_back(new_data_entry);
        }
    }

    void write_back_entries(int path)
    {
        reorder_entries();

        for(auto entry: stack)
        {
            std::string entry_key = entry->get_key();
            int entry_group = entry->get_group();
            int entry_path = data_bitmap->get_entry_path(entry_key, entry_group);
            int block_pos = path;
            while(block_pos!=2)
            {
                if(is_pos_fit_entry_path(entry_path, block_pos))
                {
                    //变为raw_data，写回
                }

            }
        }
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

    bool is_pos_fit_entry_path(int entry_path, int target_pos)
    {
        while(entry_path > target_pos)
        {
            if(entry_path % 2) entry_path = (entry_path + 1) / 2;
            else entry_path /=2;
        }
        return entry_path == target_pos;
    }

}


class DataTable
{
    std::string table_name;
    DataValListCreator* data_val_list_creator;
    DataBitmap* data_bitmap;
    DataStack* data_stack;

    // int last_path=0; //上一次读取时的路径叶子节点
    
    DataTable(std::string table_name)
    {
        this.table_name = table_name;
        data_val_list_creator = new DataValListCreator(table_name);
        data_bitmap = new DataBitmap(table_name);
        data_stack = new DataStack(table_name, data_val_list_creator, data_bitmap);

    }

    add_data_entry(std::vector<std::string> data)
    {
        DataEntry* new_data_entry = new DataEntry(data);
        temp_stack.push_back(new_data_entry);
    }

}
