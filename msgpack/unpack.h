/*
 * MessagePack for Python unpacking routine
 *
 * Copyright (C) 2009 Naoki INADA
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#define MSGPACK_EMBED_STACK_SIZE  (1024)
#include "unpack_define.h"

typedef struct unpack_user {
    bool use_list;
    bool raw;
    bool has_pairs_hook;
    PyObject *object_hook;
    PyObject *list_hook;
    PyObject *ext_hook;
    PyObject *sedes;
    const char *encoding;
    const char *unicode_errors;
    Py_ssize_t max_str_len, max_bin_len, max_array_len, max_map_len, max_ext_len;
} unpack_user;

typedef struct unpack_stack {
    PyObject* obj;
    Py_ssize_t size;
    Py_ssize_t count;
    uint64_t length;
    intptr_t start_pointer;
    unsigned int ct;
    PyObject* map_key;
} unpack_stack;

struct unpack_context {
    unpack_user user;
    unsigned int cs;
    unsigned int trail;
    unsigned int top;
    /*
    unpack_stack* stack;
    unsigned int stack_size;
    unpack_stack embed_stack[MSGPACK_EMBED_STACK_SIZE];
    */
    //sede sedes;
    unpack_stack stack[MSGPACK_EMBED_STACK_SIZE];
};

typedef PyObject* msgpack_unpack_object;
struct unpack_context;
typedef struct unpack_context unpack_context;
typedef int (*execute_fn)(unpack_context *ctx, const char* data, Py_ssize_t len, Py_ssize_t* off);

static inline msgpack_unpack_object unpack_callback_root(unpack_user* u)
{
    return NULL;
}

