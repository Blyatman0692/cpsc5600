CPPFLAGS = -std=c++11 -Wall -Werror -pedantic -ggdb -pthread -O2

hw0 : hw0.cpp ThreadGroup.h
	g++ $(CPPFLAGS) $< -o $@

hw0_setup : hw0_setup.cpp ThreadGroup.h
	g++ $(CPPFLAGS) $< -o $@

example : example.cpp ThreadGroup.h
	g++ $(CPPFLAGS) $< -o $@

all : example hw0_setup hw0 hw0_setup.zip hw0.zip
	@echo "Made it all!"

clean :
	rm -f hw0_setup example hw0 *.zip

hw0_setup.zip : hw0_setup.cpp ThreadGroup.h example.cpp Makefile
	zip $@ $^

hw0.zip : hw0.cpp hw0_setup.cpp ThreadGroup.h example.cpp Makefile
	zip $@ $^
