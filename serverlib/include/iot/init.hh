////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

namespace rohit {

void init_iot(const char *logfilename, const int thread_count = 0);
void destroy_iot();

} // namespace rohit