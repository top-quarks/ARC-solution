from sys import argv
assert(len(argv) == 3)
a = argv[1]
b = argv[2]
cnt = [[0 for i in range(4)] for j in range(4)]
for i in range(len(a)):
    cnt[int(a[i])][int(b[i])] += 1
    #if a[i] != b[i]:
    #    print(i,a[i],b[i])
for j in range(4):
    w = 0
    for i in range(4):
        w = max(w, len(str(cnt[i][j])))
    for i in range(4):
        cnt[i][j] = str(cnt[i][j]).rjust(w)

for i in range(4):
    print(' '.join(cnt[i]))
