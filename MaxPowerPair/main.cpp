//
//  main.cpp
//  MaxPowerPair
//
//  Created by Alex Parienko on 6/1/13.
//  Copyright (c) 2013 Alex Parienko. All rights reserved.
//

#include <iostream>
#include <vector>
#include <set>
#include "DataTypes.h"

#define undef -999

using namespace std;

vector<vector <EdgeState>> getEdgesList()
//read data from file 
//first line of the file consist of two numbers (devided by single space): total number of nodes (N) and edges (M)
//next M line has two numbers (in range 1 to N): start and final node of an edge for current graph
//return adjacency matrix which has state of each edge
//matrix['start']['final'] give state for edge, in which 'start' is the start node, 'final' is the final node of the edge
//both 'start' and 'final' are integers
{
    char fileName[20];
    cout << "Enter filename" << endl;
    cin >> fileName;                            //name of the data source file
    
    int nodes;                                  //total amount of nodes
    int edges;                                  //total amount of edges
    
    FILE *inFile = fopen(fileName, "r");      //file with graph data
    if (inFile != NULL) {                       //if file is exist
        fscanf(inFile, "%d%d",&nodes,&edges);   //get nodes and edges count
        vector<vector<EdgeState>> matrix (nodes, vector<EdgeState> (nodes, noEdge));    //create adjacency matrix
        cout<<"Current connections:"<<endl;
        for (int i = 0; i < edges; i++) {       //scan file for every edge
            int st,en;                          //nodes of the current edge
            fscanf(inFile, "%d%d",&st,&en);     //read current edge's nodes
            cout<<st<<" "<<en<<endl;
            matrix[st-1][en-1] = unmatched;     //set edge to "found and unused" state
            matrix[en-1][st-1] = unmatched;     //same for reversed edge
        }
        cout<<endl;
        fclose(inFile);
        return matrix;
    } else {
        cout << "Error. Can't open file.";
        vector<vector<EdgeState>> a(0);
        return a;                               //return empty vector if an error happened
    }
}

bool edgeInSet(int start, int final, vector<Edge*> edgesSet)
//search for an edge in given vector edgesSet
// int start, int final - nodes of the edge needed to find
//return 'true' if there is an edge in given vector
//otherwise return 'false'
{
    for (int i = 0; i < edgesSet.size(); i++) {         //for every edge in set
        if ((edgesSet[i]->start == start && edgesSet[i]->final == final) ||
            (edgesSet[i]->start == final && edgesSet[i]->final == start)) {
            return true;            //edge is found
        }
    }
    return false;                   //edge is not fount
}

Edge *findNextEdge(vector<vector <EdgeState>> adjMatrix, vector<Edge*> inTree, vector<Edge*> wrongEdges, vector<NodeLabel> nodes)
//find an edge which is not used in prev. steps of forming alternating tree
//vector<vector <EdgeState>> adjMatrix - adjacency matrix of the current graph
//vector<Edge*> inTree - edges in alternating tree
//vector<Edge*> wrongEdges - edges which will not be used in the alternating tree
//vector<NodeLabel> nodes - current status of every node in alternating tree
//return an edge which will be the next edge in the alternating tree
{
    for (int i = 0; i < nodes.size(); i++) {                //for every node in graph
        for (int ii = 0; ii < adjMatrix.size(); ii++) {
            //check edge for availability
            if (adjMatrix[i][ii] != noEdge && nodes[i].state == external && !edgeInSet(i, ii, inTree) && !edgeInSet(i, ii, wrongEdges)) {
                //if available
                Edge *edge = new Edge;  //record found node
                edge->start = i;
                edge->final = ii;
                return edge;            //return found node
            }
        }
    }
    return NULL;        //if there's no any edge for the next step
}

