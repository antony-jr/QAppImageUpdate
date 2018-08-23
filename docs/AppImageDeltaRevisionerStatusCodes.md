---
id: AppImageDeltaRevisionerStatusCodes
title: Handling Status from AppImageDeltaRevisioner
sidebar_label: AppImageDeltaRevisioner Status Codes
---

Using ```AppImageUpdaterBridge::AppImageDeltaRevisioner::statusCodeToString(code)``` you can get the status code as 
a QString , The below table tabulates all status codes with respect to their status name.
Status codes are usually emitted from ```AppImageDeltaRevisioner::statusChanged``` signal.


> **Note**: AppImageDeltaRevisioner is under AppImageUpdaterBridge namespace , So make sure to include it.



| Status Code | Value |
|--------------------------------------------------------------------------------------------|-------|
| AppImageDeltaRevisioner::INITIALIZING | 0 |
| AppImageDeltaRevisioner::IDLE | 1 |
| AppImageDeltaRevisioner::OPENING_APPIMAGE | 2 |
| AppImageDeltaRevisioner::CALCULATING_APPIMAGE_SHA1_HASH | 3 |
| AppImageDeltaRevisioner::READING_APPIMAGE_MAGIC_BYTES | 4 |
| AppImageDeltaRevisioner::FINDING_APPIMAGE_ARCHITECTURE | 5 |
| AppImageDeltaRevisioner::MAPPING_APPIMAGE_TO_MEMORY | 6 |
| AppImageDeltaRevisioner::SEARCHING_FOR_UPDATE_INFORMATION_SECTION_HEADER | 7 |
| AppImageDeltaRevisioner::UNMAPPING_APPIMAGE_FROM_MEMORY | 8 |
| AppImageDeltaRevisioner::FINALIZING_APPIMAGE_EMBEDED_UPDATE_INFORMATION | 9 |
| AppImageDeltaRevisioner::PARSING_APPIMAGE_EMBEDED_UPDATE_INFORMATION | 50 |
| AppImageDeltaRevisioner::REQUESTING_GITHUB_API | 51 |
| AppImageDeltaRevisioner::PARSING_GITHUB_API_RESPONSE | 52 |
| AppImageDeltaRevisioner::REQUESTING_ZSYNC_CONTROL_FILE | 53 |
| AppImageDeltaRevisioner::REQUESTING_BINTRAY | 54 |
| AppImageDeltaRevisioner::PARSING_BINTRAY_REDIRECTED_URL_FOR_LATEST_PACKAGE_URL | 55 |
| AppImageDeltaRevisioner::PARSING_ZSYNC_CONTROL_FILE | 56 |
| AppImageDeltaRevisioner::SEARCHING_TARGET_FILE_CHECKSUM_BLOCK_OFFSET_IN_ZSYNC_CONTROL_FILE | 57 |
| AppImageDeltaRevisioner::STORING_ZSYNC_CONTROL_FILE_DATA_TO_MEMORY | 58 |
| AppImageDeltaRevisioner::FINALIZING_PARSING_ZSYNC_CONTROL_FILE | 59 |
| AppImageDeltaRevisioner::WRITTING_DOWNLOADED_BLOCK_RANGES | 100 |
| AppImageDeltaRevisioner::EMITTING_REQUIRED_BLOCK_RANGES | 101 |
| AppImageDeltaRevisioner::CHECKING_CHECKSUMS_FOR_DOWNLOADED_BLOCK_RANGES | 102 |
| AppImageDeltaRevisioner::WRITTING_DOWNLOADED_BLOCK_RANGES_TO_TARGET_FILE | 103 |
| AppImageDeltaRevisioner::CALCULATING_TARGET_FILE_SHA1_HASH | 104 |
| AppImageDeltaRevisioner::CONSTRUCTING_TARGET_FILE | 105 |
