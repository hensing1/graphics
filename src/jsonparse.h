#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef enum {
    JSON_TYPE_NULL,
    JSON_TYPE_BOOL,
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT,
    JSON_TYPE_PROPERTY
} JsonType;

typedef struct _JsonElement JsonElement;

typedef struct {
    JsonElement* elements;
    size_t numElements;
    size_t capacity;
} JsonArray;

typedef struct {
    char* key;
    JsonElement* value;
} JsonProperty;

typedef struct {
    JsonProperty* props;
    size_t numProps;
    size_t capacity;
} JsonObject;

typedef struct _JsonElement {
    JsonType type;
    union {
        bool boolValue;
        float numberValue;
        char* stringValue;
        JsonArray* arrayValue;
        JsonObject* objectValue;
    } value;
} JsonElement;

typedef struct {
    void* memory;
    size_t size;
    size_t offset;
} JsonArena;

char* _jsonArenaPutChar(JsonArena* arena, char c) {
    if (arena->offset == arena->size) {
        fprintf(stderr, "Json arena ran out of memory!\n");
        exit(-1);
    }
    ((char*)arena->memory)[arena->offset] = c;
    return (char*)arena->memory + arena->offset++;
}

void* _jsonArenaAlloc(JsonArena* arena, size_t size) {
    if (arena->offset + size > arena->size) {
        fprintf(stderr, "Json arena ran out of memory!\n");
        exit(-1);
    }
    void* ptr = arena->memory + arena->offset;
    arena->offset += size;
    return ptr;
}

JsonArena jsonInit(size_t size) {
    if (size == 0) size = 1024 * 1024;

    JsonArena arena;
    arena.size = size;
    arena.offset = 0;
    arena.memory = mmap(
        NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0
    );
    return arena;
}

void jsonFree(JsonArena* arena) {
    munmap(arena->memory, arena->size);
    arena->size = 0;
}

inline char nextc(FILE* fd) {
    char c;
    do {
        c = getc(fd);
    } while (c == ' ' || c == '\t' || c == '\n');
    return c;
}
typedef enum {
    JSON_TOKEN_OBJECT_BEGIN,
    JSON_TOKEN_OBJECT_END,
    JSON_TOKEN_ARRAY_BEGIN,
    JSON_TOKEN_ARRAY_END,
    JSON_TOKEN_COLON,
    JSON_TOKEN_COMMA,
    JSON_TOKEN_STRINGLITERAL,
    JSON_TOKEN_NULL,
    JSON_TOKEN_BOOL,
    JSON_TOKEN_INVALID
} JsonTokenType;

typedef struct {
    JsonTokenType type;
    char* payload; // LeXEmE
} JsonToken;

typedef struct {
    JsonToken* tokens;
    size_t count;
    size_t capacity;
} JsonTokenArray;

void _jsonTokenArrayPush(JsonTokenArray* tokenArray, JsonToken token) {
    if (tokenArray->count == tokenArray->capacity) {
        tokenArray->tokens = realloc(tokenArray->tokens, sizeof(JsonToken*) * tokenArray->capacity * 2);
        tokenArray->capacity *= 2;
    }
    tokenArray->tokens[tokenArray->count++] = token;
}

JsonToken _jsonParseStringLiteral(FILE* fd, JsonArena* arena) {
    JsonToken token = {.type = JSON_TOKEN_STRINGLITERAL};
    char c = nextc(fd);
    if (c == '"') {
        token.payload = _jsonArenaPutChar(arena, '\0');
        return token;
    }
    else {
        token.payload = _jsonArenaPutChar(arena, c);
    }

    for (c = nextc(fd); c != '"'; c = nextc(fd)) {
        if (c == '\\') {
            if ((c = nextc(fd)) == '\\') {
                _jsonArenaPutChar(arena, '\\');
            }
            else if (c == '"'){
                _jsonArenaPutChar(arena, '"');
            }
            else {
                _jsonArenaPutChar(arena, '\\');
                _jsonArenaPutChar(arena, c);
            }
            continue;
        }
        _jsonArenaPutChar(arena, c);
    }
    _jsonArenaPutChar(arena, '\0');
    return token;
}

inline char lower(char c) {
    if (c < 'a') {c += 32;}
    return c;
}

JsonToken _jsonParseNull(FILE* fd) {
    if (lower(getc(fd)) == 'u' && lower(getc(fd)) == 'l' && lower(getc(fd)) == 'l') {
        return (JsonToken) {.type = JSON_TOKEN_NULL};
    }
    return (JsonToken) {.type = JSON_TOKEN_INVALID};
}

JsonToken _jsonParseNumber(FILE* fd, JsonArena* arena, char firstDigit) {
    float result = 0;
}

JsonTokenArray jsonTokenize(FILE* fd, JsonArena* arena) {
    size_t initCapacity = 128;
    JsonTokenArray tokenArray = {
        .tokens = malloc(sizeof(JsonToken*) * initCapacity),
        .capacity = initCapacity,
        .count = 0
    };
    for (char current_char = nextc(fd); current_char != EOF; current_char = nextc(fd)) {
        JsonToken nextToken;
        switch(current_char) {
            case '{':
                nextToken = (JsonToken) {.type = JSON_TOKEN_OBJECT_BEGIN};
                break;
            case '}':
                nextToken = (JsonToken) {.type = JSON_TOKEN_OBJECT_END};
                break;
            case '[':
                nextToken = (JsonToken) {.type = JSON_TOKEN_ARRAY_BEGIN};
                break;
            case ']':
                nextToken = (JsonToken) {.type = JSON_TOKEN_ARRAY_END};
                break;
            case ':':
                nextToken = (JsonToken) {.type = JSON_TOKEN_COLON};
                break;
            case ',':
                nextToken = (JsonToken) {.type = JSON_TOKEN_COMMA};
                break;
            case '"':
                nextToken = _jsonParseStringLiteral(fd, arena);
                break;
            case 'n':
                nextToken = _jsonParseNull(fd);
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '.':
            case '+':
            case '-':
                nextToken = _jsonParseNumber(fd, arena, current_char);
            default:
                nextToken = (JsonToken) {.type = JSON_TOKEN_INVALID};
        }
        if (nextToken.type == JSON_TOKEN_INVALID) {
            fprintf(stderr, "Could not parse json file.");
            free(tokenArray.tokens);
            return (JsonTokenArray){0};
        }
        _jsonTokenArrayPush(&tokenArray, nextToken);
    }
    return tokenArray;
}

JsonElement jsonParseFile(FILE* fd, JsonArena arena) {
    
}
