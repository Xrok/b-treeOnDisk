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

    for(int i = 1; i < 100; i++) {
          bt.insert(i);
    }

    #pragma omp parallel num_threads(5)
    {
         std::shared_ptr<pagemanager> local_pm = std::make_shared<pagemanager>("b+tree.index", false);
    
         #pragma omp for
         for(int i = 1; i < 100; i++) {
             auto it = bt.find(i, local_pm);
             std::cout << *it << " found\n";
         }
    }

}
