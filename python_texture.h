/*v Forward function declarations
 */
extern void python_texture_init(void);
extern PyObject *python_texture(PyObject* self, PyObject* args, PyObject *kwds);
extern PyObject *python_texture_from_handle(int handle);
extern int python_texture_data(PyObject* self, int id, void *data_ptr);
