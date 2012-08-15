from pygas import *

assert THREADS is 4

row = MYTHREAD / 2
col = MYTHREAD % 2

rowteam = TEAM_WORLD.split(row, col)
colteam = TEAM_WORLD.split(col, row)

r = rowteam.broadcast(MYTHREAD, from_rank=0)
c = colteam.broadcast(MYTHREAD, from_rank=0)

#print "Row bcast on t %d" % MYTHREAD, r
#print "Col bcast on t %d" % MYTHREAD, c

assert r == row*2
assert c == col

TEAM_WORLD.barrier()

if MYTHREAD == 0:
    print "Success."
