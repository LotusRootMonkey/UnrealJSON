#pragma once

//#define DEBUG_JSONTOOLS 1
#define DEBUG_JSONTOOLS 0

#if DEBUG_JSONTOOLS
#include <iostream>
#define JSONTools_Line __LINE__
#define JSONTools_Debug(name,content){std::cout<<name<<" = "<<content<<std::endl;}
#else
#define JSONTools_Line 
#define JSONTools_Debug(name,content)
#endif
