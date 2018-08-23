---
id: AppImageDeltaRevisionerErrorCodes
title: Handling Errors from AppImageDeltaRevisioner
sidebar_label: AppImageDeltaRevisioner Error Codes
---

Using ```AppImageUpdaterBridge::AppImageDeltaRevisioner::errorCodeToString(code)``` you can get the error code name as 
a QString , The below table tabulates all error codes with respect to their error name.

Error codes are usually emitted from ```AppImageDeltaRevisioner::error``` signal.

Error code such as ```AppImageDeltaRevisioner::UNKNOWN_NETWORK_ERROR``` can be further investigated to get the actual 
error , In this case you can call ```AppImageDeltaRevisioner::getNetworkError();``` to get the recent network error code 
which is of type [QNetworkReply::NetworkError](https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum).


> **Note**: AppImageDeltaRevisioner is under AppImageUpdaterBridge namespace , So make sure to include it.


| Error Code | Value |
|-----------------------------------------------------|-------|
| AppImageDeltaRevisioner::NO_ERROR | 0 |
| AppImageDeltaRevisioner::APPIMAGE_NOT_READABLE | 1 |
| AppImageDeltaRevisioner::NO_READ_PERMISSION | 2 |
| AppImageDeltaRevisioner::APPIMAGE_NOT_FOUND | 3 |
| AppImageDeltaRevisioner::CANNOT_OPEN_APPIMAGE | 4 |
| AppImageDeltaRevisioner::EMPTY_UPDATE_INFORMATION | 5 |
| AppImageDeltaRevisioner::INVALID_APPIMAGE_TYPE | 6 |
| AppImageDeltaRevisioner::INVALID_MAGIC_BYTES | 7 |
| AppImageDeltaRevisioner::INVALID_UPDATE_INFORMATION | 8 |
| AppImageDeltaRevisioner::NOT_ENOUGH_MEMORY | 9 |
| AppImageDeltaRevisioner::SECTION_HEADER_NOT_FOUND | 10 |
| AppImageDeltaRevisioner::UNSUPPORTED_ELF_FORMAT | 11 |
| AppImageDeltaRevisioner::UNSUPPORTED_TRANSPORT | 12 |
| AppImageDeltaRevisioner::UNKNOWN_NETWORK_ERROR | 50 |
|  AppImageDeltaRevisioner::IO_READ_ERROR | 51 |
| AppImageDeltaRevisioner::ERROR_RESPONSE_CODE | 52 |
| AppImageDeltaRevisioner::GITHUB_API_RATE_LIMIT_REACHED | 53 |
| AppImageDeltaRevisioner::NO_MARKER_FOUND_IN_CONTROL_FILE | 54 |
| AppImageDeltaRevisioner::INVALID_ZSYNC_HEADERS_NUMBER | 55 |
| AppImageDeltaRevisioner::INVALID_ZSYNC_MAKE_VERSION | 56 |
| AppImageDeltaRevisioner::INVALID_ZSYNC_TARGET_FILENAME | 57 |
| AppImageDeltaRevisioner::INVALID_ZSYNC_MTIME | 58 |
| AppImageDeltaRevisioner::INVALID_ZSYNC_BLOCKSIZE | 59 |
| AppImageDeltaRevisioner::INVALID_TARGET_FILE_LENGTH | 60 |
| AppImageDeltaRevisioner::INVALID_HASH_LENGTH_LINE | 61 |
| AppImageDeltaRevisioner::INVALID_HASH_LENGTHS | 62 |
| AppImageDeltaRevisioner::INVALID_TARGET_FILE_URL | 63 |
| AppImageDeltaRevisioner::INVALID_TARGET_FILE_SHA1 | 64 |
|  AppImageDeltaRevisioner::HASH_TABLE_NOT_ALLOCATED | 100 |
| AppImageDeltaRevisioner::INVALID_TARGET_FILE_CHECKSUM_BLOCKS | 101 |
| AppImageDeltaRevisioner::CANNOT_OPEN_TARGET_FILE_CHECKSUM_BLOCKS | 102 |
| AppImageDeltaRevisioner::CANNOT_CONSTRUCT_HASH_TABLE | 103 |
| AppImageDeltaRevisioner::QBUFFER_IO_READ_ERROR | 104 |
| AppImageDeltaRevisioner::SOURCE_FILE_NOT_FOUND | 105 |
| AppImageDeltaRevisioner::NO_PERMISSION_TO_READ_SOURCE_FILE | 106 |
| AppImageDeltaRevisioner::CANNOT_OPEN_SOURCE_FILE | 107 |
| AppImageDeltaRevisioner::NO_PERMISSION_TO_READ_WRITE_TARGET_FILE | 108 |
| AppImageDeltaRevisioner::CANNOT_OPEN_TARGET_FILE | 109 |
| AppImageDeltaRevisioner::TARGET_FILE_SHA1_HASH_MISMATCH | 110 |
| AppImageDeltaRevisioner::CANNOT_CONSTRUCT_TARGET_FILE | 111 |
