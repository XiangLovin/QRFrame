#include "UnionFindSet.h"
#include <vector>
#include <algorithm>

using namespace std;

int UnionFindSet::find(int x)
{
    return x == _ufs[x] ? x : (_ufs[x] = find(_ufs[x])); // 查找某个元素属于哪个集合(根下标)
}

void UnionFindSet::unionSet(int x, int y)
{
    int xRoot = find(x);
    int yRoot = find(y);
    if (xRoot == yRoot)
        return;

    // Union by rank
    if (rank[xRoot] < rank[yRoot])
    {
        _ufs[xRoot] = yRoot;
    }
    else if (rank[xRoot] > rank[yRoot])
    {
        _ufs[yRoot] = xRoot;
    }
    else
    {
        _ufs[yRoot] = xRoot;
        rank[xRoot]++;
    }
};

size_t UnionFindSet::rootsize()
{
    int size = 0;
    for (int i = 0; i < _ufs.size(); i++)
        if (_ufs[i] == i)
            size++;
    return size;
}
