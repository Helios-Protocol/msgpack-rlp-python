# coding: utf-8
#cython: embedsignature=True, c_string_encoding=ascii

from cpython cimport *
from cpython.version cimport PY_MAJOR_VERSION
from cpython.exc cimport PyErr_WarnEx

from msgpack.exceptions import PackValueError, PackOverflowError


cdef extern from "Python.h":

    int PyMemoryView_Check(object obj)
    int PyByteArray_Check(object obj)
    int PyByteArray_CheckExact(object obj)
    char* PyUnicode_AsUTF8AndSize(object obj, Py_ssize_t *l) except NULL


cdef extern from "pack.h":
    struct msgpack_packer:
        char* buf
        size_t length
        size_t buf_size
        bint use_bin_type

    int msgpack_pack_int(msgpack_packer* pk, int d)
    int msgpack_pack_nil(msgpack_packer* pk)
    int msgpack_pack_true(msgpack_packer* pk)
    int msgpack_pack_false(msgpack_packer* pk)
    int msgpack_pack_long(msgpack_packer* pk, long d)
    int msgpack_pack_long_long(msgpack_packer* pk, long long d)
    int msgpack_pack_unsigned_long_long(msgpack_packer* pk, unsigned long long d)
    int msgpack_pack_float(msgpack_packer* pk, float d)
    int msgpack_pack_double(msgpack_packer* pk, double d)
    int msgpack_pack_array(msgpack_packer* pk, size_t l, size_t position)
    int msgpack_clear_spot_for_start_byte(msgpack_packer* pk)
    int msgpack_pack_map(msgpack_packer* pk, size_t l)
    int msgpack_pack_raw(msgpack_packer* pk, size_t l)
    int msgpack_pack_bin(msgpack_packer* pk, size_t l)
    int msgpack_pack_raw_body(msgpack_packer* pk, char* body, size_t l)
    int msgpack_pack_ext(msgpack_packer* pk, char typecode, size_t l)
    int msgpack_pack_unicode(msgpack_packer* pk, object o, long long limit)


cdef int DEFAULT_RECURSE_LIMIT=511
#from https://github.com/ethereum/wiki/wiki/RLP
cdef unsigned long long ITEM_LIMIT = (2**64)-1


cdef inline int PyBytesLike_Check(object o):
    return PyBytes_Check(o) or PyByteArray_Check(o)


cdef inline int PyBytesLike_CheckExact(object o):
    return PyBytes_CheckExact(o) or PyByteArray_CheckExact(o)


