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
#include "Edge.h"
#include "PathState.h"
#include "NodeLabel.h"

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
        for (int i = 0; i < edges; i++) {       //scan file for every edge
            int st,en;                          //nodes of the current edge
            fscanf(inFile, "%d%d",&st,&en);     //read current edge's nodes
            matrix[st-1][en-1] = unmatched;     //set edge to "found and unused" state
            matrix[en-1][st-1] = unmatched;     //same for reversed edge
        }
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

bool isConnected(Edge *edge, vector<Edge*> match)
//check if the given edge is connected to the set of edges or is inside this set
{
    for (int i = 0; i < match.size(); i++) {
        if (match[i]->start == edge->start || match[i]->start == edge->final || match[i]->final == edge->start || match[i]->final == edge->final) {
            return true;
        }
    }
    return false;
}

vector<Edge*> computeAugmentingPath(vector<Edge*> inTree, vector<Edge*> match)
{
    vector<Edge*> matchInPath;
    for (int i = 0; i < inTree.size(); i++) {
        if (edgeInSet(inTree[i]->start, inTree[i]->final, match)) {
            matchInPath.push_back(inTree[i]);
        }
    }
    int i;
    for (i = 0; i < inTree.size(); i++) {
        if (!isConnected(inTree[i], matchInPath)) {
            vector<Edge*>isolatedEdges;
            isolatedEdges.push_back(inTree[i]);
            return isolatedEdges;
        }
    }
    return inTree;
}

