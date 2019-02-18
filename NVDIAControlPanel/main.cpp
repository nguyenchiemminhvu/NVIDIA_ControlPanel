#include "targetver.h"
#include "nvapi.h"
#include "NvApiDriverSettings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
This function is used to print to the command line a text message
describing the nvapi error and quits
*/
void PrintError(NvAPI_Status status)
{
	NvAPI_ShortString szDesc = { 0 };
	NvAPI_GetErrorMessage(status, szDesc);
	printf(" NVAPI error: %s\n", szDesc);
}

void CheckStatus(NvAPI_Status status)
{
	if (status != NVAPI_OK)
	{
		PrintError(status);
		exit(-1);
	}
}


namespace ControlPanel
{
	namespace Profile
	{
		bool DisplayProfileContents(NvDRSSessionHandle session, NvDRSProfileHandle profile)
		{
			NvAPI_Status status;

			NVDRS_PROFILE profileInfo = { 0 };
			profileInfo.version = NVDRS_PROFILE_VER;
			status = NvAPI_DRS_GetProfileInfo(session, profile, &profileInfo);
			if (status != NVAPI_OK)
			{
				PrintError(status);
				return false;
			}

			wprintf(L"Profile name: %s\n", profileInfo.profileName);
			printf("Number of applications associated with the profile: %d\n", profileInfo.numOfApps);
			printf("Number of settings associated with the profile: %d\n", profileInfo.numOfSettings);
			printf("Is predefined: %d\n", profileInfo.isPredefined);

			if (profileInfo.numOfApps > 0)
			{
				NVDRS_APPLICATION *apps = new NVDRS_APPLICATION[profileInfo.numOfApps];
				apps[0].version = NVDRS_APPLICATION_VER;

				NvU32 appCount = profileInfo.numOfApps;
				status = NvAPI_DRS_EnumApplications(session, profile, 0, &appCount, apps);
				if (status != NVAPI_OK)
				{
					PrintError(status);
					delete[] apps;
					return false;
				}

				for (int i = 0; i < appCount; i++)
				{
					wprintf(L"Executable: %s\n", apps[i].appName);
					wprintf(L"User friendly name: %s\n", apps[i].userFriendlyName);
					printf("Is predefined: %d\n", apps[i].isPredefined);
				}

				delete[] apps;
			}

			if (profileInfo.numOfSettings > 0)
			{
				NVDRS_SETTING *settings = new NVDRS_SETTING[profileInfo.numOfSettings];
				settings[0].version = NVDRS_SETTING_VER;

				NvU32 settingCount = profileInfo.numOfSettings;
				status = NvAPI_DRS_EnumSettings(session, profile, 0, &settingCount, settings);
				if (status != NVAPI_OK)
				{
					PrintError(status);
					delete[] settings;
					return false;
				}

				for (int i = 0; i < settingCount; i++)
				{
					if (settings[i].settingLocation != NVDRS_CURRENT_PROFILE_LOCATION)
						continue;

					wprintf(L"Setting name: %s\n", settings[i].settingName);
					printf("Setting ID: %X\n", settings[i].settingId);
					printf("Is predefined: %d\n", settings[i].isCurrentPredefined);

					switch (settings[i].settingType)
					{
					case NVDRS_DWORD_TYPE:
						printf("Setting value: %X\n", settings[i].u32CurrentValue);
						break;

					case NVDRS_BINARY_TYPE:
						printf("Setting value (length=%d): ", settings[i].binaryCurrentValue.valueLength);
						for (unsigned int len = 0; len < settings[i].binaryCurrentValue.valueLength; len++)
						{
							printf(" %02X", settings[i].binaryCurrentValue.valueData[len]);
						}
						printf("\n");
						break;

					case NVDRS_WSTRING_TYPE:
						wprintf(L"Setting value: %s\n", settings[i].wszCurrentValue);
						break;
					}
				}
			}

			printf("\n");
			return true;
		}

