/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once
#include "CoreMinimal.h"
// 100–199: motion
constexpr int32 CUBISM_EXECUTION_ORDER_MOTION = 100;
constexpr int32 CUBISM_EXECUTION_ORDER_RAYCAST = 110;

// 200–299: pose
constexpr int32 CUBISM_EXECUTION_ORDER_POSE = 200;

// 300–599: expression
constexpr int32 CUBISM_EXECUTION_ORDER_EXPRESSION = 300;
constexpr int32 CUBISM_EXECUTION_ORDER_EYEBLINK = 400;
constexpr int32 CUBISM_EXECUTION_ORDER_LIPSYNC = 500;

// 600–899: physics
constexpr int32 CUBISM_EXECUTION_ORDER_PHYSICS = 600;

// 900: model
constexpr int32 CUBISM_EXECUTION_ORDER_MODEL = 900;

// 1000: render
constexpr int32 CUBISM_EXECUTION_ORDER_RENDERER = 1000;
