=================================================
MessagePack with Ethereum RLP encoding for Python
=================================================

This library uses a modified version of msgpack to encode and decode binary
data according to the Ethereum RLP specification here https://github.com/ethereum/wiki/wiki/RLP

Because it is based on msgpack, with a back-end in cython, it is lightning fast.

This is the first release of this library, and there are likely still bugs. Use at your own risk.

Install
-------

::

$ pip install msgpack-rlp-python

USAGE
-------

RLP doesn't specify the variable type, so all encoded data is decoded to a bytestring representation.
In order to specify the variable types, we use sedes. Sedes are ways of telling
the program what variable types you expect from the encoded data. In this version
there are only 2 supported sedes: bytes (represented by the integer 0) and integers (represented by the integer 1). These sedes can also contained
within lists or nested lists. In order to show that the sedes live within lists or nested
lists, we use the standard python list notation. See below.

- All lists will be decoded to tuples by default. To decode to a list set use_list = True

- Sedes are only used when decoding. This library is smart enough to know what variable types are being encoded, and will convert them to the RLP spec correctly.

- This is meant to be a stand-in replacement of msgpack. So the package to import is still called msgpack.


Example sedes:

.. code-block:: pycon

   var = b'\x01'
   sede = 0 # A bytestring

   var = 12381239
   sede = 1 # An integer

   var = [b'\x01', b'\x02', b'\x03', b'\x04']
   sede = [0] # A list of bytestrings

   var = [12312,1234,213412,213421]
   sede = [1] # A list of integers

   var = [b'\x01', [12312,1234]]
   sede = [0,[1]] # Different types, and nested lists


Example usage without sedes. This will always decode to bytestrings.

.. code-block:: pycon

   >>> import msgpack
   >>> msgpack.packb([1, 2, 3])
   'b'\xc3\x01\x02\x03''
   >>> msgpack.unpackb(_)
   (b'\x01', b'\x02', b'\x03')

   >>> msgpack.packb([1, 2, 3])
   'b'\xc3\x01\x02\x03''
   >>> msgpack.unpackb(_, use_list = True)
   [b'\x01', b'\x02', b'\x03']

Example usage with sedes. This will decode to the variable types that you encoded from.

.. code-block:: pycon

   >>> import msgpack
   >>> msgpack.packb([1, 2, 3])
   'b'\xc3\x01\x02\x03''
   >>> msgpack.unpackb(_)
   (b'\x01', b'\x02', b'\x03')

   >>> msgpack.packb([1, 2, 3])
   'b'\xc3\x01\x02\x03''
   >>> msgpack.unpackb(_, sedes=[1], use_list=True)
   [1, 2, 3]

   >>> msgpack.packb([b'\x01', [12312,1234]])
   b'\xc8\x01\xc6\x820\x18\x82\x04\xd2'
   >>> msgpack.unpackb(_, sedes=[0,[1]], use_list=True)
   [b'\x01', [12312, 1234]]

   >>> msgpack.packb([b'\x01', [12312,1234]])
   b'\xc8\x01\xc6\x820\x18\x82\x04\xd2'
   >>> msgpack.unpackb(_, sedes=[0,[1]], use_list=True)
   [b'\x01', [12312, 1234]]


