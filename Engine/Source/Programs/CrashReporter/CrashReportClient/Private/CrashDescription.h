// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "GenericPlatformCrashContext.h"
#include "UnrealString.h"
#include "XmlFile.h"

struct FPrimaryCrashProperties;

/** PrimaryCrashProperties. Extracted from: FGenericCrashContext::SerializeContentToBuffer */
/*
	"CrashVersion"
	"ProcessId"
	"CrashGUID"
	"IsInternalBuild"
	"IsPerforceBuild"
	"IsSourceDistribution"
	"SecondsSinceStart"
	"GameName"
	"ExecutableName"
	"BuildConfiguration"
	"PlatformName"
	"PlatformNameIni"
	"PlatformFullName"
	"EngineMode"
	"EngineVersion"
	"CommandLine"
	"LanguageLCID"
	"AppDefaultLocale"
	"IsUE4Release"
	"UserName"
	"BaseDir"
	"RootDir"
	"MachineId"
	"EpicAccountId"
	"CallStack"
	"SourceContext"
	"UserDescription"
	"ErrorMessage"
	"CrashDumpMode"
	"Misc.NumberOfCores"
	"Misc.NumberOfCoresIncludingHyperthreads"
	"Misc.Is64bitOperatingSystem"
	"Misc.CPUVendor"
	"Misc.CPUBrand"
	"Misc.PrimaryGPUBrand"
	"Misc.OSVersionMajor"
	"Misc.OSVersionMinor"
	"Misc.AppDiskTotalNumberOfBytes"
	"Misc.AppDiskNumberOfFreeBytes"
	"MemoryStats.TotalPhysical"
	"MemoryStats.TotalVirtual"
	"MemoryStats.PageSize"
	"MemoryStats.TotalPhysicalGB"
	"MemoryStats.AvailablePhysical"
	"MemoryStats.AvailableVirtual"
	"MemoryStats.UsedPhysical"
	"MemoryStats.PeakUsedPhysical"
	"MemoryStats.UsedVirtual"
	"MemoryStats.PeakUsedVirtual"
	"MemoryStats.bIsOOM"
	"MemoryStats.OOMAllocationSize"
	"MemoryStats.OOMAllocationAlignment"
	"TimeofCrash"
	"bAllowToBeContacted"
 */

namespace LexicalConversion
{
	inline void FromString( ECrashDescVersions& OutValue, const TCHAR* Buffer )
	{
		OutValue = (ECrashDescVersions)FCString::Atoi( Buffer );
	}

	inline void FromString( ECrashDumpMode& OutValue, const TCHAR* Buffer )
	{
		OutValue = (ECrashDumpMode)FCString::Atoi( Buffer );
	}

	inline void FromString( FEngineVersion& OutValue, const TCHAR* Buffer )
	{
		FEngineVersion::Parse( Buffer, OutValue );
	}
}

/** Simple crash property. Only for string values. */
struct FCrashProperty
{
	friend struct FPrimaryCrashProperties;

protected:
	/** Initialization constructor. */
	FCrashProperty( const FString& InMainCategory, const FString& InSecondCategory, FPrimaryCrashProperties* InOwner );

public:
	/** Assignment operator for string. */
	FCrashProperty& operator=(const FString& NewValue);

	/** Assignment operator for TCHAR*. */
	FCrashProperty& operator=(const TCHAR* NewValue);

	/** Assignment operator for arrays. */
	FCrashProperty& operator=(const TArray<FString>& NewValue);

	/** Assignment operator for bool. */
	FCrashProperty& operator=(const bool NewValue);

	/** Assignment operator for int64. */
	FCrashProperty& operator=(const int64 NewValue);

	/** Getter for string, default. */
	const FString& AsString() const;

	/** Getter for bool. */
	bool AsBool() const;

	/** Getter for int64. */
	int64 AsInt64() const;

protected:
	/** Owner of the property. */
	FPrimaryCrashProperties* Owner;

	/** Cached value of the property. */
	mutable FString CachedValue;

	/** Main category in the crash context. */
	FString MainCategory;

	/** Second category in the crash context. */
	FString SecondCategory;

	mutable bool bSet;
};

/** Primary crash properties required by the crash report system. */
struct FPrimaryCrashProperties
{
	friend struct FCrashProperty;

	/** Version. */
	ECrashDescVersions CrashVersion;

	/** Crash dump mode. */
	ECrashDumpMode CrashDumpMode;