		NvAPI_Status EnumerateAllProfile()
		{
			NvAPI_Status status;

			NvDRSSessionHandle session = NULL;
			status = NvAPI_DRS_CreateSession(&session);
			if (status != NVAPI_OK)
			{
				return status;
			}

			status = NvAPI_DRS_LoadSettings(session);
			if (status != NVAPI_OK)
			{
				return status;
			}

			NvDRSProfileHandle profile = NULL;
			unsigned int profileIndex = 0;
			while ((status = NvAPI_DRS_EnumProfiles(session, profileIndex, &profile)) == NVAPI_OK)
			{
				printf("Retrieve information from profile: %d\n", profileIndex);
				DisplayProfileContents(session, profile);

				profileIndex++;
			}

			if (status == NVAPI_END_ENUMERATION)
			{

			}
			else
			{
				if (status != NVAPI_OK)
				{
					return status;
				}
			}

			NvAPI_DRS_DestroySession(session);
			session = NULL;

			return status;
		}

		namespace BaseProfile
		{
			NvAPI_Status DisableVsync()
			{
				NvAPI_Status status;

				NvDRSSessionHandle session = NULL;
				status = NvAPI_DRS_CreateSession(&session);
				if (status != NVAPI_OK)
				{
					return status;
				}

				status = NvAPI_DRS_LoadSettings(session);
				if (status != NVAPI_OK)
				{
					return status;
				}

				NvDRSProfileHandle profile = NULL;
				status = NvAPI_DRS_GetBaseProfile(session, &profile);
				if (status != NVAPI_OK)
				{
					return status;
				}

				NVDRS_SETTING settings = { 0 };
				settings.version = NVDRS_SETTING_VER;
				settings.settingId = ESetting::VSYNCMODE_ID;
				settings.settingType = NVDRS_SETTING_TYPE::NVDRS_DWORD_TYPE;
				settings.u32CurrentValue = EValues_VSYNCMODE::VSYNCMODE_FORCEOFF;
				status = NvAPI_DRS_SetSetting(session, profile, &settings);
				if (status != NVAPI_OK)
				{
					return status;
				}

				status = NvAPI_DRS_SaveSettings(session);
				if (status != NVAPI_OK)
				{
					return status;
				}

				NvAPI_DRS_DestroySession(session);
				session = NULL;

				return status;
			}

			NvAPI_Status EnableVsync()
			{
				NvAPI_Status status;

				NvDRSSessionHandle session = NULL;
				status = NvAPI_DRS_CreateSession(&session);
				if (status != NVAPI_OK)
				{
					return status;
				}

				status = NvAPI_DRS_LoadSettings(session);
				if (status != NVAPI_OK)
				{
					return status;
				}

				NvDRSProfileHandle profile = NULL;
				status = NvAPI_DRS_GetBaseProfile(session, &profile);
				if (status != NVAPI_OK)
				{
					return status;
				}

				NVDRS_SETTING settings = { 0 };
				settings.version = NVDRS_SETTING_VER;
				settings.settingId = ESetting::VSYNCMODE_ID;
				settings.settingType = NVDRS_SETTING_TYPE::NVDRS_DWORD_TYPE;
				settings.u32CurrentValue = EValues_VSYNCMODE::VSYNCMODE_FORCEON;
				status = NvAPI_DRS_SetSetting(session, profile, &settings);
				if (status != NVAPI_OK)
				{
					return status;
				}

				status = NvAPI_DRS_SaveSettings(session);
				if (status != NVAPI_OK)
				{
					return status;
				}

				NvAPI_DRS_DestroySession(session);
				session = NULL;

				return status;
			}
		};
	};

	NvAPI_Status GetGPUs(NvPhysicalGpuHandle gpuHandlers[NVAPI_MAX_PHYSICAL_GPUS], NvU32 &gpuCount)
	{
		NvAPI_Status status;

		// Get all GPU handles
		status = NvAPI_EnumPhysicalGPUs(gpuHandlers, &gpuCount);
		if (status != NVAPI_OK)
		{
			PrintError(status);
		}

		return status;
	}

	NvAPI_Status GetConnectedDisplays(NvPhysicalGpuHandle gpuHandle, NV_GPU_DISPLAYIDS **displayIDs, NvU32 &displayIDCount)
	{
		NvAPI_Status status;

		// Get the connected display ID's array
		status = NvAPI_GPU_GetConnectedDisplayIds(gpuHandle, NULL, &displayIDCount, NV_GPU_CONNECTED_IDS_FLAG_UNCACHED);
		if (status != NVAPI_OK)
		{
			PrintError(status);
			return status;
		}

		NV_GPU_DISPLAYIDS *tempDisplayID = (NV_GPU_DISPLAYIDS *)malloc(sizeof(NV_GPU_DISPLAYIDS)*displayIDCount);
		memset(tempDisplayID, 0, displayIDCount * sizeof(NV_GPU_DISPLAYIDS));
		tempDisplayID->version = NV_GPU_DISPLAYIDS_VER;

		// second call to get the display ids
		status = NvAPI_GPU_GetConnectedDisplayIds(gpuHandle, tempDisplayID, &displayIDCount, NV_GPU_CONNECTED_IDS_FLAG_UNCACHED);
		if (status != NVAPI_OK)
		{
			PrintError(status);
			return status;
		}

		*displayIDs = tempDisplayID;
		return status;
	}

