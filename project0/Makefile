.SILENT:
default:
	gcc -g -std=c99 -Wall -Wextra lab0.c -o lab0 
check: default code2test code3test code4test code1test code0test finaltest
	if [[ ! -s trash.txt ]]; then \
		echo "passed all test"; \
	else \
		cat trash.txt; \
		rm -f *.txt; \
	fi
clean: 
	rm -f lab0 *.txt *.tar.gz  
dist: 
	tar -czvf lab0-804796478.tar.gz README Makefile lab0.c backtrace.png breakpoint.png
code2test:
	echo "double bracket syntax is not portable, use double dollar sign, and what is that suspicious bug in EMACS!" > bail.txt; \
	chmod u-r bail.txt; \
	./lab0 --input=bail.txt --output=temp.txt 1&>/dev/null; \
	if [[ $$? -ne 2 ]]; then \
		echo "fail to open a file and did not exit 2">>trash.txt; \
	fi; \
	rm -f bail.txt temp.txt
code3test:
	echo "you need that backstick in makefile to write script in multiple lines" > backstick.txt; \
	echo "and also that semi colon to make command independent" > colon.txt; \
	chmod u-w colon.txt; \
	./lab0 --input=backstick.txt --output=colon.txt 1&> /dev/null; \
	if [[ $$? -eq 3 ]]; then \
		:; \
	else \
		echo "fail to write to file and did not exit 3">>trash.txt; \
	fi; \
	rm -f backstick.txt colon.txt
code4test:
	./lab0 --catch --segfault 1&>/dev/null; \
	if [[ $$? -ne 4 ]]; then \
		echo "your segault design is wrong and did not exit 4" >>trash.txt; \
	fi; 
code1test:
	./lab0 --trivial 1&>/dev/null; \
	if [[ $$? -ne 1 ]]; then \
		echo "unknown argument not handled and did not exit 1" >>trash.txt; \
	fi
code0test:
	echo "you don't need 1 to identify stdout but need & to identify the second" >>redirect.txt; \
	echo "-f ignores nonexistant files" >>ffile.txt; \
	./lab0 --input=redirect.txt --output=ffile.txt 1&>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
		echo "success read and write did not exit 0" >> trash.txt; \
	fi; \
	rm -f redirect.txt ffile.txt
finaltest:
	echo "this is the final test, i am almost done with it">>done.txt
	./lab0 --input=done.txt --output=whatever.txt 1&>/dev/null; \
	cmp done.txt whatever.txt; \
	if [[ $$? -ne 0 ]];then \
		echo "content do not match, no specified exit code">>trash.txt;\
	fi;\
	rm -f done.txt whatever.txt
