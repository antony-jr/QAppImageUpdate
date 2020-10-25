---
id: version-2.0.0-ErrorCodes
title: QAppImageUpdate Error Codes
sidebar_label: Error Codes
original_id: ErrorCodes
---

Using static function ```QAppImageUpdate::errorCodeToString(short)``` you can get the error code name as 
a QString, The below table tabulates all error codes with respect to their error name.


| Error Code                                               | Value |
|----------------------------------------------------------|------ |
| QAppImageUpdate::Error::NoError                          | 0 |
| QAppImageUpdate::Error::NoAppimagePathGiven              | 1 |
| QAppImageUpdate::Error::AppimageNotReadable              | 2 |
| QAppImageUpdate::Error::NoReadPermission                 | 3 |
| QAppImageUpdate::Error::AppimageNotFound                 | 4 |
| QAppImageUpdate::Error::CannotOpenAppimage               | 5 |
| QAppImageUpdate::Error::EmptyUpdateInformation           | 6 |
| QAppImageUpdate::Error::InvalidAppimageType              | 7 |
| QAppImageUpdate::Error::InvalidMagicBytes                | 8 |
| QAppImageUpdate::Error::InvalidUpdateInformation         | 9 |
| QAppImageUpdate::Error::NotEnoughMemory                  | 10 |
| QAppImageUpdate::Error::SectionHeaderNotFound            | 11 |
| QAppImageUpdate::Error::UnsupportedElfFormat             | 12 |
| QAppImageUpdate::Error::UnsupportedTransport             | 13 |
| QAppImageUpdate::Error::IoReadError                      | 50 |
| QAppImageUpdate::Error::ErrorResponseCode                | 51 |
| QAppImageUpdate::Error::GithubApiRateLimitReached        | 52 |
| QAppImageUpdate::Error::NoMarkerFoundInControlFile       | 53 |
| QAppImageUpdate::Error::InvalidZsyncHeadersNumber        | 54 |
| QAppImageUpdate::Error::InvalidZsyncMakeVersion          | 55 |
| QAppImageUpdate::Error::InvalidZsyncTargetFilename       | 56 |
| QAppImageUpdate::Error::InvalidZsyncMtime                | 57 |
| QAppImageUpdate::Error::InvalidZsyncBlocksize            | 58 |
| QAppImageUpdate::Error::InvalidTargetFileLength          | 59 |
| QAppImageUpdate::Error::InvalidHashLengthLine            | 60 |
| QAppImageUpdate::Error::InvalidHashLengths               | 61 |
| QAppImageUpdate::Error::InvalidTargetFileUrl             | 62 |
| QAppImageUpdate::Error::InvalidTargetFileSha1            | 63 |
| QAppImageUpdate::Error::ConnectionRefusedError           | 64 |
| QAppImageUpdate::Error::RemoteHostClosedError            | 65 |
| QAppImageUpdate::Error::HostNotFoundError                | 66 |
| QAppImageUpdate::Error::TimeoutError                     | 67 |
| QAppImageUpdate::Error::OperationCanceledError           | 68 |
| QAppImageUpdate::Error::SslHandshakeFailedError          | 69 |
| QAppImageUpdate::Error::TemporaryNetworkFailureError     | 70 |
| QAppImageUpdate::Error::NetworkSessionFailedError        | 71 |
| QAppImageUpdate::Error::BackgroundRequestNotAllowedError | 72 |
| QAppImageUpdate::Error::TooManyRedirectsError            | 73 |
| QAppImageUpdate::Error::InsecureRedirectError            | 74 |
| QAppImageUpdate::Error::ProxyConnectionRefusedError      | 75 |
| QAppImageUpdate::Error::ProxyConnectionClosedError       | 76 |
| QAppImageUpdate::Error::ProxyNotFoundError               | 77 |
| QAppImageUpdate::Error::ProxyTimeoutError                | 78 |
| QAppImageUpdate::Error::ProxyAuthenticationRequiredError | 79 |
| QAppImageUpdate::Error::ContentAccessDenied              | 80 |
| QAppImageUpdate::Error::ContentOperationNotPermittedError| 81 |
| QAppImageUpdate::Error::ContentNotFoundError             | 82 |
| QAppImageUpdate::Error::AuthenticationRequiredError      | 83 |
| QAppImageUpdate::Error::ContentReSendError               | 84 |
| QAppImageUpdate::Error::ContentConflictError             | 85 |
| QAppImageUpdate::Error::ContentGoneError                 | 86 |
| QAppImageUpdate::Error::InternalServerError              | 87 |
| QAppImageUpdate::Error::OperationNotImplementedError     | 88 |
| QAppImageUpdate::Error::ServiceUnavailableError          | 89 |
| QAppImageUpdate::Error::ProtocolUnknownError             | 90 |
| QAppImageUpdate::Error::ProtocolInvalidOperationError    | 91 |
| QAppImageUpdate::Error::UnknownNetworkError              | 92 |
| QAppImageUpdate::Error::UnknownProxyError                | 93 |
| QAppImageUpdate::Error::UnknownContentError              | 94 |
| QAppImageUpdate::Error::ProtocolFailure                  | 95 |
| QAppImageUpdate::Error::UnknownServerError               | 96 |
| QAppImageUpdate::Error::ZsyncControlFileNotFound         | 97 |
| QAppImageUpdate::Error::HashTableNotAllocated            | 100 |
| QAppImageUpdate::Error::InvalidTargetFileChecksumBlocks  | 101 |
| QAppImageUpdate::Error::CannotOpenTargetFileChecksumBlocks| 102 |
| QAppImageUpdate::Error::CannotConstructHashTable         | 103 |
| QAppImageUpdate::Error::QbufferIoReadError               | 104 |
| QAppImageUpdate::Error::SourceFileNotFound               | 105 |
| QAppImageUpdate::Error::NoPermissionToReadSourceFile     | 106 |
| QAppImageUpdate::Error::CannotOpenSourceFile             | 107 |
| QAppImageUpdate::Error::NoPermissionToReadWriteTargetFile| 108 |
| QAppImageUpdate::Error::CannotOpenTargetFile             | 109 |
| QAppImageUpdate::Error::TargetFileSha1HashMismatch       | 110 |
| QAppImageUpdate::Error::UnsupportedActionForBuild        | 200 |
| QAppImageUpdate::Error::InvalidAction                    | 201 | 
