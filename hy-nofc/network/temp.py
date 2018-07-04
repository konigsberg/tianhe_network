def main():
    with open('temp.txt', 'w') as f:
        for ci in range(48):
            for cp in range(36):
                ni = cp / 3 + ci * 12
                np = 72 + cp % 3
                line = '    nrm[%d].port_%d <--> C0 <--> cabinet_counter[%d].port_%d;\n' % (ni, np, ci, cp)
                f.write(line)

        f.write('\n\n')

        for ci in range(36):
            for cp in range(48):
                ri = cp / 4 + ci * 12
                rp = 48 + cp % 4
                line = '    root[%d].port_%d <--> C0 <--> root_counter[%d].port_%d;\n' % (ri, rp, ci, cp)
                f.write(line)


if __name__ == '__main__':
    main()
