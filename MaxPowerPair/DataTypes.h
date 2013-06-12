//
//  DataTypes.h
//  MaxPowerPair
//
//  Created by Alex Parienko on 6/2/13.
//  Copyright (c) 2013 Alex Parienko. All rights reserved.
//

#ifndef MaxPowerPair_DataTypes_h
#define MaxPowerPair_DataTypes_h

enum EdgeState {
    noEdge,
    unmatched,
    matched
};

struct Edge {
    int start;
    int final;
    EdgeState state;
};

enum NodeState {
    internal,
    external,
    unused,
    deadlock
};

struct NodeLabel {
    NodeState state;
    NodeLabel();
};

NodeLabel::NodeLabel() {
    state = unused;
}

enum PathState {
    augmentingPath,
    oddCycle,
    fullTree,
    error
};


#endif