int pairForNode(int node, int usedNode, vector<Edge*> edges)
//find adjacency node for a given node
//vector<Edge*> edges - set of nodes for search
//int node - node to search
//return found node
{
    for (int i = 0; i < edges.size(); i++) {    //for every edge in vector
        if (edges[i]->start == node && edges[i]->final != usedNode) {          //if the right edge is found
            return edges[i]->final;             //return adjacency node
        }
        if (edges[i]->final == node && edges[i]->start != usedNode) {          //same for reverse order
            return edges[i]->start;
        }
    }
    return undef;                                //if nothing is found
}

Edge* isConnected(Edge *edge, vector<Edge*> match)
//check if the given edge is connected to the set of edges or is inside this set
//Edge *edge - edge to search connection with
//vector<Edge*> match - edges for search
//return pointer to the connected edge if there's a connection beetwen edge and set of edges
{
    for (int i = 0; i < match.size(); i++) {        //check for every edge in set
        //if given edge is connected to edge from the set
        if (match[i]->start == edge->start || match[i]->start == edge->final || match[i]->final == edge->start || match[i]->final == edge->final) {
            return match[i];
        }
    }
    return NULL;       //if there's no edge from set that connected to given edge
}

void eraseEdge(Edge *edge, vector<Edge*> &edgesSet)
//delete given edge from the set of edges
{
    for (int i = 0; i < edgesSet.size(); i++) {     //check every edge from set
        if (edgesSet[i] == edge) {                  //if edge is found
            edgesSet.erase(edgesSet.begin() + i);       //erase from set
            break;                                  //stop looking forward
        }
    }
}

vector<Edge*> computeAugmentingPath(vector<Edge*> inTree, vector<Edge*> match)         
//augmenting path handler
//finds augmenting path from alternating tree
//vector<Edge*> inTree - edges in alternating tree
//vector<Edge*> match - edges used in the match
{
    //copy match edges from tree to the new container
    vector<Edge*> matchInPath;      //match edges from alternating tree
    for (int i = 0; i < inTree.size(); i++) {   //for every edge from tree
        //if edge is in match set
        if (edgeInSet(inTree[i]->start, inTree[i]->final, match)) {
            matchInPath.push_back(inTree[i]);
        }
    }
    
    int found = false;
    vector<Edge*> augmentingPath;
    Edge *currEdge = inTree[inTree.size() -1];      //start with the last edge of the tree
                                                    //this edge must be in the final path
    inTree.erase(inTree.end() -1);                  //every found edge will be removed from tree
    augmentingPath.push_back(currEdge);
    
    while (!found) {                                //while there's a match edge with connection to current edge
        Edge *matchEdge = isConnected(currEdge, matchInPath);   //find match edge connected to the current edge
        if (matchEdge) {                            //if match edge exist
            augmentingPath.push_back(matchEdge);
            eraseEdge(matchEdge, matchInPath);
            eraseEdge(matchEdge, inTree);
            currEdge = isConnected(matchEdge, inTree);  //find edge connected to the match edge
            augmentingPath.push_back(currEdge);
        } else {            //if there's no match edge
            found = true;       //we found augmenting path
        }
    }
    
    return augmentingPath;
}

vector<Edge*> computeOddCycle(vector<Edge*> inTree, vector<Edge*> match)
//odd cycle handler
//finds odd cycle from alternating tree
//vector<Edge*> inTree - edges in alternating tree
//vector<Edge*> match - edges used in the match
{
    vector<Edge*> cycle;                    //searched cycle
    cycle.push_back(*(inTree.end() - 1));   //at least 2 last edges is in the cycle
    cycle.push_back(*(inTree.end() - 2));   //
    for (int i = (int)inTree.size() - 3; i > 0; i++) {  //for every other edge
        cycle.push_back(inTree[i]);                     //add to cycle
        //if the last added edge is connected to the first cycle element
        //than cycle is found
        if (inTree[i]->start == cycle[0]->start || inTree[i]->start == cycle[0]->final || inTree[i]->final == cycle[0]->start || inTree[i]->final == cycle[0]->final) {
            break;
        }
    }
    return cycle;
}

