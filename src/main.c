#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_DELAY_AMOUNT 600

static void shift_text(char* buffer, int buf_len)
{
    char first = buffer[0];
    for (int i = 0; i < buf_len - 1; i++) {
        buffer[i] = buffer[i + 1];
    }
    buffer[buf_len - 1] = first;
}

// delay in millisecond.
static void print_text(const char* text, int delay)
{
    struct timespec ts;

    int length = strlen(text);
    char buffer[length + 2];
    memcpy(buffer, text, length);
    buffer[length] = ' ';
    buffer[length + 1] = '\0';

    while (1) {
        printf("%s\n", buffer);
        fflush(stdout);

        shift_text(buffer, length + 1);

        ts.tv_sec = delay / 1000;
        ts.tv_nsec = (delay % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }
}

static void print_usage_and_exit(const char* program_name)
{
    fprintf(stderr, "usage: %s [<flags>] \"<text to show>\"\n", program_name);
    fprintf(stderr, "flags:     -h                  Display usage\n");
    fprintf(stderr, "           -d <amount>         Delay the output in millisecond (default is 600ms).\n");
    exit(1);
}

typedef struct {
    int delay;
    const char* text;
} Input;

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

Input parse_command_line(const char* program_name, int argc, char** argv)
{
    Input input;
    input.delay = DEFAULT_DELAY_AMOUNT;
    input.text = NULL;

    while (input.text == NULL && argc > 0) {
        const char* flag = shift_arguments(&argc, &argv);

        if (strcmp(flag, "-d") == 0) {
            const char* amount = shift_arguments(&argc, &argv);
            if (amount == NULL) {
                print_usage_and_exit(program_name);
            }

            input.delay = atoi(amount);
        } else if (strcmp(flag, "-h") == 0) {
            print_usage_and_exit(program_name);
        } else {

            input.text = flag;
        }
    }

    if (input.text == NULL) {
        print_usage_and_exit(program_name);
    }

    return input;
}

int main(int argc, char** argv)
{
    const char* program_name = shift_arguments(&argc, &argv);

    if (argc < 1) {
        print_usage_and_exit(program_name);
    }

    Input input = parse_command_line(program_name, argc, argv);
    print_text(input.text, input.delay);

    return 0;
}
