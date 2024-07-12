set(RULE_MATCHING_SRC
        Rgraph.cpp
        Rgraph.h
        MatchQGP.cpp
        DivideQGP.cpp
        BuildTable.h
        BuildTable.cpp
        EvaluateQuery.h
        EvaluateQuery.cpp
        FilterVertices.cpp
        FilterVertices.h
        GenerateQueryPlan.cpp
        GenerateQueryPlan.h
        GenerateFilteringPlan.cpp
        GenerateFilteringPlan.h
        UnionFindSet.h
        UnionFindSet.cpp
)

add_library(Rgraph SHARED
        ${RULE_MATCHING_SRC})