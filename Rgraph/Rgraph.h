//
// Created by xiang_w on 18/3/24.
//

#ifndef RULEMATCHING GRAPH H
#define RULEMATCHING GRAPH H
#include <unordered_map>
#include <iostream>

#include "../configuration/types.h"
#include "../configuration/config.h"
#include "../graph/graph.h"

#include <utility>
#include <set>

/**
 * A Rgraph is stored as the CSR format.
 */

class RGraph
{
public:
    struct Edge
    {
        VertexID begin;
        VertexID end;
        LabelID label;
    };

    struct Rule
    {
        Edge edge;
        ui lower_bound; // rule主体部分
        ui upper_bound;
        bool is_reverse; // 语义表达方向，默认为false
        bool is_percent; // 表达模式，看是否是百分比表达，默认为false
        ui level;
    };

private:
    struct Edge_hash
    {
        size_t operator()(const Edge &r1) const
        {
            using std::hash;
            using std::size_t;
            return (hash<LabelID>()(r1.label) ^ (hash<VertexID>()(r1.begin) << 1) >> 1) ^ (hash<VertexID>()(r1.end) << 1);
        }
    };

    struct Edge_equal
    {
        bool operator()(const Edge &rc1, const Edge &rc2) const noexcept
        {
            return rc1.begin == rc2.begin && rc1.end == rc2.end && rc1.label == rc2.label;
        }
    };

    bool enable_label_offset_;

    ui vertices_count_;
    ui edges_count_;
    ui labels_count_;
    ui rules_count_;
    ui max_degree_;
    ui max_label_frequency_;
    ui max_rule_level;

    ui *all_offsets_; // 偏移数组，记录每个节点的偏移量
    ui *in_offsets_;
    ui *out_offsets_;

    VertexID *all_neighbors_;
    VertexID *in_neighbors_;
    VertexID *out_neighbors_;

    LabelID *vertex_labels_;
    std::unordered_map<Edge, ui, Edge_hash, Edge_equal> edge_labels_;

    ui *reverse_index_offsets_;
    ui *reverse_index_;

    int *core_table_;
    ui core_length_;

    std::unordered_map<LabelID, ui> labels_frequency_;
    ui *rules_offsets_;
    Rule *edges_rules_;
    std::unordered_map<VertexID, std::set<VertexID>> edge_is_ruled_;
    ui *labels_offsets_;
    std::unordered_map<LabelID, ui> *nlf_;

private:
    void BuildReverseIndex();
    void BuildNLF();
    void BuildLabelOffset();
    void BuildRuleLevel();
    friend bool cmp(VertexID &a, VertexID &b);
    bool isSameLevel(Rule rulel, Rule rule2);
    bool isReachable(Rule rulel, Rule rule2);

public:
    RGraph(const bool enable_label_offset)
    {
        enable_label_offset_ = enable_label_offset;

        vertices_count_ = 0;
        edges_count_ = 0;
        labels_count_ = 0;
        rules_count_ = 0;
        max_degree_ = 0;
        max_label_frequency_ = 0;
        max_rule_level = 0;
        core_length_ = 0;

        all_offsets_ = NULL;
        in_offsets_ = NULL;
        out_offsets_ = NULL;
        all_neighbors_ = NULL;
        in_neighbors_ = NULL;
        out_neighbors_ = NULL;
        vertex_labels_ = NULL;
        rules_offsets_ = NULL;
        edge_labels_.clear();

        reverse_index_offsets_ = NULL;
        reverse_index_ = NULL;
        core_table_ = NULL;
        labels_frequency_.clear();
        edges_rules_ = NULL;
        edge_is_ruled_.clear();
        labels_offsets_ = NULL;
        nlf_ = NULL;
    }
    ~RGraph()
    {
        delete[] in_offsets_;
        delete[] out_offsets_;
        delete[] all_offsets_;
        delete[] in_neighbors_;
        delete[] out_neighbors_;
        delete[] all_neighbors_;
        delete[] vertex_labels_;
        delete[] rules_offsets_;
        delete[] reverse_index_offsets_;
        delete[] reverse_index_;
        delete[] core_table_;
        delete[] labels_offsets_;
        delete[] edges_rules_;
        delete[] nlf_;
    }

public:
    void loadRuledGraphFromFile(const std::string &file_path);
    void printGraphMetaData();

public:
    const ui getLabelsCount() const
    {
        return labels_count_;
    }

    const ui getVerticesCount() const
    {
        return vertices_count_;
    }

    const ui getEdgesCount() const
    {
        return edges_count_;
    }

    const ui getRulesCount() const
    {
        return rules_count_;
    }

    const ui getGraphMaxDegree() const
    {
        return max_degree_;
    }

