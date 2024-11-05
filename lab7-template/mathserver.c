#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

FILE* output_file;

struct context {
    int value;
} context;

struct context contexts[16];

// Function to tokenize input
char** tokenize_input(char* input)
{
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL)
    {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}

int fib(int n) {
    if (n <= 0) {
        return 0;
    }
    else if (n == 1) {
        return 1;
    }
    else {
        return fib(n - 1) + fib(n - 2);
    }
}

int** pri(int n) {
    int** results = malloc(sizeof(int*));
    int index = 0;
    for (int i = 2; i < n; i++) {
        int is_prime = 1;
        for (int j = 2; j < i; j++) {
            if (i % j == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime) {
            results = realloc(results, (index + 1) * sizeof(int*));
            results[index] = malloc(sizeof(int));
            results[index][0] = i;
            index++;
        }
    }
    results = realloc(results, (index + 1) * sizeof(int*));
    results[index] = NULL;
    return results;
}

double pia(int n) {
    double pi = 1;
    for (int i = 1; i < n; i++) {
        int t = 1;
        if (i % 2 != 0) {
            t = -1;
        }
        pi += t / (2.0 * i + 1);
    }

    return pi * 4;
}


int main(int argc, char* argv[]) {
    const char usage[] = "Usage: mathserver.out <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 3) {
        printf("%s", usage);
        return 1;
    }
    
    input_trace = argv[1];
    output_trace = argv[2];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");

    output_file = fopen(output_trace, "w");

    for (int i = 0; i < 16; i++) {
        contexts[i].value = 0;
    }

    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez )
            break;
        else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }

        char** tokens = tokenize_input(buffer);

        if (tokens[0] == NULL) {
            for (int i = 0; tokens[i] != NULL; i++)
            {
                free(tokens[i]);
            }
            free(tokens);
            continue;
        }
        if (tokens[1] == NULL) {
            continue;
        }
        struct context* ctx = &contexts[atoi(tokens[1])];
        if (strcmp(tokens[0], "set") == 0) {
            ctx->value = atoi(tokens[2]);
            fprintf(output_file, "ctx %02d: set to value %d\n", atoi(tokens[1]), ctx->value);
        }
        else if (strcmp(tokens[0], "add") == 0) {
            ctx->value = ctx->value + atoi(tokens[2]);
            fprintf(output_file, "ctx %02d: add %d (result: %d)\n", atoi(tokens[1]), atoi(tokens[2]), ctx->value);
        }
        else if (strcmp(tokens[0], "sub") == 0) {
            ctx->value = ctx->value - atoi(tokens[2]);
            fprintf(output_file, "ctx %02d: sub %d (result: %d)\n", atoi(tokens[1]), atoi(tokens[2]), ctx->value);
        }
        else if (strcmp(tokens[0], "mul") == 0) {
            ctx->value = ctx->value * atoi(tokens[2]);
            fprintf(output_file, "ctx %02d: mul %d (result: %d)\n", atoi(tokens[1]), atoi(tokens[2]), ctx->value);
        }
        else if (strcmp(tokens[0], "div") == 0) {
            ctx->value = ctx->value / atoi(tokens[2]);
            fprintf(output_file, "ctx %02d: div %d (result: %d)\n", atoi(tokens[1]), atoi(tokens[2]), ctx->value);
        }
        else if (strcmp(tokens[0], "fib") == 0) {
            ctx->value = fib(ctx->value);
            fprintf(output_file, "ctx %02d: fib (result: %d)\n", atoi(tokens[1]), ctx->value);
        }
        else if (strcmp(tokens[0], "pri") == 0) {
            int** results = pri(ctx->value);
            fprintf(output_file, "ctx %02d: primes (result: %d", atoi(tokens[1]), results[0][0]);
            free(results[0]);
            for (int i = 1; results[i] != NULL; i++) {
                fprintf(output_file, ", %d", results[i][0]);
                free(results[i]);
            }
            free(results);
            fprintf(output_file, ")\n");
        }
        else if (strcmp(tokens[0], "pia") == 0) {
            double pi = pia(ctx->value);
            fprintf(output_file, "ctx %02d: pia (result %.15f)\n", atoi(tokens[1]), pi);
        }


        // CONTINUE...
    }
    fclose(input_file);
    fclose(output_file);

    return 0;
}