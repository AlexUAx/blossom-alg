//
//  Edge.h
//  MaxPowerPair
//
//  Created by Alex Parienko on 6/1/13.
//  Copyright (c) 2013 Alex Parienko. All rights reserved.
//

#ifndef MaxPowerPair_Edge_h
#define MaxPowerPair_Edge_h

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

#endif
