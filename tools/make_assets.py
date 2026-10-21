#!/usr/bin/env python3
"""Genere assets/icon.png (48x48), assets/banner.png (256x128), assets/audio.wav
Sans aucune dependance (Python 3 seulement). Usage : python3 tools/make_assets.py"""
import struct, zlib, wave, math, os

HEART = [".RR..RR.", "RRRRRRRR", "RRRRRRRR", "RRRRRRRR", ".RRRRRR.", "..RRRR..", "...RR...", "........"]
SPR = [
 ["................","......GGGG......","....GGGGGGGG....","...GGGGGGGGGG...","..GGGWWGGWWGGG..","..GGGWKGGWKGGG..",".GGGGGGGGGGGGGG.",".GGGGKKKKKKGGGG.","GGGGGGGGGGGGGGGG","GGGGGGGGGGGGGGGG",".GGG.GGGGGG.GGG.","................"],
 ["......RR........",".....RRR........","....SSSSSSSS....","...SSSSSSSSSS...","...SKKKKKKKKS...","...SKYKKKKYKS...","...SSSSSSSSSS...",".SSSSSSSSSSSSSS.","SSS.SSSSSSSS.SSS","SSS.SSSBBSSS.SSS","....SSSSSSSS....","....SSS..SSS...."],
 ["..P..........P..","...P...PP...P...","....P.PPPP.P....",".....PPPPPP.....","..PPPPWPPWPPPP..","PPP.PPPRRPPP.PPP","...PPPPPPPPPP...","..PPPPPPPPPPPP..","...PPPPPPPPPP...","..P..PPPPPP..P..",".P....PPPP....P.","P..............P"],
 ["....Y..Y..Y.....","....YYYYYYYY....","....WWWWWWWW....","...WWWWWWWWWW...","..WWKKWWWWKKWW..","..WWKKWWWWKKWW..","..WWWWWKKWWWWW..","..WWWWWWWWWWWW..","..WWWWKKKKWWWW..","..WWWWWWWWWWWW..","..WW.WWWW.WWWW..","..W...WW...WW..."],
 ["......CCCC......","....CCCCCCCC....","...CCCCCCCCCC...","..CCCRRCCRRCCC..","..CCCRRCCRRCCC..","..CCCCCCCCCCCC..","..CCWCWCWCWCCC..","..CCCCCCCCCCCC..",".CCCC.CCCC.CCCC.",".CCC..CCCC..CCC.","CCC...CCCC...CCC","CC....CCCC....CC"],
]
PAL = {'W':(255,255,255),'R':(255,32,32),'G':(48,208,80),'B':(48,112,255),'Y':(255,224,64),
       'P':(176,80,240),'C':(64,208,224),'S':(184,184,200),'K':(37,37,47)}

def canvas(w, h, bg):
    return [[bg] * w for _ in range(h)]

def rect(cv, x, y, w, h, c):
    for j in range(max(0, y), min(len(cv), y + h)):
        for i in range(max(0, x), min(len(cv[0]), x + w)):
            cv[j][i] = c

def sprite(cv, rows, x, y, s):
    for r, line in enumerate(rows):
        for c, ch in enumerate(line):
            if ch in PAL:
                rect(cv, x + c * s, y + r * s, s, s, PAL[ch])

def heart(cv, x, y, s, col=(255, 32, 32)):
    for r, line in enumerate(HEART):
        for c, ch in enumerate(line):
            if ch == 'R':
                rect(cv, x + c * s, y + r * s, s, s, col)

def save_png(path, cv):
    h, w = len(cv), len(cv[0])
    raw = b''.join(b'\x00' + bytes(v for p in row for v in (p[0], p[1], p[2], 255)) for row in cv)
    def chunk(t, d):
        c = struct.pack('>I', len(d)) + t + d
        return c + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))

os.makedirs('assets', exist_ok=True)

# Icone 48x48
ic = canvas(48, 48, (16, 16, 32))
rect(ic, 0, 0, 48, 2, (255, 255, 255)); rect(ic, 0, 46, 48, 2, (255, 255, 255))
rect(ic, 0, 0, 2, 48, (255, 255, 255)); rect(ic, 46, 0, 2, 48, (255, 255, 255))
heart(ic, 8, 8, 4)
save_png('assets/icon.png', ic)

# Banniere 256x128
bn = canvas(256, 128, (0, 0, 0))
heart(bn, 96, 8, 8)
for i, s in enumerate(SPR):
    sprite(bn, s, 8 + i * 49, 84, 3)
save_png('assets/banner.png', bn)

# Jingle court : 16 bits, mono, 22050 Hz
rate = 22050
notes = [(523, .18), (659, .18), (784, .18), (1047, .35), (784, .15), (1047, .5)]
frames = bytearray()
for f, d in notes:
    n = int(rate * d)
    for k in range(n):
        env = min(1.0, k / 200.0) * max(0.0, 1.0 - k / n)
        v = 1.0 if math.sin(2 * math.pi * f * k / rate) >= 0 else -1.0
        frames += struct.pack('<h', int(v * env * 9000))
with wave.open('assets/audio.wav', 'wb') as w:
    w.setnchannels(1); w.setsampwidth(2); w.setframerate(rate)
    w.writeframes(bytes(frames))
print("Assets crees dans assets/ : icon.png, banner.png, audio.wav")
