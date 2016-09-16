/*t t_key_value_entry_ptr
 */
typedef struct t_key_value_entry *t_key_value_entry_ptr;

/*f t_key_value_entry
 */
typedef struct t_key_value_entry
{
    t_key_value_entry_ptr next;
    char *key;
    char *value;
    void *data;
} t_key_value_entry;

/*t t_key_values
 */
typedef struct
{
    t_key_value_entry_ptr kv_head;
    t_key_value_entry_ptr kv_tail;
    size_t extra_room;
} t_key_values;

#define key_value_entry_key(a) (a->key)
#define key_value_entry_value(a) (a->value)
#define key_value_entry_data(a,t) ((t *)(&(a->data)))

/*a External functions
 */
/*f key_value_parse
 * Parse a string and create a list of key_values from it
 * The string should be [&]<key>=<value>[&<key>=<value>]*
 */
extern const char *
key_value_parse(const char *string, const char *string_end, t_key_values *kv);

/*f key_value_iter
 * iterate over all the key/value pairs in the t_key_values
 */
extern t_key_value_entry_ptr
key_value_iter(t_key_values *kv, t_key_value_entry_ptr kve);

/*f key_value_init
 * Initialize a t_key_values structure
 */
extern void
key_value_init(t_key_values *kv, size_t extra_room);

