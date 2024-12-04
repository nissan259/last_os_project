#include "Graph.hpp"
#include <limits>
#include <functional>
#include <queue>
#include <stack>
#include <algorithm>
#include <random>
#include <ctime>
#include <vector>
using namespace std;
class MST
{
    Graph graph;
    vector<vector<int>> mst;
    void kruskal(vector<vector<int>> &adj);
    void boruvka(vector<vector<int>> &adj);
 

public:
    MST(Graph graph, string type);
    int getWieghtMst();
    int averageDist();
    vector<int> longestPath(int s, int e);
    vector<int> shortestPath(int s, int e);
    vector<vector<int>> getMST();
   

    vector<int> reconstructPath(const vector<int> &parent_node, int start, int end);
    void dfs(const vector<vector<pair<int, int>>> &adj, int node, int parent, vector<int> &distance, vector<int> &parent_node);
    vector<vector<pair<int, int>>> buildAdjList(const vector<vector<int>> &mst);
};