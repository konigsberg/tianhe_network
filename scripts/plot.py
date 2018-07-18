import matplotlib.pyplot as plt

def main():
    fig = plt.figure()
    ax1 = plt.subplot(1,1,1)
    ax1.set_xlabel('injection rate')
    ax1.set_ylabel('latency')
    ax1.set_size_inches(8,6)

    x_axis = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
    latency = [1364.4736888226155,1401.166265698821,1441.509795906724,
    1475.7285723846496,1506.1776294486017,1525.5139614903321,
    1540.799844218341, 1552.968588931679,1569.1040303361046,1589.6437687827101]

    plt.plot(x_axis, latency)
    plt.legend(loc='best')
    plt.savefig('latency', dpi=400, bbox_inches='tight')


if __name__ == '__main__':
    main()
