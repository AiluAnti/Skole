import random, string


class GenerateTables():
    """docstring for ."""
    def __init__(self):
        self.f = open('testdb.dbcmd', 'w')
        self.table1Name = "Men"
        self.table2Name = "Women"
        self.f.write('create table '+ self.table1Name +' ( Name str(20), ID int );\n')
        self.f.write('create table '+ self.table2Name +' ( Name str(20), ID int );\n')



        for x in range (100):
            self.f.write('insert into ' + self.table1Name + ' values ( ' + "'" + self.randWord(15) + "'" +  ', ' + str(x) +' );\n')
            self.f.write('insert into ' + self.table2Name + ' values ( ' + "'" + self.randWord(15) + "'" +  ', ' + str(x) +' );\n')


        #self.f.write("select * from " + self.table1Name + " natural join " + self.table2Name + ";")
        self.f.write("quit\n")
        self.f.close()

    def randWord(self, length):
        return ''.join(random.choice(string.lowercase) for i in range(length))

if __name__ == '__main__':
    GenerateTables()
