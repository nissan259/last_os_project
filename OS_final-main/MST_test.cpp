#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "MST.hpp"

class TestGraph
{
public:
    static Graph createSampleGraph()
    {
        vector<vector<int>> adj_matrix = {
            {0, 2, 0, 6, 0},
            {2, 0, 3, 8, 5},
            {0, 3, 0, 0, 7},
            {6, 8, 0, 0, 9},
            {0, 5, 7, 9, 0}};
        return Graph(adj_matrix);
    }
};

TEST_CASE("MST Algorithms")
{
    Graph graph = TestGraph::createSampleGraph();

    SUBCASE("Kruskal's Algorithm")
    {
        MST mst(graph, "kruskal");
        CHECK(mst.getWieghtMst() == 16);
    }

    SUBCASE("Boruvka's Algorithm")
    {
        MST mst(graph, "boruvka");
        CHECK(mst.getWieghtMst() == 16);
    }

    SUBCASE("Invalid Algorithm")
    {
        std::ostringstream oss;
        auto cout_buff = std::cout.rdbuf(oss.rdbuf());

        MST mst(graph, "invalid");

        std::cout.rdbuf(cout_buff);
        CHECK(oss.str() == "Invalid algorithm\n");
    }
}

TEST_CASE("MST Path Finding")
{
    Graph graph = TestGraph::createSampleGraph();
    MST mst(graph, "kruskal");

    SUBCASE("Shortest Path")
    {
        vector<int> path = mst.shortestPath(0, 4);
        vector<int> expected_path = {0, 1, 4};

        CHECK(path.size() == expected_path.size());
        for (size_t i = 0; i < path.size(); ++i)
        {
            CHECK(path.at(i) == expected_path.at(i));
        }
    }

    SUBCASE("Longest Path")
    {
        vector<int> path = mst.longestPath(0, 4);
        CHECK(path.front() == 0);
        CHECK(path.back() == 4);
        CHECK(path.size() >= 3); // The path should have at least 3 nodes

        // Check if the path is valid in the MST
        for (size_t i = 0; i < path.size() - 1; ++i)
        {
            CHECK(mst.getMST()[path[i]][path[i + 1]] > 0);
        }
    }

    SUBCASE("Average Distance")
    {
        int avg_dist = mst.averageDist();
        CHECK(avg_dist == 2);
    }
}

TEST_CASE("Empty Graph")
{
    Graph empty_graph({});
    MST empty_mst(empty_graph, "kruskal");

    CHECK(empty_mst.getWieghtMst() == 0);
    CHECK(empty_mst.shortestPath(0, 1).empty());
    CHECK(empty_mst.longestPath(0, 1).empty());
    CHECK(empty_mst.averageDist() == -1);
}

class comlexTestGraph
{
public:
    static Graph createSampleGraph()
    {
        vector<vector<int>> adj_matrix = {
            {0, 2, 0, 6, 0},
            {2, 0, 3, 8, 5},
            {0, 3, 0, 0, 7},
            {6, 8, 0, 0, 9},
            {0, 5, 7, 9, 0}};
        return Graph(adj_matrix);
    }

    static Graph createLargeGraph(int size, int maxWeight)
    {
        vector<vector<int>> adj_matrix(size, vector<int>(size, 0));
        std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<> dis(1, maxWeight);

        for (int i = 0; i < size; ++i)
        {
            for (int j = i + 1; j < size; ++j)
            {
                if (dis(gen) % 3 == 0)
                { // 1/3 chance of edge existing
                    int weight = dis(gen);
                    adj_matrix[i][j] = weight;
                    adj_matrix[j][i] = weight;
                }
            }
        }
        return Graph(adj_matrix);
    }
};
bool isValidMST( Graph &original, const vector<vector<int>> &mst)
{
    int n = original.getAdjMat().size();
    vector<int> parent(n);
    for (int i = 0; i < n; i++)
        parent[i] = i;

    function<int(int)> find = [&](int x)
    {
        if (parent[x] != x)
            parent[x] = find(parent[x]);
        return parent[x];
    };

    function<void(int, int)> unite = [&](int x, int y)
    {
        int px = find(x), py = find(y);
        if (px != py)
            parent[py] = px;
    };

    int mstWeight = 0;
    int edgeCount = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (mst[i][j] > 0)
            {
                // Check if the edge exists in the original graph
                if (original.getAdjMat()[i][j] != mst[i][j]){
                    std::cout << "Edge doesn't exist in original graph" << std::endl;
                    return false;
                }
                mstWeight += mst[i][j];
                unite(i, j);
                edgeCount++;
            }
        }
    }

    // Check if the MST has exactly n-1 edges
    if (edgeCount != n - 1){
        std::cout << "Not n-1 edges" << std::endl;
        return false;
    }
    // Check if the MST is connected
    int root = find(0);
    for (int i = 1; i < n; i++)
    {
        if (find(i) != root){
            std::cout << "Not connected" << std::endl;
            return false;
        }
    }

    // Check if it's minimal
    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (original.getAdjMat()[i][j] > 0 && find(i) != find(j))
            {
                if (original.getAdjMat()[i][j] < mst[i][j])
                std::cout << "Not minimal" << std::endl;
                    return false;
            }
        }
    }

    return true;
}


TEST_CASE("Test MST on fully connected graph")
{
    int n = 20;
    vector<vector<int>> adj_matrix(n, vector<int>(n, 1));
    for (int i = 0; i < n; ++i)
        adj_matrix[i][i] = 0;

    Graph graph(adj_matrix);
    MST mst(graph, "kruskal");

    CHECK(mst.getWieghtMst() == n - 1);
    CHECK(isValidMST(graph, mst.getMST()));
}

TEST_CASE("Test MST on graph with large weight disparity")
{
    Graph graph = comlexTestGraph::createLargeGraph(50, 1000000);
    MST mst(graph, "kruskal");
    CHECK(isValidMST(graph, mst.getMST()));
}

TEST_CASE("Test path finding on complex graph")
{
    Graph graph = comlexTestGraph::createLargeGraph(100, 100);
    MST mst(graph, "boruvka");

    vector<int> shortPath = mst.shortestPath(0, 99);
    vector<int> longPath = mst.longestPath(0, 99);

    CHECK(shortPath.front() == 0);
    CHECK(shortPath.back() == 99);
    CHECK(longPath.front() == 0);
    CHECK(longPath.back() == 99);
    CHECK(longPath.size() >= shortPath.size());
}

TEST_CASE("Test MST algorithms performance")
{
    Graph graph = comlexTestGraph::createLargeGraph(50, 100);
    vector<string> algorithms = {"kruskal", "boruvka"};

    for (const auto &algo : algorithms)
    {
        auto start = std::chrono::high_resolution_clock::now();
        MST mst(graph, algo);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        CHECK(duration.count() < 5000); // Ensure each algorithm completes in less than 5 seconds
        CHECK(isValidMST(graph, mst.getMST()));
    }
}

TEST_CASE("Test on graph with self-loops")
{
    vector<vector<int>> adj_matrix = {
        {1, 2, 3},
        {2, 2, 4},
        {3, 4, 3}};
    CHECK_THROWS(Graph (adj_matrix));
}
TEST_CASE("Test on graph with large integer weights")
{
    Graph graph = comlexTestGraph::createLargeGraph(50, 1000000000); // weights up to 10^9
    MST mst(graph, "kruskal");
    CHECK(isValidMST(graph, mst.getMST()));
}
