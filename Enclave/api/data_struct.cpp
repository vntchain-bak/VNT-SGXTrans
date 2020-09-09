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
    int val_size;

    int val_num;

    DataVal(int val_name, int val_type, int val_size)
    {
        this.val_name = val_name;
        this.val_type = val_type;
        this.val_size = val_size;
        this.val_num = 0;
    }

    int add(std::string val) //返回增加的val长度
    {
        val_list.push_back(val);
        val_num++;
        return val_size;
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

    int create(std::vector<std::string> raw_entry_data, std::vector<DataVal*>& entry_data)
    {
        int start = val_name_num;
        int total_size=0;

        for(int i=0;i<val_name_num;++i)
        {
            int val_num = std::stoi(raw_entry_data[i]);
            int val_size = this.get_val_size(val_type[i]);

            DataVal* new_data_val = new DataVal(val_name[i], val_type[i]);
            for(int j=0;j<val_num;++j)
            {
                total_size += new_data_val->add(raw_entry_data[start+j]);
            }
            entry_data.push_back(new_data_val);
            start+=val_num;
        }
        return total_size;
    }

    int get_val_size(int type)
    {
        //返回根据type类型预设的长度值
    }

};

class DataEntry 
{
    DataValListCreator* data_val_list_creator;
    std::vector<DataVal*> entry_data; //存放所有的val
    std::string key; 
    int position; //该条目在页表中的具体位置，不一定与path相同
    int group; //该条目在同一key下的位次
    int entry_size; //由key_size、head_size和val_size组成，前两者在定义时确定
    int entry_max_size=1024; //之后设计为可修改
    
    DataEntry(std::vector<std::string> raw_entry_data, DataValListCreator* data_val_list_creator)
    {
        this.data_val_list_creator=data_val_list_creator;
        int key_size = 12;//之后设计为可修改
        int head_size = 16;//基本固定
        entry_size = key_size + head_size;
        this.entry_size+=data_val_list_creator->create(raw_entry_data, entry_data);
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
            {
                entry_size+=data_val->add(new_val);
                break;
            }
        }
        if(entry_size>entry_max_size)
        {
            //新增group
        }
    }

    std::vector<std::string> to_raw_data()
    {
        //
    }

};

class DataBlock //用于写回时组织数据
{
    std::vector<DataEntry*> block;
    int entries_num;
    int max_entries_num;

    DataBlock(int max_entries_num)
    {
        this.max_entries_num = max_entries_num;
    }

    int is_not_full()
    {
        return entries_num < max_entries_num;
    }

    char* to_raw_data()
    {
        //
    }

    int add_entry(DataEntry* entry)
    {
        if(this.is_not_full())
        {
            block.push_back(entry);
            entries_num++;
            return 0;
        }
        return 1;
    }

    
}

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
    int tree_level; 

    DataValListCreator* data_val_list_creator;
    DataBitmap* data_bitmap;

    DataStack(std::string table_name, DataValListCreator* data_val_list_creator, DataBitmap* data_bitmap, int tree_level)
    {
        this.table_name = table_name;
        this.data_val_list_creator = data_val_list_creator;
        this.data_bitmap = data_bitmap;
        this.tree_level = tree_level;
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

    void write_back_entries(int path, int entries_per_block)
    {
        reorder_entries();
        std::vector<DataBlock*> write_back_blocks;
        for(int i=0;i<tree_level;++i)
        {
            DataBlock* new_block = new DataBlock(entries_per_block);
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

}


class DataTable
{
    std::string table_name;
    DataValListCreator* data_val_list_creator;
    DataBitmap* data_bitmap;
    DataStack* data_stack;

    int tree_level; 

    int entries_per_block;

    // int last_path=0; //上一次读取时的路径叶子节点
    
    DataTable(std::string table_name)
    {
        this.table_name = table_name;
        data_val_list_creator = new DataValListCreator(table_name);
        data_bitmap = new DataBitmap(table_name);
        data_stack = new DataStack(table_name, data_val_list_creator, data_bitmap, tree_level);

    }

    add_data_entry(std::vector<std::string> data)
    {
        DataEntry* new_data_entry = new DataEntry(data);
        temp_stack.push_back(new_data_entry);
    }

}
