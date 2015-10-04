# pverify, version 1.0
# Copyright (c) 2012 Zhu Hui
# E-mail: lookatmeyou@126.com
# QQ: 1241745499


objs = http.o pverify.o

pverify : $(objs)
	g++ -o pverify $(objs) -lpthread

$(objs) : http.h
.PHONY : install uninstall
install :
	cp pverify /usr/bin

uninstall :
	-rm /usr/bin/pverify

.PHONY : clean
clean :
	-rm $(objs) pverify
