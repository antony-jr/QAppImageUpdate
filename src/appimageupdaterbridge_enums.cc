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
 * @filename    : appimageupdaterbridge_enums.cc
 * @description : implementation of errorCodeToString and statusCodeToString.
*/
#include <QString>
#include "../include/appimageupdaterbridge_enums.hpp"

using namespace AppImageUpdaterBridge;

QString errorCodeToString(short errorCode)
{
    QString ret = "AppImageUpdaterBridge::errorCode(";
    switch(errorCode) {
    case AppimageNotReadable:
        ret += "AppImageNotReadable";
        break;
    case NoReadPermission:
        ret += "NoReadPermission";
        break;
    case AppimageNotFound:
        ret += "AppimageNotFound";
        break;
    case CannotOpenAppimage:
        ret += "CannnotOpenAppimage";
        break;
    case EmptyUpdateInformation:
        ret += "EmptyUpdateInformation";
        break;
    case InvalidAppimageType:
        ret += "InvalidAppimageType";
        break;
    case InvalidMagicBytes:
        ret += "InvalidMagicBytes";
        break;
    case InvalidUpdateInformation:
        ret += "InvalidUpdateInformation";
        break;
    case NotEnoughMemory:
        ret += "NotEnoughMemory";
        break;
    case SectionHeaderNotFound:
        ret += "SectionHeaderNotFound";
        break;
    case UnsupportedElfFormat:
        ret += "UnsupportedElfFormat";
        break;
    case UnsupportedTransport:
        ret += "UnsupportedTransport";
        break;
    case UnknownNetworkError:
        ret += "UnknownNetworkError";
        break;
    case IoReadError:
        ret += "IoReadError";
        break;
    case ErrorResponseCode:
        ret += "ErrorResponseCode";
        break;
    case GithubApiRateLimitReached:
        ret += "GithubApiRateLimitReached";
        break;
    case NoMarkerFoundInControlFile:
        ret += "NoMarkerFoundInControlFile";
        break;
    case InvalidZsyncHeadersNumber:
        ret += "InvalidZsyncHeadersNumber";
        break;
    case InvalidZsyncMakeVersion:
        ret += "InvalidZsyncMakeVersion";
        break;
    case InvalidZsyncTargetFilename:
        ret += "InvalidZsyncTargetFilename";
        break;
    case InvalidZsyncMtime:
        ret += "InvalidZsyncMtime";
        break;
    case InvalidZsyncBlocksize:
        ret += "InvalidZsyncBlocksize";
        break;
    case InvalidTargetFileLength:
        ret += "InvalidTargetFileLength";
        break;
    case InvalidHashLengthLine:
        ret += "InvalidHashLengthLine";
        break;
    case InvalidHashLengths:
        ret += "InvalidHashLengths";
        break;
    case InvalidTargetFileUrl:
        ret += "InvalidTargetFileUrl";
        break;
    case InvalidTargetFileSha1:
        ret += "InvalidTargetFileSha1";
        break;
    case ConnectionRefusedError:
        ret += "ConnectionRefusedError";
        break;
    case RemoteHostClosedError:
        ret += "RemoteHostClosedError";
        break;
    case HostNotFoundError:
        ret += "HostNotFoundError";
        break;
    case TimeoutError:
        ret += "TimeoutError";
        break;
    case OperationCanceledError:
        ret += "OperationCanceledError";
        break;
    case SslHandshakeFailedError:
        ret += "SslHandshakeFailedError";
        break;
    case TemporaryNetworkFailureError:
        ret += "TemporaryNetworkFailureError";
        break;
    case NetworkSessionFailedError:
        ret += "NetworkSessionFailedError";
        break;
    case BackgroundRequestNotAllowedError:
        ret += "BackgroundRequestNotAllowedError";
        break;
    case TooManyRedirectsError:
        ret += "TooManyRedirectsError";
        break;
    case InsecureRedirectError:
        ret += "InsecureRedirectError";
        break;
    case ProxyConnectionRefusedError:
        ret += "ProxyConnectionRefusedError";
        break;
    case ProxyConnectionClosedError:
        ret += "ProxyConnectionClosedError";
        break;
    case ProxyNotFoundError:
        ret += "ProxyNotFoundError";
        break;
    case ProxyTimeoutError:
        ret += "ProxyTimeoutError";
        break;
    case ProxyAuthenticationRequiredError:
        ret += "ProxyAuthenticationRequiredError";
        break;
    case ContentAccessDenied:
        ret += "ContentAccessDenied";
        break;
    case ContentOperationNotPermittedError:
        ret += "ContentOperationNotPermittedError";
        break;
    case ContentNotFoundError:
        ret += "ContentNotFoundError";
        break;
    case AuthenticationRequiredError:
        ret += "AuthenticationRequiredError";
        break;
    case ContentReSendError:
        ret += "ContentReSendError";
        break;
    case ContentConflictError:
        ret += "ContentConflictError";
        break;
    case ContentGoneError:
        ret += "ContentGoneError";
        break;
    case InternalServerError:
        ret += "InternalServerError";
        break;
    case OperationNotImplementedError:
        ret += "OperationNotImplementedError";
        break;
    case ServiceUnavailableError:
        ret += "ServiceUnavailableError";
        break;
    case ProtocolUnknownError:
        ret += "ProtocolUnknownError";
        break;
    case ProtocolInvalidOperationError:
        ret += "ProtocolInvalidOperationError";
        break;
    case UnknownNetworkError:
        ret += "UnknownNetworkError";
        break;
    case UnknownProxyError:
        ret += "UnknownProxyError";
        break;
    case UnknownContentError:
        ret += "UnknownContentError";
        break;
    case ProtocolFailure:
        ret += "ProtocolFailure";
        break;
    case UnknownServerError:
        ret += "UnknownServerError";
        break;
    case HashTableNotAllocated:
        ret += "HashTableNotAllocated";
        break;
    case InvalidTargetFileChecksumBlocks:
        ret += "InvalidTargetFileChecksumBlocks";
        break;
    case CannotConstructHashTable:
        ret += "CannotConstructHashTable";
        break;
    case CannotOpenTargetFileChecksumBlocks:
        ret += "CannotOpenTargetFileChecksumBlocks";
        break;
    case QbufferIoReadError:
        ret += "QbufferIoReadError";
        break;
    case SourceFileNotFound:
        ret += "SourceFileNotFound";
        break;
    case NoPermissionToReadSourceFile:
        ret += "NoPermissionToReadSourceFile";
        break;
    case CannotOpenSourceFile:
        ret += "CannotOpenSourceFile";
        break;
    case NoPermissionToReadWriteTargetFile:
        ret += "NoPermissionToReadWriteTargetFile";
        break;
    case CannotOpenTargetFile:
        ret += "CannotOpenTargetFile";
        break;
    case TargetFileSha1HashMismatch:
        ret += "TargetFileSha1HashMismatch";
        break;

    default:
        ret += "Unknown";
        break;
    }
    ret += ")";
    return ret;
}

