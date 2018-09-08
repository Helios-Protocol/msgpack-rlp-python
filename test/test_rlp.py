import sys
import rlp
import msgpack


#Didn't do tests beyond a length of 2**26 because it can cause system instability. The variables just get too large.
for i in range(1,26):
    print("trying list with byte length = {} bytes".format(i))
    to_encode = [(2**i) * b'0']

    rlp_encoded = rlp.encode(to_encode)
    msg_encoded = msgpack.packb(to_encode)

    #print(rlp_encoded)
    #print(msg_encoded)

    if rlp_encoded != msg_encoded:
        print('encoding error found')
        print(rlp_encoded[:100])
        print(msg_encoded[:100])
        sys.exit()

    msg_decoded = msgpack.unpackb(msg_encoded)

    if to_encode != msg_decoded:
        print('decoding error found')
        print(to_encode[:100])
        print(msg_decoded[:100])
        sys.exit()


for i in range(1,26):
    print("trying string with length = {} bytes".format(i))
    to_encode = (2**i) * '0'

    rlp_encoded = rlp.encode(to_encode)
    msg_encoded = msgpack.packb(to_encode)

    #print(rlp_encoded)
    #print(msg_encoded)

    if rlp_encoded != msg_encoded:
        print('encoding error found')
        print(rlp_encoded[:100])
        print(msg_encoded[:100])
        sys.exit()

    msg_decoded = msgpack.unpackb(msg_encoded)

    if to_encode != msg_decoded:
        print('decoding error found')
        print(to_encode[:100])
        print(msg_decoded[:100])
        sys.exit()

sys.exit()