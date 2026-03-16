10 S = 16
20 A = 24    : REM 1.5 in Q4
30 B = 36    : REM 2.25 in Q4
40 M = (A * B) / S
50 D = (A * S) / B
60 PRINT "A="; A
70 PRINT "B="; B
80 PRINT "MUL="; M
90 PRINT "DIV="; D
100 PRINT "MUL_INT="; M / S
110 PRINT "DIV_INT="; D / S
120 A = -24
130 M = (A * B) / S
140 PRINT "NEG="; M
150 END
