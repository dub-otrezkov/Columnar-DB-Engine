import csv
import math

def parse_clickhouse_res():
    with open('clickhouse-parquet-tmp.csv', newline='') as tmp:
        ls = csv.reader(tmp, delimiter=',')
        buf = []
        with open('clickhouse-parquet-res.csv', 'w', newline='') as res:
            wrt = csv.writer(res, delimiter=',')
            for row in ls:
                # print(row)
                # wrt.writerow(list(row))
                buf.append(row[2] if row[2][0] != 'C' else "null")
                if len(buf) == 3:
                    wrt.writerow(buf)
                    buf = []


f1 = open('duckdb-parquet-res.csv', newline='')
f2 = open('clickhouse-parquet-res.csv', newline='')
f3 = open('ClickBench/clickhouse-parquet/queries.sql', newline='')
f4 = open('state_of_art.csv', newline='')

rd1 = csv.reader(f1, delimiter=',')
rd2 = csv.reader(f2, delimiter=',')
rd4 = csv.reader(f4, delimiter=',')

tgs = ["GROUP", "WHERE", "ORDER", "LIMIT"]

d = dict()

def ch(l):
    if l[0] == "null" and l[1] == "null":
        return "both failed"
    if l[0] == "null":
        return f"clickhouse better"
    if l[1] == "null":
        return f"duckdb better"
    l0 = float(l[0])
    l1 = float(l[1])

    return f"{'duckdb' if l0 < l1 else 'clickhouse'} better by {round((max(l0, l1) - min(l0, l1)) / max(l0, l1), 4) * 100}%"


def chi(l):
    if l[0] == "null" and l[1] == "null":
        return [-1, -1]
    if l[0] == "null":
        return [-1, -1]
    if l[1] == "null":
        return [-1, -1]
    l0 = float(l[0])
    l1 = float(l[1])

    return [l0 > l1, round((max(l0, l1) - min(l0, l1)) / max(l0, l1), 4) * 100]


bad1 = 0
bad2 = 0
bad3 = 0
bad4 = 0
l1 = []
l2 = []
l3 = []
for r1, r2, r3, r4 in zip(rd1, rd2, f3, rd4):
    # s = ""
    # for i in tgs:
    #     if i in r3:
    #         s = s + i + " "

    # el = [r1[0], r2[0], r3]
    # print(r1, r2, el)
    # try:
    #     d[s].append(el)
    # except:
    #     d[s] = [el]

    t1 = [r1[0], r2[0]]
    t2 = r4[0:2]
    t3 = r4[2:4]
    t4 = r4[4:6]

    # print(f"{ch(t1)} | {ch(t2)} | {ch(t3)} | {ch(t4)}")

    x1 = chi(t1)
    x2 = chi(t2)
    x3 = chi(t3)
    x4 = chi(t4)

    a1 = ""
    a2 = ""
    a3 = ""

    if x1[0] == -1:
        continue
    if x1[0] != x2[0]:
        print("opposite", end=" | ")
        bad1 += 1
    else:
        print("same, their / mine = ", x2[1] / x1[1], end=" | ")
        l1.append(x2[1] / x1[1])
    if x1[0] != x3[0]:
        print("opposite", end=" | ")
        bad2 += 1
    else:
        print("same, their / mine = ", x3[1] / x1[1], end=" | ")
        l2.append(x3[1] / x1[1])
    if x1[0] != x4[0]:
        print("opposite")
        bad3 += 1
    else:
        print("same, their / mine = ", x4[1] / x1[1])
        l3.append(x4[1] / x1[1])

l1.sort()
l2.sort()
l3.sort()
print(f"{bad1} | {bad2} | {bad3}")
print(f"{l1[len(l1) // 2]} | {l2[len(l1) // 2]} | {l3[len(l1) // 2]}")

def variance(data):
    n = len(data)
    mean = sum(data) / n
    return math.sqrt(sum((x - mean) ** 2 for x in data) / n)

print(l1)
print(variance(l1))



for k in d.keys():
    print(k)

    for i in d[k]:
        print(f"{i[:2]} {ch(i)} {i[2]}")

f1.close()
f2.close()
f3.close()
