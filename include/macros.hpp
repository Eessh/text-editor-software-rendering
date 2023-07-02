#pragma once

#include "../log-boii/log_boii.h"

#ifdef DEBUG
#  define DEBUG_MODE 1
#else
#  define DEBUG_MODE 0
#endif

/// Shows trace logs only when DEBUG mode is active.
#define TRACE_BOII(...)                                                        \
  do                                                                           \
  {                                                                            \
    if(DEBUG_MODE)                                                             \
      log_trace(__VA_ARGS__);                                                  \
  } while(0)

/// Shows debug logs only when DEBUG mode is active.
#define DEBUG_BOII(...)                                                        \
  do                                                                           \
  {                                                                            \
    if(DEBUG_MODE)                                                             \
      log_debug(__VA_ARGS__);                                                  \
  } while(0)

/// Shows info logs only when DEBUG mode is active.
#define INFO_BOII(...)                                                         \
  do                                                                           \
  {                                                                            \
    if(DEBUG_MODE)                                                             \
      log_info(__VA_ARGS__);                                                   \
  } while(0)

/// Shows warn logs only when DEBUG mode is active.
#define WARN_BOII(...)                                                         \
  do                                                                           \
  {                                                                            \
    if(DEBUG_MODE)                                                             \
      log_warn(__VA_ARGS__);                                                   \
  } while(0)

/// Shows error logs only when DEBUG mode is active.
#define ERROR_BOII(...)                                                        \
  do                                                                           \
  {                                                                            \
    if(DEBUG_MODE)                                                             \
      log_error(__VA_ARGS__);                                                  \
  } while(0)

/// Shows fatal logs only when DEBUG mode is active.
#define FATAL_BOII(...)                                                        \
  do                                                                           \
  {                                                                            \
    if(DEBUG_MODE)                                                             \
      log_fatal(__VA_ARGS__);                                                  \
  } while(0)
