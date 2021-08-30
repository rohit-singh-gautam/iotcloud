%{
#include <http11parser.hh>
#include <http11driver.hh>
#include <string>
#include <iostream>

#undef  YY_DECL
#define YY_DECL int rohit::http11scanner::yylex(rohit::parser::semantic_type * const lval, rohit::parser::location_type *loc)

using token = rohit::parser::token;

#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

#define tokVOID(tok) { return token::tok; }
#define tokINT(tok) { yylval->emplace<int>(std::stoi(yytext)); return token::tok; }
#define tokFLOAT(tok) { yylval->emplace<float>(std::stof(yytext)); return token::tok; }
#define tokSTRING(tok) { yylval->emplace<std::string>(yytext); return token::tok; }
#define tokCHAR(tok) { yylval->emplace<char>(*yytext); return token::tok; }
#define tokMETHOD(value) { yylval->emplace<rohit::http_header_request::METHOD>(rohit::http_header_request::METHOD::value); return token::METHOD; }
#define tokVERSION(value) { BEGIN(HEADER); yylval->emplace<rohit::http_header::VERSION>(rohit::http_header::VERSION::value); return token::VERSION; }
#define tokFIELD(value) { yylval->emplace<rohit::http_header::FIELD>(rohit::http_header::FIELD::value); return token::FIELD; }
#define tokFIELDCUSTOM(tok) { yylval->emplace<std::string>(yytext); return token::tok; }
#define tokFIELDVALUE(tok) { yylval->emplace<std::string>(skipFirstAndSpace(yytext)); return token::tok; }
#define tokFIELDCOLON(tok) { return token::tok; }

#define yyterminate() tokVOID(END)

%}

%option c++ nounput debug yylineno noyywrap batch
%option yyclass="rohit::http11scanner"
%x REQUEST
%x RESPONSE
%x HEADER
%x BODY

hex                 [0-9A-Fa-f]
escaped             "%"{hex}{2}
mark                "-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")"
unreserved          [[:alnum:]]|{mark}
reserved            ";"|"/"|"?"|":"|"@"|"&"|"="|"+"|"$"|","
uric                {reserved}|{unreserved}|{escaped}
uric_no_slash       {unreserved}|{escaped}|";"|"?"|":"|"@"|"&"|"="|"+"|"$"|","
opaque_part         {uric_no_slash}{uric}*
pchar               {unreserved}|{escaped}|":"|"@"|"&"|"="|"+"|"$"|","
param               {pchar}*
segment             {pchar}+(;{param})*
path_segments       {segment}("/"{segment})*
abs_path            "/"{path_segments}?
dec-octet           [0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5]
IPv4address         {dec-octet}\.{dec-octet}\.{dec-octet}\.{dec-octet}
port                [[:digit:]]+
scheme              [[:alpha:]]([[:alpha:]]|[[:digit:]]|"+"|"-"|".")*
reg_name            ({unreserved}|{escaped}|"$"|","|";"|":"|"@"|"&"|"="|"+")+
userinfo            ({unreserved}|{escaped}|";"|":"|"&"|"="|"+"|"$"|",")*
domainlabel         [:alnum:]|[:alnum:][[:alnum:]\-]*[:alnum:]
toplabel            [:alpha:]|[:alpha:][[:alnum:]\-]*[:alnum:]
hostname            ({domainlabel}".")*{toplabel}"."?
host                {hostname}|{IPv4address}
hostport            {host}(":"port)?
server              (({userinfo}"@")?{hostport})?
authority           {server}|{reg_name}
net_path            "//"{authority}{abs_path}?
query               {uric}*
hier_part           ({net_path}|{abs_path})("?"{query})?
absoluteURI         {scheme}":"({hier_part}|{opaque_part})
rel_segment         ({unreserved}|{escaped}|";"|"@"|"&"|"="|"+"|"$"|",")+
rel_path            {rel_segment}{abs_path}?
relativeURI         ({net_path}|{abs_path}|{rel_path})("?"{query})?
Request-URI         "*"|{absoluteURI}|{relativeURI}|{authority}             
field_value         [^\r\n]+
space               [ \t]+
separators          {space}|"("|")"|"<"|">"|"@"|","|";"|":"|"\"|<">|"/"|"["|"]"|"?"|"="|"{"|"}"
field_name          [^\:\r\n]+


%%
%{
    yylval = lval;
    loc->step();
    // https://www.iana.org/assignments/message-headers/message-headers.xhtml
%}

":"                             tokFIELDCOLON(COLON)

