#include "iterator.h"
#include "pagemanager.h"
#include <memory>

#ifndef B_TREE_BTREE_H
#define B_TREE_BTREE_H

namespace utec
{
    namespace disk
    {
        template <class T, int BTREE_ORDER = 3>
        class btree
        {
        public:
            typedef Node<T, BTREE_ORDER> node;
            typedef Iterator<T, BTREE_ORDER> iterator;

            iterator begin()
            {
                node aux = read_node(header.root_id);
                while (aux.children[0] != 0)
                {
                    long page_id = aux.children[0];
                    aux = read_node(page_id);
                }
                iterator it(pm);
                it.currentPosition = aux;
                return it;
            }

            iterator find(const T &object)
            {
                node root = read_node(header.root_id);
                auto it = find(object, root);
                if (*it == object)
                {
                    return it;
                }
                else
                {
                    return end();
                }
            }

            iterator find(const T &object, const node &other)
            {
                int pos = 0;
                if (other.children[0] != 0)
                {
                    while (pos < other.count && other.data[pos] <= object)
                    {
                        pos++;
                    }
                    node child = read_node(other.children[pos]);
                    return find(object, child);
                }
                else
                {
                    while (pos < other.count && other.data[pos] < object)
                    {
                        pos++;
                    }
                    iterator it(pm);
                    it.currentPosition = other;
                    it.index = pos;
                    return it;
                }
            }

            iterator end()
            {
                node aux{-1};
                iterator it(pm);
                it.currentPosition = aux;
                return it;
            }

            struct Metadata
            {
                long root_id{1};
                long count{0};
            } header;

            enum state
            {
                BT_OVERFLOW,
                BT_UNDERFLOW,
                NORMAL,
            };

        private:
            std::shared_ptr<pagemanager> pm;

        public:
            btree(std::shared_ptr<pagemanager> pm) : pm{pm}
            {
                if (pm->is_empty())
                {
                    node root{header.root_id};
                    pm->save(root.page_id, root);

                    header.count++;

                    pm->save(0, header);
                }
                else
                {
                    pm->recover(0, header);
                }
            }

            node new_node()
            {
                header.count++;
                node ret{header.count};
                pm->save(0, header);
                return ret;
            }

            node read_node(long page_id)
            {
                node n{-1};
                pm->recover(page_id, n);
                return n;
            }

            void write_node(long page_id, node n) { pm->save(page_id, n); }

            void insert(const T &value)
            {
                node root = read_node(header.root_id);
                int state = insert(root, value);

                if (state == BT_OVERFLOW)
                {
                    split_root();
                }
            }

            int insert(node &ptr, const T &value)
            {
                int pos = 0;
                while (pos < ptr.count && ptr.data[pos] < value)
                {
                    pos++;
                }
                if (ptr.children[pos] != 0)
                {
                    long page_id = ptr.children[pos];
                    node child = read_node(page_id);
                    int state = insert(child, value);
                    if (state == BT_OVERFLOW)
                    {
                        split(ptr, pos);
                    }
                }
                else
                {
                    ptr.insert_in_node(pos, value);
                    write_node(ptr.page_id, ptr);
                }
                return ptr.is_overflow() ? BT_OVERFLOW : NORMAL;
            }

            void split(node &parent, int pos)
            {
                node node_in_overflow = this->read_node(parent.children[pos]);
                node child1 = node_in_overflow;
                child1.count = 0;
                node child2 = this->new_node();

                int iter = 0;
                int i;

                #pragma omp parallel firstprivate(iter, i) num_threads(3)
                {
                #pragma omp sections
                {
                    #pragma omp section 
                    {
                        for (i = 0; iter < ceil(BTREE_ORDER / 2.0); i++)
                        {
                            child1.children[i] = node_in_overflow.children[iter];
                            child1.data[i] = node_in_overflow.data[iter];
                            child1.count++;
                            iter++;
                        }
                        child1.children[i] = node_in_overflow.children[iter];
                    }

                    #pragma omp section
                    {
                        iter = ceil(BTREE_ORDER / 2.0);
                        parent.insert_in_node(pos, node_in_overflow.data[iter]);

                        if (node_in_overflow.children[0] == 0)
                        {
                            child2.right = child1.right;
                            child1.right = child2.page_id;
                            parent.children[pos + 1] = child2.page_id;
                        }
                    }

                    #pragma omp section
                    {
                        iter = ceil(BTREE_ORDER / 2.0) + (node_in_overflow.children[0] != 0);
                        for (i = 0; iter < BTREE_ORDER + 1; i++)
                        {
                            child2.children[i] = node_in_overflow.children[iter];
                            child2.data[i] = node_in_overflow.data[iter];
                            child2.count++;
                            iter++;
                        }
                        child2.children[i] = node_in_overflow.children[iter];

                        parent.children[pos] = child1.page_id;
                        parent.children[pos + 1] = child2.page_id;
                    }
                }
                

                // implicit barrier
                #pragma omp single
                {
                    #pragma omp task
                    write_node(parent.page_id, parent);
                    #pragma omp task
                    write_node(child1.page_id, child1);
                    #pragma omp task
                    write_node(child2.page_id, child2);
                    #pragma omp taskwait
                }
                }
            }

