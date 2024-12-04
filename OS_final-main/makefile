#CXX = g++
#CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g -O0 -fprofile-arcs -ftest-coverage
#INCLUDES = -I.
#LIBS = -lgcov
## Source files for Pipeline server
#PIPELINE_SOURCES = Graph.cpp MST.cpp PipelineServer.cpp
#PIPELINE_OBJECTS = $(PIPELINE_SOURCES:.cpp=.o)
## Source files for Leader-Follower server
#LEADER_FOLLOWER_SOURCES = Graph.cpp MST.cpp LeaderFollowerServer.cpp
#LEADER_FOLLOWER_OBJECTS = $(LEADER_FOLLOWER_SOURCES:.cpp=.o)
## Executables
#PIPELINE_EXEC = pipeline_server
#LEADER_FOLLOWER_EXEC = leaderfollower_server
#
#.PHONY: all clean run_pipeline run_leaderfollower coverage_prep generate_report
#
## Default build compiles both servers with coverage instrumentation
#all: $(PIPELINE_EXEC) $(LEADER_FOLLOWER_EXEC)
#
## Compile the Pipeline server
#$(PIPELINE_EXEC): $(PIPELINE_OBJECTS)
#	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
#
## Compile the Leader-Follower server
#$(LEADER_FOLLOWER_EXEC): $(LEADER_FOLLOWER_OBJECTS)
#	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
#
## Run the Pipeline server
#run_pipeline: $(PIPELINE_EXEC)
#	./$(PIPELINE_EXEC)
#
## Run the Leader-Follower server
#run_leaderfollower: $(LEADER_FOLLOWER_EXEC)
#	./$(LEADER_FOLLOWER_EXEC)
#
#clean:
#	rm -f $(PIPELINE_OBJECTS) $(LEADER_FOLLOWER_OBJECTS) $(PIPELINE_EXEC) $(LEADER_FOLLOWER_EXEC)
#	rm -f *.gcno *.gcda
#	rm -rf coverage_report
#
## Prepare for coverage testing
#coverage_prep: all
#	@echo "Coverage-instrumented executables built. Run them manually to generate coverage data."
#	@echo "Use 'make generate_report' after testing to create the coverage report."
#
## Generate coverage report
#generate_report:
#	lcov --capture --directory . --output-file coverage.info
#	genhtml coverage.info --output-directory coverage_report
#	@echo "Coverage report generated in coverage_report/index.html"
#

#regualr makefile for running the servers
 CXX = g++
 CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g
 INCLUDES = -I.
 LIBS =
 
 # Source files for Pipeline server
 PIPELINE_SOURCES = Graph.cpp MST.cpp PipelineServer.cpp
 PIPELINE_OBJECTS = $(PIPELINE_SOURCES:.cpp=.o)
 
 # Source files for Leader-Follower server
 LEADER_FOLLOWER_SOURCES = Graph.cpp MST.cpp LeaderFollowerServer.cpp
 LEADER_FOLLOWER_OBJECTS = $(LEADER_FOLLOWER_SOURCES:.cpp=.o)
 
 # Executables
 PIPELINE_EXEC = pipeline_server
 LEADER_FOLLOWER_EXEC = leaderfollower_server
 
 .PHONY: all clean run_pipeline run_leaderfollower
 
 # Default build compiles both servers
 all: $(PIPELINE_EXEC) $(LEADER_FOLLOWER_EXEC)
 
 # Compile the Pipeline server
 $(PIPELINE_EXEC): $(PIPELINE_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
 
 # Compile the Leader-Follower server
 $(LEADER_FOLLOWER_EXEC): $(LEADER_FOLLOWER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
 
 # Run the Pipeline server
 run_pipeline: $(PIPELINE_EXEC)
	./$(PIPELINE_EXEC)
 
 # Run the Leader-Follower server
 run_leaderfollower: $(LEADER_FOLLOWER_EXEC)
	./$(LEADER_FOLLOWER_EXEC)
 
 clean:
	rm -f $(PIPELINE_OBJECTS) $(LEADER_FOLLOWER_OBJECTS) $(PIPELINE_EXEC) $(LEADER_FOLLOWER_EXEC)
