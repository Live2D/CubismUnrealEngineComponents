/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


using UnrealBuildTool;
using System.IO;

public class Live2DCubismFramework : ModuleRules
{
	private string ThirdPartyPath
	{
		get
		{
			return Path.GetFullPath(Path.Combine(PluginDirectory, "Source/ThirdParty/"));
		}
	}

	public Live2DCubismFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ThirdPartyPath, "Live2DCubismCore/include"),
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects",
				"Renderer",
				"RenderCore",
				"RHI",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		bUseUnity = false;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string CubismLibPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "lib/windows/x86_64");
			string CubismDllPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "dll/windows/x86_64");
			string DllName = "Live2DCubismCore.dll";

			PublicSystemLibraryPaths.Add(CubismLibPath);
			PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "142/Live2DCubismCore_MD.lib"));
			PublicDelayLoadDLLs.Add(DllName);
			RuntimeDependencies.Add($"$(TargetOutputDir)/{DllName}", Path.Combine(CubismDllPath, DllName));
		}
		/*
		else if (Target.Platform == UnrealTargetPlatform.Win32)
		{
			string CubismLibPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "lib/windows/x86");
			string CubismDllPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "dll/windows/x86");
			string DllName = "Live2DCubismCore.dll";

			PublicSystemLibraryPaths.Add(CubismLibPath);
			PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "142/Live2DCubismCore_MD.lib"));
			PublicDelayLoadDLLs.Add(DllName);
			RuntimeDependencies.Add($"$(TargetOutputDir)/{DllName}", Path.Combine(CubismDllPath, DllName));
		}
		*/
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string CubismLibPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "lib/macos/x86_64");
			string CubismDllPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "dll/macos");
			string DllName = "libLive2DCubismCore.dylib";

			PublicSystemLibraryPaths.Add(CubismLibPath);
			PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "libLive2DCubismCore.a"));
			PublicDelayLoadDLLs.Add(DllName);
			RuntimeDependencies.Add($"$(TargetOutputDir)/{DllName}", Path.Combine(CubismDllPath, DllName));
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string CubismLibPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "lib/linux/x86_64");
			string CubismDllPath = Path.Combine(ThirdPartyPath, "Live2DCubismCore" , "dll/linux/x86_64");
			string DllName = "libLive2DCubismCore.so";

			PublicSystemLibraryPaths.Add(CubismLibPath);
			PublicAdditionalLibraries.Add(Path.Combine(CubismLibPath, "libLive2DCubismCore.a"));
			PublicDelayLoadDLLs.Add(DllName);
			RuntimeDependencies.Add($"$(TargetOutputDir)/{DllName}", Path.Combine(CubismDllPath, DllName));
		}
	}
}
