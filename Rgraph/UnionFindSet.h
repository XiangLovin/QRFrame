#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// 并查集
class UnionFindSet
{
public:
    UnionFindSet(int size) : _ufs(size), rank(size, 0)
    {
        for (int i = 0; i < size; i++)
        {
            _ufs[i] = i;
        }
    };
    int find(int x);               // 查找某个元素属于哪个集合(根下标)
    void unionSet(int x1, int x2); // 合并两个集合
    size_t rootsize();             // 统计并查集内集合的个数

private:
    std::vector<int> _ufs;
    std::vector<int> rank;
};