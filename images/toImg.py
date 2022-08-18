#!/usr/bin/python3

import sys, os, math
import png


# Indexed, RLE rows, 8 bit
def method4(src, outFile):
    width = src[0]
    height = src[1]
    size = 2
    for row in src[2]:
        colors = {}
        values = [0 for i in range(width)]
        for i, v in enumerate(row):
            if i % 4 == 0:
                p = max(row[i:i+3])
                values[int(i / 4)] = p
                if p > 0:
                    colors[p] = 1
                    
        if len(colors) > 0:
            color = -1
            count = 0
            rle = []
            for v in values:
                if (v == color):
                    count += 1
                else:
                    if (color != -1):
                        rle.append((count, color))
                    color = v
                    count = 1
            if count > 0:
                rle.append((count, color))
            bits = int(math.log2(len(colors)) + 1)
            if bits > 5:
                print('Required bits is {}!'.format(bits))
                return -1
            size += 1 + len(colors)
            for v in rle:
                if v[0] < 4:
                    size += 1
                elif v[0] < 512:
                    size += 2
        else:
            size += 1
    return size
    
# RLE, 5 bit
def method5(src, outFile):
    
    return len(data)
    
def convert(inFile, outFile):
    src = list(png.Reader(inFile).read())
    src[2] = list(src[2])
    width = src[0]
    height = src[1]
    color = -1
    count = 0
    rle = []
    for row in src[2]:
        for i, v in enumerate(row):
            if i % 4 == 0:
                p = max(row[i:i+3]) >> 3
                if p == color:
                    count += 1
                else:
                    if color != -1:
                        rle.append((count, color))
                    color = p
                    count = 1
                if p == 0:
                    print(' ', end='')
                elif p < 16:
                    print(hex(p)[2:], end='')
                else:
                    print(hex(p-16)[2:].upper(), end='')
        print()
        
    if count > 0:
        rle.append((count, color))
        
    data = [width, height, 0, 0]
    for v in rle:
        if v[0] < 4:
            data.append((0 << 7) | ((v[0] & 0x0003) << 5) | v[1])
        elif v[0] < 512:
            data.append((1 << 7) | ((v[0] & 0x01fc) >> 2))
            data.append((0 << 7) | ((v[0] & 0x0003) << 5) | v[1])
        elif v[0] < 65536:
            data.append((1 << 7) | ((v[0] & 0xfe00) >> 9))
            data.append((1 << 7) | ((v[0] & 0x01fc) >> 2))
            data.append((0 << 7) | ((v[0] & 0x0003) << 5) | v[1])
    data.append(0)
    
    data[2] = ((len(data) - 4) & 0xff00) >> 8
    data[3] = ((len(data) - 4) & 0x00ff)
    
    file = open(outFile, 'wb')
    file.write(bytes(data))
    file.close()
    print('{} ({})-> {} ({})'.format(inFile, os.path.getsize(inFile), outFile, os.path.getsize(outFile)))
    return len(data)
    

if __name__ == '__main__':
    totalSize = 0
    for inFile in sys.argv[1:]:
        if inFile[-4:] != '.png':
            print('skipped {}'.format(inFile))
            continue
        outFile = inFile[:-3] + 'img'
        totalSize += convert(inFile, outFile)
    print('Total size: {}'.format(totalSize))
    
    
