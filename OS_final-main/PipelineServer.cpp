#include <iostream>           
#include <thread>             
#include <queue>              
#include <mutex>             
#include <condition_variable> 
#include <sys/socket.h>      
#include <netinet/in.h>       
#include <unistd.h>           
#include <cstring>            
#include <sstream>            
#include <vector>             
#include "Graph.hpp"         
#include "MST.hpp"            
#include <csignal>

#define PORT 8099 
bool close_server=false;
//this is ActiveObject class that will be used to implement the pipeline pattern
class ActiveObject
{
private:
    std::thread worker;                      // Worker thread that processes the tasks
    std::queue<std::function<void()>> tasks; // Queue of tasks to be executed
    std::mutex mutex;                        // Mutex to protect access to the task queue
    std::condition_variable cv;              // Condition variable to signal the worker thread when tasks are available
    bool running = true;                     // Indicates whether the worker thread should continue running

public:
    
    /**
     * Constructor: sources the worker thread.
     * The worker thread runs in an infinite loop, waiting for tasks to be posted in the queue.
     */
    ActiveObject()
    {
        worker = std::thread([this]()
                             {
                try {
                    while (running) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mutex);
                            cv.wait(lock, [this]() { return !tasks.empty() || !running; });
                            if (!running) return;
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        task();  // Execute the task
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Exception in ActiveObject worker thread: " << e.what() << std::endl;
                } });
    }

   // Function: post
    void post(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            tasks.push(task);
            cv.notify_one();
        }
    }

    /**
     * Function: stop
     * Stops the worker thread by setting running to false and waking up the thread if it's waiting.
     */
    void stop()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            running = false;
            cv.notify_all();
        }
        if (worker.joinable())
        {
            worker.join();
        }
    }
    ~ActiveObject()
    {
        stop();
    }
};

/**
 * Function: menu
 * Generates and returns the menu string that will be displayed to the client.
 * The menu presents options to perform different actions on the graph.
 *
 * @returns A string containing the menu options.
 */
std::string menu()
{
    std::stringstream ss;
      "Options:\n"
            "0. Shut down the server\n"
            "1. Initialize a new graph (input adjacency matrix)\n"
            "2. Insert an edge (input: source, destination, weight)\n"
            "3. Delete an edge (input: source, destination)\n"
            "4. Calculate the total weight of the MST\n"
            "5. Find the longest path in the MST (input: source, end nodes)\n"
            "6. Find the shortest path in the MST (input: source, end nodes)\n"
            "7. Compute the average edge weight in the MST\n"
            "8. Display the MST (adjacency matrix format)\n"
            "9. Disconnect from the server\n";
            // Option 9: Exit the program
    return ss.str();                                               // Converts the stringstream to a string and returns it
}



/**
 * Function: Menue_process
 * Handles client interaction and requests through a socket. The function interacts with the client
 * by receiving commands, processing them, and sending back responses.
 * This is where the pipeline pattern is implemented using multiple Active Objects for different operations.
 *
 * @param newSocket The socket descriptor for communicating with the client.
 * @param Pointer_Graph A pointer to the Graph object to perform operations on.
 */
