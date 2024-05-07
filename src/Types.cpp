#include "Types.hpp"

std::map<std::string, std::string>  Types::MimeTypes;
Types Types::singleton_;

Types::Types( void )
{   
    MimeTypes["java-archive"] = "application/java-archive";
    MimeTypes["EDI-X12"] = "application/EDI-X12";
    MimeTypes["EDIFACT"] = "application/EDIFACT";
    MimeTypes["javascript"] = "application/javascript";
    MimeTypes["octet-stream"] = "application/octet-stream";
    MimeTypes["ogg"] = "application/ogg";
    MimeTypes["pdf"] = "application/pdf";
    MimeTypes["xhtml+xml"] = "application/xhtml+xml";
    MimeTypes["x-shockwave-flash"] = "application/x-shockwave-flash";
    MimeTypes["json"] = "application/json";
    MimeTypes["ld+json"] = "application/ld+json";
    MimeTypes["xml"] = "application/xml";
    MimeTypes["zip"] = "application/zip";
    MimeTypes["x-www-form-urlencoded"] = "application/x-www-form-urlencoded";
    MimeTypes["mpeg"] = "audio/mpeg";
    MimeTypes["x-ms-wma"] = "audio/x-ms-wma";
    MimeTypes["vnd.rn-realaudio"] = "audio/vnd.rn-realaudio";
    MimeTypes["x-wav"] = "audio/x-wav";
    MimeTypes["gif"] = "image/gif";
    MimeTypes["jpeg"] = "image/jpeg";
    MimeTypes["png"] = "image/png";
    MimeTypes["tiff"] = "image/tiff";
    MimeTypes["vnd.microsoft.icon"] = "image/vnd.microsoft.icon";
    MimeTypes["x-icon"] = "image/x-icon";
    MimeTypes["vnd.djvu"] = "image/vnd.djvu";
    MimeTypes["svg+xml"] = "image/svg+xml";
    MimeTypes["mixed"] = "multipart/mixed";
    MimeTypes["alternative"] = "multipart/alternative";
    MimeTypes["related"] = "multipart/related";
    MimeTypes["form-data"] = "multipart/form-data";
    MimeTypes["css"] = "text/css";
    MimeTypes["csv"] = "text/csv";
    MimeTypes["html"] = "text/html";
    MimeTypes["javascript"] = "text/javascript";
    MimeTypes["plain"] = "text/plain";
    MimeTypes["xml"] = "text/xml";
    MimeTypes["mpeg"] = "video/mpeg";
    MimeTypes["mp4"] = "video/mp4";
    MimeTypes["quicktime"] = "video/quicktime";
    MimeTypes["x-ms-wmv"] = "video/x-ms-wmv";
    MimeTypes["x-msvideo"] = "video/x-msvideo";
    MimeTypes["x-flv"] = "video/x-flv";
    MimeTypes["webm"] = "video/webm";
    MimeTypes["vnd.android.package-archive"] = "application/vnd.android.package-archive";
    MimeTypes["vnd.oasis.opendocument.text"] = "application/vnd.oasis.opendocument.text";
    MimeTypes["vnd.oasis.opendocument.spreadsheet"] = "application/vnd.oasis.opendocument.spreadsheet";
    MimeTypes["vnd.oasis.opendocument.presentation"] = "application/vnd.oasis.opendocument.graphics";
    MimeTypes["vnd.oasis.opendocument.graphics"] = "application/vnd.oasis.opendocument.graphics";
    MimeTypes["vnd.ms-excel"] = "application/vnd.ms-excel";
    MimeTypes["vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    MimeTypes["vnd.ms-powerpoint"] = "application/vnd.ms-powerpoint";
    MimeTypes["vnd.openxmlformats-officedocument.presentationml.presentation"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    MimeTypes["msword"] = "application/msword";
    MimeTypes["vnd.openxmlformats-officedocument.wordprocessingml.document"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    MimeTypes["vnd.mozilla.xul+xml"] = "application/vnd.mozilla.xul+xml";
    MimeTypes[""] = "text/html";
}


Types &Types::GetInstance() {return (singleton_);}

Types::~Types() {};