#!/usr/bin/env python
import math

PI = 3.14159265

def fastang(ang):
    cs = math.cos(2*PI/360*ang)
    sn = math.sin(2*PI/360*ang)
    acs = abs(cs)
    asn = abs(sn)
    if (acs>asn):
        x = asn/acs
    else:
        x = 2-acs/asn
        pass
    if (cs<0): x = 4-x
    if (sn<0): x = 8-x
    return x/8.0

def fastang_cs(ang):
    ang = ang*8.0
    a_i = int(math.floor(ang))
    a_f = ang - a_i
    if (a_i&1):a_f=1-a_f
    acs2 = 1/(1+a_f*a_f)
    asn = math.sqrt(1-acs2)
    acs = math.sqrt(acs2)
    if (a_i&1): (asn,acs) = (acs,asn)
    if (a_i&2): (asn,acs) = (acs,-asn)
    if (a_i&4): (asn,acs) = (-asn,-acs)
    return (asn,acs)

def fastcossin2(ang):
    (a_cube_div, ang_mult, asn_1, asn_2) = (7.0, 43.0/45.0, 0.690105137377, 1.01789163403)
    # Goes 0 to +0.3 - hits 1 at 31 degrees
    (a_cube_div, ang_mult, asn_1, asn_2) = (6.0, 45.0/45.0, 0.704652650589, 0.924832229708)
    # Goes -0.007 to +0.13 - hits 1 at 35 degrees
    (a_cube_div, ang_mult, asn_1, asn_2) = (6.4, 45.0/45.0, 0.709699245083, 0.965204985664)
    # Goes -0.012 to +0.1 - hits 1 at 37 degrees
    (a_cube_div, ang_mult, asn_1, asn_2) = (6.5, 45.0/45.0, 0.710863843812, 0.9745217755)
    # Goes -0.013 to +0.1 - hits 1 at 38 degrees
    (a_cube_div, ang_mult, asn_1, asn_2) = (6.6, 45.0/45.0, 0.711993151671, 0.983556238371)
    # Goes 0 to -0.025 - hits 1 at 42 degrees
    (a_cube_div, ang_mult, asn_1, asn_2) = (7.0, 45.0/45.0, 0.716187723719, 1.01711281475)
    (a_cube_div, ang_mult, asn_1, asn_2) = (7.0, 42.0/45.0, 0.676767634921, 1.01591136937)

    test_ang=None
    #test_ang=1
    #test_ang=2

    ang = ang*8.0
    octant = int(math.floor(ang))
    ang45 = 1+(ang-octant)
    if octant&1:
        ang45 = 1+(ang-octant)
        pass
    else:
        ang45 = 2-(ang-octant)
        pass

    if test_ang is not None: ang45=test_ang
    a = ang45 * (PI*2.0/8)*ang_mult
    asn = a - a*a*a / a_cube_div
    if test_ang is not None: print ">>>", ang45, asn
    asn = math.sqrt(0.5) + (1.0-math.sqrt(0.5))*((asn-asn_1)/(asn_2-asn_1))
    if (asn>1.0): asn=1.0
    acs = math.sqrt(1-asn*asn)
    if octant==0: return (asn,acs)
    if octant==1: return (acs,asn)
    if octant==2: return (-acs,asn)
    if octant==3: return (-asn,acs)
    if octant==4: return (-asn,-acs)
    if octant==5: return (-acs,-asn)
    if octant==6: return (acs,-asn)
    if octant==7: return (asn,-acs)
    return (asn,acs)

