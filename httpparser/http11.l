%{
#include <http11parser.hh>
#include <http11driver.hh>
#include <string>
#include <iostream>

#undef  YY_DECL
#define YY_DECL int iotcloud::http11scanner::yylex(iotcloud::parser::semantic_type * const lval, iotcloud::parser::location_type *loc)

using token = iotcloud::parser::token;

#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

#define tokVOID(tok) { return token::tok; }
#define tokINT(tok) { yylval->emplace<int>(std::stoi(yytext)); return token::tok; }
#define tokFLOAT(tok) { yylval->emplace<float>(std::stof(yytext)); return token::tok; }
#define tokSTRING(tok) { yylval->emplace<std::string>(yytext); return token::tok; }
#define tokCHAR(tok) { yylval->emplace<char>(*yytext); return token::tok; }
#define tokMETHOD(value) { yylval->emplace<http_header_request::METHOD>(http_header_request::METHOD::value); return token::METHOD; }
#define tokVERSION(value) { BEGIN(HEADER); yylval->emplace<http_header::VERSION>(http_header::VERSION::value); return token::VERSION; }
#define tokFIELD(value) { yylval->emplace<http_header::FIELD>(http_header::FIELD::value); return token::FIELD; }
#define tokFIELDVALUE(tok) { yylval->emplace<std::string>(skipFirstAndSpace(yytext)); return token::tok; }
#define tokFIELDCOLON(tok) { return token::tok; }

#define yyterminate() tokVOID(END)

%}

%option c++ nounput debug yylineno noyywrap batch
%option yyclass="iotcloud::http11scanner"
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


%%
%{
    yylval = lval;
    loc->step();
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
<REQUEST>"HTTP/1.1"             tokVERSION(VER_1_1)
<REQUEST>"HTTP/2"               tokVERSION(VER_2)
<REQUEST>"HTTP/3"               tokVERSION(VER_3)
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

<HEADER>":"{space}{field_value}  tokFIELDVALUE(FIELD_VALUE)


<INITIAL,REQUEST,HEADER>("\r"|"\r\n")   { loc->lines(); tokVOID(NEWLINE) }
<INITIAL,REQUEST>{space}                tokVOID(SPACE)

.               tokCHAR(CHAR)

%%

void iotcloud::http11scanner::BEGIN_REQUEST() {
    BEGIN(REQUEST);
}
