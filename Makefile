#TODO: replace ID with your own IDS, for example: 123456789_123456789
SUBMITTERS := <student1-ID>_<student2-ID>
COMPILER := g++
COMPILER_FLAGS := --std=c++11 -Wall
SRCS := Commands.cpp signals.cpp smash.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
HDRS := Commands.h signals.h
TESTS_INPUTS := $(wildcard test_input*.txt)
TESTS_OUTPUTS := $(subst input,output,$(TESTS_INPUTS))
SMASH_BIN := smash

$(SMASH_BIN): $(OBJS)
	$(COMPILER) $(COMPILER_FLAGS) $^ -o $@

$(OBJS): %.o: %.cpp
	$(COMPILER) $(COMPILER_FLAGS) -c $^

zip: $(SRCS) $(HDRS)
	zip $(SUBMITTERS).zip $^ submitters.txt Makefile

clean:
	rm -rf $(SMASH_BIN) $(OBJS) $(TESTS_OUTPUTS) 
	rm -rf $(SUBMITTERS).zip
