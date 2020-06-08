import json

def read(fn):
    f = open(fn)
    t = f.read()
    f.close()
    return t

t = read('submission_part.csv')

total, correct = 0, 0
for line in t.strip().split('\n')[1:]:
    id, imgs = line.split(',')
    hash, test_ind = id.split('_')
    fn = 'dataset/evaluation/'+hash+'.json'
    with open(fn) as f:
        truth = json.load(f)['test'][int(test_ind)]['output']
        truth = '|'+'|'.join(''.join(str(j) for j in line) for line in truth)+'|'
        total += 1
        correct += truth in imgs.split()

print(correct, '/', total)