vector<Edge*> computeOddCycle(vector<Edge*> inTree, vector<Edge*> match)
{
    vector<Edge*> cycle;
    cycle.push_back(*(inTree.end() - 1));
    cycle.push_back(*(inTree.end() - 2));
    for (int i = (int)inTree.size() - 3; i > 0; i++) {
        cycle.push_back(inTree[i]);
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
                case NodeState::deadlock:
                case NodeState::external:
                    inTree.push_back(currentEdge);                                  //add node to alternating tree
                    currentPath = computeOddCycle(inTree, match);
                    return oddCycle;                                                //reached an odd cycle
                    
                case NodeState::internal:
                    wrongEdges.push_back(currentEdge);                              //edge will not be used
                    break;
                    
                case NodeState::unused:
                    inTree.push_back(currentEdge);                                  //add edge to alternating tree
                    if (unusedNodes.find(currentEdge->final) != unusedNodes.end()) {//if final node of the current edge is not in the matching set
                        currentPath = computeAugmentingPath(inTree, match);
                        return augmentingPath;                                      //augmenting path found
                    } else {                                                        //if final node of the current edge is in the matching set
                        int adjToFinal = pairForNode(currentEdge->final, currentEdge->start, match);    //find adjacency node that form an edge from the matching set with current final node
                        
                        Edge *nextEdge = new Edge;                                  //create new edge
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
{
    vector<vector <EdgeState>> matrix = oldMatrix;
    vector<EdgeState> newRow(oldMatrix.size() + 1);
    for (int i = 0; i < cycle.size(); i++) {
        for (int ii = 0; ii < oldMatrix.size(); ii++) {
            if (oldMatrix[cycle[i]->final][ii] != noEdge) {
                newRow[ii] = oldMatrix[cycle[i]->final][ii];
            }
            if (oldMatrix[cycle[i]->start][ii] != noEdge) {
                newRow[ii] = oldMatrix[cycle[i]->start][ii];
            }
        }
    }
    matrix.push_back(newRow);
    
    for (int i = 0; i < oldMatrix.size(); i++) {
        matrix[i].push_back(newRow[i]);
    }
    
    for (int i = 0; i < cycle.size(); i++) {
        for (int ii = 0; ii < matrix.size(); ii++) {
            matrix[cycle[i]->start][ii] = noEdge;
            matrix[ii][cycle[i]->start] = noEdge;
            matrix[cycle[i]->final][ii] = noEdge;
            matrix[ii][cycle[i]->final] = noEdge;
        }
    }
    return matrix;
}

vector<Edge*> generateNewMatching(vector<Edge*> oldMatching, vector<Edge*> cycle, int fakeNode)
{
    vector<Edge*> matching;
    for (int i = 0; i < oldMatching.size(); i++) {
        if (!edgeInSet(oldMatching[i]->start, oldMatching[i]->final, cycle)) {
            Edge *fakeEdge = new Edge;
            fakeEdge->start = undef;
            fakeEdge->final = oldMatching[i]->final;
            if (isConnected(fakeEdge, cycle)){
                oldMatching[i]->final = fakeNode;
            } else if (isConnected(oldMatching[i], cycle)) {
                oldMatching[i]->start = fakeNode;
            }
            matching.push_back(oldMatching[i]);
        }
    }
    return matching;
}

set<int> generateNewUnused(set<int> oldUnused, vector<Edge*> cycle)
{
    set<int> unused = oldUnused;
    for (int i = 0; i < cycle.size(); i++) {
        unused.erase(cycle[i]->final);
        unused.erase(cycle[i]->start);
    }
    return unused;
}

void invertMatchingWithPath(vector<Edge*> path, vector<Edge*> &matching, set<int> &unused)
{
    for (int i = 0; i < path.size(); i++) {
        unused.erase(path[i]->start);
        unused.erase(path[i]->final);

        if (!edgeInSet(path[i]->start, path[i]->final, matching)) {
            matching.push_back(path[i]);
        } else {
            for (int ii = 0; ii < matching.size(); ii++) {
                if ((matching[ii]->start == path[i]->start && matching[ii]->final == path[i]->final) ||
                    (matching[ii]->start == path[i]->final && matching[ii]->final == path[i]->start)) {
                    matching.erase(matching.begin() + ii);
                    break;
                }
            }
        }
    }
}

void addCycleMatch(vector<Edge*>cycle, vector<Edge*> &match)
{
    for (int i = 0; i < cycle.size(); i++) {
        if (!isConnected(cycle[i], match)) {
            match.push_back(cycle[i]);
        }
    }
}

int nodeWithConnection(vector<Edge*>cycle, vector<vector <EdgeState>> adjMatrix, int node)
{
    for (int i = 0; i < cycle.size(); i++) {
        if (adjMatrix[node][cycle[i]->start] != noEdge) {
            return cycle[i]->start;
        } else if (adjMatrix[node][cycle[i]->final] != noEdge) {
            return cycle[i]->final;
        }
    }
    return undef;
}

void updateCycleEdgesInMatch(vector<Edge*>cycle, vector<vector <EdgeState>> adjMatrix, vector<Edge*> &match)
{
    int inCycle;
    int fakeNode = (int)adjMatrix.size();
    for (int i = 0; i < match.size(); i++) {
        if (match[i]->start == fakeNode) {
            inCycle = nodeWithConnection(cycle, adjMatrix, match[i]->final);
            match[i]->start = inCycle;
            break;
        } else if (match[i]->final == fakeNode) {
            inCycle = nodeWithConnection(cycle, adjMatrix, match[i]->start);
            match[i]->final = inCycle;
            break;
        }
    }
}

void maximalMatch(vector<vector <EdgeState>> adjMatrix, vector<Edge*> &match, set<int> &unused)
{
    Edge *currEdge = new Edge;
    for (int i = 0; i < adjMatrix.size(); i++) {
        for (int ii = 0; ii < adjMatrix.size(); ii++) {
            currEdge->start = i;
            currEdge->final = ii;
            if (adjMatrix[i][ii] != noEdge && !isConnected(currEdge, match)) {
                match.push_back(currEdge);
                currEdge = new Edge;
                unused.erase(i);
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
    int stage = 0;
    vector<vector<Edge*>> cycles;
    vector<vector<Edge*>> matching;
    vector<vector<vector <EdgeState>>> stagedAdjMatrix;
    vector<set<int>> unused;
    stagedAdjMatrix.push_back(adjMatrix);
    
    set<int> unusedNodes;           //nodes not in the matching set
    for (int i = 0; i < adjMatrix.size(); i++) {
        unusedNodes.insert(i);
    }
    
    if (true) {
        vector<Edge*> result;
        maximalMatch(adjMatrix, result, unusedNodes);
        matching.push_back(result);
        vector<Edge*> fake;
        cycles.push_back(fake);
        unused.push_back(unusedNodes);
    }
    
    while (unused[stage].size() > 1) {
        vector<Edge*> treePath;
        PathState state = buildTree(stagedAdjMatrix[stage], matching[stage], unused[stage], treePath);
        switch (state) {
            case augmentingPath:
                invertMatchingWithPath(treePath, matching[stage], unused[stage]);
                break;
            
            case oddCycle: {
                stage++;
                cycles.push_back(treePath);
                vector<vector<EdgeState>> newAdjMatrix = generateNewMatrix(stagedAdjMatrix[stage-1], cycles[stage]);
                stagedAdjMatrix.push_back(newAdjMatrix);
                matching.push_back(generateNewMatching(matching[stage-1], cycles[stage], (int)newAdjMatrix.size() - 1));
                unused.push_back(generateNewUnused(unused[stage-1], cycles[stage]));
                unused[stage].insert((int)stagedAdjMatrix[stage].size()-1);
                break;
            }
            
            case fullTree: {
                Edge *stEdge = treePath[0];
                if (stEdge->start == treePath[1]->start || stEdge->start == treePath[1]->final) {
                    unused[stage].erase(stEdge->final);
                } else {
                    unused[stage].erase(stEdge->start);
                }
                
                break;
            }
                
            default:
                break;
        }
    }
    
    for (int i = stage; i > 0; i--) {
        updateCycleEdgesInMatch(cycles[i], stagedAdjMatrix[i-1], matching[stage]);
        addCycleMatch(cycles[i], matching[stage]);
    }
    return matching[stage];
}

void printEdges(vector<Edge*> edges)
//print every edge in the given set
//vector<Edge*> edges - edge's data source
{
    for (int i = 0; i < edges.size(); i++) {        //for every edge
        cout << edges[i]->start + 1 << " " << edges[i]->final + 1 << endl;  //print start and final node
    }
}

int main(int argc, const char * argv[])
{
    vector<vector<EdgeState>> adjMatrix = getEdgesList();       //get adjacency matrix of the current graph
    vector<Edge*> matching = findMaximumMatching(adjMatrix);    //find set of edges which if form maximum matching
    printEdges(matching);                                       //print every edge in the given set
}