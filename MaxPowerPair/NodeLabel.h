//
//  NodeLabel.h
//  MaxPowerPair
//
//  Created by Alex Parienko on 6/1/13.
//  Copyright (c) 2013 Alex Parienko. All rights reserved.
//

#ifndef MaxPowerPair_NodeLabel_h
#define MaxPowerPair_NodeLabel_h

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

#endif
