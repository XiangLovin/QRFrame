//
// Created by xiang_w on 18/1/24.
//

#include "Rgraph.h"
#include "UnionFindSet.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <queue>
#include <map>
using namespace std;

void RGraph::BuildReverseIndex()
{
    reverse_index_ = new ui[vertices_count_];
    reverse_index_offsets_ = new ui[labels_count_ + 1];
    reverse_index_offsets_[0] = 0;

    ui total = 0;
    for (ui i = 0; i < labels_count_; ++i)
    {
        reverse_index_offsets_[i + 1] = total;
        total += labels_frequency_[i];
    }

    for (ui i = 0; i < vertices_count_; ++i)
    {
        LabelID label = vertex_labels_[i];
        reverse_index_[reverse_index_offsets_[label + 1]++] = i;
    }
}

void RGraph::BuildNLF()
{
    nlf_ = new std::unordered_map<LabelID, ui>[vertices_count_];
    for (ui i = 0; i < vertices_count_; ++i)
    {
        ui count;
        const VertexID *in_neighbors = getVertexInNeighbors(i, count);
        for (ui j = 0; j < count; ++j)
        {
            VertexID u = in_neighbors[j];
            LabelID label = getVertexLabel(u);
            if (nlf_[i].find(label) == nlf_[i].end())
            {
                nlf_[i][label] = 0;
            }

            nlf_[i][label] += 1;
        }
        const VertexID *out_neighbors = getVertexOutNeighbors(i, count);
        for (ui j = 0; j < count; ++j)
        {
            VertexID u = out_neighbors[j];
            LabelID label = getVertexLabel(u);
            if (nlf_[i].find(label) == nlf_[i].end())
            {
                nlf_[i][label] = 0;
            }

            nlf_[i][label] += 1;
        }
    }
}

void RGraph::BuildLabelOffset()
{
    size_t labels_offset_size = (size_t)vertices_count_ * labels_count_ + 1;
    labels_offsets_ = new ui[labels_offset_size];
    std::fill(labels_offsets_, labels_offsets_ + labels_offset_size, 0);

    for (ui i = 0; i < vertices_count_; ++i)
    {
        std::sort(in_neighbors_ + in_offsets_[i], in_neighbors_ + in_offsets_[i + 1],
                  [this](const VertexID u, const VertexID v) -> bool
                  {
                      return vertex_labels_[u] == vertex_labels_[v] ? u < v : vertex_labels_[u] < vertex_labels_[v];
                  });
        std::sort(out_neighbors_ + out_offsets_[i], out_neighbors_ + out_offsets_[i + 1],
                  [this](const VertexID u, const VertexID v) -> bool
                  {
                      return vertex_labels_[u] == vertex_labels_[v] ? u < v : vertex_labels_[u] < vertex_labels_[v];
                  });
    }

    for (ui i = 0; i < vertices_count_; ++i)
    {
        LabelID previous_label = 0;
        LabelID current_label = 0;

        labels_offset_size = i * labels_count_;
        labels_offsets_[labels_offset_size] = all_offsets_[i];

        for (ui j = all_offsets_[i]; j < all_offsets_[i + 1]; ++j)
        {
            current_label = vertex_labels_[all_neighbors_[j]];

            if (current_label != previous_label)
            {
                for (ui k = previous_label + 1; k <= current_label; ++k)
                {
                    labels_offsets_[labels_offset_size + k] = j;
                }
                previous_label = current_label;
            }
        }

        for (ui l = current_label + 1; l <= labels_count_; ++l)
        {
            labels_offsets_[labels_offset_size + l] = all_offsets_[i + 1];
        }
    }
}

////判官两个规则是否相等
bool RGraph::isSameLevel(Rule rule1, Rule rule2)
{
    // 来源于一进与出两条边
    if (rule1.edge.end == rule2.edge.begin || rule1.edge.begin == rule2.edge.end)
    {
        if (rule1.is_reverse == rule2.is_reverse)
            return true;
    }
    ////同样来源于进或者出
    else if (rule1.edge.begin == rule2.edge.begin || rule1.edge.end == rule2.edge.end)
    {
        if (rule1.is_reverse != rule2.is_reverse)
            return true;
    }
    else
        return false;
}

