// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class DatabaseSupport: ModuleRules
	{
		public DatabaseSupport(TargetInfo Target)
		{
			if ((UEBuildConfiguration.bIncludeADO == true) || (UEBuildConfiguration.bCompileAgainstEngine == true))
			{
				AddThirdPartyPrivateStaticDependencies(Target, "ADO");
				bBuildLocallyWithSNDBS = true;
			}
			
			
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"DatabaseSupport/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}