PathState buildTree(vector<vector <EdgeState>> adjMatrix, vector<Edge*> match, set<int>unusedNodes, vector<Edge*> &currentPath)
//build alternating tree which could have augmenting path or odd cycle
//vector<vector <EdgeState>> adjMatrix - adjacency matrix of the current graph
//vector<Edge*> match - current set of matching edges
//set<int>unusedNodes - nodes not in matching edges
//return status of the tree
{
    //preparation
    int current = *(unusedNodes.begin());           //get first unused nodes / root of the tree
    vector<NodeLabel> nodes(adjMatrix.size());      //status of each node
    nodes[current].state = external;                //root is external node
    vector<Edge*> inTree;                           //edges in alternating tree
    vector<Edge*> wrongEdges;                       //edges which will not be used in the alternating tree
    
    
    //building
    while (true) {
        Edge *currentEdge = findNextEdge(adjMatrix, inTree, wrongEdges, nodes);     //get an edge with start in external node
        if (currentEdge) {                                                          //if edge exist
            switch (nodes[currentEdge->final].state) {                              //check state of the edge's final node
                case NodeState::external:
                    inTree.push_back(currentEdge);                                  //add node to alternating tree
                    currentPath = computeOddCycle(inTree, match);                   //find exact cycle in tree
                    return oddCycle;                                                //reached an odd cycle
                    
                case NodeState::internal:
                    wrongEdges.push_back(currentEdge);                              //edge will not be used
                    break;
                    
                case NodeState::unused:
                    inTree.push_back(currentEdge);                                  //add edge to alternating tree
                    if (unusedNodes.find(currentEdge->final) != unusedNodes.end()) {//if final node of the current edge is not in the matching set
                        //than augmenting path is found
                        currentPath = computeAugmentingPath(inTree, match);         //find exact augmenting path
                        return augmentingPath;                                      //augmenting path is found
                    } else {                                                        //if final node of the current edge is in the matching set
                        int adjToFinal = pairForNode(currentEdge->final, currentEdge->start, match);    //find adjacency node that form an edge from the matching set with current final node
                        
                        Edge *nextEdge = new Edge;                                  //create new edge
                        //edge repesents match edge which is connected to the alternating tree
                        nextEdge->start = currentEdge->final;                       //record data
                        nextEdge->final = adjToFinal;                               //same
                        inTree.push_back(nextEdge);                                 ////add matching edge to alternating tree
                        
                        nodes[currentEdge->final].state = NodeState::internal;      //update node's state
                        nodes[adjToFinal].state = external;                         //
                    }
                    break;
                    
                default:
                    return error;
            }
            
        } else {                //if edge is not exist
            currentPath = inTree;
            return fullTree;    //hungarian tree is found
        }
    }
    return error;
}

vector<vector <EdgeState>> generateNewMatrix(vector<vector <EdgeState>> oldMatrix, vector<Edge*> cycle)
//generate adj matrix with new fake node which replace edges in cycle
//vector<Edge*>cycle - current cycle
//vector<vector <EdgeState>> adjMatrix - previous adjacency matrix of the graph
//return adj. matrix with new node added which represents all cycle nodes (cycle nodes don't have connections now)
{
    vector<vector <EdgeState>> matrix = oldMatrix;      //new adj. matrix
    vector<EdgeState> newRow(oldMatrix.size() + 1);     //create additional row for fake node
    for (int i = 0; i < cycle.size(); i++) {            //for every cycle edge
        for (int ii = 0; ii < oldMatrix.size(); ii++) {
            if (oldMatrix[cycle[i]->final][ii] != noEdge) {     //if current cycle node has connection with some node
                newRow[ii] = oldMatrix[cycle[i]->final][ii];    //fake node has this connection too
            }
            if (oldMatrix[cycle[i]->start][ii] != noEdge) {
                newRow[ii] = oldMatrix[cycle[i]->start][ii];
            }
        }
    }
    matrix.push_back(newRow);                           //add row with fake node to the matrix
    
    for (int i = 0; i < oldMatrix.size(); i++) {        //fill up fake node column with same data
        matrix[i].push_back(newRow[i]);
    }
    
    //remove all connections of cycle nodes
    for (int i = 0; i < cycle.size(); i++) {
        for (int ii = 0; ii < matrix.size(); ii++) {
            matrix[cycle[i]->start][ii] = noEdge;
            matrix[ii][cycle[i]->start] = noEdge;
            matrix[cycle[i]->final][ii] = noEdge;
            matrix[ii][cycle[i]->final] = noEdge;
        }
    }
    return matrix;      //return new adj. matrix
}