// 判断是否是联通
bool RGraph::isReachable(Rule rule1, Rule rule2)
{
    VertexID rule1_domain = rule1.is_reverse ? rule1.edge.begin : rule1.edge.end;
    VertexID rule1_range = rule1.is_reverse ? rule1.edge.end : rule1.edge.begin;
    VertexID rule2_domain = rule2.is_reverse ? rule2.edge.begin : rule2.edge.end;
    VertexID rule2_range = rule2.is_reverse ? rule2.edge.end : rule2.edge.begin;

    if (rule1_domain == rule2_range)
        return true;
    if ((rule1_domain == rule2_domain) || (rule1_range == rule2_range))
        return true;
    bool *visited = new bool[vertices_count_];
    for (int i = 0; i < vertices_count_; i++)
        visited[i] = false;

    list<VertexID> queue;
    queue.push_back(rule1_domain);
    visited[rule1_domain] = true;

    while (!queue.empty())
    {
        VertexID curId = queue.front();
        queue.pop_front();
        // cout<<"当前检査节点“<<curId<<”的邻居:";
        ui count = 0;
        const VertexID *out_neighbors = getVertexOutNeighbors(curId, count);
        std::unordered_map<VertexID, ui> neibor_frequency_;
        for (int i = 0; i < count; i++)
        {
            neibor_frequency_[out_neighbors[i]]++;
            // cout<<" "<<out_neighbors[i]<<"++/";
        }
        // cout<<endl;
        for (int i = 0; i < rules_count_; i++)
        {
            if (edges_rules_[i].edge.begin == curId)
            {
                neibor_frequency_[edges_rules_[i].edge.end]--;
                // cout<<edges_rules[i].edge.begin<<"->"<<edges_rules_ [i].edge.end<<"("<<edges_rules_[i].edge.label
                //<<")让"<<edges_rules_[i].edge.end<<"减到"<<neighbors_frequency_[edges_rules[i].edge.end]<<"/";
            }
        }
        // cout<<"\nneibor:";
        for (auto item : neibor_frequency_)
        {
            // cout<<item.first<<"("<<item.second<<")，";
            if (item.second == 0)
                continue;
            if (item.first == rule2_range)
            {
                // cout<<endl<<"当前已便利到节点“<<item,first<<endl;
                return true;
            }
            if (!visited[item.first])
            {
                visited[item.first] = true;
                queue.push_back(item.first);
            }
        }

        // for(int i=0;i< getVertexOutDegree(curId); ++i){
        //     VertexID temp =out_neighbors_[out_offsets_[curId]+i];
        //     if((rule1_domain ==rule2_domain)||(rule1_range == rule2_range))continue;
        //     //应当全部规则边都不考虑
        //     int flag = 0;

        //     for(int i=0;i<rules_count_;i++){
        //         if(edges_rules_[i].edge.begin == curId && edges_rules_[i].edge.end == temp){
        //             flag = 1;break;//需要排除两个点之间有多条边的情况
        //         }
        //     }
        //     if(flag)continue;

        //     if(temp == rule2_range){
        //         cout<<endl<<"当前以遍历到节点 "<<temp<<endl;
        //         return true;
        //     }

        //     if(!visited[temp]){
        //         visited[temp] = true;
        //         queue.push_back(temp);
        //         cout<<temp<<", ";
        //     }
        // }
        // cout<<endl<<endl;
        // queue.sort(cmp);
    }
    return false;
}

