#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "ReadLine.h"

/* Pass in a pointer to a token list. Is freed and then set to null */
void FreeTokens(INOUTP char ***TokensPtr) {
	assert(*TokensPtr);
	assert(**TokensPtr);

	free(**TokensPtr); /* Free the string data */
	free(*TokensPtr); /* Free token list */
	*TokensPtr = NULL; /* Invalidate pointer since mem is gone */
}

/* Returns number of tokens in list. Zero if null list */
int CountTokens(INP char **Tokens) {
	int count = 0;

	if (NULL == Tokens) {
		return 0;
	};
	while (Tokens[count]) {
		++count;
	};
	return count;
}

/* Reads in a single line from file, splits into tokens and allocates
 * a list of tokens. Returns the an array of character arrays with the
 * final item being marked by an empty string.
 * Returns NULL on EOF
 * NB: Token list is does as two allocations, one for pointer list
 * and one for character array. Free what pointer points to and then
 * free the pointer itself */
char **
ReadLineTokens(INOUTP FILE * InFile, INOUTP int *LineNum) {

	enum {
		BUFFSIZE = 65536
	};
	/* This is much more than enough */
	char Buffer[BUFFSIZE]; /* Must match BUFFSIZE */
	char *Res;
	char *Last;
	char *Cur;
	char *Dst;
	char **Tokens;
	int TokenCount;
	int Len;
	int CurToken;
	bool InToken;

	do {
		/* Read the string */
		Res = fgets(Buffer, BUFFSIZE, InFile);
		if (NULL == Res) {
			if (feof(InFile)) {
				return NULL; /* Return NULL on EOF */
			} else {
				vpr_throw(VPR_ERROR_UNKNOWN, __FILE__, __LINE__,
					"Unexpected error reading file\n");
			}
		}
		++(*LineNum);

		/* Strip newline if any */
		Last = Buffer + strlen(Buffer);
		if ((Last > Buffer) && ('\n' == Last[-1])) {
			--Last;
		}
		if ((Last > Buffer) && ('\r' == Last[-1])) {
			--Last;
		}

		/* Handle continued lines */
		while ((Last > Buffer) && ('\\' == Last[-1])) {
			/* Strip off the backslash */
			--Last;

			/* Read next line by giving pointer to null-char as start for next */
			Res = fgets(Last, (BUFFSIZE - (Last - Buffer)), InFile);
			if (NULL == Res) {
				if (feof(InFile)) {
					return NULL; /* Return NULL on EOF */
				} else {
					vpr_throw(VPR_ERROR_UNKNOWN, __FILE__, __LINE__,
						"Unexpected error reading file\n");
				}
			}
			++(*LineNum);

			/* Strip newline */
			Last = Buffer + strlen(Buffer);
			if ((Last > Buffer) && ('\n' == Last[-1])) {
				--Last;
			}
			if ((Last > Buffer) && ('\r' == Last[-1])) {
				--Last;
			}
		}

		/* Strip comment if any */
		Cur = Buffer;
		while (Cur < Last) {
			if ('#' == *Cur) {
				Last = Cur;
				break;
			}
			++Cur;
		}

		/* Count tokens and find size */
		assert(Last < (Buffer + BUFFSIZE));
		Len = 0;
		TokenCount = 0;
		Cur = Buffer;
		InToken = false;
		while (Cur < Last) {
			if (InToken) {
				if ((' ' == *Cur) || ('\t' == *Cur)) {
					InToken = false;
				} else {
					++Len;
				}
			} else {
				if ((' ' != *Cur) && ('\t' != *Cur)) {
					++TokenCount;
					++Len;
					InToken = true;
				}
			}
			++Cur; /* Advance pointer */
		}
	} while (0 == TokenCount);

	/* Find the size of mem to alloc. Use a contiguous block so is 
	 * easy to deallocate */
	Len = (sizeof(char) * Len) + /* Length of actual data */
	(sizeof(char) * TokenCount); /* Null terminators */

	/* Alloc the pointer list and data list. Count the final 
	 * empty string we will use as list terminator */
	Tokens = (char **) my_malloc(sizeof(char *) * (TokenCount + 1));
	*Tokens = (char *) my_malloc(sizeof(char) * Len);

	/* Copy tokens to result */
	Cur = Buffer;
	Dst = *Tokens;
	InToken = false;
	CurToken = 0;
	while (Cur < Last) {
		if (InToken) {
			if ((' ' == *Cur) || ('\t' == *Cur)) {
				InToken = false;
				*Dst = '\0'; /* Null term token */
				++Dst;
				++CurToken;
			} else {
				*Dst = *Cur; /* Copy char */
				++Dst;
			}
		} else {
			if ((' ' != *Cur) && ('\t' != *Cur)) {
				Tokens[CurToken] = Dst; /* Set token start pointer */
				*Dst = *Cur; /* Copy char */
				++Dst;
				InToken = true;
			}
		}
		++Cur; /* Advance pointer */
	}
	if (InToken) {
		*Dst = '\0'; /* Null term final token */
		++Dst;
		++CurToken;
	}
	assert(CurToken == TokenCount);

	/* Set the final empty string entry */
	Tokens[CurToken] = NULL;

	/* Return the string list */
	return Tokens;
}