vector<Edge*> generateNewMatching(vector<Edge*> oldMatching, vector<Edge*> cycle, int fakeNode)
//remove cycle edges from matching
//replace node in edge which have singe cycle connection, with fake node
//vector<Edge*> oldMatching - matching with cycle edges
//vector<Edge*> cycle - current cycle
//int fakeNode - node which will replace cycle nodes
{
    vector<Edge*> matching;             //new matching
    for (int i = 0; i < oldMatching.size(); i++) {      //for every match from old matching
        if (!edgeInSet(oldMatching[i]->start, oldMatching[i]->final, cycle)) {      //if match edge not in cycle
            Edge *fakeEdge = new Edge;
            fakeEdge->start = undef;
            fakeEdge->final = oldMatching[i]->final;
            //if current match edge is connected to the cycle with only one node
            if (isConnected(fakeEdge, cycle)){
                oldMatching[i]->final = fakeNode;   //replace cycle node of the match with the fake one
            } else if (isConnected(oldMatching[i], cycle)) {
                oldMatching[i]->start = fakeNode;
            }
            matching.push_back(oldMatching[i]);     //add edge to the new matching
        }
    }
    
    return matching;        //return modified matching
}

set<int> generateNewUnused(set<int> oldUnused, vector<Edge*> cycle)
//remove cycle nodes from unused set
//set<int> oldUnused - current unused set
//vector<Edge*> cycle - current cycle
//return new set without nodes from cycle
{
    set<int> unused = oldUnused;        //new set
    for (int i = 0; i < cycle.size(); i++) {    //for every cycle edge
        unused.erase(cycle[i]->final);          //remove nodes from set
        unused.erase(cycle[i]->start);
    }
    return unused;
}

void invertMatchingWithPath(vector<Edge*> path, vector<Edge*> &matching, set<int> &unused)//
//for a given path invert every edge so matched edge become unmatched, unmatched edge become matched
//vector<Edge*> path - path to invert edges in it
//vector<Edge*> &matching - current matched set
//set<int> &unused - nodes not in matching edges
{
    for (int i = 0; i < path.size(); i++) {     //for every edge from path
        unused.erase(path[i]->start);           //erase path's nodes from unused set
        unused.erase(path[i]->final);

        if (!edgeInSet(path[i]->start, path[i]->final, matching)) { //if current path edge is not matched
            matching.push_back(path[i]);                            //add current path to matching set
        } else {                                                    //if current edge is inside match
            for (int ii = 0; ii < matching.size(); ii++) {          //for every edge in match
                //if current path edge is a match edge
                if ((matching[ii]->start == path[i]->start && matching[ii]->final == path[i]->final) ||
                    (matching[ii]->start == path[i]->final && matching[ii]->final == path[i]->start)) {
                    matching.erase(matching.begin() + ii);          //remove current edge from matching set
                    break;
                }
            }
        }
    }
}

int nodeWithConnection(vector<Edge*>cycle, vector<vector <EdgeState>> adjMatrix, int node)
//find node from cycle which have connection with given node
//vector<Edge*>cycle - current cycle
//vector<vector <EdgeState>> adjMatrix - adjacency matrix of the current graph
//int node - node to search connection for
//return node from cycle which have connection with given node
{
    for (int i = 0; i < cycle.size(); i++) {        //for every edge in cycle
        if (adjMatrix[node][cycle[i]->start] != noEdge) {   //if node of the cycle edge has connection to the given node
            return cycle[i]->start;                         //return found node
        } else if (adjMatrix[node][cycle[i]->final] != noEdge) {
            return cycle[i]->final;
        }
    }
    return undef;       //if nothing is found
}

