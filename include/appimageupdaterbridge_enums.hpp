/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018-2019, Antony jr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @filename    : appimageupdaterbridge_enums.hpp
 * @description : Error codes and status codes are described here.
*/
#ifndef APPIMAGE_UPDATER_BRIDGE_ENUMS_HPP_INCLUDED
#define APPIMAGE_UPDATER_BRIDGE_ENUMS_HPP_INCLUDED
class QString;
namespace AppImageUpdaterBridge {

/* Status codes for all process.*/
enum : short {
    /* Common status. */
    Initializing = 0,
    Idle = 1,

    /* AppImage Update Information parser status. */
    OpeningAppimage,
    CalculatingAppimageSha1Hash,
    ReadingAppimageMagicBytes,
    ReadingAppimageUpdateInformation,
    FindingAppimageType,
    FindingAppimageArchitecture,
    MappingAppimageToMemory,
    SearchingForUpdateInformationSectionHeader,
    UnmappingAppimageFromMemory,
    FinalizingAppimageEmbededUpdateInformation,

    /* Zsync control file parser status. */
    ParsingAppimageEmbededUpdateInformation = 50,
    RequestingGithubApi,
    ParsingGithubApiResponse,
    RequestingZsyncControlFile,
    RequestingBintray,
    ParsingBintrayRedirectedUrlForLatestPackageUrl,
    ParsingZsyncControlFile,
    SearchingTargetFileChecksumBlockOffsetInZsyncControlFile,
    StoringZsyncControlFileDataToMemory,
    FinalizingParsingZsyncControlFile,

    /* Zsync writer status. */
    WrittingDownloadedBlockRanges = 100,
    EmittingRequiredBlockRanges,
    CheckingChecksumsForDownloadedBlockRanges,
    WrittingDownloadedBlockRangesToTargetFile,
    CalculatingTargetFileSha1Hash,
    ConstructingTargetFile
};

/* Error codes for all process. */
enum : short {
    /* Common error. */
    NoError = 0,

    /* AppImage Update Information parser errors. */
    NoAppimagePathGiven = 1,
    AppimageNotReadable,
    NoReadPermission,
    AppimageNotFound,
    CannotOpenAppimage,
    EmptyUpdateInformation,
    InvalidAppimageType,
    InvalidMagicBytes,
    InvalidUpdateInformation,
    NotEnoughMemory,
    SectionHeaderNotFound,
    UnsupportedElfFormat,
    UnsupportedTransport,

    /* Zsync Control File parser errors. */
    IoReadError = 50,
    ErrorResponseCode,
    GithubApiRateLimitReached,
    NoMarkerFoundInControlFile,
    InvalidZsyncHeadersNumber,
    InvalidZsyncMakeVersion,
    InvalidZsyncTargetFilename,
    InvalidZsyncMtime,
    InvalidZsyncBlocksize,
    InvalidTargetFileLength,
    InvalidHashLengthLine,
    InvalidHashLengths,
    InvalidTargetFileUrl,
    InvalidTargetFileSha1,
    ConnectionRefusedError,
    RemoteHostClosedError,
    HostNotFoundError,
    TimeoutError,
    OperationCanceledError,
    SslHandshakeFailedError,
    TemporaryNetworkFailureError,
    NetworkSessionFailedError,
    BackgroundRequestNotAllowedError,
    TooManyRedirectsError,
    InsecureRedirectError,
    ProxyConnectionRefusedError,
    ProxyConnectionClosedError,
    ProxyNotFoundError,
    ProxyTimeoutError,
    ProxyAuthenticationRequiredError,
    ContentAccessDenied,
    ContentOperationNotPermittedError,
    ContentNotFoundError,
    AuthenticationRequiredError,
    ContentReSendError,
    ContentConflictError,
    ContentGoneError,
    InternalServerError,
    OperationNotImplementedError,
    ServiceUnavailableError,
    ProtocolUnknownError,
    ProtocolInvalidOperationError,
    UnknownNetworkError,
    UnknownProxyError,
    UnknownContentError,
    ProtocolFailure,
    UnknownServerError,
    ZsyncControlFileNotFound,

    /* Zsync writer errors. */
    HashTableNotAllocated = 100,
    InvalidTargetFileChecksumBlocks,
    CannotOpenTargetFileChecksumBlocks,
    CannotConstructHashTable,
    QbufferIoReadError,
    SourceFileNotFound,
    NoPermissionToReadSourceFile,
    CannotOpenSourceFile,
    NoPermissionToReadWriteTargetFile,
    CannotOpenTargetFile,
    TargetFileSha1HashMismatch
};

QString errorCodeToString(short);
QString errorCodeToDescriptionString(short);
QString statusCodeToString(short);
}
#endif // APPIMAGE_UPDATER_BRIDGE_HPP_INCLUDED
