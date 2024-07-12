# SubgraphMatching
## Compile
Under the root directory of the project, execute the following commands to compile the source code.

```zsh
mkdir build
cd build
cmake ..
make
```

## Execute
After compiling the source code, you can find the binary file 'SubgraphMatching.out' under the 'build/matching' directory. 
Execute the binary with the following command ./SubgraphMatching.out -d data_graphs -q query_graphs
-filter method_of_filtering_candidate_vertices -order method_of_ordering_query_vertices -engine method_of_enumerating_partial_results -num number_of_embeddings,
in which -d specifies the input of the data graphs and -q specifies the input of the query graphs.
The -filter parameter gives the filtering method, the -order specifies the ordering method, and the -engine
sets the enumeration method. The -num parameter sets the maximum number of embeddings that you would like to find.
If the number of embeddings enumerated reaches the limit or all the results have been found, then the program will terminate.
Set -num as 'MAX' to find all results.

Example (Use the filtering and ordering methods of GraphQL to generate the candidate vertex sets and the matching order respectively.
Enumerate results with the set-intersection based local candidate computation method):


```zsh
./SubgraphMatching.out -d ../../test/data_graph/HPRD.graph -q ../../test/ruled_graph/query_16_1.graph -filter GQL -order GQL -engine LFTJ -num MAX
```


