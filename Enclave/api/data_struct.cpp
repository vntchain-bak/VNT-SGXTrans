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

    void create(std::vector<std::string> plain_entry, std::vector<DataVal*>& data_entry)
    {
        int start = val_name_num;
        for(int i=0;i<val_name_num;++i)
        {
            int val_num = std::stoi(plain_entry[i]);
            DataVal* new_data_val = new DataVal(val_name[i], val_type[i]);
            for(int j=0;j<val_num;++j)
            {
                new_data_val->add(plain_entry[start+j]);
            }
            data_entry.push_back(new_data_val);
            start+=val_num;
        }
    }

};

class DataEntry 
{
    DataValListCreator* data_val_list_creator;
    std::vector<DataVal*> total_data;
    
    DataEntry(std::vector<std::string> entry, DataValListCreator* data_val_list_creator)
    {
        this.data_val_list_creator=data_val_list_creator;
        data_val_list_creator->create(entry, data);

    }

    void add_val(std::string val_name, std::string new_val)
    {
        for(auto data_val: total_data)
        {
            if(data_val->val_name == val_name)
                data_val->add(new_val);
        }
    }

};

class DataBlock
{
    std::vector<DataEntry*> entries;

    DataBlock(std::string table_name, int block_num)
    {
        read_block_from_file(table_name, block_num, entries);
    }

    DataEntry get_data_entry(int entry_num)
    {
        return entries[entry_num];
    }

};

class DataBitmap
{
    std::map<std::string, std::vector<int>> bitmap;

    DataBitmap(std::string table_name)
    {
        read_bitmap_from_file(table_name, bitmap);
    }

};


class DataTable
{
    std::string table_name;
    DataVal* data_val;
    DataBitmap* data_bitmap;

    int last_path=0; //上一次读取时的路径叶子节点
    std::vector<DataEntry*> temp_stack;
    

    DataTable(std::string table_name)
    {
        this.table_name = table_name;
        data_val = new DataVal(table_name);
        data_bitmap = new DataBitmap(table_name);

    }

    add_data_entry(std::vector<std::string> data)
    {
        DataEntry* new_data_entry = new DataEntry(data);
        temp_stack.push_back(new_data_entry);
    }

}
