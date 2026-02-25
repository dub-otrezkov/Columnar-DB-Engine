with open("data.csv", "w") as f:
    for i in range(10000000):
        f.write(f"john,10,frusciante,20\n")
