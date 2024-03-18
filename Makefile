 st = --std=c++20
 lib = -lboost_program_options
 all:
	make task1
	make task2
	make checker

task1:
	g++ task1.cpp $(lib) -o $@
	@echo "для взаимодействия ./$@ --help"
task2:
	g++ task2.cpp -o $@ $(st)

checker:
	g++ task2_check.cpp -o $@ $(st)

remove:all
	rm task1 task2 checker