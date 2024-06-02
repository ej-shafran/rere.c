#ifndef BI_H_
#define BI_H_

#ifndef BI_FILE
#include <stdio.h>
#define BI_FILE FILE *
#endif // BI_FILE

#ifndef BI_FPRINTF
#include <stdio.h>
#define BI_FPRINTF fprintf
#endif // BI_FILE

#ifndef BI_SSIZE
#include <stddef.h>
#define BI_SSIZE ssize_t
#endif // BI_SSIZE

#ifndef BI_GETLINE
#include <stdio.h>
#define BI_GETLINE getline
#endif // BI_GETLINE

#ifndef BI_INT
#define BI_INT int
#endif // BI_INT

#ifndef BI_STRING
#define BI_STRING char *
#endif // BI_STRING

#ifndef BI_STRLEN
#include <string.h>
#define BI_STRLEN strlen
#endif // BI_STRLEN

#ifndef BI_ATOI
#include <stdlib.h>
#define BI_ATOI atoi
#endif // BI_ATOI

#ifndef BI_ASSERT
#include <assert.h>
#define BI_ASSERT assert
#endif // BI_ASSERT

#ifndef BI_FGETC
#include <stdio.h>
#define BI_FGETC fgetc
#endif // BI_FGETC

#ifndef BI_EOF
#include <stdio.h>
#define BI_EOF EOF
#endif // BI_EOF

typedef enum BI_PARSE_ERROR {
	BI_SUCCESS = 0,
	BI_INVALID_PREFIX = 1,
	BI_INCORRECT_NAME = 2,
	BI_INVALID_SUFFIX = 3,
	BI_EARLY_EOF = 4,
} BI_PARSE_ERROR;

// Write an integer field into the specified file in BI format.
//
// RETURNS: amount of bytes written into the file;
//          a negative number if an error occurred calling `BI_FPRINTF`.
int bi_write_int_field(BI_FILE f, BI_STRING name, BI_INT value);
// Write a blob field into the specified file in BI format.
//
// RETURNS: amount of bytes written into the file;
//          a negative number if an error occurred calling `BI_FPRINTF`.
int bi_write_blob_field(BI_FILE f, BI_STRING name, BI_STRING value);
// Read an integer field from the next available lines in the specified BI-formatted file.
//
// The `out` parameter is a pointer which will be updated with the integer value of the field specified by `name`.
// The `nread` parameter will be updated with the amount of characters read from `f`.
//
// RETURNS: BI_SUCCESS if parsing the field was successful;
//          BI_INVALID_PREFIX if the field's prefix is invalid either in length or content;
//          BI_INCORRECT_NAME if the current line's name is not the one specified by `name`;
//          BI_INVALID_SUFFIX if there is not a value after the name, separated by a space.
//
// In the case of an error, `out` will not be set, but `nread` will.
BI_PARSE_ERROR bi_read_int_field(BI_FILE f, BI_STRING name, BI_INT *out,
				 BI_SSIZE *nread);

// Read an integer field from the next available lines in the specified BI-formatted file.
//
// The `out` parameter is a pointer which will be updated with the string value of the field specified by `name`.
// It must be a valid, non-null pointer to available memory.
// The `nread` parameter will be updated with the amount of characters read from `f`.
//
// RETURNS: BI_SUCCESS if parsing the field was successful;
//          BI_INVALID_PREFIX if the field's prefix is invalid either in length or content;
//          BI_INCORRECT_NAME if the current line's name is not the one specified by `name`;
//          BI_INVALID_SUFFIX if there is not a length after the name, separated by a space;
//          BI_EARLY_EOF if the file ended before the length of the field specified it would.
//
// In the case of an error, `out` will not be set, but `nread` will.
BI_PARSE_ERROR bi_read_blob_field(BI_FILE f, BI_STRING name, BI_STRING *out,
				  BI_SSIZE *nread);

#define BI_INT_PREFIX_LITERAL ":i "
#define BI_INT_PREFIX_LEN 3

#define BI_BLOB_PREFIX_LITERAL ":b "
#define BI_BLOB_PREFIX_LEN 3

#ifdef BI_IMPLEMENTATION

int bi_write_int_field(BI_FILE f, BI_STRING name, BI_INT value)
{
	return BI_FPRINTF(f, BI_INT_PREFIX_LITERAL "%s %d\n", name, value);
}

int bi_write_blob_field(BI_FILE f, BI_STRING name, BI_STRING value)
{
	int a = BI_FPRINTF(f, BI_BLOB_PREFIX_LITERAL "%s %lu\n", name,
			   strlen(value));
	if (a < 0)
		return a;
	int b = BI_FPRINTF(f, "%s\n", value);
	if (b < 0)
		return b;
	return a + b;
}

BI_PARSE_ERROR bi_read_int_field(BI_FILE f, BI_STRING name, BI_INT *out,
				 BI_SSIZE *nread)
{
	char *line = NULL;
	size_t len = 0;
	BI_SSIZE read = BI_GETLINE(&line, &len, f);
	*nread = read;

	int namelen = BI_STRLEN(name);

	// :i <name> <digits>\n
	// ~~~      ^^       ^
	// At least 3 + prefix characters bigger than <name>
	if (len < namelen + 3 + BI_INT_PREFIX_LEN)
		return BI_INVALID_PREFIX;

	int i = 0;
	for (; i < BI_INT_PREFIX_LEN; i++) {
		if (line[i] != BI_INT_PREFIX_LITERAL[i])
			return BI_INVALID_PREFIX;
	}
	for (; i - BI_INT_PREFIX_LEN < namelen; i++) {
		if (line[i] != name[i - BI_INT_PREFIX_LEN])
			return BI_INCORRECT_NAME;
	}
	if (line[i++] != ' ')
		return BI_INVALID_SUFFIX;

	// TODO: handle `errno`? use `strtol`?
	*out = BI_ATOI(line + i);

	return BI_SUCCESS;
}

BI_PARSE_ERROR bi_read_blob_field(BI_FILE f, BI_STRING name, BI_STRING *out,
				  BI_SSIZE *nread)
{
	BI_ASSERT(out);

	char *line = NULL;
	size_t len = 0;
	BI_SSIZE read = BI_GETLINE(&line, &len, f);
	*nread = read;

	int namelen = BI_STRLEN(name);

	// :b <name> <length>\n
	// ~~~      ^^       ^
	// At least 3 + prefix characters bigger than <name>
	if (len < namelen + 3 + BI_BLOB_PREFIX_LEN)
		return BI_INVALID_PREFIX;

	int i = 0;
	for (; i < BI_BLOB_PREFIX_LEN; i++) {
		if (line[i] != BI_BLOB_PREFIX_LITERAL[i])
			return BI_INVALID_PREFIX;
	}
	for (; i - BI_BLOB_PREFIX_LEN < namelen; i++) {
		if (line[i] != name[i - BI_BLOB_PREFIX_LEN])
			return BI_INCORRECT_NAME;
	}
	if (line[i++] != ' ')
		return BI_INVALID_SUFFIX;

	// TODO: handle `errno`? use `strtol`?
	int bloblen = BI_ATOI(line + i);

	int out_i = 0;
	for (; out_i < bloblen; out_i++) {
		char c = BI_FGETC(f);
		if (c == BI_EOF)
			return BI_EARLY_EOF;
		(*out)[out_i] = c;
	}
	(*out)[out_i] = '\0';

	BI_FGETC(f);

	return BI_SUCCESS;
}

#endif // BI_IMPLEMENTATION

#endif // BI_H_
