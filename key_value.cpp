/*a Documentation
 */
/*a Includes
 */
#include <stdlib.h>
#include <string.h>
#include "key_value.h"

/*a Types
 */
/*a Static functions
 */
/*f key_value_remove
 */
static void 
key_value_remove(t_key_values *kv, t_key_value_entry *kve)
{
    if (kve->prev) {
        kve->prev->next = kve->next;
    }
    if (kve->next) {
        kve->next->prev = kve->prev;
    }
    if (kv->kv_head == kve) {
        kv->kv_head = kve->next;
    }
    if (kv->kv_tail == kve) {
        kv->kv_tail = kve->prev;
    }
    free(kve);
}

/*f key_value_add
 */
static void 
key_value_add(t_key_values *kv, const char *key, int key_len, const char *value, int value_len)
{
    void *ptr;
    t_key_value_entry *kve;
    char *text_ptr;
    int kve_len;
    kve_len = sizeof(t_key_value_entry)+kv->extra_room;
    ptr = malloc(kve_len + key_len+1 + value_len+1);
    kve = (t_key_value_entry *)ptr;
    text_ptr = ((char *)kve+kve_len);
    kve->key = text_ptr;
    strncpy(kve->key, key, key_len);
    kve->key[key_len]=0;
    text_ptr = text_ptr+key_len+1;
    kve->value = text_ptr;
    strncpy(kve->value, value, value_len);
    kve->value[value_len]=0;

    if (kv->kv_tail) {
        kve->prev = kv->kv_tail;
        kv->kv_tail->next = kve;
    } else {
        kve->prev = NULL;
        kv->kv_head = kve;
    }
    kv->kv_tail = kve;
    kve->next = NULL;
}

/*a External functions
 */

/*f key_value_set
 * Set (or replace)
 */
void key_value_set(t_key_values *kv, const char *key, const char *value)
{
    key_value_delete(kv, key);
    key_value_add(kv, key, strlen(key), value, strlen(value));
}

/*f key_value_delete
 * Delete key - return 0 for okay, 1 for failure
 */
int
key_value_delete(t_key_values *kv, const char *key)
{
    t_key_value_entry_ptr kve;
    kve = key_value_find(kv, key);
    if (!kve) return 1;
    key_value_remove(kv, kve);
    return 0;
}

/*f key_value_parse
 */
const char *
key_value_parse(const char *string, const char *string_end, t_key_values *kv)
{
    const char *end;
    const char *equals;

    while (string[0]=='&') string++;
    end = strchr(string,'&');
    if (!end) {
        end = string_end;
    }
    if (end>string_end) end=string_end;
    equals = strchr(string,'=');
    if (equals>end) equals=NULL;
    if (!equals) {
        key_value_add(kv, string, end-string, NULL, 0);
    } else {
        key_value_add(kv, string, equals-string, equals+1, end-equals-1);
    }
    return end;
}

/*f key_value_iter
 */
t_key_value_entry_ptr 
key_value_iter(t_key_values *kv, t_key_value_entry_ptr kve)
{
    if (kve)
        return kve->next;
    return kv->kv_head;
}

/*f key_value_find
 */
t_key_value_entry_ptr 
key_value_find(t_key_values *kv, const char *key)
{
    t_key_value_entry_ptr kve;
    for (kve=kv->kv_head; kve; kve=kve->next) {
        if (!strcmp(kve->key,key)) {
            return kve;
        }
    }
    return NULL;
}

/*f key_value_init
 */
void
key_value_init(t_key_values *kv, size_t extra_room)
{
    kv->kv_head = NULL;
    kv->kv_tail = NULL;
    kv->extra_room = extra_room;
}

