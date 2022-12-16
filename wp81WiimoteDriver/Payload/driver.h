#pragma once

#include <ntifs.h>
#include <ntddk.h>
//#include <wdm.h>
//#include <wdf.h>
#include <ntstrsafe.h>

#define WIIMOTE_DEVICE 0x8000

#define IOCTL_WIIMOTE_TEST CTL_CODE(WIIMOTE_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

#define BTH_MAX_NAME_SIZE          (248)

#define BTH_IOCTL_BASE      0

#define BTH_CTL(id)         CTL_CODE(FILE_DEVICE_BLUETOOTH,  \
                                     (id), \
                                     METHOD_BUFFERED,  \
                                     FILE_ANY_ACCESS)

#define IOCTL_BTH_GET_LOCAL_INFO            BTH_CTL(BTH_IOCTL_BASE+0x00)


typedef ULONGLONG BTH_ADDR, *PBTH_ADDR;
typedef ULONG BTH_COD, *PBTH_COD;

typedef struct _BTH_DEVICE_INFO {
    //
    // Combination BDIF_Xxx flags
    //
    ULONG flags;

    //
    // Address of remote device.
    //
    BTH_ADDR address;

    //
    // Class Of Device.
    //
    BTH_COD classOfDevice;

    //
    // name of the device
    //
    CHAR name[BTH_MAX_NAME_SIZE];

} BTH_DEVICE_INFO, *PBTH_DEVICE_INFO;

typedef struct _BTH_RADIO_INFO {
    //
    // Supported LMP features of the radio.  Use LMP_XXX() to extract
    // the desired bits.
    //
    ULONGLONG lmpSupportedFeatures;

    //
    // Manufacturer ID (possibly BTH_MFG_XXX)
    //
    USHORT mfg;

    //
    // LMP subversion
    //
    USHORT lmpSubversion;

    //
    // LMP version
    //
    UCHAR lmpVersion;

} BTH_RADIO_INFO, *PBTH_RADIO_INFO;

typedef struct _BTH_LOCAL_RADIO_INFO {
    //
    // Local BTH_ADDR, class of defice, and radio name
    //
    BTH_DEVICE_INFO         localInfo;

    //
    // Combo of LOCAL_RADIO_XXX values
    //
    ULONG flags;

    //
    // HCI revision, see core spec
    //
    USHORT hciRevision;

    //
    // HCI version, see core spec
    //
    UCHAR hciVersion;

    //
    // More information about the local radio (LMP, MFG)
    //
    BTH_RADIO_INFO radioInfo;

} BTH_LOCAL_RADIO_INFO, *PBTH_LOCAL_RADIO_INFO;
