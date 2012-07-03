f0 = open('test-0.err', 'r')
f1 = open('test-1.err', 'r')

lines = zip(f0.readlines(), f1.readlines())
lines = map(lambda x: (x[0].rstrip() + x[1].rstrip()), lines)

keys = ['Total', 'Send_Serialize', 'Send_Transmit', 'Remote_Async',
        'Remote_ApplyD', 'Recv_Transmit', 'Local_Async', 'Recv_Deserialize']

data = {}


class MsgData(object):
    def __init__(self, msg_size):
        self.msg_size = msg_size
        self.points = 0

        self.total = 0.
        self.local_pickle = 0.
        self.send_xfer = 0.
        self.remote_async = 0.
        self.remote_apply = 0.
        self.recv_xfer = 0.
        self.local_async = 0.
        self.local_unpickle = 0.

    def __add__(self, markers):
        self.points += 1
        assert markers['0'] == self.msg_size

        self.total += markers['J'] - markers['A']
        self.local_pickle += markers['B'] - markers['A']
        self.send_xfer += markers['C'] - markers['B']
        self.remote_async += markers['D'] - markers['C']
        self.remote_apply += markers['E'] - markers['D']
        self.recv_xfer += markers['G'] - markers['E']
        self.local_async += markers['H'] - markers['G']
        self.local_unpickle += markers['J'] - markers['H']
        

    def __str__(self):
        val = ""
        val += "Msg Size:\t\t%d\n" % self.msg_size
        val += "Average of n trials:\t%d\n" % self.points
        val += "Total:\t\t\t%f\n" % (self.total/self.points)
        val += "Local Serialize:\t%f\n" % (self.local_pickle/self.points)
        val += "Send Xfer:\t\t%f\n" % (self.send_xfer/self.points)
        val += "Remote Async:\t\t%f\n" % (self.remote_async/self.points)
        val += "Remote Apply:\t\t%f\n" % (self.remote_apply/self.points)
        val += "Recv Xfer:\t\t%f\n" % (self.recv_xfer/self.points)
        val += "Local Async:\t\t%f\n" % (self.local_async/self.points)
        val += "Local Unserialize:\t%f\n" % (self.local_unpickle/self.points)
        val += '-'*40 + "\n"
        return val

    def gplot(self):
        total = 0.
        data = [self.local_pickle, self.send_xfer, self.remote_async, self.remote_apply, self.recv_xfer, self.local_async, self.local_unpickle]
        print self.msg_size,
        for val in data:
            total += val
            print (" %f" % (total/self.points)),

for line in lines:
    tokens = line.split()
    tokenc = len(tokens)/2
    markers = {}
    for n in range(tokenc):
        if tokens[n*2] == '0':
            markers[tokens[n*2]] = float(tokens[n*2+1])
        else:
            markers[tokens[n*2]] = float(tokens[n*2+1])*1e6
    try:
        msg_size = markers['0']
        msg_data = data[msg_size]
    except:
        msg_data = data[msg_size] = MsgData(msg_size)

    msg_data += markers
    
#for datum in data:
#    print data[datum]

for datum in data:
    data[datum].gplot()
    print