<REQUEST>"OPTIONS"              tokMETHOD(OPTIONS)
<REQUEST>"GET"                  tokMETHOD(GET)
<REQUEST>"HEAD"                 tokMETHOD(HEAD)
<REQUEST>"POST"                 tokMETHOD(POST)
<REQUEST>"PUT"                  tokMETHOD(PUT)
<REQUEST>"DELETE"               tokMETHOD(DELETE)
<REQUEST>"TRACE"                tokMETHOD(TRACE)
<REQUEST>"CONNECT"              tokMETHOD(CONNECT)
<REQUEST>"PRI"                  tokMETHOD(PRI)
<REQUEST>"HTTP/1.1"             tokVERSION(VER_1_1)
<REQUEST>"HTTP/2.0"               tokVERSION(VER_2)
<REQUEST>{Request-URI}          tokSTRING(PATH)

<HEADER>"Cache-Control"          tokFIELD(Cache_Control)
<HEADER>"Connection"             tokFIELD(Connection)
<HEADER>"Date"                   tokFIELD(Date)
<HEADER>"Pragma"                 tokFIELD(Pragma)
<HEADER>"Trailer"                tokFIELD(Trailer)
<HEADER>"Transfer-Encoding"      tokFIELD(Transfer_Encoding)
<HEADER>"Upgrade"                tokFIELD(Upgrade)
<HEADER>"Via"                    tokFIELD(Via)
<HEADER>"Warning"                tokFIELD(Warning)

<HEADER>"Accept"                 tokFIELD(Accept)
<HEADER>"Accept-Charset"         tokFIELD(Accept_Charset)
<HEADER>"Accept-Encoding"        tokFIELD(Accept_Encoding)
<HEADER>"Accept-Language"        tokFIELD(Accept_Language)
<HEADER>"Authorization"          tokFIELD(Authorization)
<HEADER>"Expect"                 tokFIELD(Expect)
<HEADER>"From"                   tokFIELD(From)
<HEADER>"Host"                   tokFIELD(Host)
<HEADER>"If-Match"               tokFIELD(If_Match)
<HEADER>"If-Modified-Since"      tokFIELD(If_Modified_Since)
<HEADER>"If-None-Match"          tokFIELD(If_None_Match)
<HEADER>"If-Range"               tokFIELD(If_Range)
<HEADER>"If-Unmodified-Since"    tokFIELD(If_Unmodified_Since)
<HEADER>"Max-Forwards"           tokFIELD(Max_Forwards)
<HEADER>"Proxy-Authorization"    tokFIELD(Proxy_Authorization)
<HEADER>"Range"                  tokFIELD(Range)
<HEADER>"Referer"                tokFIELD(Referer)
<HEADER>"TE"                     tokFIELD(TE)
<HEADER>"User-Agent"             tokFIELD(User_Agent)
<HEADER>"HTTP2-Settings"         tokFIELD(HTTP2_Settings)

<HEADER>"Accept-Ranges"          tokFIELD(Accept_Ranges)
<HEADER>"Age"                    tokFIELD(Age)
<HEADER>"ETag"                   tokFIELD(ETag)
<HEADER>"Location"               tokFIELD(Location)
<HEADER>"Proxy-Authenticate"     tokFIELD(Proxy_Authenticate)
<HEADER>"Retry-After"            tokFIELD(Retry_After)
<HEADER>"Server"                 tokFIELD(Server)
<HEADER>"Vary"                   tokFIELD(Vary)
<HEADER>"WWW-Authenticate"       tokFIELD(WWW_Authenticate)

<HEADER>"Allow"                  tokFIELD(Allow)
<HEADER>"Content-Encoding"       tokFIELD(Content_Encoding)
<HEADER>"Content-Language"       tokFIELD(Content_Language)
<HEADER>"Content-Length"         tokFIELD(Content_Length)
<HEADER>"Content-Location"       tokFIELD(Content_Location)
<HEADER>"Content-MD5"            tokFIELD(Content_MD5)
<HEADER>"Content-Range"          tokFIELD(Content_Range)
<HEADER>"Content-Type"           tokFIELD(Content_Type)
<HEADER>"Expires"                tokFIELD(Expires)
<HEADER>"Last-Modified"          tokFIELD(Last_Modified)

