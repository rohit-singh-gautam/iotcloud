////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <http11driver.hh>
#include <sstream>

rohit::http11driver::~http11driver()
{
   delete(scanner);
   scanner = nullptr;
   delete(parser);
   parser = nullptr;
}

void rohit::http11driver::parse(std::string &text) {
    std::stringstream textstream(text);
    parse_internal(textstream);
}

void rohit::http11driver::parse_internal(std::istream &textstream) {
    delete(scanner);
    try
    {
        scanner = new http11scanner( &textstream );
    }
    catch( std::bad_alloc &exception )
    {
        std::cerr << "Failed to allocate scanner: (" <<
            exception.what() << ")\n";
        exit( EXIT_FAILURE );
    }

    delete(parser); 
    try
    {
        parser = new rohit::parser(*scanner, *this);
    }
    catch( std::bad_alloc &ba )
    {
        std::cerr << "Failed to allocate parser: (" << 
            ba.what() << "), exiting!!\n";
        exit( EXIT_FAILURE );
    }

    scanner->BEGIN_REQUEST();
    const int accept( 0 );
    if( parser->parse() != accept )
    {
        std::cerr << "Parse failed!!\n";
    }
    return;
}

