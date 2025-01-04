
def inv(a,m) :
    if (a <= 1):
        return a
    return m - ((m//a) * inv(m%a,m) % m)
# print(inv(31,4294967291))
for i in range(2,65537):
    if 4294967291 % i == 0:
        print(i)

