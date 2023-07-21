#pragma once

// Get program invocation short name.
const char *get_invoc_short_name();

// Set Delphes runtime environment.
void setenv_delphes();

// Force line buffer on output streams.
void setlbf();

#define ignore_value(expr)  ({ __attribute__((unused)) auto _ = (expr); })
