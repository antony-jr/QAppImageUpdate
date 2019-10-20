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

QString AppImageUpdaterBridge::errorCodeToString(short errorCode) {
    QString ret = "AppImageUpdaterBridge::errorCode(";
    switch(errorCode) {
    case NoAppimagePathGiven:
        ret += "NoAppImagePathGiven";
        break;
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
    case ZsyncControlFileNotFound:
        ret += "ZsyncControlFileNotFound";
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

QString AppImageUpdaterBridge::errorCodeToDescriptionString(short errorCode) {
    QString errorString;
    switch(errorCode) {
    case ConnectionRefusedError:
        errorString = QString::fromUtf8("The update server is not accepting requests.");
        break;
    case RemoteHostClosedError:
        errorString = QString::fromUtf8("The remote server closed the connection prematurely, ");
        errorString += QString::fromUtf8("before the entire reply was received and processed.");
        break;
    case HostNotFoundError:
        errorString = QString::fromUtf8("The remote host name was not found (invalid hostname).");
        break;
    case TimeoutError:
        errorString = QString::fromUtf8("The connection to the remote server timed out.");
        break;
    case SslHandshakeFailedError:
        errorString = QString::fromUtf8("The SSL/TLS handshake failed and the encrypted channel ");
        errorString += QString::fromUtf8("could not be established.");
        break;
    case TemporaryNetworkFailureError:
        errorString = QString::fromUtf8("The connection to the network was broken.");
        break;
    case NetworkSessionFailedError:
        errorString = QString::fromUtf8("The connection to the network was broken ");
        errorString += QString::fromUtf8("or could not be initiated.");
        break;
    case BackgroundRequestNotAllowedError:
        errorString = QString::fromUtf8("The background request is not currently allowed due to platform policy.");
        break;
    case TooManyRedirectsError:
        errorString = QString::fromUtf8("While following redirects, the maximum limit was reached.");
        break;
    case InsecureRedirectError:
        errorString = QString::fromUtf8("While following redirects, there was a redirect ");
        errorString += QString::fromUtf8("from a encrypted protocol (https) to an unencrypted one (http).");
        break;
    case ContentAccessDenied:
        errorString = QString::fromUtf8("The access to the remote content was denied (HTTP error 403).");
        break;
    case ContentOperationNotPermittedError:
        errorString = QString::fromUtf8("The operation requested on the remote content is not permitted.");
        break;
    case ContentNotFoundError:
        errorString = QString::fromUtf8("The remote content was not found at the server (HTTP error 404)");
        break;
    case AuthenticationRequiredError:
        errorString = QString::fromUtf8("The remote server requires authentication to serve the content, ");
        errorString += QString::fromUtf8("but the credentials provided were not accepted or given.");
        break;
    case ContentConflictError:
        errorString = QString::fromUtf8("The request could not be completed due to a conflict with the ");
        errorString += QString::fromUtf8("current state of the resource.");
        break;
    case ContentGoneError:
        errorString = QString::fromUtf8("The requested resource is no longer available at the server.");
        break;
    case InternalServerError:
        errorString = QString::fromUtf8("The server encountered an unexpected condition which prevented ");
        errorString += QString::fromUtf8("it from fulfilling the request.");
        break;
    case OperationNotImplementedError:
        errorString = QString::fromUtf8("The server does not support the functionality required to fulfill the request.");
        break;
    case ServiceUnavailableError:
        errorString = QString::fromUtf8("The server is unable to handle the request at this time.");
        break;
    case ProtocolUnknownError:
        errorString = QString::fromUtf8("The Network Access API cannot honor the request because the protocol");
        errorString += QString::fromUtf8(" is not known.");
        break;
    case ProtocolInvalidOperationError:
        errorString = QString::fromUtf8("The requested operation is invalid for this protocol.");
        break;
    case UnknownNetworkError:
        errorString = QString::fromUtf8("An unknown network-related error was detected.");
        break;
    case UnknownContentError:
        errorString = QString::fromUtf8("An unknown error related to the remote content was detected.");
        break;
    case ProtocolFailure:
        errorString = QString::fromUtf8("A breakdown in protocol was detected ");
        errorString += QString::fromUtf8("(parsing error, invalid or unexpected responses, etc.)");
        break;
    case UnknownServerError:
        errorString = QString::fromUtf8("An unknown error related to the server response was detected.");
        break;
    case NoAppimagePathGiven:
        errorString = QString::fromUtf8("No AppImage given.");
        break;
    case AppimageNotReadable:
        errorString = QString::fromUtf8("The AppImage is not readable.");
        break;
    case NoReadPermission:
        errorString = QString::fromUtf8("You don't have the permission to read the AppImage.");
        break;
    case AppimageNotFound:
        errorString = QString::fromUtf8("The AppImage does not exist.");
        break;
    case CannotOpenAppimage:
        errorString = QString::fromUtf8("The AppImage cannot be opened.");
        break;
    case EmptyUpdateInformation:
        errorString = QString::fromUtf8("The AppImage does not include any update information.");
        break;
    case InvalidAppimageType:
        errorString = QString::fromUtf8("The AppImage has an unknown type.");
        break;
    case InvalidMagicBytes:
        errorString = QString::fromUtf8("The AppImage is not valid.");
        break;
    case InvalidUpdateInformation:
        errorString = QString::fromUtf8("The AppImage has invalid update information.");
        break;
    case NotEnoughMemory:
        errorString = QString::fromUtf8("Not enough memory.");
        break;
    case SectionHeaderNotFound:
        errorString = QString::fromUtf8("The AppImage does not contain update information ");
        errorString += QString::fromUtf8("at a valid section header.");
        break;
    case UnsupportedElfFormat:
        errorString = QString::fromUtf8("The AppImage is not in supported ELF format.");
        break;
    case UnsupportedTransport:
        errorString = QString::fromUtf8("The AppImage specifies an unsupported update transport.");
        break;
    case IoReadError:
        errorString = QString::fromUtf8("Unknown IO read error.");
        break;
    case GithubApiRateLimitReached:
        errorString = QString::fromUtf8("GitHub API rate limit reached, please try again later.");
        break;
    case ErrorResponseCode:
        errorString = QString::fromUtf8("Bad response from the server, please try again later.");
        break;
    case NoMarkerFoundInControlFile:
    case InvalidZsyncHeadersNumber:
    case InvalidZsyncMakeVersion:
    case InvalidZsyncTargetFilename:
    case InvalidZsyncMtime:
    case InvalidZsyncBlocksize:
    case InvalidTargetFileLength:
    case InvalidHashLengthLine:
    case InvalidHashLengths:
    case InvalidTargetFileUrl:
    case InvalidTargetFileSha1:
    case HashTableNotAllocated:
    case InvalidTargetFileChecksumBlocks:
    case CannotOpenTargetFileChecksumBlocks:
    case CannotConstructHashTable:
    case QbufferIoReadError:
        errorString = QString::fromUtf8("Invalid zsync meta file.");
        break;
    case ZsyncControlFileNotFound:
        errorString = QString::fromUtf8("The zsync control file was not found in the specified location.");
        break;
    case SourceFileNotFound:
        errorString = QString::fromUtf8("The current AppImage could not be found, maybe it was deleted while updating?");
        break;
    case NoPermissionToReadSourceFile:
        errorString = QString::fromUtf8("You don't have the permission to read the current AppImage.");
        break;
    case CannotOpenSourceFile:
        errorString = QString::fromUtf8("The current AppImage cannot be opened.");
        break;
    case NoPermissionToReadWriteTargetFile:
        errorString = QString::fromUtf8("You have no write or read permissions for the new version.");
        break;
    case CannotOpenTargetFile:
        errorString = QString::fromUtf8("The new version cannot be opened to write or read.");
        break;
    case TargetFileSha1HashMismatch:
        errorString = QString::fromUtf8("The newly constructed AppImage failed the integrity check, please try again.");
        break;
    default:
        errorString = QString::fromUtf8("Unknown error.");
        break;
    }
    return errorString;
}

QString AppImageUpdaterBridge::statusCodeToString(short code) {
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
