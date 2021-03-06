#!/usr/bin/env python
import math

class c_dft(object):
    bitrev = {}
    def get_bitrev(n):
        b = [0]*n
        for i in range(n):
            j = 0
            while (1<<j) < n:
                if ((i>>j)&1):
                    b[i] += n>>(j+1)
                    pass
                j += 1
                pass
            pass
        return b
    for i in [1,2,4,8,16,32,64]:
        bitrev[i] = get_bitrev(i)
        pass
    def __init__(self):
        pass
    def mult(self, c0, c1):
        return (c0[0]*c1[0] - c0[1]*c1[1],
                c0[0]*c1[1] + c0[1]*c1[0])
    def add(self, c0, c1, scale):
        return (c0[0] + scale*c1[0],
                c0[1] + scale*c1[1])
    def eiknx(self, k, n=32):
        w = (-2*math.pi*k)/n
        return (math.cos(w), math.sin(w))
    def butterfly(self, c0, c1, k, n=32):
        e = self.eiknx(k, n)
        w = self.mult(c1, e)
        return (self.add(c0,w,1), self.add(c0,w,-1))
    def fft(self, data, scale=1.0, n=32):
        n = len(data)
        if type(data[0])!=tuple:
            data = [(d,0.0) for d in data]
            pass
        data.extend( [(0.0,0.0)]*n )
        data = data[:n]
        print data
        fd = []
        fd.append(data)
        j = n/2
        while (j>=1):
            fd.append([0]*n)
            for k in range(n/j/2):
                bk = self.bitrev[n/j/2][k]
                i0 = 2*j*k+0
                i1 = 2*j*k+j
                for i in range(j):
                    # First step should be e0/2, e1/2
                    # Second step should be (e0/4, e2/4), then (e1/4, e3/4) - i.e. e(k/4), -e(k/4)
                    (fd[-1][i0+i],fd[-1][i1+i]) = self.butterfly(fd[-2][i0+i],fd[-2][i1+i],bk*j,n)#i*k,32)
                    pass
                pass
            j /= 2
            pass
        for i in range(len(fd)):
            s = ""
            for j in fd[i]:
                s += (" %4.2f,%4.2f"%j)
            print i, s
            pass
        c = [0]*n
        for i in range(n):
            c[self.bitrev[n][i]] = (fd[-1][i][0]*scale, fd[-1][i][1]*scale)
            pass
        return c
    def fft2(self, data, scale=1.0, n=32):
        n = len(data)
        if type(data[0])!=tuple:
            data = [(d,0.0) for d in data]
            pass
        data.extend( [(0.0,0.0)]*n )
        data = data[:n]
        print data

        brd = data[:]
        for i in range(n):
            brd[self.bitrev[n][i]] = data[i]
            pass
        fd = []
        fd.append(brd)
        j = 1
        while (j<32) and (j<n):
            fd.append([0]*n)
            for k in range(j):
                for i in range(n/j/2):
                    i0 = i*j*2+k
                    (fd[-1][i0],fd[-1][i0+j]) = self.butterfly(fd[-2][i0],fd[-2][i0+j],k,j*2)
                    pass
                pass
            j *= 2
            pass
        for i in range(len(fd)):
            s = ""
            for j in fd[i]:
                s += (" %4.2f,%4.2f"%j)
            print i, s
            pass
        c = [0]*n
        for i in range(n):
            c[i] = (fd[-1][i][0]*scale, fd[-1][i][1]*scale)
            pass
        return c
    def dft(self, data, scale=1.0, n=32):
        if type(data[0])!=tuple:
            data = [(d,0.0) for d in data]
            pass
        data.extend( [(0.0,0.0)]*n )
        data = data[:n]
        c = [(0,0)]*n
        for i in range(n):
            arg = -2*math.pi*i/n;
            c[i] = (0,0)
            for k in range(n):
                cosarg = math.cos(k * arg)
                sinarg = math.sin(k * arg)
                c[i] = (c[i][0] + (data[k][0]*cosarg - data[k][1]*sinarg),
                        c[i][1] + (data[k][0]*sinarg + data[k][1]*cosarg))
                pass
            pass
        for i in range(n):
            c[i] = (c[i][0]*scale, c[i][1]*scale)
        s = ""
        for j in c:
            s += (" %4.2f,%4.2f"%j)
            pass
        print s
        return c
    def dft_power(self, data):
        p = []
        max=0.1
        for d in data:
            p.append(d[0]*d[0]+d[1]*d[1])
            if p[-1]>max: max=p[-1]
            pass
        for i in range(len(p)):
            p[i] /= max
            pass
        return p
    pass

