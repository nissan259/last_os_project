#include "Graph.hpp"

Graph::Graph(vector<vector<int>> adjMat): adjMat(adjMat) {
    int VerticesNum = adjMat.size();
    numEdges = 0;
    for (int i = 0; i < VerticesNum; i++) {
        for(int j=0;j<VerticesNum;j++){
            if(adjMat.at(i).at(i)!=0){
                throw invalid_argument("The numbers on the Diagonal  must be zero");
            }
            if(adjMat.at(i).at(j)!=0){
            numEdges++;
        }
    }
}
}

int Graph::getNumVertices(){
    return VerticesNum;
}
vector<vector<int>> Graph::getAdjMat(){
    return adjMat;
}
void Graph::addEdge(int source, int destination, int weight) {
    // Validate source and destination indices
    std::cout<<source<<std::endl;
    std::cout<<destination<<std::endl;
    std::cout<<VerticesNum<<std::endl;
    
    if (source < 0 || source >= VerticesNum || destination < 0 || destination >= VerticesNum) {
        throw std::invalid_argument("Vertex index is invalid. Ensure source and destination are within bounds.");
    }
    adjMat.at(source).at(destination) = weight;
    // Optionally log the addition
    std::cout << "Edge added: (" << source << " -> " << destination << ") with weight " << weight << std::endl;
}

void Graph::removeEdge(int source, int destiantion){
    if (source <0 || source >= VerticesNum || destiantion < 0 || source >= VerticesNum||destiantion==source) {
        throw invalid_argument(" vertex index is Invalid ");
    }
    std::cout << "The edge from " << source << " to " << destiantion << " has been removed" << std::endl;
    adjMat.at(source).at(destiantion) = 0;
}   
