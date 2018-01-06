---
id: AppImageUpdaterBridgeErrorCodes
title: Handling errors with AIUpdaterBridge
sidebar_label: AppImageUpdaterBridge Error Codes
---

| ERROR CODE 	| MEANING 	|
|------------------------------------------------------	|---------------------------------------------------------------	|
| AIUpdaterBridge:: UNABLE_TO_GET_APPIMAGE_INFORMATION 	| Failed to retrive the information embeded in the AppImage. 	|
| AIUpdaterBridge::APPIMAGE_PATH_NOT_GIVEN 	| AppImage path was not given in the input json configuration. 	|
| AIUpdaterBridge::TRANSPORT_NOT_GIVEN 	| No Valid Transport Mechanism is provided. 	|
| AIUpdaterBridge::URL_NOT_GIVEN 	| Url was not given in the json configuration. 	|
| AIUpdaterBridge::INVALID_UDP_INFO_PARAMETERS 	| Invalid number of parameters given in the json configuration. 	|
| AIUpdaterBridge::INVALID_TRANSPORT_GIVEN 	| Invalid Transport Mechanism given in the json configuration 	|
| AIUpdaterBridge::NETWORK_ERROR 	| A Network Error As Occured. 	|
| AIUpdaterBridge::CANNOT_FIND_GITHUB_ASSET 	| The filename was not found at github. 	|
| AIUpdaterBridge::ZSYNC_HEADER_INVALID 	| Invalid zsync header. 	|
| AIUpdaterBridge::APPIMAGE_NOT_FOUND 	| AppImage was not found in the given path. 	|
| AIUpdaterBridge::FILENAME_MISMATCH 	| The local filename and the remote filename does not match. 	|
| AIUpdaterBridge::FAILED_TO_OPEN_ZSYNC_HANDLE 	| Failed to open zsync handler. Some fatal error. 	|
| AIUpdaterBridge::ZSYNC_RANGE_FETCH_FAILED 	| Zsync range fetch failed. Some fatal error. 	|
| AIUpdaterBridge::ZSYNC_RECIEVE_FAILED 	| Zsync recieve failed , it must mean that the update failed. 	|
| AIUpdaterBridge::CANNOT_FIND_BINTRAY_PACKAGE 	| Cannot find bintray package in the given path. 	|
| AIUpdaterBridge::POST_INSTALLATION_FAILED 	| Installation failed at the last moment. 	|
| AIUpdaterBridge::UPDATE_INTEGRITY_FAILED 	| Final SHA1 Check failed. 	|
| AIUpdaterBridge::FAILED_TO_RENAME_TEMPFILE 	| Failed to rename the tempfile created by zsync. 	|
| AIUpdaterBridge::BAD_ALLOC 	| Memory error. 	|
