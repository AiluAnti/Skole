f = open('testdb.dbcmd', 'w')

f.write('create table pupils (ID int);\n')

for x in range (500):
    f.write('insert into pupils values (' + str(x) +');\n')

f.write("quit\n")
f.close()