<HEADER>"A-IM"                   tokFIELD(A_IM)
<HEADER>"Accept-Additions"       tokFIELD(Accept_Additions)
<HEADER>"Accept-CH"              tokFIELD(Accept_CH)
<HEADER>"Accept-Datetime"        tokFIELD(Accept_Datetime)
<HEADER>"Accept-Features"        tokFIELD(Accept_Features)
<HEADER>"Accept-Patch"           tokFIELD(Accept_Patch)
<HEADER>"Accept-Post"            tokFIELD(Accept_Post)
<HEADER>"ALPN"                   tokFIELD(ALPN)
<HEADER>"Also-Control"           tokFIELD(Also_Control)
<HEADER>"Alt-Svc"                tokFIELD(Alt_Svc)
<HEADER>"Alt-Used"               tokFIELD(Alt_Used)
<HEADER>"Alternates"             tokFIELD(Alternates)
<HEADER>"Apply-To-Redirect-Ref"  tokFIELD(Apply_To_Redirect_Ref)
<HEADER>"Authentication-Control" tokFIELD(Authentication_Control)
<HEADER>"Authentication-Info"    tokFIELD(Authentication_Info)
<HEADER>"C-Ext"                  tokFIELD(C_Ext)
<HEADER>"C-Man"                  tokFIELD(C_Man)
<HEADER>"C-Opt"                  tokFIELD(C_Opt)
<HEADER>"C-PEP"                  tokFIELD(C_PEP)
<HEADER>"C-PEP-Info"             tokFIELD(C_PEP_Info)
<HEADER>"Cal-Managed-ID"         tokFIELD(Cal_Managed_ID)
<HEADER>"CalDAV-Timezones"       tokFIELD(CalDAV_Timezones)
<HEADER>"CDN-Loop"               tokFIELD(CDN_Loop)
<HEADER>"Cert-Not-After"         tokFIELD(Cert_Not_After)
<HEADER>"Cert-Not-Before"        tokFIELD(Cert_Not_Before)
<HEADER>"Close"                  tokFIELD(Close)
<HEADER>"Content-Base"           tokFIELD(Content_Base)
<HEADER>"Content-Disposition"    tokFIELD(Content_Disposition)
<HEADER>"Content-ID"             tokFIELD(Content_ID)
<HEADER>"Content-Script-Type"    tokFIELD(Content_Script_Type)
<HEADER>"Content-Style-Type"     tokFIELD(Content_Style_Type)
<HEADER>"Content-Version"        tokFIELD(Content_Version)
<HEADER>"Cookie"                 tokFIELD(Cookie)
<HEADER>"Cookie2"                tokFIELD(Cookie2)
<HEADER>"DASL"                   tokFIELD(DASL)
<HEADER>"DAV"                    tokFIELD(DAV)
<HEADER>"Default-Style"          tokFIELD(Default_Style)
<HEADER>"Delta-Base"             tokFIELD(Delta_Base)
<HEADER>"Depth"                  tokFIELD(Depth)
<HEADER>"Derived-From"           tokFIELD(Derived_From)
<HEADER>"Destination"            tokFIELD(Destination)
<HEADER>"Differential-ID"        tokFIELD(Differential_ID)
<HEADER>"Digest"                 tokFIELD(Digest)
<HEADER>"Early-Data"             tokFIELD(Early_Data)
<HEADER>"Expect-CT"              tokFIELD(Expect_CT)
<HEADER>"Ext"                    tokFIELD(Ext)
<HEADER>"Forwarded"              tokFIELD(Forwarded)
<HEADER>"GetProfile"             tokFIELD(GetProfile)
<HEADER>"Hobareg"                tokFIELD(Hobareg)
<HEADER>"IM"                     tokFIELD(IM)
<HEADER>"If"                     tokFIELD(If)
<HEADER>"If-Schedule-Tag-Match"  tokFIELD(If_Schedule_Tag_Match)
<HEADER>"Include-Referred-Token-Binding-ID"       tokFIELD(Include_Referred_Token_Binding_ID)
<HEADER>"Keep-Alive"             tokFIELD(Keep_Alive)
<HEADER>"Label"                  tokFIELD(Label)
<HEADER>"Link"                   tokFIELD(Link)
<HEADER>"Lock-Token"             tokFIELD(Lock_Token)
<HEADER>"Man"                    tokFIELD(Man)
<HEADER>"Memento-Datetime"       tokFIELD(Memento_Datetime)
<HEADER>"Meter"                  tokFIELD(Meter)
<HEADER>"MIME-Version"           tokFIELD(MIME_Version)
<HEADER>"Negotiate"              tokFIELD(Negotiate)
<HEADER>"OData-EntityId"         tokFIELD(OData_EntityId)
<HEADER>"OData-Isolation"        tokFIELD(OData_Isolation)
<HEADER>"OData-MaxVersion"       tokFIELD(OData_MaxVersion)
<HEADER>"OData-Version"          tokFIELD(OData_Version)
<HEADER>"Opt"                    tokFIELD(Opt)
<HEADER>"Optional-WWW-Authenticate"       tokFIELD(Optional_WWW_Authenticate)
<HEADER>"Ordering-Type"          tokFIELD(Ordering_Type)
<HEADER>"Origin"                 tokFIELD(Origin)
<HEADER>"OSCORE"                 tokFIELD(OSCORE)
<HEADER>"Overwrite"              tokFIELD(Overwrite)
<HEADER>"P3P"                    tokFIELD(P3P)
<HEADER>"PEP"                    tokFIELD(PEP)
<HEADER>"PICS-Label"             tokFIELD(PICS_Label)
<HEADER>"Pep-Info"               tokFIELD(Pep_Info)
<HEADER>"Position"               tokFIELD(Position)
<HEADER>"Prefer"                 tokFIELD(Prefer)
<HEADER>"Preference-Applied"     tokFIELD(Preference_Applied)
<HEADER>"ProfileObject"          tokFIELD(ProfileObject)
<HEADER>"Protocol"               tokFIELD(Protocol)
<HEADER>"Protocol-Info"          tokFIELD(Protocol_Info)
<HEADER>"Protocol-Query"         tokFIELD(Protocol_Query)
<HEADER>"Protocol-Request"       tokFIELD(Protocol_Request)
<HEADER>"Proxy-Authentication-Info"       tokFIELD(Proxy_Authentication_Info)
<HEADER>"Proxy-Features"         tokFIELD(Proxy_Features)
<HEADER>"Proxy-Instruction"      tokFIELD(Proxy_Instruction)
<HEADER>"Public"                 tokFIELD(Public)
<HEADER>"Public-Key-Pins"        tokFIELD(Public_Key_Pins)
<HEADER>"Public-Key-Pins-Report-Only"       tokFIELD(Public_Key_Pins_Report_Only)
<HEADER>"Redirect-Ref"           tokFIELD(Redirect_Ref)
<HEADER>"Replay-Nonce"           tokFIELD(Replay_Nonce)
<HEADER>"Safe"                   tokFIELD(Safe)
<HEADER>"Schedule-Reply"         tokFIELD(Schedule_Reply)
<HEADER>"Schedule-Tag"           tokFIELD(Schedule_Tag)
<HEADER>"Sec-Token-Binding"      tokFIELD(Sec_Token_Binding)
<HEADER>"Sec-WebSocket-Accept"   tokFIELD(Sec_WebSocket_Accept)
<HEADER>"Sec-WebSocket-Extensions"       tokFIELD(Sec_WebSocket_Extensions)
<HEADER>"Sec-WebSocket-Key"      tokFIELD(Sec_WebSocket_Key)
<HEADER>"Sec-WebSocket-Protocol" tokFIELD(Sec_WebSocket_Protocol)
<HEADER>"Sec-WebSocket-Version"  tokFIELD(Sec_WebSocket_Version)
<HEADER>"Security-Scheme"        tokFIELD(Security_Scheme)
<HEADER>"Set-Cookie"             tokFIELD(Set_Cookie)
<HEADER>"Set-Cookie2"            tokFIELD(Set_Cookie2)
<HEADER>"SetProfile"             tokFIELD(SetProfile)
<HEADER>"SLUG"                   tokFIELD(SLUG)
<HEADER>"SoapAction"             tokFIELD(SoapAction)
<HEADER>"Status-URI"             tokFIELD(Status_URI)
<HEADER>"Strict-Transport-Security"       tokFIELD(Strict_Transport_Security)
<HEADER>"Sunset"                 tokFIELD(Sunset)
<HEADER>"Surrogate-Capability"   tokFIELD(Surrogate_Capability)
<HEADER>"Surrogate-Control"      tokFIELD(Surrogate_Control)
<HEADER>"TCN"                    tokFIELD(TCN)
<HEADER>"Timeout"                tokFIELD(Timeout)
<HEADER>"Topic"                  tokFIELD(Topic)
<HEADER>"TTL"                    tokFIELD(TTL)
<HEADER>"Urgency"                tokFIELD(Urgency)
<HEADER>"URI"                    tokFIELD(URI)
<HEADER>"Variant-Vary"           tokFIELD(Variant_Vary)
<HEADER>"Want-Digest"            tokFIELD(Want_Digest)
<HEADER>"X-Content-Type-Options" tokFIELD(X_Content_Type_Options)
<HEADER>"X-Frame-Options"        tokFIELD(X_Frame_Options)

<HEADER>{field_name}             tokFIELDCUSTOM(FIELD_CUSTOM)

<HEADER>":"{space}{field_value}  tokFIELDVALUE(FIELD_VALUE)


<INITIAL,REQUEST,HEADER>("\r"|"\r\n")   { loc->lines(); tokVOID(NEWLINE) }
<INITIAL,REQUEST>{space}                tokVOID(SPACE)

.               tokCHAR(CHAR)

%%

void rohit::http11scanner::BEGIN_REQUEST() {
    BEGIN(REQUEST);
}
