#!/usr/bin/env python

from subprocess import *
from concurrent.futures import ThreadPoolExecutor as Pool
import os
import sys
import resource
import psutil
from random import *
import time
import math

from os import system
from glob import glob

call(['make','-j'])
call(['make','-j','count_tasks'])

SUCCESS, TLE, MLE, RTE, RUNNING = 0,1,2,3,-1
exit_names = ["SUCCESS", "TLE", "MLE", "RTE", "RUNNING"]

start_time = time.time()


MEMORY_LIMIT = 4*4096 * 0.95 # MB
TIME_LIMIT   = 9*60*60 * 0.95 # Seconds


class Process:
    def __init__(self, cmd, timeout, maxmemory):
        fn = cmd.replace(' ','_')
        self.fout = open('store/tmp/%s.out'%fn,'w')
        self.ferr = open('store/tmp/%s.err'%fn,'w')
        print(cmd)
        sys.stdout.flush()
        self.cmd = cmd
        self.process = Popen(cmd.split(), stdout=self.fout, stderr=self.ferr, shell=False)
        self.pid = self.process.pid
        self.mp = psutil.Process(self.pid)
        self.memused, self.timeused = 0, 0
        self.start_time = time.time()
        self.timeout = timeout
        self.maxmemory = maxmemory

    def update(self):
        self.memory = self.mp.memory_info().rss/2**20
        self.memused = max(self.memused, self.memory)
        self.timeused = time.time()-self.start_time
        if self.memory > self.maxmemory:
            return (MLE, self.timeused, self.memused)
        if self.timeused > self.timeout:
            return (TLE, self.timeused, self.memused)
        if not self.memory:
            if self.process.wait():
                return (RTE, self.timeused, self.memused)
            else:
                return (SUCCESS, self.timeused, self.memused)
        return (RUNNING, self.timeused, self.memused)

    def __del__(self):
        self.fout.close()
        self.ferr.close()


class Command:
    def __init__(self, cmd, expected_time = TIME_LIMIT, expected_memory = MEMORY_LIMIT, slack = 1.5):
        self.cmd = cmd
        self.time = expected_time
        self.mem = expected_memory
        self.slack = slack

    def __lt__(self, other):
        return self.time < other.time


def runAll(cmd_list, threads):
    THREAD_LIMIT = threads

    ret_stats = {}

    cmd_list = sorted(cmd_list)

    dt = 0.1
    running = []
    cmdi = 0

    def callback(process, status, timeused, memused):
        assert(status != RTE)
        print(exit_names[status], process.cmd, " %.1fs"%timeused, "%.0fMB"%memused)
        sys.stdout.flush()

        ret_stats[process.cmd] = (status, timeused, memused)

    while len(running) or cmdi < len(cmd_list):
        while cmdi < len(cmd_list) and len(running) < THREAD_LIMIT:
            cmd = cmd_list[cmdi]
            process = Process(cmd.cmd, cmd.time*cmd.slack, cmd.mem*cmd.slack)
            running.append(process)
            cmdi += 1

        torem = []
        mems = []
        for r in running:
            status, timeused, memused = r.update()
            mems.append(r.memory)
            if status != RUNNING:
                callback(r, status, timeused, memused)
                torem.append(r)

        if sum(mems) > MEMORY_LIMIT:
            r = running[mems.index(max(mems))]
            r.process.kill()
            callback(r, MLE, r.timeused, r.memused)
            torem.append(r)
            #THREAD_LIMIT = 1

        for r in torem:
            running.remove(r)

        time.sleep(dt)

        if time.time()-start_time >= TIME_LIMIT:
            for r in running:
                r.process.kill()
                callback(r, TLE, r.timeused, r.memused)
            break

    return ret_stats



system("mkdir -p output")
system("mkdir -p store/tmp")
system("rm -f output/answer*.csv")

if len(sys.argv) == 3:
    l = int(sys.argv[1])
    n = int(sys.argv[2])
    ntasks = n
    task_list = range(l, l+n)
else:
    ntasks = int(check_output('./count_tasks'))
    task_list = range(0, ntasks)
    #print("Usage: python %s <start_task> <#tasks>"%sys.argv[0])

#TODO: change back to depth 3/4
depth3 = []
for i in range(ntasks):
    depth3.append(Command("./run %d 3"%i))
stats3 = runAll(depth3, 4)

flip3 = []
for i in range(ntasks):
    status, t, m = stats3[depth3[i].cmd]
    flip3.append(Command("./run %d 23"%i, t*2, m*2, 100))
stats3_flip = runAll(flip3, 4)

flip3 = []
for i in range(ntasks):
    status, t, m = stats3[depth3[i].cmd]
    flip3.append(Command("./run %d 33"%i, t*2, m*2, 100))
runAll(flip3, 4)

depth4 = []
for i in range(ntasks):
    status, t, m = stats3[depth3[i].cmd]
    depth4.append(Command("./run %d 4"%i, t*20, m*20, 2))
stats4 = runAll(depth4, 2)

def read(fn):
    f = open(fn)
    t = f.read()
    f.close()
    return t

combined = ["output_id,output"]
for taski in task_list:
    ids = set()
    cands = []
    for fn in glob("output/answer_%d_*.csv"%taski):
        t = read(fn).strip().split('\n')
        ids.add(t[0])
        for cand in t[1:]:
            img, score = cand.split()
            cands.append((float(score), img))

    assert(len(ids) == 1)
    id = ids.pop()

    cands.sort(reverse=True)
    best = []
    for cand in cands:
        score, img = cand
        if not img in best:
            best.append(img)
            if len(best) == 3:
                break
    if not best: best.append('|0|')
    combined.append(id+','+' '.join(best))

outf = open('submission_part.csv', 'w')
for line in combined:
    print(line, file=outf)
outf.close()