data = [0x54, 0x2f, 0x23, 0x1e, 0x1e, 0x1f, 0x21, 0x2c,
        0x4b, 0x66, 0x72, 0x69, 0x53, 0x6b, 0x77, 0x33,
        0x23, 0x2e, 0x34, 0x3d, 0x40, 0x45, 0x71, 0xad,
        0xa1, 0x8c, 0x6e, 0x69, 0x79, 0x75, 0x87, 0x82]
a = c_dft()
if True:
    fft = a.fft(data)
    p = a.dft_power(fft)
    print p
    s=""
    for d in p:
        s += "%02x "%(d*255)
        pass
    print s
    #ifft = a.fft(fft)

data = [100] *32
print "\n\n","*"*80,"FFT"
fft = a.fft(data,1.0/32)
print "\n\n","*"*80,"IFFT"
ifft = a.fft(fft,1.0)

data = [math.sin(2*math.pi/8*i) for i in range(8)]
print "\n\n","*"*80,"FFT"
fft = a.fft2(data,1.0/8,8)
print "\n\n","*"*80,"DFT"
ifft = a.dft(data,1.0/8,8)

data = [0x54, 0x2f, 0x23, 0x1e, 0x1e, 0x1f, 0x21, 0x2c,
        0x4b, 0x66, 0x72, 0x69, 0x53, 0x6b, 0x77, 0x33,
        0x23, 0x2e, 0x34, 0x3d, 0x40, 0x45, 0x71, 0xad,
        0xa1, 0x8c, 0x6e, 0x69, 0x79, 0x75, 0x87, 0x82]
data = [0x54, 0x30, 0x24, 0x1f, 0x1d, 0x1f, 0x21, 0x2c,
        0x4b, 0x66, 0x72, 0x69, 0x54, 0x6b, 0x77, 0x34,
        0x2c, 0x2e, 0x34, 0x3d, 0x40, 0x45, 0x71, 0xad,
        0xa1, 0x8c, 0x6e, 0x69, 0x79, 0x75, 0x87, 0x82]
# This is from best Harris (883,867) for UNsmoothed IMG_1854
data = [0x6c, 0x3d, 0x3a, 0x45, 0x27, 0x18, 0x12, 0x11,
        0x10, 0x10, 0x0f, 0x12, 0x12, 0x11, 0x12, 0x11,
        0x11, 0x10, 0x10, 0x11, 0x11, 0x12, 0x16, 0x19,
        0x1a, 0x19, 0x19, 0x19, 0x1a, 0x1a, 0x1c, 0x28]
# This is from best Harris for smoothed IMG_1854
data = [0x47, 0x31, 0x27, 0x23, 0x25, 0x29, 0x28, 0x2e,
        0x40, 0x59, 0x6c, 0x6d, 0x5f, 0x63, 0x70, 0x59,
        0x3b, 0x34, 0x34, 0x38, 0x3d, 0x48, 0x6a, 0x87,
        0x87, 0x81, 0x7a, 0x73, 0x72, 0x78, 0x7a, 0x67]
# This is from best Harris (883,867) for unsmoothed IMG_1854???
data = [0x43, 0x2a, 0x20, 0x1e, 0x1f, 0x1f, 0x20, 0x27,
        0x42, 0x5e, 0x69, 0x74, 0x59, 0x5e, 0x7e, 0x5b,
        0x34, 0x32, 0x33, 0x36, 0x3b, 0x42, 0x68, 0x8a,
        0x77, 0x71, 0x6d, 0x65, 0x70, 0x80, 0x7d, 0x77]

for i in range(len(data)):
    data[i] = data[i]/255.0
    pass

fft_0 = a.fft2(data,1.0/32.0)
data2 = data[30:]+data[:30]
for i in range(32):
    data2[i] += 0.01*math.sin(4*math.pi*i/32)
    pass
fft_1 = a.fft2(data2,1.0/32.0)

print "\n\n"
print fft_0
print fft_1

def psd(d):
    r = []
    for i in d:
        r.append(i[0]*i[0]+i[1]*i[1])
        pass
    return r
def normalize_fft(d):
    max = 0.01
    p = psd(d)
    p[0] = 0
    for x in p:
        x=math.sqrt(x)
        if x>max: max=x
        pass
    max = 1.0
    r = []
    for i in d:
        r.append((i[0]/max, i[1]/max))
        pass
    return r
print

fft_0 = normalize_fft(fft_0)
fft_1 = normalize_fft(fft_1)
print psd(fft_0)
print psd(fft_1)