            void split_root()
            {
                node node_in_overflow = this->read_node(this->header.root_id);
                node child1 = this->new_node();
                node child2 = this->new_node();

                int pos = 0;
                int iter = 0;
                int i;

                #pragma omp parallel firstprivate(iter, i) num_threads(2)
                {
                #pragma omp sections
                {
                    #pragma omp section
                    {
                        for (i = 0; iter < ceil(BTREE_ORDER / 2.0); i++)
                        {
                            child1.children[i] = node_in_overflow.children[iter];
                            child1.data[i] = node_in_overflow.data[iter];
                            child1.count++;
                            iter++;
                        }
                        child1.children[i] = node_in_overflow.children[iter];

                        node_in_overflow.data[0] = node_in_overflow.data[iter];
                        child1.right = child2.page_id;
                    }

                    #pragma omp section
                    {
                        iter = ceil(BTREE_ORDER / 2.0) + (node_in_overflow.children[0] != 0);

                        for (i = 0; iter < BTREE_ORDER + 1; i++)
                        {
                            child2.children[i] = node_in_overflow.children[iter];
                            child2.data[i] = node_in_overflow.data[iter];
                            child2.count++;
                            iter++;
                        }
                        child2.children[i] = node_in_overflow.children[iter];

                        node_in_overflow.children[0] = child1.page_id;
                        node_in_overflow.children[1] = child2.page_id;
                        node_in_overflow.count = 1;
                    }
                }
                #pragma omp single
                {
                    #pragma omp task
                    write_node(node_in_overflow.page_id, node_in_overflow);
                    #pragma omp task
                    write_node(child1.page_id, child1);
                    #pragma omp task
                    write_node(child2.page_id, child2);
                }
                }
            }

            T succesor(node &ptr)
            {
                while (ptr.children[0] != 0)
                {
                    ptr = read_node(ptr.children[0]);
                }
                if (ptr.count == 1)
                {
                    if (ptr.right == -1)
                        return NULL;
                    ptr = read_node(ptr.right);
                    return ptr.data[0];
                }
                else
                {
                    return ptr.data[1];
                }
            }

            void print(std::ostream &out)
            {
                node root = read_node(header.root_id);
                print(root, 0, out);
            }

            void print(node &ptr, int level, std::ostream &out)
            {
                int i;
                for (i = 0; i < ptr.count; i++)
                {
                    if (ptr.children[i])
                    {
                        node child = read_node(ptr.children[i]);
                        print(child, level + 1, out);
                    }
                    out << ptr.data[i];
                }
                if (ptr.children[i])
                {
                    node child = read_node(ptr.children[i]);
                    print(child, level + 1, out);
                }
            }

            void print_tree()
            {
                node root = read_node(header.root_id);
                print_tree(root, 0);
                std::cout << "________________________\n";
            }

            void print_tree(node &ptr, int level)
            {
                int i;
                for (i = ptr.count - 1; i >= 0; i--)
                {
                    if (ptr.children[i + 1])
                    {
                        node child = read_node(ptr.children[i + 1]);
                        print_tree(child, level + 1);
                    }

                    for (int k = 0; k < level; k++)
                    {
                        std::cout << "    ";
                    }
                    std::cout << ptr.data[i] << "\n";
                }
                if (ptr.children[i + 1])
                {
                    node child = read_node(ptr.children[i + 1]);
                    print_tree(child, level + 1);
                }
            }
        };

    } // namespace disk
} // namespace utec

#endif //B_TREE_BTREE_H
