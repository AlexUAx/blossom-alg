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
    
    FILE *inFile = fopen("pair.txt", "r");      //file with graph data
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

int pairForNode(int node, vector<Edge*> edges)
//find adjacency node for a given node
//vector<Edge*> edges - set of nodes for search
//int node - node to search
//return found node
{
    for (int i = 0; i < edges.size(); i++) {    //for every edge in vector
        if (edges[i]->start == node) {          //if the right edge is found
            return edges[i]->final;             //return adjacency node
        }
        if (edges[i]->final == node) {          //same for reverse order
            return edges[i]->start;
        }
    }
    return NULL;                                //if nothing is found
}

PathState buildTree(vector<vector <EdgeState>> adjMatrix, vector<Edge*> match, set<int>unusedNodes)
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
                    return oddCycle;                                                //reached an odd cycle
                    
                case NodeState::internal:
                    wrongEdges.push_back(currentEdge);                              //edge will not be used
                    break;
                    
                case NodeState::unused:
                    inTree.push_back(currentEdge);                                  //add edge to alternating tree
                    if (unusedNodes.find(currentEdge->final) != unusedNodes.end()) {//if final node of the current edge is not in the matching set
                        return augmentingPath;                                      //augmenting path found
                    } else {                                                        //if final node of the current edge is in the matching set
                        int adjToFinal = pairForNode(currentEdge->final, match);    //find adjacency node that form an edge from the matching set with current final node
                        
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
            return fullTree;    //hungarian tree is found
        }
    }
    return error;
}


vector<Edge*> findMaximumMatching(vector<vector <EdgeState>> adjMatrix)
//main function of the program
//finds maximum matching as a vector of edges for a given graph which is represented as a adjacency matrix
//vector<vector <int>> adjMatrix - adjacency matrix for current graph
//each edge in matrix represented with its current state
{
    set<int> unusedNodes;           //nodes not in the matching set
    Edge *matchEdge = NULL;         //first edge in matching set
    for (int i = 1; i < adjMatrix.size(); i++) {
        if (!matchEdge && adjMatrix[0][i] == unmatched) {
            matchEdge = new Edge;
            adjMatrix[0][i] = matched;
            adjMatrix[i][0] = matched;
            matchEdge->start = 0;
            matchEdge->final = i;
        } else {
            unusedNodes.insert(i);
        }
    }
    vector<Edge*> result;
    result.push_back(matchEdge);
    
    
    PathState state = buildTree(adjMatrix, result, unusedNodes);
    
    return result;
}

void printEdges(vector<Edge*> edges)
//print every edge in the given set
//vector<Edge*> edges - edge's data source
{
    for (int i = 0; i < edges.size(); i++) {        //for every edge
        cout << edges[i]->start + 1 << " " << edges[i]->final << endl;  //print start and final node
    }
}

int main(int argc, const char * argv[])
{
    vector<vector<EdgeState>> adjMatrix = getEdgesList();       //get adjacency matrix of the current graph
    vector<Edge*> matching = findMaximumMatching(adjMatrix);    //find set of edges which if form maximum matching
    printEdges(matching);                                       //print every edge in the given set
}