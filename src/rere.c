#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BI_IMPLEMENTATION
#include "bi.h"

void print_bi_error(BI_PARSE_ERROR err)
{
	switch (err) {
	case BI_SUCCESS:
		break;
	case BI_INVALID_PREFIX:
		printf("ERROR: Invalid field prefix\n");
		break;
	case BI_INCORRECT_NAME:
		printf("ERROR: Incorrect field name\n");
		break;
	case BI_INVALID_SUFFIX:
		printf("ERROR: Invalid field suffix\n");
		break;
	case BI_EARLY_EOF:
		printf("ERROR: Blob content ended early\n");
		break;
	}
}

bool record(char *input_path)
{
	printf("--Recording commands from %s--\n", input_path);
	int input_path_len = strlen(input_path);

	FILE *inputf = fopen(input_path, "rb");
	if (!inputf) {
		printf("ERROR: failed to open file %s: %s\n", input_path,
		       strerror(errno));
		return false;
	}

	int count = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, inputf)) != -1) {
		count++;
		//free(line);
	}
	fseek(inputf, 0, SEEK_SET);

	char *output_path = malloc(sizeof(char) * (input_path_len + 3));
	output_path = memcpy(output_path, input_path, input_path_len);
	char extension[] = ".bi";
	int extension_len = strlen(extension);
	for (int i = 0; i < extension_len; i++) {
		output_path[input_path_len + i] = extension[i];
	}
	FILE *outputf = fopen(output_path, "wb");
	free(output_path);
	if (!outputf) {
		printf("ERROR: failed to open file %s: %s\n", input_path,
		       strerror(errno));
		fclose(inputf);
		return false;
	}

	bi_write_int_field(outputf, "count", count);

	char *shell = NULL;
	size_t nlen = 0;
	while ((read = getline(&shell, &nlen, inputf)) != -1) {
		printf("Recording `");
		char tmp;
		for (int i = 0; (tmp = shell[i]) != '\n' && tmp != '\0'; i++) {
			putc(tmp, stdout);
		}
		printf("`...\n");

		FILE *f = popen(shell, "r");
		if (f == NULL) {
			printf("ERROR: failed to run command %s: %s\n", shell,
			       strerror(errno));

			fclose(inputf);
			fclose(outputf);
			return false;
		}

		char *cmd_stdout = malloc(sizeof(char) * 2048);
		char c;
		int i = 0;
		while ((c = fgetc(f)) != EOF)
			cmd_stdout[i++] = c;
		cmd_stdout[i++] = '\0';
		int exit_code = pclose(f);

		bi_write_blob_field(outputf, "shell", shell);
		bi_write_int_field(outputf, "exit_code", exit_code);
		bi_write_blob_field(outputf, "stdout", cmd_stdout);

		free(cmd_stdout);
	}

	return true;
}

bool replay(char *input_path)
{
	printf("--Replaying commands from %s--\n", input_path);

	int input_path_len = strlen(input_path);

	char *output_path = malloc(sizeof(char) * (input_path_len + 3));
	output_path = memcpy(output_path, input_path, input_path_len);
	char extension[] = ".bi";
	int extension_len = strlen(extension);
	for (int i = 0; i < extension_len; i++) {
		output_path[input_path_len + i] = extension[i];
	}
	FILE *output = fopen(output_path, "rb");
	free(output_path);
	if (!output) {
		printf("ERROR: failed to open file %s: %s\n", input_path,
		       strerror(errno));
		return false;
	}

	int count = 0;
	ssize_t nread = 0;
	BI_PARSE_ERROR err = bi_read_int_field(output, "count", &count, &nread);
	if (err != BI_SUCCESS) {
		print_bi_error(err);
		return false;
	}

	bool failed = false;
	for (int i = 0; i < count; i++) {
		char *shell = malloc(sizeof(char) * 2048);
		int exit_code = 0;
		// TODO: handle overflow
		char *cmd_stdout = malloc(sizeof(char) * 2048);

		err = bi_read_blob_field(output, "shell", &shell, &nread);
		if (err != BI_SUCCESS) {
			print_bi_error(err);
			return false;
		}
		err = bi_read_int_field(output, "exit_code", &exit_code,
					&nread);
		if (err != BI_SUCCESS) {
			print_bi_error(err);
			return false;
		}
		err = bi_read_blob_field(output, "stdout", &cmd_stdout, &nread);
		if (err != BI_SUCCESS) {
			print_bi_error(err);
			return false;
		}

		printf("Replaying `");
		char tmp;
		for (int i = 0; (tmp = shell[i]) != '\n' && tmp != '\0'; i++) {
			putc(tmp, stdout);
		}
		printf("`...\n");

		FILE *f = popen(shell, "r");
		if (f == NULL) {
			printf("ERROR: failed to run command %s: %s\n", shell,
			       strerror(errno));

			fclose(output);
			return false;
		}

		char *new_cmd_stdout = malloc(sizeof(char) * 2048);
		char c;
		int i = 0;
		while ((c = fgetc(f)) != EOF)
			new_cmd_stdout[i++] = c;
		new_cmd_stdout[i++] = '\0';
		int new_exit_code = pclose(f);

		if (strcmp(cmd_stdout, new_cmd_stdout) != 0) {
			printf("UNEXPECTED STDOUT:\n");
			printf("    EXPECTED:\n```\n%s\n```\n", cmd_stdout);
			printf("    ACTUAL:\n```\n%s\n```\n", new_cmd_stdout);

			failed = true;
		}

		if (exit_code != new_exit_code) {
			printf("UNEXPECTED EXIT CODE:\n");
			printf("    EXPECTED: `%d`\n", exit_code);
			printf("    ACTUAL:   `%d`\n", new_exit_code);

			failed = true;
		}

		free(cmd_stdout);
		free(new_cmd_stdout);
	}

	fclose(output);
	return !failed;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s [record|replay] <files..>\n", argv[0]);
		printf("ERROR: missing required argument [record|replay]\n");
		return 1;
	}

	char *subcommand = argv[1];

	if (strcmp(subcommand, "record") != 0 && strcmp(subcommand, "replay")) {
		printf("Usage: %s [record|replay] <files..>\n", argv[0]);
		printf("ERROR: invalid subcommand '%s'\n", subcommand);
		return 1;
	}

	if (argc < 3) {
		printf("Usage: %s [record|replay] <files..>\n", argv[0]);
		printf("ERROR: missing required argument <file>\n");
		return 1;
	}

	for (int i = 2; i < argc; i++) {
		if (subcommand[2] == 'c') { // Record
			if (!record(argv[i]))
				return 1;
		} else { // Replay
			if (!replay(argv[i]))
				return 1;
		}
	}

	return 0;
}
