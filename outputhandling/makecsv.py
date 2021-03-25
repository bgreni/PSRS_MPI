import pandas as pd

"""
Parses the raw output from the PSRS program into a nice csv table used to create graphs and tables
"""

if __name__ == '__main__':
    content = open('test.out').read().split('\n\n')
    entries = [x.split('\n') for x in content]

    for i in range(len(entries)):
        for j in range(len(entries[i])):
            entries[i][j] = entries[i][j].split(':')[1][1:]

    stuff = []
    for i in range(len(entries)):
        e = entries[i]
        if e[0] == '1':
            stuff.append({
                'cores': e[0],
                'vector': e[1],
                'time': e[2],
                'p1time': None,
                'p2time': None,
                'p3time': None,
                'p4time': None,
                'speedup': 0,
                'pertotalp1': None,
                'pertotalp3': None,
                'pertotalp4': None,
                'speedpercore': None

            })
        else:
            if int(e[0]) == 16:
                ind = 5
            else:
                ind = int(e[0]) / 2
            stuff.append({
                'cores': e[0],
                'vector': e[1],
                'time': e[2],
                'p1time': e[3],
                'p2time': e[4],
                'p3time': e[5],
                'p4time': e[6],
                'speedup': float(entries[int(i - ind)][2]) / float(e[2]),
                'pertotalp1': (float(e[3]) / float(e[2])) * 100,
                'pertotalp3': (float(e[5]) / float(e[2])) * 100,
                'pertotalp4': (float(e[6]) / float(e[2])) * 100,
                'speedpercore': (float(entries[int(i - ind)][2]) / float(e[2])) / float(e[0])
            })

    df = pd.DataFrame(stuff)
    df.to_csv('out.csv')