	/** An unique report name that this crash belongs to. Folder name. */
	FString CrashGUID;

	/**
	 * The name of the game that crashed. (AppID)
	 * @GameName	varchar(64)
	 * 
	 * FApp::GetGameName()
	 */
	FString GameName;

	/**
	 * The mode the game was in e.g. editor.
	 * @EngineMode	varchar(64)
	 * 
	 * FPlatformMisc::GetEngineMode()
	 */
	FString EngineMode;

	/**
	 * The platform that crashed e.g. Win64.
	 * @PlatformName	varchar(64)	
	 * 
	 * Last path of the directory
	 */
	FCrashProperty PlatformFullName;

	/** 
	 * Encoded engine version. (AppVersion)
	 * E.g. 4.3.0.0-2215663+UE4-Releases+4.3
	 * BuildVersion-BuiltFromCL-BranchName
	 * @EngineVersion	varchar(64)	
	 * 
	 * FEngineVersion::Current().ToString()
	 * ENGINE_VERSION_STRING
	 */
	FEngineVersion EngineVersion;

	/**
	 * Built from changelist.
	 * @ChangeListVersion	varchar(64)
	 * 
	 * BUILT_FROM_CHANGELIST
	 */
	//EngineVersion.GetChangelist()
	//uint32 BuiltFromCL;

	/**
	 * The name of the branch this game was built out of.
	 * @Branch varchar(32)
	 * 
	 * BRANCH_NAME
	 */
	//EngineVersion.GetBranch();

	/**
	 * The command line of the application that crashed.
	 * @CommandLine varchar(512)
	 * 
	 * FCommandLine::Get() 
	 */
	FCrashProperty CommandLine;

	/**
	 * The base directory where the app was running.
	 * @BaseDir varchar(512)
	 * 
	 * FPlatformProcess::BaseDir()
	 */
	FString BaseDir;

	/**
	 * The language ID the application that crashed.
	 * @LanguageExt varchar(64)
	 * 
	 * FPlatformMisc::GetDefaultLocale()
	 */
	FString AppDefaultLocale;

	/** 
	 * The name of the user that caused this crash.
	 * @UserName varchar(64)
	 * 
	 * FString( FPlatformProcess::UserName() ).Replace( TEXT( "." ), TEXT( "" ) )
	 */
	FCrashProperty UserName;

	/**
	 * The unique ID used to identify the machine the crash occurred on.
	 * @ComputerName varchar(64)
	 * 
	 * FPlatformMisc::GetMachineId().ToString( EGuidFormats::Digits )
	 */
	FCrashProperty MachineId;

	/** 
	 * The Epic account ID for the user who last used the Launcher.
	 * @EpicAccountId	varchar(64)
	 * 
	 * FPlatformMisc::GetEpicAccountId()
	 */
	FCrashProperty EpicAccountId;

	/**
	 * An array of FStrings representing the callstack of the crash.
	 * @RawCallStack	varchar(MAX)
	 * 
	 */
	FCrashProperty CallStack;

	/**
	 * An array of FStrings showing the source code around the crash.
	 * @SourceContext varchar(max)
	 * 
	 */
	FCrashProperty SourceContext;

	/**
	* An array of module's name used by the game that crashed.
	*
	*/
	FCrashProperty Modules;

	/**
	 * An array of FStrings representing the user description of the crash.
	 * @Description	varchar(512)
	 * 
	 */
	FCrashProperty UserDescription;

	/**
	 * The error message, can be assertion message, ensure message or message from the fatal error.
	 * @Summary	varchar(512)
	 * 
	 * GErrorMessage
	 */
	FCrashProperty ErrorMessage;

	/** Location of full crash dump. Displayed in the crash report frontend. */
	FCrashProperty FullCrashDumpLocation;

	/**
	 * The UTC time the crash occurred.
	 * @TimeOfCrash	datetime
	 * 
	 * FDateTime::UtcNow().GetTicks()
	 */
	FCrashProperty TimeOfCrash;

	/**
	 *	Whether the user allowed us to be contacted.
	 *	If true the following properties are retrieved from the system: UserName (for non-launcher build) and EpicAccountID.
	 *	Otherwise they will be empty.
	 */
	FCrashProperty bAllowToBeContacted;

	/**
	 * Whether this crash has a minidump file.
	 * @HasMiniDumpFile bit 
	 */
	bool bHasMiniDumpFile;

	/**
	 * Whether this crash has a log file.
	 * @HasLogFile bit 
	 */
	bool bHasLogFile;

