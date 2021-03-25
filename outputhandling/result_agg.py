#! /usr/bin/python3

import pandas as pd
import os
import sys
from pprint import pprint
import pathlib
import numpy as np
import matplotlib.pyplot as plt
import begin


"""
Big dumb script to generate graphs and tables, that should have just done in a loop
"""


def config_in(newitem, allitems):
    for item in allitems:
        if int(newitem[0]) == int(item[0]) and int(newitem[1]) == int(item[1]):
            return True
    return False


def rounds(l):
    return [round(x, 4) for x in l]


@begin.start(auto_convert=True)
def main(local=False):

    base = 'local' if local else ''

    p = pathlib.Path(__file__).parent.absolute()
    content = open(os.path.join(p, base + 'results.txt')).read().split('\n\n')
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
                'perp1': None,
                'perp2': None,
                'perp3': None,
                'perp4': None,
                'speedpercore': None

            })
        else:
            if int(e[0]) == 8:
                ind = 4
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
                'perp1': (float(e[3]) / float(e[2])) * 100,
                'perp2': (float(e[4]) / float(e[2])) * 100,
                'perp3': (float(e[5]) / float(e[2])) * 100,
                'perp4': (float(e[6]) / float(e[2])) * 100,
                'speedpercore': (float(averaged[int(i - ind)][2]) / float(e[2])) / float(e[0])
            })

    df = pd.DataFrame(stuff)
    df.to_csv(os.path.join(p, 'out.csv'))

    sizes = sorted(list(map(int, set(df['size'].tolist()))))

    lines = []
    colors = ['blue', 'red', 'black', 'pink', 'orange', 'olive', 'cyan', 'brown', 'green']
    cores = [0, 1, 2, 4, 6, 8]
    ticks = [0, 2, 4, 6, 8]

    # big speedup graph
    plt.figure()
    plt.xticks(ticks)
    for i in range(len(sizes)):
        speeds = rounds(df.query(f'size=={sizes[i]}')['speedup'].tolist())
        speeds.insert(0, 0)
        plt.plot(cores, speeds, color=colors[i], label=f'{sizes[i]}')
    plt.plot(cores, cores, color='grey', label='Linear')
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Speedup')
    plt.title('Speedups')
    plt.savefig(os.path.join(p, base + 'speedups.png'))


    # total times table
    fig = plt.figure(figsize=(5, 1))
    ax = fig.add_subplot(1, 1, 1)
    cors = ['1 core', '2 cores', '4 cores', '6 cores', '8 cores']
    values = []
    for i in range(len(sizes)):
        times = rounds(df.query(f'size=={sizes[i]}')['time'].tolist())
        values.append(times)

    table = ax.table(cellText=values, rowLabels=sizes,
                     colLabels=cors, loc='upper center', colLoc='center',
                     rowLoc='center', cellLoc='center')

    table.scale(1, 1.5)
    ax.set_title('Times (In Seconds)')
    ax.axis('off')
    plt.savefig(os.path.join(p, base + 'timestable.png'), pad_inches=0.5, bbox_inches='tight')


    # single speedup graph
    plt.figure()
    plt.xticks(ticks)
    speeds = rounds(df.query(f'size=={sizes[-1]}')['speedup'].tolist())
    speeds.insert(0, 0)
    plt.plot(cores, speeds, color='red', label=f'{sizes[-1]}')
    plt.plot(cores, cores, color='grey', label='Linear')
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Speedup')
    plt.title('Speedups')
    plt.savefig(os.path.join(p, base + 'singlepeedup.png'))


    # REAL TIME PHASE BY PHASE
    plt.figure()
    plt.xticks(ticks)
    ind = -1
    qres = df.query(f'size=={sizes[ind]}')
    p1 = qres['p1time'].tolist()
    p2 = qres['p2time'].tolist()
    p3 = qres['p3time'].tolist()
    p4 = qres['p4time'].tolist()
    c = cores[1:]

    plt.plot(c, p1, color='red', label='Phase 1')
    plt.fill_between(c, p1, p3, facecolor='red', alpha=0.3)
    plt.plot(c, p3, color='blue', label='Phase 3')
    plt.fill_between(c, p3, p4, facecolor='blue', alpha=0.3)
    plt.plot(c, p4, color='green', label='Phase 4')
    plt.fill_between(c, p4, facecolor='green', alpha=0.3)
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Real Time')
    plt.title('Phase-by-Phase Analysis (Real Time)' + ' ({})'.format(sizes[ind]))

    plt.savefig(os.path.join(p, base + 'phasetotaltime.png'))

    # PERCENTAGE REAL TIME PHASE BY PHASE
    plt.figure()
    plt.xticks(ticks)

    qres = df.query(f'size=={sizes[ind]}')
    p1 = qres['perp1'].tolist()
    p2 = qres['perp2'].tolist()
    p3 = qres['perp3'].tolist()
    p4 = qres['perp4'].tolist()
    c = cores[1:]

    plt.plot(c, p1, color='red', label='Phase 1')
    plt.fill_between(c, p1, p3, facecolor='red', alpha=0.3)
    plt.plot(c, p3, color='blue', label='Phase 3')
    plt.fill_between(c, p3, p4, facecolor='blue', alpha=0.3)
    plt.plot(c, p4, color='green', label='Phase 4')
    plt.fill_between(c, p4, facecolor='green', alpha=0.3)
    plt.xlabel('Processors')
    plt.ylabel('Percentage Real Time')
    plt.title('Phase-by-Phase Analysis (Percentage Real Time)' + ' ({})'.format(sizes[ind]))

    plt.savefig(os.path.join(p, base + 'phaseperime.png'))


    # speedup table
    fig = plt.figure(figsize=(5, 1))
    ax = fig.add_subplot(1, 1, 1)
    cors = ['2 cores', '4 cores', '6 cores', '8 cores']
    values = []
    for i in range(len(sizes)):
        speeds = rounds(df.query(f'size=={sizes[i]}')['speedup'].tolist())
        values.append(speeds[1:])

    table = ax.table(cellText=values, rowLabels=sizes,
                     colLabels=cors, loc='upper center', colLoc='center',
                     rowLoc='center', cellLoc='center')

    table.scale(1, 1.5)
    ax.set_title('Speedups')
    ax.axis('off')
    plt.savefig(os.path.join(p, base + 'speedupstable.png'), pad_inches=0.5, bbox_inches='tight')


    ind = -2
    # single speedup graph for second largest
    plt.figure()
    plt.xticks(ticks)
    speeds = rounds(df.query(f'size=={sizes[ind]}')['speedup'].tolist())
    speeds.insert(0, 0)
    plt.plot(cores, speeds, color='red', label=f'{sizes[ind]}')
    plt.plot(cores, cores, color='grey', label='Linear')
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Speedup')
    plt.title('Speedups')
    plt.savefig(os.path.join(p, base + 'singlepeedupsecond.png'))


    # REAL TIME PHASE BY PHASE for second largest
    plt.figure()
    plt.xticks(ticks)

    qres = df.query(f'size=={sizes[ind]}')
    p1 = qres['p1time'].tolist()
    p2 = qres['p2time'].tolist()
    p3 = qres['p3time'].tolist()
    p4 = qres['p4time'].tolist()
    c = cores[1:]

    plt.plot(c, p1, color='red', label='Phase 1')
    plt.fill_between(c, p1, p3, facecolor='red', alpha=0.3)
    plt.plot(c, p3, color='blue', label='Phase 3')
    plt.fill_between(c, p3, p4, facecolor='blue', alpha=0.3)
    plt.plot(c, p4, color='green', label='Phase 4')
    plt.fill_between(c, p4, facecolor='green', alpha=0.3)
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Real Time')
    plt.title('Phase-by-Phase Analysis (Real Time)' + ' ({})'.format(sizes[ind]))

    plt.savefig(os.path.join(p, base + 'phasetotaltimesecond.png'))

    # PERCENTAGE REAL TIME PHASE BY PHASE for second largest
    plt.figure()
    plt.xticks(ticks)

    qres = df.query(f'size=={sizes[ind]}')
    p1 = qres['perp1'].tolist()
    p2 = qres['perp2'].tolist()
    p3 = qres['perp3'].tolist()
    p4 = qres['perp4'].tolist()
    c = cores[1:]

    plt.plot(c, p1, color='red', label='Phase 1')
    plt.fill_between(c, p1, p3, facecolor='red', alpha=0.3)
    plt.plot(c, p3, color='blue', label='Phase 3')
    plt.fill_between(c, p3, p4, facecolor='blue', alpha=0.3)
    plt.plot(c, p4, color='green', label='Phase 4')
    plt.fill_between(c, p4, facecolor='green', alpha=0.3)
    plt.xlabel('Processors')
    plt.ylabel('Percentage Real Time')
    plt.title('Phase-by-Phase Analysis (Percentage Real Time)' + ' ({})'.format(sizes[ind]))

    plt.savefig(os.path.join(p, base + 'phaseperimesecond.png'))



    ind = -3
    # single speedup graph for third largest
    plt.figure()
    plt.xticks(ticks)
    speeds = rounds(df.query(f'size=={sizes[ind]}')['speedup'].tolist())
    speeds.insert(0, 0)
    plt.plot(cores, speeds, color='red', label=f'{sizes[ind]}')
    plt.plot(cores, cores, color='grey', label='Linear')
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Speedup')
    plt.title('Speedups')
    plt.savefig(os.path.join(p, base + 'singlepeedupthird.png'))


    # REAL TIME PHASE BY PHASE for third largest
    plt.figure()
    plt.xticks(ticks)

    qres = df.query(f'size=={sizes[ind]}')
    p1 = qres['p1time'].tolist()
    p2 = qres['p2time'].tolist()
    p3 = qres['p3time'].tolist()
    p4 = qres['p4time'].tolist()
    c = cores[1:]

    plt.plot(c, p1, color='red', label='Phase 1')
    plt.fill_between(c, p1, p3, facecolor='red', alpha=0.3)
    plt.plot(c, p3, color='blue', label='Phase 3')
    plt.fill_between(c, p3, p4, facecolor='blue', alpha=0.3)
    plt.plot(c, p4, color='green', label='Phase 4')
    plt.fill_between(c, p4, facecolor='green', alpha=0.3)
    plt.legend()
    plt.xlabel('Processors')
    plt.ylabel('Real Time')
    plt.title('Phase-by-Phase Analysis (Real Time)' + ' ({})'.format(sizes[ind]))

    plt.savefig(os.path.join(p, base + 'phasetotaltimethird.png'))

    # PERCENTAGE REAL TIME PHASE BY PHASE for second largest
    plt.figure()
    plt.xticks(ticks)

    qres = df.query(f'size=={sizes[ind]}')
    p1 = qres['perp1'].tolist()
    p2 = qres['perp2'].tolist()
    p3 = qres['perp3'].tolist()
    p4 = qres['perp4'].tolist()
    c = cores[1:]

    plt.plot(c, p1, color='red', label='Phase 1')
    plt.fill_between(c, p1, p3, facecolor='red', alpha=0.3)
    plt.plot(c, p3, color='blue', label='Phase 3')
    plt.fill_between(c, p3, p4, facecolor='blue', alpha=0.3)
    plt.plot(c, p4, color='green', label='Phase 4')
    plt.fill_between(c, p4, facecolor='green', alpha=0.3)
    plt.xlabel('Processors')
    plt.ylabel('Percentage Real Time')
    plt.title('Phase-by-Phase Analysis (Percentage Real Time)' + ' ({})'.format(sizes[ind]))

    plt.savefig(os.path.join(p, base + 'phaseperimethird.png'))


    print('FIGURE CREATION COMPLETE')