def abs(x):
    if x<0:return -x
    return x
s = ""
for d in fft_0:
    s += "%02x "%(abs(d[0])*255)
    pass
print s
s = ""
for d in fft_0:
    s += "%02x "%(abs(d[1])*255)
    pass
print s    
s = ""
for d in fft_0:
    s += "%02x "%((d[0]*d[0]+d[1]*d[1])*255)
    pass
print s    
print
# This is from best Harris (883,867) for unsmoothed IMG_1854???
#data = [0x43, 0x2a, 0x20, 0x1e, 0x1f, 0x1f, 0x20, 0x27,
#        0x42, 0x5e, 0x69, 0x74, 0x59, 0x5e, 0x7e, 0x5b,
#        0x34, 0x32, 0x33, 0x36, 0x3b, 0x42, 0x68, 0x8a,
#        0x77, 0x71, 0x6d, 0x65, 0x70, 0x80, 0x7d, 0x77]
# Get 0x31 0x36 0x35 0x3d for normalized power for time_domain[4..7] for unsmoothed IMG_1854
# Get 0x28 0x29 0x29 0x33 for normalized data for time_domain[4..7] for unsmoothed IMG_1854
# Get 0x0d 0x0d 0x0d 0x14 for normalized power for time_domain[4..7] for unsmoothed IMG_1854
# Get 0x8f 0x02 0x15 0x02 for normalized power for fd_1[4..7] for unsmoothed IMG_1854 (should be fft2(td[1,9,17,25]))
# Get (0xbf,0), (-0x12,0xf), (-0x4a,0), (-0x12,-0x0f) for normalized for fd_1[4..7] for unsmoothed IMG_1854 (should be fft2(td[4,12,20,28]))
# Want (0xbf,0), (-0x12,0xf), (-0x49,0), (-0x12,-0x0f) for normalized for fd_1[4..7] for unsmoothed IMG_1854 (should be fft2(td[4,12,20,28]))
# fft2(0x1f, 0x59, 0x3b, 0x70)[0] = (0x1f+0x59+0x3b+0x70) = 291 / 1.5235 = 0xbf
# fft2(0x1f, 0x59, 0x3b, 0x70)[1] = (0x1f-0x3b,-0x59+0x70) = -28,-23 / 1.5235 = -0x12,-0xf
# fft2(0x1f, 0x59, 0x3b, 0x70)[2] = (0x1f-0x59+0x3b-0x70) = -111 / 1.5235 = -0x49
# Get (0x5,0), (0x7,0x6), (-0x18,-0x29), (0x4,-0x21) for normalized for fd_2[4..7] for unsmoothed IMG_1854
# Want (0x4,0), (0x6,0x6), (-0x18,-0x29),(0x4,-0x21) for normalized for fd_2[4..7] for unsmoothed IMG_1854
# Get (0x3,0x12), (-0x7,0x1), (-0xb,0xc), (0xb,-0x2) for normalized for fd_3[4..7] for unsmoothed IMG_1854
# Want (0x2,0x11), (-0x7,0x0), (-0x0a,0x0c), (0xa,-0x2) for normalized for fd_3[4..7] for unsmoothed IMG_1854
# Get (0x1a,0x46), (-0x30,0x26), (-0x21,0x2b), (0x27,-0xf) for normalized for fd_4[4..7] for unsmoothed IMG_1854
# Want (0x19,0x46), (-0x2f,0x26), (-0x21,0x2a), (0x26,-0xf) for normalized for fd_4[4..7] for unsmoothed IMG_1854
die
print

def cmult(c0, c1):
    return (c0[0]*c1[0] - c0[1]*c1[1],
            c0[0]*c1[1] + c0[1]*c1[0])

Xa1 = fft_0[1]
Xb1 = fft_1[1]
Xa2 = fft_0[2]
Xb2 = fft_1[2]
Xa3 = fft_0[3]
Xb3 = fft_1[3]
x0 = cmult(cmult(Xa1,Xa1),Xb2)
x1 = cmult(cmult(Xb1,Xb1),Xa2)
print x0[0]*x0[0]+x0[1]*x0[1]
print x1[0]*x1[0]+x1[1]*x1[1]

x0 = cmult(cmult(Xa1,cmult(Xa1,Xa1)),Xb3)
x1 = cmult(cmult(Xb1,cmult(Xb1,Xb1)),Xa3)
print x0[0]*x0[0]+x0[1]*x0[1]
print x1[0]*x1[0]+x1[1]*x1[1]