static inline int unpack_callback_uint16(unpack_user* u, uint16_t d, msgpack_unpack_object* o)
{
    PyObject *p = PyInt_FromLong((long)d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}
static inline int unpack_callback_uint8(unpack_user* u, uint8_t d, msgpack_unpack_object* o)
{
    return unpack_callback_uint16(u, d, o);
}


static inline int unpack_callback_uint32(unpack_user* u, uint32_t d, msgpack_unpack_object* o)
{
    PyObject *p = PyInt_FromSize_t((size_t)d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int unpack_callback_uint64(unpack_user* u, uint64_t d, msgpack_unpack_object* o)
{
    PyObject *p;
    if (d > LONG_MAX) {
        p = PyLong_FromUnsignedLongLong((unsigned PY_LONG_LONG)d);
    } else {
        p = PyInt_FromLong((long)d);
    }
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int unpack_callback_int32(unpack_user* u, int32_t d, msgpack_unpack_object* o)
{
    PyObject *p = PyInt_FromLong(d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int unpack_callback_int16(unpack_user* u, int16_t d, msgpack_unpack_object* o)
{
    return unpack_callback_int32(u, d, o);
}

static inline int unpack_callback_int8(unpack_user* u, int8_t d, msgpack_unpack_object* o)
{
    return unpack_callback_int32(u, d, o);
}

static inline int unpack_callback_int64(unpack_user* u, int64_t d, msgpack_unpack_object* o)
{
    PyObject *p;
    if (d > LONG_MAX || d < LONG_MIN) {
        p = PyLong_FromLongLong((PY_LONG_LONG)d);
    } else {
        p = PyInt_FromLong((long)d);
    }
    *o = p;
    return 0;
}

static inline int unpack_callback_double(unpack_user* u, double d, msgpack_unpack_object* o)
{
    PyObject *p = PyFloat_FromDouble(d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int unpack_callback_float(unpack_user* u, float d, msgpack_unpack_object* o)
{
    return unpack_callback_double(u, d, o);
}

static inline int unpack_callback_nil(unpack_user* u, msgpack_unpack_object* o)
{ Py_INCREF(Py_None); *o = Py_None; return 0; }

static inline int unpack_callback_true(unpack_user* u, msgpack_unpack_object* o)
{ Py_INCREF(Py_True); *o = Py_True; return 0; }

static inline int unpack_callback_false(unpack_user* u, msgpack_unpack_object* o)
{ Py_INCREF(Py_False); *o = Py_False; return 0; }


/*
*
*NEWWWWWWW
*This function will get the type of the item currently being decoded
* user will have the actual python list, unpack_user->sedes. unpack_context->stack will have the stack, unpack_context->top is the length of the stack. current item is top-1
* types: 0 = binary (default), 1 = uint64_t, 2 = unicode string
*/
static inline int get_current_item_type(unpack_user *u, unpack_context *ctx)
{
    //else if unpack_user->sedes is an intiger or long, return it. this allows you to easily set the sede for everything the same, or for all children the same.
    //else if unpack_user->sedes is a list, go to loop. then at each iteration do these two checks.

    if (u->sedes) {

        int ret;

        if(PyLong_Check(u->sedes))
        {
            ret = (int)PyLong_AsLong(u->sedes);
            return ret;
        }
        else
        {

            unsigned int _top = ctx->top;
            unsigned int i;
            Py_ssize_t stack_index;
            PyObject *current_sede = u->sedes;
            Py_ssize_t current_sede_length;

            for( i = 0; i < _top; i = i + 1 )
            {
                if(i != 0)
                {
                    if(PyLong_Check(u->sedes))
                    {
                        ret = (int)PyLong_AsLong(current_sede);
                        return ret;
                    }
                }
                if (!PyList_Check(current_sede))
                {
                    PyErr_Format(PyExc_ValueError, "Sedes can only be lists or integers");
                    return -1;
                }

                current_sede_length = PyList_GET_SIZE(current_sede);
                if (current_sede_length == 1) {
                    //this means it can be repeating. for example: [a] is a list of repeating a's, but they all have the type of the 0th element in sedes
                    // example 2: [[a,b]], even if the first list has an index of n, the sede lookup for a should be 0,0. Not n, 0.
                    current_sede = PyList_GetItem(current_sede, 0);
                } else {
                    stack_index =  ctx->stack[i].count;

                    if(current_sede_length < (stack_index + 1))
                    {
                        PyErr_Format(PyExc_ValueError, "Sede list dimensions don't match data. Current_sede_length = %u, stack_index = %u, stack height = %u", current_sede_length, stack_index, i);
                        return -1;
                    }

                    //current_sede = PyList_GET_ITEM(current_sede, stack_index);
                    current_sede = PyList_GetItem(current_sede, stack_index);
                }
            }
            //now we should be at the relevant sede.
            if(PyLong_Check(current_sede))
            {
                ret = (int)PyLong_AsLong(current_sede);
                return ret;
            }
            //PyErr_Format(PyExc_ValueError, " debug %u %zd", _top, stack_index);
            PyErr_Format(PyExc_ValueError, "There is a mismatch between the sedes and the data. Make sure the sedes are correct for this encoded data");
            return -1;

        }
    } else {
        return 0;
    }


}


//starts the array
static inline int unpack_callback_array(unpack_user* u, unsigned int n, msgpack_unpack_object* o)
{
    Py_ssize_t tuple_size;
    if (n > u->max_array_len) {
        PyErr_Format(PyExc_ValueError, "%u exceeds max_array_len(%zd)", n, u->max_array_len);
        return -1;
    }
    PyObject *p = u->use_list ? PyList_New(n) : PyTuple_New(n);

    if (!p){
        return -1;
    }
    *o = p;
    return 0;
}




//appends o to list
static inline int unpack_callback_append_array_item(unpack_user* u, msgpack_unpack_object* c, msgpack_unpack_object o)
{
    Py_ssize_t tuple_size;
    if (u->use_list) {
        PyList_Append(*c, o);
    } else {
        tuple_size = PyTuple_Size(*c);

        if(_PyTuple_Resize(c, tuple_size+1) == 0){
            if(PyTuple_SetItem(*c, tuple_size, o) != 0){
                PyErr_Format(PyExc_ValueError, "failed to set tuple item");
                return -1;
            }
        } else {
            return -1;
        }
    }
    return 0;
}

//sets a position in the list to o
static inline int unpack_callback_array_item(unpack_user* u, unsigned int current, msgpack_unpack_object* c, msgpack_unpack_object o)
{
    if (u->use_list) {
        PyList_SET_ITEM(*c, current, o);

    } else {
        PyTuple_SET_ITEM(*c, current, o);
    }
    return 0;
}

//ends the array
static inline int unpack_callback_array_end(unpack_user* u, msgpack_unpack_object* c)
{
    if (u->list_hook) {
        PyObject *new_c = PyObject_CallFunctionObjArgs(u->list_hook, *c, NULL);
        if (!new_c)
            return -1;
        Py_DECREF(*c);
        *c = new_c;
    }
    return 0;
}

static inline int unpack_callback_map(unpack_user* u, unsigned int n, msgpack_unpack_object* o)
{
    if (n > u->max_map_len) {
        PyErr_Format(PyExc_ValueError, "%u exceeds max_map_len(%zd)", n, u->max_map_len);
        return -1;
    }
    PyObject *p;
    if (u->has_pairs_hook) {
        p = PyList_New(n); // Or use tuple?
    }
    else {
        p = PyDict_New();
    }
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int unpack_callback_map_item(unpack_user* u, unsigned int current, msgpack_unpack_object* c, msgpack_unpack_object k, msgpack_unpack_object v)
{
    if (u->has_pairs_hook) {
        msgpack_unpack_object item = PyTuple_Pack(2, k, v);
        if (!item)
            return -1;
        Py_DECREF(k);
        Py_DECREF(v);
        PyList_SET_ITEM(*c, current, item);
        return 0;
    }
    else if (PyDict_SetItem(*c, k, v) == 0) {
        Py_DECREF(k);
        Py_DECREF(v);
        return 0;
    }
    return -1;
}

static inline int unpack_callback_map_end(unpack_user* u, msgpack_unpack_object* c)
{
    if (u->object_hook) {
        PyObject *new_c = PyObject_CallFunctionObjArgs(u->object_hook, *c, NULL);
        if (!new_c)
            return -1;

        Py_DECREF(*c);
        *c = new_c;
    }
    return 0;
}

static inline int unpack_callback_raw(unpack_user* u, const char* b, const char* p, unsigned int l, msgpack_unpack_object* o)
{
    if (l > u->max_str_len) {
        PyErr_Format(PyExc_ValueError, "%u exceeds max_str_len(%zd)", l, u->max_str_len);
        return -1;
    }

    PyObject *py;

    if (u->encoding) {
        py = PyUnicode_Decode(p, l, u->encoding, u->unicode_errors);
    } else if (u->raw) {
        py = PyBytes_FromStringAndSize(p, l);
    } else {
        py = PyUnicode_DecodeUTF8(p, l, u->unicode_errors);
    }
    if (!py)
        return -1;
    *o = py;
    return 0;
}

static inline int unpack_callback_bin(unpack_user* u, const char* b, const char* p, unsigned int l, msgpack_unpack_object* o)
{
    if (l > u->max_bin_len) {
        PyErr_Format(PyExc_ValueError, "%u exceeds max_bin_len(%zd)", l, u->max_bin_len);
        return -1;
    }

    PyObject *py = PyBytes_FromStringAndSize(p, l);
    if (!py)
        return -1;
    *o = py;
    return 0;
}

static inline int unpack_callback_ext(unpack_user* u, const char* base, const char* pos,
                                      unsigned int length, msgpack_unpack_object* o)
{
    PyObject *py;
    int8_t typecode = (int8_t)*pos++;
    if (!u->ext_hook) {
        PyErr_SetString(PyExc_AssertionError, "u->ext_hook cannot be NULL");
        return -1;
    }
    if (length-1 > u->max_ext_len) {
        PyErr_Format(PyExc_ValueError, "%u exceeds max_ext_len(%zd)", length, u->max_ext_len);
        return -1;
    }
    // length also includes the typecode, so the actual data is length-1
#if PY_MAJOR_VERSION == 2
    py = PyObject_CallFunction(u->ext_hook, "(is#)", (int)typecode, pos, (Py_ssize_t)length-1);
#else
    py = PyObject_CallFunction(u->ext_hook, "(iy#)", (int)typecode, pos, (Py_ssize_t)length-1);
#endif
    if (!py)
        return -1;
    *o = py;
    return 0;
}

#include "unpack_template.h"
