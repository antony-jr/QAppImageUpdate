---
id: AppImageUpdaterBridgeErrorCodes
title: Handling Errors from AppImageDeltaRevisioner
sidebar_label: Error Codes
---

Using ```AppImageUpdaterBridge::AppImageDeltaRevisioner::errorCodeToString(code)``` you can get the error code name as 
a QString , The below table tabulates all error codes with respect to their error name.

Error codes are usually emitted from ```AppImageDeltaRevisioner::error``` signal.

Error code such as ```AppImageUpdaterBridge::UNKNOWN_NETWORK_ERROR``` can be further investigated to get the actual 
error , In this case you can call ```AppImageDeltaRevisioner::getNetworkError();``` to get the recent network error code 
which is of type [QNetworkReply::NetworkError](https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum).


> **Note**: AppImageDeltaRevisioner is under AppImageUpdaterBridge namespace , So make sure to include it.


| Error Code | Value |
|-----------------------------------------------------|-------|
| AppImageUpdaterBridge::NO_ERROR | 0 |
| AppImageUpdaterBridge::APPIMAGE_NOT_READABLE | 1 |
| AppImageUpdaterBridge::NO_READ_PERMISSION | 2 |
| AppImageUpdaterBridge::APPIMAGE_NOT_FOUND | 3 |
| AppImageUpdaterBridge::CANNOT_OPEN_APPIMAGE | 4 |
| AppImageUpdaterBridge::EMPTY_UPDATE_INFORMATION | 5 |
| AppImageUpdaterBridge::INVALID_APPIMAGE_TYPE | 6 |
| AppImageUpdaterBridge::INVALID_MAGIC_BYTES | 7 |
| AppImageUpdaterBridge::INVALID_UPDATE_INFORMATION | 8 |
| AppImageUpdaterBridge::NOT_ENOUGH_MEMORY | 9 |
| AppImageUpdaterBridge::SECTION_HEADER_NOT_FOUND | 10 |
| AppImageUpdaterBridge::UNSUPPORTED_ELF_FORMAT | 11 |
| AppImageUpdaterBridge::UNSUPPORTED_TRANSPORT | 12 |
| AppImageUpdaterBridge::UNKNOWN_NETWORK_ERROR | 50 |
|  AppImageUpdaterBridge::IO_READ_ERROR | 51 |
| AppImageUpdaterBridge::ERROR_RESPONSE_CODE | 52 |
| AppImageUpdaterBridge::GITHUB_API_RATE_LIMIT_REACHED | 53 |
| AppImageUpdaterBridge::NO_MARKER_FOUND_IN_CONTROL_FILE | 54 |
| AppImageUpdaterBridge::INVALID_ZSYNC_HEADERS_NUMBER | 55 |
| AppImageUpdaterBridge::INVALID_ZSYNC_MAKE_VERSION | 56 |
| AppImageUpdaterBridge::INVALID_ZSYNC_TARGET_FILENAME | 57 |
| AppImageUpdaterBridge::INVALID_ZSYNC_MTIME | 58 |
| AppImageUpdaterBridge::INVALID_ZSYNC_BLOCKSIZE | 59 |
| AppImageUpdaterBridge::INVALID_TARGET_FILE_LENGTH | 60 |
| AppImageUpdaterBridge::INVALID_HASH_LENGTH_LINE | 61 |
| AppImageUpdaterBridge::INVALID_HASH_LENGTHS | 62 |
| AppImageUpdaterBridge::INVALID_TARGET_FILE_URL | 63 |
| AppImageUpdaterBridge::INVALID_TARGET_FILE_SHA1 | 64 |
|  AppImageUpdaterBridge::HASH_TABLE_NOT_ALLOCATED | 100 |
| AppImageUpdaterBridge::INVALID_TARGET_FILE_CHECKSUM_BLOCKS | 101 |
| AppImageUpdaterBridge::CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS | 102 |
| AppImageUpdaterBridge::CANNOT_CONSTRUCT_HASH_TABLE | 103 |
| AppImageUpdaterBridge::QBUFFER_IO_READ_ERROR | 104 |
| AppImageUpdaterBridge::SOURCE_FILE_NOT_FOUND | 105 |
| AppImageUpdaterBridge::NO_PERMISSION_TO_READ_SOURCE_FILE | 106 |
| AppImageUpdaterBridge::CANNOT_OPEN_SOURCE_FILE | 107 |
| AppImageUpdaterBridge::NO_PERMISSION_TO_READ_WRITE_TARGET_FILE | 108 |
| AppImageUpdaterBridge::CANNOT_OPEN_TARGET_FILE | 109 |
| AppImageUpdaterBridge::TARGET_FILE_SHA1_HASH_MISMATCH | 110 |
