////输入:一个R0G Q=(Vg，Eq，/cita_q)，规则函数level，当前层级cur
// //输出:匹配M

#include <iostream>
#include <chrono>
#include <future>
#include <thread>
#include <fstream>
#include <map>
#include <list>
#include <queue>
#include "../graph/graph.h"
#include "Rgraph.h"
#include "UnionFindset.h"

#include "GenerateFilteringPlan.h"
#include "FilterVertices.h"
#include "GenerateQueryPlan.h"
#include "EvaluateQuery.h"
#include "BuildTable.h"
#include "DivideQGP.cpp"

#define NANOSECTOSEC(elapsed_time) ((elapsed_time)/(double)1000000000)
#define BYTESTOMB(memory_cost) ((memory_cost)/(double)(1024 * 1024))

using namespace std;

Graph *RGraph2Graph(const RGraph *query_graph)
{
    Graph *new_query_graph = new Graph(true);
    string file_path = "../Rgraph/subquery_without_rule/subQuery.graph";
    std::ofstream outfile(file_path, ios::trunc);
    if (!outfile.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return NULL;
    }
    ui vertex_count = query_graph->getVerticesCount();
    ui edge_count = query_graph->getEdgesCount();
    outfile << "t "<<vertex_count<<" "<< edge_count<<endl;
    for(int i=0;i<vertex_count;i++){
        outfile << "v "<<i<<" "<< query_graph->getVertexLabel(i)<<" "<<query_graph->getVertexAllDegree(i)<<endl;
        ui count = 0;
        const VertexID *out_neighbors = query_graph->getVertexOutNeighbors(i, count);
        for(int j = 0 ; j < count ; j++){
            outfile << "e "<<i<<" "<< out_neighbors[j]<<endl;
        }
    }
    new_query_graph->loadGraphFromFile(file_path);
    return new_query_graph;

}


