// window_layout.h
#pragma once

const int WIN_OUTPUT_TOP        = 0;
const int WIN_OUTPUT_LEFT       = 0;
const int WIN_OUTPUT_HEIGHT     = 18;
const int WIN_OUTPUT_WIDTH      = 128;
                  
const int WIN_STENO_TOP         = WIN_OUTPUT_HEIGHT;
const int WIN_STENO_LEFT        = 0;
const int WIN_STENO_HEIGHT      = 20;
const int WIN_STENO_WIDTH       = 27;

const int WIN_TRANSLATOR_TOP    = WIN_OUTPUT_HEIGHT;
const int WIN_TRANSLATOR_LEFT   = WIN_STENO_WIDTH;
const int WIN_TRANSLATOR_HEIGHT = WIN_STENO_HEIGHT;
const int WIN_TRANSLATOR_WIDTH  = 36;

const int WIN_LOG_TOP           = WIN_OUTPUT_HEIGHT;
const int WIN_LOG_LEFT          = WIN_STENO_WIDTH + WIN_TRANSLATOR_WIDTH;
const int WIN_LOG_HEIGHT        = WIN_STENO_HEIGHT;
const int WIN_LOG_WIDTH         = WIN_OUTPUT_WIDTH - ( WIN_STENO_WIDTH + WIN_TRANSLATOR_WIDTH );
