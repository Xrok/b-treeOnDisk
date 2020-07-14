#include <iostream>
#include "pagemanager.h"
#include "btree.h"

using namespace utec::disk;

#define PAGE_SIZE  1024
#define BTREE_ORDER   ((PAGE_SIZE - (6 * sizeof(long) + sizeof(int)) ) /  (sizeof(int) + sizeof(long)))

int main(int argc, char const *argv[])
{
    std::shared_ptr<pagemanager> pm = std::make_shared<pagemanager>("b+tree.index", true);
    btree< int, BTREE_ORDER> bt(pm);
    srand (time(NULL));
    int num=0;


    for(int i = 1; i < 100; i++) {
         num = rand() % 1000 + 1;
         bt.insert(num);
     }
    bt.insert(150);
    bt.insert(200);



    auto iter = bt.find(150);
    auto end = bt.find(200);
    for(; iter != end; ++iter) {
        auto pair = *iter;
        
        std::cout<< pair <<  '\n';
    }
    return 0;
}