static int set_platform_caps(struct pp_hwmgr *hwmgr,
			unsigned long powerplay_caps)
{
	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_POWERPLAY),
		PHM_PlatformCaps_PowerPlaySupport
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_SBIOSPOWERSOURCE),
		PHM_PlatformCaps_BiosPowerSourceControl
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_ASPM_L0s),
		PHM_PlatformCaps_EnableASPML0s
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_ASPM_L1),
		PHM_PlatformCaps_EnableASPML1
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_BACKBIAS),
		PHM_PlatformCaps_EnableBackbias
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_HARDWAREDC),
		PHM_PlatformCaps_AutomaticDCTransition
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_GEMINIPRIMARY),
		PHM_PlatformCaps_GeminiPrimary
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_STEPVDDC),
		PHM_PlatformCaps_StepVddc
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_VOLTAGECONTROL),
		PHM_PlatformCaps_EnableVoltageControl
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_SIDEPORTCONTROL),
		PHM_PlatformCaps_EnableSideportControl
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_TURNOFFPLL_ASPML1),
		PHM_PlatformCaps_TurnOffPll_ASPML1
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_HTLINKCONTROL),
		PHM_PlatformCaps_EnableHTLinkControl
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_MVDDCONTROL),
		PHM_PlatformCaps_EnableMVDDControl
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_VDDCI_CONTROL),
		PHM_PlatformCaps_ControlVDDCI
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_REGULATOR_HOT),
		PHM_PlatformCaps_RegulatorHot
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_GOTO_BOOT_ON_ALERT),
		PHM_PlatformCaps_BootStateOnAlert
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_DONT_WAIT_FOR_VBLANK_ON_ALERT),
		PHM_PlatformCaps_DontWaitForVBlankOnAlert
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_BACO),
		PHM_PlatformCaps_BACO
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_NEW_CAC_VOLTAGE),
		PHM_PlatformCaps_NewCACVoltage
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_REVERT_GPIO5_POLARITY),
		PHM_PlatformCaps_RevertGPIO5Polarity
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_OUTPUT_THERMAL2GPIO17),
		PHM_PlatformCaps_Thermal2GPIO17
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_VRHOT_GPIO_CONFIGURABLE),
		PHM_PlatformCaps_VRHotGPIOConfigurable
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_TEMP_INVERSION),
		PHM_PlatformCaps_TempInversion
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_EVV),
		PHM_PlatformCaps_EVV
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_COMBINE_PCC_WITH_THERMAL_SIGNAL),
		PHM_PlatformCaps_CombinePCCWithThermalSignal
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_LOAD_POST_PRODUCTION_FIRMWARE),
		PHM_PlatformCaps_LoadPostProductionFirmware
	);

	set_hw_cap(
		hwmgr,
		0 != (powerplay_caps & ATOM_PP_PLATFORM_CAP_DISABLE_USING_ACTUAL_TEMPERATURE_FOR_POWER_CALC),
		PHM_PlatformCaps_DisableUsingActualTemperatureForPowerCalc
	);

	return 0;
}
