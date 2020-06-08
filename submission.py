from subprocess import *
from time import sleep, gmtime, ctime
from sys import *

def utctime():
    t = gmtime()
    return "%d-%02d-%02d %02d:%02d:%02d"%(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

pendings = []
while 1:
    output = check_output(["kaggle c submissions"], shell=True).decode("utf-8")
    for line in output.strip().split('\n')[3:]:
        line = line.strip()
        for i in range(10):
            line = line.replace('   ','  ')
        line = line.split('  ')
        date, status = line[1], line[3]
        if status == 'pending' and not date in pendings:
            print()
            print("Started  ", date)
            pendings.append(date)
        if date in pendings and status != 'pending':
            print()
            print("Finished  ", date, '-', utctime())
            Popen(["zenity --info --text 'Kaggle submission done!'"], shell=True)
            pendings.remove(date)

    sleep(1)
    print('.', end='')
    stdout.flush()