void Menue_process(int newSocket, Graph *Pointer_Graph)
{
    ActiveObject stage1, stage2, stage3;               // ActiveObject instances to handle stages of the pipeline
    static MST mst(*Pointer_Graph, "boruvka");              // MST object initialized with the current graph
    static bool mstCreated = false;                    // Tracks whether the MST has been created
    bool graphExists = Pointer_Graph->getNumVertices() > 0; // Checks if the graph exists

    while (true)
    {
        // Send the menu to the client
        std::string menu_message =  
            "Options:\n"
            "0. Shut down the server\n"
            "1. Initialize a new graph (input adjacency matrix)\n"
            "2. Insert an edge (input: source, destination, weight)\n"
            "3. Delete an edge (input: source, destination)\n"
            "4. Calculate the total weight of the MST\n"
            "5. Find the longest path in the MST (input: start, end nodes)\n"
            "6. Find the shortest path in the MST (input: start, end nodes)\n"
            "7. Compute the average edge weight in the MST\n"
            "8. Display the MST (adjacency matrix format)\n"
            "9. Disconnect from the server\n";;
        send(newSocket, menu_message.c_str(), menu_message.size(), 0);

        // Read client input
        char buffer[1024] = {0};
        ssize_t bytes_read = read(newSocket, buffer, 1024);
        if (bytes_read <= 0)
        {
            std::cerr << "" << strerror(errno) << std::endl;
            close(newSocket);
            return;
        }

        std::string input(buffer);
        std::istringstream iss(input);
        int choice;
        iss >> choice;

        if (choice != 1 && !graphExists)
        {
            std::string errorMsg = "Please create a graph first using option 1.\n";
            send(newSocket, errorMsg.c_str(), errorMsg.size(), 0);
            continue;
        }

        switch (choice)
        {
        case 0:
        {//close server
            close(newSocket);
            close_server=true;
            return;
        }    
        case 1:
        { // Create a new graph
            std::string response = "Enter the number of vertices: ";
            send(newSocket, response.c_str(), response.size(), 0);

            char matrixBuffer[1024] = {0};
            read(newSocket, matrixBuffer, 1024);
            int numVertices = std::stoi(matrixBuffer);

            std::vector<std::vector<int>> adjMat(numVertices, std::vector<int>(numVertices, 0)); // Empty adjacency matrix

            for (int i = 0; i < numVertices; ++i)
            {
                std::string rowRequest = "Enter row " + std::to_string(i + 1) + " of the adjacency matrix: ";
                send(newSocket, rowRequest.c_str(), rowRequest.size(), 0);

                char rowBuffer[1024] = {0};
                read(newSocket, rowBuffer, 1024);

                std::istringstream rowStream(rowBuffer);
                
                for (int j = 0; j < numVertices; ++j)
                {
                    int rowStream = std::stoi(matrixBuffer);
                    rowStream >> adjMat[i][j]; // Fill in the adjacency matrix
                }
            }

            *Pointer_Graph = Graph(adjMat);
            Pointer_Graph->setnumberofVertices(numVertices) ;// Create a new graph
            mstCreated = false;        // Reset the MST flag
            graphExists = true;        // Mark that the graph exists
            std::string graphCreatedResponse = "New graph created!\n";
            send(newSocket, graphCreatedResponse.c_str(), graphCreatedResponse.size(), 0);
            break;
        }
case 2:
{
    std::string addEdgeRequest = "Provide the edge to add (source, destination, weight): ";
    send(newSocket, addEdgeRequest.c_str(), addEdgeRequest.size(), 0);

    char edgeBuffer[1024] = {0};
    int bytesRead = read(newSocket, edgeBuffer, sizeof(edgeBuffer) - 1);
    if (bytesRead > 0) {
        edgeBuffer[bytesRead] = '\0'; // Null-terminate the string

        std::istringstream edgeStream(edgeBuffer);
        int from = -1, to = -1, weight = -1;

        if (edgeStream >> from >> to >> weight) {
            // Debug output
            std::cout << "Parsed values: Source = " << from
                      << ", Destination = " << to
                      << ", Weight = " << weight << std::endl;

            try {
                stage1.post([&, from, to, weight]()
                            {
                                try {
                                    Pointer_Graph->addEdge(from, to, weight);
                                    mstCreated = false; // Reset MST flag
                                    std::string response = "Edge added successfully!\n";
                                    send(newSocket, response.c_str(), response.size(), 0);
                                } catch (const std::invalid_argument &e) {
                                    std::string errorResponse = std::string("Error: ") + e.what() + "\n";
                                    send(newSocket, errorResponse.c_str(), errorResponse.size(), 0);
                                }
                            });
            } catch (...) {
                std::cerr << "Exception while posting task to ActiveObject." << std::endl;
            }
        } else {
            std::string errorResponse = "Invalid input format! Provide three integers (source, destination, weight).\n";
            send(newSocket, errorResponse.c_str(), errorResponse.size(), 0);
        }
    } else {
        std::string errorResponse = "Failed to read input. Please try again.\n";
        send(newSocket, errorResponse.c_str(), errorResponse.size(), 0);
    }
    break;
}


        case 3:
        { // Remove an edge
            std::string Remove_edgeRequest = "Provide the edge to remove (source, destination): ";
            send(newSocket, Remove_edgeRequest.c_str(), Remove_edgeRequest.size(), 0);

            char edgeBuffer[1024] = {0};
            read(newSocket, edgeBuffer, 1024);
            std::istringstream edgeStream(edgeBuffer);
            int from, to;
            edgeStream >> from >> to;

            stage1.post([&]()
                        {
                        Pointer_Graph->removeEdge(from, to);
                        mstCreated = false; // Reset MST flag
                        std::string response = "Edge removed successfully!\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 4:
        { // Get MST weight
            stage2.post([&]()
                        {
                       MST mst(*Pointer_Graph, "kruskal"); // Use Kruskal's algorithm to create the MST
                int weight = mst.getWieghtMst();
                std::string response = "Total weight of MST: " + std::to_string(weight) + "\n";
                send(newSocket, response.c_str(), response.size(), 0);
                        });
            break;
        }
        case 5:
        { // Get the longest path in the MST
            std::string Longest_pathRequest = "Provide the source and end vertices for the Longest path ";
            send(newSocket, Longest_pathRequest.c_str(), Longest_pathRequest.size(), 0);
            char pathBuffer[1024] = {0};
            read(newSocket, pathBuffer, 1024); // Read the source and end vertices
            std::istringstream pathStream(pathBuffer);
            int source, end;
            stage3.post([&]()
                        {
                    pathStream >> source >> end;
                        if (!mstCreated) {
                            mst = MST(*Pointer_Graph, "boruvka"); // Create the MST
                            mstCreated = true;
                        }
                     std::vector<int> path = mst.longestPath(source, end);
                    std::string response = "Longest path from "+ std::to_string(source) + " to " + std::to_string(end) + ": ";
                    for (int v : path) {
                        response += std::to_string(v) + " ";
                    }
                    response += "\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 6:
        { // Get the shortest path in the MST
            std::string Shortest_pathRequest = "Provide the source and end vertices for the shortest path: ";
            send(newSocket, Shortest_pathRequest.c_str(), Shortest_pathRequest.size(), 0);
            char pathBuffer[1024] = {0};
            read(newSocket, pathBuffer, 1024);
            std::istringstream pathStream(pathBuffer);
            int source, end;
            pathStream >> source >> end;

            stage3.post([&]()
                        {
                        if (!mstCreated) {
                            mst = MST(*Pointer_Graph, "boruvka"); // Create the MST
                            mstCreated = true;
                        }
                        auto path = mst.shortestPath(source, end);
                        std::string response = "Shortest path from " + std::to_string(source) + " to " + std::to_string(end) + ": ";
                        for (auto& p : path) {
                            response += std::to_string(p) + " "; // Build path response
                        }
                        response += "\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 7:
        { // Get the average distance in the MST
            stage3.post([&]()
                        {
                        if (!mstCreated) {
                            mst = MST(*Pointer_Graph, "boruvka"); // Create the MST
                            mstCreated = true;
                        }
                        int avg = mst.averageDist();
                        std::string response = "Average distance in MST: " + std::to_string(avg) + "\n";
                        send(newSocket, response.c_str(), response.size(), 0); });
            break;
        }
        case 8:
        { // Print the MST matrix
            stage3.post([&]()
                        {
                            if (!mstCreated)
                            {
                                mst = MST(*Pointer_Graph, "boruvka"); // Create the MST
                                mstCreated = true;
                            }
                            std::stringstream mstStream;   // To build the MST output
                            auto mstMatrix = mst.getMST(); // Get the MST matrix
                            for (const auto &row : mstMatrix)
                            {
                                for (const auto &val : row)
                                {
                                    mstStream << val << " "; // Append each value to the stringstream
                                }
                                mstStream << "\n"; // New line after each row
                            }
                            std::string mstOutput = "MST Matrix:\n" + mstStream.str(); // Build the final output string
                            send(newSocket, mstOutput.c_str(), mstOutput.size(), 0);   // Send the MST to the client
                        });
            break;
        }
        case 9: // Exit the program
            close(newSocket);
            return;
        default: // Invalid choice
            std::string response = "Invalid choice. Please try again.\n";
            send(newSocket, response.c_str(), response.size(), 0);
            break;
        }
    }
}

/**
 * Main Function: Initializes the server and listens for client connections.
 * The server uses a socket to accept connections and handle multiple clients in parallel.
 */
int main()
{
    
    int serverFd, newSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    // Create socket
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Allow port reuse
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "setsockopt failed" << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // source listening
    if (listen(serverFd, 3) < 0)
    {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // Create an empty graph with an empty adjacency matrix
    Graph graph{std::vector<std::vector<int>>{}}; // Empty graph

    std::cout << "Server is running. Waiting for clients..." << std::endl;

    // Accept clients and handle them
    while ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
    {
        std::cout << "Accepted new client" << std::endl;
        Menue_process(newSocket, &graph); // Handle client requests
        if(close_server)
        {
            close(serverFd);
            return 0;
        }
    }

    if (newSocket < 0)
    {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
    }

    close(serverFd);
    return 0;
}
