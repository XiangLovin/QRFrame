#include <iostream>
#include <chrono>
#include <future>
#include <thread>
#include <map>
#include <list>
#include <queue>
#include "../graph/graph.h"
#include "Rgraph.h"


using namespace std;

 void DivideQGP(const RGraph *query_graph, const RGraph::Rule* Rules,
       map<VertexID, RGraph*> &domQGPs, map<VertexID, RGraph*> &ranQGPs, set<RGraph::Edge>&Crs)
 {
//     list<RGraph::Rule> Rules;
//     UnionFindSet *block;
//     block = new UnionFindSet(Rules.size()); //利用数组并排查集维护划分节点之间的关系

//     list<queue<VertexID> > queueMap;//用于维护多层次遍历
//     dupNum=0;
//     dupDom=true;
//     set<VertexID> roots;
//     //统计所有根以及重复节点是定义域还是作用域
//     for(auto r : Rs)
//     {
//         if (block[r.start]==0){//第一次遇到
//             block[r.start]=r.start;
//             roots.add(r.start);
//         }
//         else if(block[r.start]!=0 && dupNum==0){    //第二次遇到
//             dupNum=r.start;
//         }
//         dumDom=false;
//         ranRoots.push(r.start);
//         ranRQGs[r.start]=new RQG;
//         if(block[r.end]==0){
//             block[r.end]=r.end;
//             roots.add(r.end);
//         }
//         else if(block[r.end]!=0 && dupNum==0){
//             dupNum=r.end;
//             dumDom=true;
//             domRQGs[r.end]=new RQG;
//             domRoots.push(r.end);
//         }
//     }
//     //初始化全部子图和层次遍历队列
//     for(ui root:roots)
//     {
//         queueMap[root]=new queue;
//         queueMap[root].push(root);
//         if(root==dupNum)
//             continue;
//         else
//         {
//             if(dupDom){
//                 ranRQGs[root]=new RQG;
//                 ranRoots.push(root);
//             }
//             else {
//                 domRQGs[root]=new RQG;
//                 domRoots.push(root);
//             }
//         }
//     }

//     //利用队列开始对图进行全局层次遍历，截止条件是全部节点均被访问过
//     //图采用CSR(Compressed Sparse Row）格式存储
//     queue<RGraph::Edge> edgeList = NULL;//利用队列保存没有用过的边
//     for(e : Q.E)edgeList.push(e);
//     while(!edgeList.size()){
//         for(VertexID root: domRoots){
//             Queue temp=new Queue<int>;//存储临时队列，保存下一层全部节点
//             Queue q=queueMap[root];
//             while(!q.empty()){
//                 Node node=q.front();
//                 domRGQs[root].V.push(node);
//                 block[node]=root;
//                 q.pop();
//                 // 找到队列头节点的全部邻居
//                 int cur_size=edgeList.size();
//                 while(cur_size--){
//                     Edge e =edgeList.front();
//                     edgeList.pop();
//                     if (e.start==node)
//                     {
//                         if(block[e.end]==0)
//                         {//node的出边没有被访问过
//                             domRQGs[root].E.push(e);
//                             block[e.end]==root;
//                             temp.push(e.end);
//                         }
//                         else if (block[e.end]==root)
//                         {//node的出边同域
//                             domRGQs[root].E.push(e);
//                         }
//                         else
//                         {//node的出边跨域
//                             Crs.push(e);
//                         }
//                     }
//                     else if(e.end==node)
//                     {
//                         if (block[e.start]==0)
//                         {//node的出边没有被访问过
//                             ranRGQs[root].E.push(e);
//                             block[e.start]==root;
//                             temp.push(e.start);
//                         }
//                         else if (block[e.start]==root)
//                         {//node的出边同域
//                             ranRGQs[root].E.push(e);
//                         }
//                         else
//                         {//node的出边跨域
//                             Crs.push(e);
//                         }
//                     }
//                     else edgeList.push(e);
//                 }
//             }
//             queueMap[root] = temp;
//         }
//     }

}