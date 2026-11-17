"""
UTF-8 variable-width encoding pipeline to demonstrate how high-level text
transforms into binary sequences.
"""

from bit import BitVector

def encodeUtf8Bytes(codePoint: int) -> [BitVector]:
    """Encodes a Unicode Code Point into an array of 8-bit BitVectors according to the UTF-8
specification."""
    if codePoint < 0 or codePoint > 0x10ffff:
        raise ValueError(f'Invalid Unicode code point ({hex(codePoint)}), range is up to U+10FFFF')

    if (codePoint <= 0x7f): # 1-Byte sequence (ASCII compatible): 0xxxxxxx
        return [BitVector(codePoint, 8)]

    elif (codePoint <= 0x7ff): # 2-Byte sequence: 110xxxxx 10xxxxxx
        byte1 = 0b11000000 | (codePoint >> 6)
        byte2 = 0b10000000 | codePoint & 0b00111111
        return [BitVector(byte1, 8), BitVector(byte2, 8)]

    elif (codePoint <= 0xffff): # 3-Byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        byte1 = 0b11100000 | (codePoint >> 12)
        byte2 = 0b10000000 | (codePoint >> 6) & 0b00111111
        byte3 = 0b10000000 | codePoint & 0b00111111
        return [BitVector(byte1, 8), BitVector(byte2, 8), BitVector(byte3, 8)]

    else: # 4-Byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        byte1 = 0b11110000 | (codePoint >> 18)
        byte2 = 0b10000000 | ((codePoint >> 12) & 0b00111111)
        byte3 = 0b10000000 | ((codePoint >> 6) & 0b00111111)
        byte4 = 0b10000000 | codePoint & 0b00111111
        return [BitVector(byte1, 8), BitVector(byte2, 8), BitVector(byte3, 8), BitVector(byte4, 8)]

def decodeUtf8Bytes(utf8Bytes: [BitVector]) -> int:
    """Decodes a valid sequence of 8-bit BitVectors into a single Unicode code point."""
    if len(utf8Bytes) == 0:
        raise ValueError('Byte sequence cannot be empty')
    for b in utf8Bytes:
        if b.width != 8:
            raise ValueError(f'BitVectors in array must all be of size 8 (have {b.width})')
    rawBytes = [a.asUnsignedInteger() for a in utf8Bytes]

    if (rawBytes[0] & 0b10000000) == 0: # 1-Byte sequence detection
        if len(rawBytes) > 1:
            raise ValueError('Expected exactly 1 byte for this UTF-8 sequence')
        return rawBytes[0]
    
    elif (rawBytes[0] & 0b11100000) == 0b11000000: # 2-Byte sequence detection
        if len(rawBytes) > 2:
            raise ValueError('Expected exactly 2 bytes for this UTF-8 sequence')
        if (rawBytes[1] & 0b11000000) != 0b10000000:
            raise ValueError('Malformed continuation byte')
        return ((rawBytes[0] & 0b00011111) << 6) | (rawBytes[1] & 0b00111111)

    elif (rawBytes[0] & 0b11110000) == 0b11100000: # 3-Byte sequence detection
        if len(rawBytes) > 3:
            raise ValueError('Expected exactly 3 bytes for this UTF-8 sequence')
        if (rawBytes[1] & 0b11000000) != 0b10000000 or (rawBytes[2] & 0b11000000) != 0b10000000:
            raise ValueError('Malformed continuation byte')
        return ((rawBytes[0] & 0b00001111) << 12) | ((rawBytes[1] & 0b00111111) << 6) | (rawBytes[2] & 0b00111111)

    elif (rawBytes[0] & 0b11111000) == 0b11110000: # 4-Byte sequence detection
        if len(rawBytes) > 4:
            raise ValueError('Expected exactly 4 bytes for this UTF-8 sequence')
        if (rawBytes[1] & 0b11000000) != 0b10000000 or (rawBytes[2] & 0b11000000) != 0b10000000 or (rawBytes[3] & 0b11000000) != 0b10000000:
            raise ValueError('Malformed continuation byte')
        return ((rawBytes[0] & 0b00000111) << 18) | ((rawBytes[1] & 0b00111111) << 12) | ((rawBytes[2] & 0b00111111) << 6) | (rawBytes[3] & 0b00111111)

    else:
        raise ValueError(f'Not a valid UTF-8 lead byte ({bin(rawBytes[0])})')

if __name__ == '__main__':
    print(f'1-Byte sequence for U+41 is {encodeUtf8Bytes(0x41)}')
    print(f'2-Byte sequence for U+a3 is {encodeUtf8Bytes(0xa3)}')
    print(f'3-Byte sequence for U+20ac is {encodeUtf8Bytes(0x20ac)}')
    print(f'4-Byte sequence for U+1f3f4 is {encodeUtf8Bytes(0x1f3f4)}')
    for u in range(0x110000):
        if u % 0x1000 == 0:
            print('.', end='', flush=True)
        if u != decodeUtf8Bytes(encodeUtf8Bytes(u)):
            print(f'\nCodepoint {hex(u)} does not encode and decode correctly')
            break
    else:
        print('\nAll Unicode codepoints encode and decode correctly')
    try:
        print(hex(decodeUtf8Bytes([BitVector('11111000')])))
    except:
        print('Invalid UTF-8 sequence trapped as expected')