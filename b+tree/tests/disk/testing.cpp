#include "gtest/gtest.h"
#include "../../src/utec/disk/btree.h"
#include "../../src/utec/disk/pagemanager.h"
#include <stdlib.h>



#define PAGE_SIZE  1024

// Other examples:
// PAGE_SIZE 1024 bytes => 1Kb
// PAGE_SIZE 1024*1024 bytes => 1Mb

// PAGE_SIZE = 4 * sizeof(long) +  (BTREE_ORDER + 1) * sizeof(int) + (BTREE_ORDER + 2) * sizeof(long)
// PAGE_SIZE = 4 * sizeof(long) +  (BTREE_ORDER) * sizeof(int) + sizeof(int) + (BTREE_ORDER) * sizeof(long) + 2 * sizeof(long)
// PAGE_SIZE = (BTREE_ORDER) * (sizeof(int) + sizeof(long))  + 4 * sizeof(long) + sizeof(int) +  2 * sizeof(long)
//  BTREE_ORDER = PAGE_SIZE - (4 * sizeof(long) + sizeof(int) +  2 * sizeof(long)) /  (sizeof(int) + sizeof(long))

#define BTREE_ORDER   ((PAGE_SIZE - (6 * sizeof(long) + sizeof(int)) ) /  (sizeof(int) + sizeof(long)))

using namespace utec::disk;

struct DiskBasedBtree : public ::testing::Test {
};

TEST_F(DiskBasedBtree, TestC) {

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

}