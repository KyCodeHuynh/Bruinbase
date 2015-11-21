SRC = main.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 
# TODO: Comment out before submission, just in case TA uses this Makefile
NODE_TEST_SRC = node-test.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc
TREE_TEST_SRC = tree-test.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc  
HDR = Bruinbase.h PageFile.h SqlEngine.h BTreeIndex.h BTreeNode.h RecordFile.h SqlParser.tab.h

bruinbase: $(SRC) $(HDR)
	g++ -ggdb -o $@ $(SRC)

lex.sql.c: SqlParser.l
	flex -Psql $<

SqlParser.tab.c: SqlParser.y
	bison -d -psql $<

# TODO: Comment out before submission, just in case TA uses this Makefile
test: bruinbase $(NODE_TEST_SRC) $(SRC)
	# Test BTreeNode
	g++ -o node-test $(NODE_TEST_SRC) 
	chmod 0755 node-test
	./node-test
	# Test BTreeIndex
	g++ -o tree-test $(TREE_TEST_SRC)
	chmod 0755 tree-test
	./tree-test
	# Test everything via SqlEngine
	bash queries.sh

clean:
	rm -f bruinbase bruinbase.exe node-test *.o *~ lex.sql.c SqlParser.tab.c SqlParser.tab.h newmovie.tbl indexmovie.tbl
