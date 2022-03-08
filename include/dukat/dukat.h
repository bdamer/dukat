#pragma once
/**
 * External header for clients using dukat library.
 */

#include "version.h"

// Engine
#include "component.h"
#include "game2.h"
#include "game3.h"
#include "gamebase.h"
#include "messenger.h"
#include "recipient.h"
#include "scene.h"
#include "scene2.h"

// UI
#include "textbutton.h"
#include "uicontrol.h"
#include "uimanager.h"

// Audio
#ifndef __ANDROID__
#include "audiocache.h"
#include "audiomanager.h"
#endif

// Input
#include "controller.h"
#include "devicemanager.h"
#include "gamepaddevice.h"
#include "inputdevice.h"
#include "inputrecorder.h"
#include "keyboarddevice.h"
#include "xboxdevice.h"

// Math
#include "eulerangles.h"
#include "geometry.h"
#include "mathutil.h"
#include "matrix2.h"
#include "matrix4.h"
#include "plane.h"
#include "quaternion.h"
#include "ray2.h"
#include "ray3.h"
#include "rect.h"
#include "transform2.h"
#include "transform3.h"
#include "transition.h"
#include "vector2.h"
#include "vector3.h"
// Disabled to avoid having to pull in boost into every client
//#include "voronoi.h"

// Collision
#include "aabb2.h"
#include "aabb3.h"
#include "boundingbody2.h"
#include "boundingbody3.h"
#include "boundingcircle.h"
#include "boundingsphere.h"
#ifndef __ANDROID__
#include "box2dmanager.h"
#endif
#include "collisionmanager2.h"
#include "obb2.h"
#include "quadtree.h"

// System
#include "animation.h"
#include "animationsequence.h"
#include "animationmanager.h"
#include "application.h"
#include "assetloader.h"
#include "bytestream.h"
#include "deferred.h"
#include "log.h"
#include "perfcounter.h"
#include "settings.h"
#include "sdlutil.h"
#include "sysutil.h"
#include "timermanager.h"
#include "window.h"

// Util
#include "bit.h"
#include "bitmapfont.h"
#include "circularbuffer.h"
#ifndef __ANDROID__
#include "dds.h"
#include "diamondsquaregenerator.h"
#endif
#include "fontcache.h"
#include "fsm.h"
#ifndef __ANDROID__
#include "heightmap.h"
#include "heightmapgenerator.h"
#include "mapgraph.h"
#include "mapshape.h"
#include "model3.h"
#include "modelconverter.h"
#include "ms3dmodel.h"
#include "octreenode.h"
#endif
#include "shape.h"
#include "string.h"
#include "textureatlas.h"
#include "textureutil.h"
#ifndef __ANDROID__
#include "voxmodel.h"
#endif

// Video
#include "blockbuilder.h"
#ifndef __ANDROID__
#include "box2deffect2.h"
#endif
#include "buffers.h"
#include "camera2.h"
#include "camera3.h"
#include "cameraeffect2.h"
#ifndef __ANDROID__
#include "clipmap.h"
#endif
#include "color.h"
#include "debugeffect2.h"
#include "draw.h"
#include "effect2.h"
#include "effect3.h"
#include "effectpass.h"
#include "environment.h"
#include "firstpersoncamera3.h"
#include "fixedcamera3.h"
#include "followercamera2.h"
#include "followercamera3.h"
#include "fullscreeneffect2.h"
#include "gridmesh.h"
#include "light.h"
#include "material.h"
#include "mesh.h"
#include "meshbuilder2.h"
#include "meshbuilder3.h"
#include "meshcache.h"
#include "meshdata.h"
#include "meshgroup.h"
#include "meshinstance.h"
#include "mirroreffect2.h"
#include "orbitallight.h"
#include "orbitcamera3.h"
#include "particle.h"
#include "particleemitter.h"
#include "particlemanager.h"
#include "renderer.h"
#include "renderer2.h"
#include "renderer3.h"
#include "renderlayer2.h"
#include "shakycameraeffect.h"
#include "shadercache.h"
#include "shaderprogram.h"
#include "shadoweffect2.h"
#include "sprite.h"
#include "surface.h"
#include "textmeshbuilder.h"
#include "textmeshinstance.h"
#include "texture.h"
#include "texturecache.h"
#include "vertextypes2.h"
#include "vertextypes3.h"
#ifndef __ANDROID__
#include "wavemesh.h"
#endif