def fastcossin(ang):
    # Want sin(0)        =sin(0)    = 0
    # Want sin(8*0.05050)=sin(fastang(22.5)) = 0.382
    # Want sin(1)        =sin(fastang(45))   = 0.707
    # Want d/dx(sin(1)) = 1
    # If sin(x) = a + bx + cx^2 + dx^3
    # Then a=0, and:
    #   3d + 2c + b = 1
    #     d + c + b = sqrt(0.5)
    #     b*0.404 + c*0.1632 + d*0.06594 = 0.382
    #     b + 0.404c + 0.1632d = 0.382 / 0.404 = 0.9456
    # 2d      + c      = 1-sqrt(0.5)
    # (3-0.1632)d + (2-0.404)c = 2.8368d + 1.596c = 
    # (3-0.1632)/(2-0.404)d + c = (1-0.382/0.404)/(2-0.404) = 0.03411
    # 1.7774d + c      = 0.0341
    # (2-(3-0.1632)/(2-0.404)) d = 1-sqrt(0.5) - (1-0.382/0.404)/(2-0.404)
    # d = (1-math.sqrt(0.5) - (1-0.382/0.404)/(2-0.404)) / (2-(3-0.1632)/(2-0.404)) 
    # d = 1.162731
    # c = (1-0.382/0.404)/(2-0.404) - (3-0.1632)/(2-0.404) * d
    # c = -2.032569
    # b = 1 - 2*c -3*d
    # b = 1.57694
    # def sin(x): return 1.57694*x-2.032569*x*x+1.162731*x*x*x


    # Want sin(0)        =sin(0)    = 0
    # Want sin(8*0.02204)=sin(fastang(10))   = 0.1736
    # Want sin(8*0.05050)=sin(fastang(22.5)) = 0.382
    # Want sin(1)        =sin(fastang(45))   = 0.707
    # Want d/dx(sin(1)) = 1
    # Then sin(x) = bx + cx^2 + dx^3 + ex^4
    # and d/dx(sin) = b + 2cx + 3dx^2 +4ex^3
    # and (    1      1       1       1       ) (b) = (0.707)
    # and (    1      2       3       4       ) (c) = (1)
    # and (    1      0.404   0.404^2 0.404^3 ) (d) = (0.382/0.404)
    # and (    1      0.176   0.176^2 0.176^3 ) (e) = (0.1736/0.176)
    # OR
    # and (    1      1       1       1       ) (b) = (0.707)
    # and (    1      2       3       4       ) (c) = (1)
    # and (    1      0.404   0.1632  0.06594 ) (d) = (0.382/0.404)
    # and (    1      0.176   0.03098 0.005452 ) (e) = (0.1736/0.176)
    # invert...
    # and (0.707537, -0.144636, -2.1719, 2.609)   (0.707)
    # and (-6.3356, 1.32476, 16.68502, -11.674)   (1)
    # and (14.54865, -3.2156, -26.854, 15.521319) (0.382/0.404)
    # and ( -7.92056, 2.0355, 12.34122,-6.45616) (0.1736/0.176)
    # b = 0.87539
    # c = 1.107111511071107
    # d = -3.01169326534653
    # e = 1.7367159917911783
    # def sin(x): return 0.87539*x + 1.107111511071107*x*x + -3.0116932653465*x*x*x + 1.7367159917911783*x*x*x*x

    test_ang=None
    ang = ang*8.0
    octant = int(math.floor(ang))
    quadrant = octant/2
    ang45 = ang - 2*quadrant

    ang_mult = 1.0

    if test_ang is not None: ang45=test_ang
    a = ang45 * (PI*2.0/8)*ang_mult
    asn = a - a*a*a/6 #+ a*a*a*a*a/120
    acs = 1 - a*a/2 #+ a*a*a*a/24
    #print ang45, asn
    asn += 0.06352-(ang45-0.424474815668)*(ang45-0.424474815668)
    x = ang-octant
    asn = 1.57694*x-2.032569*x*x+1.162731*x*x*x
    asn = 0.87539*x + 1.107111511071107*x*x + -3.0116932653465*x*x*x + 1.7367159917911783*x*x*x*x
    asn = 0.9178*x + 0.7184*x*x + -2.0683*x*x*x + 1.1396*x*x*x*x
    asn = 0.9582953837011352*x + 0.4099015829318393*x*x + -1.5726873481458234*x*x*x + 0.9121053679754603*x*x*x*x
    acs = math.sqrt(1-asn*asn)
    sc = acs*acs+asn*asn
    sc = 1.0/math.sqrt(sc)
    #acs = acs*sc
    #asn = asn*sc

    if octant==0: return (acs,asn)
    if octant==1: return (acs,asn)
    if octant==2: return (-acs,asn)
    if octant==3: return (-asn,acs)
    if octant==4: return (-asn,-acs)
    if octant==5: return (-acs,-asn)
    if octant==6: return (acs,-asn)
    if octant==7: return (asn,-acs)
    return (asn,acs)

for a in range(0,4600,100):
    ang = a/10.0
    ft = fastang(ang)
    s,c = math.sin(2*PI*ang/360), math.cos(2*PI*ang/360)
    fcs = fastang_cs(ft)
    print "%4d : %8.5f   %8.5f , %8.5f     %8.5f , %8.5f    %8.5f,%8.5f"%(ang, ft, s,c, fcs[0], fcs[1], s-fcs[0], c-fcs[1])
    continue

    #print "%4d : %8.5f %8.5f"%(ang, ft, ft*360-ang)
    acs,asn = fastcossin(ft)
    print "%4d : %8.5f %8.5f %8.5f    %8.5f %8.5f    %8.5f %8.5f   %8.5f %8.5f "%(ang,
                                                                                  ft,
                                                                                  asn, acs,
                                                                                  s,c,
                                                                                  asn-s, acs-c,
                                                                                  100*(asn-s)/(s+0.0001), 100*(acs-c)/(c+0.0001),
                                                                                  )

if False:
    a = fastang(ang)*PI*2.0
    asn = a - a*a*a/4
    a = (0.25-fastang(ang))*PI*2.0
    acs = a - a*a*a/4
    sc = asn*asn + acs*acs
    # (1-x)^-1/2 => 1 + 1/2x 
    #print 1/math.sqrt(sc), (2-sc), 1+0.5*(1-sc)
    sc = 1+0.55*(1-sc)
    sc = 1.0/math.sqrt(asn*asn + acs*acs)
    asn = asn*sc
    acs = acs*sc
    #if (ang>45): asn=math.sqrt(1-acs*acs)
    print "%4d : %8.5f %8.5f %8.5f %8.5f %8.5f %8.5f"%(ang, (asn-math.sin(2*PI*ang/360))/(asn+0.00001)*100.0,asn-math.sin(2*PI*ang/360), acs-math.cos(2*PI*ang/360), asn, acs, asn*asn+acs*acs)