    const ui getGraphMaxLabelFrequency() const
    {
        return max_label_frequency_;
    }

    const ui getMaxRuleLevel() const
    {
        return max_rule_level;
    }

    const ui getVertexOutDegree(const VertexID id) const
    {
        return out_offsets_[id + 1] - out_offsets_[id];
    }

    const ui getVertexInDegree(const VertexID id) const
    {
        return in_offsets_[id + 1] - in_offsets_[id];
    }

    const ui getVertexAllDegree(const VertexID id) const
    {
        return all_offsets_[id + 1] - all_offsets_[id];
    }

    const ui getLabelsFrequency(const LabelID label) const
    {
        return labels_frequency_.find(label) == labels_frequency_.end() ? 0 : labels_frequency_.at(label);
    }

    const ui getCoreValue(const VertexID id) const
    {
        return core_table_[id];
    }

    const ui get2CoreSize() const
    {
        return core_length_;
    }
    const LabelID getVertexLabel(const VertexID id) const
    {
        return vertex_labels_[id];
    }

    const ui *getVertexInNeighbors(const VertexID id, ui &count) const
    {
        count = in_offsets_[id + 1] - in_offsets_[id];
        return in_neighbors_ + in_offsets_[id];
    }

    const ui *getVertexOutNeighbors(const VertexID id, ui &count) const
    {
        count = out_offsets_[id + 1] - out_offsets_[id];
        return out_neighbors_ + out_offsets_[id];
    }

    const ui *getVerticesByLabel(const LabelID id, ui &count) const
    {
        count = reverse_index_offsets_[id + 1] - reverse_index_offsets_[id];
        return reverse_index_ + reverse_index_offsets_[id];
    }

    const ui *getInNeighborsByLabel(const VertexID id, const LabelID label, ui &count) const
    {
        ui offset = id * labels_count_ + label;
        count = labels_offsets_[offset + 1] - labels_offsets_[offset];
        return in_neighbors_ + labels_offsets_[offset];
    }

    const ui *getOutNeighborsByLabel(const VertexID id, const LabelID label, ui &count) const
    {
        ui offset = id * labels_count_ + label;
        count = labels_offsets_[offset + 1] - labels_offsets_[offset];
        return out_neighbors_ + labels_offsets_[offset];
    }

    const std::unordered_map<LabelID, ui> *getVertexNLF(const VertexID id) const
    {
        return nlf_ + id;
    }

    const Rule *getAllRúles() const
    {
        return edges_rules_;
    }

    const Rule *getRulesByLevel(const ui level, ui &count) const
    {
        Rule *leveled_rules = NULL; // 未完成
        if(level > max_rule_level)return NULL;
        for (int i = 0; i < rules_count_; i++)
        {
            if(edges_rules_[i].level == level)
                leveled_rules[count++] = edges_rules_[i];
        }
        return leveled_rules;
    }

    bool checkEdgeExistence(const VertexID u, const VertexID v, const LabelID u_label) const
    {
        ui countin = 0;
        ui countout = 0;
        const VertexID *in_neighbors = getInNeighborsByLabel(v, u_label, countin);
        const VertexID *out_neighbors = getOutNeighborsByLabel(v, u_label, countout);
        int begin = 0;
        int end = countin - 1;
        while (begin <= end)
        {
            int mid = begin + ((end - begin) >> 1);
            if (in_neighbors[mid] == u)
            {
                return true;
            }
            else if (in_neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }
        begin = 0;
        end = countout - 1;
        while (begin <= end)
        {
            int mid = begin + ((end - begin) >> 1);
            if (out_neighbors[mid] == u)
            {
                return true;
            }
            else if (out_neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }
        return false;
    }

    bool checkEdgeExistence(VertexID u, VertexID v) const
    {
        if (getVertexOutDegree(u) < getVertexOutDegree(v))
        {
            std::swap(u, v);
        }
        ui count = 0;
        const VertexID *in_neighbors = getVertexInNeighbors(v, count);

        int begin = 0;
        int end = count - 1;
        while (begin <= end)
        {
            int mid = begin + ((end - begin) >> 1);
            if (in_neighbors[mid] == u)
            {
                return true;
            }
            else if (in_neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }
        count = 0;
        const VertexID *out_neighbors = getVertexOutNeighbors(v, count);

        begin = 0;
        end = count - 1;
        while (begin <= end)
        {
            int mid = begin + ((end - begin) >> 1);
            if (out_neighbors[mid] == u)
            {
                return true;
            }
            else if (out_neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }
        return false;
    }

    void buildCoreTable();
};

#endif // RULEMATCHING_GRAPH_H
