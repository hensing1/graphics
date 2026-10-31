// welcome to the ghetto

#define DEFINE_ARRAY(type, name)                                               \
   typedef struct {                                                            \
      type *items;                                                             \
      int count;                                                               \
      int capacity;                                                            \
   } name##_array_t;                                                           \
                                                                               \
   name##_array_t name##_array_init();                                         \
   void name##_array_push(name##_array_t *arr, type item);                     \
   type name##_array_pop(name##_array_t *arr);                                 \
   void name##_array_insert(name##_array_t *arr, type item, int index);

#ifndef DEFAULT_ARRAY_CAPACITY
#define DEFAULT_ARRAY_CAPACITY 256
#endif

#define IMPLEMENT_ARRAY(type, name)                                            \
   name##_array_t name##_array_init() {                                        \
      name##_array_t arr;                                                      \
      arr.items    = malloc(sizeof(type) * DEFAULT_ARRAY_CAPACITY);            \
      arr.count    = 0;                                                        \
      arr.capacity = DEFAULT_ARRAY_CAPACITY;                                   \
      return arr;                                                              \
   }                                                                           \
                                                                               \
   void name##_array_push(name##_array_t *arr, type item) {                    \
      if (arr->count == arr->capacity) {                                       \
         arr->capacity *= 2;                                                   \
         arr->items = realloc(arr->items, sizeof(type) * arr->capacity);       \
      }                                                                        \
      arr->items[arr->count++] = item;                                         \
   }                                                                           \
                                                                               \
   type name##_array_pop(name##_array_t *arr) {                                \
      return arr->items[--arr->count];                                         \
   }                                                                           \
                                                                               \
   void name##_array_insert(name##_array_t *arr, type item, int index) {       \
      if (arr->count == arr->capacity) {                                       \
         arr->capacity *= 2;                                                   \
         arr->items = realloc(arr->items, sizeof(type) * arr->capacity);       \
      }                                                                        \
                                                                               \
      for (int i = arr->count; i > index; i--) {                               \
         arr->items[i] = arr->items[i - 1];                                    \
      }                                                                        \
      arr->items[index] = item;                                                \
      arr->count++;                                                            \
   }
