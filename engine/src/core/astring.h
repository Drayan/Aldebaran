#pragma once

#include "defines.h"

// Return the length of the given string.
AAPI u64 string_length(const char *str);

AAPI char *string_duplicate(const char *str);

/**
 * Case-sensitive string comparaison.
 * @param str0 The first string to compare.
 * @param str1 The second string to compare.
 * @return TRUE if the same, otherwise FALSE.
 */
AAPI b8 strings_equal(const char *str0, const char *str1);