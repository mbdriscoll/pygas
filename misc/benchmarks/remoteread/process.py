f0 = open('test-0.err', 'r')
f1 = open('test-1.err', 'r')

lines = zip(f0.readlines(), f1.readlines())
lines = map(lambda x: (x[0].rstrip() + x[1].rstrip()), lines)

for line in lines:
    data = {}
    tokens = line.split()
    tokenc = len(tokens)/2
    for n in range(tokenc):
        if tokens[n*2] == '0':
            data[tokens[n*2]] = float(tokens[n*2+1])
        else:
            data[tokens[n*2]] = float(tokens[n*2+1])*1e6

    Msg_size = data['0']
    Total = (data['J'] - data['A'])
    Send_Serialize = (data['B'] - data['A'])
    Send_Transmit = (data['C'] - data['B'])
    Remote_Async = (data['D'] - data['C'])
    Remote_ApplyD = (data['E'] - data['D'])
    Recv_Transmit = (data['G'] - data['F'])
    Local_Async = (data['H'] - data['G'])
    Recv_Deserialize = (data['J'] - data['I'])
    Unaccounted = (Total - (Send_Serialize+Send_Transmit+Remote_Async+Remote_ApplyD+Recv_Transmit+Local_Async+Recv_Deserialize))

    print "--------------------------"
    print "Msg size:\t\t%d" % Msg_size
    print "Total:\t\t\t%f" % Total
    print "Send Serialize:\t\t%f" % Send_Serialize
    print "Send Transmit:\t\t%f" % Send_Transmit
    print "Remote Async:\t\t%f" % Remote_Async
    print "Remote ApplyD:\t\t%f" % Remote_ApplyD
    print "Recv Transmit:\t\t%f" % Recv_Transmit
    print "Local Async:\t\t%f" % Local_Async
    print "Recv Deserialize:\t%f" % Recv_Deserialize
    print "Unaccounted:\t\t%d (%f%%)" % (Unaccounted, (100.*Unaccounted/Total))
