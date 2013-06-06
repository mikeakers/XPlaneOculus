//
//  XPlaneOculus.h
//  XPlaneOculus
//
//  Created by Mike Akers on 5/13/13.
//  Copyright (c) 2013 Runway 12. All rights reserved.
//

#include "XPlaneOculus.h"

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include "XPLMCamera.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if IBM
#include <windows.h>
#include <stdio.h>
#endif
#if LIN
#include <GL/gl.h>
#else
#if __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "OculusHandler.h"

using namespace OVR;

float lastSensorYawDegrees = 0.0f;
float lastSensorPitchDegrees = 0.0f;
float lastSensorRollDegrees = 0.0f;
float lastTheta = 0.0f;
float lastPsi = 0.0f;
float lastPhi = 0.0f;

int 	MyOrbitPlaneFunc(
                         XPLMCameraPosition_t * outCameraPosition,
                         int                  inIsLosingControl,
                         void *               inRefcon)
;

int	SDK200TestsDrawObject(
                          XPLMDrawingPhase     inPhase,
                          int                  inIsBefore,
                          void *               inRefcon);


void menuHandler(
                            void *               inMenuRef,
                            void *               inItemRef);

int keepControl;

PLUGIN_API int XPluginStart(
                            char *		outName,
                            char *		outSig,
                            char *		outDesc)
{
    
    XPLMDebugString("Starting oculus plugin\n");
    
	XPLMMenuID	PluginMenu;
	int			PluginSubMenuItem;
    
	strcpy(outName, "Instructions");
	strcpy(outSig, "Runway12.XPlaneOculus");
	strcpy(outDesc, "A plugin to enable head tracking for the Oculus Rift VR headset.");
    
    // Create our menu
	PluginSubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Oculus Plugin", NULL, 1);
    
	PluginMenu = XPLMCreateMenu("Oculus Plugin", XPLMFindPluginsMenu(), PluginSubMenuItem, menuHandler, NULL);
    XPLMAppendMenuItem(PluginMenu, "init", (void *)"init", 1);
	XPLMAppendMenuItem(PluginMenu, "start", (void *)"start", 1);
    XPLMAppendMenuItem(PluginMenu, "stop", (void *)"stop", 1);
    

//
//    System::Init(Log::ConfigureDefaultLog(LogMask_All));
//    
//    Ptr<DeviceManager> pManager;
//    Ptr<HMDDevice> pHMD;
//    pManager = *DeviceManager::Create();
//    pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();
//    
//    Ptr<SensorDevice> pSensor;
//    pSensor = *pHMD->GetSensor();
//    
//    SensorFusion SFusion;
//    if (pSensor) {
//        SFusion.AttachToSensor(pSensor);
//    }
    
    
	return 1;
}

void menuHandler(
                            void *               inMenuRef,
                            void *               inItemRef) {
    
    
    // Change all the strings
    if (strcmp((char *) inItemRef, "start") == 0) {
        XPLMDebugString("Starting head tracking\n");
        keepControl = 1;
        XPLMControlCamera(xplm_ControlCameraForever, MyOrbitPlaneFunc, NULL);
    } else if (strcmp((char *) inItemRef, "stop") == 0) {
        XPLMDebugString("Stopping head tracking\n");
        keepControl = 0;
    } else if (strcmp((char *) inItemRef, "init") == 0) {
        XPLMDebugString("Init occulus rift\n");
        System::Init(Log::ConfigureDefaultLog(LogMask_All));

        pManager = *DeviceManager::Create();
        pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();
        char buffer [1000];
        
        OVR::HMDInfo hmd;
        if (pHMD->GetDeviceInfo(&hmd)) {
            XPLMDebugString("We have an HMDinfo for a HMD!\n");

            sprintf(buffer, "DisplayDeviceName: %s", hmd.DisplayDeviceName);
            XPLMDebugString(buffer);
            
            sprintf(buffer, "InterpupillaryDistance: %f", hmd.InterpupillaryDistance);
            XPLMDebugString(buffer);            
        }

        pSensor = *pHMD->GetSensor();

        if (pSensor) {
            SFusion.AttachToSensor(pSensor);
        }
        
        Quatf hmdOrient = SFusion.GetOrientation();
        float yaw = 0.0f;
        float pitch = 0.0f;
        float roll = 0.0f;
        hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
        
        sprintf(buffer, "oculus orientation: Y:%f Y:%f z:%f\n", pitch, yaw, roll);
        XPLMDebugString(buffer);
        
        if (pSensor != NULL) {
            XPLMDebugString("Oculus intialized.\n");
        } else {
            XPLMDebugString("No sensor object created. Something may have gone wrong when initializing oculus rift :/\n");
        }
    }
}


