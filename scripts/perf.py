# Please add the program directory to PATH
def main():
    with open("General-#0.sca") as sca:
        sca_lines = sca.readlines()
    flit_gen_count = 0
    flit_recv_count = 0
    flit_sent_count = 0
    avg_latency_total = 0
    avg_latency_count = 0

    for line in sca_lines:
        words = line.split()
        if 'total_flit_gen' in line:
            flit_gen_count += int(words[-1])
        elif 'total_flit_sent' in line:
            flit_sent_count += int(words[-1])
        elif 'total_flit_received' in line:
            flit_recv_count += int(words[-1])
        elif 'avg_latency' in line:
            lat = float(words[-1])
            if lat > 0:
                avg_latency_total += lat
                avg_latency_count += 1

    print('total_flit_gen = ' + str(flit_gen_count))
    print('total_flit_sent = ' + str(flit_sent_count))
    print('total_flit_recv = ' + str(flit_recv_count))
    print('throughput = ' + str(flit_recv_count / flit_gen_count * 100) + ' %')
    print('average latency = ' + str(avg_latency_total / avg_latency_count)
                               + ' ns')


if __name__ == '__main__':
    main()
