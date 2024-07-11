#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>

#define DEFAULT_DELAY_AMOUNT 600
#define MAX_LINES 50
#define MAX_CHARACTERS 1000

typedef enum {
    INPUT_ARG,
    INPUT_FILE,
    INPUT_FILE_WATCH,
} InputKind;

typedef enum {
    PRINT_NEWLINE,
    PRINT_INLINE,
} PrintMethod;

typedef struct {
    InputKind kind;
    PrintMethod pmethod;

    int delay;

    wchar_t** lines;
    int line_count;

    const char* filepath;
} Input;

static void print_usage_and_exit(const char* program_name)
{
    fprintf(stderr, "usage: %s [<options>] [<text to show>]\n", program_name);
    fprintf(stderr, "options:   -h              Display usage\n");
    fprintf(stderr, "           -d <amount>     Delay the output in millisecond (default 600)\n");
    fprintf(stderr, "           -n <true/false> Print newline (default true)\n");
    fprintf(stderr, "           -f <file>       Read text from file\n");
    fprintf(stderr, "           -w <file>       Read text from file and watch for changes\n");

    exit(1);
}

static char* shift_arguments(int* argc, char*** argv)
{
    if (*argc == 0) {
        return NULL;
    }

    char* argument = **argv;
    *argc -= 1;
    *argv += 1;

    return argument;
}

static wchar_t** get_lines_from_text(const char* text, int* lc)
{
    char lines[MAX_LINES][1000];
    int line_count = 0;

    const char* start = text;
    const char* end = start;

    while (*start) {
        while (*end != '\n' && *end != '\0') {
            end++;
        }

        int length = end - start;

        strncpy(lines[line_count], start, length);
        lines[line_count][length] = '\0';
        line_count++;

        if (*end != '\0') {
            end++;
        }

        start = end;
    }

    wchar_t** wlines = malloc(sizeof(wchar_t*) * line_count);

    for (int i = 0; i < line_count; i++) {
        size_t wide_len = mbstowcs(NULL, lines[i], 0) + 1;
        wchar_t* wide_str = malloc(wide_len * sizeof(wchar_t));

        mbstowcs(wide_str, lines[i], wide_len);
        wlines[i] = wide_str;
    }

    *lc = line_count;

    return wlines;
}

static Input parse_command_line(const char* program_name, int argc, char** argv)
{
    Input input;
    input.kind = INPUT_ARG;
    input.pmethod = PRINT_NEWLINE;
    input.delay = DEFAULT_DELAY_AMOUNT;
    input.lines = NULL;
    input.line_count = 0;
    input.filepath = NULL;

    char* text = NULL;
    int text_len = 0;
    int text_cap = 1;

    while (argc > 0) {
        const char* flag = shift_arguments(&argc, &argv);

        if (strcmp(flag, "-h") == 0) {
            print_usage_and_exit(program_name);
        } else if (strcmp(flag, "-d") == 0) {
            // delay in millisecond.
            const char* amount = shift_arguments(&argc, &argv);
            if (!amount) {
                fprintf(stderr, "error: please specity an argument for -d\n");
                print_usage_and_exit(program_name);
            }

            errno = 0;
            char* endptr = NULL;

            input.delay = strtol(amount, &endptr, 10);

            if (amount == endptr) {
                fprintf(stderr, "error: please specity a valid argument for -d\n");
                print_usage_and_exit(program_name);
            }

            if (errno == EINVAL || errno == ERANGE) {
                fprintf(stderr, "error: please specity a valid argument for -d\n");
                print_usage_and_exit(program_name);
            }
        } else if (strcmp(flag, "-n") == 0) {
            const char* value = shift_arguments(&argc, &argv);
            if (!value) {
                fprintf(stderr, "error: please specify an argument for -n\n");
                print_usage_and_exit(program_name);
            }

            if (strcmp(value, "true") != 0 && strcmp(value, "false") != 0) {
                fprintf(stderr, "error: please specify a valid argument for -n\n");
                print_usage_and_exit(program_name);
            }

            if (strcmp(value, "false") == 0) {
                input.pmethod = PRINT_INLINE;
            }
        } else if (strcmp(flag, "-f") == 0) {
            const char* filepath = shift_arguments(&argc, &argv);

            if (filepath == NULL) {
                fprintf(stderr, "error: please specify an argument for -f\n");
                print_usage_and_exit(program_name);
            }

            input.kind = INPUT_FILE;
            input.filepath = filepath;
        } else if (strcmp(flag, "-w") == 0) {
            const char* filepath = shift_arguments(&argc, &argv);

            if (filepath == NULL) {
                fprintf(stderr, "error: please specify an argument for -w\n");
                print_usage_and_exit(program_name);
            }

            input.kind = INPUT_FILE_WATCH;
            input.filepath = filepath;
        } else {
            int len = strlen(flag);

            if (!text) {
                text_cap += len;

                text = malloc(sizeof(char) * text_cap);
                strncpy(text, flag, len);
                text[len] = '\0';

                text_len += len;
            } else {
                text_cap += len + 1;

                text = realloc(text, sizeof(char) * text_cap);
                text[text_len++] = ' ';

                strncpy(text + text_len, flag, len);
                text_len += len;
                text[text_len] = '\0';
            }
        }
    }

    if (input.kind == INPUT_ARG && !text) {
        print_usage_and_exit(program_name);
    }

    // TODO: implement reading from file and file watching.
    assert(input.kind == INPUT_ARG && "currently only accepting input from the command line argument.");

    input.lines = get_lines_from_text(text, &input.line_count);
    free(text);

    return input;
}

static void shift_text(wchar_t* buffer, int buf_len)
{
    wchar_t first = buffer[0];

    for (int i = 0; i < buf_len - 1; i++) {
        buffer[i] = buffer[i + 1];
    }

    buffer[buf_len - 1] = first;
}

static void print_text(Input* input)
{
    struct timespec ts;

    wchar_t buffer[input->line_count][MAX_CHARACTERS];

    for (int i = 0; i < input->line_count; i++) {
        int length = wcslen(input->lines[i]);
        wcsncpy(buffer[i], input->lines[i], length);

        buffer[i][length] = L' ';
        buffer[i][length + 1] = L'\0';
    }

    while (1) {
        for (int i = 0; i < input->line_count; i++) {
            if (input->pmethod == PRINT_NEWLINE) {
                wprintf(L"%ls\n", buffer[i]);
                fflush(stdout);
            } else {
                wprintf(L"%ls\r", buffer[i]);
                fflush(stdout);
            }
        }

        for (int i = 0; i < input->line_count; i++) {
            shift_text(buffer[i], wcslen(buffer[i]));
        }

        ts.tv_sec = input->delay / 1000;
        ts.tv_nsec = (input->delay % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
}

int main(int argc, char** argv)
{
    // set the locale to the user default
    setlocale(LC_ALL, "");

    const char* program_name = shift_arguments(&argc, &argv);

    if (argc < 1) {
        print_usage_and_exit(program_name);
    }

    Input input = parse_command_line(program_name, argc, argv);
    print_text(&input);

    return 0;
}
