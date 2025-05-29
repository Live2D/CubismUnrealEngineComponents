/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Live2DCubismFrameworkModule.h"
#include "CubismLog.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FLive2DCubismFrameworkModule"

DEFINE_LOG_CATEGORY(LogCubism)

void FLive2DCubismFrameworkModule::StartupModule()
{
	const FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("Live2DCubismSDK"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/Live2DCubismSDK"), PluginShaderDir);

	if (bIsModuleStarted)
	{
		UE_LOG(LogCubism, Verbose, TEXT("FLive2DCubismFrameworkModule::StartupModule() is already done."));
		return;
	}

	// display Live2D Cubism Core version information
	const csmVersion Version = csmGetVersion();

	const uint32 Major = static_cast<uint32>((Version & 0xFF000000) >> 24);
	const uint32 Minor = static_cast<uint32>((Version & 0x00FF0000) >> 16);
	const uint32 Patch = static_cast<uint32>((Version & 0x0000FFFF)      );
	const uint32 VersionNumber = Version;

	UE_LOG(LogCubism, Log, TEXT("Live2D Cubism Core version: %02d.%02d.%04d (%d)"), Major, Minor, Patch, VersionNumber);

	UE_LOG(LogCubism, Log, TEXT("FLive2DCubismFrameworkModule::StartupModule() is complete."));

	bIsModuleStarted = true;
}

void FLive2DCubismFrameworkModule::ShutdownModule()
{
	check(bIsModuleStarted);

	if (!bIsModuleStarted)
	{
		UE_LOG(LogCubism, Log, TEXT("FLive2DCubismFrameworkModule is not started yet."));

		return;
	}

	bIsModuleStarted = false;

	UE_LOG(LogCubism, Log, TEXT("FLive2DCubismFrameworkModule::ShutdownModule() is complete."));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLive2DCubismFrameworkModule, Live2DCubismFramework)