void RGraph::BuildRuleLevel()
{

    rules_offsets_ = new ui[rules_count_ + 1];
    vector<vector<int>> levelMap(rules_count_, vector<int>(rules_count_, 0));
    vector<std::pair<int, int>> equalIndices;
    vector<int> levels(rules_count_, 0);
    std::unordered_map<VertexID, ui> outDegree;
    list<VertexID> queue;
    bool *visited = new bool[rules_count_];
    rules_offsets_[0] = 0;
    ui curLevel = 1;
    // 构造等级依赖图
    for (int i = 0; i < rules_count_; i++)
    {
        Rule rule1 = edges_rules_[i];
        for (int j = 0; j < rules_count_; j++)
        {
            if (i == j)
                continue;
            Rule rule2 = edges_rules_[j];
            if (isReachable(rule1, rule2) && !isSameLevel(rule1, rule2))
            {
                // cout<<rule1.edge.begin<<"->"<<rule1.edge.end<<"可以连通"<<rule2.edge.begin<<"->"<<rule2.edge.end<<endl;
                levelMap[i][j] = 1; // rule2依赖于rule1
                outDegree[i]++;
                // levelMap[j][i]=1;//rule2依赖于rule1
                // inDegree[i]++;
                //  cout<<edges_rules_[i].edge.begin<<"->"<<edges_rules_[i].edge.end<<"规则此时为"<<edges_rules_[i].level<<endl;
            }
            if (isSameLevel(rule1, rule2))
            {
                std::pair<int, int> p(i, j);
                equalIndices.push_back(p);
            }
        }
    }

    for (int i = 0; i < rules_count_; i++)
    {
        if (!outDegree[i])
        {
            queue.push_back(i);
            visited[i] = true;
            levels[i] = curLevel++;
        }
    }
    while (!queue.empty())
    {
        VertexID curId = queue.front();
        queue.pop_front();
        for (int i = 0; i < rules_count_; i++)
        {
            if (levelMap[i][curId])
            {
                // edges_rules_[j].level = isSameLevel(edges_rules_[j],edges_rules_[curId])?edges_rules_[curId].level:curLevel;

                if (--outDegree[i] == 0)
                {
                    queue.push_back(i);
                    visited[i] = true;
                    if (levels[i] == 0)
                        levels[i] = curLevel++;
                }
            }
        }
    }

    cout << "-----等级相同判断前-----" << endl;
    for (int i = 0; i < rules_count_; i++)
    {
        cout << i << ":" << edges_rules_[i].lower_bound << "," << edges_rules_[i].upper_bound << ":" << levels[i] << endl;
    }

    UnionFindSet uf(curLevel - 1);

    // 合并等级相同的下标
    for (auto indices : equalIndices)
    {
        uf.unionSet(indices.first, indices.second);
    }
    // 找到每个集合的最大值
    std::vector<int> maxRank(rules_count_, 0);
    for (int i = 0; i < rules_count_; i++)
    {
        int root = uf.find(i);
        maxRank[root] = maxRank[root] > levels[i] ? maxRank[root] : levels[i];
    }

    // 更新
    for (int i = 0; i < rules_count_; i++)
    {
        int root = uf.find(i);
        levels[i] = maxRank[root];
    }

    cout << "-----等级相同判断后(未调整)-----" << endl;
    for (int i = 0; i < rules_count_; i++)
    {
        cout << i << ":" << edges_rules_[i].lower_bound << "," << edges_rules_[i].upper_bound << ":" << levels[i] << endl;
    }

    std::vector<int> sortedValues = levels;
    std::map<int, int> valueMap;
    std::sort(sortedValues.begin(), sortedValues.end());
    sortedValues.erase(unique(sortedValues.begin(), sortedValues.end()), sortedValues.end());
    for (int i = 0; i < sortedValues.size(); i++)
        valueMap[sortedValues[i]] = i + 1;

    for (int i = 0; i < rules_count_; i++)
    {
        levels[i] = valueMap[levels[i]];
        edges_rules_[i].level = levels[i];
        max_rule_level = max_rule_level > levels[i] ? max_rule_level : levels[i];
    }

    cout << "-----等级相同判断后（调整后）-----" << endl;
    for (int i = 0; i < rules_count_; i++)
    {
        cout << i << ":" << edges_rules_[i].lower_bound << "," << edges_rules_[i].upper_bound << ":" << levels[i] << endl;
    }
    cout << "图中最大等级为:" << max_rule_level << endl;
}

