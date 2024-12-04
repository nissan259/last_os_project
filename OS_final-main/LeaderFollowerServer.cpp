#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include "Graph.hpp"
#include "MST.hpp"
#include <csignal>

#define PORT 8080          //this the port number on which the server will listen for client connections
#define THREAD_POOL_SIZE 5  //number of worker threads in the thread pool
bool close_server=false;

// this is a struct that holds the client socket and the pointer to the graph
struct Task
{
    int CSocket; // The socket descriptor for client connection
    Graph *Pointer_Graph;  // Pointer to the graph the client will modify
};

// this is a class that implements the Leader-Follower thread pool pattern
class LeaderFollowerThreadPool
{
private:
    std::vector<std::thread> workers; // Vector of worker threads
    std::queue<Task> tasks;          // Queue of tasks to be executed 
    std::mutex queueMutex;            // Mutex to protect access to the task queue
    std::condition_variable cv;       // Condition variable to signal the worker threads
    bool stopFlag;                    // Flag to indicate that the thread pool should stop

    /**
     * Function: workerLoop
     * Each worker thread waits for a task to be assigned. Once assigned, it processes the task.
     */
    void Menue_process(int CSocket, Graph *Pointer_Graph)
    {
        char buffer[1024] = {0}; // Buffer for reading client input

        // Menu to display to the client
        std::string menu =
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
            "9. Disconnect from the server\n";
        while (true)
        {
            // Send the menu to the client
            send(CSocket, menu.c_str(), menu.size(), 0);

            // Read client input
            ssize_t bytes_read = read(CSocket, buffer, 1024);
            if (bytes_read <= 0)
            {
                std::cerr << "Client disconnected or read error" << std::endl;
                close(CSocket); // Close client socket upon disconnection or error
                return;
            }

            std::istringstream iss(buffer); // Parse the client's input
            int choice;
            iss >> choice;

            // If the input is invalid, notify the client
            if (iss.fail())
            {
                std::string errorMsg = "Invalid input. Please enter a number.\n";
                send(CSocket, errorMsg.c_str(), errorMsg.size(), 0);
                continue;
            }

            // Handle different menu choices
            switch (choice)
            {
            case 0:
            { // Close the server
                close(CSocket); // Close the client socket
                close_server=true;
                return;
            }    
            case 1:
            { // Create a new graph
                std::string Vertices_Request = "Enter the number of vertices: ";
                send(CSocket, Vertices_Request.c_str(), Vertices_Request.size(), 0);

                char Buffer[1024] = {0};
                read(CSocket, Buffer, 1024); // Read number of vertices from client
                int numVertices = std::stoi(Buffer);

                // Create an empty adjacency matrix
                std::vector<std::vector<int>> adjMat(numVertices, std::vector<int>(numVertices, 0));

                for (int i = 0; i < numVertices; ++i)
                {
                    std::string weightRequest = "Enter row " + std::to_string(i + 1) + " of the adjacency matrix: ";
                    send(CSocket, weightRequest.c_str(), weightRequest.size(), 0);

                    char rowBuffer[1024] = {0};
                    read(CSocket, rowBuffer, 1024);

                    std::istringstream rowStream(rowBuffer);
                    for (int j = 0; j < numVertices; ++j)
                    {
                        rowStream >> adjMat[i][j]; // Fill adjacency matrix from client's input
                    }
                }

                *Pointer_Graph = Graph(adjMat); // Assign the new graph to Pointer_Graph
                std::string Message_back = "Graph created successfully!\n";
                send(CSocket, Message_back.c_str(), Message_back.size(), 0);
                break;
            }
            case 2:
            { // Add an edge to the graph
                std::string Edge_Request = "Provide the edge to add (from, to, weight): ";
                send(CSocket, Edge_Request.c_str(), Edge_Request.size(), 0);

                char edgeBuffer[1024] = {0};
                read(CSocket, edgeBuffer, 1024); // Read the edge details (from, to, weight)

                std::istringstream edgeStream(edgeBuffer);
                int vertex1, Edge2, weight;
                edgeStream >> vertex1 >> Edge2>> weight;

                Pointer_Graph->addEdge(vertex1, Edge2, weight); // Add the edge to the graph
                std::string Message_back = "Edge added successfully!\n";
                send(CSocket, Message_back.c_str(), Message_back.size(), 0);
                break;
            }
            case 3:
            { // Remove an edge from the graph
                std::string Remoe_Edge_Request = "Provide the edge to remove (from, to): ";
                send(CSocket, Remoe_Edge_Request.c_str(), Remoe_Edge_Request.size(), 0);

                char edgeBuffer[1024] = {0};
                read(CSocket, edgeBuffer, 1024); // Read the edge details (from, to)

                std::istringstream edgeStream(edgeBuffer);
                int vertex1, vertex2;
                edgeStream >> vertex1 >> vertex2;

                Pointer_Graph->removeEdge(vertex1, vertex2); // Remove the edge from the graph
                std::string response = "Edge removed successfully!\n";
                send(CSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 4:
            {                                  // Get the total weight of the MST
                MST mst(*Pointer_Graph, "kruskal"); // Use Kruskal's algorithm to create the MST
                int weight = mst.getWieghtMst();
                std::string response = "Total weight of MST: " + std::to_string(weight) + "\n";
                send(CSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 5:
            { // Get the longest path in the MST
                std::string longest_path_Request = "Provide the start and end vertices for the longest path: ";
                send(CSocket, longest_path_Request.c_str(), longest_path_Request.size(), 0);

                char pathBuffer[1024] = {0};
                read(CSocket, pathBuffer, 1024); // Read the start and end vertices

                std::istringstream pathStream(pathBuffer);
                int vertex1, vertex2;
                pathStream >> vertex1 >> vertex2;
                MST mst(*Pointer_Graph, "kruskal");
                std::vector<int> path = mst.longestPath(vertex1, vertex2);
                std::string response = "Longest path in MST: ";
                for (int v : path)
                {
                    response += std::to_string(v) + " ";
                }
                response += "\n";
                send(CSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 6:
            { // Get the shortest path in the MST
                std::string shortest_path_Request= "Provide the start and end vertices for the shortest path: ";
                send(CSocket, shortest_path_Request.c_str(), shortest_path_Request.size(), 0);

                char pathBuffer[1024] = {0};
                read(CSocket, pathBuffer, 1024); // Read the start and end vertices

                std::istringstream pathStream(pathBuffer);
                int vertex1, vertex2;
                pathStream >> vertex1 >> vertex2;

                MST mst(*Pointer_Graph, "kruskal");
                auto path = mst.shortestPath(vertex1, vertex2);

                std::string response = "Shortest path from " + std::to_string(vertex1) + " to " + std::to_string(vertex2) + ": ";
                for (int v : path)
                {
                    response += std::to_string(v) + " ";
                }
                response += "\n";
                send(CSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 7:
            { // Get the average distance in the MST (as an integer)
                MST mst(*Pointer_Graph, "kruskal");
                int avgDist = static_cast<int>(mst.averageDist());
                std::string response = "Average distance in MST: " + std::to_string(avgDist) + "\n";
                send(CSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 8:
            { // Print the MST (adjacency matrix format)
                MST mst(*Pointer_Graph, "kruskal");
                std::stringstream mstStream;
                auto mstMatrix = mst.getMST();

                for (const auto &row : mstMatrix)
                {
                    for (const auto &val : row)
                    {
                        mstStream << val << " ";
                    }
                    mstStream << "\n";
                }
                std::string response = "MST Matrix:\n" + mstStream.str();
                send(CSocket, response.c_str(), response.size(), 0);
                break;
            }
            case 9:
            {                        // Exit the client connection
                close(CSocket); // Close the connection
                return;
            }
            default:
            { // Invalid choice handling
                std::string errorMsg = "Invalid choice. Please try again.\n";
                send(CSocket, errorMsg.c_str(), errorMsg.size(), 0);
                break;
            }
            }

            memset(buffer, 0, sizeof(buffer)); // Clear buffer for the next loop
        }
    }

public:
    LeaderFollowerThreadPool() : stopFlag(false)
    {
        // Create worker threads and assign the workerLoop function to each thread
        for (int i = 0; i < THREAD_POOL_SIZE; ++i)
        {
            workers.emplace_back([this]()
                                 { this->workerLoop(); });
        }
    }
    // Destructor to clean up the thread pool
    ~LeaderFollowerThreadPool()
    {
        // Set the stop flag and notify all threads to stop
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stopFlag = true;
        }
        cv.notify_all(); // Notify all threads to wake up and stop
        for (auto &worker : workers)
        {
            worker.join(); // Join each worker thread
        }
    }

    // Function to process the tasks in the task queue
    void workerLoop()
    {
        while (true)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this]()
                        { return !tasks.empty() || stopFlag; }); // Wait for a task or stop signal
                if (stopFlag && tasks.empty())
                    return; // Exit loop if stopFlag is set and no more tasks
                task = tasks.front();
                tasks.pop(); // Get the next task
            }
            Menue_process(task.CSocket, task.Pointer_Graph); // Process the task
        }
    }

    // Function to add a new task to the task queue
    void addTask(Task task)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push(task); // Add the task to the queue
            cv.notify_one();  // Notify one worker thread to process the task
        }
    }
};

/**
 * Main function: Initializes the server and sets up the thread pool for handling clients.
 */
int main()
{

    int serverFd, newSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    // Create socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed\n";
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow port reuse
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "setsockopt failed\n";
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // Set up the server address and port
    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP
    address.sin_port = htons(PORT);       // Bind to the specified port

    // Bind the socket to the address and port
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed\n";
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // Start listening for client connections (with a backlog of 3 clients)
    if (listen(serverFd, 3) < 0)
    {
        std::cerr << "Listen failed\n";
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // Initialize the Leader-Follower thread pool
    LeaderFollowerThreadPool threadPool;
    Graph graph(std::vector<std::vector<int>>{}); // Create an empty graph

    std::cout << "Server is running. Waiting for clients...\n";

    // Accept new client connections in a loop
    while ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
    {
        std::cout << "Accepted new client\n";
        Task newTask{newSocket, &graph}; // Create a new task for each client
        threadPool.addTask(newTask);     // Add the task to the thread pool
        if(close_server)
        {
            close(serverFd);
            return 0;
        }
    }


    // If accept() fails
    if (newSocket < 0)
    {
        std::cerr << "Accept failed\n";
    }

    close(serverFd); // Close the server socket
    return 0;
}