void updateCycleEdgesInMatch(vector<Edge*>cycle, vector<vector <EdgeState>> adjMatrix, vector<Edge*> &match)
//replace fake node which used in match edges with the node from cycle, so fake node will not be used in any match edge
//vector<Edge*>cycle - current cycle
//vector<vector <EdgeState>> adjMatrix - adjacency matrix of the current graph
//vector<Edge*> match - current set of matching edges
{
    int inCycle;    //node from cycle which will replace fake node
    int fakeNode = (int)adjMatrix.size();       //current fake node
    for (int i = 0; i < match.size(); i++) {        //for every match edge
        if (match[i]->start == fakeNode) {      //if fake node is used
            //find node from cycle which have connection with adj. node of the current edge
            inCycle = nodeWithConnection(cycle, adjMatrix, match[i]->final);    
            match[i]->start = inCycle;          //replace fake node with the cycle node
            break;
        } else if (match[i]->final == fakeNode) {       //same as above for different direction
            inCycle = nodeWithConnection(cycle, adjMatrix, match[i]->start);
            match[i]->final = inCycle;
            break;
        }
    }
}

void addCycleMatch(vector<Edge*>cycle, vector<Edge*> &match)
//add match edges from cycle to the global matching
//replace fake node which used in match edges with the node from cycle, so fake node will not be used in any match edge
//vector<Edge*>cycle - current cycle
//vector<Edge*> match - current set of matching edges
{
    for (int i = 0; i < cycle.size(); i++) {        //for every edge in cycle
        if (!isConnected(cycle[i], match)) {        //is current cycle edge is not connected to any of the edges from match
            match.push_back(cycle[i]);              //add current edge to global edge
        }
    }
}

void maximalMatch(vector<vector <EdgeState>> adjMatrix, vector<Edge*> &match, set<int> &unused)
//computing initial match from graph with empty match
//add edges to match so that no one edge is connected to another
//vector<vector <EdgeState>> adjMatrix - adjacency matrix of the current graph
//vector<Edge*> &match - match edges / empty at input
//set<int> &unused - set of edges which could be used in alternating tree / empty at input
{
    Edge *currEdge = new Edge;
    for (int i = 0; i < adjMatrix.size(); i++) {            //for every edge in current graph
        for (int ii = 0; ii < adjMatrix.size(); ii++) {
            currEdge->start = i;                           
            currEdge->final = ii;
            //if current edge is not connected to any of the edges in match set
            if (adjMatrix[i][ii] != noEdge && !isConnected(currEdge, match)) {
                //then we can use current edge as the match edge
                match.push_back(currEdge);      //add current edge to the match set
                currEdge = new Edge;            //allocate memory for the new edge
                unused.erase(i);                //nodes of the current set is used now
                unused.erase(ii);
            }
        }
    }
    
    
}