VertexID **MatchQGP(const Graph *data_graph, const RGraph *query_graph, ui cur_level,
            ui **&candidates, ui *&candidates_count, ui *&order,
            string input_filter_type, string input_order_type, string input_engine_type)
{
    // 结构初始化
    map<VertexID, RGraph*> domQGPs;    // 定义域图RGraph图
    map<VertexID, RGraph*> ranQGPs;              //作用域组RGraph图
    set<RGraph::Edge> Crs;                      // 交叉边
    //set<RGraph> domGraphs;                      //定义域诱导子图组
    //set<RGraph> ranGraphs;                      //作用域诱导子图组
    const RGraph::Rule* Rules;
    ui rule_count = 0;
    UnionFindSet *block;
       // 利用数组并查集维护划分节点之间的关系

    int max_depth = query_graph->getVerticesCount();
    VertexID **valid_candidate = new ui *[max_depth];// 单层匹配结果

    list<queue<VertexID>> queueMap;

    //获取当前级别的全部规则
    Rules = query_graph->getRulesByLevel(cur_level, rule_count);
    block = new UnionFindSet(rule_count);  

    // 边界处理
    if (!rule_count)
    {
        Graph* new_query_graph = RGraph2Graph(query_graph);
        std::cout << "Filter candidates..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        *candidates = NULL;
        candidates_count = NULL;
        order = NULL;
        TreeNode* cfl_tree = NULL;
        TreeNode* dpiso_tree = NULL;
        TreeNode* ceci_tree = NULL;
        std::vector<std::unordered_map<VertexID, std::vector<VertexID >>> TE_Candidates;
        std::vector<std::vector<std::unordered_map<VertexID, std::vector<VertexID>>>> NTE_Candidates;
        if (input_filter_type == "LDF") {
            FilterVertices::LDFFilter(data_graph, new_query_graph, candidates, candidates_count);
        } else if (input_filter_type == "NLF") {
            FilterVertices::NLFFilter(data_graph, new_query_graph, candidates, candidates_count);
        } else if (input_filter_type == "GQL") {
            FilterVertices::GQLFilter(data_graph, new_query_graph, candidates, candidates_count);
        } else if (input_filter_type == "CFL") {
            FilterVertices::CFLFilter(data_graph, new_query_graph, candidates, candidates_count, order, cfl_tree);
        } else if (input_filter_type == "DPiso") {
            FilterVertices::DPisoFilter(data_graph, new_query_graph, candidates, candidates_count, order, dpiso_tree);
        } else if (input_filter_type == "CECI") {
            FilterVertices::CECIFilter(data_graph, new_query_graph, candidates, candidates_count, order, ceci_tree, TE_Candidates, NTE_Candidates);
        }  else {
            std::cout << "The specified filter type '" << input_filter_type << "' is not supported." << std::endl;
            exit(-1);
        }

        // Sort the candidates to support the set intersections
        if (input_filter_type != "CECI")
            FilterVertices::sortCandidates(candidates, candidates_count, new_query_graph->getVerticesCount());

        auto end = std::chrono::high_resolution_clock::now();
        double filter_vertices_time_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        // Compute the candidates false positive ratio.
    #ifdef OPTIMAL_CANDIDATES
        std::vector<ui> optimal_candidates_count;
        double avg_false_positive_ratio = FilterVertices::computeCandidatesFalsePositiveRatio(data_graph, new_query_graph, candidates,
                                                                                            candidates_count, optimal_candidates_count);
        FilterVertices::printCandidatesInfo(new_query_graph, candidates_count, optimal_candidates_count);
    #endif
        std::cout << "-----" << std::endl;
        std::cout << "Build indices..." << std::endl;

        start = std::chrono::high_resolution_clock::now();

        Edges ***edge_matrix = NULL;
        if (input_filter_type != "CECI") {
            edge_matrix = new Edges **[new_query_graph->getVerticesCount()];
            for (ui i = 0; i < new_query_graph->getVerticesCount(); ++i) {
                edge_matrix[i] = new Edges *[new_query_graph->getVerticesCount()];
            }

            BuildTable::buildTables(data_graph, new_query_graph, candidates, candidates_count, edge_matrix);
        }

        end = std::chrono::high_resolution_clock::now();
        double build_table_time_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        size_t memory_cost_in_bytes = 0;
        if (input_filter_type != "CECI") {
            memory_cost_in_bytes = BuildTable::computeMemoryCostInBytes(new_query_graph, candidates_count, edge_matrix);
            BuildTable::printTableCardinality(new_query_graph, edge_matrix);
        }
        else {
            memory_cost_in_bytes = BuildTable::computeMemoryCostInBytes(new_query_graph, candidates_count, order, ceci_tree,
                    TE_Candidates, NTE_Candidates);
            BuildTable::printTableCardinality(new_query_graph, ceci_tree, order, TE_Candidates, NTE_Candidates);
        }
        std::cout << "-----" << std::endl;
        std::cout << "Generate a matching order..." << std::endl;

        start = std::chrono::high_resolution_clock::now();

        ui* matching_order = NULL;
        ui* pivots = NULL;
        ui** weight_array = NULL;

        if (input_order_type == "QSI") {
            GenerateQueryPlan::generateQSIQueryPlan(data_graph, new_query_graph, edge_matrix, matching_order, pivots);
        } else if (input_order_type == "GQL") {
            GenerateQueryPlan::generateGQLQueryPlan(data_graph, new_query_graph, candidates_count, matching_order, pivots);
        } else if (input_order_type == "CFL") {
            if (cfl_tree == NULL) {
                int level_count;
                ui* level_offset;
                GenerateFilteringPlan::generateCFLFilterPlan(data_graph, new_query_graph, cfl_tree, order, level_count, level_offset);
                delete[] level_offset;
            }
            GenerateQueryPlan::generateCFLQueryPlan(data_graph, new_query_graph, edge_matrix, matching_order, pivots, cfl_tree, order, candidates_count);
        } else if (input_order_type == "DPiso") {
            if (dpiso_tree == NULL) {
                GenerateFilteringPlan::generateDPisoFilterPlan(data_graph, new_query_graph, dpiso_tree, order);
            }

            GenerateQueryPlan::generateDSPisoQueryPlan(new_query_graph, edge_matrix, matching_order, pivots, dpiso_tree, order,
                                                        candidates_count, weight_array);
        }
        else if (input_order_type == "CECI") {
            GenerateQueryPlan::generateCECIQueryPlan(new_query_graph, ceci_tree, order, matching_order, pivots);
        }
        else {
            std::cout << "The specified order type '" << input_order_type << "' is not supported." << std::endl;
        }

        end = std::chrono::high_resolution_clock::now();
        double generate_query_plan_time_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        GenerateQueryPlan::checkQueryPlanCorrectness(new_query_graph, matching_order, pivots);
        GenerateQueryPlan::printSimplifiedQueryPlan(new_query_graph, matching_order);


        std::cout << "-----" << std::endl;
        std::cout << "Enumerate..." << std::endl;
        size_t output_limit = std::numeric_limits<size_t>::max();
        size_t embedding_count = 0;


    #if ENABLE_QFLITER == 1
        EvaluateQuery::qfliter_bsr_graph_ = BuildTable::qfliter_bsr_graph_;
    #endif

        size_t call_count = 0;
        size_t time_limit = 0;

        start = std::chrono::high_resolution_clock::now();

        if (input_engine_type == "EXPLORE") {
            embedding_count = EvaluateQuery::exploreGraph(data_graph, new_query_graph, edge_matrix, candidates,
                                                        candidates_count, matching_order, pivots, output_limit, call_count);
        } else if (input_engine_type == "LFTJ") {
            embedding_count = EvaluateQuery::LFTJ(data_graph, new_query_graph, edge_matrix, candidates, candidates_count,
                                                matching_order, output_limit, call_count);
        } else if (input_engine_type == "GQL") {
            embedding_count = EvaluateQuery::exploreGraphQLStyle(data_graph, new_query_graph, candidates, candidates_count,
                                                                matching_order, output_limit, call_count);
        } else if (input_engine_type == "QSI") {
            embedding_count = EvaluateQuery::exploreQuickSIStyle(data_graph, new_query_graph, candidates, candidates_count,
                                                                matching_order, pivots, output_limit, call_count);
        }
        else if (input_engine_type == "DPiso") {
            embedding_count = EvaluateQuery::exploreDPisoStyle(data_graph, new_query_graph, dpiso_tree,
                                                            edge_matrix, candidates, candidates_count,
                                                            weight_array, order, output_limit,
                                                            call_count);
    //        embedding_count = EvaluateQuery::exploreDPisoRecursiveStyle(data_graph, new_query_graph, dpiso_tree,
    //                                                           edge_matrix, candidates, candidates_count,
    //                                                           weight_array, order, output_limit,
    //                                                           call_count);
        }
        else if (input_engine_type == "CECI") {
            embedding_count = EvaluateQuery::exploreCECIStyle(data_graph, new_query_graph, ceci_tree, candidates, candidates_count, TE_Candidates,
                    NTE_Candidates, order, output_limit, call_count);
        }
        else {
            std::cout << "The specified engine type '" << input_engine_type << "' is not supported." << std::endl;
            exit(-1);
        }

        end = std::chrono::high_resolution_clock::now();
        double enumeration_time_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    #ifdef DISTRIBUTION
        std::ofstream outfile (input_distribution_file_path , std::ofstream::binary);
        outfile.write((char*)EvaluateQuery::distribution_count_, sizeof(size_t) * data_graph->getVerticesCount());
        delete[] EvaluateQuery::distribution_count_;
    #endif

        std::cout << "--------------------------------------------------------------------" << std::endl;
        std::cout << "Release memories..." << std::endl;
        /**
         * Release the allocated memories.
         */
        delete[] candidates_count;
        delete[] order;
        delete[] cfl_tree;
        delete[] dpiso_tree;
        delete[] ceci_tree;
        delete[] matching_order;
        delete[] pivots;
        for (ui i = 0; i < query_graph->getVerticesCount(); ++i) {
            delete[] candidates[i];
        }
        delete[] candidates;

        if (edge_matrix != NULL) {
            for (ui i = 0; i < query_graph->getVerticesCount(); ++i) {
                for (ui j = 0; j < query_graph->getVerticesCount(); ++j) {
                    delete edge_matrix[i][j];
                }
                delete[] edge_matrix[i];
            }
            delete[] edge_matrix;
        }
        if (weight_array != NULL) {
            for (ui i = 0; i < query_graph->getVerticesCount(); ++i) {
                delete[] weight_array[i];
            }
            delete[] weight_array;
        }

        delete query_graph;
        delete data_graph;
        /**
         * End.
         */
        std::cout << "--------------------------------------------------------------------" << std::endl;
        double preprocessing_time_in_ns = filter_vertices_time_in_ns + build_table_time_in_ns + generate_query_plan_time_in_ns;
        double total_time_in_ns = preprocessing_time_in_ns + enumeration_time_in_ns;

    
        printf("Filter vertices time (seconds): %.4lf\n", NANOSECTOSEC(filter_vertices_time_in_ns));
        printf("Build table time (seconds): %.4lf\n", NANOSECTOSEC(build_table_time_in_ns));
        printf("Generate query plan time (seconds): %.4lf\n", NANOSECTOSEC(generate_query_plan_time_in_ns));
        printf("Enumerate time (seconds): %.4lf\n", NANOSECTOSEC(enumeration_time_in_ns));
        printf("Preprocessing time (seconds): %.4lf\n", NANOSECTOSEC(preprocessing_time_in_ns));
        printf("Total time (seconds): %.4lf\n", NANOSECTOSEC(total_time_in_ns));
        printf("Memory cost (MB): %.4lf\n", BYTESTOMB(memory_cost_in_bytes));
        printf("#Embeddings: %zu\n", embedding_count);
        printf("Call Count: %zu\n", call_count);
        printf("Per Call Count Time (nanoseconds): %.4lf\n", enumeration_time_in_ns / (call_count == 0 ? 1 : call_count));
        std::cout << "End." << std::endl;
        return valid_candidate;
    }

    // 递前划分处理
    /*
    1. 确定好定义域作用域分别是谁，并初始化好每个部分对应的子图
    2.并查集初始化多个跟节点，每一个根节点维护好一个队列方便遍历
    */
   DivideQGP(query_graph, Rules, domQGPs, ranQGPs, Crs);

    map<VertexID, ui **&> resultMap;
    //递归过程
    // for(auto root : domQGPs){
    //     resultMap[root.first] = MatchQGP(data_graph, root.second, cur_level+1,
    //                             candidates, candidates_count, order,
    //                             input_filter_type, input_order_type, input_engine_type);
    // }
    // for(auto root : ranQGPs){
    //     resultMap[root.first] = MatchQGP(data_graph, root.second, cur_level+1,
    //                             candidates, candidates_count, order,
    //                             input_filter_type, input_order_type, input_engine_type);
    // }

    //递归后结果组合和处理
    //valid_candidate = RuleJoin(resultMap);
    //回溯结果至上层
    return valid_candidate;

}




