cmake_minimum_required(VERSION 3.28)
project(PBSD LANGUAGES C CXX)

# CMake 3.28's feature database predates Clang 18 cxx_std_23 support.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
   AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "18")
  if(NOT "cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    list(APPEND CMAKE_CXX_COMPILE_FEATURES cxx_std_23)
  endif()
  set(CMAKE_CXX23_COMPILE_FEATURES cxx_std_23)
  set(CMAKE_CXX23_COMPILER_FLAG "-std=c++23")
  set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std=c++23")
  set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std=c++23")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(KERNEL_CXX_FLAGS -fno-exceptions -fno-rtti -ffreestanding -fno-stack-protector)

add_compile_options(
    -Wthread-safety
    -Wthread-safety-analysis
    -Wthread-safety-negative
)

# --- Nucleus modules (freestanding-friendly flags on library) -----------------
# BMI consumers importing pbsd.handles (or any handle re-export) MUST use
# -fno-exceptions -fno-rtti on the importing TU — propagated via PUBLIC below.
set(PBSD_FS_CXX -fno-exceptions -fno-rtti)

add_library(pbsd_core)
target_sources(pbsd_core PUBLIC FILE_SET CXX_MODULES FILES core/pbsd.core.cppm)
target_compile_features(pbsd_core PUBLIC cxx_std_23)
target_compile_options(pbsd_core PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_rights)
target_sources(pbsd_rights PUBLIC FILE_SET CXX_MODULES FILES rights/pbsd.rights.cppm)
target_link_libraries(pbsd_rights PUBLIC pbsd_core)
target_compile_options(pbsd_rights PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_rights_audit)
add_library(pbsd_rights_audit)
target_sources(pbsd_rights_audit PUBLIC FILE_SET CXX_MODULES FILES rights/pbsd.rights.audit.cppm)
target_link_libraries(pbsd_rights_audit PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_rights_audit PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_rights_mac)
add_library(pbsd_rights_mac)
target_sources(pbsd_rights_mac PUBLIC FILE_SET CXX_MODULES FILES rights/pbsd.rights.mac.cppm)
target_link_libraries(pbsd_rights_mac PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_rights_mac PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_lineage)
target_sources(pbsd_lineage PUBLIC FILE_SET CXX_MODULES FILES lineage/pbsd.cap.lineage.cppm)
target_link_libraries(pbsd_lineage PUBLIC pbsd_core)
target_compile_options(pbsd_lineage PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_handles)
target_sources(pbsd_handles PUBLIC FILE_SET CXX_MODULES FILES handles/pbsd.handles.cppm)
target_link_libraries(pbsd_handles PUBLIC pbsd_core pbsd_rights pbsd_rights_audit pbsd_rights_mac pbsd_lineage)
target_compile_options(pbsd_handles PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_schema)
target_sources(pbsd_uda_schema PUBLIC FILE_SET CXX_MODULES FILES uda/pbsd.uda.schema.cppm)
target_link_libraries(pbsd_uda_schema PUBLIC pbsd_core)
target_compile_options(pbsd_uda_schema PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_uda_interp)
add_library(pbsd_uda_interp)
target_sources(pbsd_uda_interp PUBLIC FILE_SET CXX_MODULES FILES uda/pbsd.uda.interp.cppm)
target_link_libraries(pbsd_uda_interp PUBLIC pbsd_uda_schema pbsd_core)
target_compile_options(pbsd_uda_interp PUBLIC ${PBSD_FS_CXX})
endif()

# --- Wave 5: UDA engines (split freestanding modules) -------------------------
add_library(pbsd_uda_engine_block)
target_sources(pbsd_uda_engine_block PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.block.cppm)
target_link_libraries(pbsd_uda_engine_block PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_block PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_engine_net)
target_sources(pbsd_uda_engine_net PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.net.cppm)
target_link_libraries(pbsd_uda_engine_net PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_net PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_engine_display)
target_sources(pbsd_uda_engine_display PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.display.cppm)
target_link_libraries(pbsd_uda_engine_display PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_display PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_engine_input)
target_sources(pbsd_uda_engine_input PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.input.cppm)
target_link_libraries(pbsd_uda_engine_input PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_input PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_uda_engine_sensor)
add_library(pbsd_uda_engine_sensor)
target_sources(pbsd_uda_engine_sensor PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.sensor.cppm)
target_link_libraries(pbsd_uda_engine_sensor PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_sensor PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_engine_crypto)
add_library(pbsd_uda_engine_crypto)
target_sources(pbsd_uda_engine_crypto PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.crypto.cppm)
target_link_libraries(pbsd_uda_engine_crypto PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_crypto PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_engine_usb)
add_library(pbsd_uda_engine_usb)
target_sources(pbsd_uda_engine_usb PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.usb.cppm)
target_link_libraries(pbsd_uda_engine_usb PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_usb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_engine_pci)
add_library(pbsd_uda_engine_pci)
target_sources(pbsd_uda_engine_pci PUBLIC FILE_SET CXX_MODULES FILES
    uda/engines/pbsd.uda.engine.pci.cppm)
target_link_libraries(pbsd_uda_engine_pci PUBLIC pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_engine_pci PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_engine)
add_library(pbsd_uda_engine)
target_sources(pbsd_uda_engine PUBLIC FILE_SET CXX_MODULES FILES uda/pbsd.uda.engine.cppm)
target_link_libraries(pbsd_uda_engine PUBLIC
    pbsd_uda_engine_block pbsd_uda_engine_net pbsd_uda_engine_display
    pbsd_uda_engine_input pbsd_uda_engine_sensor
    pbsd_uda_engine_crypto pbsd_uda_engine_usb pbsd_uda_engine_pci
    pbsd_core)
target_compile_options(pbsd_uda_engine PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_uda_newbus)
target_sources(pbsd_uda_newbus PUBLIC FILE_SET CXX_MODULES FILES uda/pbsd.uda.newbus.cppm)
target_link_libraries(pbsd_uda_newbus PUBLIC pbsd_uda_engine pbsd_uda_schema pbsd_core)
target_compile_options(pbsd_uda_newbus PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_common)
target_sources(pbsd_uda_virtio_common PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/virtio_common.cppm)
target_link_libraries(pbsd_uda_virtio_common PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_virtio_common PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_blk)
target_sources(pbsd_uda_virtio_blk PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_blk.cppm)
target_link_libraries(pbsd_uda_virtio_blk PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_blk PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_net)
target_sources(pbsd_uda_virtio_net PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_net.cppm)
target_link_libraries(pbsd_uda_virtio_net PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_net PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_nvme)
target_sources(pbsd_uda_nvme PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/nvme.cppm)
target_link_libraries(pbsd_uda_nvme PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_nvme PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_usbhid)
target_sources(pbsd_uda_usbhid PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/usbhid.cppm)
target_link_libraries(pbsd_uda_usbhid PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_usbhid PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_scsi)
target_sources(pbsd_uda_virtio_scsi PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_scsi.cppm)
target_link_libraries(pbsd_uda_virtio_scsi PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_scsi PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_uda_ahci)
add_library(pbsd_uda_ahci)
target_sources(pbsd_uda_ahci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ahci.cppm)
target_link_libraries(pbsd_uda_ahci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ahci PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_uda_e1000)
target_sources(pbsd_uda_e1000 PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/e1000.cppm)
target_link_libraries(pbsd_uda_e1000 PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_e1000 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_xhci)
target_sources(pbsd_uda_xhci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/xhci.cppm)
target_link_libraries(pbsd_uda_xhci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_xhci PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_console)
target_sources(pbsd_uda_virtio_console PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_console.cppm)
target_link_libraries(pbsd_uda_virtio_console PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_console PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_gpu)
target_sources(pbsd_uda_virtio_gpu PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_gpu.cppm)
target_link_libraries(pbsd_uda_virtio_gpu PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_gpu PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_random)
target_sources(pbsd_uda_virtio_random PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_random.cppm)
target_link_libraries(pbsd_uda_virtio_random PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_random PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_balloon)
target_sources(pbsd_uda_virtio_balloon PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_balloon.cppm)
target_link_libraries(pbsd_uda_virtio_balloon PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_balloon PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_nvme_admin)
target_sources(pbsd_uda_nvme_admin PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/nvme_admin.cppm)
target_link_libraries(pbsd_uda_nvme_admin PUBLIC pbsd_uda_nvme)
target_compile_options(pbsd_uda_nvme_admin PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ixgbe)
target_sources(pbsd_uda_ixgbe PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ixgbe.cppm)
target_link_libraries(pbsd_uda_ixgbe PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ixgbe PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_uhci)
target_sources(pbsd_uda_uhci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/uhci.cppm)
target_link_libraries(pbsd_uda_uhci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_uhci PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ehci)
target_sources(pbsd_uda_ehci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ehci.cppm)
target_link_libraries(pbsd_uda_ehci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ehci PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ns8250)
target_sources(pbsd_uda_ns8250 PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ns8250.cppm)
target_link_libraries(pbsd_uda_ns8250 PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ns8250 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_gpio)
target_sources(pbsd_uda_gpio PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/gpio.cppm)
target_link_libraries(pbsd_uda_gpio PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_gpio PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_acpi_button)
target_sources(pbsd_uda_acpi_button PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/acpi_button.cppm)
target_link_libraries(pbsd_uda_acpi_button PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_acpi_button PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_igb)
target_sources(pbsd_uda_igb PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/igb.cppm)
target_link_libraries(pbsd_uda_igb PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_igb PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_em)
target_sources(pbsd_uda_em PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/em.cppm)
target_link_libraries(pbsd_uda_em PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_em PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_em_82574)
target_sources(pbsd_uda_em_82574 PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/em_82574.cppm)
target_link_libraries(pbsd_uda_em_82574 PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_em_82574 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_re)
target_sources(pbsd_uda_re PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/re.cppm)
target_link_libraries(pbsd_uda_re PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_re PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_igc)
target_sources(pbsd_uda_igc PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/igc.cppm)
target_link_libraries(pbsd_uda_igc PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_igc PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_hda)
target_sources(pbsd_uda_hda PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/hda.cppm)
target_link_libraries(pbsd_uda_hda PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_hda PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_smbus)
target_sources(pbsd_uda_smbus PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/smbus.cppm)
target_link_libraries(pbsd_uda_smbus PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_smbus PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ichwd)
target_sources(pbsd_uda_ichwd PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ichwd.cppm)
target_link_libraries(pbsd_uda_ichwd PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ichwd PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_acpi_thermal)
target_sources(pbsd_uda_acpi_thermal PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/acpi_thermal.cppm)
target_link_libraries(pbsd_uda_acpi_thermal PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_acpi_thermal PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_vtnet_mq)
target_sources(pbsd_uda_vtnet_mq PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/vtnet_mq.cppm)
target_link_libraries(pbsd_uda_vtnet_mq PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_vtnet_mq PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ahci_cam)
target_sources(pbsd_uda_ahci_cam PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ahci_cam.cppm)
target_link_libraries(pbsd_uda_ahci_cam PUBLIC pbsd_uda_ahci)
target_compile_options(pbsd_uda_ahci_cam PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_nvme_io)
target_sources(pbsd_uda_nvme_io PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/nvme_io.cppm)
target_link_libraries(pbsd_uda_nvme_io PUBLIC pbsd_uda_nvme pbsd_uda_nvme_admin)
target_compile_options(pbsd_uda_nvme_io PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_igb_vf)
target_sources(pbsd_uda_igb_vf PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/igb_vf.cppm)
target_link_libraries(pbsd_uda_igb_vf PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_igb_vf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_uart_pci)
target_sources(pbsd_uda_uart_pci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/uart_pci.cppm)
target_link_libraries(pbsd_uda_uart_pci PUBLIC pbsd_uda_ns8250)
target_compile_options(pbsd_uda_uart_pci PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_acpi_video)
target_sources(pbsd_uda_acpi_video PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/acpi_video.cppm)
target_link_libraries(pbsd_uda_acpi_video PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_acpi_video PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_sdhci)
target_sources(pbsd_uda_sdhci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/sdhci.cppm)
target_link_libraries(pbsd_uda_sdhci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_sdhci PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_uda_mpt_cam)
add_library(pbsd_uda_mpt_cam)
target_sources(pbsd_uda_mpt_cam PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mpt_cam.cppm)
target_link_libraries(pbsd_uda_mpt_cam PUBLIC pbsd_uda_schema pbsd_uda_interp pbsd_uda_cam_xpt)
target_compile_options(pbsd_uda_mpt_cam PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_uda_bge)
target_sources(pbsd_uda_bge PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/bge.cppm)
target_link_libraries(pbsd_uda_bge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_bge PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ata_pci)
target_sources(pbsd_uda_ata_pci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ata_pci.cppm)
target_link_libraries(pbsd_uda_ata_pci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ata_pci PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_msk)
target_sources(pbsd_uda_msk PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/msk.cppm)
target_link_libraries(pbsd_uda_msk PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_msk PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_cam_status)
target_sources(pbsd_uda_cam_status PUBLIC FILE_SET CXX_MODULES FILES
    uda/cam/pbsd.uda.cam.status.cppm)
target_link_libraries(pbsd_uda_cam_status PUBLIC pbsd_core)
target_compile_options(pbsd_uda_cam_status PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_cam_ccb)
target_sources(pbsd_uda_cam_ccb PUBLIC FILE_SET CXX_MODULES FILES
    uda/cam/pbsd.uda.cam.ccb.cppm)
target_link_libraries(pbsd_uda_cam_ccb PUBLIC pbsd_uda_cam_status)
target_compile_options(pbsd_uda_cam_ccb PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_cam_xpt)
target_sources(pbsd_uda_cam_xpt PUBLIC FILE_SET CXX_MODULES FILES
    uda/cam/pbsd.uda.cam.xpt.cppm)
target_link_libraries(pbsd_uda_cam_xpt PUBLIC pbsd_uda_cam_ccb)
target_compile_options(pbsd_uda_cam_xpt PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_cam)
target_sources(pbsd_uda_cam PUBLIC FILE_SET CXX_MODULES FILES
    uda/cam/pbsd.uda.cam.cppm)
target_link_libraries(pbsd_uda_cam PUBLIC pbsd_uda_cam_xpt)
target_compile_options(pbsd_uda_cam PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_scsi_cam)
target_sources(pbsd_uda_virtio_scsi_cam PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/virtio_scsi_cam.cppm)
target_link_libraries(pbsd_uda_virtio_scsi_cam PUBLIC pbsd_uda_virtio_scsi pbsd_uda_cam_xpt)
target_compile_options(pbsd_uda_virtio_scsi_cam PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ixl)
target_sources(pbsd_uda_ixl PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ixl.cppm)
target_link_libraries(pbsd_uda_ixl PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ixl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_umass)
target_sources(pbsd_uda_umass PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/umass.cppm)
target_link_libraries(pbsd_uda_umass PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_umass PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ale)
target_sources(pbsd_uda_ale PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ale.cppm)
target_link_libraries(pbsd_uda_ale PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ale PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_axe)
target_sources(pbsd_uda_axe PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/axe.cppm)
target_link_libraries(pbsd_uda_axe PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_axe PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_fxp)
target_sources(pbsd_uda_fxp PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/fxp.cppm)
target_link_libraries(pbsd_uda_fxp PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_fxp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_vmx)
target_sources(pbsd_uda_vmx PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/vmx.cppm)
target_link_libraries(pbsd_uda_vmx PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_vmx PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_bnxt)
target_sources(pbsd_uda_bnxt PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/bnxt.cppm)
target_link_libraries(pbsd_uda_bnxt PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_bnxt PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_mlx5)
target_sources(pbsd_uda_mlx5 PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mlx5.cppm)
target_link_libraries(pbsd_uda_mlx5 PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mlx5 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_oce)
target_sources(pbsd_uda_oce PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/oce.cppm)
target_link_libraries(pbsd_uda_oce PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_oce PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_qlnxe)
target_sources(pbsd_uda_qlnxe PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/qlnxe.cppm)
target_link_libraries(pbsd_uda_qlnxe PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_qlnxe PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_cdce)
target_sources(pbsd_uda_cdce PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cdce.cppm)
target_link_libraries(pbsd_uda_cdce PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_cdce PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_urtwn)
target_sources(pbsd_uda_urtwn PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/urtwn.cppm)
target_link_libraries(pbsd_uda_urtwn PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_urtwn PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_rtwn)
target_sources(pbsd_uda_rtwn PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/rtwn.cppm)
target_link_libraries(pbsd_uda_rtwn PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_rtwn PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_iwn)
target_sources(pbsd_uda_iwn PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/iwn.cppm)
target_link_libraries(pbsd_uda_iwn PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_iwn PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_iwm)
target_sources(pbsd_uda_iwm PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/iwm.cppm)
target_link_libraries(pbsd_uda_iwm PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_iwm PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ath)
target_sources(pbsd_uda_ath PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ath.cppm)
target_link_libraries(pbsd_uda_ath PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_ath PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ral)
target_sources(pbsd_uda_ral PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ral.cppm)
target_link_libraries(pbsd_uda_ral PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_ral PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_run)
target_sources(pbsd_uda_run PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/run.cppm)
target_link_libraries(pbsd_uda_run PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_run PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_usb_storage)
target_sources(pbsd_uda_usb_storage PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/usb_storage.cppm)
target_link_libraries(pbsd_uda_usb_storage PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_usb_storage PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_da)
target_sources(pbsd_uda_da PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/da.cppm)
target_link_libraries(pbsd_uda_da PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_da PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ada)
target_sources(pbsd_uda_ada PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ada.cppm)
target_link_libraries(pbsd_uda_ada PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_ada PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_cd)
target_sources(pbsd_uda_cd PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cd.cppm)
target_link_libraries(pbsd_uda_cd PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_cd PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_pass)
target_sources(pbsd_uda_pass PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/pass.cppm)
target_link_libraries(pbsd_uda_pass PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_pass PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_ses)
target_sources(pbsd_uda_ses PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ses.cppm)
target_link_libraries(pbsd_uda_ses PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_ses PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_smart)
target_sources(pbsd_uda_smart PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/smart.cppm)
target_link_libraries(pbsd_uda_smart PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_smart PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_acpi_battery)
target_sources(pbsd_uda_acpi_battery PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/acpi_battery.cppm)
target_link_libraries(pbsd_uda_acpi_battery PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_acpi_battery PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_acpi_lid)
target_sources(pbsd_uda_acpi_lid PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/acpi_lid.cppm)
target_link_libraries(pbsd_uda_acpi_lid PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_acpi_lid PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_acpi_ec)
target_sources(pbsd_uda_acpi_ec PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/acpi_ec.cppm)
target_link_libraries(pbsd_uda_acpi_ec PUBLIC pbsd_uda_schema)
target_compile_options(pbsd_uda_acpi_ec PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_virtio_input)
target_sources(pbsd_uda_virtio_input PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/virtio_input.cppm)
target_link_libraries(pbsd_uda_virtio_input PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_input PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_vr)
target_sources(pbsd_uda_vr PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/vr.cppm)
target_link_libraries(pbsd_uda_vr PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_vr PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_sis)
target_sources(pbsd_uda_sis PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/sis.cppm)
target_link_libraries(pbsd_uda_sis PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_sis PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_bce)
target_sources(pbsd_uda_bce PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/bce.cppm)
target_link_libraries(pbsd_uda_bce PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_bce PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_nfe)
target_sources(pbsd_uda_nfe PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/nfe.cppm)
target_link_libraries(pbsd_uda_nfe PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_nfe PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_jme)
target_sources(pbsd_uda_jme PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/jme.cppm)
target_link_libraries(pbsd_uda_jme PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_jme PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_vge)
target_sources(pbsd_uda_vge PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/vge.cppm)
target_link_libraries(pbsd_uda_vge PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_vge PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_bwn)
target_sources(pbsd_uda_bwn PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/bwn.cppm)
target_link_libraries(pbsd_uda_bwn PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_bwn PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_rum)
target_sources(pbsd_uda_rum PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/rum.cppm)
target_link_libraries(pbsd_uda_rum PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_rum PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_wpi)
target_sources(pbsd_uda_wpi PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/wpi.cppm)
target_link_libraries(pbsd_uda_wpi PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_wpi PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_mwl)
target_sources(pbsd_uda_mwl PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/mwl.cppm)
target_link_libraries(pbsd_uda_mwl PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_mwl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_uda_zyd)
target_sources(pbsd_uda_zyd PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/zyd.cppm)
target_link_libraries(pbsd_uda_zyd PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_zyd PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_uda_dc)
add_library(pbsd_uda_dc)
target_sources(pbsd_uda_dc PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/dc.cppm)
target_link_libraries(pbsd_uda_dc PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_dc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_xl)
add_library(pbsd_uda_xl)
target_sources(pbsd_uda_xl PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/xl.cppm)
target_link_libraries(pbsd_uda_xl PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_xl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_stge)
add_library(pbsd_uda_stge)
target_sources(pbsd_uda_stge PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/stge.cppm)
target_link_libraries(pbsd_uda_stge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_stge PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_sk)
add_library(pbsd_uda_sk)
target_sources(pbsd_uda_sk PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/sk.cppm)
target_link_libraries(pbsd_uda_sk PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_sk PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mdio)
add_library(pbsd_uda_mdio)
target_sources(pbsd_uda_mdio PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mdio.cppm)
target_link_libraries(pbsd_uda_mdio PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mdio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_backlight)
add_library(pbsd_uda_backlight)
target_sources(pbsd_uda_backlight PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/backlight.cppm)
target_link_libraries(pbsd_uda_backlight PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_backlight PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ufshci)
add_library(pbsd_uda_ufshci)
target_sources(pbsd_uda_ufshci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ufshci.cppm)
target_link_libraries(pbsd_uda_ufshci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ufshci PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_pty)
add_library(pbsd_uda_pty)
target_sources(pbsd_uda_pty PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/pty.cppm)
target_link_libraries(pbsd_uda_pty PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_pty PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_etherswitch)
add_library(pbsd_uda_etherswitch)
target_sources(pbsd_uda_etherswitch PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/etherswitch.cppm)
target_link_libraries(pbsd_uda_etherswitch PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_etherswitch PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_wtap)
add_library(pbsd_uda_wtap)
target_sources(pbsd_uda_wtap PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/wtap.cppm)
target_link_libraries(pbsd_uda_wtap PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_wtap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_videomode)
add_library(pbsd_uda_videomode)
target_sources(pbsd_uda_videomode PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/videomode.cppm)
target_link_libraries(pbsd_uda_videomode PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_videomode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_nvram)
add_library(pbsd_uda_nvram)
target_sources(pbsd_uda_nvram PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/nvram.cppm)
target_link_libraries(pbsd_uda_nvram PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_nvram PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_led)
add_library(pbsd_uda_led)
target_sources(pbsd_uda_led PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/led.cppm)
target_link_libraries(pbsd_uda_led PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_led PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_dpms)
add_library(pbsd_uda_dpms)
target_sources(pbsd_uda_dpms PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/dpms.cppm)
target_link_libraries(pbsd_uda_dpms PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_dpms PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_null)
add_library(pbsd_uda_null)
target_sources(pbsd_uda_null PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/null.cppm)
target_link_libraries(pbsd_uda_null PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_null PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_kvm_clock)
add_library(pbsd_uda_kvm_clock)
target_sources(pbsd_uda_kvm_clock PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/kvm_clock.cppm)
target_link_libraries(pbsd_uda_kvm_clock PUBLIC pbsd_uda_schema pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_kvm_clock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_dcons)
add_library(pbsd_uda_dcons)
target_sources(pbsd_uda_dcons PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/dcons.cppm)
target_link_libraries(pbsd_uda_dcons PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_dcons PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_coretemp)
add_library(pbsd_uda_coretemp)
target_sources(pbsd_uda_coretemp PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/coretemp.cppm)
target_link_libraries(pbsd_uda_coretemp PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_coretemp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_amdtemp)
add_library(pbsd_uda_amdtemp)
target_sources(pbsd_uda_amdtemp PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/amdtemp.cppm)
target_link_libraries(pbsd_uda_amdtemp PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_amdtemp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_hid)
add_library(pbsd_uda_hid)
target_sources(pbsd_uda_hid PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/hid.cppm)
target_link_libraries(pbsd_uda_hid PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_hid PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_evdev)
add_library(pbsd_uda_evdev)
target_sources(pbsd_uda_evdev PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/evdev.cppm)
target_link_libraries(pbsd_uda_evdev PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_evdev PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_efidev)
add_library(pbsd_uda_efidev)
target_sources(pbsd_uda_efidev PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/efidev.cppm)
target_link_libraries(pbsd_uda_efidev PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_efidev PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_age)
add_library(pbsd_uda_age)
target_sources(pbsd_uda_age PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/age.cppm)
target_link_libraries(pbsd_uda_age PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_age PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_axgbe)
add_library(pbsd_uda_axgbe)
target_sources(pbsd_uda_axgbe PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/axgbe.cppm)
target_link_libraries(pbsd_uda_axgbe PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_axgbe PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_enic)
add_library(pbsd_uda_enic)
target_sources(pbsd_uda_enic PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/enic.cppm)
target_link_libraries(pbsd_uda_enic PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_enic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_et)
add_library(pbsd_uda_et)
target_sources(pbsd_uda_et PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/et.cppm)
target_link_libraries(pbsd_uda_et PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_et PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_clk)
add_library(pbsd_uda_clk)
target_sources(pbsd_uda_clk PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/clk.cppm)
target_link_libraries(pbsd_uda_clk PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_clk PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_amdsbwd)
add_library(pbsd_uda_amdsbwd)
target_sources(pbsd_uda_amdsbwd PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/amdsbwd.cppm)
target_link_libraries(pbsd_uda_amdsbwd PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_amdsbwd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_amdgpio)
add_library(pbsd_uda_amdgpio)
target_sources(pbsd_uda_amdgpio PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/amdgpio.cppm)
target_link_libraries(pbsd_uda_amdgpio PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_amdgpio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cyapa)
add_library(pbsd_uda_cyapa)
target_sources(pbsd_uda_cyapa PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/cyapa.cppm)
target_link_libraries(pbsd_uda_cyapa PUBLIC
    pbsd_uda_schema
    pbsd_uda_interp)
target_compile_options(pbsd_uda_cyapa PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_chromebook_platform)
add_library(pbsd_uda_chromebook_platform)
target_sources(pbsd_uda_chromebook_platform PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/chromebook_platform.cppm)
target_link_libraries(pbsd_uda_chromebook_platform PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_chromebook_platform PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cpuctl)
add_library(pbsd_uda_cpuctl)
target_sources(pbsd_uda_cpuctl PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/cpuctl.cppm)
target_link_libraries(pbsd_uda_cpuctl PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_cpuctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_amdsmb)
add_library(pbsd_uda_amdsmb)
target_sources(pbsd_uda_amdsmb PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/amdsmb.cppm)
target_link_libraries(pbsd_uda_amdsmb PUBLIC
    pbsd_uda_schema)
target_compile_options(pbsd_uda_amdsmb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mem)
add_library(pbsd_uda_mem)
target_sources(pbsd_uda_mem PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mem.cppm)
target_link_libraries(pbsd_uda_mem PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mem PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_zero)
add_library(pbsd_uda_zero)
target_sources(pbsd_uda_zero PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/zero.cppm)
target_link_libraries(pbsd_uda_zero PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_zero PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_tun)
add_library(pbsd_uda_tun)
target_sources(pbsd_uda_tun PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/tun.cppm)
target_link_libraries(pbsd_uda_tun PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_tun PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_speaker)
add_library(pbsd_uda_speaker)
target_sources(pbsd_uda_speaker PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/speaker.cppm)
target_link_libraries(pbsd_uda_speaker PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_speaker PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_virtio_vsock)
add_library(pbsd_uda_virtio_vsock)
target_sources(pbsd_uda_virtio_vsock PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/virtio_vsock.cppm)
target_link_libraries(pbsd_uda_virtio_vsock PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_vsock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_virtio_fs)
add_library(pbsd_uda_virtio_fs)
target_sources(pbsd_uda_virtio_fs PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/virtio_fs.cppm)
target_link_libraries(pbsd_uda_virtio_fs PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_fs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_virtio_crypto)
add_library(pbsd_uda_virtio_crypto)
target_sources(pbsd_uda_virtio_crypto PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/virtio_crypto.cppm)
target_link_libraries(pbsd_uda_virtio_crypto PUBLIC pbsd_uda_virtio_common)
target_compile_options(pbsd_uda_virtio_crypto PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_pci_hostbridge)
add_library(pbsd_uda_pci_hostbridge)
target_sources(pbsd_uda_pci_hostbridge PUBLIC FILE_SET CXX_MODULES FILES
    uda/descriptors/pci_hostbridge.cppm)
target_link_libraries(pbsd_uda_pci_hostbridge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_pci_hostbridge PUBLIC ${PBSD_FS_CXX})
endif()


if(NOT TARGET pbsd_uda_ena)
add_library(pbsd_uda_ena)
target_sources(pbsd_uda_ena PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ena.cppm)
target_link_libraries(pbsd_uda_ena PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ena PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_alc)
add_library(pbsd_uda_alc)
target_sources(pbsd_uda_alc PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/alc.cppm)
target_link_libraries(pbsd_uda_alc PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_alc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_gem)
add_library(pbsd_uda_gem)
target_sources(pbsd_uda_gem PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/gem.cppm)
target_link_libraries(pbsd_uda_gem PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_gem PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_hifn)
add_library(pbsd_uda_hifn)
target_sources(pbsd_uda_hifn PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/hifn.cppm)
target_link_libraries(pbsd_uda_hifn PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_hifn PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cxgb)
add_library(pbsd_uda_cxgb)
target_sources(pbsd_uda_cxgb PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cxgb.cppm)
target_link_libraries(pbsd_uda_cxgb PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_cxgb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_eqos)
add_library(pbsd_uda_eqos)
target_sources(pbsd_uda_eqos PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/eqos.cppm)
target_link_libraries(pbsd_uda_eqos PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_eqos PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ffec)
add_library(pbsd_uda_ffec)
target_sources(pbsd_uda_ffec PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ffec.cppm)
target_link_libraries(pbsd_uda_ffec PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ffec PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_bfe)
add_library(pbsd_uda_bfe)
target_sources(pbsd_uda_bfe PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/bfe.cppm)
target_link_libraries(pbsd_uda_bfe PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_bfe PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ice)
add_library(pbsd_uda_ice)
target_sources(pbsd_uda_ice PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ice.cppm)
target_link_libraries(pbsd_uda_ice PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ice PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_gve)
add_library(pbsd_uda_gve)
target_sources(pbsd_uda_gve PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/gve.cppm)
target_link_libraries(pbsd_uda_gve PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_gve PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_iavf)
add_library(pbsd_uda_iavf)
target_sources(pbsd_uda_iavf PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/iavf.cppm)
target_link_libraries(pbsd_uda_iavf PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_iavf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_bxe)
add_library(pbsd_uda_bxe)
target_sources(pbsd_uda_bxe PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/bxe.cppm)
target_link_libraries(pbsd_uda_bxe PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_bxe PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_enetc)
add_library(pbsd_uda_enetc)
target_sources(pbsd_uda_enetc PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/enetc.cppm)
target_link_libraries(pbsd_uda_enetc PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_enetc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_al_eth)
add_library(pbsd_uda_al_eth)
target_sources(pbsd_uda_al_eth PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/al_eth.cppm)
target_link_libraries(pbsd_uda_al_eth PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_al_eth PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_arcmsr)
add_library(pbsd_uda_arcmsr)
target_sources(pbsd_uda_arcmsr PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/arcmsr.cppm)
target_link_libraries(pbsd_uda_arcmsr PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_arcmsr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_aac)
add_library(pbsd_uda_aac)
target_sources(pbsd_uda_aac PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/aac.cppm)
target_link_libraries(pbsd_uda_aac PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_aac PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ciss)
add_library(pbsd_uda_ciss)
target_sources(pbsd_uda_ciss PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ciss.cppm)
target_link_libraries(pbsd_uda_ciss PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ciss PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_hpt27xx)
add_library(pbsd_uda_hpt27xx)
target_sources(pbsd_uda_hpt27xx PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/hpt27xx.cppm)
target_link_libraries(pbsd_uda_hpt27xx PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_hpt27xx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_bwi)
add_library(pbsd_uda_bwi)
target_sources(pbsd_uda_bwi PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/bwi.cppm)
target_link_libraries(pbsd_uda_bwi PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_bwi PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ae)
add_library(pbsd_uda_ae)
target_sources(pbsd_uda_ae PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ae.cppm)
target_link_libraries(pbsd_uda_ae PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ae PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cadence)
add_library(pbsd_uda_cadence)
target_sources(pbsd_uda_cadence PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cadence.cppm)
target_link_libraries(pbsd_uda_cadence PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_cadence PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_hwpmc)
add_library(pbsd_uda_hwpmc)
target_sources(pbsd_uda_hwpmc PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/hwpmc.cppm)
target_link_libraries(pbsd_uda_hwpmc PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_hwpmc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_imcsmb)
add_library(pbsd_uda_imcsmb)
target_sources(pbsd_uda_imcsmb PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/imcsmb.cppm)
target_link_libraries(pbsd_uda_imcsmb PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_imcsmb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_iicbus)
add_library(pbsd_uda_iicbus)
target_sources(pbsd_uda_iicbus PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/iicbus.cppm)
target_link_libraries(pbsd_uda_iicbus PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_iicbus PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cardbus)
add_library(pbsd_uda_cardbus)
target_sources(pbsd_uda_cardbus PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cardbus.cppm)
target_link_libraries(pbsd_uda_cardbus PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_cardbus PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_firewire)
add_library(pbsd_uda_firewire)
target_sources(pbsd_uda_firewire PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/firewire.cppm)
target_link_libraries(pbsd_uda_firewire PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_firewire PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_drm2)
add_library(pbsd_uda_drm2)
target_sources(pbsd_uda_drm2 PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/drm2.cppm)
target_link_libraries(pbsd_uda_drm2 PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_drm2 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cas)
add_library(pbsd_uda_cas)
target_sources(pbsd_uda_cas PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cas.cppm)
target_link_libraries(pbsd_uda_cas PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_cas PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cfumass)
add_library(pbsd_uda_cfumass)
target_sources(pbsd_uda_cfumass PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cfumass.cppm)
target_link_libraries(pbsd_uda_cfumass PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_cfumass PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_vte)
add_library(pbsd_uda_vte)
target_sources(pbsd_uda_vte PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/vte.cppm)
target_link_libraries(pbsd_uda_vte PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_vte PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_lge)
add_library(pbsd_uda_lge)
target_sources(pbsd_uda_lge PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/lge.cppm)
target_link_libraries(pbsd_uda_lge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_lge PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_rl)
add_library(pbsd_uda_rl)
target_sources(pbsd_uda_rl PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/rl.cppm)
target_link_libraries(pbsd_uda_rl PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_rl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_nge)
add_library(pbsd_uda_nge)
target_sources(pbsd_uda_nge PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/nge.cppm)
target_link_libraries(pbsd_uda_nge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_nge PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_my)
add_library(pbsd_uda_my)
target_sources(pbsd_uda_my PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/my.cppm)
target_link_libraries(pbsd_uda_my PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_my PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mgb)
add_library(pbsd_uda_mgb)
target_sources(pbsd_uda_mgb PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mgb.cppm)
target_link_libraries(pbsd_uda_mgb PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mgb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_hn)
add_library(pbsd_uda_hn)
target_sources(pbsd_uda_hn PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/hn.cppm)
target_link_libraries(pbsd_uda_hn PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_hn PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ix)
add_library(pbsd_uda_ix)
target_sources(pbsd_uda_ix PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ix.cppm)
target_link_libraries(pbsd_uda_ix PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ix PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ixv)
add_library(pbsd_uda_ixv)
target_sources(pbsd_uda_ixv PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ixv.cppm)
target_link_libraries(pbsd_uda_ixv PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ixv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ipw)
add_library(pbsd_uda_ipw)
target_sources(pbsd_uda_ipw PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ipw.cppm)
target_link_libraries(pbsd_uda_ipw PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ipw PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_iwi)
add_library(pbsd_uda_iwi)
target_sources(pbsd_uda_iwi PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/iwi.cppm)
target_link_libraries(pbsd_uda_iwi PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_iwi PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_iwx)
add_library(pbsd_uda_iwx)
target_sources(pbsd_uda_iwx PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/iwx.cppm)
target_link_libraries(pbsd_uda_iwx PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_iwx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_otus)
add_library(pbsd_uda_otus)
target_sources(pbsd_uda_otus PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/otus.cppm)
target_link_libraries(pbsd_uda_otus PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_otus PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_sge)
add_library(pbsd_uda_sge)
target_sources(pbsd_uda_sge PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/sge.cppm)
target_link_libraries(pbsd_uda_sge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_sge PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_dwc)
add_library(pbsd_uda_dwc)
target_sources(pbsd_uda_dwc PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/dwc.cppm)
target_link_libraries(pbsd_uda_dwc PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_dwc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mvneta)
add_library(pbsd_uda_mvneta)
target_sources(pbsd_uda_mvneta PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mvneta.cppm)
target_link_libraries(pbsd_uda_mvneta PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mvneta PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mxge)
add_library(pbsd_uda_mxge)
target_sources(pbsd_uda_mxge PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mxge.cppm)
target_link_libraries(pbsd_uda_mxge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mxge PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mge)
add_library(pbsd_uda_mge)
target_sources(pbsd_uda_mge PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mge.cppm)
target_link_libraries(pbsd_uda_mge PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mge PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_wg)
add_library(pbsd_uda_wg)
target_sources(pbsd_uda_wg PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/wg.cppm)
target_link_libraries(pbsd_uda_wg PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_wg PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_cxgbe)
add_library(pbsd_uda_cxgbe)
target_sources(pbsd_uda_cxgbe PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/cxgbe.cppm)
target_link_libraries(pbsd_uda_cxgbe PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_cxgbe PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_pvscsi)
add_library(pbsd_uda_pvscsi)
target_sources(pbsd_uda_pvscsi PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/pvscsi.cppm)
target_link_libraries(pbsd_uda_pvscsi PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_pvscsi PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_netfront)
add_library(pbsd_uda_netfront)
target_sources(pbsd_uda_netfront PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/netfront.cppm)
target_link_libraries(pbsd_uda_netfront PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_netfront PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_blkfront)
add_library(pbsd_uda_blkfront)
target_sources(pbsd_uda_blkfront PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/blkfront.cppm)
target_link_libraries(pbsd_uda_blkfront PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_blkfront PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_nicvf)
add_library(pbsd_uda_nicvf)
target_sources(pbsd_uda_nicvf PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/nicvf.cppm)
target_link_libraries(pbsd_uda_nicvf PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_nicvf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_thunder_bgx)
add_library(pbsd_uda_thunder_bgx)
target_sources(pbsd_uda_thunder_bgx PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/thunder_bgx.cppm)
target_link_libraries(pbsd_uda_thunder_bgx PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_thunder_bgx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_xae)
add_library(pbsd_uda_xae)
target_sources(pbsd_uda_xae PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/xae.cppm)
target_link_libraries(pbsd_uda_xae PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_xae PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_watchdog)
add_library(pbsd_uda_watchdog)
target_sources(pbsd_uda_watchdog PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/watchdog.cppm)
target_link_libraries(pbsd_uda_watchdog PUBLIC pbsd_uda_schema pbsd_uda_interp pbsd_core)
target_compile_options(pbsd_uda_watchdog PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_le)
add_library(pbsd_uda_le)
target_sources(pbsd_uda_le PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/le.cppm)
target_link_libraries(pbsd_uda_le PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_le PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_liquidio)
add_library(pbsd_uda_liquidio)
target_sources(pbsd_uda_liquidio PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/liquidio.cppm)
target_link_libraries(pbsd_uda_liquidio PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_liquidio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mana)
add_library(pbsd_uda_mana)
target_sources(pbsd_uda_mana PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mana.cppm)
target_link_libraries(pbsd_uda_mana PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mana PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_malo)
add_library(pbsd_uda_malo)
target_sources(pbsd_uda_malo PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/malo.cppm)
target_link_libraries(pbsd_uda_malo PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_malo PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_irdma)
add_library(pbsd_uda_irdma)
target_sources(pbsd_uda_irdma PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/irdma.cppm)
target_link_libraries(pbsd_uda_irdma PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_irdma PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mfi)
add_library(pbsd_uda_mfi)
target_sources(pbsd_uda_mfi PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mfi.cppm)
target_link_libraries(pbsd_uda_mfi PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mfi PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mrsas)
add_library(pbsd_uda_mrsas)
target_sources(pbsd_uda_mrsas PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mrsas.cppm)
target_link_libraries(pbsd_uda_mrsas PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mrsas PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mpi3mr)
add_library(pbsd_uda_mpi3mr)
target_sources(pbsd_uda_mpi3mr PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mpi3mr.cppm)
target_link_libraries(pbsd_uda_mpi3mr PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mpi3mr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mpr)
add_library(pbsd_uda_mpr)
target_sources(pbsd_uda_mpr PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mpr.cppm)
target_link_libraries(pbsd_uda_mpr PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mpr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_mps)
add_library(pbsd_uda_mps)
target_sources(pbsd_uda_mps PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/mps.cppm)
target_link_libraries(pbsd_uda_mps PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_mps PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_isci)
add_library(pbsd_uda_isci)
target_sources(pbsd_uda_isci PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/isci.cppm)
target_link_libraries(pbsd_uda_isci PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_isci PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_ips)
add_library(pbsd_uda_ips)
target_sources(pbsd_uda_ips PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/ips.cppm)
target_link_libraries(pbsd_uda_ips PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_ips PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_hptiop)
add_library(pbsd_uda_hptiop)
target_sources(pbsd_uda_hptiop PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/hptiop.cppm)
target_link_libraries(pbsd_uda_hptiop PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_hptiop PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_vkbd)
add_library(pbsd_uda_vkbd)
target_sources(pbsd_uda_vkbd PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/vkbd.cppm)
target_link_libraries(pbsd_uda_vkbd PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_vkbd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_kbd)
add_library(pbsd_uda_kbd)
target_sources(pbsd_uda_kbd PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/kbd.cppm)
target_link_libraries(pbsd_uda_kbd PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_kbd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_simplefb)
add_library(pbsd_uda_simplefb)
target_sources(pbsd_uda_simplefb PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/simplefb.cppm)
target_link_libraries(pbsd_uda_simplefb PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_simplefb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_efifb)
add_library(pbsd_uda_efifb)
target_sources(pbsd_uda_efifb PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/efifb.cppm)
target_link_libraries(pbsd_uda_efifb PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_efifb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_wbwd)
add_library(pbsd_uda_wbwd)
target_sources(pbsd_uda_wbwd PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/wbwd.cppm)
target_link_libraries(pbsd_uda_wbwd PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_wbwd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda_wdatwd)
add_library(pbsd_uda_wdatwd)
target_sources(pbsd_uda_wdatwd PUBLIC FILE_SET CXX_MODULES FILES uda/descriptors/wdatwd.cppm)
target_link_libraries(pbsd_uda_wdatwd PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options(pbsd_uda_wdatwd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_uda)
add_library(pbsd_uda INTERFACE)
target_link_libraries(pbsd_uda INTERFACE
    pbsd_uda_schema pbsd_uda_interp pbsd_uda_engine pbsd_uda_newbus
    pbsd_uda_virtio_common
    pbsd_uda_virtio_blk pbsd_uda_virtio_net pbsd_uda_nvme pbsd_uda_usbhid
    pbsd_uda_virtio_scsi pbsd_uda_ahci pbsd_uda_e1000 pbsd_uda_xhci
    pbsd_uda_virtio_console
    pbsd_uda_virtio_gpu pbsd_uda_virtio_random pbsd_uda_virtio_balloon
    pbsd_uda_virtio_input
    pbsd_uda_nvme_admin pbsd_uda_ixgbe pbsd_uda_uhci pbsd_uda_ehci
    pbsd_uda_ns8250 pbsd_uda_gpio pbsd_uda_acpi_button
    pbsd_uda_igb pbsd_uda_em pbsd_uda_em_82574 pbsd_uda_re pbsd_uda_igc
    pbsd_uda_hda pbsd_uda_smbus pbsd_uda_ichwd pbsd_uda_acpi_thermal
    pbsd_uda_vtnet_mq pbsd_uda_ahci_cam pbsd_uda_nvme_io pbsd_uda_igb_vf
    pbsd_uda_uart_pci pbsd_uda_acpi_video
    pbsd_uda_sdhci pbsd_uda_mpt_cam pbsd_uda_bge pbsd_uda_ata_pci pbsd_uda_msk
    pbsd_uda_cam_status pbsd_uda_cam_ccb pbsd_uda_cam_xpt pbsd_uda_cam
    pbsd_uda_virtio_scsi_cam pbsd_uda_ixl pbsd_uda_umass
    pbsd_uda_bnxt pbsd_uda_mlx5 pbsd_uda_oce pbsd_uda_qlnxe pbsd_uda_cdce
    pbsd_uda_urtwn pbsd_uda_rtwn pbsd_uda_iwn pbsd_uda_iwm pbsd_uda_ath
    pbsd_uda_ral pbsd_uda_run pbsd_uda_usb_storage
    pbsd_uda_da pbsd_uda_ada pbsd_uda_cd pbsd_uda_pass pbsd_uda_ses pbsd_uda_smart
    pbsd_uda_acpi_battery pbsd_uda_acpi_lid pbsd_uda_acpi_ec
    pbsd_uda_ale pbsd_uda_axe pbsd_uda_fxp pbsd_uda_vmx
    pbsd_uda_vr
    pbsd_uda_sis
    pbsd_uda_bce
    pbsd_uda_nfe
    pbsd_uda_jme
    pbsd_uda_vge
    pbsd_uda_bwn
    pbsd_uda_rum
    pbsd_uda_wpi
    pbsd_uda_mwl
    pbsd_uda_zyd
    pbsd_uda_dc pbsd_uda_xl pbsd_uda_stge pbsd_uda_sk
    pbsd_uda_mdio pbsd_uda_backlight pbsd_uda_ufshci pbsd_uda_pty pbsd_uda_etherswitch
    pbsd_uda_wtap pbsd_uda_videomode pbsd_uda_nvram pbsd_uda_led pbsd_uda_dpms
    pbsd_uda_null pbsd_uda_kvm_clock pbsd_uda_dcons
    pbsd_uda_coretemp pbsd_uda_amdtemp pbsd_uda_hid pbsd_uda_evdev pbsd_uda_efidev
    pbsd_uda_age pbsd_uda_axgbe pbsd_uda_enic pbsd_uda_et pbsd_uda_clk
    pbsd_uda_amdsbwd pbsd_uda_amdgpio pbsd_uda_cyapa pbsd_uda_chromebook_platform
    pbsd_uda_cpuctl pbsd_uda_amdsmb
    pbsd_uda_mem pbsd_uda_zero pbsd_uda_tun pbsd_uda_speaker pbsd_uda_ena pbsd_uda_alc pbsd_uda_gem pbsd_uda_hifn pbsd_uda_cxgb pbsd_uda_eqos pbsd_uda_ffec pbsd_uda_bfe
    pbsd_uda_virtio_vsock
    pbsd_uda_virtio_fs
    pbsd_uda_virtio_crypto
    pbsd_uda_pci_hostbridge
    pbsd_uda_ice     pbsd_uda_gve     pbsd_uda_iavf     pbsd_uda_bxe     pbsd_uda_enetc     pbsd_uda_al_eth     pbsd_uda_arcmsr     pbsd_uda_aac     pbsd_uda_ciss     pbsd_uda_hpt27xx     pbsd_uda_bwi     pbsd_uda_ae     pbsd_uda_cadence     pbsd_uda_hwpmc     pbsd_uda_imcsmb     pbsd_uda_iicbus     pbsd_uda_cardbus     pbsd_uda_firewire     pbsd_uda_drm2     pbsd_uda_cas
    pbsd_uda_cfumass pbsd_uda_dwc pbsd_uda_hn pbsd_uda_ipw pbsd_uda_iwi pbsd_uda_iwx pbsd_uda_ix pbsd_uda_ixv pbsd_uda_lge pbsd_uda_mgb pbsd_uda_mge pbsd_uda_mvneta pbsd_uda_mxge pbsd_uda_my pbsd_uda_nge pbsd_uda_otus pbsd_uda_rl pbsd_uda_sge pbsd_uda_vte pbsd_uda_wg
    pbsd_uda_cxgbe pbsd_uda_pvscsi pbsd_uda_netfront pbsd_uda_blkfront pbsd_uda_nicvf pbsd_uda_thunder_bgx pbsd_uda_xae pbsd_uda_le pbsd_uda_liquidio pbsd_uda_mana pbsd_uda_malo pbsd_uda_irdma
    pbsd_uda_mfi pbsd_uda_mrsas pbsd_uda_mpi3mr pbsd_uda_mpr pbsd_uda_mps pbsd_uda_isci pbsd_uda_ips pbsd_uda_hptiop
    pbsd_uda_vkbd pbsd_uda_kbd pbsd_uda_simplefb pbsd_uda_efifb pbsd_uda_wbwd pbsd_uda_wdatwd pbsd_uda_watchdog
    pbsd_handles)
endif()

# --- Analyser (hosted) -------------------------------------------------------
add_subdirectory(analyser)

# --- SI harness + demos ------------------------------------------------------
enable_testing()
if(NOT TARGET pbsd_userland_hosted)
add_subdirectory(userland)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/kde/CMakeLists.txt")
  add_subdirectory(kde)
endif()
add_subdirectory(theme)
add_subdirectory(bifrost)
add_subdirectory(compositor)
add_subdirectory(pkg)
add_subdirectory(kernel)

# --- Wave 4: kernel core modules ---------------------------------------------
add_library(pbsd_kernel_capsicum_core)
target_sources(pbsd_kernel_capsicum_core PUBLIC FILE_SET CXX_MODULES FILES
    kernel/capsicum/pbsd.kernel.capsicum.cppm)
target_link_libraries(pbsd_kernel_capsicum_core PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_capsicum_core PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_capsicum)
target_sources(pbsd_kernel_capsicum PUBLIC FILE_SET CXX_MODULES FILES
    kernel/capsicum_bridge.cppm)
target_link_libraries(pbsd_kernel_capsicum PUBLIC pbsd_kernel_capsicum_core pbsd_handles)
target_compile_options(pbsd_kernel_capsicum PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_prot)
target_sources(pbsd_kernel_prot PUBLIC FILE_SET CXX_MODULES FILES
    kernel/process/pbsd.kernel.prot.cppm)
target_link_libraries(pbsd_kernel_prot PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_prot PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_process)
target_sources(pbsd_kernel_process PUBLIC FILE_SET CXX_MODULES FILES
    kernel/process/pbsd.kernel.process.cppm)
target_link_libraries(pbsd_kernel_process PUBLIC pbsd_kernel_capsicum pbsd_kernel_prot pbsd_handles)
target_compile_options(pbsd_kernel_process PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vm)
target_sources(pbsd_kernel_vm PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm.cppm)
target_link_libraries(pbsd_kernel_vm PUBLIC pbsd_kernel_capsicum_core pbsd_handles)
target_compile_options(pbsd_kernel_vm PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_alloc)
target_sources(pbsd_kernel_alloc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/alloc/pbsd.kernel.alloc.cppm)
target_link_libraries(pbsd_kernel_alloc PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_alloc PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_resource)
target_sources(pbsd_kernel_resource PUBLIC FILE_SET CXX_MODULES FILES
    kernel/resource/pbsd.kernel.resource.cppm)
target_link_libraries(pbsd_kernel_resource PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_resource PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_errno)
target_sources(pbsd_kernel_errno PUBLIC FILE_SET CXX_MODULES FILES
    kernel/errno/pbsd.kernel.errno.cppm)
target_link_libraries(pbsd_kernel_errno PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_errno PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_signal)
target_sources(pbsd_kernel_signal PUBLIC FILE_SET CXX_MODULES FILES
    kernel/signal/pbsd.kernel.signal.cppm)
target_link_libraries(pbsd_kernel_signal PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_signal PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_syscall)
target_sources(pbsd_kernel_syscall PUBLIC FILE_SET CXX_MODULES FILES
    kernel/syscall/pbsd.kernel.syscall.cppm)
target_link_libraries(pbsd_kernel_syscall PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_syscall PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sched)
target_sources(pbsd_kernel_sched PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sched/pbsd.kernel.sched.cppm)
target_link_libraries(pbsd_kernel_sched PUBLIC pbsd_handles)
target_compile_options(pbsd_kernel_sched PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_pax_mac)
target_sources(pbsd_kernel_pax_mac PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.pax_mac.cppm)
target_link_libraries(pbsd_kernel_pax_mac PUBLIC pbsd_kernel_vm pbsd_core)
target_compile_options(pbsd_kernel_pax_mac PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_pax_aslr)
target_sources(pbsd_kernel_pax_aslr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.pax_aslr.cppm)
target_link_libraries(pbsd_kernel_pax_aslr PUBLIC pbsd_kernel_pax_mac pbsd_core)
target_compile_options(pbsd_kernel_pax_aslr PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sysctl)
target_sources(pbsd_kernel_sysctl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sysctl/pbsd.kernel.sysctl.cppm)
target_link_libraries(pbsd_kernel_sysctl PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sysctl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vm_page)
target_sources(pbsd_kernel_vm_page PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_page.cppm)
target_link_libraries(pbsd_kernel_vm_page PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vm_page PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vm_map)
target_sources(pbsd_kernel_vm_map PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_map.cppm)
target_link_libraries(pbsd_kernel_vm_map PUBLIC pbsd_kernel_vm)
target_compile_options(pbsd_kernel_vm_map PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vm_object)
target_sources(pbsd_kernel_vm_object PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_object.cppm)
target_link_libraries(pbsd_kernel_vm_object PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vm_object PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vm_fault)
target_sources(pbsd_kernel_vm_fault PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_fault.cppm)
target_link_libraries(pbsd_kernel_vm_fault PUBLIC pbsd_kernel_vm)
target_compile_options(pbsd_kernel_vm_fault PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_uma)
target_sources(pbsd_kernel_uma PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.uma.cppm)
target_link_libraries(pbsd_kernel_uma PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_uma PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vm_pager)
target_sources(pbsd_kernel_vm_pager PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_pager.cppm)
target_link_libraries(pbsd_kernel_vm_pager PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vm_pager PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vm_phys)
target_sources(pbsd_kernel_vm_phys PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_phys.cppm)
target_link_libraries(pbsd_kernel_vm_phys PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vm_phys PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_param)
target_sources(pbsd_kernel_param PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.param.cppm)
target_link_libraries(pbsd_kernel_param PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_param PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_security_bsd)
target_sources(pbsd_kernel_security_bsd PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.security_bsd.cppm)
target_link_libraries(pbsd_kernel_security_bsd PUBLIC pbsd_kernel_prot)
target_compile_options(pbsd_kernel_security_bsd PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_ucred)
target_sources(pbsd_kernel_ucred PUBLIC FILE_SET CXX_MODULES FILES
    kernel/cred/pbsd.kernel.ucred.cppm)
target_link_libraries(pbsd_kernel_ucred PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_ucred PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_jail)
target_sources(pbsd_kernel_jail PUBLIC FILE_SET CXX_MODULES FILES
    kernel/jail/pbsd.kernel.jail.cppm)
target_link_libraries(pbsd_kernel_jail PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_jail PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_timekeeping)
target_sources(pbsd_kernel_timekeeping PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.timekeeping.cppm)
target_link_libraries(pbsd_kernel_timekeeping PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_timekeeping PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_mutex)
target_sources(pbsd_kernel_mutex PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.mutex.cppm)
target_link_libraries(pbsd_kernel_mutex PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mutex PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_kernel_turnstile)
add_library(pbsd_kernel_turnstile)
target_sources(pbsd_kernel_turnstile PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.turnstile.cppm)
target_link_libraries(pbsd_kernel_turnstile PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_turnstile PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_kernel_ktrace)
target_sources(pbsd_kernel_ktrace PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.ktrace.cppm)
target_link_libraries(pbsd_kernel_ktrace PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_ktrace PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_kevent)
target_sources(pbsd_kernel_kevent PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kevent/pbsd.kernel.kevent.cppm)
target_link_libraries(pbsd_kernel_kevent PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kevent PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_syscall_args)
target_sources(pbsd_kernel_syscall_args PUBLIC FILE_SET CXX_MODULES FILES
    kernel/syscall/pbsd.kernel.syscall_args.cppm)
target_link_libraries(pbsd_kernel_syscall_args PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_syscall_args PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_poll)
target_sources(pbsd_kernel_poll PUBLIC FILE_SET CXX_MODULES FILES
    kernel/poll/pbsd.kernel.poll.cppm)
target_link_libraries(pbsd_kernel_poll PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_poll PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_socket_syscall)
target_sources(pbsd_kernel_socket_syscall PUBLIC FILE_SET CXX_MODULES FILES
    kernel/socket/pbsd.kernel.socket_syscall.cppm)
target_link_libraries(pbsd_kernel_socket_syscall PUBLIC pbsd_kernel_syscall)
target_compile_options(pbsd_kernel_socket_syscall PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vnode)
target_sources(pbsd_kernel_vnode PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vnode/pbsd.kernel.vnode.cppm)
target_link_libraries(pbsd_kernel_vnode PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vnode PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_pipe)
target_sources(pbsd_kernel_pipe PUBLIC FILE_SET CXX_MODULES FILES
    kernel/pipe/pbsd.kernel.pipe.cppm)
target_link_libraries(pbsd_kernel_pipe PUBLIC pbsd_kernel_syscall)
target_compile_options(pbsd_kernel_pipe PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_fifo)
target_sources(pbsd_kernel_fifo PUBLIC FILE_SET CXX_MODULES FILES
    kernel/fifo/pbsd.kernel.fifo.cppm)
target_link_libraries(pbsd_kernel_fifo PUBLIC pbsd_kernel_vnode)
target_compile_options(pbsd_kernel_fifo PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_namei)
target_sources(pbsd_kernel_namei PUBLIC FILE_SET CXX_MODULES FILES
    kernel/namei/pbsd.kernel.namei.cppm)
target_link_libraries(pbsd_kernel_namei PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_namei PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_bpf)
target_sources(pbsd_kernel_bpf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/bpf/pbsd.kernel.bpf.cppm)
target_link_libraries(pbsd_kernel_bpf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bpf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_audit)
target_sources(pbsd_kernel_audit PUBLIC FILE_SET CXX_MODULES FILES
    kernel/audit/pbsd.kernel.audit.cppm)
target_link_libraries(pbsd_kernel_audit PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_audit PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_callout)
target_sources(pbsd_kernel_callout PUBLIC FILE_SET CXX_MODULES FILES
    kernel/callout/pbsd.kernel.callout.cppm)
target_link_libraries(pbsd_kernel_callout PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_callout PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_tty)
target_sources(pbsd_kernel_tty PUBLIC FILE_SET CXX_MODULES FILES
    kernel/tty/pbsd.kernel.tty.cppm)
target_link_libraries(pbsd_kernel_tty PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_tty PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_filedesc)
target_sources(pbsd_kernel_filedesc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/filedesc/pbsd.kernel.filedesc.cppm)
target_link_libraries(pbsd_kernel_filedesc PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_filedesc PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_selinfo)
target_sources(pbsd_kernel_selinfo PUBLIC FILE_SET CXX_MODULES FILES
    kernel/selinfo/pbsd.kernel.selinfo.cppm)
target_link_libraries(pbsd_kernel_selinfo PUBLIC pbsd_kernel_kevent)
target_compile_options(pbsd_kernel_selinfo PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_select)
target_sources(pbsd_kernel_select PUBLIC FILE_SET CXX_MODULES FILES
    kernel/select/pbsd.kernel.select.cppm)
target_link_libraries(pbsd_kernel_select PUBLIC pbsd_kernel_poll pbsd_kernel_syscall)
target_compile_options(pbsd_kernel_select PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_aio)
target_sources(pbsd_kernel_aio PUBLIC FILE_SET CXX_MODULES FILES
    kernel/aio/pbsd.kernel.aio.cppm)
target_link_libraries(pbsd_kernel_aio PUBLIC pbsd_kernel_syscall)
target_compile_options(pbsd_kernel_aio PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_mbuf)
target_sources(pbsd_kernel_mbuf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/mbuf/pbsd.kernel.mbuf.cppm)
target_link_libraries(pbsd_kernel_mbuf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mbuf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_pcb)
target_sources(pbsd_kernel_pcb PUBLIC FILE_SET CXX_MODULES FILES
    kernel/pcb/pbsd.kernel.pcb.cppm)
target_link_libraries(pbsd_kernel_pcb PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_pcb PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_inpcb)
target_sources(pbsd_kernel_inpcb PUBLIC FILE_SET CXX_MODULES FILES
    kernel/inpcb/pbsd.kernel.inpcb.cppm)
target_link_libraries(pbsd_kernel_inpcb PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_inpcb PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_socket_ops)
target_sources(pbsd_kernel_socket_ops PUBLIC FILE_SET CXX_MODULES FILES
    kernel/socket/pbsd.kernel.socket_ops.cppm)
target_link_libraries(pbsd_kernel_socket_ops PUBLIC pbsd_kernel_filedesc)
target_compile_options(pbsd_kernel_socket_ops PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_route)
target_sources(pbsd_kernel_route PUBLIC FILE_SET CXX_MODULES FILES
    kernel/route/pbsd.kernel.route.cppm)
target_link_libraries(pbsd_kernel_route PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_route PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_ifnet)
target_sources(pbsd_kernel_ifnet PUBLIC FILE_SET CXX_MODULES FILES
    kernel/ifnet/pbsd.kernel.ifnet.cppm)
target_link_libraries(pbsd_kernel_ifnet PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_ifnet PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_netgraph)
target_sources(pbsd_kernel_netgraph PUBLIC FILE_SET CXX_MODULES FILES
    kernel/netgraph/pbsd.kernel.netgraph.cppm)
target_link_libraries(pbsd_kernel_netgraph PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_netgraph PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sockbuf)
target_sources(pbsd_kernel_sockbuf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sockbuf/pbsd.kernel.sockbuf.cppm)
target_link_libraries(pbsd_kernel_sockbuf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sockbuf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_domain)
target_sources(pbsd_kernel_domain PUBLIC FILE_SET CXX_MODULES FILES
    kernel/domain/pbsd.kernel.domain.cppm)
target_link_libraries(pbsd_kernel_domain PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_domain PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_protosw)
target_sources(pbsd_kernel_protosw PUBLIC FILE_SET CXX_MODULES FILES
    kernel/protosw/pbsd.kernel.protosw.cppm)
target_link_libraries(pbsd_kernel_protosw PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_protosw PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sockio)
target_sources(pbsd_kernel_sockio PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sockio/pbsd.kernel.sockio.cppm)
target_link_libraries(pbsd_kernel_sockio PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sockio PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_rtentry)
target_sources(pbsd_kernel_rtentry PUBLIC FILE_SET CXX_MODULES FILES
    kernel/rtentry/pbsd.kernel.rtentry.cppm)
target_link_libraries(pbsd_kernel_rtentry PUBLIC pbsd_kernel_route)
target_compile_options(pbsd_kernel_rtentry PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_carp)
target_sources(pbsd_kernel_carp PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.carp.cppm)
target_link_libraries(pbsd_kernel_carp PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_carp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_gif)
target_sources(pbsd_kernel_gif PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.gif.cppm)
target_link_libraries(pbsd_kernel_gif PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_gif PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_gre)
target_sources(pbsd_kernel_gre PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.gre.cppm)
target_link_libraries(pbsd_kernel_gre PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_gre PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_bridge)
target_sources(pbsd_kernel_bridge PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.bridge.cppm)
target_link_libraries(pbsd_kernel_bridge PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bridge PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_vlan)
target_sources(pbsd_kernel_vlan PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.vlan.cppm)
target_link_libraries(pbsd_kernel_vlan PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vlan PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_lagg)
target_sources(pbsd_kernel_lagg PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.lagg.cppm)
target_link_libraries(pbsd_kernel_lagg PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_lagg PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_if_media)
target_sources(pbsd_kernel_if_media PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.if_media.cppm)
target_link_libraries(pbsd_kernel_if_media PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_if_media PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_if_clone)
target_sources(pbsd_kernel_if_clone PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.if_clone.cppm)
target_link_libraries(pbsd_kernel_if_clone PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_if_clone PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_pfil)
target_sources(pbsd_kernel_pfil PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.pfil.cppm)
target_link_libraries(pbsd_kernel_pfil PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_pfil PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_random)
target_sources(pbsd_kernel_random PUBLIC FILE_SET CXX_MODULES FILES
    kernel/random/pbsd.kernel.random.cppm)
target_link_libraries(pbsd_kernel_random PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_random PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_entropy)
target_sources(pbsd_kernel_entropy PUBLIC FILE_SET CXX_MODULES FILES
    kernel/random/pbsd.kernel.entropy.cppm)
target_link_libraries(pbsd_kernel_entropy PUBLIC pbsd_kernel_random)
target_compile_options(pbsd_kernel_entropy PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_crypto)
target_sources(pbsd_kernel_crypto PUBLIC FILE_SET CXX_MODULES FILES
    kernel/crypto/pbsd.kernel.crypto.cppm)
target_link_libraries(pbsd_kernel_crypto PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_crypto PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_geom)
target_sources(pbsd_kernel_geom PUBLIC FILE_SET CXX_MODULES FILES
    kernel/geom/pbsd.kernel.geom.cppm)
target_link_libraries(pbsd_kernel_geom PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_geom PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_cam)
target_sources(pbsd_kernel_cam PUBLIC FILE_SET CXX_MODULES FILES
    kernel/cam/pbsd.kernel.cam.cppm)
target_link_libraries(pbsd_kernel_cam PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_cam PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_cons)
target_sources(pbsd_kernel_cons PUBLIC FILE_SET CXX_MODULES FILES
    kernel/cons/pbsd.kernel.cons.cppm)
target_link_libraries(pbsd_kernel_cons PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_cons PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_uipc)
target_sources(pbsd_kernel_uipc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/uipc/pbsd.kernel.uipc.cppm)
target_link_libraries(pbsd_kernel_uipc PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_uipc PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sysvshm)
target_sources(pbsd_kernel_sysvshm PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sysvshm/pbsd.kernel.sysvshm.cppm)
target_link_libraries(pbsd_kernel_sysvshm PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sysvshm PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sysvsem)
target_sources(pbsd_kernel_sysvsem PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sysvsem/pbsd.kernel.sysvsem.cppm)
target_link_libraries(pbsd_kernel_sysvsem PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sysvsem PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sysvmsg)
target_sources(pbsd_kernel_sysvmsg PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sysvmsg/pbsd.kernel.sysvmsg.cppm)
target_link_libraries(pbsd_kernel_sysvmsg PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sysvmsg PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_posix_shm)
target_sources(pbsd_kernel_posix_shm PUBLIC FILE_SET CXX_MODULES FILES
    kernel/posix_shm/pbsd.kernel.posix_shm.cppm)
target_link_libraries(pbsd_kernel_posix_shm PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_posix_shm PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_acct)
target_sources(pbsd_kernel_acct PUBLIC FILE_SET CXX_MODULES FILES
    kernel/acct/pbsd.kernel.acct.cppm)
target_link_libraries(pbsd_kernel_acct PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_acct PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_racct)
target_sources(pbsd_kernel_racct PUBLIC FILE_SET CXX_MODULES FILES
    kernel/racct/pbsd.kernel.racct.cppm)
target_link_libraries(pbsd_kernel_racct PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_racct PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_rctl)
target_sources(pbsd_kernel_rctl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/rctl/pbsd.kernel.rctl.cppm)
target_link_libraries(pbsd_kernel_rctl PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_rctl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_cpuset)
target_sources(pbsd_kernel_cpuset PUBLIC FILE_SET CXX_MODULES FILES
    kernel/cpuset/pbsd.kernel.cpuset.cppm)
target_link_libraries(pbsd_kernel_cpuset PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_cpuset PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_numa)
target_sources(pbsd_kernel_numa PUBLIC FILE_SET CXX_MODULES FILES
    kernel/numa/pbsd.kernel.numa.cppm)
target_link_libraries(pbsd_kernel_numa PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_numa PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_smr)
target_sources(pbsd_kernel_smr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/smr/pbsd.kernel.smr.cppm)
target_link_libraries(pbsd_kernel_smr PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_smr PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_epoch)
target_sources(pbsd_kernel_epoch PUBLIC FILE_SET CXX_MODULES FILES
    kernel/epoch/pbsd.kernel.epoch.cppm)
target_link_libraries(pbsd_kernel_epoch PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_epoch PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_timeout)
target_sources(pbsd_kernel_timeout PUBLIC FILE_SET CXX_MODULES FILES
    kernel/timeout/pbsd.kernel.timeout.cppm)
target_link_libraries(pbsd_kernel_timeout PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_timeout PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_intr)
target_sources(pbsd_kernel_intr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/intr/pbsd.kernel.intr.cppm)
target_link_libraries(pbsd_kernel_intr PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_intr PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_busdma)
target_sources(pbsd_kernel_busdma PUBLIC FILE_SET CXX_MODULES FILES
    kernel/busdma/pbsd.kernel.busdma.cppm)
target_link_libraries(pbsd_kernel_busdma PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_busdma PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sx)
target_sources(pbsd_kernel_sx PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.sx.cppm)
target_link_libraries(pbsd_kernel_sx PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sx PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_rwlock)
target_sources(pbsd_kernel_rwlock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.rwlock.cppm)
target_link_libraries(pbsd_kernel_rwlock PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_rwlock PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_condvar)
target_sources(pbsd_kernel_condvar PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.condvar.cppm)
target_link_libraries(pbsd_kernel_condvar PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_condvar PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_taskqueue)
target_sources(pbsd_kernel_taskqueue PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.taskqueue.cppm)
target_link_libraries(pbsd_kernel_taskqueue PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_taskqueue PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_uio)
target_sources(pbsd_kernel_uio PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.uio.cppm)
target_link_libraries(pbsd_kernel_uio PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_uio PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_lockf)
target_sources(pbsd_kernel_lockf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.lockf.cppm)
target_link_libraries(pbsd_kernel_lockf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_lockf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_sleepq)
target_sources(pbsd_kernel_sleepq PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sleepq/pbsd.kernel.sleepq.cppm)
target_link_libraries(pbsd_kernel_sleepq PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sleepq PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_shim)
target_sources(pbsd_kernel_shim PUBLIC FILE_SET CXX_MODULES FILES
    kernel/shims/pbsd.kernel.shim.cppm)
target_link_libraries(pbsd_kernel_shim PUBLIC
    pbsd_kernel_capsicum pbsd_kernel_vm pbsd_kernel_pax_mac pbsd_kernel_pax_aslr
    pbsd_kernel_prot pbsd_kernel_resource pbsd_kernel_sysctl pbsd_kernel_signal
    pbsd_kernel_errno pbsd_kernel_vm_pager)
target_compile_options(pbsd_kernel_shim PUBLIC ${PBSD_FS_CXX})
target_include_directories(pbsd_kernel_shim PUBLIC kernel/shims)

add_library(pbsd_kernel_rmlock)
target_sources(pbsd_kernel_rmlock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.rmlock.cppm)
target_link_libraries(pbsd_kernel_rmlock PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_rmlock PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_bio)
target_sources(pbsd_kernel_bio PUBLIC FILE_SET CXX_MODULES FILES
    kernel/io/pbsd.kernel.bio.cppm)
target_link_libraries(pbsd_kernel_bio PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_bio PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_disk)
target_sources(pbsd_kernel_disk PUBLIC FILE_SET CXX_MODULES FILES
    kernel/io/pbsd.kernel.disk.cppm)
target_link_libraries(pbsd_kernel_disk PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_disk PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_buf)
target_sources(pbsd_kernel_buf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/io/pbsd.kernel.buf.cppm)
target_link_libraries(pbsd_kernel_buf PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_buf PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_kernel_eventhandler)
add_library(pbsd_kernel_eventhandler)
target_sources(pbsd_kernel_eventhandler PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.eventhandler.cppm)
target_link_libraries(pbsd_kernel_eventhandler PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_eventhandler PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_kernel_refcnt)
target_sources(pbsd_kernel_refcnt PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.refcnt.cppm)
target_link_libraries(pbsd_kernel_refcnt PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_refcnt PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_kobj)
target_sources(pbsd_kernel_kobj PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kobj.cppm)
target_link_libraries(pbsd_kernel_kobj PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_kobj PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_pci)
target_sources(pbsd_kernel_pci PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.pci.cppm)
target_link_libraries(pbsd_kernel_pci PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_pci PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_kernel_devstat)
target_sources(pbsd_kernel_devstat PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.devstat.cppm)
target_link_libraries(pbsd_kernel_devstat PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_devstat PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_kernel_witness)
add_library(pbsd_kernel_witness)
target_sources(pbsd_kernel_witness PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.witness.cppm)
target_link_libraries(pbsd_kernel_witness PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_witness PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sysinit)
add_library(pbsd_kernel_sysinit)
target_sources(pbsd_kernel_sysinit PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.sysinit.cppm)
target_link_libraries(pbsd_kernel_sysinit PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sysinit PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_uma_helpers)
add_library(pbsd_kernel_uma_helpers)
target_sources(pbsd_kernel_uma_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.uma_helpers.cppm)
target_link_libraries(pbsd_kernel_uma_helpers PUBLIC pbsd_kernel_uma)
target_compile_options(pbsd_kernel_uma_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kqueue)
add_library(pbsd_kernel_kqueue)
target_sources(pbsd_kernel_kqueue PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kqueue/pbsd.kernel.kqueue.cppm)
target_link_libraries(pbsd_kernel_kqueue PUBLIC pbsd_kernel_kevent)
target_compile_options(pbsd_kernel_kqueue PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_cap_rights)
add_library(pbsd_kernel_cap_rights)
target_sources(pbsd_kernel_cap_rights PUBLIC FILE_SET CXX_MODULES FILES
    kernel/capsicum/pbsd.kernel.cap_rights.cppm)
target_link_libraries(pbsd_kernel_cap_rights PUBLIC
    pbsd_core pbsd_rights pbsd_kernel_capsicum_core)
target_compile_options(pbsd_kernel_cap_rights PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_signal_helpers)
add_library(pbsd_kernel_signal_helpers)
target_sources(pbsd_kernel_signal_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/signal/pbsd.kernel.signal_helpers.cppm)
target_link_libraries(pbsd_kernel_signal_helpers PUBLIC pbsd_kernel_signal)
target_compile_options(pbsd_kernel_signal_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kmod)
add_library(pbsd_kernel_kmod)
target_sources(pbsd_kernel_kmod PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kmod.cppm)
target_link_libraries(pbsd_kernel_kmod PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kmod PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_counter)
add_library(pbsd_kernel_counter)
target_sources(pbsd_kernel_counter PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.counter.cppm)
target_link_libraries(pbsd_kernel_counter PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_counter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mp_ring)
add_library(pbsd_kernel_mp_ring)
target_sources(pbsd_kernel_mp_ring PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.mp_ring.cppm)
target_link_libraries(pbsd_kernel_mp_ring PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mp_ring PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_toeplitz)
add_library(pbsd_kernel_toeplitz)
target_sources(pbsd_kernel_toeplitz PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.toeplitz.cppm)
target_link_libraries(pbsd_kernel_toeplitz PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_toeplitz PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bufring)
add_library(pbsd_kernel_bufring)
target_sources(pbsd_kernel_bufring PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.bufring.cppm)
target_link_libraries(pbsd_kernel_bufring PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bufring PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_none)
add_library(pbsd_kernel_mac_none)
target_sources(pbsd_kernel_mac_none PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_none.cppm)
target_link_libraries(pbsd_kernel_mac_none PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_none PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_jail)
add_library(pbsd_kernel_mac_jail)
target_sources(pbsd_kernel_mac_jail PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_jail.cppm)
target_link_libraries(pbsd_kernel_mac_jail PUBLIC pbsd_core pbsd_kernel_jail)
target_compile_options(pbsd_kernel_mac_jail PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_label)
add_library(pbsd_kernel_mac_label)
target_sources(pbsd_kernel_mac_label PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_label.cppm)
target_link_libraries(pbsd_kernel_mac_label PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_label PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_if_mib)
add_library(pbsd_kernel_if_mib)
target_sources(pbsd_kernel_if_mib PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.if_mib.cppm)
target_link_libraries(pbsd_kernel_if_mib PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_if_mib PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_if_dead)
add_library(pbsd_kernel_if_dead)
target_sources(pbsd_kernel_if_dead PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.if_dead.cppm)
target_link_libraries(pbsd_kernel_if_dead PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_if_dead PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_ifq)
add_library(pbsd_kernel_ifq)
target_sources(pbsd_kernel_ifq PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.ifq.cppm)
target_link_libraries(pbsd_kernel_ifq PUBLIC pbsd_core pbsd_kernel_bufring)
target_compile_options(pbsd_kernel_ifq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_radix)
add_library(pbsd_kernel_vm_radix)
target_sources(pbsd_kernel_vm_radix PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_radix.cppm)
target_link_libraries(pbsd_kernel_vm_radix PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vm_radix PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_redzone)
add_library(pbsd_kernel_redzone)
target_sources(pbsd_kernel_redzone PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.redzone.cppm)
target_link_libraries(pbsd_kernel_redzone PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_redzone PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bpf_buffer)
add_library(pbsd_kernel_bpf_buffer)
target_sources(pbsd_kernel_bpf_buffer PUBLIC FILE_SET CXX_MODULES FILES
    kernel/bpf/pbsd.kernel.bpf_buffer.cppm)
target_link_libraries(pbsd_kernel_bpf_buffer PUBLIC pbsd_core pbsd_kernel_bpf)
target_compile_options(pbsd_kernel_bpf_buffer PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bpf_jitter)
add_library(pbsd_kernel_bpf_jitter)
target_sources(pbsd_kernel_bpf_jitter PUBLIC FILE_SET CXX_MODULES FILES
    kernel/bpf/pbsd.kernel.bpf_jitter.cppm)
target_link_libraries(pbsd_kernel_bpf_jitter PUBLIC pbsd_core pbsd_kernel_bpf)
target_compile_options(pbsd_kernel_bpf_jitter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sbuf)
add_library(pbsd_kernel_sbuf)
target_sources(pbsd_kernel_sbuf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.sbuf.cppm)
target_link_libraries(pbsd_kernel_sbuf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_sbuf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_panic)
add_library(pbsd_kernel_panic)
target_sources(pbsd_kernel_panic PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.panic.cppm)
target_link_libraries(pbsd_kernel_panic PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_panic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_uuid)
add_library(pbsd_kernel_uuid)
target_sources(pbsd_kernel_uuid PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.uuid.cppm)
target_link_libraries(pbsd_kernel_uuid PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_uuid PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_linker)
add_library(pbsd_kernel_linker)
target_sources(pbsd_kernel_linker PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.linker.cppm)
target_link_libraries(pbsd_kernel_linker PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_linker PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mfence)
add_library(pbsd_kernel_mfence)
target_sources(pbsd_kernel_mfence PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.mfence.cppm)
target_link_libraries(pbsd_kernel_mfence PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mfence PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_route_temporal)
add_library(pbsd_kernel_route_temporal)
target_sources(pbsd_kernel_route_temporal PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.route_temporal.cppm)
target_link_libraries(pbsd_kernel_route_temporal PUBLIC pbsd_core pbsd_kernel_route)
target_compile_options(pbsd_kernel_route_temporal PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_nhop_utils)
add_library(pbsd_kernel_nhop_utils)
target_sources(pbsd_kernel_nhop_utils PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.nhop_utils.cppm)
target_link_libraries(pbsd_kernel_nhop_utils PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_nhop_utils PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_if_disc)
add_library(pbsd_kernel_if_disc)
target_sources(pbsd_kernel_if_disc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.if_disc.cppm)
target_link_libraries(pbsd_kernel_if_disc PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_if_disc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_if_loop)
add_library(pbsd_kernel_if_loop)
target_sources(pbsd_kernel_if_loop PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.if_loop.cppm)
target_link_libraries(pbsd_kernel_if_loop PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_if_loop PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_uma_dbg)
add_library(pbsd_kernel_uma_dbg)
target_sources(pbsd_kernel_uma_dbg PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.uma_dbg.cppm)
target_link_libraries(pbsd_kernel_uma_dbg PUBLIC pbsd_core pbsd_kernel_uma)
target_compile_options(pbsd_kernel_uma_dbg PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_audit_trigger)
add_library(pbsd_kernel_audit_trigger)
target_sources(pbsd_kernel_audit_trigger PUBLIC FILE_SET CXX_MODULES FILES
    kernel/audit/pbsd.kernel.audit_trigger.cppm)
target_link_libraries(pbsd_kernel_audit_trigger PUBLIC pbsd_core pbsd_kernel_audit)
target_compile_options(pbsd_kernel_audit_trigger PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bsm_socket_type)
add_library(pbsd_kernel_bsm_socket_type)
target_sources(pbsd_kernel_bsm_socket_type PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.bsm_socket_type.cppm)
target_link_libraries(pbsd_kernel_bsm_socket_type PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bsm_socket_type PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_idle)
add_library(pbsd_kernel_idle)
target_sources(pbsd_kernel_idle PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.idle.cppm)
target_link_libraries(pbsd_kernel_idle PUBLIC pbsd_core pbsd_kernel_sched)
target_compile_options(pbsd_kernel_idle PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_lockstat)
add_library(pbsd_kernel_lockstat)
target_sources(pbsd_kernel_lockstat PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.lockstat.cppm)
target_link_libraries(pbsd_kernel_lockstat PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_lockstat PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_power)
add_library(pbsd_kernel_power)
target_sources(pbsd_kernel_power PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.power.cppm)
target_link_libraries(pbsd_kernel_power PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_power PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_context)
add_library(pbsd_kernel_context)
target_sources(pbsd_kernel_context PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.context.cppm)
target_link_libraries(pbsd_kernel_context PUBLIC pbsd_core pbsd_kernel_signal)
target_compile_options(pbsd_kernel_context PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_hash)
add_library(pbsd_kernel_hash)
target_sources(pbsd_kernel_hash PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.hash.cppm)
target_link_libraries(pbsd_kernel_hash PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_hash PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_fattime)
add_library(pbsd_kernel_fattime)
target_sources(pbsd_kernel_fattime PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.fattime.cppm)
target_link_libraries(pbsd_kernel_fattime PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_fattime PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_firmware)
add_library(pbsd_kernel_firmware)
target_sources(pbsd_kernel_firmware PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.firmware.cppm)
target_link_libraries(pbsd_kernel_firmware PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_firmware PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_pctrie)
add_library(pbsd_kernel_pctrie)
target_sources(pbsd_kernel_pctrie PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.pctrie.cppm)
target_link_libraries(pbsd_kernel_pctrie PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_pctrie PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sglist)
add_library(pbsd_kernel_sglist)
target_sources(pbsd_kernel_sglist PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.sglist.cppm)
target_link_libraries(pbsd_kernel_sglist PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_sglist PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sfbuf)
add_library(pbsd_kernel_sfbuf)
target_sources(pbsd_kernel_sfbuf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.sfbuf.cppm)
target_link_libraries(pbsd_kernel_sfbuf PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_sfbuf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_rman)
add_library(pbsd_kernel_rman)
target_sources(pbsd_kernel_rman PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.rman.cppm)
target_link_libraries(pbsd_kernel_rman PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_rman PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_blist)
add_library(pbsd_kernel_blist)
target_sources(pbsd_kernel_blist PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.blist.cppm)
target_link_libraries(pbsd_kernel_blist PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_blist PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_hints)
add_library(pbsd_kernel_hints)
target_sources(pbsd_kernel_hints PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.hints.cppm)
target_link_libraries(pbsd_kernel_hints PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_hints PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_log)
add_library(pbsd_kernel_kern_log)
target_sources(pbsd_kernel_kern_log PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_log.cppm)
target_link_libraries(pbsd_kernel_kern_log PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_kern_log PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_mount)
add_library(pbsd_kernel_vfs_mount)
target_sources(pbsd_kernel_vfs_mount PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_mount.cppm)
target_link_libraries(pbsd_kernel_vfs_mount PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_vfs_mount PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_subr)
add_library(pbsd_kernel_vfs_subr)
target_sources(pbsd_kernel_vfs_subr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_subr.cppm)
target_link_libraries(pbsd_kernel_vfs_subr PUBLIC
    pbsd_core pbsd_kernel_vfs_mount)
target_compile_options(pbsd_kernel_vfs_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_lookup)
add_library(pbsd_kernel_vfs_lookup)
target_sources(pbsd_kernel_vfs_lookup PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_lookup.cppm)
target_link_libraries(pbsd_kernel_vfs_lookup PUBLIC
    pbsd_core pbsd_kernel_namei)
target_compile_options(pbsd_kernel_vfs_lookup PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_default)
add_library(pbsd_kernel_vfs_default)
target_sources(pbsd_kernel_vfs_default PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_default.cppm)
target_link_libraries(pbsd_kernel_vfs_default PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_vfs_default PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_syscalls)
add_library(pbsd_kernel_vfs_syscalls)
target_sources(pbsd_kernel_vfs_syscalls PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_syscalls.cppm)
target_link_libraries(pbsd_kernel_vfs_syscalls PUBLIC
    pbsd_core pbsd_kernel_vfs_mount)
target_compile_options(pbsd_kernel_vfs_syscalls PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_vnops)
add_library(pbsd_kernel_vfs_vnops)
target_sources(pbsd_kernel_vfs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_vnops.cppm)
target_link_libraries(pbsd_kernel_vfs_vnops PUBLIC
    pbsd_core pbsd_kernel_vnode)
target_compile_options(pbsd_kernel_vfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_hash)
add_library(pbsd_kernel_vfs_hash)
target_sources(pbsd_kernel_vfs_hash PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_hash.cppm)
target_link_libraries(pbsd_kernel_vfs_hash PUBLIC
    pbsd_core pbsd_kernel_hash)
target_compile_options(pbsd_kernel_vfs_hash PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vfs_cache)
add_library(pbsd_kernel_vfs_cache)
target_sources(pbsd_kernel_vfs_cache PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vfs/pbsd.kernel.vfs_cache.cppm)
target_link_libraries(pbsd_kernel_vfs_cache PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_vfs_cache PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_synch)
add_library(pbsd_kernel_synch)
target_sources(pbsd_kernel_synch PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.synch.cppm)
target_link_libraries(pbsd_kernel_synch PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_synch PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_mib)
add_library(pbsd_kernel_kern_mib)
target_sources(pbsd_kernel_kern_mib PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_mib.cppm)
target_link_libraries(pbsd_kernel_kern_mib PUBLIC
    pbsd_kernel_sysctl)
target_compile_options(pbsd_kernel_kern_mib PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_malloc)
add_library(pbsd_kernel_kern_malloc)
target_sources(pbsd_kernel_kern_malloc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_malloc.cppm)
target_link_libraries(pbsd_kernel_kern_malloc PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_kern_malloc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_proc)
add_library(pbsd_kernel_kern_proc)
target_sources(pbsd_kernel_kern_proc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_proc.cppm)
target_link_libraries(pbsd_kernel_kern_proc PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_kern_proc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_sig)
add_library(pbsd_kernel_kern_sig)
target_sources(pbsd_kernel_kern_sig PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_sig.cppm)
target_link_libraries(pbsd_kernel_kern_sig PUBLIC
    pbsd_kernel_signal)
target_compile_options(pbsd_kernel_kern_sig PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_fork)
add_library(pbsd_kernel_kern_fork)
target_sources(pbsd_kernel_kern_fork PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_fork.cppm)
target_link_libraries(pbsd_kernel_kern_fork PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_kern_fork PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_exit)
add_library(pbsd_kernel_kern_exit)
target_sources(pbsd_kernel_kern_exit PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_exit.cppm)
target_link_libraries(pbsd_kernel_kern_exit PUBLIC
    pbsd_core)
target_compile_options(pbsd_kernel_kern_exit PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sys_getrandom)
add_library(pbsd_kernel_sys_getrandom)
target_sources(pbsd_kernel_sys_getrandom PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.sys_getrandom.cppm)
target_link_libraries(pbsd_kernel_sys_getrandom PUBLIC pbsd_core pbsd_kernel_random)
target_compile_options(pbsd_kernel_sys_getrandom PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_unit)
add_library(pbsd_kernel_subr_unit)
target_sources(pbsd_kernel_subr_unit PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_unit.cppm)
target_link_libraries(pbsd_kernel_subr_unit PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_unit PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_hash)
add_library(pbsd_kernel_subr_hash)
target_sources(pbsd_kernel_subr_hash PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_hash.cppm)
target_link_libraries(pbsd_kernel_subr_hash PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_hash PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_msgbuf)
add_library(pbsd_kernel_subr_msgbuf)
target_sources(pbsd_kernel_subr_msgbuf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_msgbuf.cppm)
target_link_libraries(pbsd_kernel_subr_msgbuf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_msgbuf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_prf)
add_library(pbsd_kernel_subr_prf)
target_sources(pbsd_kernel_subr_prf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_prf.cppm)
target_link_libraries(pbsd_kernel_subr_prf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_prf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_rangeset)
add_library(pbsd_kernel_rangeset)
target_sources(pbsd_kernel_rangeset PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.rangeset.cppm)
target_link_libraries(pbsd_kernel_rangeset PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_rangeset PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_filter)
add_library(pbsd_kernel_filter)
target_sources(pbsd_kernel_filter PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.filter.cppm)
target_link_libraries(pbsd_kernel_filter PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_filter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_stack)
add_library(pbsd_kernel_stack)
target_sources(pbsd_kernel_stack PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.stack.cppm)
target_link_libraries(pbsd_kernel_stack PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_stack PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_pidctrl)
add_library(pbsd_kernel_pidctrl)
target_sources(pbsd_kernel_pidctrl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.pidctrl.cppm)
target_link_libraries(pbsd_kernel_pidctrl PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_pidctrl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_prng)
add_library(pbsd_kernel_prng)
target_sources(pbsd_kernel_prng PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.prng.cppm)
target_link_libraries(pbsd_kernel_prng PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_prng PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_lock)
add_library(pbsd_kernel_subr_lock)
target_sources(pbsd_kernel_subr_lock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_lock.cppm)
target_link_libraries(pbsd_kernel_subr_lock PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_lock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_scanf)
add_library(pbsd_kernel_scanf)
target_sources(pbsd_kernel_scanf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.scanf.cppm)
target_link_libraries(pbsd_kernel_scanf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_scanf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_gtaskqueue)
add_library(pbsd_kernel_gtaskqueue)
target_sources(pbsd_kernel_gtaskqueue PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.gtaskqueue.cppm)
target_link_libraries(pbsd_kernel_gtaskqueue PUBLIC pbsd_core pbsd_kernel_taskqueue)
target_compile_options(pbsd_kernel_gtaskqueue PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_physmem)
add_library(pbsd_kernel_physmem)
target_sources(pbsd_kernel_physmem PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.physmem.cppm)
target_link_libraries(pbsd_kernel_physmem PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_physmem PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vmem)
add_library(pbsd_kernel_vmem)
target_sources(pbsd_kernel_vmem PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.vmem.cppm)
target_link_libraries(pbsd_kernel_vmem PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vmem PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_stats)
add_library(pbsd_kernel_stats)
target_sources(pbsd_kernel_stats PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.stats.cppm)
target_link_libraries(pbsd_kernel_stats PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_stats PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_clock)
add_library(pbsd_kernel_clock)
target_sources(pbsd_kernel_clock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.clock.cppm)
target_link_libraries(pbsd_kernel_clock PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_clock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_boot)
add_library(pbsd_kernel_boot)
target_sources(pbsd_kernel_boot PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.boot.cppm)
target_link_libraries(pbsd_kernel_boot PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_boot PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_rangeset)
add_library(pbsd_kernel_subr_rangeset)
target_sources(pbsd_kernel_subr_rangeset PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_rangeset.cppm)
target_link_libraries(pbsd_kernel_subr_rangeset PUBLIC pbsd_core pbsd_kernel_pctrie)
target_compile_options(pbsd_kernel_subr_rangeset PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_filter)
add_library(pbsd_kernel_subr_filter)
target_sources(pbsd_kernel_subr_filter PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_filter.cppm)
target_link_libraries(pbsd_kernel_subr_filter PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_filter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_stack)
add_library(pbsd_kernel_subr_stack)
target_sources(pbsd_kernel_subr_stack PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_stack.cppm)
target_link_libraries(pbsd_kernel_subr_stack PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_stack PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_pidctrl)
add_library(pbsd_kernel_subr_pidctrl)
target_sources(pbsd_kernel_subr_pidctrl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_pidctrl.cppm)
target_link_libraries(pbsd_kernel_subr_pidctrl PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_pidctrl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_prng)
add_library(pbsd_kernel_subr_prng)
target_sources(pbsd_kernel_subr_prng PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_prng.cppm)
target_link_libraries(pbsd_kernel_subr_prng PUBLIC pbsd_core pbsd_kernel_random)
target_compile_options(pbsd_kernel_subr_prng PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mchain)
add_library(pbsd_kernel_mchain)
target_sources(pbsd_kernel_mchain PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.mchain.cppm)
target_link_libraries(pbsd_kernel_mchain PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mchain PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_mchain)
add_library(pbsd_kernel_subr_mchain)
target_sources(pbsd_kernel_subr_mchain PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_mchain.cppm)
target_link_libraries(pbsd_kernel_subr_mchain PUBLIC pbsd_core pbsd_kernel_mchain)
target_compile_options(pbsd_kernel_subr_mchain PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_rss)
add_library(pbsd_kernel_rss)
target_sources(pbsd_kernel_rss PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.rss.cppm)
target_link_libraries(pbsd_kernel_rss PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_rss PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_flowtable)
add_library(pbsd_kernel_flowtable)
target_sources(pbsd_kernel_flowtable PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.flowtable.cppm)
target_link_libraries(pbsd_kernel_flowtable PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_flowtable PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_memdesc)
add_library(pbsd_kernel_memdesc)
target_sources(pbsd_kernel_memdesc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.memdesc.cppm)
target_link_libraries(pbsd_kernel_memdesc PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_memdesc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_memdesc)
add_library(pbsd_kernel_subr_memdesc)
target_sources(pbsd_kernel_subr_memdesc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_memdesc.cppm)
target_link_libraries(pbsd_kernel_subr_memdesc PUBLIC pbsd_core pbsd_kernel_memdesc)
target_compile_options(pbsd_kernel_subr_memdesc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_map_helpers)
add_library(pbsd_kernel_vm_map_helpers)
target_sources(pbsd_kernel_vm_map_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_map_helpers.cppm)
target_link_libraries(pbsd_kernel_vm_map_helpers PUBLIC pbsd_core pbsd_kernel_vm pbsd_kernel_vm_map)
target_compile_options(pbsd_kernel_vm_map_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_ether)
add_library(pbsd_kernel_ether)
target_sources(pbsd_kernel_ether PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.ether.cppm)
target_link_libraries(pbsd_kernel_ether PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_ether PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_prf)
add_library(pbsd_kernel_prf)
target_sources(pbsd_kernel_prf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.prf.cppm)
target_link_libraries(pbsd_kernel_prf PUBLIC pbsd_core pbsd_kernel_subr_prf)
target_compile_options(pbsd_kernel_prf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_unit)
add_library(pbsd_kernel_unit)
target_sources(pbsd_kernel_unit PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.unit.cppm)
target_link_libraries(pbsd_kernel_unit PUBLIC pbsd_core pbsd_kernel_subr_unit)
target_compile_options(pbsd_kernel_unit PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_altq)
add_library(pbsd_kernel_altq)
target_sources(pbsd_kernel_altq PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.altq.cppm)
target_link_libraries(pbsd_kernel_altq PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_altq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_counter)
add_library(pbsd_kernel_subr_counter)
target_sources(pbsd_kernel_subr_counter PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_counter.cppm)
target_link_libraries(pbsd_kernel_subr_counter PUBLIC pbsd_core pbsd_kernel_counter)
target_compile_options(pbsd_kernel_subr_counter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_sbuf)
add_library(pbsd_kernel_subr_sbuf)
target_sources(pbsd_kernel_subr_sbuf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_sbuf.cppm)
target_link_libraries(pbsd_kernel_subr_sbuf PUBLIC pbsd_core pbsd_kernel_sbuf)
target_compile_options(pbsd_kernel_subr_sbuf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_panic)
add_library(pbsd_kernel_subr_panic)
target_sources(pbsd_kernel_subr_panic PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_panic.cppm)
target_link_libraries(pbsd_kernel_subr_panic PUBLIC pbsd_core pbsd_kernel_panic)
target_compile_options(pbsd_kernel_subr_panic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_uuid)
add_library(pbsd_kernel_subr_uuid)
target_sources(pbsd_kernel_subr_uuid PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_uuid.cppm)
target_link_libraries(pbsd_kernel_subr_uuid PUBLIC pbsd_core pbsd_kernel_uuid)
target_compile_options(pbsd_kernel_subr_uuid PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_linker)
add_library(pbsd_kernel_subr_linker)
target_sources(pbsd_kernel_subr_linker PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_linker.cppm)
target_link_libraries(pbsd_kernel_subr_linker PUBLIC pbsd_core pbsd_kernel_linker)
target_compile_options(pbsd_kernel_subr_linker PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_vmem)
add_library(pbsd_kernel_subr_vmem)
target_sources(pbsd_kernel_subr_vmem PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_vmem.cppm)
target_link_libraries(pbsd_kernel_subr_vmem PUBLIC pbsd_core pbsd_kernel_vmem)
target_compile_options(pbsd_kernel_subr_vmem PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_bus)
add_library(pbsd_kernel_subr_bus)
target_sources(pbsd_kernel_subr_bus PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_bus.cppm)
target_link_libraries(pbsd_kernel_subr_bus PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_bus PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_device)
add_library(pbsd_kernel_device)
target_sources(pbsd_kernel_device PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.device.cppm)
target_link_libraries(pbsd_kernel_device PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_device PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_meter)
add_library(pbsd_kernel_vm_meter)
target_sources(pbsd_kernel_vm_meter PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_meter.cppm)
target_link_libraries(pbsd_kernel_vm_meter PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vm_meter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_kern)
add_library(pbsd_kernel_vm_kern)
target_sources(pbsd_kernel_vm_kern PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_kern.cppm)
target_link_libraries(pbsd_kernel_vm_kern PUBLIC pbsd_core pbsd_kernel_vm)
target_compile_options(pbsd_kernel_vm_kern PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_reserv)
add_library(pbsd_kernel_vm_reserv)
target_sources(pbsd_kernel_vm_reserv PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_reserv.cppm)
target_link_libraries(pbsd_kernel_vm_reserv PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_vm_reserv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_ether)
add_library(pbsd_kernel_subr_ether)
target_sources(pbsd_kernel_subr_ether PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.subr_ether.cppm)
target_link_libraries(pbsd_kernel_subr_ether PUBLIC pbsd_core pbsd_kernel_ether)
target_compile_options(pbsd_kernel_subr_ether PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mbuf_helpers)
add_library(pbsd_kernel_mbuf_helpers)
target_sources(pbsd_kernel_mbuf_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/mbuf/pbsd.kernel.mbuf_helpers.cppm)
target_link_libraries(pbsd_kernel_mbuf_helpers PUBLIC pbsd_core pbsd_kernel_mbuf)
target_compile_options(pbsd_kernel_mbuf_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_ifnet_helpers)
add_library(pbsd_kernel_ifnet_helpers)
target_sources(pbsd_kernel_ifnet_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/ifnet/pbsd.kernel.ifnet_helpers.cppm)
target_link_libraries(pbsd_kernel_ifnet_helpers PUBLIC pbsd_core pbsd_kernel_ifnet)
target_compile_options(pbsd_kernel_ifnet_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_rtentry_helpers)
add_library(pbsd_kernel_rtentry_helpers)
target_sources(pbsd_kernel_rtentry_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.rtentry_helpers.cppm)
target_link_libraries(pbsd_kernel_rtentry_helpers PUBLIC pbsd_core pbsd_kernel_rtentry)
target_compile_options(pbsd_kernel_rtentry_helpers PUBLIC ${PBSD_FS_CXX})
endif()


if(NOT TARGET pbsd_kernel_subr_uio)
add_library(pbsd_kernel_subr_uio)
target_sources(pbsd_kernel_subr_uio PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_uio.cppm)
target_link_libraries(pbsd_kernel_subr_uio PUBLIC pbsd_core pbsd_kernel_uio)
target_compile_options(pbsd_kernel_subr_uio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_turnstile)
add_library(pbsd_kernel_subr_turnstile)
target_sources(pbsd_kernel_subr_turnstile PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_turnstile.cppm)
target_link_libraries(pbsd_kernel_subr_turnstile PUBLIC pbsd_core pbsd_kernel_turnstile)
target_compile_options(pbsd_kernel_subr_turnstile PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_sleepqueue)
add_library(pbsd_kernel_subr_sleepqueue)
target_sources(pbsd_kernel_subr_sleepqueue PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_sleepqueue.cppm)
target_link_libraries(pbsd_kernel_subr_sleepqueue PUBLIC pbsd_core pbsd_kernel_sleepq)
target_compile_options(pbsd_kernel_subr_sleepqueue PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_witness)
add_library(pbsd_kernel_subr_witness)
target_sources(pbsd_kernel_subr_witness PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_witness.cppm)
target_link_libraries(pbsd_kernel_subr_witness PUBLIC pbsd_core pbsd_kernel_witness)
target_compile_options(pbsd_kernel_subr_witness PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_bufring)
add_library(pbsd_kernel_subr_bufring)
target_sources(pbsd_kernel_subr_bufring PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_bufring.cppm)
target_link_libraries(pbsd_kernel_subr_bufring PUBLIC pbsd_core pbsd_kernel_bufring)
target_compile_options(pbsd_kernel_subr_bufring PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_eventhandler)
add_library(pbsd_kernel_subr_eventhandler)
target_sources(pbsd_kernel_subr_eventhandler PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_eventhandler.cppm)
target_link_libraries(pbsd_kernel_subr_eventhandler PUBLIC pbsd_core pbsd_kernel_eventhandler)
target_compile_options(pbsd_kernel_subr_eventhandler PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_object_helpers)
add_library(pbsd_kernel_vm_object_helpers)
target_sources(pbsd_kernel_vm_object_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_object_helpers.cppm)
target_link_libraries(pbsd_kernel_vm_object_helpers PUBLIC pbsd_core pbsd_kernel_vm_object)
target_compile_options(pbsd_kernel_vm_object_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_page_helpers)
add_library(pbsd_kernel_vm_page_helpers)
target_sources(pbsd_kernel_vm_page_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_page_helpers.cppm)
target_link_libraries(pbsd_kernel_vm_page_helpers PUBLIC pbsd_core pbsd_kernel_vm_page)
target_compile_options(pbsd_kernel_vm_page_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_fault_helpers)
add_library(pbsd_kernel_vm_fault_helpers)
target_sources(pbsd_kernel_vm_fault_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_fault_helpers.cppm)
target_link_libraries(pbsd_kernel_vm_fault_helpers PUBLIC pbsd_core pbsd_kernel_vm_fault pbsd_kernel_vm)
target_compile_options(pbsd_kernel_vm_fault_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sync_helpers)
add_library(pbsd_kernel_sync_helpers)
target_sources(pbsd_kernel_sync_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/sync/pbsd.kernel.sync_helpers.cppm)
target_link_libraries(pbsd_kernel_sync_helpers PUBLIC pbsd_core pbsd_kernel_mutex pbsd_kernel_turnstile pbsd_kernel_subr_turnstile pbsd_kernel_sleepq)
target_compile_options(pbsd_kernel_sync_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_in_cksum)
add_library(pbsd_kernel_in_cksum)
target_sources(pbsd_kernel_in_cksum PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.in_cksum.cppm)
target_link_libraries(pbsd_kernel_in_cksum PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_in_cksum PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_link_addr)
add_library(pbsd_kernel_link_addr)
target_sources(pbsd_kernel_link_addr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.link_addr.cppm)
target_link_libraries(pbsd_kernel_link_addr PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_link_addr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_pktsched)
add_library(pbsd_kernel_pktsched)
target_sources(pbsd_kernel_pktsched PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.pktsched.cppm)
target_link_libraries(pbsd_kernel_pktsched PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_pktsched PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_intr)
add_library(pbsd_kernel_subr_intr)
target_sources(pbsd_kernel_subr_intr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_intr.cppm)
target_link_libraries(pbsd_kernel_subr_intr PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_intr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_epoch)
add_library(pbsd_kernel_subr_epoch)
target_sources(pbsd_kernel_subr_epoch PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_epoch.cppm)
target_link_libraries(pbsd_kernel_subr_epoch PUBLIC pbsd_core pbsd_kernel_epoch)
target_compile_options(pbsd_kernel_subr_epoch PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_smr)
add_library(pbsd_kernel_subr_smr)
target_sources(pbsd_kernel_subr_smr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_smr.cppm)
target_link_libraries(pbsd_kernel_subr_smr PUBLIC pbsd_core pbsd_kernel_smr)
target_compile_options(pbsd_kernel_subr_smr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_taskqueue)
add_library(pbsd_kernel_subr_taskqueue)
target_sources(pbsd_kernel_subr_taskqueue PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_taskqueue.cppm)
target_link_libraries(pbsd_kernel_subr_taskqueue PUBLIC pbsd_core pbsd_kernel_taskqueue)
target_compile_options(pbsd_kernel_subr_taskqueue PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_devstat)
add_library(pbsd_kernel_subr_devstat)
target_sources(pbsd_kernel_subr_devstat PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_devstat.cppm)
target_link_libraries(pbsd_kernel_subr_devstat PUBLIC pbsd_core pbsd_kernel_devstat)
target_compile_options(pbsd_kernel_subr_devstat PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_disk)
add_library(pbsd_kernel_subr_disk)
target_sources(pbsd_kernel_subr_disk PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_disk.cppm)
target_link_libraries(pbsd_kernel_subr_disk PUBLIC pbsd_core pbsd_kernel_disk)
target_compile_options(pbsd_kernel_subr_disk PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_power)
add_library(pbsd_kernel_subr_power)
target_sources(pbsd_kernel_subr_power PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_power.cppm)
target_link_libraries(pbsd_kernel_subr_power PUBLIC pbsd_core pbsd_kernel_power)
target_compile_options(pbsd_kernel_subr_power PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_pcpu)
add_library(pbsd_kernel_subr_pcpu)
target_sources(pbsd_kernel_subr_pcpu PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_pcpu.cppm)
target_link_libraries(pbsd_kernel_subr_pcpu PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_pcpu PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_stats)
add_library(pbsd_kernel_subr_stats)
target_sources(pbsd_kernel_subr_stats PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_stats.cppm)
target_link_libraries(pbsd_kernel_subr_stats PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_stats PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_trap)
add_library(pbsd_kernel_subr_trap)
target_sources(pbsd_kernel_subr_trap PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_trap.cppm)
target_link_libraries(pbsd_kernel_subr_trap PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_trap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_module)
add_library(pbsd_kernel_subr_module)
target_sources(pbsd_kernel_subr_module PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_module.cppm)
target_link_libraries(pbsd_kernel_subr_module PUBLIC pbsd_core pbsd_kernel_kmod)
target_compile_options(pbsd_kernel_subr_module PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_log)
add_library(pbsd_kernel_subr_log)
target_sources(pbsd_kernel_subr_log PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_log.cppm)
target_link_libraries(pbsd_kernel_subr_log PUBLIC pbsd_core pbsd_kernel_kern_log)
target_compile_options(pbsd_kernel_subr_log PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_autoconf)
add_library(pbsd_kernel_subr_autoconf)
target_sources(pbsd_kernel_subr_autoconf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_autoconf.cppm)
target_link_libraries(pbsd_kernel_subr_autoconf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_autoconf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_bus_dma)
add_library(pbsd_kernel_subr_bus_dma)
target_sources(pbsd_kernel_subr_bus_dma PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_bus_dma.cppm)
target_link_libraries(pbsd_kernel_subr_bus_dma PUBLIC pbsd_core pbsd_kernel_busdma)
target_compile_options(pbsd_kernel_subr_bus_dma PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_capability)
add_library(pbsd_kernel_subr_capability)
target_sources(pbsd_kernel_subr_capability PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_capability.cppm)
target_link_libraries(pbsd_kernel_subr_capability PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_capability PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_clock)
add_library(pbsd_kernel_subr_clock)
target_sources(pbsd_kernel_subr_clock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_clock.cppm)
target_link_libraries(pbsd_kernel_subr_clock PUBLIC pbsd_core pbsd_kernel_clock)
target_compile_options(pbsd_kernel_subr_clock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_early)
add_library(pbsd_kernel_subr_early)
target_sources(pbsd_kernel_subr_early PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_early.cppm)
target_link_libraries(pbsd_kernel_subr_early PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_early PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_kdb)
add_library(pbsd_kernel_subr_kdb)
target_sources(pbsd_kernel_subr_kdb PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_kdb.cppm)
target_link_libraries(pbsd_kernel_subr_kdb PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_kdb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_param)
add_library(pbsd_kernel_subr_param)
target_sources(pbsd_kernel_subr_param PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_param.cppm)
target_link_libraries(pbsd_kernel_subr_param PUBLIC pbsd_core pbsd_kernel_param)
target_compile_options(pbsd_kernel_subr_param PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_prof)
add_library(pbsd_kernel_subr_prof)
target_sources(pbsd_kernel_subr_prof PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_prof.cppm)
target_link_libraries(pbsd_kernel_subr_prof PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_prof PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_rtc)
add_library(pbsd_kernel_subr_rtc)
target_sources(pbsd_kernel_subr_rtc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_rtc.cppm)
target_link_libraries(pbsd_kernel_subr_rtc PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_rtc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_smp)
add_library(pbsd_kernel_subr_smp)
target_sources(pbsd_kernel_subr_smp PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_smp.cppm)
target_link_libraries(pbsd_kernel_subr_smp PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_smp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_terminal)
add_library(pbsd_kernel_subr_terminal)
target_sources(pbsd_kernel_subr_terminal PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_terminal.cppm)
target_link_libraries(pbsd_kernel_subr_terminal PUBLIC pbsd_core pbsd_kernel_tty)
target_compile_options(pbsd_kernel_subr_terminal PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_acl_nfs4)
add_library(pbsd_kernel_subr_acl_nfs4)
target_sources(pbsd_kernel_subr_acl_nfs4 PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_acl_nfs4.cppm)
target_link_libraries(pbsd_kernel_subr_acl_nfs4 PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_acl_nfs4 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_acl_posix1e)
add_library(pbsd_kernel_subr_acl_posix1e)
target_sources(pbsd_kernel_subr_acl_posix1e PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_acl_posix1e.cppm)
target_link_libraries(pbsd_kernel_subr_acl_posix1e PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_acl_posix1e PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_devmap)
add_library(pbsd_kernel_subr_devmap)
target_sources(pbsd_kernel_subr_devmap PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_devmap.cppm)
target_link_libraries(pbsd_kernel_subr_devmap PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_devmap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_syscall)
add_library(pbsd_kernel_subr_syscall)
target_sources(pbsd_kernel_subr_syscall PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_syscall.cppm)
target_link_libraries(pbsd_kernel_subr_syscall PUBLIC pbsd_core pbsd_kernel_syscall)
target_compile_options(pbsd_kernel_subr_syscall PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_boot)
add_library(pbsd_kernel_subr_boot)
target_sources(pbsd_kernel_subr_boot PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_boot.cppm)
target_link_libraries(pbsd_kernel_subr_boot PUBLIC pbsd_core pbsd_kernel_boot)
target_compile_options(pbsd_kernel_subr_boot PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_busdma_bounce)
add_library(pbsd_kernel_subr_busdma_bounce)
target_sources(pbsd_kernel_subr_busdma_bounce PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_busdma_bounce.cppm)
target_link_libraries(pbsd_kernel_subr_busdma_bounce PUBLIC pbsd_core pbsd_kernel_busdma)
target_compile_options(pbsd_kernel_subr_busdma_bounce PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_busdma_bufalloc)
add_library(pbsd_kernel_subr_busdma_bufalloc)
target_sources(pbsd_kernel_subr_busdma_bufalloc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_busdma_bufalloc.cppm)
target_link_libraries(pbsd_kernel_subr_busdma_bufalloc PUBLIC pbsd_core pbsd_kernel_busdma)
target_compile_options(pbsd_kernel_subr_busdma_bufalloc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_madvise)
add_library(pbsd_kernel_madvise)
target_sources(pbsd_kernel_madvise PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.madvise.cppm)
target_link_libraries(pbsd_kernel_madvise PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_madvise PUBLIC ${PBSD_FS_CXX})
endif()


if(NOT TARGET pbsd_kernel_kern_exec)
add_library(pbsd_kernel_kern_exec)
target_sources(pbsd_kernel_kern_exec PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_exec.cppm)
target_link_libraries(pbsd_kernel_kern_exec PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_exec PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_descrip)
add_library(pbsd_kernel_kern_descrip)
target_sources(pbsd_kernel_kern_descrip PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_descrip.cppm)
target_link_libraries(pbsd_kernel_kern_descrip PUBLIC pbsd_core pbsd_kernel_filedesc)
target_compile_options(pbsd_kernel_kern_descrip PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_syscalls)
add_library(pbsd_kernel_kern_syscalls)
target_sources(pbsd_kernel_kern_syscalls PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_syscalls.cppm)
target_link_libraries(pbsd_kernel_kern_syscalls PUBLIC pbsd_core pbsd_kernel_syscall)
target_compile_options(pbsd_kernel_kern_syscalls PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_thread)
add_library(pbsd_kernel_kern_thread)
target_sources(pbsd_kernel_kern_thread PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_thread.cppm)
target_link_libraries(pbsd_kernel_kern_thread PUBLIC pbsd_core pbsd_handles)
target_compile_options(pbsd_kernel_kern_thread PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_switch)
add_library(pbsd_kernel_kern_switch)
target_sources(pbsd_kernel_kern_switch PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_switch.cppm)
target_link_libraries(pbsd_kernel_kern_switch PUBLIC pbsd_core pbsd_kernel_sched)
target_compile_options(pbsd_kernel_kern_switch PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_time)
add_library(pbsd_kernel_kern_time)
target_sources(pbsd_kernel_kern_time PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_time.cppm)
target_link_libraries(pbsd_kernel_kern_time PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_kern_time PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_event)
add_library(pbsd_kernel_kern_event)
target_sources(pbsd_kernel_kern_event PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_event.cppm)
target_link_libraries(pbsd_kernel_kern_event PUBLIC pbsd_core pbsd_kernel_kevent)
target_compile_options(pbsd_kernel_kern_event PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_umtx)
add_library(pbsd_kernel_kern_umtx)
target_sources(pbsd_kernel_kern_umtx PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_umtx.cppm)
target_link_libraries(pbsd_kernel_kern_umtx PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_umtx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_module)
add_library(pbsd_kernel_kern_module)
target_sources(pbsd_kernel_kern_module PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_module.cppm)
target_link_libraries(pbsd_kernel_kern_module PUBLIC pbsd_core pbsd_kernel_kmod)
target_compile_options(pbsd_kernel_kern_module PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_cpu)
add_library(pbsd_kernel_kern_cpu)
target_sources(pbsd_kernel_kern_cpu PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_cpu.cppm)
target_link_libraries(pbsd_kernel_kern_cpu PUBLIC pbsd_core pbsd_kernel_cpuset)
target_compile_options(pbsd_kernel_kern_cpu PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_conf)
add_library(pbsd_kernel_kern_conf)
target_sources(pbsd_kernel_kern_conf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_conf.cppm)
target_link_libraries(pbsd_kernel_kern_conf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_conf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_environment)
add_library(pbsd_kernel_kern_environment)
target_sources(pbsd_kernel_kern_environment PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_environment.cppm)
target_link_libraries(pbsd_kernel_kern_environment PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_environment PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_lock)
add_library(pbsd_kernel_kern_lock)
target_sources(pbsd_kernel_kern_lock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_lock.cppm)
target_link_libraries(pbsd_kernel_kern_lock PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_lock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_sendfile)
add_library(pbsd_kernel_kern_sendfile)
target_sources(pbsd_kernel_kern_sendfile PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_sendfile.cppm)
target_link_libraries(pbsd_kernel_kern_sendfile PUBLIC pbsd_core pbsd_kernel_socket_syscall)
target_compile_options(pbsd_kernel_kern_sendfile PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_sharedpage)
add_library(pbsd_kernel_kern_sharedpage)
target_sources(pbsd_kernel_kern_sharedpage PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_sharedpage.cppm)
target_link_libraries(pbsd_kernel_kern_sharedpage PUBLIC pbsd_core pbsd_kernel_vm)
target_compile_options(pbsd_kernel_kern_sharedpage PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_thr)
add_library(pbsd_kernel_kern_thr)
target_sources(pbsd_kernel_kern_thr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_thr.cppm)
target_link_libraries(pbsd_kernel_kern_thr PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_thr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_ntptime)
add_library(pbsd_kernel_kern_ntptime)
target_sources(pbsd_kernel_kern_ntptime PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_ntptime.cppm)
target_link_libraries(pbsd_kernel_kern_ntptime PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_kern_ntptime PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_physio)
add_library(pbsd_kernel_kern_physio)
target_sources(pbsd_kernel_kern_physio PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_physio.cppm)
target_link_libraries(pbsd_kernel_kern_physio PUBLIC pbsd_core pbsd_kernel_bio)
target_compile_options(pbsd_kernel_kern_physio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_procctl)
add_library(pbsd_kernel_kern_procctl)
target_sources(pbsd_kernel_kern_procctl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_procctl.cppm)
target_link_libraries(pbsd_kernel_kern_procctl PUBLIC pbsd_core pbsd_kernel_process)
target_compile_options(pbsd_kernel_kern_procctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_membarrier)
add_library(pbsd_kernel_kern_membarrier)
target_sources(pbsd_kernel_kern_membarrier PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_membarrier.cppm)
target_link_libraries(pbsd_kernel_kern_membarrier PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_membarrier PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_resource)
add_library(pbsd_kernel_kern_resource)
target_sources(pbsd_kernel_kern_resource PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_resource.cppm)
target_link_libraries(pbsd_kernel_kern_resource PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_resource PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_alq)
add_library(pbsd_kernel_kern_alq)
target_sources(pbsd_kernel_kern_alq PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_alq.cppm)
target_link_libraries(pbsd_kernel_kern_alq PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_alq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_boottrace)
add_library(pbsd_kernel_kern_boottrace)
target_sources(pbsd_kernel_kern_boottrace PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_boottrace.cppm)
target_link_libraries(pbsd_kernel_kern_boottrace PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_boottrace PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_clocksource)
add_library(pbsd_kernel_kern_clocksource)
target_sources(pbsd_kernel_kern_clocksource PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_clocksource.cppm)
target_link_libraries(pbsd_kernel_kern_clocksource PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_kern_clocksource PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_ctf)
add_library(pbsd_kernel_kern_ctf)
target_sources(pbsd_kernel_kern_ctf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_ctf.cppm)
target_link_libraries(pbsd_kernel_kern_ctf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_ctf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_devctl)
add_library(pbsd_kernel_kern_devctl)
target_sources(pbsd_kernel_kern_devctl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_devctl.cppm)
target_link_libraries(pbsd_kernel_kern_devctl PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_devctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_dump)
add_library(pbsd_kernel_kern_dump)
target_sources(pbsd_kernel_kern_dump PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_dump.cppm)
target_link_libraries(pbsd_kernel_kern_dump PUBLIC pbsd_core pbsd_kernel_bio)
target_compile_options(pbsd_kernel_kern_dump PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_fail)
add_library(pbsd_kernel_kern_fail)
target_sources(pbsd_kernel_kern_fail PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_fail.cppm)
target_link_libraries(pbsd_kernel_kern_fail PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_fail PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_ffclock)
add_library(pbsd_kernel_kern_ffclock)
target_sources(pbsd_kernel_kern_ffclock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_ffclock.cppm)
target_link_libraries(pbsd_kernel_kern_ffclock PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_kern_ffclock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_hhook)
add_library(pbsd_kernel_kern_hhook)
target_sources(pbsd_kernel_kern_hhook PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_hhook.cppm)
target_link_libraries(pbsd_kernel_kern_hhook PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_hhook PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_jaildesc)
add_library(pbsd_kernel_kern_jaildesc)
target_sources(pbsd_kernel_kern_jaildesc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_jaildesc.cppm)
target_link_libraries(pbsd_kernel_kern_jaildesc PUBLIC pbsd_core pbsd_kernel_jail)
target_compile_options(pbsd_kernel_kern_jaildesc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_jailmeta)
add_library(pbsd_kernel_kern_jailmeta)
target_sources(pbsd_kernel_kern_jailmeta PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_jailmeta.cppm)
target_link_libraries(pbsd_kernel_kern_jailmeta PUBLIC pbsd_core pbsd_kernel_jail)
target_compile_options(pbsd_kernel_kern_jailmeta PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_kcov)
add_library(pbsd_kernel_kern_kcov)
target_sources(pbsd_kernel_kern_kcov PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_kcov.cppm)
target_link_libraries(pbsd_kernel_kern_kcov PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_kcov PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_khelp)
add_library(pbsd_kernel_kern_khelp)
target_sources(pbsd_kernel_kern_khelp PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_khelp.cppm)
target_link_libraries(pbsd_kernel_kern_khelp PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_khelp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_kthread)
add_library(pbsd_kernel_kern_kthread)
target_sources(pbsd_kernel_kern_kthread PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_kthread.cppm)
target_link_libraries(pbsd_kernel_kern_kthread PUBLIC pbsd_core pbsd_handles)
target_compile_options(pbsd_kernel_kern_kthread PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_ktr)
add_library(pbsd_kernel_kern_ktr)
target_sources(pbsd_kernel_kern_ktr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_ktr.cppm)
target_link_libraries(pbsd_kernel_kern_ktr PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_ktr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_loginclass)
add_library(pbsd_kernel_kern_loginclass)
target_sources(pbsd_kernel_kern_loginclass PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_loginclass.cppm)
target_link_libraries(pbsd_kernel_kern_loginclass PUBLIC pbsd_core pbsd_kernel_racct)
target_compile_options(pbsd_kernel_kern_loginclass PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_mtxpool)
add_library(pbsd_kernel_kern_mtxpool)
target_sources(pbsd_kernel_kern_mtxpool PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_mtxpool.cppm)
target_link_libraries(pbsd_kernel_kern_mtxpool PUBLIC pbsd_core pbsd_kernel_mutex)
target_compile_options(pbsd_kernel_kern_mtxpool PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_osd)
add_library(pbsd_kernel_kern_osd)
target_sources(pbsd_kernel_kern_osd PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_osd.cppm)
target_link_libraries(pbsd_kernel_kern_osd PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_osd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_pmc)
add_library(pbsd_kernel_kern_pmc)
target_sources(pbsd_kernel_kern_pmc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_pmc.cppm)
target_link_libraries(pbsd_kernel_kern_pmc PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_pmc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_priv)
add_library(pbsd_kernel_kern_priv)
target_sources(pbsd_kernel_kern_priv PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_priv.cppm)
target_link_libraries(pbsd_kernel_kern_priv PUBLIC pbsd_core pbsd_kernel_ucred)
target_compile_options(pbsd_kernel_kern_priv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_rangelock)
add_library(pbsd_kernel_kern_rangelock)
target_sources(pbsd_kernel_kern_rangelock PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_rangelock.cppm)
target_link_libraries(pbsd_kernel_kern_rangelock PUBLIC pbsd_core pbsd_kernel_rmlock)
target_compile_options(pbsd_kernel_kern_rangelock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_sema)
add_library(pbsd_kernel_kern_sema)
target_sources(pbsd_kernel_kern_sema PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_sema.cppm)
target_link_libraries(pbsd_kernel_kern_sema PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_sema PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_shutdown)
add_library(pbsd_kernel_kern_shutdown)
target_sources(pbsd_kernel_kern_shutdown PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_shutdown.cppm)
target_link_libraries(pbsd_kernel_kern_shutdown PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_shutdown PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_tc)
add_library(pbsd_kernel_kern_tc)
target_sources(pbsd_kernel_kern_tc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_tc.cppm)
target_link_libraries(pbsd_kernel_kern_tc PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_kern_tc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_tslog)
add_library(pbsd_kernel_kern_tslog)
target_sources(pbsd_kernel_kern_tslog PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_tslog.cppm)
target_link_libraries(pbsd_kernel_kern_tslog PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_tslog PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_ucoredump)
add_library(pbsd_kernel_kern_ucoredump)
target_sources(pbsd_kernel_kern_ucoredump PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_ucoredump.cppm)
target_link_libraries(pbsd_kernel_kern_ucoredump PUBLIC pbsd_core pbsd_kernel_process)
target_compile_options(pbsd_kernel_kern_ucoredump PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_vnodedumper)
add_library(pbsd_kernel_kern_vnodedumper)
target_sources(pbsd_kernel_kern_vnodedumper PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_vnodedumper.cppm)
target_link_libraries(pbsd_kernel_kern_vnodedumper PUBLIC pbsd_core pbsd_kernel_vnode)
target_compile_options(pbsd_kernel_kern_vnodedumper PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_atomic64)
add_library(pbsd_kernel_subr_atomic64)
target_sources(pbsd_kernel_subr_atomic64 PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_atomic64.cppm)
target_link_libraries(pbsd_kernel_subr_atomic64 PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_atomic64 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_clockcalib)
add_library(pbsd_kernel_subr_clockcalib)
target_sources(pbsd_kernel_subr_clockcalib PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_clockcalib.cppm)
target_link_libraries(pbsd_kernel_subr_clockcalib PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_subr_clockcalib PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_compressor)
add_library(pbsd_kernel_subr_compressor)
target_sources(pbsd_kernel_subr_compressor PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_compressor.cppm)
target_link_libraries(pbsd_kernel_subr_compressor PUBLIC pbsd_core pbsd_kernel_vm)
target_compile_options(pbsd_kernel_subr_compressor PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_coverage)
add_library(pbsd_kernel_subr_coverage)
target_sources(pbsd_kernel_subr_coverage PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_coverage.cppm)
target_link_libraries(pbsd_kernel_subr_coverage PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_coverage PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_dummy_vdso_tc)
add_library(pbsd_kernel_subr_dummy_vdso_tc)
target_sources(pbsd_kernel_subr_dummy_vdso_tc PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_dummy_vdso_tc.cppm)
target_link_libraries(pbsd_kernel_subr_dummy_vdso_tc PUBLIC pbsd_core pbsd_kernel_timekeeping)
target_compile_options(pbsd_kernel_subr_dummy_vdso_tc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_efi_map)
add_library(pbsd_kernel_subr_efi_map)
target_sources(pbsd_kernel_subr_efi_map PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_efi_map.cppm)
target_link_libraries(pbsd_kernel_subr_efi_map PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_efi_map PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_sdt)
add_library(pbsd_kernel_kern_sdt)
target_sources(pbsd_kernel_kern_sdt PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_sdt.cppm)
target_link_libraries(pbsd_kernel_kern_sdt PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_sdt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_ubsan)
add_library(pbsd_kernel_kern_ubsan)
target_sources(pbsd_kernel_kern_ubsan PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_ubsan.cppm)
target_link_libraries(pbsd_kernel_kern_ubsan PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_ubsan PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_dtrace)
add_library(pbsd_kernel_kern_dtrace)
target_sources(pbsd_kernel_kern_dtrace PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_dtrace.cppm)
target_link_libraries(pbsd_kernel_kern_dtrace PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_dtrace PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_et)
add_library(pbsd_kernel_kern_et)
target_sources(pbsd_kernel_kern_et PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.kern_et.cppm)
target_link_libraries(pbsd_kernel_kern_et PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_kern_et PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_asan)
add_library(pbsd_kernel_subr_asan)
target_sources(pbsd_kernel_subr_asan PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_asan.cppm)
target_link_libraries(pbsd_kernel_subr_asan PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_asan PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_csan)
add_library(pbsd_kernel_subr_csan)
target_sources(pbsd_kernel_subr_csan PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_csan.cppm)
target_link_libraries(pbsd_kernel_subr_csan PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_csan PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_subr_msan)
add_library(pbsd_kernel_subr_msan)
target_sources(pbsd_kernel_subr_msan PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.subr_msan.cppm)
target_link_libraries(pbsd_kernel_subr_msan PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_subr_msan PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_pageout)
add_library(pbsd_kernel_vm_pageout)
target_sources(pbsd_kernel_vm_pageout PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_pageout.cppm)
target_link_libraries(pbsd_kernel_vm_pageout PUBLIC pbsd_core pbsd_kernel_vm_page)
target_compile_options(pbsd_kernel_vm_pageout PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_mmap)
add_library(pbsd_kernel_vm_mmap)
target_sources(pbsd_kernel_vm_mmap PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_mmap.cppm)
target_link_libraries(pbsd_kernel_vm_mmap PUBLIC pbsd_core pbsd_kernel_vm_map)
target_compile_options(pbsd_kernel_vm_mmap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_init)
add_library(pbsd_kernel_vm_init)
target_sources(pbsd_kernel_vm_init PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_init.cppm)
target_link_libraries(pbsd_kernel_vm_init PUBLIC pbsd_core pbsd_kernel_vm)
target_compile_options(pbsd_kernel_vm_init PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_glue)
add_library(pbsd_kernel_vm_glue)
target_sources(pbsd_kernel_vm_glue PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_glue.cppm)
target_link_libraries(pbsd_kernel_vm_glue PUBLIC pbsd_core pbsd_kernel_vm)
target_compile_options(pbsd_kernel_vm_glue PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_device_pager)
add_library(pbsd_kernel_device_pager)
target_sources(pbsd_kernel_device_pager PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.device_pager.cppm)
target_link_libraries(pbsd_kernel_device_pager PUBLIC pbsd_core pbsd_kernel_vm_pager)
target_compile_options(pbsd_kernel_device_pager PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_swap_pager)
add_library(pbsd_kernel_swap_pager)
target_sources(pbsd_kernel_swap_pager PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.swap_pager.cppm)
target_link_libraries(pbsd_kernel_swap_pager PUBLIC pbsd_core pbsd_kernel_vm_pager)
target_compile_options(pbsd_kernel_swap_pager PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_phys_pager)
add_library(pbsd_kernel_phys_pager)
target_sources(pbsd_kernel_phys_pager PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.phys_pager.cppm)
target_link_libraries(pbsd_kernel_phys_pager PUBLIC pbsd_core pbsd_kernel_vm_pager)
target_compile_options(pbsd_kernel_phys_pager PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sg_pager)
add_library(pbsd_kernel_sg_pager)
target_sources(pbsd_kernel_sg_pager PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.sg_pager.cppm)
target_link_libraries(pbsd_kernel_sg_pager PUBLIC pbsd_core pbsd_kernel_vm_pager)
target_compile_options(pbsd_kernel_sg_pager PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vnode_pager)
add_library(pbsd_kernel_vnode_pager)
target_sources(pbsd_kernel_vnode_pager PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vnode_pager.cppm)
target_link_libraries(pbsd_kernel_vnode_pager PUBLIC pbsd_core pbsd_kernel_vm_pager)
target_compile_options(pbsd_kernel_vnode_pager PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_domainset)
add_library(pbsd_kernel_vm_domainset)
target_sources(pbsd_kernel_vm_domainset PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_domainset.cppm)
target_link_libraries(pbsd_kernel_vm_domainset PUBLIC pbsd_core pbsd_kernel_numa)
target_compile_options(pbsd_kernel_vm_domainset PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_memguard)
add_library(pbsd_kernel_memguard)
target_sources(pbsd_kernel_memguard PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.memguard.cppm)
target_link_libraries(pbsd_kernel_memguard PUBLIC pbsd_core pbsd_kernel_vm)
target_compile_options(pbsd_kernel_memguard PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_uma_core)
add_library(pbsd_kernel_uma_core)
target_sources(pbsd_kernel_uma_core PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.uma_core.cppm)
target_link_libraries(pbsd_kernel_uma_core PUBLIC pbsd_core pbsd_kernel_uma)
target_compile_options(pbsd_kernel_uma_core PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_unix)
add_library(pbsd_kernel_vm_unix)
target_sources(pbsd_kernel_vm_unix PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_unix.cppm)
target_link_libraries(pbsd_kernel_vm_unix PUBLIC pbsd_core pbsd_kernel_vm)
target_compile_options(pbsd_kernel_vm_unix PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_vm_swapout)
add_library(pbsd_kernel_vm_swapout)
target_sources(pbsd_kernel_vm_swapout PUBLIC FILE_SET CXX_MODULES FILES
    kernel/vm/pbsd.kernel.vm_swapout.cppm)
target_link_libraries(pbsd_kernel_vm_swapout PUBLIC pbsd_core pbsd_kernel_vm_page)
target_compile_options(pbsd_kernel_vm_swapout PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_ethersubr)
add_library(pbsd_kernel_ethersubr)
target_sources(pbsd_kernel_ethersubr PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.ethersubr.cppm)
target_link_libraries(pbsd_kernel_ethersubr PUBLIC pbsd_core pbsd_kernel_ether)
target_compile_options(pbsd_kernel_ethersubr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_llatbl)
add_library(pbsd_kernel_llatbl)
target_sources(pbsd_kernel_llatbl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.llatbl.cppm)
target_link_libraries(pbsd_kernel_llatbl PUBLIC pbsd_core pbsd_kernel_route)
target_compile_options(pbsd_kernel_llatbl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bpf_filter)
add_library(pbsd_kernel_bpf_filter)
target_sources(pbsd_kernel_bpf_filter PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.bpf_filter.cppm)
target_link_libraries(pbsd_kernel_bpf_filter PUBLIC pbsd_core pbsd_kernel_bpf)
target_compile_options(pbsd_kernel_bpf_filter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bridgestp)
add_library(pbsd_kernel_bridgestp)
target_sources(pbsd_kernel_bridgestp PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.bridgestp.cppm)
target_link_libraries(pbsd_kernel_bridgestp PUBLIC pbsd_core pbsd_kernel_bridge)
target_compile_options(pbsd_kernel_bridgestp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_debugnet)
add_library(pbsd_kernel_debugnet)
target_sources(pbsd_kernel_debugnet PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.debugnet.cppm)
target_link_libraries(pbsd_kernel_debugnet PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_debugnet PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_pfil_helpers)
add_library(pbsd_kernel_pfil_helpers)
target_sources(pbsd_kernel_pfil_helpers PUBLIC FILE_SET CXX_MODULES FILES
    kernel/net/pbsd.kernel.pfil_helpers.cppm)
target_link_libraries(pbsd_kernel_pfil_helpers PUBLIC pbsd_core pbsd_kernel_pfil)
target_compile_options(pbsd_kernel_pfil_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bsm_errno)
add_library(pbsd_kernel_bsm_errno)
target_sources(pbsd_kernel_bsm_errno PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.bsm_errno.cppm)
target_link_libraries(pbsd_kernel_bsm_errno PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bsm_errno PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bsm_fcntl)
add_library(pbsd_kernel_bsm_fcntl)
target_sources(pbsd_kernel_bsm_fcntl PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.bsm_fcntl.cppm)
target_link_libraries(pbsd_kernel_bsm_fcntl PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bsm_fcntl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bsm_domain)
add_library(pbsd_kernel_bsm_domain)
target_sources(pbsd_kernel_bsm_domain PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.bsm_domain.cppm)
target_link_libraries(pbsd_kernel_bsm_domain PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bsm_domain PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_bsm_token)
add_library(pbsd_kernel_bsm_token)
target_sources(pbsd_kernel_bsm_token PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.bsm_token.cppm)
target_link_libraries(pbsd_kernel_bsm_token PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_bsm_token PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_audit_arg)
add_library(pbsd_kernel_audit_arg)
target_sources(pbsd_kernel_audit_arg PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.audit_arg.cppm)
target_link_libraries(pbsd_kernel_audit_arg PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_audit_arg PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_cred)
add_library(pbsd_kernel_mac_cred)
target_sources(pbsd_kernel_mac_cred PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_cred.cppm)
target_link_libraries(pbsd_kernel_mac_cred PUBLIC pbsd_core pbsd_kernel_mac_label)
target_compile_options(pbsd_kernel_mac_cred PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_process)
add_library(pbsd_kernel_mac_process)
target_sources(pbsd_kernel_mac_process PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_process.cppm)
target_link_libraries(pbsd_kernel_mac_process PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_process PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_socket)
add_library(pbsd_kernel_mac_socket)
target_sources(pbsd_kernel_mac_socket PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_socket.cppm)
target_link_libraries(pbsd_kernel_mac_socket PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_socket PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_vfs)
add_library(pbsd_kernel_mac_vfs)
target_sources(pbsd_kernel_mac_vfs PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_vfs.cppm)
target_link_libraries(pbsd_kernel_mac_vfs PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_vfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_framework)
add_library(pbsd_kernel_mac_framework)
target_sources(pbsd_kernel_mac_framework PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_framework.cppm)
target_link_libraries(pbsd_kernel_mac_framework PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_framework PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_inet)
add_library(pbsd_kernel_mac_inet)
target_sources(pbsd_kernel_mac_inet PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_inet.cppm)
target_link_libraries(pbsd_kernel_mac_inet PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_inet PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_priv)
add_library(pbsd_kernel_mac_priv)
target_sources(pbsd_kernel_mac_priv PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_priv.cppm)
target_link_libraries(pbsd_kernel_mac_priv PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_priv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_mac_syscalls)
add_library(pbsd_kernel_mac_syscalls)
target_sources(pbsd_kernel_mac_syscalls PUBLIC FILE_SET CXX_MODULES FILES
    kernel/security/pbsd.kernel.mac_syscalls.cppm)
target_link_libraries(pbsd_kernel_mac_syscalls PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_mac_syscalls PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_p1003_1b)
add_library(pbsd_kernel_p1003_1b)
target_sources(pbsd_kernel_p1003_1b PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.p1003_1b.cppm)
target_link_libraries(pbsd_kernel_p1003_1b PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_p1003_1b PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_posix4_mib)
add_library(pbsd_kernel_posix4_mib)
target_sources(pbsd_kernel_posix4_mib PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.posix4_mib.cppm)
target_link_libraries(pbsd_kernel_posix4_mib PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_posix4_mib PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_stack_protector)
add_library(pbsd_kernel_stack_protector)
target_sources(pbsd_kernel_stack_protector PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.stack_protector.cppm)
target_link_libraries(pbsd_kernel_stack_protector PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_stack_protector PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_imgact_elf)
add_library(pbsd_kernel_imgact_elf)
target_sources(pbsd_kernel_imgact_elf PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.imgact_elf.cppm)
target_link_libraries(pbsd_kernel_imgact_elf PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_imgact_elf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_imgact_elf32)
add_library(pbsd_kernel_imgact_elf32)
target_sources(pbsd_kernel_imgact_elf32 PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.imgact_elf32.cppm)
target_link_libraries(pbsd_kernel_imgact_elf32 PUBLIC pbsd_core pbsd_kernel_imgact_elf)
target_compile_options(pbsd_kernel_imgact_elf32 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_imgact_elf64)
add_library(pbsd_kernel_imgact_elf64)
target_sources(pbsd_kernel_imgact_elf64 PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.imgact_elf64.cppm)
target_link_libraries(pbsd_kernel_imgact_elf64 PUBLIC pbsd_core pbsd_kernel_imgact_elf)
target_compile_options(pbsd_kernel_imgact_elf64 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_coredump_vnode)
add_library(pbsd_kernel_coredump_vnode)
target_sources(pbsd_kernel_coredump_vnode PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.coredump_vnode.cppm)
target_link_libraries(pbsd_kernel_coredump_vnode PUBLIC pbsd_core pbsd_kernel_vnode)
target_compile_options(pbsd_kernel_coredump_vnode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_sched_ule)
add_library(pbsd_kernel_sched_ule)
target_sources(pbsd_kernel_sched_ule PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.sched_ule.cppm)
target_link_libraries(pbsd_kernel_sched_ule PUBLIC pbsd_core pbsd_kernel_sched)
target_compile_options(pbsd_kernel_sched_ule PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_systrace_args)
add_library(pbsd_kernel_systrace_args)
target_sources(pbsd_kernel_systrace_args PUBLIC FILE_SET CXX_MODULES FILES
    kernel/kern/pbsd.kernel.systrace_args.cppm)
target_link_libraries(pbsd_kernel_systrace_args PUBLIC pbsd_core)
target_compile_options(pbsd_kernel_systrace_args PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel_kern_jail)
add_library(pbsd_kernel_kern_jail)
target_sources(pbsd_kernel_kern_jail PUBLIC FILE_SET CXX_MODULES FILES kernel/kern/pbsd.kernel.kern_jail.cppm)
target_link_libraries(pbsd_kernel_kern_jail PUBLIC pbsd_core pbsd_kernel_jail)
target_compile_options(pbsd_kernel_kern_jail PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_kernel)
add_library(pbsd_kernel INTERFACE)
target_link_libraries(pbsd_kernel INTERFACE
    pbsd_kernel_capsicum_core pbsd_kernel_capsicum pbsd_kernel_prot
    pbsd_kernel_process pbsd_kernel_vm pbsd_kernel_alloc pbsd_kernel_resource
    pbsd_kernel_errno pbsd_kernel_signal pbsd_kernel_sysctl
    pbsd_kernel_vm_page pbsd_kernel_vm_map pbsd_kernel_vm_object pbsd_kernel_vm_fault
    pbsd_kernel_uma pbsd_kernel_vm_pager pbsd_kernel_vm_phys
    pbsd_kernel_param pbsd_kernel_security_bsd pbsd_kernel_syscall
    pbsd_kernel_ucred pbsd_kernel_jail pbsd_kernel_timekeeping
    pbsd_kernel_mutex pbsd_kernel_turnstile pbsd_kernel_ktrace
    pbsd_kernel_syscall_args pbsd_kernel_kevent pbsd_kernel_poll
    pbsd_kernel_socket_syscall pbsd_kernel_vnode pbsd_kernel_pipe
    pbsd_kernel_fifo pbsd_kernel_namei pbsd_kernel_bpf pbsd_kernel_audit pbsd_kernel_filedesc pbsd_kernel_selinfo
    pbsd_kernel_select pbsd_kernel_aio pbsd_kernel_mbuf pbsd_kernel_pcb
    pbsd_kernel_inpcb pbsd_kernel_socket_ops
    pbsd_kernel_route pbsd_kernel_ifnet pbsd_kernel_netgraph
    pbsd_kernel_sockbuf pbsd_kernel_domain pbsd_kernel_protosw
    pbsd_kernel_sockio pbsd_kernel_rtentry
    pbsd_kernel_carp pbsd_kernel_gif pbsd_kernel_gre pbsd_kernel_bridge
    pbsd_kernel_vlan pbsd_kernel_lagg pbsd_kernel_if_media pbsd_kernel_if_clone
    pbsd_kernel_pfil pbsd_kernel_random pbsd_kernel_entropy pbsd_kernel_crypto
    pbsd_kernel_geom pbsd_kernel_cam
    pbsd_kernel_callout pbsd_kernel_tty pbsd_kernel_cons pbsd_kernel_uipc
    pbsd_kernel_sysvshm pbsd_kernel_sysvsem pbsd_kernel_sysvmsg pbsd_kernel_posix_shm
    pbsd_kernel_acct pbsd_kernel_racct pbsd_kernel_rctl pbsd_kernel_cpuset
    pbsd_kernel_numa pbsd_kernel_smr pbsd_kernel_epoch pbsd_kernel_timeout
    pbsd_kernel_intr pbsd_kernel_busdma
    pbsd_kernel_sx pbsd_kernel_rwlock pbsd_kernel_condvar pbsd_kernel_taskqueue
    pbsd_kernel_uio pbsd_kernel_lockf pbsd_kernel_sleepq
    pbsd_kernel_rmlock pbsd_kernel_bio pbsd_kernel_disk pbsd_kernel_buf
    pbsd_kernel_eventhandler pbsd_kernel_refcnt pbsd_kernel_kobj
    pbsd_kernel_pci pbsd_kernel_devstat
    pbsd_kernel_witness pbsd_kernel_sysinit pbsd_kernel_uma_helpers
    pbsd_kernel_kqueue pbsd_kernel_cap_rights pbsd_kernel_signal_helpers
    pbsd_kernel_kmod pbsd_kernel_counter pbsd_kernel_mp_ring
    pbsd_kernel_toeplitz pbsd_kernel_bufring
    pbsd_kernel_mac_none pbsd_kernel_mac_jail pbsd_kernel_mac_label
    pbsd_kernel_if_mib pbsd_kernel_if_dead pbsd_kernel_ifq
    pbsd_kernel_vm_radix pbsd_kernel_redzone
    pbsd_kernel_bpf_buffer pbsd_kernel_bpf_jitter
    pbsd_kernel_sched pbsd_kernel_pax_mac pbsd_kernel_pax_aslr pbsd_kernel_shim
    pbsd_kernel_sbuf pbsd_kernel_panic pbsd_kernel_uuid pbsd_kernel_linker
    pbsd_kernel_mfence
    pbsd_kernel_route_temporal pbsd_kernel_nhop_utils
    pbsd_kernel_if_disc pbsd_kernel_if_loop pbsd_kernel_uma_dbg
    pbsd_kernel_audit_trigger pbsd_kernel_bsm_socket_type
    pbsd_kernel_bsm_errno pbsd_kernel_bsm_fcntl pbsd_kernel_bsm_domain
    pbsd_kernel_bsm_token pbsd_kernel_audit_arg
    pbsd_kernel_mac_cred pbsd_kernel_mac_process pbsd_kernel_mac_socket
    pbsd_kernel_mac_vfs pbsd_kernel_mac_framework pbsd_kernel_mac_inet
    pbsd_kernel_mac_priv pbsd_kernel_mac_syscalls
    pbsd_kernel_idle pbsd_kernel_lockstat pbsd_kernel_power
    pbsd_kernel_context pbsd_kernel_sys_getrandom
    pbsd_kernel_subr_unit pbsd_kernel_subr_hash
    pbsd_kernel_subr_msgbuf pbsd_kernel_subr_prf
    pbsd_kernel_hash pbsd_kernel_fattime pbsd_kernel_firmware
    pbsd_kernel_pctrie pbsd_kernel_sglist pbsd_kernel_sfbuf pbsd_kernel_rman
    pbsd_kernel_blist pbsd_kernel_hints pbsd_kernel_kern_log
    pbsd_kernel_vfs_mount pbsd_kernel_vfs_subr pbsd_kernel_vfs_lookup
    pbsd_kernel_vfs_default pbsd_kernel_vfs_syscalls pbsd_kernel_vfs_vnops
    pbsd_kernel_vfs_hash pbsd_kernel_vfs_cache
    pbsd_kernel_synch pbsd_kernel_kern_mib pbsd_kernel_kern_malloc
    pbsd_kernel_kern_proc pbsd_kernel_kern_sig pbsd_kernel_kern_fork
    pbsd_kernel_kern_exit
    pbsd_kernel_kern_exec pbsd_kernel_kern_descrip pbsd_kernel_kern_syscalls
    pbsd_kernel_kern_thread pbsd_kernel_kern_switch pbsd_kernel_kern_time
    pbsd_kernel_kern_event pbsd_kernel_kern_umtx pbsd_kernel_kern_module
    pbsd_kernel_kern_cpu pbsd_kernel_kern_conf pbsd_kernel_kern_environment
    pbsd_kernel_kern_lock pbsd_kernel_kern_sendfile pbsd_kernel_kern_sharedpage
    pbsd_kernel_kern_thr pbsd_kernel_kern_ntptime pbsd_kernel_kern_physio
    pbsd_kernel_kern_procctl pbsd_kernel_kern_membarrier
    pbsd_kernel_prf pbsd_kernel_unit
    pbsd_kernel_rangeset pbsd_kernel_filter pbsd_kernel_stack pbsd_kernel_pidctrl
    pbsd_kernel_prng pbsd_kernel_subr_lock pbsd_kernel_scanf pbsd_kernel_gtaskqueue
    pbsd_kernel_physmem pbsd_kernel_vmem pbsd_kernel_stats pbsd_kernel_clock
    pbsd_kernel_boot pbsd_kernel_subr_rangeset pbsd_kernel_subr_filter
    pbsd_kernel_subr_stack pbsd_kernel_subr_pidctrl pbsd_kernel_subr_prng
    pbsd_kernel_mchain pbsd_kernel_subr_mchain
    pbsd_kernel_rss pbsd_kernel_flowtable
    pbsd_kernel_memdesc pbsd_kernel_subr_memdesc pbsd_kernel_vm_map_helpers
    pbsd_kernel_ether pbsd_kernel_altq pbsd_kernel_subr_counter
    pbsd_kernel_subr_sbuf
    pbsd_kernel_subr_panic
    pbsd_kernel_subr_uuid
    pbsd_kernel_subr_linker
    pbsd_kernel_subr_vmem
    pbsd_kernel_subr_bus
    pbsd_kernel_device
    pbsd_kernel_vm_meter
    pbsd_kernel_vm_kern
    pbsd_kernel_vm_reserv
    pbsd_kernel_subr_ether
    pbsd_kernel_mbuf_helpers
    pbsd_kernel_ifnet_helpers
    pbsd_kernel_rtentry_helpers
    pbsd_kernel_subr_uio pbsd_kernel_subr_turnstile pbsd_kernel_subr_sleepqueue
    pbsd_kernel_subr_witness pbsd_kernel_subr_bufring pbsd_kernel_subr_eventhandler
    pbsd_kernel_vm_object_helpers pbsd_kernel_vm_page_helpers pbsd_kernel_vm_fault_helpers
    pbsd_kernel_sync_helpers pbsd_kernel_in_cksum pbsd_kernel_link_addr
    pbsd_kernel_pktsched
    pbsd_kernel_subr_intr pbsd_kernel_subr_epoch pbsd_kernel_subr_smr
    pbsd_kernel_subr_taskqueue pbsd_kernel_subr_devstat pbsd_kernel_subr_disk
    pbsd_kernel_subr_power pbsd_kernel_subr_pcpu pbsd_kernel_subr_stats
    pbsd_kernel_subr_trap pbsd_kernel_subr_module pbsd_kernel_subr_log
    pbsd_kernel_subr_autoconf pbsd_kernel_subr_bus_dma pbsd_kernel_subr_capability
    pbsd_kernel_subr_clock pbsd_kernel_subr_early pbsd_kernel_subr_kdb
    pbsd_kernel_subr_param pbsd_kernel_subr_prof pbsd_kernel_subr_rtc
    pbsd_kernel_subr_smp pbsd_kernel_subr_terminal pbsd_kernel_subr_acl_nfs4
    pbsd_kernel_subr_acl_posix1e pbsd_kernel_subr_devmap pbsd_kernel_subr_syscall
    pbsd_kernel_subr_boot pbsd_kernel_subr_busdma_bounce pbsd_kernel_subr_busdma_bufalloc
    pbsd_kernel_kern_alq
    pbsd_kernel_kern_boottrace
    pbsd_kernel_kern_clocksource
    pbsd_kernel_kern_ctf
    pbsd_kernel_kern_devctl
    pbsd_kernel_kern_dump
    pbsd_kernel_kern_fail
    pbsd_kernel_kern_ffclock
    pbsd_kernel_kern_hhook
    pbsd_kernel_kern_jaildesc
    pbsd_kernel_kern_jailmeta
    pbsd_kernel_kern_kcov
    pbsd_kernel_kern_khelp
    pbsd_kernel_kern_kthread
    pbsd_kernel_kern_ktr
    pbsd_kernel_kern_loginclass
    pbsd_kernel_kern_mtxpool
    pbsd_kernel_kern_osd
    pbsd_kernel_kern_pmc
    pbsd_kernel_kern_priv
    pbsd_kernel_kern_rangelock
    pbsd_kernel_kern_sema
    pbsd_kernel_kern_shutdown
    pbsd_kernel_kern_tc
    pbsd_kernel_kern_tslog
    pbsd_kernel_kern_ucoredump
    pbsd_kernel_kern_vnodedumper
    pbsd_kernel_subr_atomic64
    pbsd_kernel_subr_clockcalib
    pbsd_kernel_subr_compressor
    pbsd_kernel_subr_coverage
    pbsd_kernel_subr_dummy_vdso_tc
    pbsd_kernel_subr_efi_map
    pbsd_kernel_kern_dtrace
    pbsd_kernel_kern_et
    pbsd_kernel_kern_sdt
    pbsd_kernel_kern_ubsan
    pbsd_kernel_subr_asan
    pbsd_kernel_subr_csan
    pbsd_kernel_subr_msan
    pbsd_kernel_bpf_filter
    pbsd_kernel_bridgestp
    pbsd_kernel_debugnet
    pbsd_kernel_ethersubr
    pbsd_kernel_llatbl
    pbsd_kernel_pfil_helpers
    pbsd_kernel_device_pager
    pbsd_kernel_memguard
    pbsd_kernel_phys_pager
    pbsd_kernel_sg_pager
    pbsd_kernel_swap_pager
    pbsd_kernel_uma_core
    pbsd_kernel_vm_domainset
    pbsd_kernel_vm_glue
    pbsd_kernel_vm_init
    pbsd_kernel_vm_mmap
    pbsd_kernel_vm_pageout
    pbsd_kernel_vm_swapout
    pbsd_kernel_vm_unix
    pbsd_kernel_vnode_pager
    pbsd_kernel_p1003_1b pbsd_kernel_posix4_mib pbsd_kernel_stack_protector
    pbsd_kernel_imgact_elf pbsd_kernel_imgact_elf32 pbsd_kernel_imgact_elf64
    pbsd_kernel_coredump_vnode pbsd_kernel_sched_ule pbsd_kernel_systrace_args
    pbsd_kernel_madvise pbsd_kernel_kern_jail)
target_compile_options(pbsd_kernel INTERFACE ${PBSD_FS_CXX})
endif()

# --- Wave 6: net / fs / geom / zfs -------------------------------------------
add_library(pbsd_net_sockopt)
target_sources(pbsd_net_sockopt PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sockopt.cppm)
target_link_libraries(pbsd_net_sockopt PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_net_sockopt PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_netinet)
target_sources(pbsd_net_netinet PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.netinet.cppm)
target_link_libraries(pbsd_net_netinet PUBLIC pbsd_core)
target_compile_options(pbsd_net_netinet PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_tcp)
target_sources(pbsd_net_tcp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp.cppm)
target_link_libraries(pbsd_net_tcp PUBLIC pbsd_core pbsd_rights pbsd_net_sockopt)
target_compile_options(pbsd_net_tcp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_udp)
target_sources(pbsd_net_udp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.udp.cppm)
target_link_libraries(pbsd_net_udp PUBLIC pbsd_core pbsd_rights pbsd_net_sockopt)
target_compile_options(pbsd_net_udp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_ip)
target_sources(pbsd_net_ip PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip.cppm)
target_link_libraries(pbsd_net_ip PUBLIC pbsd_core)
target_compile_options(pbsd_net_ip PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_icmp)
target_sources(pbsd_net_icmp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.icmp.cppm)
target_link_libraries(pbsd_net_icmp PUBLIC pbsd_core)
target_compile_options(pbsd_net_icmp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_ifnet)
target_sources(pbsd_net_ifnet PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ifnet.cppm)
target_link_libraries(pbsd_net_ifnet PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_net_ifnet PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_route)
target_sources(pbsd_net_route PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route.cppm)
target_link_libraries(pbsd_net_route PUBLIC pbsd_core)
target_compile_options(pbsd_net_route PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_in_pcb)
target_sources(pbsd_net_in_pcb PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_pcb.cppm)
target_link_libraries(pbsd_net_in_pcb PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_pcb PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_arp)
target_sources(pbsd_net_arp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.arp.cppm)
target_link_libraries(pbsd_net_arp PUBLIC pbsd_core)
target_compile_options(pbsd_net_arp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_ether)
target_sources(pbsd_net_ether PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ether.cppm)
target_link_libraries(pbsd_net_ether PUBLIC pbsd_core)
target_compile_options(pbsd_net_ether PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_vlan)
target_sources(pbsd_net_vlan PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.vlan.cppm)
target_link_libraries(pbsd_net_vlan PUBLIC pbsd_core)
target_compile_options(pbsd_net_vlan PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_mbuf)
target_sources(pbsd_net_mbuf PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.mbuf.cppm)
target_link_libraries(pbsd_net_mbuf PUBLIC pbsd_core)
target_compile_options(pbsd_net_mbuf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_bpf)
target_sources(pbsd_net_bpf PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.bpf.cppm)
target_link_libraries(pbsd_net_bpf PUBLIC pbsd_core)
target_compile_options(pbsd_net_bpf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_igmp)
target_sources(pbsd_net_igmp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.igmp.cppm)
target_link_libraries(pbsd_net_igmp PUBLIC pbsd_core)
target_compile_options(pbsd_net_igmp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_bridge)
target_sources(pbsd_net_bridge PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.bridge.cppm)
target_link_libraries(pbsd_net_bridge PUBLIC pbsd_core)
target_compile_options(pbsd_net_bridge PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_pf)
target_sources(pbsd_net_pf PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pf.cppm)
target_link_libraries(pbsd_net_pf PUBLIC pbsd_core)
target_compile_options(pbsd_net_pf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_altq)
target_sources(pbsd_net_altq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.altq.cppm)
target_link_libraries(pbsd_net_altq PUBLIC pbsd_core)
target_compile_options(pbsd_net_altq PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_in6)
target_sources(pbsd_net_in6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in6.cppm)
target_link_libraries(pbsd_net_in6 PUBLIC pbsd_core)
target_compile_options(pbsd_net_in6 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_gre)
target_sources(pbsd_net_gre PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.gre.cppm)
target_link_libraries(pbsd_net_gre PUBLIC pbsd_core)
target_compile_options(pbsd_net_gre PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_epair)
target_sources(pbsd_net_epair PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.epair.cppm)
target_link_libraries(pbsd_net_epair PUBLIC pbsd_core)
target_compile_options(pbsd_net_epair PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_tun)
target_sources(pbsd_net_tun PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tun.cppm)
target_link_libraries(pbsd_net_tun PUBLIC pbsd_core)
target_compile_options(pbsd_net_tun PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_tap)
target_sources(pbsd_net_tap PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tap.cppm)
target_link_libraries(pbsd_net_tap PUBLIC pbsd_core)
target_compile_options(pbsd_net_tap PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_ndp)
target_sources(pbsd_net_ndp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ndp.cppm)
target_link_libraries(pbsd_net_ndp PUBLIC pbsd_core)
target_compile_options(pbsd_net_ndp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_lagg)
target_sources(pbsd_net_lagg PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.lagg.cppm)
target_link_libraries(pbsd_net_lagg PUBLIC pbsd_core)
target_compile_options(pbsd_net_lagg PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_gif)
target_sources(pbsd_net_gif PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.gif.cppm)
target_link_libraries(pbsd_net_gif PUBLIC pbsd_core)
target_compile_options(pbsd_net_gif PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_loop)
target_sources(pbsd_net_loop PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.loop.cppm)
target_link_libraries(pbsd_net_loop PUBLIC pbsd_core)
target_compile_options(pbsd_net_loop PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_ipfw)
target_sources(pbsd_net_ipfw PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ipfw.cppm)
target_link_libraries(pbsd_net_ipfw PUBLIC pbsd_core)
target_compile_options(pbsd_net_ipfw PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_carp)
target_sources(pbsd_net_carp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.carp.cppm)
target_link_libraries(pbsd_net_carp PUBLIC pbsd_core)
target_compile_options(pbsd_net_carp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_ip6)
target_sources(pbsd_net_ip6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip6.cppm)
target_link_libraries(pbsd_net_ip6 PUBLIC pbsd_core)
target_compile_options(pbsd_net_ip6 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_udp6)
target_sources(pbsd_net_udp6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.udp6.cppm)
target_link_libraries(pbsd_net_udp6 PUBLIC pbsd_core pbsd_rights pbsd_net_sockopt)
target_compile_options(pbsd_net_udp6 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_tcp6)
target_sources(pbsd_net_tcp6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp6.cppm)
target_link_libraries(pbsd_net_tcp6 PUBLIC pbsd_core pbsd_rights pbsd_net_sockopt)
target_compile_options(pbsd_net_tcp6 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_sctp)
target_sources(pbsd_net_sctp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp.cppm)
target_link_libraries(pbsd_net_sctp PUBLIC pbsd_core)
target_compile_options(pbsd_net_sctp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_netmap)
target_sources(pbsd_net_netmap PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.netmap.cppm)
target_link_libraries(pbsd_net_netmap PUBLIC pbsd_core)
target_compile_options(pbsd_net_netmap PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_netgraph)
target_sources(pbsd_net_netgraph PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.netgraph.cppm)
target_link_libraries(pbsd_net_netgraph PUBLIC pbsd_core)
target_compile_options(pbsd_net_netgraph PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_vxlan)
target_sources(pbsd_net_vxlan PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.vxlan.cppm)
target_link_libraries(pbsd_net_vxlan PUBLIC pbsd_core)
target_compile_options(pbsd_net_vxlan PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_ipsec)
target_sources(pbsd_net_ipsec PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ipsec.cppm)
target_link_libraries(pbsd_net_ipsec PUBLIC pbsd_core)
target_compile_options(pbsd_net_ipsec PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_stf)
target_sources(pbsd_net_stf PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.stf.cppm)
target_link_libraries(pbsd_net_stf PUBLIC pbsd_core)
target_compile_options(pbsd_net_stf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_wg)
target_sources(pbsd_net_wg PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.wg.cppm)
target_link_libraries(pbsd_net_wg PUBLIC pbsd_core)
target_compile_options(pbsd_net_wg PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_pfsync)
target_sources(pbsd_net_pfsync PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pfsync.cppm)
target_link_libraries(pbsd_net_pfsync PUBLIC pbsd_core)
target_compile_options(pbsd_net_pfsync PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_dlt)
target_sources(pbsd_net_dlt PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.dlt.cppm)
target_link_libraries(pbsd_net_dlt PUBLIC pbsd_core)
target_compile_options(pbsd_net_dlt PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_media)
target_sources(pbsd_net_media PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.media.cppm)
target_link_libraries(pbsd_net_media PUBLIC pbsd_core)
target_compile_options(pbsd_net_media PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_pfil)
target_sources(pbsd_net_pfil PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pfil.cppm)
target_link_libraries(pbsd_net_pfil PUBLIC pbsd_core)
target_compile_options(pbsd_net_pfil PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_lacp)
target_sources(pbsd_net_lacp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.lacp.cppm)
target_link_libraries(pbsd_net_lacp PUBLIC pbsd_core)
target_compile_options(pbsd_net_lacp PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_net_clone)
target_sources(pbsd_net_clone PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.clone.cppm)
target_link_libraries(pbsd_net_clone PUBLIC pbsd_core)
target_compile_options(pbsd_net_clone PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_net_divert)
add_library(pbsd_net_divert)
target_sources(pbsd_net_divert PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.divert.cppm)
target_link_libraries(pbsd_net_divert PUBLIC pbsd_core)
target_compile_options(pbsd_net_divert PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_netgraph_node)
add_library(pbsd_net_netgraph_node)
target_sources(pbsd_net_netgraph_node PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.netgraph.node.cppm)
target_link_libraries(pbsd_net_netgraph_node PUBLIC pbsd_core pbsd_net_netgraph)
target_compile_options(pbsd_net_netgraph_node PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp_assoc)
add_library(pbsd_net_sctp_assoc)
target_sources(pbsd_net_sctp_assoc PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp_assoc.cppm)
target_link_libraries(pbsd_net_sctp_assoc PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp_assoc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_bridge_fdb)
add_library(pbsd_net_bridge_fdb)
target_sources(pbsd_net_bridge_fdb PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.bridge_fdb.cppm)
target_link_libraries(pbsd_net_bridge_fdb PUBLIC pbsd_core pbsd_net_bridge)
target_compile_options(pbsd_net_bridge_fdb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ipfw_rule)
add_library(pbsd_net_ipfw_rule)
target_sources(pbsd_net_ipfw_rule PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ipfw_rule.cppm)
target_link_libraries(pbsd_net_ipfw_rule PUBLIC pbsd_core pbsd_net_ipfw)
target_compile_options(pbsd_net_ipfw_rule PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_enc)
add_library(pbsd_net_enc)
target_sources(pbsd_net_enc PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.enc.cppm)
target_link_libraries(pbsd_net_enc PUBLIC pbsd_core)
target_compile_options(pbsd_net_enc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_mroute)
add_library(pbsd_net_mroute)
target_sources(pbsd_net_mroute PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.mroute.cppm)
target_link_libraries(pbsd_net_mroute PUBLIC pbsd_core)
target_compile_options(pbsd_net_mroute PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ipdivert)
add_library(pbsd_net_ipdivert)
target_sources(pbsd_net_ipdivert PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ipdivert.cppm)
target_link_libraries(pbsd_net_ipdivert PUBLIC pbsd_core)
target_compile_options(pbsd_net_ipdivert PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_icmp6)
add_library(pbsd_net_icmp6)
target_sources(pbsd_net_icmp6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.icmp6.cppm)
target_link_libraries(pbsd_net_icmp6 PUBLIC pbsd_core)
target_compile_options(pbsd_net_icmp6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_mld6)
add_library(pbsd_net_mld6)
target_sources(pbsd_net_mld6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.mld6.cppm)
target_link_libraries(pbsd_net_mld6 PUBLIC pbsd_core)
target_compile_options(pbsd_net_mld6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_raw6)
add_library(pbsd_net_raw6)
target_sources(pbsd_net_raw6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.raw6.cppm)
target_link_libraries(pbsd_net_raw6 PUBLIC pbsd_core)
target_compile_options(pbsd_net_raw6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_dummynet)
add_library(pbsd_net_dummynet)
target_sources(pbsd_net_dummynet PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.dummynet.cppm)
target_link_libraries(pbsd_net_dummynet PUBLIC pbsd_core)
target_compile_options(pbsd_net_dummynet PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip6_mroute)
add_library(pbsd_net_ip6_mroute)
target_sources(pbsd_net_ip6_mroute PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip6_mroute.cppm)
target_link_libraries(pbsd_net_ip6_mroute PUBLIC pbsd_core)
target_compile_options(pbsd_net_ip6_mroute PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_pim6)
add_library(pbsd_net_pim6)
target_sources(pbsd_net_pim6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pim6.cppm)
target_link_libraries(pbsd_net_pim6 PUBLIC pbsd_core)
target_compile_options(pbsd_net_pim6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_if_ethersubr)
add_library(pbsd_net_if_ethersubr)
target_sources(pbsd_net_if_ethersubr PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.if_ethersubr.cppm)
target_link_libraries(pbsd_net_if_ethersubr PUBLIC pbsd_core)
target_compile_options(pbsd_net_if_ethersubr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_netisr)
add_library(pbsd_net_netisr)
target_sources(pbsd_net_netisr PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.netisr.cppm)
target_link_libraries(pbsd_net_netisr PUBLIC pbsd_core)
target_compile_options(pbsd_net_netisr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_rmx)
add_library(pbsd_net_in_rmx)
target_sources(pbsd_net_in_rmx PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_rmx.cppm)
target_link_libraries(pbsd_net_in_rmx PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_rmx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_ecn)
add_library(pbsd_net_ip_ecn)
target_sources(pbsd_net_ip_ecn PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_ecn.cppm)
target_link_libraries(pbsd_net_ip_ecn PUBLIC pbsd_core)
target_compile_options(pbsd_net_ip_ecn PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_bridgestp)
add_library(pbsd_net_bridgestp)
target_sources(pbsd_net_bridgestp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.bridgestp.cppm)
target_link_libraries(pbsd_net_bridgestp PUBLIC pbsd_core pbsd_net_bridge)
target_compile_options(pbsd_net_bridgestp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_timer)
add_library(pbsd_net_tcp_timer)
target_sources(pbsd_net_tcp_timer PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_timer.cppm)
target_link_libraries(pbsd_net_tcp_timer PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_timer PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_timer_wheel)
add_library(pbsd_net_tcp_timer_wheel)
target_sources(pbsd_net_tcp_timer_wheel PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_timer_wheel.cppm)
target_link_libraries(pbsd_net_tcp_timer_wheel PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_timer_wheel PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_ecn)
add_library(pbsd_net_tcp_ecn)
target_sources(pbsd_net_tcp_ecn PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_ecn.cppm)
target_link_libraries(pbsd_net_tcp_ecn PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_ecn PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_reass)
add_library(pbsd_net_tcp_reass)
target_sources(pbsd_net_tcp_reass PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_reass.cppm)
target_link_libraries(pbsd_net_tcp_reass PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_reass PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_sack)
add_library(pbsd_net_tcp_sack)
target_sources(pbsd_net_tcp_sack PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_sack.cppm)
target_link_libraries(pbsd_net_tcp_sack PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_sack PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_syncache)
add_library(pbsd_net_tcp_syncache)
target_sources(pbsd_net_tcp_syncache PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_syncache.cppm)
target_link_libraries(pbsd_net_tcp_syncache PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_syncache PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_timewait)
add_library(pbsd_net_tcp_timewait)
target_sources(pbsd_net_tcp_timewait PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_timewait.cppm)
target_link_libraries(pbsd_net_tcp_timewait PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_timewait PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_reass)
add_library(pbsd_net_ip_reass)
target_sources(pbsd_net_ip_reass PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_reass.cppm)
target_link_libraries(pbsd_net_ip_reass PUBLIC pbsd_core pbsd_net_ip)
target_compile_options(pbsd_net_ip_reass PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_options)
add_library(pbsd_net_ip_options)
target_sources(pbsd_net_ip_options PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_options.cppm)
target_link_libraries(pbsd_net_ip_options PUBLIC pbsd_core pbsd_net_ip)
target_compile_options(pbsd_net_ip_options PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_cksum)
add_library(pbsd_net_in_cksum)
target_sources(pbsd_net_in_cksum PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_cksum.cppm)
target_link_libraries(pbsd_net_in_cksum PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_cksum PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_raw_ip)
add_library(pbsd_net_raw_ip)
target_sources(pbsd_net_raw_ip PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.raw_ip.cppm)
target_link_libraries(pbsd_net_raw_ip PUBLIC pbsd_core pbsd_rights pbsd_net_sockopt)
target_compile_options(pbsd_net_raw_ip PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_mcast)
add_library(pbsd_net_in_mcast)
target_sources(pbsd_net_in_mcast PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_mcast.cppm)
target_link_libraries(pbsd_net_in_mcast PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_mcast PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_encap)
add_library(pbsd_net_ip_encap)
target_sources(pbsd_net_ip_encap PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_encap.cppm)
target_link_libraries(pbsd_net_ip_encap PUBLIC pbsd_core pbsd_net_ip)
target_compile_options(pbsd_net_ip_encap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_if_llatbl)
add_library(pbsd_net_if_llatbl)
target_sources(pbsd_net_if_llatbl PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.if_llatbl.cppm)
target_link_libraries(pbsd_net_if_llatbl PUBLIC pbsd_core pbsd_net_ifnet)
target_compile_options(pbsd_net_if_llatbl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_vnet)
add_library(pbsd_net_vnet)
target_sources(pbsd_net_vnet PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.vnet.cppm)
target_link_libraries(pbsd_net_vnet PUBLIC pbsd_core)
target_compile_options(pbsd_net_vnet PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_radix)
add_library(pbsd_net_radix)
target_sources(pbsd_net_radix PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.radix.cppm)
target_link_libraries(pbsd_net_radix PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_radix PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ifq)
add_library(pbsd_net_ifq)
target_sources(pbsd_net_ifq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ifq.cppm)
target_link_libraries(pbsd_net_ifq PUBLIC pbsd_core pbsd_net_ifnet)
target_compile_options(pbsd_net_ifq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_fastopen)
add_library(pbsd_net_tcp_fastopen)
target_sources(pbsd_net_tcp_fastopen PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_fastopen.cppm)
target_link_libraries(pbsd_net_tcp_fastopen PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_fastopen PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_hpts)
add_library(pbsd_net_tcp_hpts)
target_sources(pbsd_net_tcp_hpts PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_hpts.cppm)
target_link_libraries(pbsd_net_tcp_hpts PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_hpts PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_rss)
add_library(pbsd_net_rss)
target_sources(pbsd_net_rss PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.rss.cppm)
target_link_libraries(pbsd_net_rss PUBLIC pbsd_core)
target_compile_options(pbsd_net_rss PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_flowtable)
add_library(pbsd_net_flowtable)
target_sources(pbsd_net_flowtable PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.flowtable.cppm)
target_link_libraries(pbsd_net_flowtable PUBLIC pbsd_core)
target_compile_options(pbsd_net_flowtable PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_input)
add_library(pbsd_net_tcp_input)
target_sources(pbsd_net_tcp_input PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_input.cppm)
target_link_libraries(pbsd_net_tcp_input PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_input PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_output)
add_library(pbsd_net_tcp_output)
target_sources(pbsd_net_tcp_output PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_output.cppm)
target_link_libraries(pbsd_net_tcp_output PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_output PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_pcbgroup)
add_library(pbsd_net_in_pcbgroup)
target_sources(pbsd_net_in_pcbgroup PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_pcbgroup.cppm)
target_link_libraries(pbsd_net_in_pcbgroup PUBLIC pbsd_core pbsd_net_in_pcb)
target_compile_options(pbsd_net_in_pcbgroup PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_proto)
add_library(pbsd_net_in_proto)
target_sources(pbsd_net_in_proto PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_proto.cppm)
target_link_libraries(pbsd_net_in_proto PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_proto PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_prot)
add_library(pbsd_net_in_prot)
target_sources(pbsd_net_in_prot PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_prot.cppm)
target_link_libraries(pbsd_net_in_prot PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_prot PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_fw_nat)
add_library(pbsd_net_ip_fw_nat)
target_sources(pbsd_net_ip_fw_nat PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_fw_nat.cppm)
target_link_libraries(pbsd_net_ip_fw_nat PUBLIC pbsd_core pbsd_net_ipfw)
target_compile_options(pbsd_net_ip_fw_nat PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ether_ifattach)
add_library(pbsd_net_ether_ifattach)
target_sources(pbsd_net_ether_ifattach PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ether_ifattach.cppm)
target_link_libraries(pbsd_net_ether_ifattach PUBLIC pbsd_core pbsd_net_ether)
target_compile_options(pbsd_net_ether_ifattach PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_udp_usrreq)
add_library(pbsd_net_udp_usrreq)
target_sources(pbsd_net_udp_usrreq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.udp_usrreq.cppm)
target_link_libraries(pbsd_net_udp_usrreq PUBLIC pbsd_core pbsd_net_udp)
target_compile_options(pbsd_net_udp_usrreq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_usrreq)
add_library(pbsd_net_tcp_usrreq)
target_sources(pbsd_net_tcp_usrreq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_usrreq.cppm)
target_link_libraries(pbsd_net_tcp_usrreq PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_usrreq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_input)
add_library(pbsd_net_ip_input)
target_sources(pbsd_net_ip_input PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_input.cppm)
target_link_libraries(pbsd_net_ip_input PUBLIC pbsd_core pbsd_net_ip)
target_compile_options(pbsd_net_ip_input PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_output)
add_library(pbsd_net_ip_output)
target_sources(pbsd_net_ip_output PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_output.cppm)
target_link_libraries(pbsd_net_ip_output PUBLIC pbsd_core pbsd_net_ip)
target_compile_options(pbsd_net_ip_output PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_udp6_usrreq)
add_library(pbsd_net_udp6_usrreq)
target_sources(pbsd_net_udp6_usrreq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.udp6_usrreq.cppm)
target_link_libraries(pbsd_net_udp6_usrreq PUBLIC pbsd_core pbsd_net_udp6)
target_compile_options(pbsd_net_udp6_usrreq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip6_input)
add_library(pbsd_net_ip6_input)
target_sources(pbsd_net_ip6_input PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip6_input.cppm)
target_link_libraries(pbsd_net_ip6_input PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_ip6_input PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip6_output)
add_library(pbsd_net_ip6_output)
target_sources(pbsd_net_ip6_output PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip6_output.cppm)
target_link_libraries(pbsd_net_ip6_output PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_ip6_output PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_subr)
add_library(pbsd_net_tcp_subr)
target_sources(pbsd_net_tcp_subr PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_subr.cppm)
target_link_libraries(pbsd_net_tcp_subr PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_udp_input)
add_library(pbsd_net_udp_input)
target_sources(pbsd_net_udp_input PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.udp_input.cppm)
target_link_libraries(pbsd_net_udp_input PUBLIC pbsd_core pbsd_net_udp)
target_compile_options(pbsd_net_udp_input PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_pcblist)
add_library(pbsd_net_in_pcblist)
target_sources(pbsd_net_in_pcblist PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_pcblist.cppm)
target_link_libraries(pbsd_net_in_pcblist PUBLIC pbsd_core pbsd_net_in_pcb)
target_compile_options(pbsd_net_in_pcblist PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_raw_usrreq)
add_library(pbsd_net_raw_usrreq)
target_sources(pbsd_net_raw_usrreq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.raw_usrreq.cppm)
target_link_libraries(pbsd_net_raw_usrreq PUBLIC pbsd_core pbsd_net_raw_ip)
target_compile_options(pbsd_net_raw_usrreq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_log)
add_library(pbsd_net_tcp_log)
target_sources(pbsd_net_tcp_log PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_log.cppm)
target_link_libraries(pbsd_net_tcp_log PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_log PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc)
add_library(pbsd_net_cc)
target_sources(pbsd_net_cc PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.cc.cppm)
target_link_libraries(pbsd_net_cc PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_cubic)
add_library(pbsd_net_cc_cubic)
target_sources(pbsd_net_cc_cubic PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.cc_cubic.cppm)
target_link_libraries(pbsd_net_cc_cubic PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_cubic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_pf_ioctl)
add_library(pbsd_net_pf_ioctl)
target_sources(pbsd_net_pf_ioctl PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pf_ioctl.cppm)
target_link_libraries(pbsd_net_pf_ioctl PUBLIC pbsd_core pbsd_net_pf)
target_compile_options(pbsd_net_pf_ioctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_rip_input)
add_library(pbsd_net_rip_input)
target_sources(pbsd_net_rip_input PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.rip_input.cppm)
target_link_libraries(pbsd_net_rip_input PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_rip_input PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_rip_output)
add_library(pbsd_net_rip_output)
target_sources(pbsd_net_rip_output PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.rip_output.cppm)
target_link_libraries(pbsd_net_rip_output PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_rip_output PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_fastforward)
add_library(pbsd_net_ip_fastforward)
target_sources(pbsd_net_ip_fastforward PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_fastforward.cppm)
target_link_libraries(pbsd_net_ip_fastforward PUBLIC pbsd_core pbsd_net_ip)
target_compile_options(pbsd_net_ip_fastforward PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_rfc1323)
add_library(pbsd_net_tcp_rfc1323)
target_sources(pbsd_net_tcp_rfc1323 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_rfc1323.cppm)
target_link_libraries(pbsd_net_tcp_rfc1323 PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_rfc1323 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_frag6)
add_library(pbsd_net_frag6)
target_sources(pbsd_net_frag6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.frag6.cppm)
target_link_libraries(pbsd_net_frag6 PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_frag6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip6_forward)
add_library(pbsd_net_ip6_forward)
target_sources(pbsd_net_ip6_forward PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip6_forward.cppm)
target_link_libraries(pbsd_net_ip6_forward PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_ip6_forward PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_nd6_nbr)
add_library(pbsd_net_nd6_nbr)
target_sources(pbsd_net_nd6_nbr PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.nd6_nbr.cppm)
target_link_libraries(pbsd_net_nd6_nbr PUBLIC pbsd_core pbsd_net_ndp)
target_compile_options(pbsd_net_nd6_nbr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_nd6_rtr)
add_library(pbsd_net_nd6_rtr)
target_sources(pbsd_net_nd6_rtr PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.nd6_rtr.cppm)
target_link_libraries(pbsd_net_nd6_rtr PUBLIC pbsd_core pbsd_net_ndp)
target_compile_options(pbsd_net_nd6_rtr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in6_cksum)
add_library(pbsd_net_in6_cksum)
target_sources(pbsd_net_in6_cksum PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in6_cksum.cppm)
target_link_libraries(pbsd_net_in6_cksum PUBLIC pbsd_core)
target_compile_options(pbsd_net_in6_cksum PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in6_proto)
add_library(pbsd_net_in6_proto)
target_sources(pbsd_net_in6_proto PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in6_proto.cppm)
target_link_libraries(pbsd_net_in6_proto PUBLIC pbsd_core)
target_compile_options(pbsd_net_in6_proto PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in6_pcb)
add_library(pbsd_net_in6_pcb)
target_sources(pbsd_net_in6_pcb PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in6_pcb.cppm)
target_link_libraries(pbsd_net_in6_pcb PUBLIC pbsd_core pbsd_net_in_pcb)
target_compile_options(pbsd_net_in6_pcb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_scope6)
add_library(pbsd_net_scope6)
target_sources(pbsd_net_scope6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.scope6.cppm)
target_link_libraries(pbsd_net_scope6 PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_scope6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp6_usrreq)
add_library(pbsd_net_sctp6_usrreq)
target_sources(pbsd_net_sctp6_usrreq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp6_usrreq.cppm)
target_link_libraries(pbsd_net_sctp6_usrreq PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp6_usrreq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip6_id)
add_library(pbsd_net_ip6_id)
target_sources(pbsd_net_ip6_id PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip6_id.cppm)
target_link_libraries(pbsd_net_ip6_id PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_ip6_id PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip6_fastfwd)
add_library(pbsd_net_ip6_fastfwd)
target_sources(pbsd_net_ip6_fastfwd PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip6_fastfwd.cppm)
target_link_libraries(pbsd_net_ip6_fastfwd PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_ip6_fastfwd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_dest6)
add_library(pbsd_net_dest6)
target_sources(pbsd_net_dest6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.dest6.cppm)
target_link_libraries(pbsd_net_dest6 PUBLIC pbsd_core pbsd_net_ip6)
target_compile_options(pbsd_net_dest6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route6)
add_library(pbsd_net_route6)
target_sources(pbsd_net_route6 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route6.cppm)
target_link_libraries(pbsd_net_route6 PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route6 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_hostcache)
add_library(pbsd_net_tcp_hostcache)
target_sources(pbsd_net_tcp_hostcache PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_hostcache.cppm)
target_link_libraries(pbsd_net_tcp_hostcache PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_hostcache PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_lro)
add_library(pbsd_net_tcp_lro)
target_sources(pbsd_net_tcp_lro PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_lro.cppm)
target_link_libraries(pbsd_net_tcp_lro PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_lro PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_lro_hpts)
add_library(pbsd_net_tcp_lro_hpts)
target_sources(pbsd_net_tcp_lro_hpts PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_lro_hpts.cppm)
target_link_libraries(pbsd_net_tcp_lro_hpts PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_lro_hpts PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_id)
add_library(pbsd_net_ip_id)
target_sources(pbsd_net_ip_id PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_id.cppm)
target_link_libraries(pbsd_net_ip_id PUBLIC pbsd_core pbsd_net_ip)
target_compile_options(pbsd_net_ip_id PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_fib)
add_library(pbsd_net_in_fib)
target_sources(pbsd_net_in_fib PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_fib.cppm)
target_link_libraries(pbsd_net_in_fib PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_in_fib PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_fib_algo)
add_library(pbsd_net_in_fib_algo)
target_sources(pbsd_net_in_fib_algo PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_fib_algo.cppm)
target_link_libraries(pbsd_net_in_fib_algo PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_in_fib_algo PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_stats)
add_library(pbsd_net_tcp_stats)
target_sources(pbsd_net_tcp_stats PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_stats.cppm)
target_link_libraries(pbsd_net_tcp_stats PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_stats PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_cbq)
add_library(pbsd_net_altq_cbq)
target_sources(pbsd_net_altq_cbq PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_cbq.cppm)
target_link_libraries(pbsd_net_altq_cbq PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_cbq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_codel)
add_library(pbsd_net_altq_codel)
target_sources(pbsd_net_altq_codel PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_codel.cppm)
target_link_libraries(pbsd_net_altq_codel PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_codel PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_fairq)
add_library(pbsd_net_altq_fairq)
target_sources(pbsd_net_altq_fairq PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_fairq.cppm)
target_link_libraries(pbsd_net_altq_fairq PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_fairq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_hfsc)
add_library(pbsd_net_altq_hfsc)
target_sources(pbsd_net_altq_hfsc PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_hfsc.cppm)
target_link_libraries(pbsd_net_altq_hfsc PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_hfsc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_priq)
add_library(pbsd_net_altq_priq)
target_sources(pbsd_net_altq_priq PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_priq.cppm)
target_link_libraries(pbsd_net_altq_priq PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_priq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_red)
add_library(pbsd_net_altq_red)
target_sources(pbsd_net_altq_red PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_red.cppm)
target_link_libraries(pbsd_net_altq_red PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_red PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_rio)
add_library(pbsd_net_altq_rio)
target_sources(pbsd_net_altq_rio PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_rio.cppm)
target_link_libraries(pbsd_net_altq_rio PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_rio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_rmclass)
add_library(pbsd_net_altq_rmclass)
target_sources(pbsd_net_altq_rmclass PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_rmclass.cppm)
target_link_libraries(pbsd_net_altq_rmclass PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_rmclass PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_altq_subr)
add_library(pbsd_net_altq_subr)
target_sources(pbsd_net_altq_subr PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.altq_subr.cppm)
target_link_libraries(pbsd_net_altq_subr PUBLIC pbsd_core pbsd_net_altq)
target_compile_options(pbsd_net_altq_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_bpf_buffer)
add_library(pbsd_net_bpf_buffer)
target_sources(pbsd_net_bpf_buffer PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.bpf_buffer.cppm)
target_link_libraries(pbsd_net_bpf_buffer PUBLIC pbsd_core pbsd_net_bpf)
target_compile_options(pbsd_net_bpf_buffer PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_bpf_filter)
add_library(pbsd_net_bpf_filter)
target_sources(pbsd_net_bpf_filter PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.bpf_filter.cppm)
target_link_libraries(pbsd_net_bpf_filter PUBLIC pbsd_core pbsd_net_bpf)
target_compile_options(pbsd_net_bpf_filter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_bpf_jitter)
add_library(pbsd_net_bpf_jitter)
target_sources(pbsd_net_bpf_jitter PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.bpf_jitter.cppm)
target_link_libraries(pbsd_net_bpf_jitter PUBLIC pbsd_core pbsd_net_bpf)
target_compile_options(pbsd_net_bpf_jitter PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_bpf_zerocopy)
add_library(pbsd_net_bpf_zerocopy)
target_sources(pbsd_net_bpf_zerocopy PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.bpf_zerocopy.cppm)
target_link_libraries(pbsd_net_bpf_zerocopy PUBLIC pbsd_core pbsd_net_bpf)
target_compile_options(pbsd_net_bpf_zerocopy PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_debugnet)
add_library(pbsd_net_debugnet)
target_sources(pbsd_net_debugnet PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.debugnet.cppm)
target_link_libraries(pbsd_net_debugnet PUBLIC pbsd_core)
target_compile_options(pbsd_net_debugnet PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_debugnet_inet)
add_library(pbsd_net_debugnet_inet)
target_sources(pbsd_net_debugnet_inet PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.debugnet_inet.cppm)
target_link_libraries(pbsd_net_debugnet_inet PUBLIC pbsd_core)
target_compile_options(pbsd_net_debugnet_inet PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_if_subr)
add_library(pbsd_net_if_subr)
target_sources(pbsd_net_if_subr PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.if_subr.cppm)
target_link_libraries(pbsd_net_if_subr PUBLIC pbsd_core pbsd_net_ifnet)
target_compile_options(pbsd_net_if_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_helpers)
add_library(pbsd_net_route_helpers)
target_sources(pbsd_net_route_helpers PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.route_helpers.cppm)
target_link_libraries(pbsd_net_route_helpers PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_helpers PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_rtentry)
add_library(pbsd_net_route_rtentry)
target_sources(pbsd_net_route_rtentry PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.route_rtentry.cppm)
target_link_libraries(pbsd_net_route_rtentry PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_rtentry PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_rtsock)
add_library(pbsd_net_rtsock)
target_sources(pbsd_net_rtsock PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.rtsock.cppm)
target_link_libraries(pbsd_net_rtsock PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_rtsock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_rss_config)
add_library(pbsd_net_rss_config)
target_sources(pbsd_net_rss_config PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.rss_config.cppm)
target_link_libraries(pbsd_net_rss_config PUBLIC pbsd_core pbsd_net_rss)
target_compile_options(pbsd_net_rss_config PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_toeplitz)
add_library(pbsd_net_toeplitz)
target_sources(pbsd_net_toeplitz PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.toeplitz.cppm)
target_link_libraries(pbsd_net_toeplitz PUBLIC pbsd_core)
target_compile_options(pbsd_net_toeplitz PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_cdg)
add_library(pbsd_net_cc_cdg)
target_sources(pbsd_net_cc_cdg PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.cc_cdg.cppm)
target_link_libraries(pbsd_net_cc_cdg PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_cdg PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_chd)
add_library(pbsd_net_cc_chd)
target_sources(pbsd_net_cc_chd PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.cc_chd.cppm)
target_link_libraries(pbsd_net_cc_chd PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_chd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_dctcp)
add_library(pbsd_net_cc_dctcp)
target_sources(pbsd_net_cc_dctcp PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.cc_dctcp.cppm)
target_link_libraries(pbsd_net_cc_dctcp PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_dctcp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_hd)
add_library(pbsd_net_cc_hd)
target_sources(pbsd_net_cc_hd PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.cc_hd.cppm)
target_link_libraries(pbsd_net_cc_hd PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_hd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_htcp)
add_library(pbsd_net_cc_htcp)
target_sources(pbsd_net_cc_htcp PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.cc_htcp.cppm)
target_link_libraries(pbsd_net_cc_htcp PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_htcp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_newreno)
add_library(pbsd_net_cc_newreno)
target_sources(pbsd_net_cc_newreno PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.cc_newreno.cppm)
target_link_libraries(pbsd_net_cc_newreno PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_newreno PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_accf_data)
add_library(pbsd_net_accf_data)
target_sources(pbsd_net_accf_data PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.accf_data.cppm)
target_link_libraries(pbsd_net_accf_data PUBLIC pbsd_core)
target_compile_options(pbsd_net_accf_data PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_accf_http)
add_library(pbsd_net_accf_http)
target_sources(pbsd_net_accf_http PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.accf_http.cppm)
target_link_libraries(pbsd_net_accf_http PUBLIC pbsd_core)
target_compile_options(pbsd_net_accf_http PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_offload)
add_library(pbsd_net_tcp_offload)
target_sources(pbsd_net_tcp_offload PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.tcp_offload.cppm)
target_link_libraries(pbsd_net_tcp_offload PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_offload PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_icmp6_input)
add_library(pbsd_net_icmp6_input)
target_sources(pbsd_net_icmp6_input PUBLIC FILE_SET CXX_MODULES FILES
    net/pbsd.net.icmp6_input.cppm)
target_link_libraries(pbsd_net_icmp6_input PUBLIC pbsd_core pbsd_net_icmp6)
target_compile_options(pbsd_net_icmp6_input PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_dummymbuf)
add_library(pbsd_net_dummymbuf)
target_sources(pbsd_net_dummymbuf PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.dummymbuf.cppm)
target_link_libraries(pbsd_net_dummymbuf PUBLIC pbsd_core)
target_compile_options(pbsd_net_dummymbuf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ieee8023ad_lacp)
add_library(pbsd_net_ieee8023ad_lacp)
target_sources(pbsd_net_ieee8023ad_lacp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ieee8023ad_lacp.cppm)
target_link_libraries(pbsd_net_ieee8023ad_lacp PUBLIC pbsd_core pbsd_net_lacp)
target_compile_options(pbsd_net_ieee8023ad_lacp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_iflib)
add_library(pbsd_net_iflib)
target_sources(pbsd_net_iflib PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.iflib.cppm)
target_link_libraries(pbsd_net_iflib PUBLIC pbsd_core pbsd_net_ifnet)
target_compile_options(pbsd_net_iflib PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_mp_ring)
add_library(pbsd_net_mp_ring)
target_sources(pbsd_net_mp_ring PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.mp_ring.cppm)
target_link_libraries(pbsd_net_mp_ring PUBLIC pbsd_core)
target_compile_options(pbsd_net_mp_ring PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_mppcc)
add_library(pbsd_net_mppcc)
target_sources(pbsd_net_mppcc PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.mppcc.cppm)
target_link_libraries(pbsd_net_mppcc PUBLIC pbsd_core)
target_compile_options(pbsd_net_mppcc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_mppcd)
add_library(pbsd_net_mppcd)
target_sources(pbsd_net_mppcd PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.mppcd.cppm)
target_link_libraries(pbsd_net_mppcd PUBLIC pbsd_core)
target_compile_options(pbsd_net_mppcd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_fib_algo)
add_library(pbsd_net_fib_algo)
target_sources(pbsd_net_fib_algo PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.fib_algo.cppm)
target_link_libraries(pbsd_net_fib_algo PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_fib_algo PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_nhgrp)
add_library(pbsd_net_nhgrp)
target_sources(pbsd_net_nhgrp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.nhgrp.cppm)
target_link_libraries(pbsd_net_nhgrp PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_nhgrp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_nhgrp_ctl)
add_library(pbsd_net_nhgrp_ctl)
target_sources(pbsd_net_nhgrp_ctl PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.nhgrp_ctl.cppm)
target_link_libraries(pbsd_net_nhgrp_ctl PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_nhgrp_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_nhop)
add_library(pbsd_net_nhop)
target_sources(pbsd_net_nhop PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.nhop.cppm)
target_link_libraries(pbsd_net_nhop PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_nhop PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_nhop_ctl)
add_library(pbsd_net_nhop_ctl)
target_sources(pbsd_net_nhop_ctl PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.nhop_ctl.cppm)
target_link_libraries(pbsd_net_nhop_ctl PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_nhop_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_nhop_utils)
add_library(pbsd_net_nhop_utils)
target_sources(pbsd_net_nhop_utils PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.nhop_utils.cppm)
target_link_libraries(pbsd_net_nhop_utils PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_nhop_utils PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_ctl)
add_library(pbsd_net_route_ctl)
target_sources(pbsd_net_route_ctl PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route_ctl.cppm)
target_link_libraries(pbsd_net_route_ctl PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_ddb)
add_library(pbsd_net_route_ddb)
target_sources(pbsd_net_route_ddb PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route_ddb.cppm)
target_link_libraries(pbsd_net_route_ddb PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_ddb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_ifaddrs)
add_library(pbsd_net_route_ifaddrs)
target_sources(pbsd_net_route_ifaddrs PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route_ifaddrs.cppm)
target_link_libraries(pbsd_net_route_ifaddrs PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_ifaddrs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_subscription)
add_library(pbsd_net_route_subscription)
target_sources(pbsd_net_route_subscription PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route_subscription.cppm)
target_link_libraries(pbsd_net_route_subscription PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_subscription PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_tables)
add_library(pbsd_net_route_tables)
target_sources(pbsd_net_route_tables PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route_tables.cppm)
target_link_libraries(pbsd_net_route_tables PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_tables PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_route_temporal)
add_library(pbsd_net_route_temporal)
target_sources(pbsd_net_route_temporal PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.route_temporal.cppm)
target_link_libraries(pbsd_net_route_temporal PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_route_temporal PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_slcompress)
add_library(pbsd_net_slcompress)
target_sources(pbsd_net_slcompress PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.slcompress.cppm)
target_link_libraries(pbsd_net_slcompress PUBLIC pbsd_core)
target_compile_options(pbsd_net_slcompress PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ieee80211)
add_library(pbsd_net_ieee80211)
target_sources(pbsd_net_ieee80211 PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ieee80211.cppm)
target_link_libraries(pbsd_net_ieee80211 PUBLIC pbsd_core)
target_compile_options(pbsd_net_ieee80211 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ieee80211_acl)
add_library(pbsd_net_ieee80211_acl)
target_sources(pbsd_net_ieee80211_acl PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ieee80211_acl.cppm)
target_link_libraries(pbsd_net_ieee80211_acl PUBLIC pbsd_core)
target_compile_options(pbsd_net_ieee80211_acl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ieee80211_action)
add_library(pbsd_net_ieee80211_action)
target_sources(pbsd_net_ieee80211_action PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ieee80211_action.cppm)
target_link_libraries(pbsd_net_ieee80211_action PUBLIC pbsd_core)
target_compile_options(pbsd_net_ieee80211_action PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ieee80211_adhoc)
add_library(pbsd_net_ieee80211_adhoc)
target_sources(pbsd_net_ieee80211_adhoc PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ieee80211_adhoc.cppm)
target_link_libraries(pbsd_net_ieee80211_adhoc PUBLIC pbsd_core)
target_compile_options(pbsd_net_ieee80211_adhoc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ieee80211_ageq)
add_library(pbsd_net_ieee80211_ageq)
target_sources(pbsd_net_ieee80211_ageq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ieee80211_ageq.cppm)
target_link_libraries(pbsd_net_ieee80211_ageq PUBLIC pbsd_core)
target_compile_options(pbsd_net_ieee80211_ageq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_accf_dns)
add_library(pbsd_net_accf_dns)
target_sources(pbsd_net_accf_dns PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.accf_dns.cppm)
target_link_libraries(pbsd_net_accf_dns PUBLIC pbsd_core)
target_compile_options(pbsd_net_accf_dns PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_accf_tls)
add_library(pbsd_net_accf_tls)
target_sources(pbsd_net_accf_tls PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.accf_tls.cppm)
target_link_libraries(pbsd_net_accf_tls PUBLIC pbsd_core)
target_compile_options(pbsd_net_accf_tls PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_cc_vegas)
add_library(pbsd_net_cc_vegas)
target_sources(pbsd_net_cc_vegas PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.cc_vegas.cppm)
target_link_libraries(pbsd_net_cc_vegas PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_cc_vegas PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_core)
add_library(pbsd_net_in_core)
target_sources(pbsd_net_in_core PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_core.cppm)
target_link_libraries(pbsd_net_in_core PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_core PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_debug)
add_library(pbsd_net_in_debug)
target_sources(pbsd_net_in_debug PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_debug.cppm)
target_link_libraries(pbsd_net_in_debug PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_debug PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_fib_dxr)
add_library(pbsd_net_in_fib_dxr)
target_sources(pbsd_net_in_fib_dxr PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_fib_dxr.cppm)
target_link_libraries(pbsd_net_in_fib_dxr PUBLIC pbsd_core pbsd_net_route)
target_compile_options(pbsd_net_in_fib_dxr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_gif)
add_library(pbsd_net_in_gif)
target_sources(pbsd_net_in_gif PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_gif.cppm)
target_link_libraries(pbsd_net_in_gif PUBLIC pbsd_core pbsd_net_gif)
target_compile_options(pbsd_net_in_gif PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_jail)
add_library(pbsd_net_in_jail)
target_sources(pbsd_net_in_jail PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_jail.cppm)
target_link_libraries(pbsd_net_in_jail PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_jail PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_kdtrace)
add_library(pbsd_net_in_kdtrace)
target_sources(pbsd_net_in_kdtrace PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_kdtrace.cppm)
target_link_libraries(pbsd_net_in_kdtrace PUBLIC pbsd_core)
target_compile_options(pbsd_net_in_kdtrace PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_in_rss)
add_library(pbsd_net_in_rss)
target_sources(pbsd_net_in_rss PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.in_rss.cppm)
target_link_libraries(pbsd_net_in_rss PUBLIC pbsd_core pbsd_net_rss)
target_compile_options(pbsd_net_in_rss PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_carp)
add_library(pbsd_net_ip_carp)
target_sources(pbsd_net_ip_carp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_carp.cppm)
target_link_libraries(pbsd_net_ip_carp PUBLIC pbsd_core pbsd_net_carp)
target_compile_options(pbsd_net_ip_carp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_divert)
add_library(pbsd_net_ip_divert)
target_sources(pbsd_net_ip_divert PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_divert.cppm)
target_link_libraries(pbsd_net_ip_divert PUBLIC pbsd_core pbsd_net_divert)
target_compile_options(pbsd_net_ip_divert PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_gre)
add_library(pbsd_net_ip_gre)
target_sources(pbsd_net_ip_gre PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_gre.cppm)
target_link_libraries(pbsd_net_ip_gre PUBLIC pbsd_core pbsd_net_gre)
target_compile_options(pbsd_net_ip_gre PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_icmp)
add_library(pbsd_net_ip_icmp)
target_sources(pbsd_net_ip_icmp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_icmp.cppm)
target_link_libraries(pbsd_net_ip_icmp PUBLIC pbsd_core pbsd_net_icmp)
target_compile_options(pbsd_net_ip_icmp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_ip_mroute)
add_library(pbsd_net_ip_mroute)
target_sources(pbsd_net_ip_mroute PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.ip_mroute.cppm)
target_link_libraries(pbsd_net_ip_mroute PUBLIC pbsd_core pbsd_net_mroute)
target_compile_options(pbsd_net_ip_mroute PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_ratelimit)
add_library(pbsd_net_tcp_ratelimit)
target_sources(pbsd_net_tcp_ratelimit PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_ratelimit.cppm)
target_link_libraries(pbsd_net_tcp_ratelimit PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_ratelimit PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_tcp_log_buf)
add_library(pbsd_net_tcp_log_buf)
target_sources(pbsd_net_tcp_log_buf PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.tcp_log_buf.cppm)
target_link_libraries(pbsd_net_tcp_log_buf PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_tcp_log_buf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_toecore)
add_library(pbsd_net_toecore)
target_sources(pbsd_net_toecore PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.toecore.cppm)
target_link_libraries(pbsd_net_toecore PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_toecore PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_siftr)
add_library(pbsd_net_siftr)
target_sources(pbsd_net_siftr PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.siftr.cppm)
target_link_libraries(pbsd_net_siftr PUBLIC pbsd_core)
target_compile_options(pbsd_net_siftr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_h_ertt)
add_library(pbsd_net_h_ertt)
target_sources(pbsd_net_h_ertt PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.h_ertt.cppm)
target_link_libraries(pbsd_net_h_ertt PUBLIC pbsd_core pbsd_net_tcp)
target_compile_options(pbsd_net_h_ertt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp_auth)
add_library(pbsd_net_sctp_auth)
target_sources(pbsd_net_sctp_auth PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp_auth.cppm)
target_link_libraries(pbsd_net_sctp_auth PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp_auth PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp_input)
add_library(pbsd_net_sctp_input)
target_sources(pbsd_net_sctp_input PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp_input.cppm)
target_link_libraries(pbsd_net_sctp_input PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp_input PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp_output)
add_library(pbsd_net_sctp_output)
target_sources(pbsd_net_sctp_output PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp_output.cppm)
target_link_libraries(pbsd_net_sctp_output PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp_output PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp_pcb)
add_library(pbsd_net_sctp_pcb)
target_sources(pbsd_net_sctp_pcb PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp_pcb.cppm)
target_link_libraries(pbsd_net_sctp_pcb PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp_pcb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp_timer)
add_library(pbsd_net_sctp_timer)
target_sources(pbsd_net_sctp_timer PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp_timer.cppm)
target_link_libraries(pbsd_net_sctp_timer PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp_timer PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctp_usrreq)
add_library(pbsd_net_sctp_usrreq)
target_sources(pbsd_net_sctp_usrreq PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctp_usrreq.cppm)
target_link_libraries(pbsd_net_sctp_usrreq PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctp_usrreq PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_sctputil)
add_library(pbsd_net_sctputil)
target_sources(pbsd_net_sctputil PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.sctputil.cppm)
target_link_libraries(pbsd_net_sctputil PUBLIC pbsd_core pbsd_net_sctp)
target_compile_options(pbsd_net_sctputil PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_pf_table)
add_library(pbsd_net_pf_table)
target_sources(pbsd_net_pf_table PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pf_table.cppm)
target_link_libraries(pbsd_net_pf_table PUBLIC pbsd_core pbsd_net_pf)
target_compile_options(pbsd_net_pf_table PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_pf_osfp)
add_library(pbsd_net_pf_osfp)
target_sources(pbsd_net_pf_osfp PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pf_osfp.cppm)
target_link_libraries(pbsd_net_pf_osfp PUBLIC pbsd_core pbsd_net_pf)
target_compile_options(pbsd_net_pf_osfp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_pf_norm)
add_library(pbsd_net_pf_norm)
target_sources(pbsd_net_pf_norm PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pf_norm.cppm)
target_link_libraries(pbsd_net_pf_norm PUBLIC pbsd_core pbsd_net_pf)
target_compile_options(pbsd_net_pf_norm PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net_pf_lb)
add_library(pbsd_net_pf_lb)
target_sources(pbsd_net_pf_lb PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.pf_lb.cppm)
target_link_libraries(pbsd_net_pf_lb PUBLIC pbsd_core pbsd_net_pf)
target_compile_options(pbsd_net_pf_lb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_net)
add_library(pbsd_net)
target_sources(pbsd_net PUBLIC FILE_SET CXX_MODULES FILES net/pbsd.net.cppm)
target_link_libraries(pbsd_net PUBLIC pbsd_handles pbsd_net_sockopt pbsd_net_netinet
    pbsd_net_tcp pbsd_net_udp pbsd_net_ip pbsd_net_icmp pbsd_net_ifnet
    pbsd_net_route pbsd_net_in_pcb pbsd_net_arp pbsd_net_ether pbsd_net_vlan
    pbsd_net_mbuf pbsd_net_bpf pbsd_net_igmp pbsd_net_bridge pbsd_net_pf
    pbsd_net_altq pbsd_net_in6 pbsd_net_gre pbsd_net_epair
    pbsd_net_tun pbsd_net_tap pbsd_net_ndp
    pbsd_net_lagg pbsd_net_gif pbsd_net_loop pbsd_net_ipfw pbsd_net_carp
    pbsd_net_ip6 pbsd_net_udp6 pbsd_net_tcp6 pbsd_net_sctp
    pbsd_net_netmap pbsd_net_netgraph
    pbsd_net_vxlan pbsd_net_ipsec pbsd_net_stf pbsd_net_wg pbsd_net_pfsync
    pbsd_net_dlt pbsd_net_media pbsd_net_pfil pbsd_net_lacp pbsd_net_clone
    pbsd_net_divert pbsd_net_netgraph_node pbsd_net_sctp_assoc
    pbsd_net_bridge_fdb pbsd_net_ipfw_rule pbsd_net_enc pbsd_net_mroute
    pbsd_net_ipdivert pbsd_net_icmp6 pbsd_net_mld6 pbsd_net_raw6
    pbsd_net_dummynet pbsd_net_ip6_mroute pbsd_net_pim6
    pbsd_net_if_ethersubr pbsd_net_netisr pbsd_net_in_rmx pbsd_net_ip_ecn
    pbsd_net_bridgestp pbsd_net_tcp_timer pbsd_net_tcp_timer_wheel
    pbsd_net_tcp_ecn pbsd_net_tcp_reass pbsd_net_tcp_sack pbsd_net_tcp_syncache
    pbsd_net_tcp_timewait pbsd_net_ip_reass pbsd_net_ip_options pbsd_net_in_cksum
    pbsd_net_raw_ip pbsd_net_in_mcast pbsd_net_ip_encap pbsd_net_if_llatbl
    pbsd_net_vnet pbsd_net_radix pbsd_net_ifq pbsd_net_tcp_fastopen
    pbsd_net_tcp_hpts pbsd_net_rss pbsd_net_flowtable
    pbsd_net_tcp_input pbsd_net_tcp_output pbsd_net_in_pcbgroup pbsd_net_in_proto
    pbsd_net_ip_fw_nat pbsd_net_ether_ifattach pbsd_net_udp_usrreq
    pbsd_net_tcp_usrreq pbsd_net_ip_input pbsd_net_ip_output pbsd_net_udp6_usrreq
    pbsd_net_ip6_input pbsd_net_ip6_output pbsd_net_tcp_subr
    pbsd_net_udp_input pbsd_net_in_pcblist pbsd_net_raw_usrreq pbsd_net_tcp_log
    pbsd_net_cc pbsd_net_cc_cubic pbsd_net_pf_ioctl pbsd_net_rip_input pbsd_net_rip_output
    pbsd_net_ip_fastforward pbsd_net_tcp_rfc1323
    pbsd_net_frag6 pbsd_net_ip6_forward pbsd_net_nd6_nbr pbsd_net_nd6_rtr
    pbsd_net_in6_cksum pbsd_net_in6_proto pbsd_net_in6_pcb pbsd_net_scope6
    pbsd_net_sctp6_usrreq pbsd_net_ip6_id pbsd_net_ip6_fastfwd pbsd_net_dest6
    pbsd_net_route6 pbsd_net_tcp_hostcache pbsd_net_tcp_lro pbsd_net_tcp_lro_hpts
    pbsd_net_ip_id pbsd_net_in_fib pbsd_net_in_fib_algo pbsd_net_tcp_stats pbsd_net_altq_cbq pbsd_net_altq_codel pbsd_net_altq_fairq pbsd_net_altq_hfsc pbsd_net_altq_priq pbsd_net_altq_red pbsd_net_altq_rio pbsd_net_altq_rmclass pbsd_net_altq_subr pbsd_net_bpf_buffer pbsd_net_bpf_filter pbsd_net_bpf_jitter pbsd_net_bpf_zerocopy pbsd_net_debugnet pbsd_net_debugnet_inet pbsd_net_if_subr pbsd_net_route_helpers pbsd_net_route_rtentry pbsd_net_rtsock pbsd_net_rss_config pbsd_net_toeplitz     pbsd_net_cc_cdg pbsd_net_cc_chd pbsd_net_cc_dctcp pbsd_net_cc_hd pbsd_net_cc_htcp pbsd_net_cc_newreno pbsd_net_accf_data pbsd_net_accf_http
    pbsd_net_tcp_offload pbsd_net_icmp6_input pbsd_net_dummymbuf pbsd_net_ieee8023ad_lacp pbsd_net_iflib pbsd_net_mp_ring pbsd_net_mppcc pbsd_net_mppcd pbsd_net_fib_algo pbsd_net_nhgrp pbsd_net_nhgrp_ctl pbsd_net_nhop pbsd_net_nhop_ctl pbsd_net_nhop_utils pbsd_net_route_ctl pbsd_net_route_ddb pbsd_net_route_ifaddrs pbsd_net_route_subscription pbsd_net_route_tables pbsd_net_route_temporal pbsd_net_slcompress pbsd_net_ieee80211 pbsd_net_ieee80211_acl pbsd_net_ieee80211_action pbsd_net_ieee80211_adhoc pbsd_net_ieee80211_ageq pbsd_net_accf_dns pbsd_net_accf_tls pbsd_net_cc_vegas pbsd_net_in_core pbsd_net_in_debug pbsd_net_in_fib_dxr pbsd_net_in_gif pbsd_net_in_jail pbsd_net_in_kdtrace pbsd_net_in_prot pbsd_net_in_rss pbsd_net_ip_carp pbsd_net_ip_divert pbsd_net_ip_gre pbsd_net_ip_icmp pbsd_net_ip_mroute pbsd_net_tcp_ratelimit pbsd_net_tcp_log_buf pbsd_net_toecore pbsd_net_siftr pbsd_net_h_ertt pbsd_net_sctp_auth pbsd_net_sctp_input pbsd_net_sctp_output pbsd_net_sctp_pcb pbsd_net_sctp_timer pbsd_net_sctp_usrreq pbsd_net_sctputil pbsd_net_pf_table pbsd_net_pf_osfp pbsd_net_pf_norm pbsd_net_pf_lb)
target_compile_options(pbsd_net PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_fs_vop)
target_sources(pbsd_fs_vop PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vop.cppm)
target_link_libraries(pbsd_fs_vop PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_fs_vop PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_mount)
target_sources(pbsd_fs_mount PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.mount.cppm)
target_link_libraries(pbsd_fs_mount PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_fs_mount PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_namei)
target_sources(pbsd_fs_namei PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.namei.cppm)
target_link_libraries(pbsd_fs_namei PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_fs_namei PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_stat)
target_sources(pbsd_fs_stat PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.stat.cppm)
target_link_libraries(pbsd_fs_stat PUBLIC pbsd_core)
target_compile_options(pbsd_fs_stat PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_fcntl)
target_sources(pbsd_fs_fcntl PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fcntl.cppm)
target_link_libraries(pbsd_fs_fcntl PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_fs_fcntl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_dirent)
target_sources(pbsd_fs_dirent PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.dirent.cppm)
target_link_libraries(pbsd_fs_dirent PUBLIC pbsd_core)
target_compile_options(pbsd_fs_dirent PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_attr)
target_sources(pbsd_fs_attr PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.attr.cppm)
target_link_libraries(pbsd_fs_attr PUBLIC pbsd_core)
target_compile_options(pbsd_fs_attr PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_lockf)
target_sources(pbsd_fs_lockf PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.lockf.cppm)
target_link_libraries(pbsd_fs_lockf PUBLIC pbsd_core)
target_compile_options(pbsd_fs_lockf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_quota)
target_sources(pbsd_fs_quota PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.quota.cppm)
target_link_libraries(pbsd_fs_quota PUBLIC pbsd_core)
target_compile_options(pbsd_fs_quota PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_ufs)
target_sources(pbsd_fs_ufs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ufs.cppm)
target_link_libraries(pbsd_fs_ufs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_ufs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_tmpfs)
target_sources(pbsd_fs_tmpfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.tmpfs.cppm)
target_link_libraries(pbsd_fs_tmpfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_tmpfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_pipefs)
target_sources(pbsd_fs_pipefs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.pipefs.cppm)
target_link_libraries(pbsd_fs_pipefs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_pipefs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_extattr)
target_sources(pbsd_fs_extattr PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.extattr.cppm)
target_link_libraries(pbsd_fs_extattr PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_fs_extattr PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_nfs)
target_sources(pbsd_fs_nfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nfs.cppm)
target_link_libraries(pbsd_fs_nfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_nfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_nullfs)
target_sources(pbsd_fs_nullfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nullfs.cppm)
target_link_libraries(pbsd_fs_nullfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_nullfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_procfs)
target_sources(pbsd_fs_procfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.procfs.cppm)
target_link_libraries(pbsd_fs_procfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_procfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_devfs)
target_sources(pbsd_fs_devfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.devfs.cppm)
target_link_libraries(pbsd_fs_devfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_devfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_fdescfs)
target_sources(pbsd_fs_fdescfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fdescfs.cppm)
target_link_libraries(pbsd_fs_fdescfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_fdescfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_linprocfs)
target_sources(pbsd_fs_linprocfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.linprocfs.cppm)
target_link_libraries(pbsd_fs_linprocfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_linprocfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_fusefs)
target_sources(pbsd_fs_fusefs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fusefs.cppm)
target_link_libraries(pbsd_fs_fusefs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_fusefs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_msdosfs)
target_sources(pbsd_fs_msdosfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.msdosfs.cppm)
target_link_libraries(pbsd_fs_msdosfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_msdosfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_cd9660)
target_sources(pbsd_fs_cd9660 PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660.cppm)
target_link_libraries(pbsd_fs_cd9660 PUBLIC pbsd_core)
target_compile_options(pbsd_fs_cd9660 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_unionfs)
target_sources(pbsd_fs_unionfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.unionfs.cppm)
target_link_libraries(pbsd_fs_unionfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_unionfs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_isofs)
target_sources(pbsd_fs_isofs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.isofs.cppm)
target_link_libraries(pbsd_fs_isofs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_isofs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_autofs)
target_sources(pbsd_fs_autofs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.autofs.cppm)
target_link_libraries(pbsd_fs_autofs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_autofs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_ext2fs)
target_sources(pbsd_fs_ext2fs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2fs.cppm)
target_link_libraries(pbsd_fs_ext2fs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_ext2fs PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_fs_smbfs)
target_sources(pbsd_fs_smbfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.smbfs.cppm)
target_link_libraries(pbsd_fs_smbfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_smbfs PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_fs_ffs)
add_library(pbsd_fs_ffs)
target_sources(pbsd_fs_ffs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ffs.cppm)
target_link_libraries(pbsd_fs_ffs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_ffs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_p9fs)
add_library(pbsd_fs_p9fs)
target_sources(pbsd_fs_p9fs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.p9fs.cppm)
target_link_libraries(pbsd_fs_p9fs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_p9fs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_udf)
add_library(pbsd_fs_udf)
target_sources(pbsd_fs_udf PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.udf.cppm)
target_link_libraries(pbsd_fs_udf PUBLIC pbsd_core)
target_compile_options(pbsd_fs_udf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_specfs)
add_library(pbsd_fs_specfs)
target_sources(pbsd_fs_specfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.specfs.cppm)
target_link_libraries(pbsd_fs_specfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_specfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_cache)
add_library(pbsd_fs_vfs_cache)
target_sources(pbsd_fs_vfs_cache PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_cache.cppm)
target_link_libraries(pbsd_fs_vfs_cache PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_cache PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_aio)
add_library(pbsd_fs_vfs_aio)
target_sources(pbsd_fs_vfs_aio PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_aio.cppm)
target_link_libraries(pbsd_fs_vfs_aio PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_aio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ufs_dir)
add_library(pbsd_fs_ufs_dir)
target_sources(pbsd_fs_ufs_dir PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ufs_dir.cppm)
target_link_libraries(pbsd_fs_ufs_dir PUBLIC pbsd_core pbsd_fs_ufs)
target_compile_options(pbsd_fs_ufs_dir PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_tmpfs_node)
add_library(pbsd_fs_tmpfs_node)
target_sources(pbsd_fs_tmpfs_node PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.tmpfs_node.cppm)
target_link_libraries(pbsd_fs_tmpfs_node PUBLIC pbsd_core pbsd_fs_tmpfs)
target_compile_options(pbsd_fs_tmpfs_node PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_null_vnode)
add_library(pbsd_fs_null_vnode)
target_sources(pbsd_fs_null_vnode PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.null_vnode.cppm)
target_link_libraries(pbsd_fs_null_vnode PUBLIC pbsd_core pbsd_fs_nullfs)
target_compile_options(pbsd_fs_null_vnode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_deadfs)
add_library(pbsd_fs_deadfs)
target_sources(pbsd_fs_deadfs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.deadfs.cppm)
target_link_libraries(pbsd_fs_deadfs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_deadfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_pseudofs)
add_library(pbsd_fs_pseudofs)
target_sources(pbsd_fs_pseudofs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.pseudofs.cppm)
target_link_libraries(pbsd_fs_pseudofs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_pseudofs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_mqueuefs)
add_library(pbsd_fs_mqueuefs)
target_sources(pbsd_fs_mqueuefs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.mqueuefs.cppm)
target_link_libraries(pbsd_fs_mqueuefs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_mqueuefs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fifofs)
add_library(pbsd_fs_fifofs)
target_sources(pbsd_fs_fifofs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fifofs.cppm)
target_link_libraries(pbsd_fs_fifofs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_fifofs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ufs_inode)
add_library(pbsd_fs_ufs_inode)
target_sources(pbsd_fs_ufs_inode PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ufs_inode.cppm)
target_link_libraries(pbsd_fs_ufs_inode PUBLIC pbsd_core pbsd_fs_ufs)
target_compile_options(pbsd_fs_ufs_inode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_bio)
add_library(pbsd_fs_vfs_bio)
target_sources(pbsd_fs_vfs_bio PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_bio.cppm)
target_link_libraries(pbsd_fs_vfs_bio PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_bio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_mount)
add_library(pbsd_fs_vfs_mount)
target_sources(pbsd_fs_vfs_mount PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_mount.cppm)
target_link_libraries(pbsd_fs_vfs_mount PUBLIC pbsd_core pbsd_fs_mount)
target_compile_options(pbsd_fs_vfs_mount PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_mqueuefs)
add_library(pbsd_fs_mqueuefs)
target_sources(pbsd_fs_mqueuefs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.mqueuefs.cppm)
target_link_libraries(pbsd_fs_mqueuefs PUBLIC pbsd_core)
target_compile_options(pbsd_fs_mqueuefs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_union_subr)
add_library(pbsd_fs_union_subr)
target_sources(pbsd_fs_union_subr PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.union_subr.cppm)
target_link_libraries(pbsd_fs_union_subr PUBLIC pbsd_core pbsd_fs_unionfs)
target_compile_options(pbsd_fs_union_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_union_vfsops)
add_library(pbsd_fs_union_vfsops)
target_sources(pbsd_fs_union_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.union_vfsops.cppm)
target_link_libraries(pbsd_fs_union_vfsops PUBLIC pbsd_core pbsd_fs_unionfs)
target_compile_options(pbsd_fs_union_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_union_vnops)
add_library(pbsd_fs_union_vnops)
target_sources(pbsd_fs_union_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.union_vnops.cppm)
target_link_libraries(pbsd_fs_union_vnops PUBLIC pbsd_core pbsd_fs_unionfs)
target_compile_options(pbsd_fs_union_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_nfs_commonport)
add_library(pbsd_fs_nfs_commonport)
target_sources(pbsd_fs_nfs_commonport PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nfs_commonport.cppm)
target_link_libraries(pbsd_fs_nfs_commonport PUBLIC pbsd_core pbsd_fs_nfs)
target_compile_options(pbsd_fs_nfs_commonport PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_nfs_commonsubs)
add_library(pbsd_fs_nfs_commonsubs)
target_sources(pbsd_fs_nfs_commonsubs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nfs_commonsubs.cppm)
target_link_libraries(pbsd_fs_nfs_commonsubs PUBLIC pbsd_core pbsd_fs_nfs)
target_compile_options(pbsd_fs_nfs_commonsubs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2fs_inode)
add_library(pbsd_fs_ext2fs_inode)
target_sources(pbsd_fs_ext2fs_inode PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2fs_inode.cppm)
target_link_libraries(pbsd_fs_ext2fs_inode PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2fs_inode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_subr)
add_library(pbsd_fs_vfs_subr)
target_sources(pbsd_fs_vfs_subr PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_subr.cppm)
target_link_libraries(pbsd_fs_vfs_subr PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_lookup)
add_library(pbsd_fs_vfs_lookup)
target_sources(pbsd_fs_vfs_lookup PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_lookup.cppm)
target_link_libraries(pbsd_fs_vfs_lookup PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_lookup PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_syscalls)
add_library(pbsd_fs_vfs_syscalls)
target_sources(pbsd_fs_vfs_syscalls PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_syscalls.cppm)
target_link_libraries(pbsd_fs_vfs_syscalls PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_syscalls PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_vnode)
add_library(pbsd_fs_vfs_vnode)
target_sources(pbsd_fs_vfs_vnode PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_vnode.cppm)
target_link_libraries(pbsd_fs_vfs_vnode PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_vnode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fifo_vnops)
add_library(pbsd_fs_fifo_vnops)
target_sources(pbsd_fs_fifo_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fifo_vnops.cppm)
target_link_libraries(pbsd_fs_fifo_vnops PUBLIC pbsd_core pbsd_fs_fifofs)
target_compile_options(pbsd_fs_fifo_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ufs_vnops)
add_library(pbsd_fs_ufs_vnops)
target_sources(pbsd_fs_ufs_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ufs_vnops.cppm)
target_link_libraries(pbsd_fs_ufs_vnops PUBLIC pbsd_core pbsd_fs_ufs)
target_compile_options(pbsd_fs_ufs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ffs_vnops)
add_library(pbsd_fs_ffs_vnops)
target_sources(pbsd_fs_ffs_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ffs_vnops.cppm)
target_link_libraries(pbsd_fs_ffs_vnops PUBLIC pbsd_core pbsd_fs_ffs)
target_compile_options(pbsd_fs_ffs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_devfs_vnops)
add_library(pbsd_fs_devfs_vnops)
target_sources(pbsd_fs_devfs_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.devfs_vnops.cppm)
target_link_libraries(pbsd_fs_devfs_vnops PUBLIC pbsd_core pbsd_fs_devfs)
target_compile_options(pbsd_fs_devfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_procfs_vnops)
add_library(pbsd_fs_procfs_vnops)
target_sources(pbsd_fs_procfs_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.procfs_vnops.cppm)
target_link_libraries(pbsd_fs_procfs_vnops PUBLIC pbsd_core pbsd_fs_procfs)
target_compile_options(pbsd_fs_procfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_hash)
add_library(pbsd_fs_vfs_hash)
target_sources(pbsd_fs_vfs_hash PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_hash.cppm)
target_link_libraries(pbsd_fs_vfs_hash PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_hash PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_init)
add_library(pbsd_fs_vfs_init)
target_sources(pbsd_fs_vfs_init PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_init.cppm)
target_link_libraries(pbsd_fs_vfs_init PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_init PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_conf)
add_library(pbsd_fs_vfs_conf)
target_sources(pbsd_fs_vfs_conf PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.vfs_conf.cppm)
target_link_libraries(pbsd_fs_vfs_conf PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_conf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_acl)
add_library(pbsd_fs_vfs_acl)
target_sources(pbsd_fs_vfs_acl PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_acl.cppm)
target_link_libraries(pbsd_fs_vfs_acl PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_acl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_cluster)
add_library(pbsd_fs_vfs_cluster)
target_sources(pbsd_fs_vfs_cluster PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_cluster.cppm)
target_link_libraries(pbsd_fs_vfs_cluster PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_cluster PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_default)
add_library(pbsd_fs_vfs_default)
target_sources(pbsd_fs_vfs_default PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_default.cppm)
target_link_libraries(pbsd_fs_vfs_default PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_default PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_export)
add_library(pbsd_fs_vfs_export)
target_sources(pbsd_fs_vfs_export PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_export.cppm)
target_link_libraries(pbsd_fs_vfs_export PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_export PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_extattr)
add_library(pbsd_fs_vfs_extattr)
target_sources(pbsd_fs_vfs_extattr PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_extattr.cppm)
target_link_libraries(pbsd_fs_vfs_extattr PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_extattr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_inotify)
add_library(pbsd_fs_vfs_inotify)
target_sources(pbsd_fs_vfs_inotify PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_inotify.cppm)
target_link_libraries(pbsd_fs_vfs_inotify PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_inotify PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_mountroot)
add_library(pbsd_fs_vfs_mountroot)
target_sources(pbsd_fs_vfs_mountroot PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_mountroot.cppm)
target_link_libraries(pbsd_fs_vfs_mountroot PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_mountroot PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_vfs_vnops)
add_library(pbsd_fs_vfs_vnops)
target_sources(pbsd_fs_vfs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.vfs_vnops.cppm)
target_link_libraries(pbsd_fs_vfs_vnops PUBLIC pbsd_core)
target_compile_options(pbsd_fs_vfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_autofs_vnops)
add_library(pbsd_fs_autofs_vnops)
target_sources(pbsd_fs_autofs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.autofs_vnops.cppm)
target_link_libraries(pbsd_fs_autofs_vnops PUBLIC pbsd_core pbsd_fs_autofs)
target_compile_options(pbsd_fs_autofs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_vnops)
add_library(pbsd_fs_cd9660_vnops)
target_sources(pbsd_fs_cd9660_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.cd9660_vnops.cppm)
target_link_libraries(pbsd_fs_cd9660_vnops PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_dead_vnops)
add_library(pbsd_fs_dead_vnops)
target_sources(pbsd_fs_dead_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.dead_vnops.cppm)
target_link_libraries(pbsd_fs_dead_vnops PUBLIC pbsd_core pbsd_fs_deadfs)
target_compile_options(pbsd_fs_dead_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_subr)
add_library(pbsd_fs_ext2_subr)
target_sources(pbsd_fs_ext2_subr PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.ext2_subr.cppm)
target_link_libraries(pbsd_fs_ext2_subr PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_vnops)
add_library(pbsd_fs_ext2_vnops)
target_sources(pbsd_fs_ext2_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.ext2_vnops.cppm)
target_link_libraries(pbsd_fs_ext2_vnops PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fdesc_vnops)
add_library(pbsd_fs_fdesc_vnops)
target_sources(pbsd_fs_fdesc_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.fdesc_vnops.cppm)
target_link_libraries(pbsd_fs_fdesc_vnops PUBLIC pbsd_core pbsd_fs_fdescfs)
target_compile_options(pbsd_fs_fdesc_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_vnops)
add_library(pbsd_fs_fuse_vnops)
target_sources(pbsd_fs_fuse_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.fuse_vnops.cppm)
target_link_libraries(pbsd_fs_fuse_vnops PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_msdosfs_vnops)
add_library(pbsd_fs_msdosfs_vnops)
target_sources(pbsd_fs_msdosfs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.msdosfs_vnops.cppm)
target_link_libraries(pbsd_fs_msdosfs_vnops PUBLIC pbsd_core pbsd_fs_msdosfs)
target_compile_options(pbsd_fs_msdosfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_null_subr)
add_library(pbsd_fs_null_subr)
target_sources(pbsd_fs_null_subr PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.null_subr.cppm)
target_link_libraries(pbsd_fs_null_subr PUBLIC pbsd_core pbsd_fs_nullfs)
target_compile_options(pbsd_fs_null_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_null_vnops)
add_library(pbsd_fs_null_vnops)
target_sources(pbsd_fs_null_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.null_vnops.cppm)
target_link_libraries(pbsd_fs_null_vnops PUBLIC pbsd_core pbsd_fs_nullfs)
target_compile_options(pbsd_fs_null_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_p9fs_subr)
add_library(pbsd_fs_p9fs_subr)
target_sources(pbsd_fs_p9fs_subr PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.p9fs_subr.cppm)
target_link_libraries(pbsd_fs_p9fs_subr PUBLIC pbsd_core pbsd_fs_p9fs)
target_compile_options(pbsd_fs_p9fs_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_p9fs_vnops)
add_library(pbsd_fs_p9fs_vnops)
target_sources(pbsd_fs_p9fs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.p9fs_vnops.cppm)
target_link_libraries(pbsd_fs_p9fs_vnops PUBLIC pbsd_core pbsd_fs_p9fs)
target_compile_options(pbsd_fs_p9fs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_pseudofs_vnops)
add_library(pbsd_fs_pseudofs_vnops)
target_sources(pbsd_fs_pseudofs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.pseudofs_vnops.cppm)
target_link_libraries(pbsd_fs_pseudofs_vnops PUBLIC pbsd_core pbsd_fs_pseudofs)
target_compile_options(pbsd_fs_pseudofs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_smbfs_subr)
add_library(pbsd_fs_smbfs_subr)
target_sources(pbsd_fs_smbfs_subr PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.smbfs_subr.cppm)
target_link_libraries(pbsd_fs_smbfs_subr PUBLIC pbsd_core pbsd_fs_smbfs)
target_compile_options(pbsd_fs_smbfs_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_smbfs_vnops)
add_library(pbsd_fs_smbfs_vnops)
target_sources(pbsd_fs_smbfs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.smbfs_vnops.cppm)
target_link_libraries(pbsd_fs_smbfs_vnops PUBLIC pbsd_core pbsd_fs_smbfs)
target_compile_options(pbsd_fs_smbfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_tmpfs_subr)
add_library(pbsd_fs_tmpfs_subr)
target_sources(pbsd_fs_tmpfs_subr PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.tmpfs_subr.cppm)
target_link_libraries(pbsd_fs_tmpfs_subr PUBLIC pbsd_core pbsd_fs_tmpfs)
target_compile_options(pbsd_fs_tmpfs_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_tmpfs_vnops)
add_library(pbsd_fs_tmpfs_vnops)
target_sources(pbsd_fs_tmpfs_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.tmpfs_vnops.cppm)
target_link_libraries(pbsd_fs_tmpfs_vnops PUBLIC pbsd_core pbsd_fs_tmpfs)
target_compile_options(pbsd_fs_tmpfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_udf_vnops)
add_library(pbsd_fs_udf_vnops)
target_sources(pbsd_fs_udf_vnops PUBLIC FILE_SET CXX_MODULES FILES
    fs/pbsd.fs.udf_vnops.cppm)
target_link_libraries(pbsd_fs_udf_vnops PUBLIC pbsd_core pbsd_fs_udf)
target_compile_options(pbsd_fs_udf_vnops PUBLIC ${PBSD_FS_CXX})
endif()


if(NOT TARGET pbsd_fs_autofs_vfsops)
add_library(pbsd_fs_autofs_vfsops)
target_sources(pbsd_fs_autofs_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.autofs_vfsops.cppm)
target_link_libraries(pbsd_fs_autofs_vfsops PUBLIC pbsd_core pbsd_fs_autofs)
target_compile_options(pbsd_fs_autofs_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_bmap)
add_library(pbsd_fs_cd9660_bmap)
target_sources(pbsd_fs_cd9660_bmap PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660_bmap.cppm)
target_link_libraries(pbsd_fs_cd9660_bmap PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_bmap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_lookup)
add_library(pbsd_fs_cd9660_lookup)
target_sources(pbsd_fs_cd9660_lookup PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660_lookup.cppm)
target_link_libraries(pbsd_fs_cd9660_lookup PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_lookup PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_node)
add_library(pbsd_fs_cd9660_node)
target_sources(pbsd_fs_cd9660_node PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660_node.cppm)
target_link_libraries(pbsd_fs_cd9660_node PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_node PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_vfsops)
add_library(pbsd_fs_cd9660_vfsops)
target_sources(pbsd_fs_cd9660_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660_vfsops.cppm)
target_link_libraries(pbsd_fs_cd9660_vfsops PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cuse)
add_library(pbsd_fs_cuse)
target_sources(pbsd_fs_cuse PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cuse.cppm)
target_link_libraries(pbsd_fs_cuse PUBLIC pbsd_core)
target_compile_options(pbsd_fs_cuse PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_devfs_devs)
add_library(pbsd_fs_devfs_devs)
target_sources(pbsd_fs_devfs_devs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.devfs_devs.cppm)
target_link_libraries(pbsd_fs_devfs_devs PUBLIC pbsd_core pbsd_fs_devfs)
target_compile_options(pbsd_fs_devfs_devs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_devfs_dir)
add_library(pbsd_fs_devfs_dir)
target_sources(pbsd_fs_devfs_dir PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.devfs_dir.cppm)
target_link_libraries(pbsd_fs_devfs_dir PUBLIC pbsd_core pbsd_fs_devfs)
target_compile_options(pbsd_fs_devfs_dir PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_devfs_rule)
add_library(pbsd_fs_devfs_rule)
target_sources(pbsd_fs_devfs_rule PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.devfs_rule.cppm)
target_link_libraries(pbsd_fs_devfs_rule PUBLIC pbsd_core pbsd_fs_devfs)
target_compile_options(pbsd_fs_devfs_rule PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_devfs_vfsops)
add_library(pbsd_fs_devfs_vfsops)
target_sources(pbsd_fs_devfs_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.devfs_vfsops.cppm)
target_link_libraries(pbsd_fs_devfs_vfsops PUBLIC pbsd_core pbsd_fs_devfs)
target_compile_options(pbsd_fs_devfs_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_acl)
add_library(pbsd_fs_ext2_acl)
target_sources(pbsd_fs_ext2_acl PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_acl.cppm)
target_link_libraries(pbsd_fs_ext2_acl PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_acl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_alloc)
add_library(pbsd_fs_ext2_alloc)
target_sources(pbsd_fs_ext2_alloc PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_alloc.cppm)
target_link_libraries(pbsd_fs_ext2_alloc PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_alloc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_bmap)
add_library(pbsd_fs_ext2_bmap)
target_sources(pbsd_fs_ext2_bmap PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_bmap.cppm)
target_link_libraries(pbsd_fs_ext2_bmap PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_bmap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_lookup)
add_library(pbsd_fs_ext2_lookup)
target_sources(pbsd_fs_ext2_lookup PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_lookup.cppm)
target_link_libraries(pbsd_fs_ext2_lookup PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_lookup PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_vfsops)
add_library(pbsd_fs_ext2_vfsops)
target_sources(pbsd_fs_ext2_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_vfsops.cppm)
target_link_libraries(pbsd_fs_ext2_vfsops PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_vfsops)
add_library(pbsd_fs_fuse_vfsops)
target_sources(pbsd_fs_fuse_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_vfsops.cppm)
target_link_libraries(pbsd_fs_fuse_vfsops PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_msdosfs_vfsops)
add_library(pbsd_fs_msdosfs_vfsops)
target_sources(pbsd_fs_msdosfs_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.msdosfs_vfsops.cppm)
target_link_libraries(pbsd_fs_msdosfs_vfsops PUBLIC pbsd_core pbsd_fs_msdosfs)
target_compile_options(pbsd_fs_msdosfs_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_null_vfsops)
add_library(pbsd_fs_null_vfsops)
target_sources(pbsd_fs_null_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.null_vfsops.cppm)
target_link_libraries(pbsd_fs_null_vfsops PUBLIC pbsd_core pbsd_fs_nullfs)
target_compile_options(pbsd_fs_null_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_procfs_vfsops)
add_library(pbsd_fs_procfs_vfsops)
target_sources(pbsd_fs_procfs_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.procfs_vfsops.cppm)
target_link_libraries(pbsd_fs_procfs_vfsops PUBLIC pbsd_core pbsd_fs_procfs)
target_compile_options(pbsd_fs_procfs_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_tmpfs_vfsops)
add_library(pbsd_fs_tmpfs_vfsops)
target_sources(pbsd_fs_tmpfs_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.tmpfs_vfsops.cppm)
target_link_libraries(pbsd_fs_tmpfs_vfsops PUBLIC pbsd_core pbsd_fs_tmpfs)
target_compile_options(pbsd_fs_tmpfs_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_iconv)
add_library(pbsd_fs_cd9660_iconv)
target_sources(pbsd_fs_cd9660_iconv PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660_iconv.cppm)
target_link_libraries(pbsd_fs_cd9660_iconv PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_iconv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_rrip)
add_library(pbsd_fs_cd9660_rrip)
target_sources(pbsd_fs_cd9660_rrip PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660_rrip.cppm)
target_link_libraries(pbsd_fs_cd9660_rrip PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_rrip PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_cd9660_util)
add_library(pbsd_fs_cd9660_util)
target_sources(pbsd_fs_cd9660_util PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cd9660_util.cppm)
target_link_libraries(pbsd_fs_cd9660_util PUBLIC pbsd_core pbsd_fs_cd9660)
target_compile_options(pbsd_fs_cd9660_util PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_balloc)
add_library(pbsd_fs_ext2_balloc)
target_sources(pbsd_fs_ext2_balloc PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_balloc.cppm)
target_link_libraries(pbsd_fs_ext2_balloc PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_balloc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_csum)
add_library(pbsd_fs_ext2_csum)
target_sources(pbsd_fs_ext2_csum PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_csum.cppm)
target_link_libraries(pbsd_fs_ext2_csum PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_csum PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_extattr)
add_library(pbsd_fs_ext2_extattr)
target_sources(pbsd_fs_ext2_extattr PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_extattr.cppm)
target_link_libraries(pbsd_fs_ext2_extattr PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_extattr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_extents)
add_library(pbsd_fs_ext2_extents)
target_sources(pbsd_fs_ext2_extents PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_extents.cppm)
target_link_libraries(pbsd_fs_ext2_extents PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_extents PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_hash)
add_library(pbsd_fs_ext2_hash)
target_sources(pbsd_fs_ext2_hash PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_hash.cppm)
target_link_libraries(pbsd_fs_ext2_hash PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_hash PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_htree)
add_library(pbsd_fs_ext2_htree)
target_sources(pbsd_fs_ext2_htree PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_htree.cppm)
target_link_libraries(pbsd_fs_ext2_htree PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_htree PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_inode)
add_library(pbsd_fs_ext2_inode)
target_sources(pbsd_fs_ext2_inode PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_inode.cppm)
target_link_libraries(pbsd_fs_ext2_inode PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_inode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ext2_inode_cnv)
add_library(pbsd_fs_ext2_inode_cnv)
target_sources(pbsd_fs_ext2_inode_cnv PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ext2_inode_cnv.cppm)
target_link_libraries(pbsd_fs_ext2_inode_cnv PUBLIC pbsd_core pbsd_fs_ext2fs)
target_compile_options(pbsd_fs_ext2_inode_cnv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fdesc_vfsops)
add_library(pbsd_fs_fdesc_vfsops)
target_sources(pbsd_fs_fdesc_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fdesc_vfsops.cppm)
target_link_libraries(pbsd_fs_fdesc_vfsops PUBLIC pbsd_core pbsd_fs_fdescfs)
target_compile_options(pbsd_fs_fdesc_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_device)
add_library(pbsd_fs_fuse_device)
target_sources(pbsd_fs_fuse_device PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_device.cppm)
target_link_libraries(pbsd_fs_fuse_device PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_device PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_file)
add_library(pbsd_fs_fuse_file)
target_sources(pbsd_fs_fuse_file PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_file.cppm)
target_link_libraries(pbsd_fs_fuse_file PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_file PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_internal)
add_library(pbsd_fs_fuse_internal)
target_sources(pbsd_fs_fuse_internal PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_internal.cppm)
target_link_libraries(pbsd_fs_fuse_internal PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_internal PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_io)
add_library(pbsd_fs_fuse_io)
target_sources(pbsd_fs_fuse_io PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_io.cppm)
target_link_libraries(pbsd_fs_fuse_io PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_io PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_ipc)
add_library(pbsd_fs_fuse_ipc)
target_sources(pbsd_fs_fuse_ipc PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_ipc.cppm)
target_link_libraries(pbsd_fs_fuse_ipc PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_ipc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_main)
add_library(pbsd_fs_fuse_main)
target_sources(pbsd_fs_fuse_main PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_main.cppm)
target_link_libraries(pbsd_fs_fuse_main PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_main PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_fuse_node)
add_library(pbsd_fs_fuse_node)
target_sources(pbsd_fs_fuse_node PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.fuse_node.cppm)
target_link_libraries(pbsd_fs_fuse_node PUBLIC pbsd_core pbsd_fs_fusefs)
target_compile_options(pbsd_fs_fuse_node PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_mntfs_vnops)
add_library(pbsd_fs_mntfs_vnops)
target_sources(pbsd_fs_mntfs_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.mntfs_vnops.cppm)
target_link_libraries(pbsd_fs_mntfs_vnops PUBLIC pbsd_core)
target_compile_options(pbsd_fs_mntfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_msdosfs_conv)
add_library(pbsd_fs_msdosfs_conv)
target_sources(pbsd_fs_msdosfs_conv PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.msdosfs_conv.cppm)
target_link_libraries(pbsd_fs_msdosfs_conv PUBLIC pbsd_core pbsd_fs_msdosfs)
target_compile_options(pbsd_fs_msdosfs_conv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_msdosfs_denode)
add_library(pbsd_fs_msdosfs_denode)
target_sources(pbsd_fs_msdosfs_denode PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.msdosfs_denode.cppm)
target_link_libraries(pbsd_fs_msdosfs_denode PUBLIC pbsd_core pbsd_fs_msdosfs)
target_compile_options(pbsd_fs_msdosfs_denode PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_msdosfs_fat)
add_library(pbsd_fs_msdosfs_fat)
target_sources(pbsd_fs_msdosfs_fat PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.msdosfs_fat.cppm)
target_link_libraries(pbsd_fs_msdosfs_fat PUBLIC pbsd_core pbsd_fs_msdosfs)
target_compile_options(pbsd_fs_msdosfs_fat PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_msdosfs_iconv)
add_library(pbsd_fs_msdosfs_iconv)
target_sources(pbsd_fs_msdosfs_iconv PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.msdosfs_iconv.cppm)
target_link_libraries(pbsd_fs_msdosfs_iconv PUBLIC pbsd_core pbsd_fs_msdosfs)
target_compile_options(pbsd_fs_msdosfs_iconv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_msdosfs_lookup)
add_library(pbsd_fs_msdosfs_lookup)
target_sources(pbsd_fs_msdosfs_lookup PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.msdosfs_lookup.cppm)
target_link_libraries(pbsd_fs_msdosfs_lookup PUBLIC pbsd_core pbsd_fs_msdosfs)
target_compile_options(pbsd_fs_msdosfs_lookup PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_nfs_clport)
add_library(pbsd_fs_nfs_clport)
target_sources(pbsd_fs_nfs_clport PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nfs_clport.cppm)
target_link_libraries(pbsd_fs_nfs_clport PUBLIC pbsd_core pbsd_fs_nfs)
target_compile_options(pbsd_fs_nfs_clport PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_nfs_clsubs)
add_library(pbsd_fs_nfs_clsubs)
target_sources(pbsd_fs_nfs_clsubs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nfs_clsubs.cppm)
target_link_libraries(pbsd_fs_nfs_clsubs PUBLIC pbsd_core pbsd_fs_nfs)
target_compile_options(pbsd_fs_nfs_clsubs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ufs_bmap)
add_library(pbsd_fs_ufs_bmap)
target_sources(pbsd_fs_ufs_bmap PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ufs_bmap.cppm)
target_link_libraries(pbsd_fs_ufs_bmap PUBLIC pbsd_core pbsd_fs_ufs)
target_compile_options(pbsd_fs_ufs_bmap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_ufs_quota)
add_library(pbsd_fs_ufs_quota)
target_sources(pbsd_fs_ufs_quota PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.ufs_quota.cppm)
target_link_libraries(pbsd_fs_ufs_quota PUBLIC pbsd_core pbsd_fs_quota)
target_compile_options(pbsd_fs_ufs_quota PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_nfs_clvnops)
add_library(pbsd_fs_nfs_clvnops)
target_sources(pbsd_fs_nfs_clvnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nfs_clvnops.cppm)
target_link_libraries(pbsd_fs_nfs_clvnops PUBLIC pbsd_core pbsd_fs_nfs)
target_compile_options(pbsd_fs_nfs_clvnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_nfs_clvfsops)
add_library(pbsd_fs_nfs_clvfsops)
target_sources(pbsd_fs_nfs_clvfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.nfs_clvfsops.cppm)
target_link_libraries(pbsd_fs_nfs_clvfsops PUBLIC pbsd_core pbsd_fs_nfs)
target_compile_options(pbsd_fs_nfs_clvfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_procfs_status)
add_library(pbsd_fs_procfs_status)
target_sources(pbsd_fs_procfs_status PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.procfs_status.cppm)
target_link_libraries(pbsd_fs_procfs_status PUBLIC pbsd_core pbsd_fs_procfs)
target_compile_options(pbsd_fs_procfs_status PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_udf_vfsops)
add_library(pbsd_fs_udf_vfsops)
target_sources(pbsd_fs_udf_vfsops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.udf_vfsops.cppm)
target_link_libraries(pbsd_fs_udf_vfsops PUBLIC pbsd_core pbsd_fs_udf)
target_compile_options(pbsd_fs_udf_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs_unionfs_vnops)
add_library(pbsd_fs_unionfs_vnops)
target_sources(pbsd_fs_unionfs_vnops PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.unionfs_vnops.cppm)
target_link_libraries(pbsd_fs_unionfs_vnops PUBLIC pbsd_core pbsd_fs_unionfs)
target_compile_options(pbsd_fs_unionfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_fs)
add_library(pbsd_fs)
target_sources(pbsd_fs PUBLIC FILE_SET CXX_MODULES FILES fs/pbsd.fs.cppm)
target_link_libraries(pbsd_fs PUBLIC pbsd_handles pbsd_fs_vop pbsd_fs_mount pbsd_fs_namei
    pbsd_fs_stat pbsd_fs_fcntl pbsd_fs_dirent
    pbsd_fs_attr pbsd_fs_lockf pbsd_fs_quota pbsd_fs_ufs
    pbsd_fs_tmpfs pbsd_fs_pipefs pbsd_fs_extattr pbsd_fs_nfs
    pbsd_fs_nullfs pbsd_fs_procfs pbsd_fs_devfs pbsd_fs_fdescfs
    pbsd_fs_linprocfs pbsd_fs_fusefs
    pbsd_fs_msdosfs pbsd_fs_cd9660 pbsd_fs_unionfs
    pbsd_fs_isofs pbsd_fs_autofs pbsd_fs_ext2fs pbsd_fs_smbfs
    pbsd_fs_ffs pbsd_fs_p9fs pbsd_fs_udf pbsd_fs_specfs
    pbsd_fs_vfs_cache pbsd_fs_vfs_aio pbsd_fs_ufs_dir pbsd_fs_tmpfs_node
    pbsd_fs_null_vnode pbsd_fs_deadfs pbsd_fs_pseudofs
    pbsd_fs_fifofs pbsd_fs_ufs_inode pbsd_fs_vfs_bio pbsd_fs_vfs_mount
    pbsd_fs_mqueuefs pbsd_fs_union_subr pbsd_fs_union_vfsops pbsd_fs_union_vnops
    pbsd_fs_nfs_commonport pbsd_fs_nfs_commonsubs pbsd_fs_ext2fs_inode
    pbsd_fs_vfs_subr pbsd_fs_vfs_lookup pbsd_fs_vfs_syscalls pbsd_fs_vfs_vnode
    pbsd_fs_fifo_vnops pbsd_fs_ufs_vnops pbsd_fs_ffs_vnops pbsd_fs_devfs_vnops
    pbsd_fs_procfs_vnops pbsd_fs_vfs_hash pbsd_fs_vfs_init pbsd_fs_vfs_conf pbsd_fs_vfs_acl pbsd_fs_vfs_cluster pbsd_fs_vfs_default pbsd_fs_vfs_export pbsd_fs_vfs_extattr pbsd_fs_vfs_inotify pbsd_fs_vfs_mountroot pbsd_fs_vfs_vnops pbsd_fs_autofs_vnops pbsd_fs_cd9660_vnops pbsd_fs_dead_vnops pbsd_fs_ext2_subr pbsd_fs_ext2_vnops pbsd_fs_fdesc_vnops pbsd_fs_fuse_vnops pbsd_fs_msdosfs_vnops pbsd_fs_null_subr pbsd_fs_null_vnops pbsd_fs_p9fs_subr pbsd_fs_p9fs_vnops pbsd_fs_pseudofs_vnops pbsd_fs_smbfs_subr pbsd_fs_smbfs_vnops pbsd_fs_tmpfs_subr pbsd_fs_tmpfs_vnops pbsd_fs_udf_vnops pbsd_fs_autofs_vfsops pbsd_fs_cd9660_bmap pbsd_fs_cd9660_lookup pbsd_fs_cd9660_node pbsd_fs_cd9660_vfsops pbsd_fs_cuse pbsd_fs_devfs_devs pbsd_fs_devfs_dir pbsd_fs_devfs_rule pbsd_fs_devfs_vfsops pbsd_fs_ext2_acl pbsd_fs_ext2_alloc pbsd_fs_ext2_bmap pbsd_fs_ext2_lookup pbsd_fs_ext2_vfsops pbsd_fs_fuse_vfsops pbsd_fs_msdosfs_vfsops pbsd_fs_null_vfsops pbsd_fs_procfs_vfsops pbsd_fs_tmpfs_vfsops pbsd_fs_cd9660_iconv pbsd_fs_cd9660_rrip pbsd_fs_cd9660_util pbsd_fs_ext2_balloc pbsd_fs_ext2_csum pbsd_fs_ext2_extattr pbsd_fs_ext2_extents pbsd_fs_ext2_hash pbsd_fs_ext2_htree pbsd_fs_ext2_inode pbsd_fs_ext2_inode_cnv pbsd_fs_fdesc_vfsops pbsd_fs_fuse_device pbsd_fs_fuse_file pbsd_fs_fuse_internal pbsd_fs_fuse_io pbsd_fs_fuse_ipc pbsd_fs_fuse_main pbsd_fs_fuse_node pbsd_fs_mntfs_vnops pbsd_fs_msdosfs_conv pbsd_fs_msdosfs_denode pbsd_fs_msdosfs_fat pbsd_fs_msdosfs_iconv     pbsd_fs_msdosfs_lookup pbsd_fs_nfs_clport pbsd_fs_nfs_clsubs pbsd_fs_ufs_bmap pbsd_fs_ufs_quota pbsd_fs_nfs_clvnops pbsd_fs_nfs_clvfsops pbsd_fs_procfs_status pbsd_fs_udf_vfsops pbsd_fs_unionfs_vnops)
target_compile_options(pbsd_fs PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_geom_ctl)
target_sources(pbsd_geom_ctl PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.ctl.cppm)
target_link_libraries(pbsd_geom_ctl PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_geom_ctl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_disk)
target_sources(pbsd_geom_disk PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.disk.cppm)
target_link_libraries(pbsd_geom_disk PUBLIC pbsd_core)
target_compile_options(pbsd_geom_disk PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_slice)
target_sources(pbsd_geom_slice PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.slice.cppm)
target_link_libraries(pbsd_geom_slice PUBLIC pbsd_core)
target_compile_options(pbsd_geom_slice PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_flashmap)
target_sources(pbsd_geom_flashmap PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.flashmap.cppm)
target_link_libraries(pbsd_geom_flashmap PUBLIC pbsd_core)
target_compile_options(pbsd_geom_flashmap PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_part)
target_sources(pbsd_geom_part PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part.cppm)
target_link_libraries(pbsd_geom_part PUBLIC pbsd_core)
target_compile_options(pbsd_geom_part PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_mirror)
target_sources(pbsd_geom_mirror PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.mirror.cppm)
target_link_libraries(pbsd_geom_mirror PUBLIC pbsd_core)
target_compile_options(pbsd_geom_mirror PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_label)
target_sources(pbsd_geom_label PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label.cppm)
target_link_libraries(pbsd_geom_label PUBLIC pbsd_core)
target_compile_options(pbsd_geom_label PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_taste)
target_sources(pbsd_geom_taste PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.taste.cppm)
target_link_libraries(pbsd_geom_taste PUBLIC pbsd_core)
target_compile_options(pbsd_geom_taste PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_stripe)
target_sources(pbsd_geom_stripe PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.stripe.cppm)
target_link_libraries(pbsd_geom_stripe PUBLIC pbsd_core)
target_compile_options(pbsd_geom_stripe PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_concat)
target_sources(pbsd_geom_concat PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.concat.cppm)
target_link_libraries(pbsd_geom_concat PUBLIC pbsd_core)
target_compile_options(pbsd_geom_concat PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_geom_eli)
add_library(pbsd_geom_eli)
target_sources(pbsd_geom_eli PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli.cppm)
target_link_libraries(pbsd_geom_eli PUBLIC pbsd_core)
target_compile_options(pbsd_geom_eli PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_geom_raid3)
target_sources(pbsd_geom_raid3 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid3.cppm)
target_link_libraries(pbsd_geom_raid3 PUBLIC pbsd_core)
target_compile_options(pbsd_geom_raid3 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_journal)
target_sources(pbsd_geom_journal PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.journal.cppm)
target_link_libraries(pbsd_geom_journal PUBLIC pbsd_core)
target_compile_options(pbsd_geom_journal PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_gate)
target_sources(pbsd_geom_gate PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.gate.cppm)
target_link_libraries(pbsd_geom_gate PUBLIC pbsd_core)
target_compile_options(pbsd_geom_gate PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_multipath)
target_sources(pbsd_geom_multipath PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.multipath.cppm)
target_link_libraries(pbsd_geom_multipath PUBLIC pbsd_core)
target_compile_options(pbsd_geom_multipath PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_cache)
target_sources(pbsd_geom_cache PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.cache.cppm)
target_link_libraries(pbsd_geom_cache PUBLIC pbsd_core)
target_compile_options(pbsd_geom_cache PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_shsec)
target_sources(pbsd_geom_shsec PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.shsec.cppm)
target_link_libraries(pbsd_geom_shsec PUBLIC pbsd_core)
target_compile_options(pbsd_geom_shsec PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_virstor)
target_sources(pbsd_geom_virstor PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.virstor.cppm)
target_link_libraries(pbsd_geom_virstor PUBLIC pbsd_core)
target_compile_options(pbsd_geom_virstor PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_raid)
target_sources(pbsd_geom_raid PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid.cppm)
target_link_libraries(pbsd_geom_raid PUBLIC pbsd_core)
target_compile_options(pbsd_geom_raid PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_geom_mountver)
target_sources(pbsd_geom_mountver PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.mountver.cppm)
target_link_libraries(pbsd_geom_mountver PUBLIC pbsd_core)
target_compile_options(pbsd_geom_mountver PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_geom_nop)
add_library(pbsd_geom_nop)
target_sources(pbsd_geom_nop PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.nop.cppm)
target_link_libraries(pbsd_geom_nop PUBLIC pbsd_core)
target_compile_options(pbsd_geom_nop PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_zero)
add_library(pbsd_geom_zero)
target_sources(pbsd_geom_zero PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.zero.cppm)
target_link_libraries(pbsd_geom_zero PUBLIC pbsd_core)
target_compile_options(pbsd_geom_zero PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_uzip)
add_library(pbsd_geom_uzip)
target_sources(pbsd_geom_uzip PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.uzip.cppm)
target_link_libraries(pbsd_geom_uzip PUBLIC pbsd_core)
target_compile_options(pbsd_geom_uzip PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_gunion)
add_library(pbsd_geom_gunion)
target_sources(pbsd_geom_gunion PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.gunion.cppm)
target_link_libraries(pbsd_geom_gunion PUBLIC pbsd_core)
target_compile_options(pbsd_geom_gunion PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_shred)
add_library(pbsd_geom_shred)
target_sources(pbsd_geom_shred PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.shred.cppm)
target_link_libraries(pbsd_geom_shred PUBLIC pbsd_core)
target_compile_options(pbsd_geom_shred PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_linux_lvm)
add_library(pbsd_geom_linux_lvm)
target_sources(pbsd_geom_linux_lvm PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.linux_lvm.cppm)
target_link_libraries(pbsd_geom_linux_lvm PUBLIC pbsd_core)
target_compile_options(pbsd_geom_linux_lvm PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_subr)
add_library(pbsd_geom_subr)
target_sources(pbsd_geom_subr PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.subr.cppm)
target_link_libraries(pbsd_geom_subr PUBLIC pbsd_core)
target_compile_options(pbsd_geom_subr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_io)
add_library(pbsd_geom_io)
target_sources(pbsd_geom_io PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.io.cppm)
target_link_libraries(pbsd_geom_io PUBLIC pbsd_core)
target_compile_options(pbsd_geom_io PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_event)
add_library(pbsd_geom_event)
target_sources(pbsd_geom_event PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.event.cppm)
target_link_libraries(pbsd_geom_event PUBLIC pbsd_core)
target_compile_options(pbsd_geom_event PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_vfs)
add_library(pbsd_geom_vfs)
target_sources(pbsd_geom_vfs PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.vfs.cppm)
target_link_libraries(pbsd_geom_vfs PUBLIC pbsd_core)
target_compile_options(pbsd_geom_vfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_ccd)
add_library(pbsd_geom_ccd)
target_sources(pbsd_geom_ccd PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.ccd.cppm)
target_link_libraries(pbsd_geom_ccd PUBLIC pbsd_core)
target_compile_options(pbsd_geom_ccd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_bsd)
add_library(pbsd_geom_bsd)
target_sources(pbsd_geom_bsd PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.bsd.cppm)
target_link_libraries(pbsd_geom_bsd PUBLIC pbsd_core)
target_compile_options(pbsd_geom_bsd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_core)
add_library(pbsd_geom_core)
target_sources(pbsd_geom_core PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.core.cppm)
target_link_libraries(pbsd_geom_core PUBLIC pbsd_core)
target_compile_options(pbsd_geom_core PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_bsdlabel)
add_library(pbsd_geom_bsdlabel)
target_sources(pbsd_geom_bsdlabel PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.bsdlabel.cppm)
target_link_libraries(pbsd_geom_bsdlabel PUBLIC pbsd_core pbsd_geom_bsd)
target_compile_options(pbsd_geom_bsdlabel PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_dump)
add_library(pbsd_geom_dump)
target_sources(pbsd_geom_dump PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.dump.cppm)
target_link_libraries(pbsd_geom_dump PUBLIC pbsd_core)
target_compile_options(pbsd_geom_dump PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_part_gpt)
add_library(pbsd_geom_part_gpt)
target_sources(pbsd_geom_part_gpt PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part_gpt.cppm)
target_link_libraries(pbsd_geom_part_gpt PUBLIC pbsd_core pbsd_geom_part)
target_compile_options(pbsd_geom_part_gpt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_part_mbr)
add_library(pbsd_geom_part_mbr)
target_sources(pbsd_geom_part_mbr PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part_mbr.cppm)
target_link_libraries(pbsd_geom_part_mbr PUBLIC pbsd_core pbsd_geom_part)
target_compile_options(pbsd_geom_part_mbr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_eli_key)
add_library(pbsd_geom_eli_key)
target_sources(pbsd_geom_eli_key PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli_key.cppm)
target_link_libraries(pbsd_geom_eli_key PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_eli_key PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_journal_ufs)
add_library(pbsd_geom_journal_ufs)
target_sources(pbsd_geom_journal_ufs PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.journal_ufs.cppm)
target_link_libraries(pbsd_geom_journal_ufs PUBLIC pbsd_core pbsd_geom_journal)
target_compile_options(pbsd_geom_journal_ufs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_mirror_ctl)
add_library(pbsd_geom_mirror_ctl)
target_sources(pbsd_geom_mirror_ctl PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.mirror_ctl.cppm)
target_link_libraries(pbsd_geom_mirror_ctl PUBLIC pbsd_core pbsd_geom_mirror)
target_compile_options(pbsd_geom_mirror_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_ctl)
add_library(pbsd_geom_raid_ctl)
target_sources(pbsd_geom_raid_ctl PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_ctl.cppm)
target_link_libraries(pbsd_geom_raid_ctl PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_gpt)
add_library(pbsd_geom_label_gpt)
target_sources(pbsd_geom_label_gpt PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_gpt.cppm)
target_link_libraries(pbsd_geom_label_gpt PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_gpt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_ufs)
add_library(pbsd_geom_label_ufs)
target_sources(pbsd_geom_label_ufs PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_ufs.cppm)
target_link_libraries(pbsd_geom_label_ufs PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_ufs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_orphan)
add_library(pbsd_geom_orphan)
target_sources(pbsd_geom_orphan PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.orphan.cppm)
target_link_libraries(pbsd_geom_orphan PUBLIC pbsd_core)
target_compile_options(pbsd_geom_orphan PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_up)
add_library(pbsd_geom_up)
target_sources(pbsd_geom_up PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.up.cppm)
target_link_libraries(pbsd_geom_up PUBLIC pbsd_core)
target_compile_options(pbsd_geom_up PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_subr_disk)
add_library(pbsd_geom_subr_disk)
target_sources(pbsd_geom_subr_disk PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.subr_disk.cppm)
target_link_libraries(pbsd_geom_subr_disk PUBLIC pbsd_core pbsd_geom_disk)
target_compile_options(pbsd_geom_subr_disk PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_eli_crypto)
add_library(pbsd_geom_eli_crypto)
target_sources(pbsd_geom_eli_crypto PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli_crypto.cppm)
target_link_libraries(pbsd_geom_eli_crypto PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_eli_crypto PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_eli_ctl)
add_library(pbsd_geom_eli_ctl)
target_sources(pbsd_geom_eli_ctl PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli_ctl.cppm)
target_link_libraries(pbsd_geom_eli_ctl PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_eli_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_eli_hmac)
add_library(pbsd_geom_eli_hmac)
target_sources(pbsd_geom_eli_hmac PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli_hmac.cppm)
target_link_libraries(pbsd_geom_eli_hmac PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_eli_hmac PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_eli_integrity)
add_library(pbsd_geom_eli_integrity)
target_sources(pbsd_geom_eli_integrity PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli_integrity.cppm)
target_link_libraries(pbsd_geom_eli_integrity PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_eli_integrity PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_eli_key_cache)
add_library(pbsd_geom_eli_key_cache)
target_sources(pbsd_geom_eli_key_cache PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli_key_cache.cppm)
target_link_libraries(pbsd_geom_eli_key_cache PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_eli_key_cache PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_eli_privacy)
add_library(pbsd_geom_eli_privacy)
target_sources(pbsd_geom_eli_privacy PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.eli_privacy.cppm)
target_link_libraries(pbsd_geom_eli_privacy PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_eli_privacy PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_pkcs5v2)
add_library(pbsd_geom_pkcs5v2)
target_sources(pbsd_geom_pkcs5v2 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.pkcs5v2.cppm)
target_link_libraries(pbsd_geom_pkcs5v2 PUBLIC pbsd_core pbsd_geom_eli)
target_compile_options(pbsd_geom_pkcs5v2 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_geom_kern)
add_library(pbsd_geom_geom_kern)
target_sources(pbsd_geom_geom_kern PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.geom_kern.cppm)
target_link_libraries(pbsd_geom_geom_kern PUBLIC pbsd_core)
target_compile_options(pbsd_geom_geom_kern PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_geom_dev)
add_library(pbsd_geom_geom_dev)
target_sources(pbsd_geom_geom_dev PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.geom_dev.cppm)
target_link_libraries(pbsd_geom_geom_dev PUBLIC pbsd_core)
target_compile_options(pbsd_geom_geom_dev PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_disk_ident)
add_library(pbsd_geom_label_disk_ident)
target_sources(pbsd_geom_label_disk_ident PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_disk_ident.cppm)
target_link_libraries(pbsd_geom_label_disk_ident PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_disk_ident PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_ext2fs)
add_library(pbsd_geom_label_ext2fs)
target_sources(pbsd_geom_label_ext2fs PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_ext2fs.cppm)
target_link_libraries(pbsd_geom_label_ext2fs PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_ext2fs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_flashmap)
add_library(pbsd_geom_label_flashmap)
target_sources(pbsd_geom_label_flashmap PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_flashmap.cppm)
target_link_libraries(pbsd_geom_label_flashmap PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_flashmap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_iso9660)
add_library(pbsd_geom_label_iso9660)
target_sources(pbsd_geom_label_iso9660 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_iso9660.cppm)
target_link_libraries(pbsd_geom_label_iso9660 PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_iso9660 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_msdosfs)
add_library(pbsd_geom_label_msdosfs)
target_sources(pbsd_geom_label_msdosfs PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_msdosfs.cppm)
target_link_libraries(pbsd_geom_label_msdosfs PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_msdosfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_reiserfs)
add_library(pbsd_geom_label_reiserfs)
target_sources(pbsd_geom_label_reiserfs PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_reiserfs.cppm)
target_link_libraries(pbsd_geom_label_reiserfs PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_reiserfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_ufs_id)
add_library(pbsd_geom_label_ufs_id)
target_sources(pbsd_geom_label_ufs_id PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_ufs_id.cppm)
target_link_libraries(pbsd_geom_label_ufs_id PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_ufs_id PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_geom_bsd_enc)
add_library(pbsd_geom_geom_bsd_enc)
target_sources(pbsd_geom_geom_bsd_enc PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.geom_bsd_enc.cppm)
target_link_libraries(pbsd_geom_geom_bsd_enc PUBLIC pbsd_core)
target_compile_options(pbsd_geom_geom_bsd_enc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_ntfs)
add_library(pbsd_geom_label_ntfs)
target_sources(pbsd_geom_label_ntfs PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_ntfs.cppm)
target_link_libraries(pbsd_geom_label_ntfs PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_ntfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_swaplinux)
add_library(pbsd_geom_label_swaplinux)
target_sources(pbsd_geom_label_swaplinux PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_swaplinux.cppm)
target_link_libraries(pbsd_geom_label_swaplinux PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_swaplinux PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_swap)
add_library(pbsd_geom_label_swap)
target_sources(pbsd_geom_label_swap PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_swap.cppm)
target_link_libraries(pbsd_geom_label_swap PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_swap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_vtoc8)
add_library(pbsd_geom_label_vtoc8)
target_sources(pbsd_geom_label_vtoc8 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_vtoc8.cppm)
target_link_libraries(pbsd_geom_label_vtoc8 PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_vtoc8 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_label_gpt_uuid)
add_library(pbsd_geom_label_gpt_uuid)
target_sources(pbsd_geom_label_gpt_uuid PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.label_gpt_uuid.cppm)
target_link_libraries(pbsd_geom_label_gpt_uuid PUBLIC pbsd_core pbsd_geom_label)
target_compile_options(pbsd_geom_label_gpt_uuid PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_multipath_ctl)
add_library(pbsd_geom_multipath_ctl)
target_sources(pbsd_geom_multipath_ctl PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.multipath_ctl.cppm)
target_link_libraries(pbsd_geom_multipath_ctl PUBLIC pbsd_core pbsd_geom_multipath)
target_compile_options(pbsd_geom_multipath_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_cls)
add_library(pbsd_geom_raid_cls)
target_sources(pbsd_geom_raid_cls PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_cls.cppm)
target_link_libraries(pbsd_geom_raid_cls PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_cls PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_tr)
add_library(pbsd_geom_raid_tr)
target_sources(pbsd_geom_raid_tr PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_tr.cppm)
target_link_libraries(pbsd_geom_raid_tr PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_tr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_journal_core)
add_library(pbsd_geom_journal_core)
target_sources(pbsd_geom_journal_core PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.journal_core.cppm)
target_link_libraries(pbsd_geom_journal_core PUBLIC pbsd_core pbsd_geom_journal)
target_compile_options(pbsd_geom_journal_core PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_part_apm)
add_library(pbsd_geom_part_apm)
target_sources(pbsd_geom_part_apm PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part_apm.cppm)
target_link_libraries(pbsd_geom_part_apm PUBLIC pbsd_core pbsd_geom_part)
target_compile_options(pbsd_geom_part_apm PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_part_bsd)
add_library(pbsd_geom_part_bsd)
target_sources(pbsd_geom_part_bsd PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part_bsd.cppm)
target_link_libraries(pbsd_geom_part_bsd PUBLIC pbsd_core pbsd_geom_part)
target_compile_options(pbsd_geom_part_bsd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_tr_raid5)
add_library(pbsd_geom_raid_tr_raid5)
target_sources(pbsd_geom_raid_tr_raid5 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_tr_raid5.cppm)
target_link_libraries(pbsd_geom_raid_tr_raid5 PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_tr_raid5 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_uzip_lzma)
add_library(pbsd_geom_uzip_lzma)
target_sources(pbsd_geom_uzip_lzma PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.uzip_lzma.cppm)
target_link_libraries(pbsd_geom_uzip_lzma PUBLIC pbsd_core pbsd_geom_uzip)
target_compile_options(pbsd_geom_uzip_lzma PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_tr_raid0)
add_library(pbsd_geom_raid_tr_raid0)
target_sources(pbsd_geom_raid_tr_raid0 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_tr_raid0.cppm)
target_link_libraries(pbsd_geom_raid_tr_raid0 PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_tr_raid0 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_tr_raid1)
add_library(pbsd_geom_raid_tr_raid1)
target_sources(pbsd_geom_raid_tr_raid1 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_tr_raid1.cppm)
target_link_libraries(pbsd_geom_raid_tr_raid1 PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_tr_raid1 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_tr_raid1e)
add_library(pbsd_geom_raid_tr_raid1e)
target_sources(pbsd_geom_raid_tr_raid1e PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_tr_raid1e.cppm)
target_link_libraries(pbsd_geom_raid_tr_raid1e PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_tr_raid1e PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_tr_concat)
add_library(pbsd_geom_raid_tr_concat)
target_sources(pbsd_geom_raid_tr_concat PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_tr_concat.cppm)
target_link_libraries(pbsd_geom_raid_tr_concat PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_tr_concat PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_md_intel)
add_library(pbsd_geom_raid_md_intel)
target_sources(pbsd_geom_raid_md_intel PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_md_intel.cppm)
target_link_libraries(pbsd_geom_raid_md_intel PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_md_intel PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_md_ddf)
add_library(pbsd_geom_raid_md_ddf)
target_sources(pbsd_geom_raid_md_ddf PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_md_ddf.cppm)
target_link_libraries(pbsd_geom_raid_md_ddf PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_md_ddf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_md_nvidia)
add_library(pbsd_geom_raid_md_nvidia)
target_sources(pbsd_geom_raid_md_nvidia PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_md_nvidia.cppm)
target_link_libraries(pbsd_geom_raid_md_nvidia PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_md_nvidia PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_md_promise)
add_library(pbsd_geom_raid_md_promise)
target_sources(pbsd_geom_raid_md_promise PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_md_promise.cppm)
target_link_libraries(pbsd_geom_raid_md_promise PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_md_promise PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_md_sii)
add_library(pbsd_geom_raid_md_sii)
target_sources(pbsd_geom_raid_md_sii PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_md_sii.cppm)
target_link_libraries(pbsd_geom_raid_md_sii PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_md_sii PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid_md_jmicron)
add_library(pbsd_geom_raid_md_jmicron)
target_sources(pbsd_geom_raid_md_jmicron PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid_md_jmicron.cppm)
target_link_libraries(pbsd_geom_raid_md_jmicron PUBLIC pbsd_core pbsd_geom_raid)
target_compile_options(pbsd_geom_raid_md_jmicron PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_part_ldm)
add_library(pbsd_geom_part_ldm)
target_sources(pbsd_geom_part_ldm PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part_ldm.cppm)
target_link_libraries(pbsd_geom_part_ldm PUBLIC pbsd_core pbsd_geom_part)
target_compile_options(pbsd_geom_part_ldm PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_part_ebr)
add_library(pbsd_geom_part_ebr)
target_sources(pbsd_geom_part_ebr PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part_ebr.cppm)
target_link_libraries(pbsd_geom_part_ebr PUBLIC pbsd_core pbsd_geom_part)
target_compile_options(pbsd_geom_part_ebr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_part_bsd64)
add_library(pbsd_geom_part_bsd64)
target_sources(pbsd_geom_part_bsd64 PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.part_bsd64.cppm)
target_link_libraries(pbsd_geom_part_bsd64 PUBLIC pbsd_core pbsd_geom_part)
target_compile_options(pbsd_geom_part_bsd64 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_raid3_ctl)
add_library(pbsd_geom_raid3_ctl)
target_sources(pbsd_geom_raid3_ctl PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.raid3_ctl.cppm)
target_link_libraries(pbsd_geom_raid3_ctl PUBLIC pbsd_core pbsd_geom_raid3)
target_compile_options(pbsd_geom_raid3_ctl PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_uzip_zstd)
add_library(pbsd_geom_uzip_zstd)
target_sources(pbsd_geom_uzip_zstd PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.uzip_zstd.cppm)
target_link_libraries(pbsd_geom_uzip_zstd PUBLIC pbsd_core pbsd_geom_uzip)
target_compile_options(pbsd_geom_uzip_zstd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_uzip_zlib)
add_library(pbsd_geom_uzip_zlib)
target_sources(pbsd_geom_uzip_zlib PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.uzip_zlib.cppm)
target_link_libraries(pbsd_geom_uzip_zlib PUBLIC pbsd_core pbsd_geom_uzip)
target_compile_options(pbsd_geom_uzip_zlib PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_uzip_wrkthr)
add_library(pbsd_geom_uzip_wrkthr)
target_sources(pbsd_geom_uzip_wrkthr PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.uzip_wrkthr.cppm)
target_link_libraries(pbsd_geom_uzip_wrkthr PUBLIC pbsd_core pbsd_geom_uzip)
target_compile_options(pbsd_geom_uzip_wrkthr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom_virstor_md)
add_library(pbsd_geom_virstor_md)
target_sources(pbsd_geom_virstor_md PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.virstor_md.cppm)
target_link_libraries(pbsd_geom_virstor_md PUBLIC pbsd_core pbsd_geom_virstor)
target_compile_options(pbsd_geom_virstor_md PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_geom)
add_library(pbsd_geom)
target_sources(pbsd_geom PUBLIC FILE_SET CXX_MODULES FILES geom/pbsd.geom.cppm)
target_link_libraries(pbsd_geom PUBLIC pbsd_handles pbsd_geom_ctl pbsd_geom_disk
    pbsd_geom_slice pbsd_geom_flashmap
    pbsd_geom_part pbsd_geom_mirror pbsd_geom_label pbsd_geom_taste
    pbsd_geom_stripe pbsd_geom_concat pbsd_geom_eli
    pbsd_geom_raid3 pbsd_geom_journal pbsd_geom_gate pbsd_geom_multipath
    pbsd_geom_cache pbsd_geom_shsec pbsd_geom_virstor
    pbsd_geom_raid pbsd_geom_mountver pbsd_geom_nop pbsd_geom_zero
    pbsd_geom_uzip pbsd_geom_gunion pbsd_geom_shred
    pbsd_geom_linux_lvm pbsd_geom_subr pbsd_geom_io pbsd_geom_event pbsd_geom_vfs
    pbsd_geom_ccd pbsd_geom_bsd pbsd_geom_core pbsd_geom_bsdlabel
    pbsd_geom_dump pbsd_geom_up pbsd_geom_orphan
    pbsd_geom_part_gpt pbsd_geom_part_mbr pbsd_geom_eli_key pbsd_geom_journal_ufs
    pbsd_geom_mirror_ctl pbsd_geom_raid_ctl pbsd_geom_label_gpt pbsd_geom_label_ufs
    pbsd_geom_subr_disk pbsd_geom_eli_crypto pbsd_geom_eli_ctl pbsd_geom_eli_hmac pbsd_geom_eli_integrity pbsd_geom_eli_key_cache pbsd_geom_eli_privacy pbsd_geom_pkcs5v2 pbsd_geom_geom_kern pbsd_geom_geom_dev pbsd_geom_label_disk_ident pbsd_geom_label_ext2fs pbsd_geom_label_flashmap pbsd_geom_label_iso9660 pbsd_geom_label_msdosfs pbsd_geom_label_reiserfs pbsd_geom_label_ufs_id     pbsd_geom_geom_bsd_enc pbsd_geom_label_ntfs pbsd_geom_label_swap pbsd_geom_label_swaplinux pbsd_geom_label_vtoc8 pbsd_geom_label_gpt_uuid pbsd_geom_multipath_ctl pbsd_geom_raid_cls pbsd_geom_raid_tr pbsd_geom_journal_core pbsd_geom_part_apm pbsd_geom_part_bsd pbsd_geom_raid_tr_raid5 pbsd_geom_uzip_lzma pbsd_geom_raid_tr_raid0 pbsd_geom_raid_tr_raid1 pbsd_geom_raid_tr_raid1e pbsd_geom_raid_tr_concat pbsd_geom_raid_md_intel pbsd_geom_raid_md_ddf pbsd_geom_raid_md_nvidia pbsd_geom_raid_md_promise pbsd_geom_raid_md_sii pbsd_geom_raid_md_jmicron pbsd_geom_part_ldm pbsd_geom_part_ebr pbsd_geom_part_bsd64 pbsd_geom_raid3_ctl pbsd_geom_uzip_zstd pbsd_geom_uzip_zlib pbsd_geom_uzip_wrkthr pbsd_geom_virstor_md)
target_compile_options(pbsd_geom PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_zfs_ioctl)
target_sources(pbsd_zfs_ioctl PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.ioctl.cppm)
target_link_libraries(pbsd_zfs_ioctl PUBLIC pbsd_core pbsd_rights)
target_compile_options(pbsd_zfs_ioctl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_features)
target_sources(pbsd_zfs_features PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.features.cppm)
target_link_libraries(pbsd_zfs_features PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_features PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_props)
target_sources(pbsd_zfs_props PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.props.cppm)
target_link_libraries(pbsd_zfs_props PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_props PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_zio)
target_sources(pbsd_zfs_zio PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zio.cppm)
target_link_libraries(pbsd_zfs_zio PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_zio PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_spa)
target_sources(pbsd_zfs_spa PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.spa.cppm)
target_link_libraries(pbsd_zfs_spa PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_spa PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_dmu)
target_sources(pbsd_zfs_dmu PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu.cppm)
target_link_libraries(pbsd_zfs_dmu PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_dmu PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_dnode)
target_sources(pbsd_zfs_dnode PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dnode.cppm)
target_link_libraries(pbsd_zfs_dnode PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_dnode PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_vdev)
target_sources(pbsd_zfs_vdev PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vdev.cppm)
target_link_libraries(pbsd_zfs_vdev PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_vdev PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_arc)
target_sources(pbsd_zfs_arc PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.arc.cppm)
target_link_libraries(pbsd_zfs_arc PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_arc PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_dsl)
target_sources(pbsd_zfs_dsl PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl.cppm)
target_link_libraries(pbsd_zfs_dsl PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_dsl PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_zio_checksum)
target_sources(pbsd_zfs_zio_checksum PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zio_checksum.cppm)
target_link_libraries(pbsd_zfs_zio_checksum PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_zio_checksum PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_zap)
target_sources(pbsd_zfs_zap PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zap.cppm)
target_link_libraries(pbsd_zfs_zap PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_zap PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_zfs_metaslab)
add_library(pbsd_zfs_metaslab)
target_sources(pbsd_zfs_metaslab PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.metaslab.cppm)
target_link_libraries(pbsd_zfs_metaslab PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_metaslab PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_zfs_zil)
target_sources(pbsd_zfs_zil PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zil.cppm)
target_link_libraries(pbsd_zfs_zil PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_zil PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_l2arc)
target_sources(pbsd_zfs_l2arc PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.l2arc.cppm)
target_link_libraries(pbsd_zfs_l2arc PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_l2arc PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_zio_flag)
target_sources(pbsd_zfs_zio_flag PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zio_flag.cppm)
target_link_libraries(pbsd_zfs_zio_flag PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_zio_flag PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_zfs_compress)
target_sources(pbsd_zfs_compress PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.compress.cppm)
target_link_libraries(pbsd_zfs_compress PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_compress PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_zfs_dbuf)
add_library(pbsd_zfs_dbuf)
target_sources(pbsd_zfs_dbuf PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dbuf.cppm)
target_link_libraries(pbsd_zfs_dbuf PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_dbuf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_sa)
add_library(pbsd_zfs_sa)
target_sources(pbsd_zfs_sa PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.sa.cppm)
target_link_libraries(pbsd_zfs_sa PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_sa PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_zfetch)
add_library(pbsd_zfs_zfetch)
target_sources(pbsd_zfs_zfetch PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zfetch.cppm)
target_link_libraries(pbsd_zfs_zfetch PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_zfetch PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_txg)
add_library(pbsd_zfs_txg)
target_sources(pbsd_zfs_txg PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.txg.cppm)
target_link_libraries(pbsd_zfs_txg PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_txg PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_uberblock)
add_library(pbsd_zfs_uberblock)
target_sources(pbsd_zfs_uberblock PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.uberblock.cppm)
target_link_libraries(pbsd_zfs_uberblock PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_uberblock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_space_map)
add_library(pbsd_zfs_space_map)
target_sources(pbsd_zfs_space_map PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.space_map.cppm)
target_link_libraries(pbsd_zfs_space_map PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_space_map PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_vdev_queue)
add_library(pbsd_zfs_vdev_queue)
target_sources(pbsd_zfs_vdev_queue PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vdev_queue.cppm)
target_link_libraries(pbsd_zfs_vdev_queue PUBLIC pbsd_core pbsd_zfs_vdev)
target_compile_options(pbsd_zfs_vdev_queue PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_spa_load)
add_library(pbsd_zfs_spa_load)
target_sources(pbsd_zfs_spa_load PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.spa_load.cppm)
target_link_libraries(pbsd_zfs_spa_load PUBLIC pbsd_core pbsd_zfs_spa)
target_compile_options(pbsd_zfs_spa_load PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_pool)
add_library(pbsd_zfs_dsl_pool)
target_sources(pbsd_zfs_dsl_pool PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_pool.cppm)
target_link_libraries(pbsd_zfs_dsl_pool PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_dsl_pool PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_dir)
add_library(pbsd_zfs_dsl_dir)
target_sources(pbsd_zfs_dsl_dir PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_dir.cppm)
target_link_libraries(pbsd_zfs_dsl_dir PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_dir PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_dataset)
add_library(pbsd_zfs_dsl_dataset)
target_sources(pbsd_zfs_dsl_dataset PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_dataset.cppm)
target_link_libraries(pbsd_zfs_dsl_dataset PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_dataset PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_vdev_label)
add_library(pbsd_zfs_vdev_label)
target_sources(pbsd_zfs_vdev_label PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vdev_label.cppm)
target_link_libraries(pbsd_zfs_vdev_label PUBLIC pbsd_core pbsd_zfs_vdev)
target_compile_options(pbsd_zfs_vdev_label PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_zil_header)
add_library(pbsd_zfs_zil_header)
target_sources(pbsd_zfs_zil_header PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zil_header.cppm)
target_link_libraries(pbsd_zfs_zil_header PUBLIC pbsd_core pbsd_zfs_zil)
target_compile_options(pbsd_zfs_zil_header PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_vnops)
add_library(pbsd_zfs_vnops)
target_sources(pbsd_zfs_vnops PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vnops.cppm)
target_link_libraries(pbsd_zfs_vnops PUBLIC pbsd_core pbsd_fs)
target_compile_options(pbsd_zfs_vnops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_vfsops)
add_library(pbsd_zfs_vfsops)
target_sources(pbsd_zfs_vfsops PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vfsops.cppm)
target_link_libraries(pbsd_zfs_vfsops PUBLIC pbsd_core pbsd_fs)
target_compile_options(pbsd_zfs_vfsops PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_scan)
add_library(pbsd_zfs_dsl_scan)
target_sources(pbsd_zfs_dsl_scan PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_scan.cppm)
target_link_libraries(pbsd_zfs_dsl_scan PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_scan PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_arc_stats)
add_library(pbsd_zfs_arc_stats)
target_sources(pbsd_zfs_arc_stats PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.arc_stats.cppm)
target_link_libraries(pbsd_zfs_arc_stats PUBLIC pbsd_core pbsd_zfs_arc)
target_compile_options(pbsd_zfs_arc_stats PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_zio_inject)
add_library(pbsd_zfs_zio_inject)
target_sources(pbsd_zfs_zio_inject PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zio_inject.cppm)
target_link_libraries(pbsd_zfs_zio_inject PUBLIC pbsd_core pbsd_zfs_zio)
target_compile_options(pbsd_zfs_zio_inject PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_fuid)
add_library(pbsd_zfs_fuid)
target_sources(pbsd_zfs_fuid PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.fuid.cppm)
target_link_libraries(pbsd_zfs_fuid PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_fuid PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_blkptr)
add_library(pbsd_zfs_blkptr)
target_sources(pbsd_zfs_blkptr PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.blkptr.cppm)
target_link_libraries(pbsd_zfs_blkptr PUBLIC pbsd_core pbsd_zfs_zio)
target_compile_options(pbsd_zfs_blkptr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_object)
add_library(pbsd_zfs_dmu_object)
target_sources(pbsd_zfs_dmu_object PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_object.cppm)
target_link_libraries(pbsd_zfs_dmu_object PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_object PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_tx)
add_library(pbsd_zfs_dmu_tx)
target_sources(pbsd_zfs_dmu_tx PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_tx.cppm)
target_link_libraries(pbsd_zfs_dmu_tx PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_tx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dnode_sync)
add_library(pbsd_zfs_dnode_sync)
target_sources(pbsd_zfs_dnode_sync PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dnode_sync.cppm)
target_link_libraries(pbsd_zfs_dnode_sync PUBLIC pbsd_core pbsd_zfs_dnode)
target_compile_options(pbsd_zfs_dnode_sync PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_spa_config)
add_library(pbsd_zfs_spa_config)
target_sources(pbsd_zfs_spa_config PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.spa_config.cppm)
target_link_libraries(pbsd_zfs_spa_config PUBLIC pbsd_core pbsd_zfs_spa)
target_compile_options(pbsd_zfs_spa_config PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_vdev_mirror)
add_library(pbsd_zfs_vdev_mirror)
target_sources(pbsd_zfs_vdev_mirror PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vdev_mirror.cppm)
target_link_libraries(pbsd_zfs_vdev_mirror PUBLIC pbsd_core pbsd_zfs_vdev)
target_compile_options(pbsd_zfs_vdev_mirror PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_bpobj)
add_library(pbsd_zfs_bpobj)
target_sources(pbsd_zfs_bpobj PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.bpobj.cppm)
target_link_libraries(pbsd_zfs_bpobj PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_bpobj PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_ddt_zap)
add_library(pbsd_zfs_ddt_zap)
target_sources(pbsd_zfs_ddt_zap PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.ddt_zap.cppm)
target_link_libraries(pbsd_zfs_ddt_zap PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_ddt_zap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_deadlist)
add_library(pbsd_zfs_dsl_deadlist)
target_sources(pbsd_zfs_dsl_deadlist PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_deadlist.cppm)
target_link_libraries(pbsd_zfs_dsl_deadlist PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_deadlist PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_arc_evict)
add_library(pbsd_zfs_arc_evict)
target_sources(pbsd_zfs_arc_evict PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.arc_evict.cppm)
target_link_libraries(pbsd_zfs_arc_evict PUBLIC pbsd_core pbsd_zfs_arc)
target_compile_options(pbsd_zfs_arc_evict PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_abd)
add_library(pbsd_zfs_abd)
target_sources(pbsd_zfs_abd PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.abd.cppm)
target_link_libraries(pbsd_zfs_abd PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_abd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_aggsum)
add_library(pbsd_zfs_aggsum)
target_sources(pbsd_zfs_aggsum PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.aggsum.cppm)
target_link_libraries(pbsd_zfs_aggsum PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_aggsum PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_bplist)
add_library(pbsd_zfs_bplist)
target_sources(pbsd_zfs_bplist PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.bplist.cppm)
target_link_libraries(pbsd_zfs_bplist PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_bplist PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_bptree)
add_library(pbsd_zfs_bptree)
target_sources(pbsd_zfs_bptree PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.bptree.cppm)
target_link_libraries(pbsd_zfs_bptree PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_bptree PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_ddt)
add_library(pbsd_zfs_ddt)
target_sources(pbsd_zfs_ddt PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.ddt.cppm)
target_link_libraries(pbsd_zfs_ddt PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_ddt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_ddt_log)
add_library(pbsd_zfs_ddt_log)
target_sources(pbsd_zfs_ddt_log PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.ddt_log.cppm)
target_link_libraries(pbsd_zfs_ddt_log PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_ddt_log PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_ddt_stats)
add_library(pbsd_zfs_ddt_stats)
target_sources(pbsd_zfs_ddt_stats PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.ddt_stats.cppm)
target_link_libraries(pbsd_zfs_ddt_stats PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_ddt_stats PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_objset)
add_library(pbsd_zfs_dmu_objset)
target_sources(pbsd_zfs_dmu_objset PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_objset.cppm)
target_link_libraries(pbsd_zfs_dmu_objset PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_objset PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_recv)
add_library(pbsd_zfs_dmu_recv)
target_sources(pbsd_zfs_dmu_recv PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_recv.cppm)
target_link_libraries(pbsd_zfs_dmu_recv PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_recv PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_send)
add_library(pbsd_zfs_dmu_send)
target_sources(pbsd_zfs_dmu_send PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_send.cppm)
target_link_libraries(pbsd_zfs_dmu_send PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_send PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_traverse)
add_library(pbsd_zfs_dmu_traverse)
target_sources(pbsd_zfs_dmu_traverse PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_traverse.cppm)
target_link_libraries(pbsd_zfs_dmu_traverse PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_traverse PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_zfetch)
add_library(pbsd_zfs_dmu_zfetch)
target_sources(pbsd_zfs_dmu_zfetch PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_zfetch.cppm)
target_link_libraries(pbsd_zfs_dmu_zfetch PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_zfetch PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_bookmark)
add_library(pbsd_zfs_dsl_bookmark)
target_sources(pbsd_zfs_dsl_bookmark PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_bookmark.cppm)
target_link_libraries(pbsd_zfs_dsl_bookmark PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_bookmark PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_crypt)
add_library(pbsd_zfs_dsl_crypt)
target_sources(pbsd_zfs_dsl_crypt PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_crypt.cppm)
target_link_libraries(pbsd_zfs_dsl_crypt PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_crypt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_destroy)
add_library(pbsd_zfs_dsl_destroy)
target_sources(pbsd_zfs_dsl_destroy PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_destroy.cppm)
target_link_libraries(pbsd_zfs_dsl_destroy PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_destroy PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_prop)
add_library(pbsd_zfs_dsl_prop)
target_sources(pbsd_zfs_dsl_prop PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_prop.cppm)
target_link_libraries(pbsd_zfs_dsl_prop PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_prop PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dataset_kstats)
add_library(pbsd_zfs_dataset_kstats)
target_sources(pbsd_zfs_dataset_kstats PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dataset_kstats.cppm)
target_link_libraries(pbsd_zfs_dataset_kstats PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dataset_kstats PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dbuf_stats)
add_library(pbsd_zfs_dbuf_stats)
target_sources(pbsd_zfs_dbuf_stats PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dbuf_stats.cppm)
target_link_libraries(pbsd_zfs_dbuf_stats PUBLIC pbsd_core pbsd_zfs_dbuf)
target_compile_options(pbsd_zfs_dbuf_stats PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_spa_misc)
add_library(pbsd_zfs_spa_misc)
target_sources(pbsd_zfs_spa_misc PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.spa_misc.cppm)
target_link_libraries(pbsd_zfs_spa_misc PUBLIC pbsd_core pbsd_zfs_spa)
target_compile_options(pbsd_zfs_spa_misc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_spa_errlog)
add_library(pbsd_zfs_spa_errlog)
target_sources(pbsd_zfs_spa_errlog PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.spa_errlog.cppm)
target_link_libraries(pbsd_zfs_spa_errlog PUBLIC pbsd_core pbsd_zfs_spa)
target_compile_options(pbsd_zfs_spa_errlog PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_blake3_zfs)
add_library(pbsd_zfs_blake3_zfs)
target_sources(pbsd_zfs_blake3_zfs PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.blake3_zfs.cppm)
target_link_libraries(pbsd_zfs_blake3_zfs PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_blake3_zfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_bqueue)
add_library(pbsd_zfs_bqueue)
target_sources(pbsd_zfs_bqueue PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.bqueue.cppm)
target_link_libraries(pbsd_zfs_bqueue PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_bqueue PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_brt)
add_library(pbsd_zfs_brt)
target_sources(pbsd_zfs_brt PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.brt.cppm)
target_link_libraries(pbsd_zfs_brt PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_brt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_btree)
add_library(pbsd_zfs_btree)
target_sources(pbsd_zfs_btree PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.btree.cppm)
target_link_libraries(pbsd_zfs_btree PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_btree PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_diff)
add_library(pbsd_zfs_dmu_diff)
target_sources(pbsd_zfs_dmu_diff PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_diff.cppm)
target_link_libraries(pbsd_zfs_dmu_diff PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_diff PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_direct)
add_library(pbsd_zfs_dmu_direct)
target_sources(pbsd_zfs_dmu_direct PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_direct.cppm)
target_link_libraries(pbsd_zfs_dmu_direct PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_direct PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dmu_redact)
add_library(pbsd_zfs_dmu_redact)
target_sources(pbsd_zfs_dmu_redact PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dmu_redact.cppm)
target_link_libraries(pbsd_zfs_dmu_redact PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_dmu_redact PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_deleg)
add_library(pbsd_zfs_dsl_deleg)
target_sources(pbsd_zfs_dsl_deleg PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_deleg.cppm)
target_link_libraries(pbsd_zfs_dsl_deleg PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_deleg PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_synctask)
add_library(pbsd_zfs_dsl_synctask)
target_sources(pbsd_zfs_dsl_synctask PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_synctask.cppm)
target_link_libraries(pbsd_zfs_dsl_synctask PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_synctask PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_dsl_userhold)
add_library(pbsd_zfs_dsl_userhold)
target_sources(pbsd_zfs_dsl_userhold PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.dsl_userhold.cppm)
target_link_libraries(pbsd_zfs_dsl_userhold PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_dsl_userhold PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_edonr_zfs)
add_library(pbsd_zfs_edonr_zfs)
target_sources(pbsd_zfs_edonr_zfs PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.edonr_zfs.cppm)
target_link_libraries(pbsd_zfs_edonr_zfs PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_edonr_zfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_fm)
add_library(pbsd_zfs_fm)
target_sources(pbsd_zfs_fm PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.fm.cppm)
target_link_libraries(pbsd_zfs_fm PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_fm PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_gzip)
add_library(pbsd_zfs_gzip)
target_sources(pbsd_zfs_gzip PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.gzip.cppm)
target_link_libraries(pbsd_zfs_gzip PUBLIC pbsd_core pbsd_zfs_compress)
target_compile_options(pbsd_zfs_gzip PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_hkdf)
add_library(pbsd_zfs_hkdf)
target_sources(pbsd_zfs_hkdf PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.hkdf.cppm)
target_link_libraries(pbsd_zfs_hkdf PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_hkdf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_lz4)
add_library(pbsd_zfs_lz4)
target_sources(pbsd_zfs_lz4 PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.lz4.cppm)
target_link_libraries(pbsd_zfs_lz4 PUBLIC pbsd_core pbsd_zfs_compress)
target_compile_options(pbsd_zfs_lz4 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_lz4_zfs)
add_library(pbsd_zfs_lz4_zfs)
target_sources(pbsd_zfs_lz4_zfs PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.lz4_zfs.cppm)
target_link_libraries(pbsd_zfs_lz4_zfs PUBLIC pbsd_core pbsd_zfs_compress)
target_compile_options(pbsd_zfs_lz4_zfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_lzjb)
add_library(pbsd_zfs_lzjb)
target_sources(pbsd_zfs_lzjb PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.lzjb.cppm)
target_link_libraries(pbsd_zfs_lzjb PUBLIC pbsd_core pbsd_zfs_compress)
target_compile_options(pbsd_zfs_lzjb PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_mmp)
add_library(pbsd_zfs_mmp)
target_sources(pbsd_zfs_mmp PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.mmp.cppm)
target_link_libraries(pbsd_zfs_mmp PUBLIC pbsd_core pbsd_zfs_spa)
target_compile_options(pbsd_zfs_mmp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_multilist)
add_library(pbsd_zfs_multilist)
target_sources(pbsd_zfs_multilist PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.multilist.cppm)
target_link_libraries(pbsd_zfs_multilist PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_multilist PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_objlist)
add_library(pbsd_zfs_objlist)
target_sources(pbsd_zfs_objlist PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.objlist.cppm)
target_link_libraries(pbsd_zfs_objlist PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_objlist PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_pathname)
add_library(pbsd_zfs_pathname)
target_sources(pbsd_zfs_pathname PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.pathname.cppm)
target_link_libraries(pbsd_zfs_pathname PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_pathname PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_range_tree)
add_library(pbsd_zfs_range_tree)
target_sources(pbsd_zfs_range_tree PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.range_tree.cppm)
target_link_libraries(pbsd_zfs_range_tree PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_range_tree PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_sha256_zfs)
add_library(pbsd_zfs_sha256_zfs)
target_sources(pbsd_zfs_sha256_zfs PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.sha256_zfs.cppm)
target_link_libraries(pbsd_zfs_sha256_zfs PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_sha256_zfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_refcount)
add_library(pbsd_zfs_refcount)
target_sources(pbsd_zfs_refcount PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.refcount.cppm)
target_link_libraries(pbsd_zfs_refcount PUBLIC pbsd_core)
target_compile_options(pbsd_zfs_refcount PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_vdev_raidz)
add_library(pbsd_zfs_vdev_raidz)
target_sources(pbsd_zfs_vdev_raidz PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vdev_raidz.cppm)
target_link_libraries(pbsd_zfs_vdev_raidz PUBLIC pbsd_core pbsd_zfs_vdev)
target_compile_options(pbsd_zfs_vdev_raidz PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_zvol)
add_library(pbsd_zfs_zvol)
target_sources(pbsd_zfs_zvol PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zvol.cppm)
target_link_libraries(pbsd_zfs_zvol PUBLIC pbsd_core pbsd_zfs_dmu)
target_compile_options(pbsd_zfs_zvol PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_zap_leaf)
add_library(pbsd_zfs_zap_leaf)
target_sources(pbsd_zfs_zap_leaf PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zap_leaf.cppm)
target_link_libraries(pbsd_zfs_zap_leaf PUBLIC pbsd_core pbsd_zfs_zap)
target_compile_options(pbsd_zfs_zap_leaf PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_zcp)
add_library(pbsd_zfs_zcp)
target_sources(pbsd_zfs_zcp PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.zcp.cppm)
target_link_libraries(pbsd_zfs_zcp PUBLIC pbsd_core pbsd_zfs_dsl)
target_compile_options(pbsd_zfs_zcp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_spa_checkpoint)
add_library(pbsd_zfs_spa_checkpoint)
target_sources(pbsd_zfs_spa_checkpoint PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.spa_checkpoint.cppm)
target_link_libraries(pbsd_zfs_spa_checkpoint PUBLIC pbsd_core pbsd_zfs_spa)
target_compile_options(pbsd_zfs_spa_checkpoint PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs_vdev_file)
add_library(pbsd_zfs_vdev_file)
target_sources(pbsd_zfs_vdev_file PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.vdev_file.cppm)
target_link_libraries(pbsd_zfs_vdev_file PUBLIC pbsd_core pbsd_zfs_vdev)
target_compile_options(pbsd_zfs_vdev_file PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_zfs)
add_library(pbsd_zfs)
target_sources(pbsd_zfs PUBLIC FILE_SET CXX_MODULES FILES zfs/pbsd.zfs.cppm)
target_link_libraries(pbsd_zfs PUBLIC pbsd_fs pbsd_handles pbsd_zfs_ioctl pbsd_zfs_features
    pbsd_zfs_props pbsd_zfs_zio pbsd_zfs_spa
    pbsd_zfs_dmu pbsd_zfs_dnode pbsd_zfs_vdev pbsd_zfs_arc pbsd_zfs_dsl
    pbsd_zfs_zio_checksum pbsd_zfs_zap pbsd_zfs_metaslab pbsd_zfs_zil pbsd_zfs_l2arc
    pbsd_zfs_zio_flag pbsd_zfs_compress
    pbsd_zfs_dbuf pbsd_zfs_sa pbsd_zfs_zfetch
    pbsd_zfs_txg pbsd_zfs_uberblock pbsd_zfs_space_map pbsd_zfs_vdev_queue
    pbsd_zfs_spa_load pbsd_zfs_dsl_pool
    pbsd_zfs_dsl_dir pbsd_zfs_dsl_dataset pbsd_zfs_vdev_label pbsd_zfs_zil_header
    pbsd_zfs_vnops pbsd_zfs_vfsops pbsd_zfs_dsl_scan pbsd_zfs_arc_stats
    pbsd_zfs_zio_inject pbsd_zfs_fuid
    pbsd_zfs_blkptr pbsd_zfs_dmu_object pbsd_zfs_dmu_tx pbsd_zfs_dnode_sync
    pbsd_zfs_spa_config pbsd_zfs_vdev_mirror pbsd_zfs_bpobj pbsd_zfs_ddt_zap
    pbsd_zfs_dsl_deadlist pbsd_zfs_arc_evict pbsd_zfs_abd pbsd_zfs_aggsum pbsd_zfs_bplist pbsd_zfs_bptree pbsd_zfs_ddt pbsd_zfs_ddt_log pbsd_zfs_ddt_stats pbsd_zfs_dmu_objset pbsd_zfs_dmu_recv pbsd_zfs_dmu_send pbsd_zfs_dmu_traverse pbsd_zfs_dmu_zfetch pbsd_zfs_dsl_bookmark pbsd_zfs_dsl_crypt pbsd_zfs_dsl_destroy pbsd_zfs_dsl_prop pbsd_zfs_dataset_kstats pbsd_zfs_dbuf_stats pbsd_zfs_spa_misc pbsd_zfs_spa_errlog pbsd_zfs_blake3_zfs pbsd_zfs_bqueue pbsd_zfs_brt pbsd_zfs_btree pbsd_zfs_dmu_diff pbsd_zfs_dmu_direct pbsd_zfs_dmu_redact pbsd_zfs_dsl_deleg pbsd_zfs_dsl_synctask pbsd_zfs_dsl_userhold pbsd_zfs_edonr_zfs pbsd_zfs_fm pbsd_zfs_gzip pbsd_zfs_hkdf pbsd_zfs_lz4 pbsd_zfs_lz4_zfs pbsd_zfs_lzjb pbsd_zfs_mmp pbsd_zfs_multilist     pbsd_zfs_objlist pbsd_zfs_pathname pbsd_zfs_range_tree pbsd_zfs_sha256_zfs pbsd_zfs_refcount pbsd_zfs_vdev_raidz pbsd_zfs_zvol pbsd_zfs_zap_leaf pbsd_zfs_zcp pbsd_zfs_spa_checkpoint pbsd_zfs_vdev_file)
target_compile_options(pbsd_zfs PUBLIC ${PBSD_FS_CXX})
endif()

# --- Wave 7: stand / arch ----------------------------------------------------
add_library(pbsd_stand_pe)
target_sources(pbsd_stand_pe PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.pe.cppm)
target_link_libraries(pbsd_stand_pe PUBLIC pbsd_core)
target_compile_options(pbsd_stand_pe PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_efi)
target_sources(pbsd_stand_efi PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.efi.cppm)
target_link_libraries(pbsd_stand_efi PUBLIC pbsd_core)
target_compile_options(pbsd_stand_efi PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_efi_protocols)
target_sources(pbsd_stand_efi_protocols PUBLIC FILE_SET CXX_MODULES FILES
    stand/pbsd.stand.efi.protocols.cppm)
target_link_libraries(pbsd_stand_efi_protocols PUBLIC pbsd_core)
target_compile_options(pbsd_stand_efi_protocols PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_common)
target_sources(pbsd_stand_common PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.common.cppm)
target_link_libraries(pbsd_stand_common PUBLIC pbsd_core)
target_compile_options(pbsd_stand_common PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_bootinfo)
target_sources(pbsd_stand_bootinfo PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.bootinfo.cppm)
target_link_libraries(pbsd_stand_bootinfo PUBLIC pbsd_core)
target_compile_options(pbsd_stand_bootinfo PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_loader)
target_sources(pbsd_stand_loader PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.loader.cppm)
target_link_libraries(pbsd_stand_loader PUBLIC pbsd_core)
target_compile_options(pbsd_stand_loader PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_reboot)
target_sources(pbsd_stand_reboot PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.reboot.cppm)
target_link_libraries(pbsd_stand_reboot PUBLIC pbsd_core)
target_compile_options(pbsd_stand_reboot PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_gpt)
target_sources(pbsd_stand_gpt PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.gpt.cppm)
target_link_libraries(pbsd_stand_gpt PUBLIC pbsd_core)
target_compile_options(pbsd_stand_gpt PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_kmod)
target_sources(pbsd_stand_kmod PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.kmod.cppm)
target_link_libraries(pbsd_stand_kmod PUBLIC pbsd_core)
target_compile_options(pbsd_stand_kmod PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_elf)
target_sources(pbsd_stand_elf PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.elf.cppm)
target_link_libraries(pbsd_stand_elf PUBLIC pbsd_core)
target_compile_options(pbsd_stand_elf PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_env)
target_sources(pbsd_stand_env PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.env.cppm)
target_link_libraries(pbsd_stand_env PUBLIC pbsd_core)
target_compile_options(pbsd_stand_env PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_devinfo)
target_sources(pbsd_stand_devinfo PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.devinfo.cppm)
target_link_libraries(pbsd_stand_devinfo PUBLIC pbsd_core)
target_compile_options(pbsd_stand_devinfo PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_zfsboot)
target_sources(pbsd_stand_zfsboot PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.zfsboot.cppm)
target_link_libraries(pbsd_stand_zfsboot PUBLIC pbsd_core)
target_compile_options(pbsd_stand_zfsboot PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_stand_net)
target_sources(pbsd_stand_net PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.net.cppm)
target_link_libraries(pbsd_stand_net PUBLIC pbsd_core)
target_compile_options(pbsd_stand_net PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_stand_bios)
add_library(pbsd_stand_bios)
target_sources(pbsd_stand_bios PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.bios.cppm)
target_link_libraries(pbsd_stand_bios PUBLIC pbsd_core)
target_compile_options(pbsd_stand_bios PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_ufs)
add_library(pbsd_stand_ufs)
target_sources(pbsd_stand_ufs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.ufs.cppm)
target_link_libraries(pbsd_stand_ufs PUBLIC pbsd_core)
target_compile_options(pbsd_stand_ufs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_iscsi)
add_library(pbsd_stand_iscsi)
target_sources(pbsd_stand_iscsi PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.iscsi.cppm)
target_link_libraries(pbsd_stand_iscsi PUBLIC pbsd_core)
target_compile_options(pbsd_stand_iscsi PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_cd9660)
add_library(pbsd_stand_cd9660)
target_sources(pbsd_stand_cd9660 PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.cd9660.cppm)
target_link_libraries(pbsd_stand_cd9660 PUBLIC pbsd_core)
target_compile_options(pbsd_stand_cd9660 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_cdboot)
add_library(pbsd_stand_cdboot)
target_sources(pbsd_stand_cdboot PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.cdboot.cppm)
target_link_libraries(pbsd_stand_cdboot PUBLIC pbsd_core pbsd_stand_cd9660)
target_compile_options(pbsd_stand_cdboot PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_pxe)
add_library(pbsd_stand_pxe)
target_sources(pbsd_stand_pxe PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.pxe.cppm)
target_link_libraries(pbsd_stand_pxe PUBLIC pbsd_core pbsd_stand_net)
target_compile_options(pbsd_stand_pxe PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_gzip)
add_library(pbsd_stand_gzip)
target_sources(pbsd_stand_gzip PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.gzip.cppm)
target_link_libraries(pbsd_stand_gzip PUBLIC pbsd_core)
target_compile_options(pbsd_stand_gzip PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_lz4)
add_library(pbsd_stand_lz4)
target_sources(pbsd_stand_lz4 PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.lz4.cppm)
target_link_libraries(pbsd_stand_lz4 PUBLIC pbsd_core)
target_compile_options(pbsd_stand_lz4 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_bootp)
add_library(pbsd_stand_bootp)
target_sources(pbsd_stand_bootp PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.bootp.cppm)
target_link_libraries(pbsd_stand_bootp PUBLIC pbsd_core pbsd_stand_net)
target_compile_options(pbsd_stand_bootp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_disk)
add_library(pbsd_stand_disk)
target_sources(pbsd_stand_disk PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.disk.cppm)
target_link_libraries(pbsd_stand_disk PUBLIC pbsd_core)
target_compile_options(pbsd_stand_disk PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_bcache)
add_library(pbsd_stand_bcache)
target_sources(pbsd_stand_bcache PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.bcache.cppm)
target_link_libraries(pbsd_stand_bcache PUBLIC pbsd_core)
target_compile_options(pbsd_stand_bcache PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_modload)
add_library(pbsd_stand_modload)
target_sources(pbsd_stand_modload PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.modload.cppm)
target_link_libraries(pbsd_stand_modload PUBLIC pbsd_core)
target_compile_options(pbsd_stand_modload PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_multiboot)
add_library(pbsd_stand_multiboot)
target_sources(pbsd_stand_multiboot PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.multiboot.cppm)
target_link_libraries(pbsd_stand_multiboot PUBLIC pbsd_core)
target_compile_options(pbsd_stand_multiboot PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_geli)
add_library(pbsd_stand_geli)
target_sources(pbsd_stand_geli PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.geli.cppm)
target_link_libraries(pbsd_stand_geli PUBLIC pbsd_core)
target_compile_options(pbsd_stand_geli PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_tftp)
add_library(pbsd_stand_tftp)
target_sources(pbsd_stand_tftp PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.tftp.cppm)
target_link_libraries(pbsd_stand_tftp PUBLIC pbsd_core pbsd_stand_net)
target_compile_options(pbsd_stand_tftp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_part)
add_library(pbsd_stand_part)
target_sources(pbsd_stand_part PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.part.cppm)
target_link_libraries(pbsd_stand_part PUBLIC pbsd_core)
target_compile_options(pbsd_stand_part PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_nfs)
add_library(pbsd_stand_nfs)
target_sources(pbsd_stand_nfs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.nfs.cppm)
target_link_libraries(pbsd_stand_nfs PUBLIC pbsd_core pbsd_stand_net)
target_compile_options(pbsd_stand_nfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_pkgfs)
add_library(pbsd_stand_pkgfs)
target_sources(pbsd_stand_pkgfs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.pkgfs.cppm)
target_link_libraries(pbsd_stand_pkgfs PUBLIC pbsd_core)
target_compile_options(pbsd_stand_pkgfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_dosfs)
add_library(pbsd_stand_dosfs)
target_sources(pbsd_stand_dosfs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.dosfs.cppm)
target_link_libraries(pbsd_stand_dosfs PUBLIC pbsd_core)
target_compile_options(pbsd_stand_dosfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_ext2fs)
add_library(pbsd_stand_ext2fs)
target_sources(pbsd_stand_ext2fs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.ext2fs.cppm)
target_link_libraries(pbsd_stand_ext2fs PUBLIC pbsd_core)
target_compile_options(pbsd_stand_ext2fs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_preload)
add_library(pbsd_stand_preload)
target_sources(pbsd_stand_preload PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.preload.cppm)
target_link_libraries(pbsd_stand_preload PUBLIC pbsd_core)
target_compile_options(pbsd_stand_preload PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_ffs)
add_library(pbsd_stand_ffs)
target_sources(pbsd_stand_ffs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.ffs.cppm)
target_link_libraries(pbsd_stand_ffs PUBLIC pbsd_core)
target_compile_options(pbsd_stand_ffs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_zstd)
add_library(pbsd_stand_zstd)
target_sources(pbsd_stand_zstd PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.zstd.cppm)
target_link_libraries(pbsd_stand_zstd PUBLIC pbsd_core)
target_compile_options(pbsd_stand_zstd PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_smbfs)
add_library(pbsd_stand_smbfs)
target_sources(pbsd_stand_smbfs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.smbfs.cppm)
target_link_libraries(pbsd_stand_smbfs PUBLIC pbsd_core)
target_compile_options(pbsd_stand_smbfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_lua)
add_library(pbsd_stand_lua)
target_sources(pbsd_stand_lua PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.lua.cppm)
target_link_libraries(pbsd_stand_lua PUBLIC pbsd_core)
target_compile_options(pbsd_stand_lua PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_nullfs)
add_library(pbsd_stand_nullfs)
target_sources(pbsd_stand_nullfs PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.nullfs.cppm)
target_link_libraries(pbsd_stand_nullfs PUBLIC pbsd_core)
target_compile_options(pbsd_stand_nullfs PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_verify)
add_library(pbsd_stand_verify)
target_sources(pbsd_stand_verify PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.verify.cppm)
target_link_libraries(pbsd_stand_verify PUBLIC pbsd_core)
target_compile_options(pbsd_stand_verify PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_hash)
add_library(pbsd_stand_hash)
target_sources(pbsd_stand_hash PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.hash.cppm)
target_link_libraries(pbsd_stand_hash PUBLIC pbsd_core)
target_compile_options(pbsd_stand_hash PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_uuid)
add_library(pbsd_stand_uuid)
target_sources(pbsd_stand_uuid PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.uuid.cppm)
target_link_libraries(pbsd_stand_uuid PUBLIC pbsd_core)
target_compile_options(pbsd_stand_uuid PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_install)
add_library(pbsd_stand_install)
target_sources(pbsd_stand_install PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.install.cppm)
target_link_libraries(pbsd_stand_install PUBLIC pbsd_core)
target_compile_options(pbsd_stand_install PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_decompress)
add_library(pbsd_stand_decompress)
target_sources(pbsd_stand_decompress PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.decompress.cppm)
target_link_libraries(pbsd_stand_decompress PUBLIC pbsd_core)
target_compile_options(pbsd_stand_decompress PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_bzip2)
add_library(pbsd_stand_bzip2)
target_sources(pbsd_stand_bzip2 PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.bzip2.cppm)
target_link_libraries(pbsd_stand_bzip2 PUBLIC pbsd_core)
target_compile_options(pbsd_stand_bzip2 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_xz)
add_library(pbsd_stand_xz)
target_sources(pbsd_stand_xz PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.xz.cppm)
target_link_libraries(pbsd_stand_xz PUBLIC pbsd_core)
target_compile_options(pbsd_stand_xz PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_crc)
add_library(pbsd_stand_crc)
target_sources(pbsd_stand_crc PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.crc.cppm)
target_link_libraries(pbsd_stand_crc PUBLIC pbsd_core)
target_compile_options(pbsd_stand_crc PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_cpio)
add_library(pbsd_stand_cpio)
target_sources(pbsd_stand_cpio PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.cpio.cppm)
target_link_libraries(pbsd_stand_cpio PUBLIC pbsd_core)
target_compile_options(pbsd_stand_cpio PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_tar)
add_library(pbsd_stand_tar)
target_sources(pbsd_stand_tar PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.tar.cppm)
target_link_libraries(pbsd_stand_tar PUBLIC pbsd_core)
target_compile_options(pbsd_stand_tar PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_md5)
add_library(pbsd_stand_md5)
target_sources(pbsd_stand_md5 PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.md5.cppm)
target_link_libraries(pbsd_stand_md5 PUBLIC pbsd_core)
target_compile_options(pbsd_stand_md5 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_sha256)
add_library(pbsd_stand_sha256)
target_sources(pbsd_stand_sha256 PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.sha256.cppm)
target_link_libraries(pbsd_stand_sha256 PUBLIC pbsd_core)
target_compile_options(pbsd_stand_sha256 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_random)
add_library(pbsd_stand_random)
target_sources(pbsd_stand_random PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.random.cppm)
target_link_libraries(pbsd_stand_random PUBLIC pbsd_core)
target_compile_options(pbsd_stand_random PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_split)
add_library(pbsd_stand_split)
target_sources(pbsd_stand_split PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.split.cppm)
target_link_libraries(pbsd_stand_split PUBLIC pbsd_core)
target_compile_options(pbsd_stand_split PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_manifest)
add_library(pbsd_stand_manifest)
target_sources(pbsd_stand_manifest PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.manifest.cppm)
target_link_libraries(pbsd_stand_manifest PUBLIC pbsd_core)
target_compile_options(pbsd_stand_manifest PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_signature)
add_library(pbsd_stand_signature)
target_sources(pbsd_stand_signature PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.signature.cppm)
target_link_libraries(pbsd_stand_signature PUBLIC pbsd_core)
target_compile_options(pbsd_stand_signature PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_chain)
add_library(pbsd_stand_chain)
target_sources(pbsd_stand_chain PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.chain.cppm)
target_link_libraries(pbsd_stand_chain PUBLIC pbsd_core)
target_compile_options(pbsd_stand_chain PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_config)
add_library(pbsd_stand_config)
target_sources(pbsd_stand_config PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.config.cppm)
target_link_libraries(pbsd_stand_config PUBLIC pbsd_core)
target_compile_options(pbsd_stand_config PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_fat)
add_library(pbsd_stand_fat)
target_sources(pbsd_stand_fat PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.fat.cppm)
target_link_libraries(pbsd_stand_fat PUBLIC pbsd_core)
target_compile_options(pbsd_stand_fat PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_mbr)
add_library(pbsd_stand_mbr)
target_sources(pbsd_stand_mbr PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.mbr.cppm)
target_link_libraries(pbsd_stand_mbr PUBLIC pbsd_core)
target_compile_options(pbsd_stand_mbr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_secureboot)
add_library(pbsd_stand_secureboot)
target_sources(pbsd_stand_secureboot PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.secureboot.cppm)
target_link_libraries(pbsd_stand_secureboot PUBLIC pbsd_core)
target_compile_options(pbsd_stand_secureboot PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_veriexec)
add_library(pbsd_stand_veriexec)
target_sources(pbsd_stand_veriexec PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.veriexec.cppm)
target_link_libraries(pbsd_stand_veriexec PUBLIC pbsd_core)
target_compile_options(pbsd_stand_veriexec PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_console)
add_library(pbsd_stand_console)
target_sources(pbsd_stand_console PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.console.cppm)
target_link_libraries(pbsd_stand_console PUBLIC pbsd_core)
target_compile_options(pbsd_stand_console PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_stand_acpi)
add_library(pbsd_stand_acpi)
target_sources(pbsd_stand_acpi PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.acpi.cppm)
target_link_libraries(pbsd_stand_acpi PUBLIC pbsd_core)
target_compile_options(pbsd_stand_acpi PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_stand)
target_sources(pbsd_stand PUBLIC FILE_SET CXX_MODULES FILES stand/pbsd.stand.cppm)
target_link_libraries(pbsd_stand PUBLIC pbsd_core pbsd_stand_pe pbsd_stand_efi
    pbsd_stand_efi_protocols pbsd_stand_common pbsd_stand_bootinfo
    pbsd_stand_loader pbsd_stand_reboot
    pbsd_stand_gpt pbsd_stand_kmod pbsd_stand_elf
    pbsd_stand_env pbsd_stand_devinfo pbsd_stand_zfsboot pbsd_stand_net
    pbsd_stand_bios pbsd_stand_ufs pbsd_stand_iscsi pbsd_stand_cd9660
    pbsd_stand_cdboot pbsd_stand_pxe pbsd_stand_gzip pbsd_stand_lz4
    pbsd_stand_bootp pbsd_stand_disk pbsd_stand_bcache pbsd_stand_modload
    pbsd_stand_multiboot pbsd_stand_geli pbsd_stand_tftp pbsd_stand_part
    pbsd_stand_nfs pbsd_stand_pkgfs pbsd_stand_dosfs pbsd_stand_ext2fs
    pbsd_stand_preload pbsd_stand_ffs pbsd_stand_zstd pbsd_stand_smbfs
    pbsd_stand_lua pbsd_stand_nullfs pbsd_stand_bzip2 pbsd_stand_xz pbsd_stand_crc
    pbsd_stand_cpio pbsd_stand_tar pbsd_stand_md5 pbsd_stand_sha256
    pbsd_stand_random pbsd_stand_split pbsd_stand_verify pbsd_stand_hash
    pbsd_stand_uuid pbsd_stand_install pbsd_stand_decompress
    pbsd_stand_manifest pbsd_stand_signature pbsd_stand_chain pbsd_stand_config
    pbsd_stand_fat pbsd_stand_mbr pbsd_stand_secureboot pbsd_stand_veriexec
    pbsd_stand_console pbsd_stand_acpi)
target_compile_options(pbsd_stand PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_amd64_msr)
target_sources(pbsd_arch_amd64_msr PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.msr.cppm)
target_link_libraries(pbsd_arch_amd64_msr PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_msr PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_amd64_cpufunc)
target_sources(pbsd_arch_amd64_cpufunc PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.cpufunc.cppm)
target_link_libraries(pbsd_arch_amd64_cpufunc PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_cpufunc PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_amd64_pmap)
target_sources(pbsd_arch_amd64_pmap PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.pmap.cppm)
target_link_libraries(pbsd_arch_amd64_pmap PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_pmap PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_amd64_apic)
target_sources(pbsd_arch_amd64_apic PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.apic.cppm)
target_link_libraries(pbsd_arch_amd64_apic PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_apic PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_amd64_exception)
target_sources(pbsd_arch_amd64_exception PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.exception.cppm)
target_link_libraries(pbsd_arch_amd64_exception PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_exception PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_amd64_cr)
target_sources(pbsd_arch_amd64_cr PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.cr.cppm)
target_link_libraries(pbsd_arch_amd64_cr PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_cr PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_arch_amd64_gdt)
add_library(pbsd_arch_amd64_gdt)
target_sources(pbsd_arch_amd64_gdt PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.gdt.cppm)
target_link_libraries(pbsd_arch_amd64_gdt PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_gdt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_idt)
add_library(pbsd_arch_amd64_idt)
target_sources(pbsd_arch_amd64_idt PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.idt.cppm)
target_link_libraries(pbsd_arch_amd64_idt PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_idt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_tss)
add_library(pbsd_arch_amd64_tss)
target_sources(pbsd_arch_amd64_tss PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.tss.cppm)
target_link_libraries(pbsd_arch_amd64_tss PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_tss PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_fpu)
add_library(pbsd_arch_amd64_fpu)
target_sources(pbsd_arch_amd64_fpu PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.fpu.cppm)
target_link_libraries(pbsd_arch_amd64_fpu PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_fpu PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_ident)
add_library(pbsd_arch_amd64_ident)
target_sources(pbsd_arch_amd64_ident PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.ident.cppm)
target_link_libraries(pbsd_arch_amd64_ident PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_ident PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_pic)
add_library(pbsd_arch_amd64_pic)
target_sources(pbsd_arch_amd64_pic PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.pic.cppm)
target_link_libraries(pbsd_arch_amd64_pic PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_pic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_lapic)
add_library(pbsd_arch_amd64_lapic)
target_sources(pbsd_arch_amd64_lapic PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.lapic.cppm)
target_link_libraries(pbsd_arch_amd64_lapic PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_lapic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_cpuid)
add_library(pbsd_arch_amd64_cpuid)
target_sources(pbsd_arch_amd64_cpuid PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.cpuid.cppm)
target_link_libraries(pbsd_arch_amd64_cpuid PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_cpuid PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_segments)
add_library(pbsd_arch_amd64_segments)
target_sources(pbsd_arch_amd64_segments PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.segments.cppm)
target_link_libraries(pbsd_arch_amd64_segments PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_segments PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_pte)
add_library(pbsd_arch_amd64_pte)
target_sources(pbsd_arch_amd64_pte PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.pte.cppm)
target_link_libraries(pbsd_arch_amd64_pte PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_pte PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_rdrand)
add_library(pbsd_arch_amd64_rdrand)
target_sources(pbsd_arch_amd64_rdrand PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.rdrand.cppm)
target_link_libraries(pbsd_arch_amd64_rdrand PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_rdrand PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_mtrr)
add_library(pbsd_arch_amd64_mtrr)
target_sources(pbsd_arch_amd64_mtrr PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.mtrr.cppm)
target_link_libraries(pbsd_arch_amd64_mtrr PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_mtrr PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_smp)
add_library(pbsd_arch_amd64_smp)
target_sources(pbsd_arch_amd64_smp PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.smp.cppm)
target_link_libraries(pbsd_arch_amd64_smp PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_smp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_pat)
add_library(pbsd_arch_amd64_pat)
target_sources(pbsd_arch_amd64_pat PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.pat.cppm)
target_link_libraries(pbsd_arch_amd64_pat PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_pat PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_nx)
add_library(pbsd_arch_amd64_nx)
target_sources(pbsd_arch_amd64_nx PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.nx.cppm)
target_link_libraries(pbsd_arch_amd64_nx PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_nx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_avx)
add_library(pbsd_arch_amd64_avx)
target_sources(pbsd_arch_amd64_avx PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.avx.cppm)
target_link_libraries(pbsd_arch_amd64_avx PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_avx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_tsx)
add_library(pbsd_arch_amd64_tsx)
target_sources(pbsd_arch_amd64_tsx PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.tsx.cppm)
target_link_libraries(pbsd_arch_amd64_tsx PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_tsx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_smap)
add_library(pbsd_arch_amd64_smap)
target_sources(pbsd_arch_amd64_smap PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.smap.cppm)
target_link_libraries(pbsd_arch_amd64_smap PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_smap PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_svm)
add_library(pbsd_arch_amd64_svm)
target_sources(pbsd_arch_amd64_svm PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.svm.cppm)
target_link_libraries(pbsd_arch_amd64_svm PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_svm PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_vmx)
add_library(pbsd_arch_amd64_vmx)
target_sources(pbsd_arch_amd64_vmx PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.vmx.cppm)
target_link_libraries(pbsd_arch_amd64_vmx PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_vmx PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_acpi)
add_library(pbsd_arch_amd64_acpi)
target_sources(pbsd_arch_amd64_acpi PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.acpi.cppm)
target_link_libraries(pbsd_arch_amd64_acpi PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_acpi PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_ioapic)
add_library(pbsd_arch_amd64_ioapic)
target_sources(pbsd_arch_amd64_ioapic PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.ioapic.cppm)
target_link_libraries(pbsd_arch_amd64_ioapic PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_ioapic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_nmi)
add_library(pbsd_arch_amd64_nmi)
target_sources(pbsd_arch_amd64_nmi PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.nmi.cppm)
target_link_libraries(pbsd_arch_amd64_nmi PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_nmi PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_x2apic)
add_library(pbsd_arch_amd64_x2apic)
target_sources(pbsd_arch_amd64_x2apic PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.x2apic.cppm)
target_link_libraries(pbsd_arch_amd64_x2apic PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_x2apic PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_rdt)
add_library(pbsd_arch_amd64_rdt)
target_sources(pbsd_arch_amd64_rdt PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.rdt.cppm)
target_link_libraries(pbsd_arch_amd64_rdt PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_rdt PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_amd64_amd)
add_library(pbsd_arch_amd64_amd)
target_sources(pbsd_arch_amd64_amd PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.amd.cppm)
target_link_libraries(pbsd_arch_amd64_amd PUBLIC pbsd_core)
target_compile_options(pbsd_arch_amd64_amd PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_arch_amd64)
target_sources(pbsd_arch_amd64 PUBLIC FILE_SET CXX_MODULES FILES
    arch/amd64/pbsd.arch.amd64.cppm)
target_link_libraries(pbsd_arch_amd64 PUBLIC pbsd_core pbsd_arch_amd64_msr
    pbsd_arch_amd64_cpufunc pbsd_arch_amd64_pmap pbsd_arch_amd64_apic
    pbsd_arch_amd64_exception pbsd_arch_amd64_cr
    pbsd_arch_amd64_gdt pbsd_arch_amd64_idt pbsd_arch_amd64_tss
    pbsd_arch_amd64_fpu pbsd_arch_amd64_ident pbsd_arch_amd64_pic
    pbsd_arch_amd64_lapic pbsd_arch_amd64_segments pbsd_arch_amd64_pte
    pbsd_arch_amd64_cpuid
    pbsd_arch_amd64_rdrand pbsd_arch_amd64_mtrr pbsd_arch_amd64_smp pbsd_arch_amd64_pat
    pbsd_arch_amd64_nx pbsd_arch_amd64_avx pbsd_arch_amd64_tsx pbsd_arch_amd64_smap
    pbsd_arch_amd64_svm pbsd_arch_amd64_vmx pbsd_arch_amd64_acpi
    pbsd_arch_amd64_ioapic pbsd_arch_amd64_nmi
    pbsd_arch_amd64_x2apic pbsd_arch_amd64_rdt pbsd_arch_amd64_amd)
target_compile_options(pbsd_arch_amd64 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_arm64_sysreg)
target_sources(pbsd_arch_arm64_sysreg PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.sysreg.cppm)
target_link_libraries(pbsd_arch_arm64_sysreg PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_sysreg PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_arm64_exception)
target_sources(pbsd_arch_arm64_exception PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.exception.cppm)
target_link_libraries(pbsd_arch_arm64_exception PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_exception PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_arm64_pmap)
target_sources(pbsd_arch_arm64_pmap PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.pmap.cppm)
target_link_libraries(pbsd_arch_arm64_pmap PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_pmap PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_arm64_gic)
target_sources(pbsd_arch_arm64_gic PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.gic.cppm)
target_link_libraries(pbsd_arch_arm64_gic PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_gic PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch_arm64_cpufunc)
target_sources(pbsd_arch_arm64_cpufunc PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.cpufunc.cppm)
target_link_libraries(pbsd_arch_arm64_cpufunc PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_cpufunc PUBLIC ${PBSD_FS_CXX})

if(NOT TARGET pbsd_arch_arm64_gicv3)
add_library(pbsd_arch_arm64_gicv3)
target_sources(pbsd_arch_arm64_gicv3 PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.gicv3.cppm)
target_link_libraries(pbsd_arch_arm64_gicv3 PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_gicv3 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_timer)
add_library(pbsd_arch_arm64_timer)
target_sources(pbsd_arch_arm64_timer PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.timer.cppm)
target_link_libraries(pbsd_arch_arm64_timer PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_timer PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_fpu)
add_library(pbsd_arch_arm64_fpu)
target_sources(pbsd_arch_arm64_fpu PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.fpu.cppm)
target_link_libraries(pbsd_arch_arm64_fpu PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_fpu PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_ident)
add_library(pbsd_arch_arm64_ident)
target_sources(pbsd_arch_arm64_ident PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.ident.cppm)
target_link_libraries(pbsd_arch_arm64_ident PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_ident PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_vmm)
add_library(pbsd_arch_arm64_vmm)
target_sources(pbsd_arch_arm64_vmm PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.vmm.cppm)
target_link_libraries(pbsd_arch_arm64_vmm PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_vmm PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_smmu)
add_library(pbsd_arch_arm64_smmu)
target_sources(pbsd_arch_arm64_smmu PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.smmu.cppm)
target_link_libraries(pbsd_arch_arm64_smmu PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_smmu PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_vfp)
add_library(pbsd_arch_arm64_vfp)
target_sources(pbsd_arch_arm64_vfp PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.vfp.cppm)
target_link_libraries(pbsd_arch_arm64_vfp PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_vfp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_dcz)
add_library(pbsd_arch_arm64_dcz)
target_sources(pbsd_arch_arm64_dcz PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.dcz.cppm)
target_link_libraries(pbsd_arch_arm64_dcz PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_dcz PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_spinlock)
add_library(pbsd_arch_arm64_spinlock)
target_sources(pbsd_arch_arm64_spinlock PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.spinlock.cppm)
target_link_libraries(pbsd_arch_arm64_spinlock PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_spinlock PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_smp)
add_library(pbsd_arch_arm64_smp)
target_sources(pbsd_arch_arm64_smp PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.smp.cppm)
target_link_libraries(pbsd_arch_arm64_smp PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_smp PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_mmu)
add_library(pbsd_arch_arm64_mmu)
target_sources(pbsd_arch_arm64_mmu PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.mmu.cppm)
target_link_libraries(pbsd_arch_arm64_mmu PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_mmu PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_pauth)
add_library(pbsd_arch_arm64_pauth)
target_sources(pbsd_arch_arm64_pauth PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.pauth.cppm)
target_link_libraries(pbsd_arch_arm64_pauth PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_pauth PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_bti)
add_library(pbsd_arch_arm64_bti)
target_sources(pbsd_arch_arm64_bti PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.bti.cppm)
target_link_libraries(pbsd_arch_arm64_bti PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_bti PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_pac)
add_library(pbsd_arch_arm64_pac)
target_sources(pbsd_arch_arm64_pac PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.pac.cppm)
target_link_libraries(pbsd_arch_arm64_pac PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_pac PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_mte)
add_library(pbsd_arch_arm64_mte)
target_sources(pbsd_arch_arm64_mte PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.mte.cppm)
target_link_libraries(pbsd_arch_arm64_mte PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_mte PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_sve)
add_library(pbsd_arch_arm64_sve)
target_sources(pbsd_arch_arm64_sve PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.sve.cppm)
target_link_libraries(pbsd_arch_arm64_sve PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_sve PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_el2)
add_library(pbsd_arch_arm64_el2)
target_sources(pbsd_arch_arm64_el2 PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.el2.cppm)
target_link_libraries(pbsd_arch_arm64_el2 PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_el2 PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_ras)
add_library(pbsd_arch_arm64_ras)
target_sources(pbsd_arch_arm64_ras PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.ras.cppm)
target_link_libraries(pbsd_arch_arm64_ras PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_ras PUBLIC ${PBSD_FS_CXX})
endif()

if(NOT TARGET pbsd_arch_arm64_spectre)
add_library(pbsd_arch_arm64_spectre)
target_sources(pbsd_arch_arm64_spectre PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.spectre.cppm)
target_link_libraries(pbsd_arch_arm64_spectre PUBLIC pbsd_core)
target_compile_options(pbsd_arch_arm64_spectre PUBLIC ${PBSD_FS_CXX})
endif()

add_library(pbsd_arch_arm64)
target_sources(pbsd_arch_arm64 PUBLIC FILE_SET CXX_MODULES FILES
    arch/arm64/pbsd.arch.arm64.cppm)
target_link_libraries(pbsd_arch_arm64 PUBLIC pbsd_core pbsd_arch_arm64_sysreg
    pbsd_arch_arm64_exception pbsd_arch_arm64_pmap pbsd_arch_arm64_gic
    pbsd_arch_arm64_gicv3 pbsd_arch_arm64_timer pbsd_arch_arm64_cpufunc
    pbsd_arch_arm64_fpu pbsd_arch_arm64_ident
    pbsd_arch_arm64_vmm pbsd_arch_arm64_smmu pbsd_arch_arm64_vfp
    pbsd_arch_arm64_dcz pbsd_arch_arm64_spinlock pbsd_arch_arm64_smp
    pbsd_arch_arm64_mmu pbsd_arch_arm64_pac pbsd_arch_arm64_pauth pbsd_arch_arm64_bti
    pbsd_arch_arm64_mte pbsd_arch_arm64_sve pbsd_arch_arm64_el2
    pbsd_arch_arm64_ras pbsd_arch_arm64_spectre)
target_compile_options(pbsd_arch_arm64 PUBLIC ${PBSD_FS_CXX})

add_library(pbsd_arch INTERFACE)
target_link_libraries(pbsd_arch INTERFACE pbsd_arch_amd64 pbsd_arch_arm64)

# --- Harnesses (after all library targets exist) ------------------------------
add_executable(pbsd_si_harness tests/si_harness.cpp)
target_link_libraries(pbsd_si_harness PRIVATE
    pbsd_handles pbsd_uda
    pbsd_uda_virtio_balloon pbsd_uda_nvme_admin
    pbsd_uda_cam_status pbsd_uda_cam_ccb pbsd_uda_cam_xpt pbsd_uda_cam
    pbsd_uda_virtio_scsi_cam
    pbsd_uda_sdhci pbsd_uda_mpt_cam pbsd_uda_bge pbsd_uda_ata_pci pbsd_uda_msk
    pbsd_kernel_capsicum pbsd_kernel_vm pbsd_kernel_alloc pbsd_kernel_sched
    pbsd_kernel_pax_mac pbsd_kernel_pax_aslr pbsd_kernel_resource pbsd_kernel_shim
    pbsd_kernel_sysctl pbsd_kernel_signal pbsd_kernel_errno
    pbsd_kernel_vm_page pbsd_kernel_vm_map pbsd_kernel_vm_object pbsd_kernel_vm_fault
    pbsd_kernel_uma pbsd_kernel_vm_pager pbsd_kernel_vm_phys
    pbsd_kernel_param pbsd_kernel_security_bsd pbsd_kernel_prot
    pbsd_kernel_syscall pbsd_kernel_ucred pbsd_kernel_jail
    pbsd_kernel_timekeeping pbsd_kernel_mutex pbsd_kernel_turnstile
    pbsd_kernel_ktrace
    pbsd_kernel_syscall_args pbsd_kernel_kevent pbsd_kernel_poll
    pbsd_kernel_socket_syscall pbsd_kernel_vnode pbsd_kernel_pipe
    pbsd_kernel_fifo pbsd_kernel_namei pbsd_kernel_filedesc pbsd_kernel_selinfo
    pbsd_kernel_select pbsd_kernel_aio pbsd_kernel_mbuf pbsd_kernel_pcb
    pbsd_kernel_inpcb pbsd_kernel_socket_ops
    pbsd_kernel_route pbsd_kernel_ifnet pbsd_kernel_bpf pbsd_kernel_netgraph
    pbsd_kernel_audit pbsd_kernel_sockbuf pbsd_kernel_domain pbsd_kernel_protosw
    pbsd_kernel_sockio pbsd_kernel_rtentry
    pbsd_kernel_carp pbsd_kernel_gif pbsd_kernel_gre pbsd_kernel_bridge
    pbsd_kernel_vlan pbsd_kernel_lagg pbsd_kernel_if_media pbsd_kernel_if_clone
    pbsd_kernel_pfil pbsd_kernel_random pbsd_kernel_entropy pbsd_kernel_crypto
    pbsd_kernel_geom pbsd_kernel_cam
    pbsd_kernel_callout pbsd_kernel_tty pbsd_kernel_cons pbsd_kernel_uipc
    pbsd_kernel_sysvshm pbsd_kernel_intr pbsd_kernel_busdma
    pbsd_kernel_sx pbsd_kernel_rwlock pbsd_kernel_condvar pbsd_kernel_taskqueue
    pbsd_kernel_uio pbsd_kernel_lockf
    pbsd_uda_ixl pbsd_uda_umass pbsd_uda_da
    pbsd_uda_ale pbsd_uda_axe pbsd_uda_fxp pbsd_uda_vmx
    pbsd_kernel_dual_link
    pbsd_bifrost pbsd_net pbsd_fs pbsd_geom pbsd_zfs
    pbsd_kde pbsd_theme
    pbsd_stand pbsd_arch_amd64 pbsd_arch_arm64 pbsd_compositor pbsd_pkg
    pbsd_userland_echo pbsd_userland_hostname)
target_compile_options(pbsd_si_harness PUBLIC ${PBSD_FS_CXX})
add_test(NAME si_harness COMMAND pbsd_si_harness)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/userland_harness.cpp")
  add_executable(pbsd_userland_harness tests/userland_harness.cpp)
  target_link_libraries(pbsd_userland_harness PRIVATE pbsd_userland)
  target_compile_options(pbsd_userland_harness PUBLIC ${PBSD_FS_CXX})
  add_test(NAME userland_harness COMMAND pbsd_userland_harness)
endif()
