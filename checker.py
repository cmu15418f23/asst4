#!/usr/bin/env python3

import sys
import os
import re

# Usage: ./checker.py v1/v2 0/1 -ghc/-psc


if len(sys.argv) != 4:
    print("Usage: ./checker.py v1/v2 0/1 -ghc/-psc")
    exit(-1)

version = sys.argv[1]
mode = sys.argv[2]
machine = sys.argv[3]

valid = True
valid = valid and (version == 'v1' or version == 'v2')
valid = valid and (mode == '0' or mode == '1')
valid = valid and (machine == '-ghc' or machine == '-psc')

if not valid:
    print("Usage: ./checker.py v1/v2 0/1 -ghc/-psc")
    exit(-1)

load_balance = '-lb' if sys.argv[2] == '1' else ''
prog = 'nbody-release-' + version

# set number of workers
if sys.argv[3] == '-ghc':
    workers = [4, 8] if version == 'v1' else [1, 4]
elif sys.argc[3] == '-psc':
    workers = [16, 128] if version == 'v1' else [16,121]


scenes = (
    ('random-50000', 50000, 500.0, 5),
    ('corner-50000', 50000, 500.0, 5),
    ('repeat-10000', 10000, 100.0, 50),
    ('sparse-50000', 50000, 5.0, 50),
    ('sparse-200000', 200000, 20.0, 50),
)
ref_perfs = (
    (0.588422, 0.161911),
    (1.148824, 0.241397),
    (0.183667, 0.136549),
    (0.354411, 0.784285),
    (1.582532, 3.821544),
) if version == 'v1' else (
    (0.819216, 0.176659),
    (3.568420, 0.754285),
    (0.297109, 0.084589),
    (0.335568, 0.100940),
    (1.532440, 0.417115),
)

perfs = [[None] * len(workers) for _ in range(len(scenes))]

scene_names = ('random-50000', 'corner-50000', 'repeat-10000',
               'sparse-50000', 'sparse-200000')
particle_nums = (50000, 50000, 10000, 50000, 200000)
space_sizes = (500.0, 500.0, 100.0, 5.0, 20.0)
iterations = (5, 5, 50, 50, 50)


def compare(actual, ref):
    threshold = 1.0 if 'repeat' in actual else 0.1
    actual = open(actual).readlines()
    ref = open(ref).readlines()
    assert len(actual) == len(ref), \
        f'ERROR -- number of particles is {len(actual)}, should be {len(ref)}'
    for i, (l1, l2) in enumerate(zip(actual, ref)):
        l1 = [float(x) for x in l1.split()]
        l2 = [float(x) for x in l2.split()]
        assert len(l2) == 5 and len(l1) == len(l2),\
            f'ERROR -- invalid format at line {i}, should contain {len(l2)} floats'
        assert all(abs(x - y) < threshold for x, y in zip(l1, l2)),\
            f'ERROR -- incorrect result at line {i}'


def compute_score(actual, ref):
    # actual <= 1.2 * ref: full score
    # actual >= 3.0 * ref: zero score
    # otherwise: linear in (actual / ref)
    return min(max((3.0 - actual / ref) / 1.8, 0.0), 1.0)


os.system('mkdir -p logs')
os.system('rm -rf logs/*')
for i, (scene_name, particle_num, space_size, iteration) in enumerate(scenes):
    for j, worker in enumerate(workers):
        print(f'--- running {scene_name} on {worker} workers ---')
        init_file = f'src/benchmark-files/{scene_name}-init.txt'
        output_file = f'logs/{scene_name}.txt'
        log_file = f'logs/{scene_name}.log'
        cmd = f'mpirun -n {worker} {prog} {load_balance} -n {particle_num} -i {iteration} -in {init_file} -s {space_size} -o {output_file} > {log_file}'
        ret = os.system(cmd)
        assert ret == 0, 'ERROR -- nbody exited with errors'
        compare(output_file, f'src/benchmark-files/{scene_name}-ref.txt')
        t = float(re.findall(
            r'total simulation time: (.*?)s', open(log_file).read())[0])
        print(f'total simulation time: {t:.6f}s')
        perfs[i][j] = t

print('\n-- Performance Table ---')
header = '|'.join(f' {x:<15} ' for x in ['Scene Name'] + workers)
print(header)
print('-' * len(header))
for scene, perf in zip(scenes, perfs):
    print('|'.join(f' {x:<15} ' for x in [scene[0]] + perf))

score = 0.0
print('\n-- Score Table ---')
print(header)
print('-' * len(header))
for i, (scene, perf) in enumerate(zip(scenes, perfs)):
    scores = [compute_score(perf[j], ref)
              for j, ref in enumerate(ref_perfs[i])]
    score += sum(scores)
    print('|'.join(f' {x:<15} ' for x in [scene[0]] + scores))
print(f'total score: {score}/{len(workers) * len(scenes)}')
