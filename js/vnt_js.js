const axios = require("axios").default;
const fs = require("fs");

const get_height = { "jsonrpc": "2.0", "method": "core_blockNumber", "params": [], "id": 1 };
const get_block = { "jsonrpc": "2.0", "method": "core_getBlockByNumber", "params": ["0x2", true], "id": 1 };

// let transactions = [];
let currentBlockHeight = 0;
let currentTopHeight = 1;

async function request(data, resolve, reject){
    try {
        const res = await axios.post("http://47.111.100.232:8880", data);
        return resolve(res.data);
    }
    catch (err) {
        return reject(err);
    }
}

async function run()
{
    while (currentBlockHeight < currentTopHeight)
    {
        await request(get_height, res => { currentTopHeight = Number(parseInt(res.result, 16).toString(10))}, console.log);
        while (currentBlockHeight < currentTopHeight){
            get_block.params[0]='0x'+currentBlockHeight.toString(16);
            await request(get_block, 
                res=>{
                    // console.log(res.result);
                    let transactions = res.result.transactions;
                    if(transactions){
                        for(transaction of transactions){
                            fs.appendFile('data.json', JSON.stringify(transaction, null, "\t")+',\n',()=>{});
                        }
                    }
                },
                console.log);
            currentBlockHeight++;
        }
    }
    // console.log(transactions);
}

run();
