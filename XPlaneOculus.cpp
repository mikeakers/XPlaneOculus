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
        
        //get the plane's position
        float planeX = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/local_x"));
        float planeY = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/local_y"));
        float planeZ = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/local_z"));
        
        //get the vector to the players head relative to the plane's center of gravity
        float headX = XPLMGetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peX"));
        float headY = XPLMGetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peY"));
        float headZ = XPLMGetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peZ"));
        
        //the planes orientation in euler angles
        float theta = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/theta")) * (M_PI / 180.0f);
        float psi = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/psi"))  * (M_PI / 180.0f);
        float phi = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/phi"))  * (M_PI / 180.0f);

        
        
        float q[4];
        
        XPLMGetDatavf(XPLMFindDataRef("sim/flightmodel/position/q"), q, 0, 4);
        
        
        
        //Make a quaternion for our rotation
        Quat<float> *planeQuat = new Quat<float>(q[1], q[2], q[3], q[0]);
        //planeQuat->Normalize();
        
        Vector3<float> *headVector = new Vector3<float>(headX, headY, headZ);
        
        
        //rotate headVector by plane quat
        Vector3<float> rotatedHeadVec = planeQuat->Rotate(*headVector);
        
        //output our final camera position and orientation
        outCameraPosition->x = planeX + rotatedHeadVec.x;
        outCameraPosition->y = planeY + rotatedHeadVec.y;
        outCameraPosition->z = planeZ + rotatedHeadVec.z;
        outCameraPosition->pitch = theta * (180.0f / M_PI);
        outCameraPosition->heading = psi * (180.0f / M_PI);
        outCameraPosition->roll = phi * (180.0f / M_PI);
        
        sprintf(buffer, "hi X:%f Y:%f Z:%f      theta:%f psii:%f phi:%f \n",
                outCameraPosition->x,
                outCameraPosition->y,
                outCameraPosition->z,
                outCameraPosition->pitch,
                outCameraPosition->heading,
                outCameraPosition->roll);
        XPLMDebugString(buffer);
	}
    
    
    return keepControl;
}


PLUGIN_API void	XPluginStop(void)
{
    XPLMDebugString("Stoping oculus plugin\n");
    XPLMReloadPlugins();
    
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
