import pygame
from pygame.locals import *
import json
from sys import argv

def read(ind):
    f = open('../second/train/%s.json'%ind)
    t = f.read()
    f.close()
    data = json.loads(t)
    return data['train']

def readAll():
    l = []
    for i in range(100):
        l.append(read(i))
    return [str(i) for i in range(100)], l

def readOutputs():
    f = open('visu.txt')
    t = f.read()
    f.close()
    names = []
    tasks = []
    for task in t.split('Task ')[1:]:
        task = task.split('Pair')
        names.append(task[0].strip())
        out = []
        for pair in task[1:]:
            outp = {}
            io = ['input','output']
            for i,img in enumerate(pair.split('Image')[1:]):
                rows = img.split('\n')
                w,h = map(int,rows[0].split())
                outp[io[i]] = []
                for row in rows[1:-1]:
                    outp[io[i]].append([int(i) for i in list(row)])
                if w*h == 0:
                    outp[io[i]] = [[9]]
            out.append(outp)
        tasks.append(out)
    return names, tasks

def main():
    sw, sh = 1500,800
    screen = pygame.display.set_mode((sw, sh))
    clock = pygame.time.Clock()

    pygame.key.set_repeat(200, 30)

    ni = 0
    ids, data = readOutputs()
    if len(data) == 0 or len(argv) > 1:
        ids, data = readAll()
        #hards = [12,16,21,23,26,33,38,41,44,45,48,49,52,55,63,64,65,66,69,72,74,77,79,86,87,91,95,97]
        #ids = [ids[i] for i in hards]
        #data = [data[i] for i in hards]
    task = data[ni]
    print("Read %d tasks"%len(data))

    pygame.display.set_caption(ids[ni] + ' - ' + str(ni))

    cols = [0x000000, 0x0074D9, 0xFF4136, 0x2ECC40, 0xFFDC00, 0xAAAAAA, 0xF012BE, 0xFF851B, 0x7FDBFF, 0x870C25]
    cols = [(i>>16&255, i>>8&255, i&255) for i in cols]

    while 1:
        for e in pygame.event.get():
            if e.type == QUIT or e.type == KEYDOWN and e.key == K_ESCAPE: return
            elif e.type == KEYDOWN:
                if e.key == K_RIGHT or e.key == K_d:
                    ni = (ni+1)%len(data)
                elif e.key == K_LEFT or e.key == K_a:
                    ni = (ni-1)%len(data)
                task = data[ni]
                pygame.display.set_caption(ids[ni] + ' - ' + str(ni))

        screen.fill((200,180,150))

        trains = len(task)
        tw = sw // max(trains,1)
        th = sh//2
        bw = 10
        offx = 0

        def draw(img, bw,bord):
            w = len(img[0])
            h = len(img)
            ret = pygame.Surface((w*(bw+bord)+bord,h*(bw+bord)+bord))
            ret.fill((0x60,0x60,0x60))
            for (r, row) in enumerate(img):
                for (c, col) in enumerate(row):
                    pygame.draw.rect(ret, cols[col], (c*(bw+bord)+bord,r*(bw+bord)+bord,bw,bw))
            return ret

        for io in task:
            offy = 0
            for img in [io['input'], io['output']]:
                h = len(img)
                if not h: continue
                w = len(img[0])

                mul = int(min(tw/w, th/h)*4/5)
                bd = max(mul//50,1)
                render = draw(img, mul-bd, bd)

                rw, rh = render.get_width(), render.get_height()
                #render = pygame.transform.scale(render, (int(rw*mul), int(rh*mul)))

                screen.blit(render, (offx+(tw-int(rw))//2, offy+(th-int(rh))//2))
                offy += th
            offx += tw
        pygame.display.flip()
        clock.tick(30)

if __name__ == "__main__": main()
