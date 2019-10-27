TARGET = scheme
CC = clang
CFLAGS = -O3 -Wall -Wconversion -Wstrict-prototypes
LIBS = -lm

DIRS = . bigint primitives unicode
HEADERS = $(filter-out unicode/unicode_data.h,$(wildcard $(addsuffix /*.h,$(DIRS))))
OBJECTS = $(patsubst %.c,%.o,$(wildcard $(addsuffix /*.c,$(DIRS))))
COMPILER_SRC = $(wildcard *.scm)
UNICODE_DATA = $(addprefix unicode/,CaseFolding.txt DerivedGeneralCategory.txt PropList.txt SpecialCasing.txt UnicodeData.txt)

.PHONY: all

all: $(TARGET) compiler.sss

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJECTS) -o $@

unicode/unicode.o: unicode/unicode.c unicode/unicode_data.h
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

compiler.sss: $(TARGET)
	./$(TARGET) $(COMPILER_SRC) --compile compiler.sss
	./$(TARGET) $(COMPILER_SRC) --compile _compiler.sss
	diff compiler.sss _compiler.sss || echo "Warning: compiler does not reproduce itself"
	-rm _compiler.sss

unicode/unicode_data.h: $(UNICODE_DATA)
	cd unicode && ./generate.py

unicode/DerivedGeneralCategory.txt:
	cd unicode && wget https://unicode.org/Public/12.1.0/ucd/extracted/$(notdir $@)

unicode/%.txt:
	cd unicode && wget https://unicode.org/Public/12.1.0/ucd/$(notdir $@)
