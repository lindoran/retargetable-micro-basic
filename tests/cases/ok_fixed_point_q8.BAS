10 S = 256
20 A = 384   : REM 1.5 in Q8
30 B = 128   : REM 0.5 in Q8
40 M = (A * B) / S
50 D = (A * S) / B
60 PRINT "A="; A
70 PRINT "B="; B
80 PRINT "MUL="; M
90 PRINT "DIV="; D
100 PRINT "MUL_INT="; M / S
110 PRINT "DIV_INT="; D / S
120 A = -384
130 M = (A * B) / S
140 PRINT "NEG="; M
150 END
