10 OPEN #1, "_testio", "w"
20 PRINT #1, "HELLO"
30 CLOSE #1
40 OPEN #1, "_testio", "r"
50 INPUT #1, A$
60 CLOSE #1
70 PRINT A$
80 END
