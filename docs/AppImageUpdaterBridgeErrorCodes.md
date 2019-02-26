---
id: AppImageUpdaterBridgeErrorCodes
title: Handling Errors from AppImageUpdaterBridge
sidebar_label: Error Codes
---

Using static function ```AppImageUpdaterBridge::errorCodeToString(short)``` you can get the error code name as 
a QString , The below table tabulates all error codes with respect to their error name.

> Note: All are under the AppImageUpdaterBridge namespace so make sure to include it.


| Error Code                       | Value |
|----------------------------------|------ |
| NoError                          | 0 |
| NoAppimagePathGiven              | 1 |
| AppimageNotReadable              | 2 |
| NoReadPermission                 | 3 |
| AppimageNotFound                 | 4 |
| CannotOpenAppimage               | 5 |
| EmptyUpdateInformation           | 6 |
| InvalidAppimageType              | 7 |
| InvalidMagicBytes                | 8 |
| InvalidUpdateInformation         | 9 |
| NotEnoughMemory                  | 10 |
| SectionHeaderNotFound            | 11 |
| UnsupportedElfFormat             | 12 |
| UnsupportedTransport             | 13 |
| IoReadError                      | 50 |
| ErrorResponseCode                | 51 |
| GithubApiRateLimitReached        | 52 |
| NoMarkerFoundInControlFile       | 53 |
| InvalidZsyncHeadersNumber        | 54 |
| InvalidZsyncMakeVersion          | 55 |
| InvalidZsyncTargetFilename       | 56 |
| InvalidZsyncMtime                | 57 |
| InvalidZsyncBlocksize            | 58 |
| InvalidTargetFileLength          | 59 |
| InvalidHashLengthLine            | 60 |
| InvalidHashLengths               | 61 |
| InvalidTargetFileUrl             | 62 |
| InvalidTargetFileSha1            | 63 |
| ConnectionRefusedError           | 64 |
| RemoteHostClosedError            | 65 |
| HostNotFoundError                | 66 |
| TimeoutError                     | 67 |
| OperationCanceledError           | 68 |
| SslHandshakeFailedError          | 69 |
| TemporaryNetworkFailureError     | 70 |
| NetworkSessionFailedError        | 71 |
| BackgroundRequestNotAllowedError | 72 |
| TooManyRedirectsError            | 73 |
| InsecureRedirectError            | 74 |
| ProxyConnectionRefusedError      | 75 |
| ProxyConnectionClosedError       | 76 |
| ProxyNotFoundError               | 77 |
| ProxyTimeoutError                | 78 |
| ProxyAuthenticationRequiredError | 79 |
| ContentAccessDenied              | 80 |
| ContentOperationNotPermittedError| 81 |
| ContentNotFoundError             | 82 |
| AuthenticationRequiredError      | 83 |
| ContentReSendError               | 84 |
| ContentConflictError             | 85 |
| ContentGoneError                 | 86 |
| InternalServerError              | 87 |
| OperationNotImplementedError     | 88 |
| ServiceUnavailableError          | 89 |
| ProtocolUnknownError             | 90 |
| ProtocolInvalidOperationError    | 91 |
| UnknownNetworkError              | 92 |
| UnknownProxyError                | 93 |
| UnknownContentError              | 94 |
| ProtocolFailure                  | 95 |
| UnknownServerError               | 96 |
| HashTableNotAllocated            | 100 |
| InvalidTargetFileChecksumBlocks  | 101 |
| CannotOpenTargetFileChecksumBlocks| 102 |
| CannotConstructHashTable         | 103 |
| QbufferIoReadError               | 104 |
| SourceFileNotFound               | 105 |
| NoPermissionToReadSourceFile     | 106 |
| CannotOpenSourceFile             | 107 |
| NoPermissionToReadWriteTargetFile| 108 |
| CannotOpenTargetFile             | 109 |
| TargetFileSha1HashMismatch       | 110 |
