#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

FILE* output_file;

struct operation {
    char* name;
    int value;
    struct operation* next;
};

struct context {
    int id;
    int value;
    struct operation* operations;
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

int isPrime(int n)
{
    if (n <= 1)
        return 0;
    if (n <= 3)
        return 1;

    if (n % 2 == 0 || n % 3 == 0)
        return 0;

    for (int i = 5; i * i <= n; i = i + 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;

    return 1;
}


int** pri(int n) {
    int** results = malloc(sizeof(int*));
    int index = 0;
    for (int i = 0; i < n; i++) {
        if (isPrime(i)) {
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

char* executeOperation(struct context* ctx, struct operation* op) {
    char* result = malloc(1024); // Allocate space for result string

    if (strcmp(op->name, "set") == 0) {
        ctx->value = op->value;
        sprintf(result, "ctx %02d: set to value %d\n", ctx->id, ctx->value);
    }
    else if (strcmp(op->name, "add") == 0) {
        ctx->value = ctx->value + op->value;
        sprintf(result, "ctx %02d: add %d (result: %d)\n", ctx->id, op->value, ctx->value);
    }
    else if (strcmp(op->name, "sub") == 0) {
        ctx->value = ctx->value - op->value;
        sprintf(result, "ctx %02d: sub %d (result: %d)\n", ctx->id, op->value, ctx->value);
    }
    else if (strcmp(op->name, "mul") == 0) {
        ctx->value = ctx->value * op->value;
        sprintf(result, "ctx %02d: mul %d (result: %d)\n", ctx->id, op->value, ctx->value);
    }
    else if (strcmp(op->name, "div") == 0) {
        ctx->value = ctx->value / op->value;
        sprintf(result, "ctx %02d: div %d (result: %d)\n", ctx->id, op->value, ctx->value);
    }
    else if (strcmp(op->name, "fib") == 0) {
        int value = fib(ctx->value);
        sprintf(result, "ctx %02d: fib (result: %d)\n", ctx->id, value);
    }
    else if (strcmp(op->name, "pri") == 0) {
        int** results = pri(ctx->value);
        size_t bufsize = 1024;
        char* temp = malloc(bufsize);
        sprintf(temp, "ctx %02d: primes (result: %d", ctx->id, results[0][0]);
        size_t current_len = strlen(temp);
        result = malloc(bufsize);
        strcpy(result, temp);
        free(results[0]);
        
        for (int i = 1; results[i] != NULL; i++) {
            sprintf(temp, ", %d", results[i][0]);
            size_t needed_len = current_len + strlen(temp) + 3; // +3 for ")\n\0"
            if (needed_len > bufsize) {
            bufsize *= 2;
            result = realloc(result, bufsize);
            temp = realloc(temp, bufsize);
            }
            strcat(result, temp);
            current_len += strlen(temp);
            free(results[i]);
        }
        strcat(result, ")\n");
        free(temp);
        free(results);
    }
    else if (strcmp(op->name, "pia") == 0) {
        double pi = pia(ctx->value);
        sprintf(result, "ctx %02d: pia (result %.15f)\n", ctx->id, pi);
    }
    else {
        sprintf(result, "ctx %02d: unknown operation %s\n", ctx->id, op->name);
    }

    return result;
}

void logBuf(char** buf, size_t size) {
    for (int i = 0; i < size; i++) {
        fprintf(output_file, "%s", buf[i]);
        free(buf[i]);
    }
}

void* threadExecution(void* arg) {
    char* printbuf[10] = {0};
    struct context* ctx = (struct context*)arg;
    struct operation* op = ctx->operations;
    int i = 0;
    while (op != NULL) {
        if (i == 10) {
            pthread_mutex_t logBuf_mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_lock(&logBuf_mutex);
            logBuf(printbuf, 10);
            pthread_mutex_unlock(&logBuf_mutex);
            i = 0;
        }
        printbuf[i++] = executeOperation(ctx, op);
        struct operation* next = op->next;
        free(op->name);
        free(op);
        op = next;
    }
    pthread_mutex_t logBuf_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&logBuf_mutex);
    logBuf(printbuf, i);
    pthread_mutex_unlock(&logBuf_mutex);
    return NULL;
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
        contexts[i].id = i;
        contexts[i].operations = NULL;
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

        struct operation* op = malloc(sizeof(struct operation));
        op->name = strdup(tokens[0]);
        if (tokens[2] != NULL) {
            op->value = atoi(tokens[2]);
        }
        op->next = NULL;

        if (ctx->operations == NULL) {
            ctx->operations = op;
        } else {
            struct operation* current = ctx->operations;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = op;
        }

        for (int i = 0; tokens[i] != NULL; i++)
        {
            free(tokens[i]);
        }
        // CONTINUE...
    }
    fclose(input_file);

    // Create 16 threads
    pthread_t threads[16];
    for (int i = 0; i < 16; i++) {
        pthread_create(&threads[i], NULL, threadExecution, &contexts[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < 16; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(output_file);

    return 0;
}