QString statusCodeToString(short code)
{
    QString ret = "AppImageUpdaterBridge::statusCode(";
    switch(code) {
    case Initializing:
        ret += "Initializing";
        break;
    case Idle:
        ret += "Idle";
        break;
    case OpeningAppimage:
        ret += "OpeningAppimage";
        break;
    case CalculatingAppimageSha1Hash:
        ret += "CalculatingAppimageSha1Hash";
        break;
    case ReadingAppimageMagicBytes:
        ret += "ReadingAppimageMagicBytes";
        break;
    case FindingAppimageType:
        ret += "FindingAppimageType";
        break;
    case FindingAppimageArchitecture:
        ret += "FindingAppimageArchitecture";
        break;
    case MappingAppimageToMemory:
        ret += "MappingAppimageToMemory";
        break;
    case ReadingAppimageUpdateInformation:
        ret += "ReadingAppimageUpdateInformation";
        break;
    case SearchingForUpdateInformationSectionHeader:
        ret += "SearchingForUpdateInformationSectionHeader";
        break;
    case UnmappingAppimageFromMemory:
        ret += "UnmappingAppimageFromMemory";
        break;
    case FinalizingAppimageEmbededUpdateInformation:
        ret += "FinalizingAppimageEmbededUpdateInformation";
        break;
    case ParsingAppimageEmbededUpdateInformation:
        ret += "ParsingAppimageEmbededUpdateInformation";
        break;
    case RequestingGithubApi:
        ret += "RequestingGithubApi";
        break;
    case ParsingGithubApiResponse:
        ret += "ParsingGithubApiResponse";
        break;
    case RequestingZsyncControlFile:
        ret += "RequestingZsyncControlFile";
        break;
    case RequestingBintray:
        ret += "RequestingBintray";
        break;
    case ParsingBintrayRedirectedUrlForLatestPackageUrl:
        ret += "ParsingBintrayRedirectedUrlForLatestPackageUrl";
        break;
    case ParsingZsyncControlFile:
        ret += "ParsingZsyncControlFile";
        break;
    case SearchingTargetFileChecksumBlockOffsetInZsyncControlFile:
        ret += "SearchingTargetFileChecksumBlockOffsetInZsyncControlFile";
        break;
    case StoringZsyncControlFileDataToMemory:
        ret += "StoringZsyncControlFileDataToMemory";
        break;
    case FinalizingParsingZsyncControlFile:
        ret += "FinalizingParsingZsyncControlFile";
        break;
    case WrittingDownloadedBlockRanges:
        ret += "WrittingDownloadedBlockRanges";
        break;
    case EmittingRequiredBlockRanges:
        ret += "EmittingRequiredBlockRanges";
        break;
    case CheckingChecksumsForDownloadedBlockRanges:
        ret += "CheckingChecksumsForDownloadedBlockRanges";
        break;
    case WrittingDownloadedBlockRangesToTargetFile:
        ret += "WrittingDownloadedBlockRangesToTargetFile";
        break;
    case CalculatingTargetFileSha1Hash:
        ret += "CalculatingTargetFileSha1Hash";
        break;
    case ConstructingTargetFile:
        ret += "ConstructingTargetFile";
        break;

    default:
        ret += "Unknown";
    }
    ret += ")";
    return ret;
}
