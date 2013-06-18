//
//  XPlaneOculus.h
//  XPlaneOculus
//
//  Created by Mike Akers on 5/13/13.
//  Copyright (c) 2013 Runway 12. All rights reserved.
//

#ifndef __XPlaneOculus__XPlaneOculus__
#define __XPlaneOculus__XPlaneOculus__


#include <iostream>

#if APL
#import <CoreFoundation/CoreFoundation.h>
#endif


#include "OVR.h"

using namespace OVR;

// *** Oculus HMD Variables

Ptr<DeviceManager>  pManager;
Ptr<SensorDevice>   pSensor;
Ptr<HMDDevice>      pHMD;
SensorFusion        SFusion;
OVR::HMDInfo        HMDInfo;


#endif /* defined(__XPlaneOculus__XPlaneOculus__) */
