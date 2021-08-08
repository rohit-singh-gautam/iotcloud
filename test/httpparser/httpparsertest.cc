#include <iostream>
#include "http11driver.hh"



int main() {
    const char header[] = 
        "GET /images/rohit.jpg HTTP/1.1\r\n"
        "Host: 172.24.201.159:8061\r\n"
        "Connection: keep-alive\r\n"
        "sec-ch-ua: ""Chromium"";v=""92"", "" Not A;Brand"";v=""99"", ""Microsoft Edge"";v=""92""\r\n"
        "sec-ch-ua-mobile: ?0\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.131 Safari/537.36 Edg/92.0.902.67\r\n"
        "Accept: image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8\r\n"
        "Sec-Fetch-Site: same-origin\r\n"
        "Sec-Fetch-Mode: no-cors\r\n"
        "Sec-Fetch-Dest: image\r\n"
        "Referer: https://172.24.201.159:8061/about.html\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: hi,en-IN;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n";

    const char header1[] = 
        "GET /about.html HTTP/1.1\r\n"
        "Accept-Language: hi,en-IN;q=0.9,en-US;q=0.8,en;q=0.7\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
        "If-None-Match: MaAAElR/YaB\r\n"
        "Upgrade: 1\r\n"
        "Cache-Control: max-age=0\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.131 Safari/537.36 Edg/92.0.902.67\r\n"
        "Connection: keep-alive\r\n"
        "Host: 172.24.201.159:8060\r\n\r\n";

    std::string headerstr(header);
    std::cout << "--- Original result \n" << headerstr << "\n --- Original end \n";

    rohit::http11driver driver;
    driver.parse(headerstr);

    std::cout << "--- Parsed result \n" << driver << "\n --- Parsed end \n";

    return EXIT_SUCCESS;
}