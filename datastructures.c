#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  TYPE_STRING,
  TYPE_FLOAT,
  TYPE_NOT_FOUND
} ValueType;

typedef struct {
  union {
    char* s;
    float f;
  } value;
  ValueType type;
} HashmapValue;

typedef struct _HashmapItem {
  char* key;
  HashmapValue value;
  struct _HashmapItem* next;
} HashmapItem;

typedef struct {
  HashmapItem* items;
  size_t size;
  size_t capacity;
} ItemList;

typedef struct {
  size_t* indices;
  int* numItemsForHash;
  size_t size;
} IndexList;

typedef struct {
  ItemList itemlist;
  IndexList indexlist;
} Hashmap;

size_t hash(char* key, size_t seed) {
  size_t hash = seed;
  for (; *key != '\0'; key++) {
    hash += *key;
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

void itemlist_push(ItemList* list, HashmapItem item) {
  if (list->size == list->capacity) {
    list->items = realloc(list->items, sizeof(HashmapItem) * list->size * 2);
    list->capacity *= 2;
  }
  list->items[list->size++] = item;
}

Hashmap hashmap_init() {
  size_t indexListSize = 1024;
  return (Hashmap) {
    .indexlist = {
      .size = indexListSize,
      .indices = malloc(sizeof(size_t) * indexListSize),
      .numItemsForHash = calloc(1024, sizeof(int))
    },
    .itemlist = {
      .capacity = 64,
      .items = malloc(sizeof(HashmapItem) * 64),
      .size = 0
    }
  };
}

void hashmap_push(Hashmap* map, char* key, HashmapValue value) {
  HashmapItem newItem = {
    .key = key,
    .value = value
  };
  itemlist_push(&map->itemlist, newItem);
  size_t list_idx = map->itemlist.size - 1;
  size_t hash_idx = hash(key, 0) % map->indexlist.size;

  if (map->indexlist.numItemsForHash[hash_idx] > 0) {
    HashmapItem* curr = &(map->itemlist.items[map->indexlist.indices[hash_idx]]);
    for (int i = 0; i < map->indexlist.numItemsForHash[hash_idx] - 1; i++) {
      curr = curr->next;
    }
    curr->next = &(map->itemlist.items[list_idx]);
  }
  else {
    map->indexlist.indices[hash_idx] = list_idx;
  }
  map->indexlist.numItemsForHash[hash_idx]++;
}

bool streq(char* a, char* b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {return false;}
    a++, b++;
  }
  return *a == '\0' && *b == '\0';
}

HashmapValue hashmap_get(Hashmap* map, char* key) {
  size_t hash_idx = hash(key, 0) % map->indexlist.size;
  int k = map->indexlist.numItemsForHash[hash_idx];
  HashmapItem* curr;
  if (k > 0) {
    size_t list_idx = map->indexlist.indices[hash_idx];
    curr = &map->itemlist.items[list_idx];
    do {
      if (streq(key, curr->key)) {
        return curr->value;
      }
      curr = curr->next;
      k--;
    } while (k > 0);
  }
  return (HashmapValue) {.type = TYPE_NOT_FOUND};
}

int main() {
  Hashmap h = hashmap_init();
  hashmap_push(&h, "hello",   (HashmapValue){.type = TYPE_FLOAT, .value.f = 3.14});
  hashmap_push(&h, "this",    (HashmapValue){.type = TYPE_STRING, .value.s = "yeee"});
  hashmap_push(&h, "is",      (HashmapValue){.type = TYPE_FLOAT, .value.f = 4.14});
  hashmap_push(&h, "a",       (HashmapValue){.type = TYPE_FLOAT, .value.f = 5.14});
  hashmap_push(&h, "hashmap", (HashmapValue){.type = TYPE_FLOAT, .value.f = 6.14});
  hashmap_push(&h, "hashmap", (HashmapValue){.type = TYPE_STRING, .value.s = "7.14"});
  HashmapValue v = hashmap_get(&h, "a");
  printf("%f\n", v.value.f);
  v = hashmap_get(&h, "this");
  printf("%s\n", v.value.s);
  v = hashmap_get(&h, "hashmap");
  printf("%d\n", v.type);
  v = hashmap_get(&h, "thisi");
  printf("%d\n", v.type);
  return 0;
}
