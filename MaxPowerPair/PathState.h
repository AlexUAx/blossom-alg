//
//  PathState.h
//  MaxPowerPair
//
//  Created by Alex Parienko on 6/1/13.
//  Copyright (c) 2013 Alex Parienko. All rights reserved.
//

#ifndef MaxPowerPair_PathState_h
#define MaxPowerPair_PathState_h

enum PathState {
    augmentingPath,
    oddCycle,
    fullTree,
    error
};

#endif
