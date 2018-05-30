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
        elif 'total_flit_received' in lines:
            flit_recv_count += int(words[-1])
        elif 'avg_latency' in lines:
            avg_latency_total += double(words[-1])
            avg_latency_count += 1

    print('total_flit_gen = ' + str(total_flit_gen))
    print('total_flit_sent = ' + str(total_flit_sent))
    print('total_flit_recv = ' + str(total_flit_recv))
    print('throughput = ' + str(total_flit_recv / total_flit_gen))
    print('average latency = ' + str(avg_latency_total / avg_latency_count))


if __name__ == '__main__':
    main()
