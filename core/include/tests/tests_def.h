#pragma once

#include <cstdio>

#define MESSAGE(MSG) fputs(#MSG, stdout);
#define MESSAGELN(MSG) fputs(#MSG "\n", stdout);
#define TEST(MSG, expr) MESSAGE(MSG ) {if (!expr) {MESSAGELN([OK]) return;}else{MESSAGELN([ERROR])}}