vector<Edge*> findMaximumMatching(vector<vector <EdgeState>> adjMatrix)
//main function of the program
//finds maximum matching as a vector of edges for a given graph which is represented as a adjacency matrix
//vector<vector <int>> adjMatrix - adjacency matrix for current graph
//each edge in matrix represented with its current state
{
    int stage = 0;                          //represent total number of 'fake' nodes
    vector<vector<Edge*>> cycles;           //odd cycles / which of cycles represented by fake node
    vector<vector<Edge*>> matching;         //matched edges for every stage
    vector<vector<vector <EdgeState>>> stagedAdjMatrix; //adj matrix for every stage
    vector<set<int>> unused;                //unused nodes for the current stage / unused is the node which is not deadblock or not in the match
    stagedAdjMatrix.push_back(adjMatrix);   //set given adj matrix as stage 0
    
    set<int> unusedNodes;           //nodes not in the matching set
    for (int i = 0; i < adjMatrix.size(); i++) {    //set every node as unused
        unusedNodes.insert(i);
    }
    
    vector<Edge*> result;           //maximal match
    maximalMatch(adjMatrix, result, unusedNodes);   //find maximal match
    matching.push_back(result);                     //set maximal match as stage 0 match
    vector<Edge*> fake;                             //empty set of edges
    cycles.push_back(fake);                         //empty cycle in stage 0
    unused.push_back(unusedNodes);                  //unused after initial match
    
    while (unused[stage].size() > 1) {              //while we have unsisited nodes
        vector<Edge*> treePath;                     //edges in alternating tree
        //build alternating tree for current stage matrix and matching
        //returns result of the execution
        PathState state = buildTree(stagedAdjMatrix[stage], matching[stage], unused[stage], treePath);
        switch (state) {            //parse result
            case augmentingPath:    //finished with augmenting path
                //this path must be inverted, so match edges in path become unmatched, and unmatched edges become matched
                invertMatchingWithPath(treePath, matching[stage], unused[stage]);   
                break;
            
            case oddCycle: {        //finished with odd cycle
                //we have odd number of edges (2k+1) with k math edges
                //this cycle must be replaced with some fake node, which will  have all connections every cycle node had
                stage++;                        //go to the next 'transformation' of the graph
                cycles.push_back(treePath);     //add found cycle to the current stage
                //replace nodes in cycle with new node
                vector<vector<EdgeState>> newAdjMatrix = generateNewMatrix(stagedAdjMatrix[stage-1], cycles[stage]);
                stagedAdjMatrix.push_back(newAdjMatrix);        //add generated matrix to the current stage
                //erase match edges in cycle from current stage matching
                //in edge with only single connection to cycle, replace cycle node to the fake node
                matching.push_back(generateNewMatching(matching[stage-1], cycles[stage], (int)newAdjMatrix.size() - 1));
                //remove nodes in the cycle from unused set
                //add fake node to this set
                unused.push_back(generateNewUnused(unused[stage-1], cycles[stage]));
                unused[stage].insert((int)stagedAdjMatrix[stage].size()-1);
                break;
            }
            
            case fullTree: {    //if we reached a deadlock
                Edge *stEdge = treePath[0];         //get first edge of the tree
                //if the final node is the root of the tree
                if (stEdge->start == treePath[1]->start || stEdge->start == treePath[1]->final) {
                    unused[stage].erase(stEdge->final);         //remove final node from set, so it can't become the root again
                } else {            //if start
                    unused[stage].erase(stEdge->start);     //remove start
                }
                
                break;
            }
                
            default:
                break;
        }
    }
    
    //exrtacting cycle edges to graph
    for (int i = stage; i > 0; i--) {           //for every stage
        //if we have match edge in which one of the node is the fake, this node must be replaced with node from cycle
        updateCycleEdgesInMatch(cycles[i], stagedAdjMatrix[i-1], matching[stage]);
        addCycleMatch(cycles[i], matching[stage]);      //add match edges from cycle
    }
    return matching[stage];     //the final result
}

void printEdges(vector<Edge*> edges)
//print every edge in the given set
//vector<Edge*> edges - edge's data source
{
    cout<<"Edges in match:"<<endl;
    for (int i = 0; i < edges.size(); i++) {        //for every edge
        cout << edges[i]->start + 1 << " " << edges[i]->final + 1 << endl;  //print start and final node
    }
}

void printList(vector < vector<EdgeState> > adjList)
{
    cout<<"Adjency list:"<<endl;
    for (int i = 0; i<adjList.size(); i++) {
        cout<<"nodes, adjecend to "<<i+1<<": ";
        for (int j = 0 ; j < adjList[i].size(); j++) {
            if(adjList[i][j]!= noEdge)
                cout << j+1<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
}

int main(int argc, const char * argv[])
{
    vector<vector<EdgeState>> adjMatrix = getEdgesList();       //get adjacency matrix of the current graph
    vector<Edge*> matching = findMaximumMatching(adjMatrix);    //find set of edges which if form maximum matching
    printList(adjMatrix);
    printEdges(matching);                                       //print every edge in the given set
}