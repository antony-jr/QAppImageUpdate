---
id: AppImageUpdaterBridgeStatusCodes
title: Handling Status from AppImageUpdaterBridge
sidebar_label: Status Codes
---

Using ```AppImageUpdaterBridge::AppImageDeltaRevisioner::statusCodeToString(code)``` you can get the status code as 
a QString , The below table tabulates all status codes with respect to their status name.
Status codes are usually emitted from ```AppImageDeltaRevisioner::statusChanged``` signal.



| Status Code | Value |
|--------------------------------------------------------------------------------------------|-------|
| AppImageUpdaterBridge::INITIALIZING | 0 |
| AppImageUpdaterBridge::IDLE | 1 |
| AppImageUpdaterBridge::OPENING_APPIMAGE | 2 |
| AppImageUpdaterBridge::CALCULATING_APPIMAGE_SHA1_HASH | 3 |
| AppImageUpdaterBridge::READING_APPIMAGE_MAGIC_BYTES | 4 |
| AppImageUpdaterBridge::FINDING_APPIMAGE_ARCHITECTURE | 5 |
| AppImageUpdaterBridge::MAPPING_APPIMAGE_TO_MEMORY | 6 |
| AppImageUpdaterBridge::SEARCHING_FOR_UPDATE_INFORMATION_SECTION_HEADER | 7 |
| AppImageUpdaterBridge::UNMAPPING_APPIMAGE_FROM_MEMORY | 8 |
| AppImageUpdaterBridge::FINALIZING_APPIMAGE_EMBEDED_UPDATE_INFORMATION | 9 |
| AppImageUpdaterBridge::PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION | 50 |
| AppImageUpdaterBridge::REQUESTING_GITHUB_API | 51 |
| AppImageUpdaterBridge::PARSING_GITHUB_API_RESPONSE | 52 |
| AppImageUpdaterBridge::REQUESTING_ZSYNC_CONTROL_FILE | 53 |
| AppImageUpdaterBridge::REQUESTING_BINTRAY | 54 |
| AppImageUpdaterBridge::PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL | 55 |
| AppImageUpdaterBridge::PARSING_ZSYNC_CONTROL_FILE | 56 |
| AppImageUpdaterBridge::SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE | 57 |
| AppImageUpdaterBridge::STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY | 58 |
| AppImageUpdaterBridge::FINALIZING_PARSING_ZSYNC_CONTROL_FILE | 59 |
| AppImageUpdaterBridge::WRITTING_DOWNLOADED_BLOCK_RANGES | 100 |
| AppImageUpdaterBridge::EMITTING_REQUIRED_BLOCK_RANGES | 101 |
| AppImageUpdaterBridge::CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES | 102 |
| AppImageUpdaterBridge::WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE | 103 |
| AppImageUpdaterBridge::CALCULATING_TARGET_FILE_SHA1_HASH | 104 |
| AppImageUpdaterBridge::CONSTRUCTING_TARGET_FILE | 105 |
