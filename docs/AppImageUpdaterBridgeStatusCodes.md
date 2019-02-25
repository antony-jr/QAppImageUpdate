---
id: AppImageUpdaterBridgeStatusCodes
title: Handling Status from AppImageUpdaterBridge
sidebar_label: Status Codes
---

Using ```AppImageUpdaterBridge::statusCodeToString(code)``` you can get the status code as 
a QString , The below table tabulates all status codes with respect to their status name.

> Note: All are under the AppImageUpdaterBridge namespace so make sure to include it.


| Status Code                                                  |Value|
|--------------------------------------------------------------|-----|
| Initializing                                                 | 0 |
| Idle                                                         | 1 |
| OpeningAppimage                                              | 2 |
| CalculatingAppimageSha1Hash                                  | 3 |
| ReadingAppimageMagicBytes                                    | 4 |
| ReadingAppimageUpdateInformation                             | 5 |
| FindingAppimageType                                          | 6 |
| FindingAppimageArchitecture                                  | 7 |
| MappingAppimageToMemory                                      | 8 |
| SearchingForUpdateInformationSectionHeader                   | 9 |
| UnmappingAppimageFromMemory                                  | 10 |
| FinalizingAppimageEmbededUpdateInformation                   | 11 |
| ParsingAppimageEmbededUpdateInformation=50                   | 50 |
| RequestingGithubApi                                          | 51 |
| ParsingGithubApiResponse                                     | 52 |
| RequestingZsyncControlFile                                   | 53 |
| RequestingBintray                                            | 54 |
| ParsingBintrayRedirectedUrlForLatestPackageUrl               | 55 |
| ParsingZsyncControlFile                                      | 56 |
| SearchingTargetFileChecksumBlockOffsetInZsyncControlFile     | 57 |
| StoringZsyncControlFileDataToMemory                          | 58 |
| FinalizingParsingZsyncControlFile                            | 59 |
| WrittingDownloadedBlockRanges                                | 100 |
| EmittingRequiredBlockRanges                                  | 101 |
| CheckingChecksumsForDownloadedBlockRanges                    | 102 |
| WrittingDownloadedBlockRangesToTargetFile                    | 103 |
| CalculatingTargetFileSha1Hash                                | 104 |
| ConstructingTargetFile                                       | 105 |