cdef class Packer(object):
    """
    MessagePack Packer

    usage::

        packer = Packer()
        astream.write(packer.pack(a))
        astream.write(packer.pack(b))

    Packer's constructor has some keyword arguments:

    :param callable default:
        Convert user type to builtin type that Packer supports.
        See also simplejson's document.

    :param bool use_single_float:
        Use single precision float type for float. (default: False)

    :param bool autoreset:
        Reset buffer after each pack and return its content as `bytes`. (default: True).
        If set this to false, use `bytes()` to get content and `.reset()` to clear buffer.

    :param bool use_bin_type:
        Use bin type introduced in msgpack spec 2.0 for bytes.
        It also enables str8 type for unicode.
        Current default value is false, but it will be changed to true
        in future version.  You should specify it explicitly.

    :param bool strict_types:
        If set to true, types will be checked to be exact. Derived classes
        from serializeable types will not be serialized and will be
        treated as unsupported type and forwarded to default.
        Additionally tuples will not be serialized as lists.
        This is useful when trying to implement accurate serialization
        for python types.

    :param str unicode_errors:
        Error handler for encoding unicode. (default: 'strict')

    :param str encoding:
        (deprecated) Convert unicode to bytes with this encoding. (default: 'utf-8')
    """
    cdef msgpack_packer pk
    cdef object _default
    cdef object _bencoding
    cdef object _berrors
    cdef const char *encoding
    cdef const char *unicode_errors
    cdef bint strict_types
    cdef bool use_float
    cdef bint autoreset

    def __cinit__(self):
        cdef int buf_size = 1024*2024
        self.pk.buf = <char*> PyMem_Malloc(buf_size)
        if self.pk.buf == NULL:
            raise MemoryError("Unable to allocate internal buffer.")
        self.pk.buf_size = buf_size
        self.pk.length = 0

    def __init__(self, default=None, encoding=None, unicode_errors=None,
                 bint use_single_float=False, bint autoreset=True, bint use_bin_type=False,
                 bint strict_types=False):
        if encoding is not None:
            PyErr_WarnEx(PendingDeprecationWarning, "encoding is deprecated.", 1)
        self.use_float = use_single_float
        self.strict_types = strict_types
        self.autoreset = autoreset
        self.pk.use_bin_type = use_bin_type
        if default is not None:
            if not PyCallable_Check(default):
                raise TypeError("default must be a callable.")
        self._default = default

        self._bencoding = encoding
        if encoding is None:
            if PY_MAJOR_VERSION < 3:
                self.encoding = 'utf-8'
            else:
                self.encoding = NULL
        else:
            self.encoding = self._bencoding

        self._berrors = unicode_errors
        if unicode_errors is None:
            self.unicode_errors = NULL
        else:
            self.unicode_errors = self._berrors

    def __dealloc__(self):
        PyMem_Free(self.pk.buf)
        self.pk.buf = NULL

    cdef int _pack(self, object o, int nest_limit=DEFAULT_RECURSE_LIMIT) except -1:
        cdef long long llval
        cdef unsigned long long ullval
        cdef long longval
        cdef float fval
        cdef double dval
        cdef char* rawval
        cdef int ret
        cdef dict d
        cdef Py_ssize_t L
        cdef int default_used = 0
        cdef bint strict_types = self.strict_types
        cdef Py_buffer view

        if nest_limit < 0:
            raise PackValueError("recursion limit exceeded.")

        while True:
            if o is None:
                ret = msgpack_pack_nil(&self.pk)

            elif PyBytesLike_CheckExact(o) if strict_types else PyBytesLike_Check(o):
                #this is a python byte array
                L = len(o)
                if L > ITEM_LIMIT:
                    raise PackValueError("%s is too large" % type(o).__name__)
                rawval = o
                #ret = msgpack_pack_bin(&self.pk, L)
                #we don't have a special type for binary, so lets skip this and use the same function as unicode
                if L == 1 and ord(o) < 0x80:
                    #print('encoding single byte as is')
                    ret = msgpack_pack_raw_body(&self.pk, rawval, L)
                else:
                    #print('encoding byte string normally')
                    #print('length = {}'.format(L))
                    ret = msgpack_pack_raw(&self.pk, L)
                    if ret == 0:
                        ret = msgpack_pack_raw_body(&self.pk, rawval, L)

            elif PyUnicode_CheckExact(o) if strict_types else PyUnicode_Check(o):
                ret = msgpack_pack_unicode(&self.pk, o, ITEM_LIMIT);
                if ret == -2:
                    raise PackValueError("unicode string is too large")


            elif PyLong_CheckExact(o) if strict_types else PyLong_Check(o):
                # PyInt_Check(long) is True for Python 3.
                # So we should test long before int.
                try:
                    if o >= 0:
                        #print("saving unsigned long")
                        ullval = o
                        ret = msgpack_pack_unsigned_long_long(&self.pk, ullval)
                    else:
                        raise PackValueError("Cannot encode negative numbers to big endian int.")
                        # llval = o
                        # ret = msgpack_pack_long_long(&self.pk, llval)
                except OverflowError as oe:
                    if not default_used and self._default is not None:
                        o = self._default(o)
                        default_used = True
                        continue
                    else:
                        raise PackOverflowError("Integer value out of range for fast automatic sedes. Use python fallback instead")


            elif PyList_CheckExact(o) if strict_types else (PyTuple_Check(o) or PyList_Check(o)):
                #this is a python list
                L = len(o)
                if L > ITEM_LIMIT:
                    raise PackValueError("list is too large")
                #save a position in the buffer for the initial byte. we need to see how big the list is before we know what to write there
                start_byte_position = self.pk.length
                #msgpack_clear_spot_for_start_byte(&self.pk);
                if ret == 0:
                    for v in o:
                        ret = self._pack(v, nest_limit-1)
                        if ret != 0: break

                total_length_of_list_in_bytes = self.pk.length - start_byte_position
                ret = msgpack_pack_array(&self.pk, total_length_of_list_in_bytes, start_byte_position);

            elif not default_used and self._default:
                #this allows for adding additional objects
                o = self._default(o)
                default_used = 1
                continue
            else:
                raise TypeError("can't serialize %r" % (o,))
            return ret

    cpdef pack(self, object obj):
        cdef int ret
        try:
            ret = self._pack(obj, DEFAULT_RECURSE_LIMIT)
        except:
            self.pk.length = 0
            raise
        if ret:  # should not happen.
            raise RuntimeError("internal error")
        buf = PyBytes_FromStringAndSize(self.pk.buf, self.pk.length)
        if self.autoreset:
            self.pk.length = 0
        return buf

    def get_length(self):
        return self.pk.length

    def reset(self):
        """Clear internal buffer."""
        self.pk.length = 0

    def bytes(self):
        """Return buffer content."""
        return PyBytes_FromStringAndSize(self.pk.buf, self.pk.length)
