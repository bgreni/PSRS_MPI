#! /usr/bin/python3

import pandas as pd
import os
import sys
from pprint import pprint
import pathlib
import numpy as np
import matplotlib.pyplot as plt

def config_in(newitem, allitems):
    for item in allitems:
        if int(newitem[0]) == int(item[0]) and int(newitem[1]) == int(item[1]):
            return True
    return False


def rounds(l):
    return [round(x, 4) for x in l]


if __name__ == '__main__':

    p = pathlib.Path(__file__).parent.absolute()
    content = open(os.path.join(p, 'results.txt')).read().split('\n\n')
    entries = [x.split('\n') for x in content]
    entries[-1] = entries[-1][:-1]

    for i in range(len(entries)):
        for j in range(len(entries[i])):
            entries[i][j] = float(entries[i][j].split(':')[1])

    seen = set()

    averaged = []
    for entry in entries:
        config = (entry[0], entry[1])
        if not config in seen:
            av_entry = []
            commons = list(filter(lambda x: x[0] == config[0] and x[1] == config[1], entries))
            for i in range(len(entry)):
                av_entry.append(np.mean([c[i] for c in commons]))
            averaged.append(av_entry)
            seen.add(config)

    # pprint(averaged)

    stuff = []
    for i in range(len(averaged)):
        e = averaged[i]
        if e[0] == 1:
            stuff.append({
                'cores': e[0],
                'size': e[1],
                'time': e[2],
                'p1time': None,
                'p2time': None,
                'p3time': None,
                'p4time': None,
                'speedup': 0,
                'pertotalp1': None,
                'pertotalp2': None,
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
                'size': e[1],
                'time': e[2],
                'p1time': e[3],
                'p2time': e[4],
                'p3time': e[5],
                'p4time': e[6],
                'speedup': float(averaged[int(i - ind)][2]) / float(e[2]),
                'pertotalp1': (float(e[3]) / float(e[2])) * 100,
                'pertotalp2': (float(e[4]) / float(e[2])) * 100,
                'pertotalp3': (float(e[5]) / float(e[2])) * 100,
                'pertotalp4': (float(e[6]) / float(e[2])) * 100,
                'speedpercore': (float(averaged[int(i - ind)][2]) / float(e[2])) / float(e[0])
            })

    df = pd.DataFrame(stuff)
    df.to_csv(os.path.join(p, 'out.csv'))

    sizes = list(map(int, set(df['size'].tolist())))

    lines = []
    colors = ['blue', 'red', 'black', 'pink', 'orange', 'olive', 'cyan', 'brown', 'green']

    plt.figure()
    plt.xticks([1, 2, 4, 6, 8])
    for i in range(len(sizes)):
        cores = list(map(int, df.query(f'size=={sizes[i]}')['cores'].tolist()))
        cores.insert(0, 0)
        speeds = rounds(df.query(f'size=={sizes[i]}')['speedup'].tolist())
        speeds.insert(0, 0)
        plt.plot(cores, speeds, color=colors[i], label=f'{sizes[i]}')
    plt.plot([0, 1, 2, 4, 6, 8], [0, 1, 2, 4, 6, 8], color='grey', label='Linear')
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Speedup')
    plt.title('Speedups')
    plt.savefig(os.path.join(p, 'speedups.png'))

    fig = plt.figure(figsize=(5, 1))
    ax = fig.add_subplot(1, 1, 1)
    cores = ['1 core', '2 cores', '4 cores', '6 cores', '8 cores']
    values = []
    for i in range(len(sizes)):
        times = rounds(df.query(f'size=={sizes[i]}')['time'].tolist())
        values.append(times)

    table = ax.table(cellText=values, rowLabels=sizes,
                     colLabels=cores, loc='upper center', colLoc='center',
                     rowLoc='center', cellLoc='center')

    table.scale(1, 1.5)
    ax.set_title('Times')
    ax.axis('off')
    # table.set_fontsize(12)
    plt.savefig(os.path.join(p, 'timestable.png'), pad_inches=0.5, bbox_inches='tight')
    print('FIGURE CREATION COMPLETE')