	/** Whether this crash contains primary usable data. */
	bool bHasPrimaryData;

protected:
	/** Default constructor. */
	FPrimaryCrashProperties();

	/** Destructor. */
	~FPrimaryCrashProperties()
	{
		delete XmlFile;
	}

public:
	/** Sets new instance as the global. */
	static void Set( FPrimaryCrashProperties* NewInstance )
	{
		Singleton = NewInstance;
	}

	/**
	 * @return global instance of the primary crash properties for the currently processed/displayed crash
	 */
	static FPrimaryCrashProperties* Get()
	{
		return Singleton;
	}

	/**
	* @return false, if there is no crash
	*/
	static bool IsValid()
	{
		return Singleton != nullptr;
	}

	/** Shutdowns the global instance. */
	static void Shutdown();

	/** Whether this crash contains callstack, error message and source context thus it means that crash is complete. */
	bool HasProcessedData() const
	{
		return CallStack.AsString().Len() > 0 && ErrorMessage.AsString().Len() > 0;
	}

	/** Updates following properties: UserName, MachineID and EpicAccountID. */
	void UpdateIDs();

	/** Sends this crash for analytics. */
	void SendAnalytics();

	/** Saves the data. */
	void Save();

protected:
	/** Reads previously set XML file. */
	void ReadXML( const FString& CrashContextFilepath );

	/** Sets the CrasgGUID based on the report's path. */
	void SetCrashGUID( const FString& Filepath );

	/** Gets a crash property from the XML file. */
	template <typename Type>
	void GetCrashProperty( Type& out_ReadValue, const FString& MainCategory, const FString& SecondCategory ) const
	{
		const FXmlNode* MainNode = XmlFile->GetRootNode()->FindChildNode( MainCategory );
		if (MainNode)
		{
			const FXmlNode* CategoryNode = MainNode->FindChildNode( SecondCategory );
			if (CategoryNode)
			{
				LexicalConversion::FromString( out_ReadValue, *FGenericCrashContext::UnescapeXMLString( CategoryNode->GetContent() ) );
			}
		}
	}

	/** Sets a crash property to a new value. */
	template <typename Type>
	void SetCrashProperty( const FString& MainCategory, const FString& SecondCategory, const Type& Value )
	{
		SetCrashProperty( MainCategory, SecondCategory, *TTypeToString<Type>::ToString( Value ) );
	}

	/** Sets a crash property to a new value. */
	void SetCrashProperty( const FString& MainCategory, const FString& SecondCategory, const FString& NewValue )
	{
		FXmlNode* MainNode = XmlFile->GetRootNode()->FindChildNode( MainCategory );
		if (MainNode)
		{
			FXmlNode* CategoryNode = MainNode->FindChildNode( SecondCategory );
			const FString EscapedValue = FGenericCrashContext::EscapeXMLString( NewValue );
			if (CategoryNode)
			{
				CategoryNode->SetContent( EscapedValue );
			}
			else
			{
				MainNode->AppendChildNode( SecondCategory, EscapedValue );
			}
		}
	}

	/** Encodes multi line property to be saved as single line. */
	FString EncodeArrayStringAsXMLString( const TArray<FString>& ArrayString ) const;

	/** Reader for the xml file. */
	FXmlFile* XmlFile;

	/** Cached filepath. */
	FString XmlFilepath;

	/** Global instance. */
	static FPrimaryCrashProperties* Singleton;
};

/**
 *	Describes a unified crash, should be used by all platforms.
 *	Based on FGenericCrashContext, reads all saved properties, accessed by looking into read XML. 
 *	Still lacks some of the properties, they will be added later.
 *	Must contain the same properties as ...\CrashReportServer\CrashReportCommon\CrashDescription.cs.
 *	Contains all usable information about the crash. 
 *	
 */
struct FCrashContext : public FPrimaryCrashProperties
{
	/** Initializes instance based on specified Crash Context filepath. */
	explicit FCrashContext( const FString& CrashContextFilepath );
};

/** Crash context based on the Window Error Reporting WER files, obsolete, only for backward compatibility. */
struct FCrashWERContext : public FPrimaryCrashProperties
{
	/** Initializes instance based on specified WER XML filepath. */
	explicit FCrashWERContext( const FString& WERXMLFilepath );

	/** Initializes engine version from the separate components. */
	void InitializeEngineVersion( const FString& BuildVersion, const FString& BranchName, uint32 BuiltFromCL );
};