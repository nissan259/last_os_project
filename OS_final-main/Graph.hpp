#pragma once
#include <iostream>
#include <vector>
using namespace std;
// this is a calss for a weighted directed Graph
class Graph {
    vector<vector<int>> adjMat;
    int VerticesNum;
    int numEdges;
public:
Graph(vector<vector<int>> adjMat);
vector<vector<int>> getAdjMat();
int getNumVertices();
void addEdge(int source, int destiantion, int weight);
void removeEdge(int source, int destiantion);
void setnumberofVertices(int num)
{
    VerticesNum = num;
}
};