XPLMDataRef		gPlaneX = NULL;
XPLMDataRef		gPlaneY = NULL;
XPLMDataRef		gPlaneZ = NULL;

int 	MyOrbitPlaneFunc(
                     XPLMCameraPosition_t * outCameraPosition,
                     int                  inIsLosingControl,
                     void *               inRefcon)
{
    if (outCameraPosition && !inIsLosingControl)
	{
        char buffer [1000];

        XPLMDebugString(buffer);
        
        
        Quatf hmdOrient = SFusion.GetOrientation();
        float sensorYaw = 0.0f;
        float sensorPitch = 0.0f;
        float sensorRoll = 0.0f;
        
        hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&sensorYaw, &sensorPitch, &sensorRoll);
        
        float sensorYawDegrees = sensorYaw * (180.0f / M_PI);
        float sensorPitchDegrees = sensorPitch * (180.0f / M_PI);
        float sensorRollDegrees = sensorRoll * (180.0f / M_PI);
        
        float deltaPitchDegrees = lastSensorPitchDegrees - sensorPitchDegrees;
        float deltaYawDegrees = lastSensorYawDegrees - sensorYawDegrees;
        float deltaRollDegrees = lastSensorRollDegrees - sensorRollDegrees;
        
        if (lastSensorPitchDegrees == 0.0f) {
            deltaPitchDegrees = 0.0f;
            deltaYawDegrees = 0.0f;
            deltaRollDegrees = 0.0f;
        }
        
        lastSensorPitchDegrees = sensorPitchDegrees;
        lastSensorYawDegrees = sensorYawDegrees;
        lastSensorRollDegrees = sensorRollDegrees;
        
//        sprintf(buffer, "oculus orientation: X:%f Y:%f Z:%f dX:%f dY:%f dZ:%f\n", sensorPitchDegrees,
//                                                                                    sensorYawDegrees,
//                                                                                    sensorRollDegrees,
//                                                                                    deltaPitchDegrees,
//                                                                                    deltaYawDegrees,
//                                                                                    deltaRollDegrees);
        XPLMDebugString(buffer);

        XPLMDataRef pitchDataRef = XPLMFindDataRef("sim/flightmodel/position/theta");
        XPLMDataRef rollDataRef = XPLMFindDataRef("sim/flightmodel/position/phi");
        XPLMDataRef headingDataRef = XPLMFindDataRef("sim/flightmodel/position/psi");
        
        float theta = XPLMGetDataf(pitchDataRef);
        float phi = XPLMGetDataf(rollDataRef);
        float psi = XPLMGetDataf(headingDataRef);
        
        float deltaTheta = lastTheta - theta;
        float deltaPhi = lastPhi - phi;
        float deltaPsi = lastPsi - psi;
        
        if (lastPsi == 0.0f) {
            deltaTheta = 0.0f;
            deltaPsi = 0.0f;
            deltaPhi = 0.0f;
        }
        
        lastTheta = theta;
        lastPhi = phi;
        lastPsi = psi;
        
        outCameraPosition->pitch = outCameraPosition->pitch - deltaPitchDegrees + deltaTheta;
        outCameraPosition->heading = outCameraPosition->heading + deltaYawDegrees - deltaPsi;
        outCameraPosition->roll = outCameraPosition->roll + deltaRollDegrees - deltaPhi;

        XPLMDataRef gPlaneX = XPLMFindDataRef("sim/flightmodel/position/local_x");
        XPLMDataRef gPlaneY = XPLMFindDataRef("sim/flightmodel/position/local_y");
        XPLMDataRef gPlaneZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
        
        
        XPLMDataRef gHeadX = XPLMFindDataRef("sim/aircraft/view/acf_peX");
        XPLMDataRef gHeadY = XPLMFindDataRef("sim/aircraft/view/acf_peY");
        XPLMDataRef gHeadZ = XPLMFindDataRef("sim/aircraft/view/acf_peZ");
        
        
//        outCameraPosition->x = XPLMGetDataf(gPlaneX);// + XPLMGetDataf(gHeadX);
//		outCameraPosition->y = XPLMGetDataf(gPlaneY);// + XPLMGetDataf(gHeadY);
//		outCameraPosition->z = XPLMGetDataf(gPlaneZ);// - XPLMGetDataf(gHeadZ);
//        
        
	}
    
    
    return keepControl;
}


PLUGIN_API void	XPluginStop(void)
{
    XPLMDebugString("Stoping oculus plugin\n");
    
//	if (gMenuItem == 1)
//	{
//		XPDestroyWidget(InstructionsWidget, 1);
//		gMenuItem = 0;
//	}
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void * inParam)
{
}