	NvAPI_Status GetConnectedDisplays(NvU32 *displayIDs, NvU32 *numDisplay)
	{
		NvAPI_Status status;

		NvPhysicalGpuHandle gpuHandlers[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
		NvU32 gpuCount = 0;
		NvU32 num = 0;

		// Get all the physical GPU handlers
		GetGPUs(gpuHandlers, gpuCount);

		for (NvU32 i = 0; i < gpuCount; i++)
		{
			NvU32 displayIDCount = 0;

			// alocations for the display ids
			NV_GPU_DISPLAYIDS *tempDisplayIDs = NULL;
			status = GetConnectedDisplays(gpuHandlers[i], &tempDisplayIDs, displayIDCount);
			if (status != NVAPI_OK)
			{
				return status;
			}

			for (NvU32 dispIndex = 0; dispIndex < displayIDCount; dispIndex++)
			{
				displayIDs[num] = tempDisplayIDs[dispIndex].displayId;
				num++;
			}

			if (tempDisplayIDs != NULL)
				free((void *)tempDisplayIDs);
		}

		*numDisplay = num;
		return status;
	}

	NvAPI_Status ApplyCustomDisplay(NV_CUSTOM_DISPLAY *custom)
	{
		NvAPI_Status status;

		NvU32 numDisplay = 0;
		NvU32 displayIDs[NVAPI_MAX_DISPLAYS] = { 0 };
		NvDisplayHandle displayHandles[NVAPI_MAX_DISPLAYS] = { 0 };
		
		status = GetConnectedDisplays(displayIDs, &numDisplay);
		if (status != NVAPI_OK)
		{
			printf("GetConnectedDisplays() failed\n");
			return status;
		}

		printf("Number of Displays in the system = %2d\n", numDisplay);

		NV_CUSTOM_DISPLAY customs[NVAPI_MAX_DISPLAYS] = { 0 };

		float rr = 60.0f;

		//timing computation (to get timing that suits the changes made)
		NV_TIMING_FLAG flag = { 0 };

		NV_TIMING_INPUT timing = { 0 };

		timing.version = NV_TIMING_INPUT_VER;

		for (NvU32 count = 0; count < numDisplay; count++)
		{
			customs[count] = *custom;

			timing.height = customs[count].height;
			timing.width = customs[count].width;
			timing.rr = rr;

			timing.flag = flag;
			timing.type = NV_TIMING_OVERRIDE_AUTO;

			status = NvAPI_DISP_GetTiming(displayIDs[0], &timing, &customs[count].timing);

			if (status != NVAPI_OK)
			{
				printf("NvAPI_DISP_GetTiming() failed = %d\n", status);		//failed to get custom display timing
				return status;
			}
		}

		printf("Custom Timing to be tried: ");
		printf("%d X %d @ %0.2f hz\n", customs[0].width, customs[0].height, rr);

		printf("NvAPI_DISP_TryCustomDisplay()\n");
		status = NvAPI_DISP_TryCustomDisplay(&displayIDs[0], numDisplay, &customs[0]); // trying to set custom display
		if (status != NVAPI_OK)
		{
			printf("NvAPI_DISP_TryCustomDisplay() failed = %d\n", status);		//failed to set custom display
			return status;
		}
		else
			printf(".....Success!\n");
		_sleep(5000);

		printf("NvAPI_DISP_SaveCustomDisplay()\n");

		status = NvAPI_DISP_SaveCustomDisplay(&displayIDs[0], numDisplay, true, true);
		if (status != NVAPI_OK)
		{
			printf("NvAPI_DISP_SaveCustomDisplay() failed = %d\n", status);		//failed to save custom display
			return status;
		}
		else
			printf(".....Success!\n");

		_sleep(5000);

		printf("NvAPI_DISP_RevertCustomDisplayTrial()\n");

		// Revert the new custom display settings tried.
		status = NvAPI_DISP_RevertCustomDisplayTrial(&displayIDs[0], 1);
		if (status != NVAPI_OK)
		{
			printf("NvAPI_DISP_RevertCustomDisplayTrial() failed = %d\n", status);		//failed to revert custom display trail
			return status;
		}
		else
		{
			printf(".....Success!\n");
			_sleep(5000);

		}

		return status;	// Custom Display.
	}

	NvAPI_Status ColorControl(NV_COLOR_CMD command, NV_COLOR_DATA *data = NULL)
	{
		NvAPI_Status status;

		NV_GPU_DISPLAYIDS *displayIDs = NULL;
		NvU32 displayIDCount = 0;

		NvPhysicalGpuHandle gpuHandlers[NVAPI_MAX_PHYSICAL_GPUS] = { 0 };
		NvU32 gpuCount = 0;

		status = GetGPUs(gpuHandlers, gpuCount);
		if (status != NVAPI_OK)
		{
			return status;
		}

		for (NvU32 i = 0; i < gpuCount; i++)
		{
			status = GetConnectedDisplays(gpuHandlers[i], &displayIDs, displayIDCount);
			if (status != NVAPI_OK)
			{
				return status;
			}

			NV_COLOR_DATA colorData;
			memset(&colorData, 0, sizeof(NV_COLOR_DATA));
			colorData.version = NV_COLOR_DATA_VER;
			colorData.size = sizeof(NV_COLOR_DATA);
			colorData.cmd = NV_COLOR_CMD_GET;

			for (NvU32 j = 0; j < displayIDCount; j++)
			{
				status = NvAPI_Disp_ColorControl(displayIDs[j].displayId, data ? data : &colorData);
				if (status != NVAPI_OK)
				{
					int a = 0;
				}
			}
		}

		return status;
	}

	NvAPI_Status AllocateAndGetDisplayConfig(NvU32* pathInfoCount, NV_DISPLAYCONFIG_PATH_INFO** pPathInfo)
	{
		NvAPI_Status status;

		// Retrieve the display path information
		NvU32 pathCount = 0;
		NV_DISPLAYCONFIG_PATH_INFO *pathInfo = NULL;

		status = NvAPI_DISP_GetDisplayConfig(&pathCount, NULL);
		if (status != NVAPI_OK)
			return status;

		pathInfo = (NV_DISPLAYCONFIG_PATH_INFO*)malloc(pathCount * sizeof(NV_DISPLAYCONFIG_PATH_INFO));
		if (!pathInfo)
		{
			return NVAPI_OUT_OF_MEMORY;
		}

		memset(pathInfo, 0, pathCount * sizeof(NV_DISPLAYCONFIG_PATH_INFO));
		for (NvU32 i = 0; i < pathCount; i++)
		{
			pathInfo[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER;
		}

		// Retrieve the targetInfo counts
		status = NvAPI_DISP_GetDisplayConfig(&pathCount, pathInfo);
		if (status != NVAPI_OK)
		{
			return status;
		}

		for (NvU32 i = 0; i < pathCount; i++)
		{
			// Allocate the source mode info

			if (pathInfo[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER1 || pathInfo[i].version == NV_DISPLAYCONFIG_PATH_INFO_VER2)
			{
				pathInfo[i].sourceModeInfo = (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
			}
			else
			{

#ifdef NV_DISPLAYCONFIG_PATH_INFO_VER3
				pathInfo[i].sourceModeInfo = (NV_DISPLAYCONFIG_SOURCE_MODE_INFO*)malloc(pathInfo[i].sourceModeInfoCount * sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));
#endif

			}
			if (pathInfo[i].sourceModeInfo == NULL)
			{
				return NVAPI_OUT_OF_MEMORY;
			}
			memset(pathInfo[i].sourceModeInfo, 0, sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO));

			// Allocate the target array
			pathInfo[i].targetInfo = (NV_DISPLAYCONFIG_PATH_TARGET_INFO*)malloc(pathInfo[i].targetInfoCount * sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
			if (pathInfo[i].targetInfo == NULL)
			{
				return NVAPI_OUT_OF_MEMORY;
			}
			// Allocate the target details
			memset(pathInfo[i].targetInfo, 0, pathInfo[i].targetInfoCount * sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO));
			for (NvU32 j = 0; j < pathInfo[i].targetInfoCount; j++)
			{
				pathInfo[i].targetInfo[j].details = (NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO*)malloc(sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
				memset(pathInfo[i].targetInfo[j].details, 0, sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO));
				pathInfo[i].targetInfo[j].details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER;
			}
		}

		// Retrieve the full path info
		status = NvAPI_DISP_GetDisplayConfig(&pathCount, pathInfo);
		if (status != NVAPI_OK)
		{
			return status;
		}

		*pathInfoCount = pathCount;
		*pPathInfo = pathInfo;
		return NVAPI_OK;
	}

	NvAPI_Status ShowCurrentDisplayConfigs()
	{
		NvAPI_Status status;

		NV_DISPLAYCONFIG_PATH_INFO *pathInfo = NULL;
		NvU32 pathCount = 0;
		NV_DISPLAYCONFIG_PATH_INFO *pathInfo1 = NULL;
		NvU32 nDisplayIds = 0;
		NvU32 physicalGpuCount = 0;
		NV_GPU_DISPLAYIDS* pDisplayIds = NULL;
		NvPhysicalGpuHandle hPhysicalGpu[NVAPI_MAX_PHYSICAL_GPUS];

		for (NvU32 PhysicalGpuIndex = 0; PhysicalGpuIndex < NVAPI_MAX_PHYSICAL_GPUS; PhysicalGpuIndex++)
		{
			hPhysicalGpu[PhysicalGpuIndex] = 0;
		}

		printf("\nThe currently running display configuration is as follows:\n");
		for (NvU32 count = 0; count < 60; count++)	printf("#");
		printf("\nGPU index\tGPU ID\t\tDisplayIDs of displays\n");

		// Enumerate the physical GPU handle
		status = NvAPI_EnumPhysicalGPUs(hPhysicalGpu, &physicalGpuCount);
		if (status != NVAPI_OK)
		{
			printf("Cannot enumerate GPUs in the system...\n");
			return status;
		}
		// get the display ids of connected displays
		NvU32 DisplayGpuIndex = 0;

		for (NvU32 GpuIndex = 0; GpuIndex < physicalGpuCount; GpuIndex++)
		{
			status = NvAPI_GPU_GetConnectedDisplayIds(hPhysicalGpu[GpuIndex], pDisplayIds, &nDisplayIds, 0);
			if ((status == NVAPI_OK) && nDisplayIds)
			{
				DisplayGpuIndex = GpuIndex;
				pDisplayIds = (NV_GPU_DISPLAYIDS*)malloc(nDisplayIds * sizeof(NV_GPU_DISPLAYIDS));
				if (pDisplayIds)
				{
					memset(pDisplayIds, 0, nDisplayIds * sizeof(NV_GPU_DISPLAYIDS));
					pDisplayIds[GpuIndex].version = NV_GPU_DISPLAYIDS_VER;
					status = NvAPI_GPU_GetConnectedDisplayIds(hPhysicalGpu[DisplayGpuIndex], pDisplayIds, &nDisplayIds, 0);
					for (NvU32 DisplayIdIndex = 0; DisplayIdIndex < nDisplayIds; DisplayIdIndex++)
					{
						printf("%2d\t\t0x%x\t0x%x", GpuIndex, hPhysicalGpu[DisplayGpuIndex], pDisplayIds[DisplayIdIndex].displayId);
						if (!pDisplayIds[DisplayIdIndex].displayId)printf("(NONE)");
						printf("\n");
					}
				}
			}
			else
			{
				printf("%2d\t\t0x%x\n", GpuIndex, hPhysicalGpu[GpuIndex]);
			}
		}
		for (NvU32 count = 0; count < 60; count++)printf("#");
		printf("\n");

		status = AllocateAndGetDisplayConfig(&pathCount, &pathInfo);
		if (status != NVAPI_OK)
		{
			printf("AllocateAndGetDisplayConfig failed!\n");
			return status;
		}
		if (pathCount == 1)
		{
			if (pathInfo[0].targetInfoCount == 1) // if pathCount = 1 and targetInfoCount =1 it is Single Mode
				printf("Single MODE\n");
			else if (pathInfo[0].targetInfoCount > 1) // if pathCount >= 1 and targetInfoCount >1 it is Clone Mode
				printf("Monitors in Clone MODE\n");
		}
		else
		{
			for (NvU32 PathIndex = 0; PathIndex < pathCount; PathIndex++)
			{
				if (pathInfo[PathIndex].targetInfoCount == 1)
				{
					printf("Monitor with Display Id 0x%x is in Extended MODE\n", pathInfo[PathIndex].targetInfo->displayId);
					// if pathCount > 1 and targetInfoCount =1 it is Extended Mode
				}
				else if (pathInfo[PathIndex].targetInfoCount > 1)
				{
					for (NvU32 TargetIndex = 0; TargetIndex < pathInfo[PathIndex].targetInfoCount; TargetIndex++)
					{
						// if pathCount >= 1 and targetInfoCount > 1 it is Clone Mode
						printf("Monitors with Display Id 0x%x are in Clone MODE\n", pathInfo[PathIndex].targetInfo[TargetIndex].displayId);
					}
				}
			}
		}

		return status;
	}

	NvAPI_Status RestoreAllDefaults()
	{
		NvAPI_Status status;

		NvDRSSessionHandle session = NULL;
		status = NvAPI_DRS_CreateSession(&session);
		if (status != NVAPI_OK)
		{
			return status;
		}

		status = NvAPI_DRS_RestoreAllDefaults(session);
		if (status != NVAPI_OK)
		{
			return status;
		}

		status = NvAPI_DRS_SaveSettings(session);
		if (status != NVAPI_OK)
		{
			return status;
		}

		NvAPI_DRS_DestroySession(session);
		session = NULL;

		return status;
	}
};


namespace Examples
{
	void EnumerateAllProfile()
	{
		NvAPI_Status status = ControlPanel::Profile::EnumerateAllProfile();
		CheckStatus(status);
	}

	void DisableVSync()
	{
		NvAPI_Status status = ControlPanel::Profile::BaseProfile::DisableVsync();
		CheckStatus(status);
	}

	void CustomizeDisplay()
	{
		NV_CUSTOM_DISPLAY custom;
		custom.version = NV_CUSTOM_DISPLAY_VER;
		custom.width = 1024;
		custom.height = 999;
		custom.depth = 32;
		custom.colorFormat = NV_FORMAT_A8R8G8B8;
		custom.srcPartition.x = 0;
		custom.srcPartition.y = 0;
		custom.srcPartition.w = 1;
		custom.srcPartition.h = 1;
		custom.xRatio = 1;
		custom.yRatio = 1;
		NvAPI_Status status = ControlPanel::ApplyCustomDisplay(&custom);
		CheckStatus(status);
	}

	void GetColorControl()
	{
		NvAPI_Status status;
		status = ControlPanel::ColorControl(NV_COLOR_CMD_GET);
		CheckStatus(status);
	}

	void SetColorControl()
	{
		NvAPI_Status status;

		NV_COLOR_DATA data;
		data.version = NV_COLOR_DATA_VER;
		data.size = sizeof(NV_COLOR_DATA);
		data.cmd = NV_COLOR_CMD_SET;
		data.data.bpc = NV_BPC::NV_BPC_10;
		data.data.colorFormat = NV_COLOR_FORMAT::NV_COLOR_FORMAT_DEFAULT;
		data.data.colorimetry = NV_COLOR_COLORIMETRY::NV_COLOR_COLORIMETRY_DEFAULT;
		data.data.colorSelectionPolicy = NV_COLOR_SELECTION_POLICY::NV_COLOR_SELECTION_POLICY_USER;
		data.data.dynamicRange = NV_DYNAMIC_RANGE::NV_DYNAMIC_RANGE_AUTO;
		status = ControlPanel::ColorControl(NV_COLOR_CMD_SET, &data);
		
		CheckStatus(status);
	}

	void DisplayConfiguration()
	{
		NvAPI_Status status;
		status = ControlPanel::ShowCurrentDisplayConfigs();
		CheckStatus(status);
	}

	void ResetAllDefaults()
	{
		NvAPI_Status status = ControlPanel::RestoreAllDefaults();
		CheckStatus(status);
	}
};


int main(int argc, char **argv)
{
	NvAPI_Status status;

	status = NvAPI_Initialize();
	if (status != NVAPI_OK)
		PrintError(status);

	Examples::DisplayConfiguration();

	NvAPI_Unload();
	return 0;
}