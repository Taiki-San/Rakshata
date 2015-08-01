/* -*- mode: c; c-file-style: "k&r" -*-

	strnatcmp.c -- Perform 'natural order' comparisons of strings in C.
	Copyright (C) 2000, 2004 by Martin Pool <mbp sourcefrog net>

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/


/* partial change history:
 *
 * 2004-10-10 mbp: Lift out character type dependencies into macros.
 *
 * Eric Sosman pointed out that ctype functions take a parameter whose
 * value must be that of an unsigned int, even on platforms that have
 * negative chars in their default char type.
 *
 * 2015-07-29 Taiki: Tweak to get close of the convention of the project this code will be poured in, plus add support for prioritary chars
 */

#include <getopt.h>
#include <ctype.h>
#include <assert.h>

/* These are defined as macros to make it easier to adapt this code to
 * different characters types or comparison functions. */
static inline int nat_isdigit(const char a)
{
	return isdigit((unsigned char) a);
}

static inline int nat_isspace(const char a)
{
	return isspace((unsigned char) a);
}

static inline char nat_toupper(const char a)
{
	return toupper((unsigned char) a);
}

static inline bool isPrioritary(const char a)
{
	return a == '(' || a == '[' || a == '{'
		|| a == ')' || a == ']' || a == '}';
}

static int compare_right(const char *a, const char *b)
{
	int bias = 0;

	/* The longest run of digits wins.  That aside, the greatest
	value wins, but we can't know that it will until we've scanned
	both numbers to know that they have the same magnitude, so we
	remember it in BIAS. */
	for (;; a++, b++)
	{
		if (!nat_isdigit(*a)  &&  !nat_isdigit(*b))
			return bias;
		else if (!nat_isdigit(*a))
			return -1;
		else if (!nat_isdigit(*b))
			return +1;
		else if (*a < *b)
		{
			if (!bias)
				bias = -1;
		}
		else if (*a > *b)
		{
			if (!bias)
				bias = +1;
		}
		else if (!*a && !*b)
			return bias;
	}

	return 0;
}

static int compare_left(const char *a, const char *b)
{
	/* Compare two left-aligned numbers: the first to have a
		 different value wins. */
	for (;; a++, b++)
	{
		if (!nat_isdigit(*a)  &&  !nat_isdigit(*b))
			return 0;
		else if (!nat_isdigit(*a))
			return -1;
		else if (!nat_isdigit(*b))
			return 1;
		else if (*a < *b)
			return -1;
		else if (*a > *b)
			return 1;
	}

	return 0;
}

int _strnatcmp(const char *a, const char *b)
{
	char ca, cb;
	int result;

	if(a == NULL)
		return 1;
	else if(b == NULL)
		return -1;

	for(uint ai = 0, bi = 0; ; ++ai, ++bi)
	{
		ca = a[ai]; cb = b[bi];

		//Can we skip the lengthy comparaison?
		if(ca == cb && !nat_isdigit(ca))
			continue;

		//If one string end early
		if((ca == 0) != (cb == 0))
			return ca == 0 ? -1 : 1;

		//Skip over leading spaces or zeros
		while (nat_isspace(ca))
			ca = a[++ai];

		while (nat_isspace(cb))
			cb = b[++bi];

		//Process run of digits
		bool isDigitA = nat_isdigit(ca), isDigitB = nat_isdigit(cb);
		if (isDigitA && isDigitB)
		{
			if(ca == '0' || cb == '0')
			{
				if((result = compare_left(a+ai, b + bi)) != 0)
					return result;
			}
			else if ((result = compare_right(a + ai, b + bi)) != 0)
				return result;
		}

		//The strings compare the same, we try to break the tie
		if (!ca && !cb)
			return strcmp(a, b);

		if((ca = nat_toupper(ca)) == (cb = nat_toupper(cb)))
			continue;

		bool isPrioA = isPrioritary(ca), isPrioB = isPrioritary(cb);

		if((isPrioA && isDigitB) || (isPrioB && isDigitA) || isPrioA != isPrioB)
			return isPrioA ? -1 : 1;
		else if (ca < cb)
			return -1;
		else if (ca > cb)
			return 1;
	}
}

int strnatcmp(const void *a, const void *b)
{
	return _strnatcmp(* (char**) a, * (char**) b);
}