void RGraph::loadRuledGraphFromFile(const std::string &file_path)
{
    std::ifstream infile(file_path);

    if (!infile.is_open())
    {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    infile >> type >> vertices_count_ >> edges_count_ >> rules_count_;
    // 用于判断入边/出边/全部另据的偏移量
    in_offsets_ = new ui[vertices_count_ + 1];
    in_offsets_[0] = 0;
    out_offsets_ = new ui[vertices_count_ + 1];
    out_offsets_[0] = 0;
    all_offsets_ = new ui[vertices_count_ + 1];
    all_offsets_[0] = 0;

    all_neighbors_ = new VertexID[edges_count_ * 2];
    in_neighbors_ = new VertexID[edges_count_];
    out_neighbors_ = new VertexID[edges_count_];

    vertex_labels_ = new LabelID[vertices_count_];
    edges_rules_ = new Rule[rules_count_];
    labels_count_ = 0;
    max_degree_ = 0;

    LabelID max_label_id = 0;
    ui rules_index = 0;
    std::vector<ui> neighbors_in_offset(vertices_count_, 0);
    std::vector<ui> neighbors_out_offset(vertices_count_, 0);
    std::vector<ui> neighbors_all_offset(vertices_count_, 0);
    std::unordered_map<VertexID, ui> vertex_in_degrees;
    std::unordered_map<VertexID, ui> vertex_out_degrees;

    std::vector<Edge> edges;

    // 只负责统计
    while (infile >> type)
    {
        if (type == 'v')
        { // Read vertex.
            VertexID id;
            LabelID label;
            infile >> id >> label;

            vertex_labels_[id] = label; // ID直接作为下标，记录对应的节点标签

            // 如果当前label没有被保存，则先保存
            if (labels_frequency_.find(label) == labels_frequency_.end())
            {
                labels_frequency_[label] = 0;
                if (label > max_label_id)
                    max_label_id = label;
            }

            labels_frequency_[label] += 1;
        }
        else if (type == 'e')
        { // Read edge.
            Edge e;
            infile >> e.begin >> e.end >> e.label;

            LabelID label = e.label;
            edge_labels_[e] = label;
            if (labels_frequency_.find(label) == labels_frequency_.end())
            {
                labels_frequency_[label] = 0;
                if (label > max_label_id)
                    max_label_id = label;
            }
            labels_frequency_[label] += 1;
            edges.push_back(e);
            vertex_in_degrees[e.end] += 1;
            vertex_out_degrees[e.begin] += 1;
        }
        else if (type == 'r')
        { // Read rule.
            Edge e;
            Rule ra;
            infile >> e.begin >> e.end >> e.label >> ra.lower_bound >> ra.upper_bound >> ra.is_reverse >> ra.is_percent;
            ra.edge = e;
            ra.level = 0;
            edges_rules_[rules_index++] = ra;
        }
    }
    infile.close();

    // 处理偏移信息
    for (VertexID id = 0; id < vertices_count_; id++)
    {
        in_offsets_[id + 1] = in_offsets_[id] + vertex_in_degrees[id];
        out_offsets_[id + 1] = out_offsets_[id] + vertex_out_degrees[id];
        all_offsets_[id + 1] = all_offsets_[id] + vertex_out_degrees[id] + vertex_in_degrees[id];
        if (vertex_in_degrees[id] + vertex_out_degrees[id] > max_degree_)
        {
            max_degree_ = vertex_in_degrees[id] + vertex_out_degrees[id];
        }
    }
    // 处理节点出边邻居和入边邻居
    for (Edge e : edges)
    {
        ui out_offset = out_offsets_[e.begin] + neighbors_out_offset[e.begin];
        ui in_offset = in_offsets_[e.end] + neighbors_in_offset[e.end];
        in_neighbors_[in_offset] = e.begin;
        out_neighbors_[out_offset] = e.end;

        ui offset = all_offsets_[e.begin] + neighbors_all_offset[e.begin];
        all_neighbors_[offset] = e.end;
        offset = all_offsets_[e.end] + neighbors_all_offset[e.end];
        all_neighbors_[offset] = e.begin;

        neighbors_out_offset[e.begin] += 1;
        neighbors_in_offset[e.end] += 1;
        neighbors_all_offset[e.begin] += 1;
        neighbors_all_offset[e.end] += 1;
    }

    labels_count_ = (ui)labels_frequency_.size() > (max_label_id + 1) ? (ui)labels_frequency_.size() : max_label_id + 1;

    for (auto element : labels_frequency_)
    {
        if (element.second > max_label_frequency_)
        {
            max_label_frequency_ = element.second;
        }
    }

    for (ui i = 0; i < vertices_count_; ++i)
    {
        std::sort(in_neighbors_ + in_offsets_[i], in_neighbors_ + in_offsets_[i + 1]);
        std::sort(out_neighbors_ + out_offsets_[i], out_neighbors_ + out_offsets_[i + 1]);
        std::sort(all_neighbors_ + all_offsets_[i], all_neighbors_ + all_offsets_[i + 1]);
    }

    BuildReverseIndex();
    if (enable_label_offset_)
    {
        BuildNLF();
        // BuildLabelOffset();
    }

    // TEST
    cout << "-----------------------------vertex offsets:" << endl;
    for (int i = 0; i < vertices_count_ + 1; i++)
        cout << i << ":" << in_offsets_[i] << "/" << out_offsets_[i] << "/" << all_offsets_[i] << endl;
    cout << endl;

    cout << "-----------------------------vertex neighbors:" << endl;
    for (int i = 0; i < vertices_count_; i++)
    {
        cout << "id:" << i << "，入边:";
        for (int j = 0; j < in_offsets_[i + 1] - in_offsets_[i]; j++)
        {
            cout << in_neighbors_[in_offsets_[i] + j] << ",";
        }
        cout << ",    出边";
        for (int j = 0; j < out_offsets_[1 + 1] - out_offsets_[i]; j++)
        {
            cout << out_neighbors_[out_offsets_[i] + j] << ",";
        }
        cout << endl;
        cout << "全部边:";
        for (int j = 0; i < all_offsets_[i + 1] - all_offsets_[i]; i++)
        {
            cout << all_neighbors_[all_offsets_[i] + j] << ",";
        }
        cout << endl;
    }
    cout << endl;

    cout << "-----------------------------edge rules(" << rules_count_ << "):" << endl;
    for (int i = 0; i < rules_count_; i++)
        cout << edges_rules_[i].edge.begin << ", " << edges_rules_[i].edge.end << "(" << edges_rules_[i].edge.label << ") : ["
             << edges_rules_[i].lower_bound << "," << edges_rules_[i].upper_bound << "], "
             << (edges_rules_[i].is_reverse ? "逆" : "正") << "," << (edges_rules_[i].is_percent ? "百分比" : "正常")
             << ", " << edges_rules_[i].level << endl;
    cout << endl;

    for (int i = 0; i < vertices_count_; i++)
    {
        set<VertexID> curSet = edge_is_ruled_[i];
        if (curSet.size() != 0)
        {
            cout << i << "出边邻居:";
            for (auto b : curSet)
            {
                cout << b << ", ";
            }
            cout << endl;
        }
    }
    cout << endl;

    cout << "--------------------边连通性" << rules_count_ << "):" << endl;
    BuildRuleLevel();
}

void RGraph::printGraphMetaData()
{
    std::cout << "|V|: " << vertices_count_ << ", |E|: " << edges_count_ << ", |R|: " << rules_count_ << ", |\u03A3|: " << labels_count_ << std::endl;
    std::cout << "Max Degree: " << max_degree_ << ", Max Label Frequency: " << max_label_frequency_ << ", Max Rule Level: " << max_rule_level << endl;
}

void RGraph::buildCoreTable()
{
    core_table_ = new int[vertices_count_];
    // GraphOperations::getKCore(this, core_table_);

    for (ui i = 0; i < vertices_count_; ++i)
    {
        if (core_table_[i] > 1)
        {
            core_length_ += 1;
        }
    }
}